/*
 *  Copyright (C) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "media/import/handlers/VideoImportHandler.h"

class CMovieSetImportHandler : public CVideoImportHandler
{
public:
  CMovieSetImportHandler(const IMediaImportHandlerManager* importHandlerManager)
    : CVideoImportHandler(importHandlerManager)
  {
  }
  virtual ~CMovieSetImportHandler() = default;

  CMovieSetImportHandler* Create() const override
  {
    return new CMovieSetImportHandler(m_importHandlerManager);
  }

  MediaType GetMediaType() const override { return MediaTypeVideoCollection; }
  MediaTypes GetRequiredMediaTypes() const override { return {MediaTypeMovie}; }
  GroupedMediaTypes GetGroupedMediaTypes() const override
  {
    return {MediaTypeMovie, MediaTypeVideoCollection};
  }

  CFileItemPtr FindMatchingLocalItem(const CMediaImport& import,
                                     const CFileItem* item,
                                     const std::vector<CFileItemPtr>& localItems) const override;

  bool UpdateImportedItem(const CMediaImport& import, CFileItem* item) override;
  bool RemoveImportedItem(const CMediaImport& import, const CFileItem* item) override;
  bool CleanupImportedItems(const CMediaImport& import) override;

protected:
  bool GetLocalItems(CVideoDatabase& videodb,
                     const CMediaImport& import,
                     std::vector<CFileItemPtr>& items) override;

  std::set<Field> IgnoreDifferences() const override;

  bool AddImportedItem(CVideoDatabase& videodb,
                       const CMediaImport& import,
                       CFileItem* item) override;

  bool RemoveImportedItems(CVideoDatabase& videodb,
                           const CMediaImport& import,
                           bool onlyIfEmpty) override;
  bool RemoveImportedItem(CVideoDatabase& videodb,
                          const CMediaImport& import,
                          const CFileItem* item,
                          bool onlyIfEmpty);
};
