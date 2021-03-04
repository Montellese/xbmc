/*
 *  Copyright (C) 2013-2019 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "MovieImportHandler.h"

#include "FileItem.h"
#include "media/import/MediaImport.h"
#include "utils/log.h"
#include "video/VideoDatabase.h"

#include <fmt/ostream.h>

bool CMovieImportHandler::UpdateImportedItem(const CMediaImport& import, CFileItem* item)
{
  if (item == nullptr || !item->HasVideoInfoTag() || item->GetVideoInfoTag()->m_iDbId <= 0)
    return false;

  if (m_db.SetDetailsForMovieInTransaction(*(item->GetVideoInfoTag()), item->GetArt(),
                                           item->GetVideoInfoTag()->m_iDbId) <= 0)
  {
    GetLogger()->error("failed to set details for movie \"{}\" imported from {}", item->GetLabel(),
                       import);
    return false;
  }

  if (import.Settings()->UpdatePlaybackMetadataFromSource())
    SetDetailsForFile(m_db, item, true);

  return true;
}

bool CMovieImportHandler::RemoveImportedItem(const CMediaImport& import, const CFileItem* item)
{
  if (item == nullptr || !item->HasVideoInfoTag())
    return false;

  m_db.DeleteMovieInTransaction(item->GetVideoInfoTag()->m_iDbId);
  RemoveFile(m_db, item);

  return true;
}

bool CMovieImportHandler::GetLocalItems(CVideoDatabase& videodb,
                                        const CMediaImport& import,
                                        std::vector<CFileItemPtr>& items)
{
  CVideoDbUrl videoUrl;
  videoUrl.FromString("videodb://movies/titles/");
  videoUrl.AddOption("imported", true);
  videoUrl.AddOption("source", import.GetSource().GetIdentifier());
  videoUrl.AddOption("import", import.GetMediaTypesAsString());

  CFileItemList movies;
  if (!videodb.GetMoviesByWhere(videoUrl.ToString(), CDatabase::Filter(), movies, SortDescription(),
                                import.Settings()->UpdateImportedMediaItems() ? VideoDbDetailsAll
                                                                              : VideoDbDetailsNone))
  {
    GetLogger()->error("failed to get previously imported movies from {}", import);
    return false;
  }

  items.insert(items.begin(), movies.cbegin(), movies.cend());

  return true;
}

std::set<Field> CMovieImportHandler::IgnoreDifferences() const
{
  return {
      FieldAlbum,          FieldArtist,     FieldEpisodeNumber,     FieldEpisodeNumberSpecialSort,
      FieldProductionCode, FieldSeason,     FieldSeasonSpecialSort, FieldTrackNumber,
      FieldTvShowStatus,   FieldTvShowTitle};
}

bool CMovieImportHandler::AddImportedItem(CVideoDatabase& videodb,
                                          const CMediaImport& import,
                                          CFileItem* item)
{
  if (item == nullptr)
    return false;

  PrepareItem(videodb, import, item);

  item->GetVideoInfoTag()->m_iDbId =
      videodb.SetDetailsForMovieInTransaction(*(item->GetVideoInfoTag()), item->GetArt());
  if (item->GetVideoInfoTag()->m_iDbId <= 0)
  {
    GetLogger()->error("failed to add movie \"{}\" imported from {}", item->GetLabel(), import);
    return false;
  }

  SetDetailsForFile(videodb, item, false);
  return SetImportForItem(videodb, item, import, m_db.GetFileId(item->GetPath()));
}
