/*
 *  Copyright (C) 2013-2019 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "FileItem.h"
#include "media/import/IMediaImportHandler.h"
#include "media/import/MediaImportChangesetTypes.h"
#include "media/import/jobs/tasks/IMediaImportTask.h"
#include "threads/CriticalSection.h"
#include "threads/Event.h"

class CMediaImportSynchronisationTaskBase : public IMediaImportTask
{
public:
  virtual ~CMediaImportSynchronisationTaskBase() = default;

  /*!
  * \brief Get the media type being synchronised
  */
  MediaType GetMediaType() const { return m_importHandler->GetMediaType(); }

  // partial implementation of IMediaImportTask
  MediaImportTaskType GetType() const override { return MediaImportTaskType::Synchronisation; }

protected:
  CMediaImportSynchronisationTaskBase(const std::string& name,
                                      const CMediaImport& import,
                                      MediaImportHandlerPtr importHandler);

  virtual bool StartSynchronisation();
  bool StartSynchronisationBatch();
  bool SynchroniseItem(const ChangesetItemPtr& changedItem);
  bool FinishSynchronisationBatch();
  virtual bool FinishSynchronisation();

  MediaImportHandlerPtr m_importHandler;
};

class CMediaImportSynchronisationTask : public CMediaImportSynchronisationTaskBase
{
public:
  CMediaImportSynchronisationTask(const CMediaImport& import,
                                  MediaImportHandlerPtr importHandler,
                                  const ChangesetItems& items);

  // implementation of IMediaImportTask
  bool DoWork() override;

protected:
  bool StartSynchronisation() override;
  bool FinishSynchronisation() override;

  ChangesetItems m_items;
};

class CMediaImportSynchronisationAsyncTask : public CMediaImportSynchronisationTaskBase
{
public:
  CMediaImportSynchronisationAsyncTask(const CMediaImport& import,
                                       MediaImportHandlerPtr importHandler);

  void AddItemsToProcess(const ChangesetItems& items);
  void FinalizeSynchronisation();

  // implementation of IMediaImportTask
  bool DoWork() override;

protected:
  ChangesetItems m_itemsToProcess;

  bool m_finish = false;

  CCriticalSection m_critical;
  CEvent m_processItemsEvent;
};
