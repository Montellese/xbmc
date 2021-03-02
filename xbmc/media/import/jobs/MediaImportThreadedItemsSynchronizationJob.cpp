/*
 *  Copyright (C) 2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "MediaImportThreadedItemsSynchronizationJob.h"

#include "media/import/IMediaImportHandler.h"
#include "media/import/IMediaImportHandlerManager.h"
#include "media/import/IMediaImporter.h"
#include "media/import/IMediaImporterManager.h"
#include "media/import/jobs/tasks/IMediaImportTask.h"
#include "media/import/jobs/tasks/MediaImportCleanupTask.h"
#include "media/import/jobs/tasks/MediaImportLocalItemsRetrievalTask.h"
#include "media/import/jobs/tasks/MediaImportSynchronisationTask.h"
#include "threads/Thread.h"
#include "utils/StringUtils.h"
#include "utils/log.h"

#include <fmt/ostream.h>

CMediaImportThreadedItemsSynchronizationJob::CMediaImportThreadedItemsSynchronizationJob(
    const CMediaImport& import,
    const IMediaImporterManager* importerManager,
    const IMediaImportHandlerManager* importHandlerManager,
    IMediaImportTaskCallback* callback)
  : CMediaImportTaskProcessorJob(import.GetSource(), callback, true),
    m_import(import),
    m_importerManager(importerManager),
    m_importHandlerManager(importHandlerManager)
{
  // get the import handlers
  for (const auto& mediaType : import.GetMediaTypes())
  {
    m_mediaTypeData.emplace_back(
        MediaTypeTaskData{ mediaType, m_importHandlerManager->GetImportHandler(mediaType) });
  }
}

bool CMediaImportThreadedItemsSynchronizationJob::operator==(const CJob* job) const
{
  if (!CMediaImportTaskProcessorJob::operator==(job))
    return false;

  const CMediaImportThreadedItemsSynchronizationJob* rjob =
      dynamic_cast<const CMediaImportThreadedItemsSynchronizationJob*>(job);
  if (rjob == nullptr)
    return false;

  // compare the details
  if (m_import != m_import || m_partialChangeset != m_partialChangeset)
    return false;

  return true;
}

bool CMediaImportThreadedItemsSynchronizationJob::ShouldCancel(unsigned int progress,
                                                               unsigned int total) const
{
  // order matters so that progress is always reported
  return CJob::ShouldCancel(progress, total) || m_cancel;
}

bool CMediaImportThreadedItemsSynchronizationJob::Cancel()
{
  m_cancel = true;
  return true;
}

bool CMediaImportThreadedItemsSynchronizationJob::DoWork()
{
  // get all previously imported items
  if (!ProcessLocalItemsRetrievalTasks())
    return false;

  // create all asynchronous tasks
  CreateImportItemsRetrievalTask();
  CreateChangesetTasks();
  CreateSynchronizationTasks();

  // ATTENTION: start all asynchronous tasks in reverse order
  // start the synchronisation and changeset tasks
  std::unordered_map<MediaType, AsyncExecution> asyncSynchronizations;
  std::unordered_map<MediaType, AsyncExecution> asyncChangesets;
  for (auto& mediaTypeData : m_mediaTypeData)
  {
    asyncSynchronizations.emplace(mediaTypeData.m_mediaType,
                                  StartSynchronizationTask(mediaTypeData));
    asyncChangesets.emplace(mediaTypeData.m_mediaType, StartChangesetTask(mediaTypeData));
  }

  auto asyncImport = StartImportItemsRetrievalTask();

  bool importRunning = true;
  bool changesetsRunning = true;
  bool synchronizationsRunning = true;
  bool success = true;
  while (importRunning || changesetsRunning || synchronizationsRunning)
  {
    if (ShouldCancel(0, 0))
      break;

    // check if the import task has finished
    if (importRunning && !asyncImport.thread->IsRunning())
    {
      importRunning = false;

      bool importTaskSuccess = asyncImport.taskRunner->WasSuccessful();

      // get back the import (in case it has changed)
      m_import = m_itemsRetrievalTask->GetImport();

      // check whether to perform a full or partial changeset
      bool partialChangeset = m_itemsRetrievalTask->IsChangeset();

      // finalize the changeset tasks
      for (auto& mediaTypeData : m_mediaTypeData)
        mediaTypeData.m_changesetTask->FinalizeChangeset(partialChangeset);

      // let the observer know that the task has completed
      importTaskSuccess &= OnTaskComplete(importTaskSuccess, m_itemsRetrievalTask.get());

      // destroy the items retrieval task
      asyncImport.Reset();
      m_itemsRetrievalTask.reset();

      // if processing the task failed abort
      if (!importTaskSuccess)
      {
        GetLogger()->warn("import items retrieval task for items from {} failed", m_import);
        success = false;

        // cancel the job
        Cancel();
        break;
      }
    }

    // if the import task is not running anymore check if the changeset tasks are still running
    if (!importRunning && changesetsRunning)
    {
      // go through all possible changeset tasks
      for (auto&& mediaTypeData = m_mediaTypeData.begin(); mediaTypeData != m_mediaTypeData.end();)
      {
        if (mediaTypeData->m_changesetTask != nullptr)
        {
          auto& asyncChangeset = asyncChangesets.find(mediaTypeData->m_mediaType);
          // if there is no matching async thread and task runner destroy the changeset task
          if (asyncChangeset == asyncChangesets.end())
            mediaTypeData->m_changesetTask.reset();
          // otherwise check whether the async thread has finished
          else if (!asyncChangeset->second.thread->IsRunning())
          {
            // get the result
            bool asyncChangesetSuccess = asyncChangeset->second.taskRunner->WasSuccessful();

            // finish synchronization
            mediaTypeData->m_synchronizationTask->FinalizeSynchronisation();

            // remove the async thread and task runner
            asyncChangesets.erase(asyncChangeset);

            // check if this is the last changeset task
            if (asyncChangesets.empty())
            {
              // let the observer know that the task has completed
              asyncChangesetSuccess &= OnTaskComplete(asyncChangesetSuccess,
                                                      mediaTypeData->m_changesetTask.get());
            }

            // destroy the changeset task
            mediaTypeData->m_changesetTask.reset();

            // if processing the task failed remove the import (no cleanup needed)
            if (!asyncChangesetSuccess)
            {
              GetLogger()->warn("import changeset task for {} items from {} failed",
                  mediaTypeData->m_mediaType, m_import);
              mediaTypeData = m_mediaTypeData.erase(mediaTypeData);
              continue;
            }
          }
        }

        ++mediaTypeData;
      }

      changesetsRunning = !asyncChangesets.empty();
    }

    // if the changeset task isn't running anymore check if the synchronization tasks are still running
    if (!changesetsRunning && synchronizationsRunning)
    {
      // go through all possible changeset tasks
      for (auto& mediaTypeData : m_mediaTypeData)
      {
        if (mediaTypeData.m_synchronizationTask == nullptr)
          continue;

        auto& asyncSynchronization = asyncSynchronizations.find(mediaTypeData.m_mediaType);
        // if there is no matching async thread and task runner destroy the synchronization task
        if (asyncSynchronization == asyncSynchronizations.end())
        {
          mediaTypeData.m_synchronizationTask.reset();
          continue;
        }

        // otherwise check whether the async thread has finished
        if (asyncSynchronization->second.thread->IsRunning())
          continue;

        // get the result
        bool asyncSyncSuccess = asyncSynchronization->second.taskRunner->WasSuccessful();

        // remove the async thread and task runner
        asyncSynchronizations.erase(asyncSynchronization);

        // check if this is the last synchronization task
        if (asyncSynchronizations.empty())
        {
          // let the observer know that the last synchronization task has completed
          asyncSyncSuccess &= OnTaskComplete(asyncSyncSuccess,
                                             mediaTypeData.m_synchronizationTask.get());
        }

        // destroy the finished synchronization task
        mediaTypeData.m_synchronizationTask.reset();

        // if processing the task failed remove the import (no cleanup needed)
        if (!asyncSyncSuccess)
        {
          GetLogger()->warn("import synchronization task for {} items from {} failed",
              mediaTypeData.m_mediaType, m_import);
          // don't remove the import even though it failed because we should run the cleanup
        }
      }

      synchronizationsRunning = !asyncSynchronizations.empty();
    }
  }

  // make sure all async threads and task runners stop
  if (asyncImport.thread != nullptr)
    asyncImport.thread->StopThread(true);
  for (auto& asyncChangeset : asyncChangesets)
    asyncChangeset.second.thread->StopThread(true);
  asyncChangesets.clear();
  for (auto& asyncSynchronization : asyncSynchronizations)
    asyncSynchronization.second.thread->StopThread(true);
  asyncChangesets.clear();

  // cleanup everything
  ProcessCleanupTasks();

  return success;
}

void CMediaImportThreadedItemsSynchronizationJob::ItemsRetrieved(const MediaType& mediaType)
{
  if (m_itemsRetrievalTask == nullptr)
  {
    GetLogger()->error("{} items received for importing but no import task running", mediaType);
    return;
  }
  auto taskData = std::find_if(m_mediaTypeData.begin(), m_mediaTypeData.end(), [mediaType](const MediaTypeTaskData& taskData) { return taskData.m_mediaType == mediaType; });
  if (taskData == m_mediaTypeData.end() || taskData->m_changesetTask == nullptr)
  {
    GetLogger()->error("{} items received for importing but no changeset task running", mediaType);
    return;
  }

  // get (and clear) the retrieved items and pass them to the changeset task for processing
  taskData->m_changesetTask->AddItemsToProcess(m_itemsRetrievalTask->GetAndClearRetrievedItems(mediaType));
}

void CMediaImportThreadedItemsSynchronizationJob::ChangesetDetermined(
    const MediaType& mediaType,
    const ChangesetItems& changesetItems)
{
  auto taskData = std::find_if(m_mediaTypeData.begin(), m_mediaTypeData.end(), [mediaType](const MediaTypeTaskData& taskData) { return taskData.m_mediaType == mediaType; });
  if (taskData == m_mediaTypeData.end() || taskData->m_synchronizationTask == nullptr)
  {
    GetLogger()->error("{} {} changeset items processed but no synchronization task running",
                       changesetItems.size(),
                       mediaType);
    return;
  }

  // pass the changeset items to the synchronization task for processing
  taskData->m_synchronizationTask->AddItemsToProcess(changesetItems);
}

bool CMediaImportThreadedItemsSynchronizationJob::ProcessLocalItemsRetrievalTasks()
{
  std::map<MediaType, MediaImportHandlerPtr> mediaImportHandlers;
  for (const auto& mediaTypeData : m_mediaTypeData)
    mediaImportHandlers.insert(std::make_pair(
        mediaTypeData.m_mediaType, MediaImportHandlerPtr(mediaTypeData.m_importHandler->Create())));

  const CMediaImport& import = m_import;
  auto localItemsRetrievalTask =
      std::make_shared<CMediaImportLocalItemsRetrievalTask>(import, mediaImportHandlers);

  // if processing the task failed remove the import (no cleanup needed)
  GetLogger()->info("starting local items retrieval task for items from {}...", import);
  if (!CMediaImportTaskProcessorJob::ProcessTask(localItemsRetrievalTask))
  {
    GetLogger()->error("local items retrieval task for items from {} failed", import);
    return false;
  }

  // get the local items
  for (auto& mediaTypeData : m_mediaTypeData)
    mediaTypeData.m_localItems = localItemsRetrievalTask->GetLocalItems(mediaTypeData.m_mediaType);

  return true;
}

void CMediaImportThreadedItemsSynchronizationJob::CreateImportItemsRetrievalTask()
{
  m_itemsRetrievalTask =
    std::make_shared<CMediaImportImportItemsRetrievalTask>(m_import, m_importerManager, this);
  m_itemsRetrievalTask->SetProcessorJob(this);

  // add all previously imported items
  const auto& mediaTypes = m_import.GetMediaTypes();
  for (auto& mediaTypeData : m_mediaTypeData)
  {
    if (std::find(mediaTypes.begin(), mediaTypes.end(), mediaTypeData.m_mediaType) !=
        mediaTypes.end())
      m_itemsRetrievalTask->SetLocalItems(mediaTypeData.m_localItems, mediaTypeData.m_mediaType);
  }
}

CMediaImportThreadedItemsSynchronizationJob::AsyncExecution
CMediaImportThreadedItemsSynchronizationJob::StartImportItemsRetrievalTask()
{
  GetLogger()->info("starting import items retrieval task for items from {}...", m_import);
  auto asyncImportTask = std::make_shared<CAsyncTaskRunner>(m_itemsRetrievalTask);
  auto asyncImportThread = std::make_shared<CThread>(asyncImportTask.get(), "MediaImportAsyncItemsRetrieval");
  asyncImportThread->Create();

  return { std::move(asyncImportTask), std::move(asyncImportThread) };
}

void CMediaImportThreadedItemsSynchronizationJob::CreateChangesetTasks()
{
  for (auto& mediaTypeData : m_mediaTypeData)
  {
    mediaTypeData.m_changesetTask = std::make_shared<CMediaImportChangesetAsyncTask>(
        m_import,
        MediaImportHandlerPtr(mediaTypeData.m_importHandler->Create()),
        mediaTypeData.m_localItems,
        this);
    mediaTypeData.m_changesetTask->SetProcessorJob(this);
  }
}

CMediaImportThreadedItemsSynchronizationJob::AsyncExecution
CMediaImportThreadedItemsSynchronizationJob::StartChangesetTask(
    const MediaTypeTaskData& taskData)
{
  GetLogger()->info("starting import changeset task for {} items from {}...",
      taskData.m_mediaType, m_import);
  auto asyncChangeset = std::make_shared<CAsyncTaskRunner>(taskData.m_changesetTask);
  auto asyncChangesetThread = std::make_shared<CThread>(
      asyncChangeset.get(),
      StringUtils::Format("MediaImportAsyncChangeset[{}]", taskData.m_mediaType).c_str());
  asyncChangesetThread->Create();

  return { std::move(asyncChangeset), std::move(asyncChangesetThread) };
}

void CMediaImportThreadedItemsSynchronizationJob::CreateSynchronizationTasks()
{
  for (auto& mediaTypeData : m_mediaTypeData)
  {
    mediaTypeData.m_synchronizationTask = std::make_shared<CMediaImportSynchronisationAsyncTask>(
      m_import,
      MediaImportHandlerPtr(mediaTypeData.m_importHandler->Create()));
    mediaTypeData.m_synchronizationTask->SetProcessorJob(this);
  }
}

CMediaImportThreadedItemsSynchronizationJob::AsyncExecution
CMediaImportThreadedItemsSynchronizationJob::StartSynchronizationTask(
    const MediaTypeTaskData& taskData)
{
  GetLogger()->info("starting import synchronisation task for {} items from {}...",
      taskData.m_mediaType, m_import);
  auto asyncSync = std::make_shared<CAsyncTaskRunner>(taskData.m_synchronizationTask);
  auto asyncSyncThread = std::make_shared<CThread>(
      asyncSync.get(),
      StringUtils::Format("MediaImportAsyncSynchronization[{}]", taskData.m_mediaType).c_str());
  asyncSyncThread->Create();

  return { std::move(asyncSync), std::move(asyncSyncThread) };
}

void CMediaImportThreadedItemsSynchronizationJob::ProcessCleanupTasks()
{
  // go through all media types in the proper order and clean them up
  for (auto&& mediaTypeData = m_mediaTypeData.rbegin(); mediaTypeData != m_mediaTypeData.rend();
       ++mediaTypeData)
  {
    const CMediaImport& import = m_import;
    auto cleanupTask = std::make_shared<CMediaImportCleanupTask>(
        import, MediaImportHandlerPtr(mediaTypeData->m_importHandler->Create()));

    // if processing the task failed remove the import (no cleanup needed)
    GetLogger()->info("starting import cleanup task for {} items from {}...",
                      mediaTypeData->m_mediaType, import);
    if (!CMediaImportTaskProcessorJob::ProcessTask(cleanupTask))
    {
      GetLogger()->warn("import cleanup task for {} items from {} failed",
                        mediaTypeData->m_mediaType, import);
    }
  }
}

Logger CMediaImportThreadedItemsSynchronizationJob::GetLogger()
{
  static Logger s_logger =
      CServiceBroker::GetLogging().GetLogger("CMediaImportThreadedItemsSynchronizationJob");
  return s_logger;
}

CMediaImportThreadedItemsSynchronizationJob::CAsyncTaskRunner::CAsyncTaskRunner(
    MediaImportTaskPtr task)
  : m_task(task),
    m_success(false)
{
}

void CMediaImportThreadedItemsSynchronizationJob::CAsyncTaskRunner::Run()
{
  m_success = false;

  if (m_task != nullptr)
    m_success = m_task->DoWork();
}

bool CMediaImportThreadedItemsSynchronizationJob::CAsyncTaskRunner::WasSuccessful() const
{
  return m_success;
}

void CMediaImportThreadedItemsSynchronizationJob::AsyncExecution::Reset()
{
  thread.reset();
  taskRunner.reset();
}
