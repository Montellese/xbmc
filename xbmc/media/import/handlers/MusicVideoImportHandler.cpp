/*
 *  Copyright (C) 2013-2019 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "MusicVideoImportHandler.h"

#include "FileItem.h"
#include "media/import/MediaImport.h"
#include "utils/log.h"
#include "video/VideoDatabase.h"

#include <fmt/ostream.h>

bool CMusicVideoImportHandler::UpdateImportedItem(const CMediaImport& import, CFileItem* item)
{
  if (item == nullptr || !item->HasVideoInfoTag() || item->GetVideoInfoTag()->m_iDbId <= 0)
    return false;

  if (m_db.SetDetailsForMusicVideo(item->GetPath(), *(item->GetVideoInfoTag()), item->GetArt(),
                                   item->GetVideoInfoTag()->m_iDbId) <= 0)
  {
    GetLogger()->error("failed to set details for music video \"{}\" imported from {}",
                       item->GetLabel(), import);
    return false;
  }

  if (import.Settings()->UpdatePlaybackMetadataFromSource())
    SetDetailsForFile(m_db, item, true);

  return true;
}

bool CMusicVideoImportHandler::RemoveImportedItem(const CMediaImport& import, const CFileItem* item)
{
  if (item == nullptr || !item->HasVideoInfoTag())
    return false;

  m_db.DeleteMusicVideo(item->GetVideoInfoTag()->m_iDbId);
  RemoveFile(m_db, item);

  return true;
}

bool CMusicVideoImportHandler::GetLocalItems(CVideoDatabase& videodb,
                                             const CMediaImport& import,
                                             std::vector<CFileItemPtr>& items)
{
  CVideoDbUrl videoUrl;
  videoUrl.FromString("videodb://musicvideos/titles/");
  videoUrl.AddOption("imported", true);
  videoUrl.AddOption("source", import.GetSource().GetIdentifier());
  videoUrl.AddOption("import", import.GetMediaTypesAsString());

  CFileItemList musicvideos;
  if (!videodb.GetMusicVideosByWhere(
          videoUrl.ToString(), CDatabase::Filter(), musicvideos, true, SortDescription(),
          import.Settings()->UpdateImportedMediaItems() ? VideoDbDetailsAll : VideoDbDetailsNone))
  {
    GetLogger()->error("failed to get previously imported seasons from {}", import);
    return false;
  }

  items.insert(items.begin(), musicvideos.cbegin(), musicvideos.cend());

  return true;
}

std::set<Field> CMusicVideoImportHandler::IgnoreDifferences() const
{
  return {FieldActor,         FieldCountry,
          FieldEpisodeNumber, FieldEpisodeNumberSpecialSort,
          FieldMPAA,          FieldOriginalTitle,
          FieldPlotOutline,   FieldProductionCode,
          FieldSeason,        FieldSeasonSpecialSort,
          FieldSet,           FieldSortTitle,
          FieldTagline,       FieldTop250,
          FieldTrackNumber,   FieldTrailer,
          FieldTvShowStatus,  FieldTvShowTitle,
          FieldWriter};
}

bool CMusicVideoImportHandler::AddImportedItem(CVideoDatabase& videodb,
                                               const CMediaImport& import,
                                               CFileItem* item)
{
  if (item == nullptr)
    return false;

  PrepareItem(videodb, import, item);

  item->GetVideoInfoTag()->m_iDbId =
      videodb.SetDetailsForMusicVideo(item->GetPath(), *(item->GetVideoInfoTag()), item->GetArt());
  if (item->GetVideoInfoTag()->m_iDbId <= 0)
  {
    GetLogger()->error("failed to set details for added music video \"{}\" imported from {}",
                       item->GetLabel(), import);
    return false;
  }

  SetDetailsForFile(videodb, item, false);
  return SetImportForItem(videodb, item, import, videodb.GetFileId(item->GetPath()));
}
