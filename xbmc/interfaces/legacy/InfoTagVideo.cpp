/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "InfoTagVideo.h"

#include "AddonUtils.h"
#include "ServiceBroker.h"
#include "interfaces/legacy/DictUtils.h"
#include "interfaces/legacy/Exception.h"
#include "settings/AdvancedSettings.h"
#include "settings/SettingsComponent.h"
#include "utils/StringUtils.h"

using namespace XBMCAddonUtils;

namespace XBMCAddon
{
  namespace xbmc
  {
    Actor::Actor(const String& name /* = emptyString */,
                 const String& role /* = emptyString */,
                 int order /* = -1 */,
                 const String& thumbnail /* = emptyString */)
      : m_name(name), m_role(role), m_order(order), m_thumbnail(thumbnail)
    {
      if (m_name.empty())
        throw WrongTypeException("Actor: name property must not be empty");
    }

    SActorInfo Actor::ToActorInfo() const
    {
      SActorInfo actorInfo;
      actorInfo.strName = m_name;
      actorInfo.strRole = m_role;
      actorInfo.order = m_order;
      actorInfo.thumbUrl = CScraperUrl(m_thumbnail);
      if (!actorInfo.thumbUrl.GetFirstThumbUrl().empty())
        actorInfo.thumb = CScraperUrl::GetThumbUrl(actorInfo.thumbUrl.GetFirstUrlByType());

      return actorInfo;
    }

    VideoStreamDetail::VideoStreamDetail(int width /* = 0 */,
                                         int height /* = 0 */,
                                         float aspect /* = 0.0f */,
                                         int duration /* = 0 */,
                                         const String& codec /* = emptyString */,
                                         const String& stereoMode /* = emptyString */,
                                         const String& language /* = emptyString */)
      : m_width(width),
        m_height(height),
        m_aspect(aspect),
        m_duration(duration),
        m_codec(codec),
        m_stereoMode(stereoMode),
        m_language(language)
    {
    }

    CStreamDetailVideo* VideoStreamDetail::ToStreamDetailVideo() const
    {
      auto streamDetail = new CStreamDetailVideo();
      streamDetail->m_iWidth = m_width;
      streamDetail->m_iHeight = m_height;
      streamDetail->m_fAspect = m_aspect;
      streamDetail->m_iDuration = m_duration;
      streamDetail->m_strCodec = m_codec;
      streamDetail->m_strStereoMode = m_stereoMode;
      streamDetail->m_strLanguage = m_language;

      return streamDetail;
    }

    AudioStreamDetail::AudioStreamDetail(int channels /* = -1 */,
                                         const String& codec /* = emptyString */,
                                         const String& language /* = emptyString */)
      : m_channels(channels), m_codec(codec), m_language(language)
    {
    }

    CStreamDetailAudio* AudioStreamDetail::ToStreamDetailAudio() const
    {
      auto streamDetail = new CStreamDetailAudio();
      streamDetail->m_iChannels = m_channels;
      streamDetail->m_strCodec = m_codec;
      streamDetail->m_strLanguage = m_language;

      return streamDetail;
    }

    SubtitleStreamDetail::SubtitleStreamDetail(const String& language /* = emptyString */)
      : m_language(language)
    {
    }

    CStreamDetailSubtitle* SubtitleStreamDetail::ToStreamDetailSubtitle() const
    {
      auto streamDetail = new CStreamDetailSubtitle();
      streamDetail->m_strLanguage = m_language;

      return streamDetail;
    }

    Fanart::Fanart(const String& image /* = emptyString */,
                   const String& preview /* = emptyString */,
                   const String& colors /* = emptyString */)
      : m_image(image), m_preview(preview), m_colors(colors)
    {
      if (m_image.empty())
        throw WrongTypeException("Fanart: image property must not be empty");
    }

    InfoTagVideo::InfoTagVideo(bool offscreen /* = false */)
      : infoTag(new CVideoInfoTag), offscreen(offscreen), owned(true)
    {
    }

    InfoTagVideo::InfoTagVideo(const CVideoInfoTag* tag)
      : infoTag(new CVideoInfoTag(*tag)), offscreen(true), owned(true)
    {
    }

    InfoTagVideo::InfoTagVideo(CVideoInfoTag* tag, bool offscreen /* = false */)
      : infoTag(tag), offscreen(offscreen), owned(false)
    {
    }

    InfoTagVideo::~InfoTagVideo()
    {
      if (owned)
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
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
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
      return StringUtils::Format("%i", infoTag->GetRating().votes);
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
      return infoTag->GetRating(type).rating;
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
      return infoTag->m_lastPlayed.GetAsLocalizedDateTime();
    }

    String InfoTagVideo::getOriginalTitle()
    {
      return infoTag->m_strOriginalTitle;
    }

    String InfoTagVideo::getPremiered()
    {
      return infoTag->GetPremiered().GetAsLocalizedDate();
    }

    String InfoTagVideo::getFirstAired()
    {
      return infoTag->m_firstAired.GetAsLocalizedDate();
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

    void InfoTagVideo::setUniqueID(const String& uniqueID,
                                   const String& type /* = "" */,
                                   bool isDefault /* = false */)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setUniqueIDRaw(infoTag, uniqueID, type, isDefault);
    }

    void InfoTagVideo::setUniqueIDs(const std::map<String, String>& uniqueIDs,
                                    const String& defaultUniqueID /* = "" */)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setUniqueIDsRaw(infoTag, uniqueIDs, defaultUniqueID);
    }

    void InfoTagVideo::setDbId(int dbId)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setDbIdRaw(infoTag, dbId);
    }

    void InfoTagVideo::setYear(int year)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setYearRaw(infoTag, year);
    }

    void InfoTagVideo::setEpisode(int episode)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setEpisodeRaw(infoTag, episode);
    }

    void InfoTagVideo::setSeason(int season)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setSeasonRaw(infoTag, season);
    }

    void InfoTagVideo::setSortEpisode(int sortEpisode)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setSortEpisodeRaw(infoTag, sortEpisode);
    }

    void InfoTagVideo::setSortSeason(int sortSeason)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setSortSeasonRaw(infoTag, sortSeason);
    }

    void InfoTagVideo::setEpisodeGuide(const String& episodeGuide)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setEpisodeGuideRaw(infoTag, episodeGuide);
    }

    void InfoTagVideo::setTop250(int top250)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setTop250Raw(infoTag, top250);
    }

    void InfoTagVideo::setSetId(int setId)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setSetIdRaw(infoTag, setId);
    }

    void InfoTagVideo::setTrackNumber(int trackNumber)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setTrackNumberRaw(infoTag, trackNumber);
    }

    void InfoTagVideo::setRating(float rating,
                                 int votes /* = 0 */,
                                 const String& type /* = "" */,
                                 bool isDefault /* = false */)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setRatingRaw(infoTag, rating, votes, type, isDefault);
    }

    void InfoTagVideo::setRatings(const std::map<String, Tuple<float, int>>& ratings,
                                  const String& defaultRating /* = "" */)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setRatingsRaw(infoTag, ratings, defaultRating);
    }

    void InfoTagVideo::setUserRating(int userRating)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setUserRatingRaw(infoTag, userRating);
    }

    void InfoTagVideo::setPlaycount(int playcount)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setPlaycountRaw(infoTag, playcount);
    }

    void InfoTagVideo::setMpaa(const String& mpaa)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setMpaaRaw(infoTag, mpaa);
    }

    void InfoTagVideo::setPlot(const String& plot)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setPlotRaw(infoTag, plot);
    }

    void InfoTagVideo::setPlotOutline(const String& plotOutline)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setPlotOutlineRaw(infoTag, plotOutline);
    }

    void InfoTagVideo::setTitle(const String& title)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setTitleRaw(infoTag, title);
    }

    void InfoTagVideo::setOriginalTitle(const String& originalTitle)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setOriginalTitleRaw(infoTag, originalTitle);
    }

    void InfoTagVideo::setSortTitle(const String& sortTitle)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setSortTitleRaw(infoTag, sortTitle);
    }

    void InfoTagVideo::setTagLine(const String& tagLine)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setTagLineRaw(infoTag, tagLine);
    }

    void InfoTagVideo::setTvShowTitle(const String& tvshowTitle)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setTvShowTitleRaw(infoTag, tvshowTitle);
    }

    void InfoTagVideo::setTvShowStatus(const String& tvshowStatus)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setTvShowStatusRaw(infoTag, tvshowStatus);
    }

    void InfoTagVideo::setGenre(std::vector<String> genre)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setGenreRaw(infoTag, std::move(genre));
    }

    void InfoTagVideo::setCountries(std::vector<String> countries)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setCountriesRaw(infoTag, std::move(countries));
    }

    void InfoTagVideo::setDirectors(std::vector<String> directors)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setDirectorsRaw(infoTag, std::move(directors));
    }

    void InfoTagVideo::setStudios(std::vector<String> studios)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setStudiosRaw(infoTag, std::move(studios));
    }

    void InfoTagVideo::setWriters(std::vector<String> writers)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setWritersRaw(infoTag, std::move(writers));
    }

    void InfoTagVideo::setDuration(int duration)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setDurationRaw(infoTag, duration);
    }

    void InfoTagVideo::setPremiered(const String& premiered)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setPremieredRaw(infoTag, premiered);
    }

    void InfoTagVideo::setSet(const String& set)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setSetRaw(infoTag, set);
    }

    void InfoTagVideo::setSetOverview(const String& setOverview)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setSetOverviewRaw(infoTag, setOverview);
    }

    void InfoTagVideo::setTags(std::vector<String> tags)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setTagsRaw(infoTag, tags);
    }

    void InfoTagVideo::setProductionCode(const String& productionCode)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setProductionCodeRaw(infoTag, productionCode);
    }

    void InfoTagVideo::setFirstAired(const String& firstAired)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setFirstAiredRaw(infoTag, firstAired);
    }

    void InfoTagVideo::setLastPlayed(const String& lastPlayed)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setLastPlayedRaw(infoTag, lastPlayed);
    }

    void InfoTagVideo::setAlbum(const String& album)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setAlbumRaw(infoTag, album);
    }

    void InfoTagVideo::setVotes(int votes)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setVotesRaw(infoTag, votes);
    }

    void InfoTagVideo::setTrailer(const String& trailer)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setTrailerRaw(infoTag, trailer);
    }

    void InfoTagVideo::setPath(const String& path)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setPathRaw(infoTag, path);
    }

    void InfoTagVideo::setFilenameAndPath(const String& filenameAndPath)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setFilenameAndPathRaw(infoTag, filenameAndPath);
    }

    void InfoTagVideo::setIMDBNumber(const String& imdbNumber)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setIMDBNumberRaw(infoTag, imdbNumber);
    }

    void InfoTagVideo::setDateAdded(const String& dateAdded)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setDateAddedRaw(infoTag, dateAdded);
    }

    void InfoTagVideo::setMediaType(const String& mediaType)
    {
      setMediaTypeRaw(infoTag, mediaType);
    }

    void InfoTagVideo::setShowLinks(std::vector<String> showLinks)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setShowLinksRaw(infoTag, std::move(showLinks));
    }

    void InfoTagVideo::setArtists(std::vector<String> artists)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setArtistsRaw(infoTag, std::move(artists));
    }

    void InfoTagVideo::setCast(std::vector<const Actor*> actors)
    {
      std::vector<SActorInfo> cast;
      cast.reserve(actors.size());
      for (const auto& actor : actors)
        cast.push_back(actor->ToActorInfo());

      {
        XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
        setCastRaw(infoTag, std::move(cast));
      }
    }

    void InfoTagVideo::setResumePoint(double time, double totalTime /* = 0.0 */)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setResumePointRaw(infoTag, time, totalTime);
    }

    void InfoTagVideo::addSeason(int number, std::string name /* = "" */)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      addSeasonRaw(infoTag, number, name);
    }

    void InfoTagVideo::addSeasons(std::vector<Tuple<int, std::string>> namedSeasons)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      addSeasonsRaw(infoTag, namedSeasons);
    }

    void InfoTagVideo::addVideoStream(const VideoStreamDetail* stream)
    {
      if (stream == nullptr)
        return;

      auto streamDetail = stream->ToStreamDetailVideo();
      {
        XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
        addStreamRaw(infoTag, streamDetail);
      }
    }

    void InfoTagVideo::addAudioStream(const AudioStreamDetail* stream)
    {
      if (stream == nullptr)
        return;

      auto streamDetail = stream->ToStreamDetailAudio();
      {
        XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
        addStreamRaw(infoTag, streamDetail);
      }
    }

    void InfoTagVideo::addSubtitleStream(const SubtitleStreamDetail* stream)
    {
      if (stream == nullptr)
        return;

      auto streamDetail = stream->ToStreamDetailSubtitle();
      {
        XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
        addStreamRaw(infoTag, streamDetail);
      }
    }

    void InfoTagVideo::setAvailableFanart(const std::vector<const Fanart*>& fanart)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      prepareAvailableFanartRaw(infoTag);
      for (const auto& image : fanart)
        addAvailableFanartRaw(infoTag, image->getImage(), image->getPreview(), image->getColors());
      finalizeAvailableFanartRaw(infoTag);
    }

    void InfoTagVideo::addAvailableArtwork(const std::string& url,
      const std::string& art_type,
      const std::string& preview,
      const std::string& referrer,
      const std::string& cache,
      bool post,
      bool isgz,
      int season)
    {
      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      addAvailableArtworkRaw(infoTag, url, art_type, preview, referrer, cache, post, isgz, season);
    }

    void InfoTagVideo::setInfo(const CVariant& info)
    {
      if (info.empty())
        return;

      XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
      setInfoRaw(infoTag, info);
    }

    void InfoTagVideo::setDbIdRaw(CVideoInfoTag* infoTag, int dbId)
    {
      infoTag->m_iDbId = dbId;
    }

    void InfoTagVideo::setUniqueIDRaw(CVideoInfoTag* infoTag,
                                      const String& uniqueID,
                                      const String& type /* = "" */,
                                      bool isDefault /* = false */)
    {
      infoTag->SetUniqueID(uniqueID, type, isDefault);
    }

    void InfoTagVideo::setUniqueIDsRaw(CVideoInfoTag* infoTag,
                                       std::map<String, String> uniqueIDs,
                                       const String& defaultUniqueID /* = "" */)
    {
      infoTag->SetUniqueIDs(uniqueIDs);
      auto defaultUniqueIDEntry = uniqueIDs.find(defaultUniqueID);
      if (defaultUniqueIDEntry != uniqueIDs.end())
        infoTag->SetUniqueID(defaultUniqueIDEntry->second, defaultUniqueIDEntry->first, true);
    }

    void InfoTagVideo::setYearRaw(CVideoInfoTag* infoTag, int year)
    {
      infoTag->SetYear(year);
    }

    void InfoTagVideo::setEpisodeRaw(CVideoInfoTag* infoTag, int episode)
    {
      infoTag->m_iEpisode = episode;
    }

    void InfoTagVideo::setSeasonRaw(CVideoInfoTag* infoTag, int season)
    {
      infoTag->m_iSeason = season;
    }

    void InfoTagVideo::setSortEpisodeRaw(CVideoInfoTag* infoTag, int sortEpisode)
    {
      infoTag->m_iSpecialSortEpisode = sortEpisode;
    }

    void InfoTagVideo::setSortSeasonRaw(CVideoInfoTag* infoTag, int sortSeason)
    {
      infoTag->m_iSpecialSortSeason = sortSeason;
    }

    void InfoTagVideo::setEpisodeGuideRaw(CVideoInfoTag* infoTag, const String& episodeGuide)
    {
      infoTag->SetEpisodeGuide(episodeGuide);
    }

    void InfoTagVideo::setTop250Raw(CVideoInfoTag* infoTag, int top250)
    {
      infoTag->m_iTop250 = top250;
    }

    void InfoTagVideo::setSetIdRaw(CVideoInfoTag* infoTag, int setId)
    {
      infoTag->m_set.id = setId;
    }

    void InfoTagVideo::setTrackNumberRaw(CVideoInfoTag* infoTag, int trackNumber)
    {
      infoTag->m_iTrack = trackNumber;
    }

    void InfoTagVideo::setRatingRaw(CVideoInfoTag* infoTag,
                                    float rating,
                                    int votes /* = 0 */,
                                    std::string type /* = "" */,
                                    bool isDefault /* = false */)
    {
      infoTag->SetRating(rating, votes, type, isDefault);
    }

    void InfoTagVideo::setRatingsRaw(CVideoInfoTag* infoTag,
                                     const std::map<String, Tuple<float, int>>& ratings,
                                     const String& defaultRating /* = "" */)
    {
      RatingMap ratingMap;
      for (const auto& rating : ratings)
        ratingMap.emplace(rating.first, CRating{rating.second.first(), rating.second.second()});

      infoTag->SetRatings(std::move(ratingMap), defaultRating);
    }

    void InfoTagVideo::setUserRatingRaw(CVideoInfoTag* infoTag, int userRating)
    {
      infoTag->m_iUserRating = userRating;
    }

    void InfoTagVideo::setPlaycountRaw(CVideoInfoTag* infoTag, int playcount)
    {
      infoTag->SetPlayCount(playcount);
    }

    void InfoTagVideo::setMpaaRaw(CVideoInfoTag* infoTag, const String& mpaa)
    {
      infoTag->SetMPAARating(mpaa);
    }

    void InfoTagVideo::setPlotRaw(CVideoInfoTag* infoTag, const String& plot)
    {
      infoTag->SetPlot(plot);
    }

    void InfoTagVideo::setPlotOutlineRaw(CVideoInfoTag* infoTag, const String& plotOutline)
    {
      infoTag->SetPlotOutline(plotOutline);
    }

    void InfoTagVideo::setTitleRaw(CVideoInfoTag* infoTag, const String& title)
    {
      infoTag->SetTitle(title);
    }

    void InfoTagVideo::setOriginalTitleRaw(CVideoInfoTag* infoTag, const String& originalTitle)
    {
      infoTag->SetOriginalTitle(originalTitle);
    }

    void InfoTagVideo::setSortTitleRaw(CVideoInfoTag* infoTag, const String& sortTitle)
    {
      infoTag->SetSortTitle(sortTitle);
    }

    void InfoTagVideo::setTagLineRaw(CVideoInfoTag* infoTag, const String& tagLine)
    {
      infoTag->SetTagLine(tagLine);
    }

    void InfoTagVideo::setTvShowTitleRaw(CVideoInfoTag* infoTag, const String& tvshowTitle)
    {
      infoTag->SetShowTitle(tvshowTitle);
    }

    void InfoTagVideo::setTvShowStatusRaw(CVideoInfoTag* infoTag, const String& tvshowStatus)
    {
      infoTag->SetStatus(tvshowStatus);
    }

    void InfoTagVideo::setGenreRaw(CVideoInfoTag* infoTag, std::vector<String> genre)
    {
      infoTag->SetGenre(std::move(genre));
    }

    void InfoTagVideo::setCountriesRaw(CVideoInfoTag* infoTag, std::vector<String> countries)
    {
      infoTag->SetCountry(std::move(countries));
    }

    void InfoTagVideo::setDirectorsRaw(CVideoInfoTag* infoTag, std::vector<String> directors)
    {
      infoTag->SetDirector(std::move(directors));
    }

    void InfoTagVideo::setStudiosRaw(CVideoInfoTag* infoTag, std::vector<String> studios)
    {
      infoTag->SetStudio(std::move(studios));
    }

    void InfoTagVideo::setWritersRaw(CVideoInfoTag* infoTag, std::vector<String> writers)
    {
      infoTag->SetWritingCredits(std::move(writers));
    }

    void InfoTagVideo::setDurationRaw(CVideoInfoTag* infoTag, int duration)
    {
      infoTag->SetDuration(duration);
    }

    void InfoTagVideo::setPremieredRaw(CVideoInfoTag* infoTag, const String& premiered)
    {
      CDateTime premieredDate;
      premieredDate.SetFromDateString(premiered);
      infoTag->SetPremiered(premieredDate);
    }

    void InfoTagVideo::setSetRaw(CVideoInfoTag* infoTag, const String& set)
    {
      infoTag->SetSet(set);
    }

    void InfoTagVideo::setSetOverviewRaw(CVideoInfoTag* infoTag, const String& setOverview)
    {
      infoTag->SetSetOverview(setOverview);
    }

    void InfoTagVideo::setTagsRaw(CVideoInfoTag* infoTag, std::vector<String> tags)
    {
      infoTag->SetTags(tags);
    }

    void InfoTagVideo::setProductionCodeRaw(CVideoInfoTag* infoTag, const String& productionCode)
    {
      infoTag->SetProductionCode(productionCode);
    }

    void InfoTagVideo::setFirstAiredRaw(CVideoInfoTag* infoTag, const String& firstAired)
    {
      CDateTime firstAiredDate;
      firstAiredDate.SetFromDateString(firstAired);
      infoTag->m_firstAired = std::move(firstAiredDate);
    }

    void InfoTagVideo::setLastPlayedRaw(CVideoInfoTag* infoTag, const String& lastPlayed)
    {
      CDateTime lastPlayedDate;
      lastPlayedDate.SetFromDBDateTime(lastPlayed);
      infoTag->m_lastPlayed = std::move(lastPlayedDate);
    }

    void InfoTagVideo::setAlbumRaw(CVideoInfoTag* infoTag, const String& album)
    {
      infoTag->SetAlbum(album);
    }

    void InfoTagVideo::setVotesRaw(CVideoInfoTag* infoTag, int votes)
    {
      infoTag->SetVotes(votes);
    }

    void InfoTagVideo::setTrailerRaw(CVideoInfoTag* infoTag, const String& trailer)
    {
      infoTag->SetTrailer(trailer);
    }

    void InfoTagVideo::setPathRaw(CVideoInfoTag* infoTag, const String& path)
    {
      infoTag->SetPath(path);
    }

    void InfoTagVideo::setFilenameAndPathRaw(CVideoInfoTag* infoTag, const String& filenameAndPath)
    {
      infoTag->SetFileNameAndPath(filenameAndPath);
    }

    void InfoTagVideo::setIMDBNumberRaw(CVideoInfoTag* infoTag, const String& imdbNumber)
    {
      infoTag->SetUniqueID(imdbNumber);
    }

    void InfoTagVideo::setDateAddedRaw(CVideoInfoTag* infoTag, const String& dateAdded)
    {
      CDateTime dateAddedDate;
      dateAddedDate.SetFromDBDateTime(dateAdded);
      infoTag->m_dateAdded = std::move(dateAddedDate);
    }

    void InfoTagVideo::setMediaTypeRaw(CVideoInfoTag* infoTag, const String& mediaType)
    {
      if (CMediaTypes::IsValidMediaType(mediaType))
        infoTag->m_type = mediaType;
    }

    void InfoTagVideo::setShowLinksRaw(CVideoInfoTag* infoTag, std::vector<String> showLinks)
    {
      infoTag->SetShowLink(std::move(showLinks));
    }

    void InfoTagVideo::setArtistsRaw(CVideoInfoTag* infoTag, std::vector<String> artists)
    {
      infoTag->m_artist = std::move(artists);
    }

    void InfoTagVideo::setCastRaw(CVideoInfoTag* infoTag, std::vector<SActorInfo> cast)
    {
      infoTag->m_cast = std::move(cast);
    }

    void InfoTagVideo::setResumePointRaw(CVideoInfoTag* infoTag,
                                         double time,
                                         double totalTime /* = 0.0 */)
    {
      auto resumePoint = infoTag->GetResumePoint();
      resumePoint.timeInSeconds = time;
      if (totalTime > 0.0)
        resumePoint.totalTimeInSeconds = totalTime;
      infoTag->SetResumePoint(resumePoint);
    }

    void InfoTagVideo::addSeasonRaw(CVideoInfoTag* infoTag, int number, std::string name /* = "" */)
    {
      infoTag->m_namedSeasons[number] = std::move(name);
    }

    void InfoTagVideo::addSeasonsRaw(CVideoInfoTag* infoTag,
                                     std::vector<Tuple<int, std::string>> namedSeasons)
    {
      for (const auto& season : namedSeasons)
        addSeasonRaw(infoTag, season.first(), season.second());
    }

    void InfoTagVideo::addStreamRaw(CVideoInfoTag* infoTag, CStreamDetail* stream)
    {
      infoTag->m_streamDetails.AddStream(stream);
    }

    void InfoTagVideo::finalizeStreamsRaw(CVideoInfoTag* infoTag)
    {
      infoTag->m_streamDetails.DetermineBestStreams();
    }

    void InfoTagVideo::addAvailableArtworkRaw(CVideoInfoTag* infoTag,
                                              const std::string& url,
                                              const std::string& art_type,
                                              const std::string& preview,
                                              const std::string& referrer,
                                              const std::string& cache,
                                              bool post,
                                              bool isgz,
                                              int season)
    {
      infoTag->m_strPictureURL.AddParsedUrl(url, art_type, preview, referrer, cache, post, isgz,
                                            season);
    }

    void InfoTagVideo::prepareAvailableFanartRaw(CVideoInfoTag* infoTag)
    {
      infoTag->m_fanart.Clear();
    }

    void InfoTagVideo::addAvailableFanartRaw(CVideoInfoTag* infoTag,
                                             const std::string& image,
                                             const std::string& preview,
                                             const std::string& colors)
    {
      infoTag->m_fanart.AddFanart(image, preview, colors);
    }

    void InfoTagVideo::finalizeAvailableFanartRaw(CVideoInfoTag* infoTag)
    {
      infoTag->m_fanart.Pack();
    }

    void InfoTagVideo::setInfoRaw(CVideoInfoTag* infoTag, const CVariant& info)
    {
      typedef struct Rating
      {
        float rating = 0.0f;
        int votes = 0;
        std::string type;
        bool isDefault = false;
      } Rating;

      auto checkAndGetRatingProperty = [&](const std::string& key, const CVariant& value) {
        auto getRatingValue = [&](const std::string& key, const CVariant& value) {
          if (value.isDouble())
            return DictUtils::CheckAndGetFloatProperty(key, value);
          else if (value.isInteger())
            return static_cast<float>(DictUtils::CheckAndGetIntegerProperty(key, value));
          else
            throw WrongTypeException("%s expects a float or integer", key.c_str());
        };

        Rating rating;
        if (value.isDouble() || value.isInteger())
          rating.rating = getRatingValue(key, value);
        else if (value.isObject())
        {
          DictUtils::ProcessObject(
              key, value,
              {
                  std::make_pair(
                      "rating",
                      DictUtils::ObjectProperty{[&](const std::string& key, const CVariant& value) {
                                                  rating.rating = getRatingValue(key, value);
                                                },
                                                true}),
                  std::make_pair("votes",
                                 DictUtils::ObjectProperty{[&rating](const std::string& key,
                                                                     const CVariant& value) {
                                   rating.votes = DictUtils::CheckAndGetIntegerProperty(key, value);
                                 }}),
                  std::make_pair("type",
                                 DictUtils::ObjectProperty{[&rating](const std::string& key,
                                                                     const CVariant& value) {
                                   rating.type = DictUtils::CheckAndGetStringProperty(key, value);
                                 }}),
                  std::make_pair(
                      "default", DictUtils::ObjectProperty{[&rating](const std::string& key,
                                                                     const CVariant& value) {
                        rating.isDefault = DictUtils::CheckAndGetBoolProperty(key, value);
                      }}),
              });
        }
        else
          throw WrongTypeException("%s expects a float, integer or dict", key.c_str());

        return rating;
      };

      typedef struct UniqueID
      {
        std::string uniqueID;
        bool isDefault = false;
      } UniqueID;

      auto checkAndGetUniqueIDProperty = [&](const std::string& key, const CVariant& value) {
        UniqueID uniqueID;
        if (value.isString())
          uniqueID.uniqueID = DictUtils::CheckAndGetStringProperty(key, value);
        else if (value.isObject())
        {
          DictUtils::ProcessObject(
              key, value,
              {
                  std::make_pair("uniqueid",
                                 DictUtils::ObjectProperty{
                                     [&uniqueID](const std::string& key, const CVariant& value) {
                                       uniqueID.uniqueID =
                                           DictUtils::CheckAndGetStringProperty(key, value);
                                     },
                                     true}),
                  std::make_pair(
                      "default", DictUtils::ObjectProperty{[&uniqueID](const std::string& key,
                                                                       const CVariant& value) {
                        uniqueID.isDefault = DictUtils::CheckAndGetBoolProperty(key, value);
                      }}),
              });
        }
        else
          throw WrongTypeException("%s expects a string or dict", key.c_str());

        return uniqueID;
      };

      for (const auto& prop : info.asMap())
      {
        const auto key = StringUtils::ToLower(prop.first);
        const auto value = prop.second;

        if (key == "dbid")
          setDbIdRaw(infoTag, DictUtils::CheckAndGetIntegerProperty(key, value));
        else if (key == "year")
          setYearRaw(infoTag, DictUtils::CheckAndGetIntegerProperty(key, value));
        else if (key == "episode")
          setEpisodeRaw(infoTag, DictUtils::CheckAndGetIntegerProperty(key, value));
        else if (key == "season")
          setSeasonRaw(infoTag, DictUtils::CheckAndGetIntegerProperty(key, value));
        else if (key == "seasons")
        {
          if (!value.isArray())
            throw WrongTypeException("%s: %s expects a list", __FUNCTION__, key.c_str());

          for (const auto& val : value.asArray())
          {
            int seasonNumber = -1;
            std::string seasonName;

            if (val.isInteger())
              seasonNumber = DictUtils::CheckAndGetIntegerProperty(key, val);
            else if (val.isObject())
            {
              DictUtils::ProcessObject(
                  key, val,
                  {
                      std::make_pair(
                          "number",
                          DictUtils::ObjectProperty{
                              [&seasonNumber](const std::string& key, const CVariant& value) {
                                seasonNumber = DictUtils::CheckAndGetIntegerProperty(key, value);
                              },
                              true}),
                      std::make_pair(
                          "name", DictUtils::ObjectProperty{[&seasonName](const std::string& key,
                                                                          const CVariant& value) {
                            seasonName = DictUtils::CheckAndGetStringProperty(key, value);
                          }}),
                  });
            }
            else
            {
              throw WrongTypeException("%s: %s expects a list of integers or objects", __FUNCTION__,
                                       key.c_str());
            }

            addSeasonRaw(infoTag, seasonNumber, seasonName);
          }
        }
        else if (key == "sortepisode")
          setSortEpisodeRaw(infoTag, DictUtils::CheckAndGetIntegerProperty(key, value));
        else if (key == "sortseason")
          setSortSeasonRaw(infoTag, DictUtils::CheckAndGetIntegerProperty(key, value));
        else if (key == "episodeguide")
          setEpisodeGuideRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "showlink")
          setShowLinksRaw(infoTag, DictUtils::CheckAndGetStringArrayProperty(key, value));
        else if (key == "top250")
          setTop250Raw(infoTag, DictUtils::CheckAndGetIntegerProperty(key, value));
        else if (key == "setid")
          setSetIdRaw(infoTag, DictUtils::CheckAndGetIntegerProperty(key, value));
        else if (key == "tracknumber")
          setTrackNumberRaw(infoTag, DictUtils::CheckAndGetIntegerProperty(key, value));
        else if (key == "rating")
        {
          const auto rating = checkAndGetRatingProperty(key, value);
          setRatingRaw(infoTag, rating.rating, rating.votes, rating.type, rating.isDefault);
        }
        else if (key == "ratings")
        {
          if (value.isArray())
          {
            for (const auto& val : value.asArray())
            {
              const auto rating = checkAndGetRatingProperty(key, val);
              setRatingRaw(infoTag, rating.rating, rating.votes, rating.type, rating.isDefault);
            }
          }
          else if (value.isObject())
          {
            for (const auto& val : value.asMap())
            {
              auto rating = checkAndGetRatingProperty(key, val.second);
              if (rating.type.empty())
                rating.type = val.first;

              setRatingRaw(infoTag, rating.rating, rating.votes, rating.type, rating.isDefault);
            }
          }
          else
            throw WrongTypeException("%s: %s expects a list or dict", __FUNCTION__, key.c_str());
        }
        else if (key == "userrating")
          setUserRatingRaw(infoTag, DictUtils::CheckAndGetIntegerProperty(key, value));
        else if (key == "playcount")
          setPlaycountRaw(infoTag, DictUtils::CheckAndGetIntegerProperty(key, value));
        else if (key == "cast" || key == "castandrole")
        {
          if (!value.isArray())
            throw WrongTypeException("%s: %s expects a list", __FUNCTION__, key.c_str());

          std::vector<SActorInfo> cast;
          cast.reserve(value.size());
          for (const auto& val : value.asArray())
          {
            SActorInfo info;
            // castEntry can be a string meaning it's the actor
            // or it can be a tuple meaning it's the actor and the role.
            if (val.isString())
              info.strName = val.asString();
            else if (val.isObject())
            {
              DictUtils::ProcessObject(
                  key, val,
                  {
                      std::make_pair("name",
                                     DictUtils::ObjectProperty{
                                         [&info](const std::string& key, const CVariant& value) {
                                           info.strName =
                                               DictUtils::CheckAndGetStringProperty(key, value);
                                         },
                                         true}),
                      std::make_pair(
                          "role", DictUtils::ObjectProperty{[&info](const std::string& key,
                                                                    const CVariant& value) {
                            info.strRole = DictUtils::CheckAndGetStringProperty(key, value);
                          }}),
                      std::make_pair(
                          "order", DictUtils::ObjectProperty{[&info](const std::string& key,
                                                                     const CVariant& value) {
                            info.order = DictUtils::CheckAndGetIntegerProperty(key, value);
                          }}),
                      std::make_pair(
                          "thumbnail", DictUtils::ObjectProperty{[&info](const std::string& key,
                                                                         const CVariant& value) {
                            info.thumbUrl =
                                CScraperUrl(DictUtils::CheckAndGetStringProperty(key, value));
                            if (!info.thumbUrl.GetFirstThumbUrl().empty())
                              info.thumb =
                                  CScraperUrl::GetThumbUrl(info.thumbUrl.GetFirstUrlByType());
                          }}),
                  });
            }
            else
            {
              throw WrongTypeException("%s: %s expects a list of strings or objects", __FUNCTION__,
                                       key.c_str());
            }

            cast.push_back(std::move(info));
          }
          setCastRaw(infoTag, std::move(cast));
        }
        else if (key == "artist")
          setArtistsRaw(infoTag, DictUtils::CheckAndGetStringArrayProperty(key, value));
        else if (key == "genre")
          setGenreRaw(infoTag, DictUtils::CheckAndGetStringArrayProperty(key, value));
        else if (key == "country")
          setCountriesRaw(infoTag, DictUtils::CheckAndGetStringArrayProperty(key, value));
        else if (key == "director")
          setDirectorsRaw(infoTag, DictUtils::CheckAndGetStringArrayProperty(key, value));
        else if (key == "mpaa")
          setMpaaRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "plot")
          setPlotRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "plotoutline")
          setPlotOutlineRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "title")
          setTitleRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "originaltitle")
          setOriginalTitleRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "sorttitle")
          setSortTitleRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "duration")
          setDurationRaw(infoTag, DictUtils::CheckAndGetIntegerProperty(key, value));
        else if (key == "studio")
          setStudiosRaw(infoTag, DictUtils::CheckAndGetStringArrayProperty(key, value));
        else if (key == "tagline")
          setTagLineRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "writer" || key == "credits")
          setWritersRaw(infoTag, DictUtils::CheckAndGetStringArrayProperty(key, value));
        else if (key == "tvshowtitle")
          setTvShowTitleRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "premiered")
          setPremieredRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "status")
          setTvShowStatusRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "set")
          setSetRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "setoverview")
          setSetOverviewRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "tag")
          setTagsRaw(infoTag, DictUtils::CheckAndGetStringArrayProperty(key, value));
        else if (key == "imdbnumber")
          setIMDBNumberRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "code")
          setProductionCodeRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "aired")
          setFirstAiredRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "lastplayed")
          setLastPlayedRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "album")
          setAlbumRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "votes")
          setVotesRaw(infoTag, DictUtils::CheckAndGetIntegerProperty(key, value));
        else if (key == "trailer")
          setTrailerRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "path")
          setPathRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "filenameandpath")
          setFilenameAndPathRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "dateadded")
          setDateAddedRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "mediatype")
          setMediaTypeRaw(infoTag, DictUtils::CheckAndGetStringProperty(key, value));
        else if (key == "uniqueids")
        {
          if (value.isObject())
          {
            std::map<String, String> uniqueIDs;
            String defaultUniqueID;
            for (const auto& val : value.asMap())
            {
              auto uniqueID = checkAndGetUniqueIDProperty(key, val.second);
              uniqueIDs.emplace(val.first, uniqueID.uniqueID);

              if (uniqueID.isDefault && defaultUniqueID.empty())
                defaultUniqueID = val.first;
            }

            setUniqueIDsRaw(infoTag, std::move(uniqueIDs), defaultUniqueID);
          }
          else
            throw WrongTypeException("%s: %s expects a dict", __FUNCTION__, key.c_str());
        }
        else if (key == "resumepoint")
        {
          double time;
          double totalTime = infoTag->m_duration;
          if (value.isDouble() || value.asInteger())
            time = DictUtils::CheckAndGetDoubleProperty(key, value);
          else if (value.isObject())
          {
            DictUtils::ProcessObject(
                key, value,
                {
                    std::make_pair("resumetime",
                                   DictUtils::ObjectProperty{
                                       [&time](const std::string& key, const CVariant& value) {
                                         time = DictUtils::CheckAndGetDoubleProperty(key, value);
                                       },
                                       true}),
                    std::make_pair("totaltime",
                                   DictUtils::ObjectProperty{[&totalTime](const std::string& key,
                                                                          const CVariant& value) {
                                     totalTime = DictUtils::CheckAndGetDoubleProperty(key, value);
                                   }}),
                });
          }
          else
          {
            throw WrongTypeException("%s: %s expects a float, integer or dict", __FUNCTION__,
                                     key.c_str());
          }

          setResumePointRaw(infoTag, time, totalTime);
        }
        else if (key == "streaminfo")
        {
          if (value.isObject())
          {
            static const std::string keyVideo = "video";
            static const std::string keyAudio = "audio";
            static const std::string keySubtitle = "subtitle";

            CStreamDetails streamDetails;
            for (const auto& stream : value.asMap())
            {
              const auto streamType = StringUtils::ToLower(stream.first);

              if (streamType == keyVideo)
              {
                const auto& videoStreams = stream.second;
                if (videoStreams.isArray())
                {
                  for (const auto& videoStream : videoStreams.asArray())
                  {
                    if (!videoStream.isObject() || videoStream.empty())
                    {
                      throw WrongTypeException("%s: video %s expects a dict", __FUNCTION__,
                                               key.c_str());
                    }

                    CStreamDetailVideo* video = new CStreamDetailVideo();
                    DictUtils::ProcessObject(
                        streamType, videoStream,
                        {
                            std::make_pair(
                                "codec", DictUtils::ObjectProperty{[&video](const std::string& key,
                                                                            const CVariant& value) {
                                  video->m_strCodec =
                                      DictUtils::CheckAndGetStringProperty(key, value);
                                }}),
                            std::make_pair(
                                "aspect",
                                DictUtils::ObjectProperty{
                                    [&video](const std::string& key, const CVariant& value) {
                                      video->m_fAspect =
                                          DictUtils::CheckAndGetFloatProperty(key, value);
                                    }}),
                            std::make_pair(
                                "width", DictUtils::ObjectProperty{[&video](const std::string& key,
                                                                            const CVariant& value) {
                                  video->m_iWidth =
                                      DictUtils::CheckAndGetIntegerProperty(key, value);
                                }}),
                            std::make_pair(
                                "height",
                                DictUtils::ObjectProperty{
                                    [&video](const std::string& key, const CVariant& value) {
                                      video->m_iHeight =
                                          DictUtils::CheckAndGetIntegerProperty(key, value);
                                    }}),
                            std::make_pair(
                                "duration",
                                DictUtils::ObjectProperty{
                                    [&video](const std::string& key, const CVariant& value) {
                                      video->m_iDuration =
                                          DictUtils::CheckAndGetIntegerProperty(key, value);
                                    }}),
                            std::make_pair(
                                "stereomode",
                                DictUtils::ObjectProperty{
                                    [&video](const std::string& key, const CVariant& value) {
                                      video->m_strStereoMode =
                                          DictUtils::CheckAndGetStringProperty(key, value);
                                    }}),
                            std::make_pair(
                                "language",
                                DictUtils::ObjectProperty{
                                    [&video](const std::string& key, const CVariant& value) {
                                      video->m_strLanguage =
                                          DictUtils::CheckAndGetStringProperty(key, value);
                                    }}),
                        });
                    addStreamRaw(infoTag, video);
                  }
                }
                else
                  throw WrongTypeException("%s: %s expects a dict", __FUNCTION__, key.c_str());
              }
              else if (streamType == keyAudio)
              {
                const auto audioStreams = stream.second;
                if (audioStreams.isArray())
                {
                  for (const auto& audioStream : audioStreams.asArray())
                  {
                    if (!audioStream.isObject() || audioStream.empty())
                    {
                      throw WrongTypeException("%s: audio %s expects a dict", __FUNCTION__,
                                               key.c_str());
                    }

                    CStreamDetailAudio* audio = new CStreamDetailAudio();
                    DictUtils::ProcessObject(
                        streamType, audioStream,
                        {
                            std::make_pair(
                                "codec", DictUtils::ObjectProperty{[&audio](const std::string& key,
                                                                            const CVariant& value) {
                                  audio->m_strCodec =
                                      DictUtils::CheckAndGetStringProperty(key, value);
                                }}),
                            std::make_pair(
                                "channels",
                                DictUtils::ObjectProperty{
                                    [&audio](const std::string& key, const CVariant& value) {
                                      audio->m_iChannels =
                                          DictUtils::CheckAndGetIntegerProperty(key, value);
                                    }}),
                            std::make_pair(
                                "language",
                                DictUtils::ObjectProperty{
                                    [&audio](const std::string& key, const CVariant& value) {
                                      audio->m_strLanguage =
                                          DictUtils::CheckAndGetStringProperty(key, value);
                                    }}),
                        });
                    addStreamRaw(infoTag, audio);
                  }
                }
                else
                  throw WrongTypeException("%s: %s expects a dict", __FUNCTION__, key.c_str());
              }
              else if (streamType == keySubtitle)
              {
                const auto subtitleStreams = stream.second;
                if (subtitleStreams.isArray())
                {
                  for (const auto& subtitleStream : subtitleStreams.asArray())
                  {
                    if (!subtitleStream.isObject() || subtitleStream.empty())
                    {
                      throw WrongTypeException("%s: subtitle %s expects a dict", __FUNCTION__,
                                               key.c_str());
                    }

                    CStreamDetailSubtitle* subtitle = new CStreamDetailSubtitle;
                    DictUtils::ProcessObject(
                        streamType, subtitleStream,
                        {
                            std::make_pair(
                                "language",
                                DictUtils::ObjectProperty{
                                    [&subtitle](const std::string& key, const CVariant& value) {
                                      subtitle->m_strLanguage =
                                          DictUtils::CheckAndGetStringProperty(key, value);
                                    }}),
                        });
                    addStreamRaw(infoTag, subtitle);
                  }
                }
                else
                  throw WrongTypeException("%s: %s expects a dict", __FUNCTION__, key.c_str());
              }
            }
            finalizeStreamsRaw(infoTag);
          }
          else
            throw WrongTypeException("%s: %s expects a dict", __FUNCTION__, key.c_str());
        }
        else if (key == "availableart")
        {
          if (value.isArray())
          {
            const auto& artwork = value;
            for (const auto& art : artwork.asArray())
            {
              std::string url;
              std::string type;
              std::string preview;
              std::string referrer;
              std::string cache;
              bool post = false;
              bool isGz = false;
              int season = -1;
              DictUtils::ProcessObject(key, art, {
                std::make_pair("url", DictUtils::ObjectProperty{ [&url](const std::string& key, const CVariant& value) { url = DictUtils::CheckAndGetStringProperty(key, value); }, true }),
                std::make_pair("type", DictUtils::ObjectProperty{ [&type](const std::string& key, const CVariant& value) { type = DictUtils::CheckAndGetStringProperty(key, value); } }),
                std::make_pair("preview", DictUtils::ObjectProperty{ [&preview](const std::string& key, const CVariant& value) { preview = DictUtils::CheckAndGetStringProperty(key, value); } }),
                std::make_pair("referrer", DictUtils::ObjectProperty{ [&referrer](const std::string& key, const CVariant& value) { referrer = DictUtils::CheckAndGetStringProperty(key, value); } }),
                std::make_pair("cache", DictUtils::ObjectProperty{ [&cache](const std::string& key, const CVariant& value) { cache = DictUtils::CheckAndGetStringProperty(key, value); } }),
                std::make_pair("post", DictUtils::ObjectProperty{ [&post](const std::string& key, const CVariant& value) { post = DictUtils::CheckAndGetBoolProperty(key, value); } }),
                std::make_pair("isgz", DictUtils::ObjectProperty{ [&isGz](const std::string& key, const CVariant& value) { isGz = DictUtils::CheckAndGetBoolProperty(key, value); } }),
                std::make_pair("season", DictUtils::ObjectProperty{ [&season](const std::string& key, const CVariant& value) { season = DictUtils::CheckAndGetIntegerProperty(key, value); } }),
                });

              addAvailableArtworkRaw(infoTag, url, type, preview, referrer, cache, post, isGz, season);
            }
          }
          else
            throw WrongTypeException("ListItem.setInfo: \"%s\" property must be a list", key);
        }
        else if (key == "availablefanart")
        {
          if (value.isArray())
          {
            prepareAvailableFanartRaw(infoTag);
            const auto& availableFanart = value;
            for (const auto& fanart : availableFanart.asArray())
            {
              std::string image;
              std::string preview;
              std::string colors;
              DictUtils::ProcessObject(key, fanart, {
                std::make_pair("image", DictUtils::ObjectProperty{ [&image](const std::string& key, const CVariant& value) { image = DictUtils::CheckAndGetStringProperty(key, value); }, true }),
                std::make_pair("preview", DictUtils::ObjectProperty{ [&preview](const std::string& key, const CVariant& value) { preview = DictUtils::CheckAndGetStringProperty(key, value); } }),
                std::make_pair("colors", DictUtils::ObjectProperty{ [&colors](const std::string& key, const CVariant& value) { colors = DictUtils::CheckAndGetStringProperty(key, value); } }),
                });

              addAvailableFanartRaw(infoTag, image, preview, colors);
            }
            finalizeAvailableFanartRaw(infoTag);
          }
          else
            throw WrongTypeException("ListItem.setInfo: \"%s\" property must be a list", key);
        }
        else
          CLog::Log(LOGWARNING, "InfoTagVideo.setInfo: unknown property \"{}\"", key);
      }
    }
  }
}
