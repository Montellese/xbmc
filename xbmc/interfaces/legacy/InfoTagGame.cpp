/*
 *  Copyright (C) 2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "InfoTagGame.h"

#include "AddonUtils.h"
#include "interfaces/legacy/DictUtils.h"
#include "interfaces/legacy/Exception.h"
#include "utils/StringUtils.h"

using namespace KODI::GAME;
using namespace XBMCAddonUtils;

namespace XBMCAddon
{
namespace xbmc
{

InfoTagGame::InfoTagGame(bool offscreen /* = false */)
  : infoTag(new CGameInfoTag), offscreen(offscreen), owned(true)
{
}

InfoTagGame::InfoTagGame(const CGameInfoTag* tag)
  : infoTag(new CGameInfoTag(*tag)), offscreen(true), owned(true)
{
}

InfoTagGame::InfoTagGame(CGameInfoTag* tag, bool offscreen /* = false */)
  : infoTag(tag), offscreen(offscreen), owned(false)
{
}

InfoTagGame::~InfoTagGame()
{
  if (owned)
    delete infoTag;
}

/* TODO(Montellese)
int InfoTagGame::getDbId()
{
  return infoTag->m_iDbId;
}

String InfoTagGame::getDirector()
{
  return StringUtils::Join(infoTag->m_director, CServiceBroker::GetSettingsComponent()->GetAdvancedSettings()->m_videoItemSeparator);
}

void InfoTagGame::setUniqueID(const String& uniqueID,
                                const String& type /* = "" *//*,
                                bool isDefault /* = false *//*)
{
  XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
  setUniqueIDRaw(infoTag, uniqueID, type, isDefault);
}

void InfoTagGame::setUniqueIDs(const std::map<String, String>& uniqueIDs,
                                const String& defaultUniqueID /* = "" *//*)
{
  XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
  setUniqueIDsRaw(infoTag, uniqueIDs, defaultUniqueID);
}
*/

void InfoTagGame::setInfo(const CVariant& info)
{
  if (info.empty())
    return;

  XBMCAddonUtils::GuiLock lock(languageHook, offscreen);
  setInfoRaw(infoTag, info);
}

/* TODO(Montellese)
void InfoTagGame::setDbIdRaw(CGameInfoTag* infoTag, int dbId)
{
  infoTag->m_iDbId = dbId;
}

void InfoTagGame::setUniqueIDRaw(CGameInfoTag* infoTag,
                                  const String& uniqueID,
                                  const String& type /* = "" *//*,
                                  bool isDefault /* = false *//*)
{
  infoTag->SetUniqueID(uniqueID, type, isDefault);
}

void InfoTagGame::setUniqueIDsRaw(CGameInfoTag* infoTag,
                                    std::map<String, String> uniqueIDs,
                                    const String& defaultUniqueID /* = "" *//*)
{
  infoTag->SetUniqueIDs(uniqueIDs);
  auto defaultUniqueIDEntry = uniqueIDs.find(defaultUniqueID);
  if (defaultUniqueIDEntry != uniqueIDs.end())
    infoTag->SetUniqueID(defaultUniqueIDEntry->second, defaultUniqueIDEntry->first, true);
}
*/

void InfoTagGame::setInfoRaw(CGameInfoTag* infoTag, const CVariant& info)
{
  /* TODO(Montellese)
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

        static const std::string keyLanguage = "language";

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
              static const std::string keyCodec = "codec";
              static const std::string keyChannels = "channels";

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
      CLog::Log(LOGWARNING, "InfoTagGame.setInfo: unknown property \"{}\"", key);
  }
  */
}

}
}
