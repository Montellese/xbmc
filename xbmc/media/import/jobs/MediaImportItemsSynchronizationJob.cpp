/*
 *  Copyright (C) 2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "MediaImportItemsSynchronizationJob.h"

#include "media/import/IMediaImportHandler.h"
#include "media/import/IMediaImportHandlerManager.h"
#include "media/import/IMediaImporter.h"
#include "media/import/IMediaImporterManager.h"
#include "media/import/jobs/tasks/IMediaImportTask.h"
#include "media/import/jobs/tasks/MediaImportChangesetTask.h"
#include "media/import/jobs/tasks/MediaImportCleanupTask.h"
#include "media/import/jobs/tasks/MediaImportImportItemsRetrievalTask.h"
#include "media/import/jobs/tasks/MediaImportLocalItemsRetrievalTask.h"
#include "media/import/jobs/tasks/MediaImportSynchronisationTask.h"
#include "utils/log.h"

#include <fmt/ostream.h>

CMediaImportItemsSynchronizationJob::CMediaImportItemsSynchronizationJob(
    const CMediaImportSource& source,
    const IMediaImporterManager* importerManager,
    const IMediaImportHandlerManager* importHandlerManager,
    IMediaImportTaskCallback* callback,
    bool hasProgress)
  : CMediaImportTaskProcessorJob(source, callback, hasProgress),
    m_importerManager(importerManager),
    m_importHandlerManager(importHandlerManager),
    m_partialChangeset(false)
{
}

CMediaImportItemsSynchronizationJob* CMediaImportItemsSynchronizationJob::Import(
    const CMediaImport& import,
    bool automatically,
    const IMediaImporterManager* importerManager,
    const IMediaImportHandlerManager* importHandlerManager,
    IMediaImportTaskCallback* callback)
{
  if (importerManager == nullptr)
  {
    GetLogger()->error("invalid media importer manager implementation");
    return nullptr;
  }

  if (importHandlerManager == nullptr)
  {
    GetLogger()->error("invalid media import handler manager implementation");
    return nullptr;
  }

  CMediaImport tmpImport = import;
  if (automatically && tmpImport.Settings()->Load() &&
      tmpImport.Settings()->GetImportTrigger() != MediaImportTrigger::Auto)
  {
    GetLogger()->debug("automatic import of items from {} is disabled", import);
    return nullptr;
  }

  auto processorJob = new CMediaImportItemsSynchronizationJob(import.GetSource(), importerManager,
                                                              importHandlerManager, callback, true);
  if (!processorJob->SetImport(import, {}))
  {
    GetLogger()->warn("failed to import items from {}", import);
    return nullptr;
  }

  return processorJob;
}

CMediaImportItemsSynchronizationJob* CMediaImportItemsSynchronizationJob::ChangeImportedItems(
    const CMediaImport& import,
    const ChangesetItems& items,
    const IMediaImportHandlerManager* importHandlerManager,
    IMediaImportTaskCallback* callback)
{
  if (importHandlerManager == nullptr)
  {
    GetLogger()->error("invalid media import handler manager implementation");
    return nullptr;
  }

  auto processorJob = new CMediaImportItemsSynchronizationJob(
      import.GetSource(), nullptr, importHandlerManager, callback, false);

  // set the import and remember to perform a partial changeset
  processorJob->m_import = import;
  processorJob->m_partialChangeset = true;

  // prepare the media type data
  std::map<MediaType, MediaTypeTaskData> mediaTypeDataMap;
  for (const auto& mediaType : import.GetMediaTypes())
  {
    // get the import handler
    const auto importer = importHandlerManager->GetImportHandler(mediaType);
    if (importer == nullptr)
      continue;

    mediaTypeDataMap.insert(std::make_pair(mediaType, MediaTypeTaskData{mediaType, importer}));
  }

  for (const auto& changedItem : items)
  {
    if (changedItem.second == nullptr)
      continue;

    // check the media type
    auto mediaTypeDataIt = mediaTypeDataMap.find(changedItem.second->GetMediaType());
    if (mediaTypeDataIt == mediaTypeDataMap.end())
      continue;

    mediaTypeDataIt->second.m_importedItems.push_back(changedItem);
  }

  for (const auto& mediaTypeData : mediaTypeDataMap)
  {
    // ignore media type data without any changed items
    if (mediaTypeData.second.m_importedItems.empty())
      continue;

    processorJob->m_mediaTypeData.push_back(mediaTypeData.second);
  }

  // get all local items
  processorJob->m_taskTypesToBeProcessed.push_back(MediaImportTaskType::LocalItemsRetrieval);

  // determine the partial changeset for the given items
  processorJob->m_taskTypesToBeProcessed.push_back(MediaImportTaskType::Changeset);

  // do a sychronisation and cleanup
  processorJob->m_taskTypesToBeProcessed.push_back(MediaImportTaskType::Synchronisation);
  processorJob->m_taskTypesToBeProcessed.push_back(MediaImportTaskType::Cleanup);

  return processorJob;
}

bool CMediaImportItemsSynchronizationJob::operator==(const CJob* job) const
{
  if (!CMediaImportTaskProcessorJob::operator==(job))
    return false;

  const CMediaImportItemsSynchronizationJob* rjob =
      dynamic_cast<const CMediaImportItemsSynchronizationJob*>(job);
  if (rjob == nullptr)
    return false;

  // compare the details
  if (m_import != m_import || m_partialChangeset != m_partialChangeset)
    return false;

  return true;
}

bool CMediaImportItemsSynchronizationJob::DoWork()
{
  // check if no task is set and there are no more task types to be performed
  if (m_task == nullptr && m_taskTypesToBeProcessed.empty())
    return true;

  // if a task is set perform it
  if (m_task != nullptr)
  {
    // let the current task do its work
    return CMediaImportTaskProcessorJob::ProcessTask(m_task);
  }

  // there's no task set and still task types to perform so go through all the media imports and perform the next task type
  MediaImportTaskType currentTaskType = m_taskTypesToBeProcessed.front();

  switch (currentTaskType)
  {
    case MediaImportTaskType::LocalItemsRetrieval:
      ProcessLocalItemsRetrievalTasks();
      break;

    case MediaImportTaskType::ImportItemsRetrieval:
      ProcessImportItemsRetrievalTasks();
      break;

    case MediaImportTaskType::Changeset:
      ProcessChangesetTasks();
      break;

    case MediaImportTaskType::Synchronisation:
      ProcessSynchronisationTasks();
      break;

    case MediaImportTaskType::Cleanup:
      ProcessCleanupTasks();
      break;

    default:
      GetLogger()->warn("unknown import task type {}", static_cast<int>(currentTaskType));
      return false;
  }

  // remove the processed task type from the list of task types to process
  m_taskTypesToBeProcessed.erase(m_taskTypesToBeProcessed.begin());

  // let's do another round of processing in case there's more to do
  return DoWork();
}

void CMediaImportItemsSynchronizationJob::ProcessLocalItemsRetrievalTasks()
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
    return;
  }

  // get the local items
  for (auto& mediaTypeData : m_mediaTypeData)
    mediaTypeData.m_localItems = localItemsRetrievalTask->GetLocalItems(mediaTypeData.m_mediaType);
}

void CMediaImportItemsSynchronizationJob::ProcessImportItemsRetrievalTasks()
{
  const CMediaImport& import = m_import;
  auto importItemsRetrievalTask =
      std::make_shared<CMediaImportImportItemsRetrievalTask>(import, m_importerManager);

  // add all previously imported items
  const auto& mediaTypes = import.GetMediaTypes();
  for (auto& mediaTypeData : m_mediaTypeData)
  {
    if (std::find(mediaTypes.begin(), mediaTypes.end(), mediaTypeData.m_mediaType) !=
        mediaTypes.end())
      importItemsRetrievalTask->SetLocalItems(mediaTypeData.m_localItems,
                                              mediaTypeData.m_mediaType);
  }

  // if processing the task failed remove the import (no cleanup needed)
  GetLogger()->info("starting import items retrieval task for items from {}...", import);
  if (!CMediaImportTaskProcessorJob::ProcessTask(importItemsRetrievalTask))
  {
    GetLogger()->warn("import items retrieval task for items from {} failed", import);
    return;
  }

  // get back the import (in case it has changed)
  m_import = importItemsRetrievalTask->GetImport();

  // check whether to perform a full or partial changeset
  m_partialChangeset = importItemsRetrievalTask->IsChangeset();

  // get the retrieved items
  for (auto& mediaTypeData : m_mediaTypeData)
  {
    mediaTypeData.m_importedItems =
        importItemsRetrievalTask->GetRetrievedItems(mediaTypeData.m_mediaType);
  }
}

void CMediaImportItemsSynchronizationJob::ProcessChangesetTasks()
{
  const CMediaImport& import = m_import;
  for (auto&& mediaTypeData = m_mediaTypeData.begin(); mediaTypeData != m_mediaTypeData.end();)
  {
    auto changesetTask = std::make_shared<CMediaImportChangesetTask>(
        import, MediaImportHandlerPtr(mediaTypeData->m_importHandler->Create()),
        mediaTypeData->m_localItems, mediaTypeData->m_importedItems, m_partialChangeset);

    // if processing the task failed remove the import (no cleanup needed)
    GetLogger()->info("starting import changeset task for {} items from {}...",
                      mediaTypeData->m_mediaType, import);
    if (!CMediaImportTaskProcessorJob::ProcessTask(changesetTask))
    {
      GetLogger()->warn("import changeset task for {} items from {} failed",
                        mediaTypeData->m_mediaType, import);
      mediaTypeData = m_mediaTypeData.erase(mediaTypeData);
      continue;
    }

    // get the changeset
    mediaTypeData->m_importedItems = changesetTask->GetChangeset();

    // if the changeset is empty there is nothing else to do
    if (mediaTypeData->m_importedItems.empty())
    {
      GetLogger()->debug("no {} items from {} changed", mediaTypeData->m_mediaType, import);
    }

    ++mediaTypeData;
  }
}

void CMediaImportItemsSynchronizationJob::ProcessSynchronisationTasks()
{
  // go through all media types in the proper order and perform the synchronisation
  for (auto& mediaTypeData : m_mediaTypeData)
  {
    const CMediaImport& import = m_import;
    auto synchronisationTask = std::make_shared<CMediaImportSynchronisationTask>(
        import,
        MediaImportHandlerPtr(mediaTypeData.m_importHandler->Create()),
        mediaTypeData.m_importedItems);

    // if processing the task failed remove the import (no cleanup needed)
    GetLogger()->info("starting import synchronisation task for {} items from {}...",
                      mediaTypeData.m_mediaType, import);
    if (!CMediaImportTaskProcessorJob::ProcessTask(synchronisationTask))
    {
      GetLogger()->warn("import synchronization task for {} items from {} failed",
                        mediaTypeData.m_mediaType, import);
      // don't remove the import even though it failed because we should run the cleanup
    }
  }
}

void CMediaImportItemsSynchronizationJob::ProcessCleanupTasks()
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

bool CMediaImportItemsSynchronizationJob::SetImport(
    const CMediaImport& import, std::vector<MediaImportTaskType> tasksToBeProcessed)
{
  if (m_importHandlerManager == nullptr)
  {
    GetLogger()->error("invalid media import handler manager implementation");
    return false;
  }

  // set the import
  m_import = import;

  // get the import handlers
  for (const auto& mediaType : import.GetMediaTypes())
  {
    MediaTypeTaskData mediaTypeData = {mediaType,
                                       m_importHandlerManager->GetImportHandler(mediaType)};
    if (mediaTypeData.m_importHandler == nullptr)
      return false;

    m_mediaTypeData.push_back(mediaTypeData);
  }

  m_taskTypesToBeProcessed = tasksToBeProcessed;

  // determine the tasks (and their order) to process
  if (m_taskTypesToBeProcessed.empty())
  {
    // always do a retrieval
    m_taskTypesToBeProcessed.push_back(MediaImportTaskType::LocalItemsRetrieval);
    m_taskTypesToBeProcessed.push_back(MediaImportTaskType::ImportItemsRetrieval);

    // also add the changeset task (even though it might not be performed depending on the importer being used)
    m_taskTypesToBeProcessed.push_back(MediaImportTaskType::Changeset);

    // always do a sychronisation and cleanup
    m_taskTypesToBeProcessed.push_back(MediaImportTaskType::Synchronisation);
    m_taskTypesToBeProcessed.push_back(MediaImportTaskType::Cleanup);
  }

  return true;
}

Logger CMediaImportItemsSynchronizationJob::GetLogger()
{
  static Logger s_logger =
      CServiceBroker::GetLogging().GetLogger("CMediaImportItemsSynchronizationJob");
  return s_logger;
}
