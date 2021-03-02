/*
 *  Copyright (C) 2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "FileItem.h"
#include "media/MediaType.h"
#include "media/import/MediaImport.h"
#include "media/import/MediaImportChangesetTypes.h"
#include "media/import/jobs/MediaImportTaskProcessorJob.h"
#include "media/import/jobs/tasks/MediaImportChangesetTask.h"
#include "media/import/jobs/tasks/MediaImportImportItemsRetrievalTask.h"
#include "threads/IRunnable.h"
#include "utils/logtypes.h"

#include <atomic>
#include <memory>
#include <unordered_map>
#include <vector>

class CMediaImportSynchronisationAsyncTask;
class CThread;
class IMediaImportHandler;
class IMediaImportHandlerManager;
class IMediaImporterManager;

class CMediaImportThreadedItemsSynchronizationJob
  : public CMediaImportTaskProcessorJob,
    public IMediaImportItemsRetrievalObserver,
    public IMediaImportChangesetItemsObserver
{
public:
  CMediaImportThreadedItemsSynchronizationJob(const CMediaImport& import,
      const IMediaImporterManager* importerManager,
      const IMediaImportHandlerManager* importHandlerManager,
      IMediaImportTaskCallback* callback);

  // implementation of CJob
  const char* GetType() const override { return "MediaImportThreadedItemsSynchronizationJob"; }

  // specialization of CJob
  bool ShouldCancel(unsigned int progress, unsigned int total) const override;

  // specialization of CLibraryJob
  bool CanBeCancelled() const override { return true; }
  bool Cancel() override;

  // specialization of CMediaImportTaskProcessorJob
  bool DoWork() override;
  bool operator==(const CJob* job) const override;

protected:
  class CAsyncTaskRunner : public IRunnable
  {
  public:
    CAsyncTaskRunner(MediaImportTaskPtr task);

    // implementation of IRunnable
    void Run();

    bool WasSuccessful() const;

  private:
    MediaImportTaskPtr m_task;
    bool m_success;
  };

  using AsyncTaskRunnerPtr = std::shared_ptr<CAsyncTaskRunner>;
  using ThreadPtr = std::shared_ptr<CThread>;

  typedef struct AsyncExecution
  {
    void Reset();

    AsyncTaskRunnerPtr taskRunner;
    ThreadPtr thread;
  } AsyncExecution;

  typedef struct MediaTypeTaskData
  {
    MediaType m_mediaType;
    std::shared_ptr<const IMediaImportHandler> m_importHandler;
    std::vector<CFileItemPtr> m_localItems;

    std::shared_ptr<CMediaImportChangesetAsyncTask> m_changesetTask;
    std::shared_ptr<CMediaImportSynchronisationAsyncTask> m_synchronizationTask;
  } MediaTypeTaskData;

  // implementation of IMediaImportItemsRetrievalObserver
  void ItemsRetrieved(const MediaType& mediaType) override;

  // implementation of IMediaImportChangesetItemsObserver
  void ChangesetDetermined(const MediaType& mediaType,
                           const ChangesetItems& changesetItems) override;

  bool ProcessLocalItemsRetrievalTasks();
  void CreateImportItemsRetrievalTask();
  AsyncExecution StartImportItemsRetrievalTask();
  void CreateChangesetTasks();
  AsyncExecution StartChangesetTask(const MediaTypeTaskData& taskData);
  void CreateSynchronizationTasks();
  AsyncExecution StartSynchronizationTask(const MediaTypeTaskData& taskData);
  void ProcessCleanupTasks();

  static Logger GetLogger();

  CMediaImport m_import;
  const IMediaImporterManager* m_importerManager;
  const IMediaImportHandlerManager* m_importHandlerManager;

  std::atomic<bool> m_cancel = false;

  bool m_partialChangeset = false;
  std::vector<MediaTypeTaskData> m_mediaTypeData;

  std::shared_ptr<CMediaImportImportItemsRetrievalTask> m_itemsRetrievalTask;
};
