/*
 *  Copyright (C) 2013-2019 Team Kodi
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
#include "media/import/jobs/MediaImportTaskTypes.h"
#include "utils/logtypes.h"

#include <memory>
#include <vector>

class IMediaImportHandler;
class IMediaImportHandlerManager;
class IMediaImporterManager;

class CMediaImportItemsSynchronizationJob : public CMediaImportTaskProcessorJob
{
public:
  static CMediaImportItemsSynchronizationJob* Import(
      const CMediaImport& import,
      bool automatically,
      const IMediaImporterManager* importerManager,
      const IMediaImportHandlerManager* importHandlerManager,
      IMediaImportTaskCallback* callback);

  static CMediaImportItemsSynchronizationJob* ChangeImportedItems(
      const CMediaImport& import,
      const ChangesetItems& items,
      const IMediaImportHandlerManager* importHandlerManager,
      IMediaImportTaskCallback* callback);

  // implementation of CJob
  const char* GetType() const override { return "MediaImportItemsSynchronizationJob"; }

  // specialization of CMediaImportTaskProcessorJob
  bool DoWork() override;
  bool operator==(const CJob* job) const override;

protected:
  CMediaImportItemsSynchronizationJob(const CMediaImportSource& source,
                                      const IMediaImporterManager* importerManager,
                                      const IMediaImportHandlerManager* importHandlerManager,
                                      IMediaImportTaskCallback* callback,
                                      bool hasProgress);

  void ProcessLocalItemsRetrievalTasks();
  void ProcessImportItemsRetrievalTasks();
  void ProcessChangesetTasks();
  void ProcessSynchronisationTasks();
  void ProcessCleanupTasks();
  bool OnTaskComplete(bool success, const IMediaImportTask* task);

  bool SetImport(const CMediaImport& import, std::vector<MediaImportTaskType> tasksToBeProcessed);

  static Logger GetLogger();

  const IMediaImporterManager* m_importerManager;
  const IMediaImportHandlerManager* m_importHandlerManager;

  CMediaImport m_import;

  bool m_partialChangeset;
  std::vector<MediaImportTaskType> m_taskTypesToBeProcessed;

  typedef struct MediaTypeTaskData
  {
    MediaType m_mediaType;
    std::shared_ptr<const IMediaImportHandler> m_importHandler;
    std::vector<CFileItemPtr> m_localItems;
    ChangesetItems m_importedItems;
  } MediaTypeTaskData;
  std::vector<MediaTypeTaskData> m_mediaTypeData;
};
