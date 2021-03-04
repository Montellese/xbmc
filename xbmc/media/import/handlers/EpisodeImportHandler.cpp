/*
 *  Copyright (C) 2013-2019 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "EpisodeImportHandler.h"

#include "FileItem.h"
#include "guilib/LocalizeStrings.h"
#include "media/import/IMediaImportHandlerManager.h"
#include "media/import/MediaImport.h"
#include "media/import/handlers/TvShowImportHandler.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/log.h"

#include <fmt/ostream.h>

using namespace fmt::literals;

std::string CEpisodeImportHandler::GetItemLabel(const CFileItem* item) const
{
  if (item != nullptr && item->HasVideoInfoTag() &&
      !item->GetVideoInfoTag()->m_strShowTitle.empty())
  {
    return StringUtils::Format(g_localizeStrings.Get(39565),
                               "tvshow"_a = item->GetVideoInfoTag()->m_strShowTitle,
                               "mediaitem"_a = item->GetVideoInfoTag()->m_strTitle);
  }

  return CVideoImportHandler::GetItemLabel(item);
}

bool CEpisodeImportHandler::StartSynchronisation(const CMediaImport& import)
{
  if (!CVideoImportHandler::StartSynchronisation(import))
    return false;

  if (m_importHandlerManager == nullptr)
    return false;

  MediaImportHandlerConstPtr tvshowHandlerCreator =
      m_importHandlerManager->GetImportHandler(MediaTypeTvShow);
  if (tvshowHandlerCreator == nullptr)
    return false;

  MediaImportHandlerPtr tvshowHandler(tvshowHandlerCreator->Create());
  auto tvshowImportHandler = std::dynamic_pointer_cast<CTvShowImportHandler>(tvshowHandler);
  if (tvshowImportHandler == nullptr)
    return false;


  // get all previously imported tvshows
  std::vector<CFileItemPtr> tvshows;
  if (!tvshowImportHandler->GetLocalItems(m_db, import, tvshows))
    return false;

  // create a map of tvshows imported from the same source
  m_tvshows.clear();
  for (const auto tvshow : tvshows)
  {
    if (!tvshow->HasVideoInfoTag() || tvshow->GetVideoInfoTag()->m_strTitle.empty())
      continue;

    auto tvshowsIter = m_tvshows.find(tvshow->GetVideoInfoTag()->m_strTitle);
    if (tvshowsIter == m_tvshows.end())
    {
      TvShowsSet tvshowsSet;
      tvshowsSet.insert(tvshow);
      m_tvshows.insert(make_pair(tvshow->GetVideoInfoTag()->m_strTitle, tvshowsSet));
    }
    else
      tvshowsIter->second.insert(tvshow);
  }

  return true;
}

bool CEpisodeImportHandler::UpdateImportedItem(const CMediaImport& import, CFileItem* item)
{
  if (item == nullptr || !item->HasVideoInfoTag() || item->GetVideoInfoTag()->m_iDbId <= 0)
    return false;

  const auto episode = item->GetVideoInfoTag();

  if (m_db.SetDetailsForEpisodeInTransaction(*episode, item->GetArt(), episode->m_iIdShow,
                                             episode->m_iDbId) <= 0)
  {
    GetLogger()->error(
        "failed to set details for added episode \"{}\" S{:02d}E{:02d} imported from {}",
        episode->m_strShowTitle, episode->m_iSeason, episode->m_iEpisode, import);
    return false;
  }

  if (import.Settings()->UpdatePlaybackMetadataFromSource())
    SetDetailsForFile(m_db, item, true);

  return true;
}

bool CEpisodeImportHandler::RemoveImportedItem(const CMediaImport& import, const CFileItem* item)
{
  if (item == nullptr || !item->HasVideoInfoTag())
    return false;

  m_db.DeleteEpisode(item->GetVideoInfoTag()->m_iDbId);
  RemoveFile(m_db, item);

  return true;
}

bool CEpisodeImportHandler::GetLocalItems(CVideoDatabase& videodb,
                                          const CMediaImport& import,
                                          std::vector<CFileItemPtr>& items)
{
  CVideoDbUrl videoUrl;
  videoUrl.FromString("videodb://tvshows/titles/-1/-1/");
  videoUrl.AddOption("imported", true);
  videoUrl.AddOption("source", import.GetSource().GetIdentifier());
  videoUrl.AddOption("import", import.GetMediaTypesAsString());

  CFileItemList episodes;
  if (!videodb.GetEpisodesByWhere(
          videoUrl.ToString(), CDatabase::Filter(), episodes, false, SortDescription(),
          import.Settings()->UpdateImportedMediaItems() ? VideoDbDetailsAll : VideoDbDetailsNone))
  {
    GetLogger()->error("failed to get previously imported episodes from {}", import);
    return false;
  }

  items.insert(items.begin(), episodes.cbegin(), episodes.cend());

  return true;
}

std::set<Field> CEpisodeImportHandler::IgnoreDifferences() const
{
  return {FieldActor,
          FieldAlbum,
          FieldArtist,
          FieldCountry,
          FieldGenre,
          FieldMPAA,
          FieldPlotOutline,
          FieldSet,
          FieldSortTitle,
          FieldStudio,
          FieldTag,
          FieldTagline,
          FieldTop250,
          FieldTrackNumber,
          FieldTrailer,
          FieldTvShowStatus,
          FieldTvShowTitle};
}

bool CEpisodeImportHandler::AddImportedItem(CVideoDatabase& videodb,
                                            const CMediaImport& import,
                                            CFileItem* item)
{
  if (item == nullptr)
    return false;

  PrepareItem(videodb, import, item);

  CVideoInfoTag* episode = item->GetVideoInfoTag();

  // try to find an existing tvshow that the episode belongs to
  episode->m_iIdShow = FindTvShowId(item);

  // if the tvshow doesn't exist, create a very basic version of it with the info we got from the
  // episode
  if (episode->m_iIdShow <= 0)
  {
    CVideoInfoTag tvshow;
    tvshow.m_basePath = episode->m_basePath;
    tvshow.m_cast = episode->m_cast;
    tvshow.m_country = episode->m_country;
    tvshow.m_director = episode->m_director;
    tvshow.m_genre = episode->m_genre;
    tvshow.SetYear(episode->GetYear());
    tvshow.m_parentPathID = episode->m_parentPathID;
    tvshow.m_premiered = episode->m_premiered;
    tvshow.m_strMPAARating = episode->m_strMPAARating;
    tvshow.m_strTitle = tvshow.m_strShowTitle = episode->m_strShowTitle;
    tvshow.m_studio = episode->m_studio;
    tvshow.m_type = MediaTypeTvShow;
    tvshow.m_writingCredits = episode->m_writingCredits;

    // try to find a proper path by going up in the path hierarchy twice
    // (once for season and once for tvshow)
    std::string showPath, testPath;
    showPath = tvshow.m_basePath;
    testPath = URIUtils::GetParentPath(episode->GetPath());
    if (testPath != tvshow.m_basePath)
    {
      showPath = testPath;
      testPath = URIUtils::GetParentPath(showPath);
      if (testPath != tvshow.m_basePath)
        showPath = testPath;
    }
    tvshow.m_strPath = showPath;

    // create an item for the tvshow
    CFileItemPtr tvshowItem(new CFileItem(tvshow));
    tvshowItem->SetPath(tvshow.m_strPath);
    tvshowItem->SetSource(item->GetSource());

    // try to use a tvshow-specific import handler
    bool tvshowImported = false;
    if (m_importHandlerManager != nullptr)
    {
      MediaImportHandlerConstPtr tvshowHandlerCreator =
          m_importHandlerManager->GetImportHandler(MediaTypeTvShow);
      if (tvshowHandlerCreator != nullptr)
      {
        MediaImportHandlerPtr tvshowHandler(tvshowHandlerCreator->Create());
        auto tvshowImportHandler = std::dynamic_pointer_cast<CTvShowImportHandler>(tvshowHandler);
        if (tvshowImportHandler != nullptr &&
            tvshowImportHandler->AddImportedItem(videodb, import, tvshowItem.get()))
        {
          tvshowImported = true;
          tvshow.m_iDbId = tvshowItem->GetVideoInfoTag()->m_iDbId;
        }
      }
    }

    // fall back to direct database access
    if (!tvshowImported)
    {
      std::vector<std::pair<std::string, std::string>> tvshowPaths;
      tvshowPaths.push_back(std::make_pair(tvshow.m_strPath, tvshow.m_basePath));
      tvshow.m_iDbId = tvshow.m_iIdShow = videodb.SetDetailsForTvShowInTransaction(
          tvshowPaths, tvshow, CGUIListItem::ArtMap(),
          std::map<int, std::map<std::string, std::string>>());
      if (tvshow.m_iDbId <= 0)
      {
        GetLogger()->error("failed to set details for added tvshow \"{}\" imported from {}",
                           tvshow.m_strTitle, import);
        return false;
      }
    }

    // store the tvshow's database ID in the episode
    episode->m_iIdShow = tvshow.m_iDbId;

    // add the tvshow to the tvshow map
    auto&& tvshowsIter = m_tvshows.find(tvshow.m_strTitle);
    if (tvshowsIter == m_tvshows.end())
    {
      TvShowsSet tvshowsSet;
      tvshowsSet.insert(tvshowItem);
      m_tvshows.insert(make_pair(tvshow.m_strTitle, tvshowsSet));
    }
    else
      tvshowsIter->second.insert(tvshowItem);
  }

  episode->m_iDbId =
      videodb.SetDetailsForEpisodeInTransaction(*episode, item->GetArt(), episode->m_iIdShow);
  if (episode->m_iDbId <= 0)
  {
    GetLogger()->error("failed to add episode \"{}\" S{:02d}E{:02d} imported from {}",
                       episode->m_strShowTitle, episode->m_iSeason, episode->m_iEpisode, import);
    return false;
  }

  SetDetailsForFile(videodb, item, false);
  return SetImportForItem(videodb, item, import, videodb.GetFileId(item->GetPath()));
}

int CEpisodeImportHandler::FindTvShowId(const CFileItem* episodeItem)
{
  if (episodeItem == nullptr || !episodeItem->HasVideoInfoTag())
    return -1;

  // no comparison possible without a title
  if (episodeItem->GetVideoInfoTag()->m_strShowTitle.empty())
    return -1;

  // check if there is a tvshow with a matching title
  const auto& tvshowsIter = m_tvshows.find(episodeItem->GetVideoInfoTag()->m_strShowTitle);
  if (tvshowsIter == m_tvshows.end() || tvshowsIter->second.size() <= 0)
    return -1;

  // if there is only one matching tvshow, we can go with that one
  if (tvshowsIter->second.size() == 1)
    return tvshowsIter->second.begin()->get()->GetVideoInfoTag()->m_iDbId;

  // use the path of the episode and tvshow to find the right tvshow
  for (const auto& it : tvshowsIter->second)
  {
    if (URIUtils::PathHasParent(episodeItem->GetVideoInfoTag()->GetPath(),
                                it->GetVideoInfoTag()->GetPath()))
      return it->GetVideoInfoTag()->m_iDbId;
  }

  return -1;
}
