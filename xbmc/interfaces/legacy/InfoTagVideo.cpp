/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "InfoTagVideo.h"

#include "ServiceBroker.h"
#include "settings/AdvancedSettings.h"
#include "settings/SettingsComponent.h"
#include "utils/StringUtils.h"
#include "utils/log.h"

namespace XBMCAddon
{
  namespace xbmc
  {
    InfoTagVideo::InfoTagVideo()
    {
      infoTag = new CVideoInfoTag();
    }

    InfoTagVideo::InfoTagVideo(const CVideoInfoTag& tag)
    {
      infoTag = new CVideoInfoTag();
      *infoTag = tag;
    }

    InfoTagVideo::~InfoTagVideo()
    {
      delete infoTag;
    }

    int InfoTagVideo::getDbId()
    {
      return infoTag->m_iDbId;
    }

    String InfoTagVideo::getDirector()
    {
      return StringUtils::Join(infoTag->m_director, CServiceBroker::GetSettingsComponent()->GetAdvancedSettings()->m_videoItemSeparator);
    }

    String InfoTagVideo::getWritingCredits()
    {
      return StringUtils::Join(infoTag->m_writingCredits, CServiceBroker::GetSettingsComponent()->GetAdvancedSettings()->m_videoItemSeparator);
    }

    String InfoTagVideo::getGenre()
    {
      return StringUtils::Join(infoTag->m_genre, CServiceBroker::GetSettingsComponent()->GetAdvancedSettings()->m_videoItemSeparator);
    }

    String InfoTagVideo::getTagLine()
    {
      return infoTag->m_strTagLine;
    }

    String InfoTagVideo::getPlotOutline()
    {
      return infoTag->m_strPlotOutline;
    }

    String InfoTagVideo::getPlot()
    {
      return infoTag->m_strPlot;
    }

    String InfoTagVideo::getPictureURL()
    {
      infoTag->m_strPictureURL.Parse();
      return infoTag->m_strPictureURL.GetFirstThumbUrl();
    }

    String InfoTagVideo::getTVShowTitle()
    {
      return infoTag->m_strShowTitle;
    }

    String InfoTagVideo::getTitle()
    {
      return infoTag->m_strTitle;
    }

    String InfoTagVideo::getMediaType()
    {
      return infoTag->m_type;
    }

    String InfoTagVideo::getVotes()
    {
      CLog::Log(
          LOGWARNING,
          "InfoTagVideo.getVotes() is deprecated and might be removed in future Kodi versions. "
          "Please use InfoTagVideo.getVotesAsInt().");

      return std::to_string(getVotesAsInt());
    }

    int InfoTagVideo::getVotesAsInt(const String& type /* = "" */)
    {
      return infoTag->GetRating(type).votes;
    }

    String InfoTagVideo::getCast()
    {
      return infoTag->GetCast(true);
    }

    String InfoTagVideo::getFile()
    {
      return infoTag->m_strFile;
    }

    String InfoTagVideo::getPath()
    {
      return infoTag->m_strPath;
    }

    String InfoTagVideo::getFilenameAndPath()
    {
      return infoTag->m_strFileNameAndPath;
    }

    String InfoTagVideo::getIMDBNumber()
    {
      return infoTag->GetUniqueID();
    }

    int InfoTagVideo::getSeason()
    {
      return infoTag->m_iSeason;
    }

    int InfoTagVideo::getEpisode()
    {
      return infoTag->m_iEpisode;
    }

    int InfoTagVideo::getYear()
    {
      return infoTag->GetYear();
    }

    double InfoTagVideo::getRating(const String& type /* = "" */)
    {
      return static_cast<double>(infoTag->GetRating(type).rating);
    }

    int InfoTagVideo::getUserRating()
    {
      return infoTag->m_iUserRating;
    }

    int InfoTagVideo::getPlayCount()
    {
      return infoTag->GetPlayCount();
    }

    String InfoTagVideo::getLastPlayed()
    {
      CLog::Log(LOGWARNING, "InfoTagVideo.getLastPlayed() is deprecated and might be removed in "
                            "future Kodi versions. Please use InfoTagVideo.getLastPlayedAsW3C().");

      return infoTag->m_lastPlayed.GetAsLocalizedDateTime();
    }

    String InfoTagVideo::getLastPlayedAsW3C()
    {
      return infoTag->m_lastPlayed.GetAsW3CDateTime();
    }

    String InfoTagVideo::getOriginalTitle()
    {
      return infoTag->m_strOriginalTitle;
    }

    String InfoTagVideo::getPremiered()
    {
      CLog::Log(LOGWARNING, "InfoTagVideo.getPremiered() is deprecated and might be removed in "
                            "future Kodi versions. Please use InfoTagVideo.getPremieredAsW3C().");

      return infoTag->GetPremiered().GetAsLocalizedDate();
    }

    String InfoTagVideo::getPremieredAsW3C()
    {
      return infoTag->GetPremiered().GetAsW3CDate();
    }

    String InfoTagVideo::getFirstAired()
    {
      CLog::Log(LOGWARNING, "InfoTagVideo.getFirstAired() is deprecated and might be removed in "
                            "future Kodi versions. Please use InfoTagVideo.getFirstAiredAsW3C().");

      return infoTag->m_firstAired.GetAsLocalizedDate();
    }

    String InfoTagVideo::getFirstAiredAsW3C()
    {
      return infoTag->m_firstAired.GetAsW3CDate();
    }

    String InfoTagVideo::getTrailer()
    {
      return infoTag->m_strTrailer;
    }

    std::vector<std::string> InfoTagVideo::getArtist()
    {
      return infoTag->m_artist;
    }

    String InfoTagVideo::getAlbum()
    {
      return infoTag->m_strAlbum;
    }

    int InfoTagVideo::getTrack()
    {
      return infoTag->m_iTrack;
    }

    unsigned int InfoTagVideo::getDuration()
    {
      return infoTag->GetDuration();
    }

    double InfoTagVideo::getResumeTime()
    {
      return infoTag->GetResumePoint().timeInSeconds;
    }

    double InfoTagVideo::getResumeTimeTotal()
    {
      return infoTag->GetResumePoint().totalTimeInSeconds;
    }

    String InfoTagVideo::getUniqueID(const char* key)
    {
      return infoTag->GetUniqueID(key);
    }
  }
}
