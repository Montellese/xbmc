/*
 *  Copyright (C) 2013-2019 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "dbwrappers/Database.h"
#include "media/import/IMediaImportHandler.h"
#include "utils/logtypes.h"
#include "video/VideoDatabase.h"
#include "video/VideoThumbLoader.h"

class CFileItem;

class CVideoImportHandler : public IMediaImportHandler
{
public:
  virtual ~CVideoImportHandler() = default;

  std::string GetItemLabel(const CFileItem* item) const override;

  bool GetLocalItems(const CMediaImport& import, std::vector<CFileItemPtr>& items) override;

  bool StartChangeset(const CMediaImport& import) override;
  bool FinishChangeset(const CMediaImport& import) override;
  CFileItemPtr FindMatchingLocalItem(const CMediaImport& import,
                                     const CFileItem* item,
                                     const std::vector<CFileItemPtr>& localItems) const override;
  MediaImportChangesetType DetermineChangeset(const CMediaImport& import,
                                              const CFileItem* item,
                                              const CFileItemPtr& localItem) override;
  void PrepareImportedItem(const CMediaImport& import,
                           CFileItem* item,
                           const CFileItemPtr& localItem) const override;

  bool StartSynchronisation(const CMediaImport& import) override;
  bool FinishSynchronisation(const CMediaImport& import) override;

  bool AddImportedItem(const CMediaImport& import, CFileItem* item) override;

  bool RemoveImportedItems(const CMediaImport& import) override;

  void SetImportedItemsEnabled(const CMediaImport& import, bool enable) override;

protected:
  CVideoImportHandler(const IMediaImportHandlerManager* importHandlerManager);

  virtual bool GetLocalItems(CVideoDatabase& videodb,
                             const CMediaImport& import,
                             std::vector<CFileItemPtr>& items) = 0;

  virtual std::set<Field> IgnoreDifferences() const { return std::set<Field>(); }

  virtual bool AddImportedItem(CVideoDatabase& videodb,
                               const CMediaImport& import,
                               CFileItem* item) = 0;

  virtual bool RemoveImportedItems(CVideoDatabase& videodb,
                                   const CMediaImport& import,
                                   bool onlyIfEmpty);

  void PrepareItem(CVideoDatabase& videodb, const CMediaImport& import, CFileItem* pItem);
  void SetDetailsForFile(CVideoDatabase& videodb, const CFileItem* pItem, bool reset);
  bool SetImportForItem(CVideoDatabase& videodb,
                        const CFileItem* pItem,
                        const CMediaImport& import,
                        int idFilesystem = -1);

  bool Compare(const CFileItem* originalItem,
               const CFileItem* newItem,
               bool allMetadata = true,
               bool playbackMetadata = true) const;

  static int GetTotalItemsInDb(const CFileItemList& itemsFromDb);

  static void RemoveFile(CVideoDatabase& videodb, const CFileItem* item);

  static void RemoveAutoArtwork(CGUIListItem::ArtMap& artwork,
                                const std::set<std::string>& parentPrefixes);

  static Logger GetLogger();

  CVideoDatabase m_db;
  CVideoThumbLoader m_thumbLoader;
  std::map<std::string, int> m_sourceIds;
};
