#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "dbwrappers/Database.h"
#include "media/import/IMediaImportHandler.h"
#include "video/VideoDatabase.h"
#include "video/VideoThumbLoader.h"

class CFileItem;

class CVideoImportHandler : public IMediaImportHandler
{
public:
  virtual ~CVideoImportHandler() { }

  virtual std::string GetItemLabel(const CFileItem* item) const override;

  virtual bool GetLocalItems(const CMediaImport &import, std::vector<CFileItemPtr>& items) const override;

  virtual bool StartChangeset(const CMediaImport &import) override;
  virtual bool FinishChangeset(const CMediaImport &import) override;
  virtual MediaImportChangesetType DetermineChangeset(const CMediaImport &import, CFileItem* item, std::vector<CFileItemPtr>& localItems) override;

  virtual bool StartSynchronisation(const CMediaImport &import) override;
  virtual bool FinishSynchronisation(const CMediaImport &import) override;

  virtual bool RemoveImportedItems(const CMediaImport &import) override;

  virtual void SetImportedItemsEnabled(const CMediaImport &import, bool enable) override;

protected:
  CVideoImportHandler(const IMediaImportHandlerManager* importHandlerManager)
    : IMediaImportHandler(importHandlerManager)
  { }

  virtual bool GetLocalItems(CVideoDatabase &videodb, const CMediaImport &import, std::vector<CFileItemPtr>& items) const = 0;

  virtual CFileItemPtr FindMatchingLocalItem(const CFileItem* item, std::vector<CFileItemPtr>& localItems) const;
  virtual MediaImportChangesetType DetermineChangeset(const CMediaImport &import, CFileItem* item, CFileItemPtr localItem, std::vector<CFileItemPtr>& localItems, bool updatePlaybackMetadata);

  virtual bool RemoveImportedItems(CVideoDatabase &videodb, const CMediaImport &import) const;
  virtual void RemoveImportedItem(CVideoDatabase &videodb, const CMediaImport &import, const CFileItem* item) const = 0;

  void PrepareItem(const CMediaImport &import, CFileItem* pItem);
  void PrepareExistingItem(CFileItem *updatedItem, const CFileItem *originalItem);
  void SetDetailsForFile(const CFileItem *pItem, bool reset);
  bool SetImportForItem(const CFileItem *pItem, const CMediaImport &import);
  void RemoveFile(CVideoDatabase &videodb, const CFileItem *item) const;

  static bool Compare(const CFileItem *originalItem, const CFileItem *newItem, bool allMetadata = true, bool playbackMetadata = true);

  mutable CVideoDatabase m_db;
  CVideoThumbLoader m_thumbLoader;
};
