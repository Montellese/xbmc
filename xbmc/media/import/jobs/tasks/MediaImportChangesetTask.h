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

#include <vector>

class CMediaImportChangesetTaskBase : public IMediaImportTask
{
public:
  virtual ~CMediaImportChangesetTaskBase() = default;

  // partial implementation of IMediaImportTask
  MediaImportTaskType GetType() const override { return MediaImportTaskType::Changeset; }

protected:
  CMediaImportChangesetTaskBase(const std::string& name,
                                const CMediaImport& import,
                                MediaImportHandlerPtr importHandler,
                                const std::vector<CFileItemPtr>& localItems);

  bool StartChangeset();

  MediaImportChangesetType DetermineChangeset(MediaImportChangesetType changesetType,
                                              CFileItemPtr item,
                                              CFileItemPtr& matchingLocalItem) const;

  MediaImportChangesetType ProcessImportedItem(MediaImportChangesetType changesetType,
                                               CFileItemPtr item);

  ChangesetItems FinishChangeset(bool partialChangeset);

  std::vector<CFileItemPtr> m_localItems;
  MediaImportHandlerPtr m_importHandler;
};

class CMediaImportChangesetTask : public CMediaImportChangesetTaskBase
{
public:
  CMediaImportChangesetTask(const CMediaImport& import,
                            MediaImportHandlerPtr importHandler,
                            const std::vector<CFileItemPtr>& localItems,
                            const ChangesetItems& retrievedItems,
                            bool partialChangeset = false);
  virtual ~CMediaImportChangesetTask() = default;

  const ChangesetItems& GetChangeset() const { return m_retrievedItems; }

  // implementation of IMediaImportTask
  bool DoWork() override;

private:
  ChangesetItems m_retrievedItems;
  bool m_partialChangeset;
};

class IMediaImportChangesetItemsObserver
{
public:
  virtual ~IMediaImportChangesetItemsObserver() = default;

  virtual void ChangesetDetermined(const MediaType& mediaType,
                                   const ChangesetItems& changesetItems) = 0;
};

class CMediaImportChangesetAsyncTask : public CMediaImportChangesetTaskBase
{
public:
  CMediaImportChangesetAsyncTask(const CMediaImport& import,
                                 MediaImportHandlerPtr importHandler,
                                 const std::vector<CFileItemPtr>& localItems,
                                 IMediaImportChangesetItemsObserver* observer);
  virtual ~CMediaImportChangesetAsyncTask() = default;

  void AddItemsToProcess(const ChangesetItems& items);
  void FinalizeChangeset(bool partialChangeset);

  // implementation of IMediaImportTask
  bool DoWork() override;

private:
  void NotifyChangesetItemsObserver(const ChangesetItems& changesetItems);

  IMediaImportChangesetItemsObserver* m_changesetItemsObserver;

  ChangesetItems m_itemsToProcess;

  bool m_partialChangeset = false;
  bool m_finish = false;

  mutable CCriticalSection m_critical;
  CEvent m_processItemsEvent;
};
