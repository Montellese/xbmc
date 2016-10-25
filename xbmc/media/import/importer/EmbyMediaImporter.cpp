/*
 *      Copyright (C) 2016 Team XBMC
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

#include <algorithm>

#include "EmbyMediaImporter.h"
#include "URL.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "filesystem/CurlFile.h"
#include "guilib/LocalizeStrings.h"
#include "media/import/MediaImportManager.h"
#include "media/import/task/MediaImportRetrievalTask.h"
#include "media/import/task/MediaImportUpdateTask.h"
#include "network/Socket.h"
#include "utils/JSONVariantParser.h"
#include "utils/JSONVariantWriter.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/SystemInfo.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"
#include "video/VideoInfoTag.h"

static const std::map<MediaType, std::string> MediaTypeMapping = {
  { MediaTypeMovie, "Movie" },
  { MediaTypeTvShow, "Series" },
  { MediaTypeSeason, "Season" },
  { MediaTypeEpisode, "Episode" },
  { MediaTypeMusicVideo, "MusicVideo" },
};

static const uint32_t ItemRequestLimit = 100;

static const char* EmbyProtocol = "emby";
static const std::string EmbyApiKeyHeader = "X-MediaBrowser-Token";
static const std::string EmbyAuthorizationHeader = "X-Emby-Authorization";
static const std::string EmbyAcceptEncoding = "application/json";
static const std::string EmbyContentType = EmbyAcceptEncoding;

static const std::string UrlUsers = "Users";
static const std::string UrlUsersPublic = "Public";
static const std::string UrlItems = "Items";
static const std::string UrlVideos = "Videos";
static const std::string UrlPlayedItems = "PlayedItems";

static const std::string PropertyItemTotalRecordCount = "TotalRecordCount";
static const std::string PropertyItemItems = "Items";
static const std::string PropertyItemMediaType = "MediaType";
static const std::string PropertyItemType = "Type";
static const std::string PropertyItemId = "Id";
static const std::string PropertyItemIsFolder = "IsFolder";
static const std::string PropertyItemContainer = "Container";
static const std::string PropertyItemName = "Name";
static const std::string PropertyItemPremiereDate = "PremiereDate";
static const std::string PropertyItemProductionYear = "ProductionYear";
static const std::string PropertyItemPath = "Path";
static const std::string PropertyItemSortName = "SortName";
static const std::string PropertyItemOriginalTitle = "OriginalTitle";
static const std::string PropertyItemDateCreated = "DateCreated";
static const std::string PropertyItemCommunityRating = "CommunityRating";
static const std::string PropertyItemVoteCount = "VoteCount";
static const std::string PropertyItemOfficialRating = "OfficialRating";
static const std::string PropertyItemRunTimeTicks = "RunTimeTicks";
static const std::string PropertyItemUserData = "UserData";
static const std::string PropertyItemUserDataPlaybackPositionTicks = "PlaybackPositionTicks";
static const std::string PropertyItemUserDataPlayCount = "PlayCount";
static const std::string PropertyItemUserDataLastPlayedDate = "LastPlayedDate";
static const std::string PropertyItemUserDataPlayed = "Played";
static const std::string PropertyItemOverview = "Overview";
static const std::string PropertyItemShortOverview = "ShortOverview";
static const std::string PropertyItemTaglines = "Taglines";
static const std::string PropertyItemGenres = "Genres";
static const std::string PropertyItemStudios = "Studios";
static const std::string PropertyItemProductionLocations = "ProductionLocations";
static const std::string PropertyItemProviderIds = "ProviderIds";
static const std::string PropertyItemTags = "Tags";
static const std::string PropertyItemPeople = "People";
static const std::string PropertyItemRole = "Role";
static const std::string PropertyItemIndexNumber = "IndexNumber";
static const std::string PropertyItemParentIndexNumber = "ParentIndexNumber";
static const std::string PropertyItemSeriesName = "SeriesName";
static const std::string PropertyItemStatus = "Status";
static const std::string PropertyItemArtists = "Artists";
static const std::string PropertyItemAlbum = "Album";
static const std::string PropertyItemImageTags = "ImageTags";
static const std::string PropertyItemImageTagsPrimary = "Primary";
static const std::string PropertyItemImageTagsLogo = "Logo";
static const std::string PropertyItemBackdropImageTags = "BackdropImageTags";
static const std::string PropertyItemMediaStreams = "MediaStreams";
static const std::string PropertyItemMediaStreamType = "Type";
static const std::string PropertyItemMediaStreamCodec = "Codec";
static const std::string PropertyItemMediaStreamLanguage = "Language";
static const std::string PropertyItemMediaStreamHeight = "Height";
static const std::string PropertyItemMediaStreamWidth = "Width";
static const std::string PropertyItemMediaStreamChannels = "Channels";

static const std::string PropertyUserName = "Name";
static const std::string PropertyUserId = "Id";
static const std::string PropertyUserPolicy = "Policy";
static const std::string PropertyUserIsDisabled = "IsDisabled";

static const std::string SettingApiKey = "emby.apikey";
static const std::string SettingUser = "emby.user";
static const std::string SettingDeviceId = "emby.deviceid";

static void PrepareApiCall(const std::string& apiKey, const std::string& userId, const std::string& deviceId, XFILE::CCurlFile& curl)
{
  curl.SetRequestHeader("Accept", EmbyAcceptEncoding);

  // set the API key if possible
  if (!apiKey.empty())
    curl.SetRequestHeader(EmbyApiKeyHeader, apiKey);

  // set the Authorization header if possible
  if (!deviceId.empty())
    curl.SetRequestHeader(EmbyAuthorizationHeader,
      StringUtils::Format("MediaBrowser Client=\"%s\", Device=\"%s\", DeviceId=\"%s\", Version=\"%s\", UserId=\"%s\"",
        CSysInfo::GetAppName().c_str(), CSysInfo::GetDeviceName().c_str(), deviceId.c_str(), CSysInfo::GetVersionShort().c_str(), userId.c_str()));
}

static std::string BuildSourceIdentifier(const std::string& id)
{
  CURL embyUrl;
  embyUrl.SetProtocol(EmbyProtocol);
  embyUrl.SetHostName(id);

  return embyUrl.Get();
}

// one tick is 0.1 microseconds
static const uint64_t TicksToSecondsFactor = 10000000;

static uint64_t TicksToSeconds(uint64_t ticks)
{
  return ticks / TicksToSecondsFactor;
}

static uint64_t SecondsToTicks(uint64_t seconds)
{
  return seconds * TicksToSecondsFactor;
}

CEmbyMediaImporter::CEmbyMediaImporter()
  : IMediaImporter(CMediaImport())
{ }

CEmbyMediaImporter::~CEmbyMediaImporter()
{ }

CEmbyMediaImporter::CEmbyMediaImporter(const CMediaImport &import)
  : IMediaImporter(import)
  , m_url(import.GetSource().GetBasePath())
{
  GetServerId(import.GetPath(), m_serverId);

  // try to load the API key and user from the source's settings
  auto source = import.GetSource();
  if (LoadSourceSettings(source))
  {
    m_deviceId = source.Settings().GetString(SettingDeviceId);
    m_apiKey = source.Settings().GetString(SettingApiKey);
    m_userId = source.Settings().GetString(SettingUser);
  }
}

bool CEmbyMediaImporter::CanImport(const std::string& path) const
{
  std::string id;
  return GetServerId(path, id);
}

bool CEmbyMediaImporter::LoadSourceSettings(CMediaImportSource& source) const
{
  static const std::string SettingsDefinition = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
    "<settings>\n"
      "<section id=\"emby\" label=\"39400\">\n"
        "<category id=\"access\" label=\"39401\">\n"
          "<group id=\"1\">\n"
            "<setting id=\"" + SettingApiKey + "\" type=\"string\" label=\"39402\">\n"
              "<level>0</level>\n"
              "<default></default>\n"
              "<constraints>\n"
                "<allowempty>true</allowempty>\n"
              "</constraints>\n"
              "<control type=\"edit\" format=\"string\" />\n"
            "</setting>\n"
            "<setting id=\"" + SettingUser + "\" type=\"string\" label=\"39403\">\n"
              "<level>0</level>\n"
              "<default></default>\n"
              "<constraints>\n"
                "<allowempty>true</allowempty>\n"
              "</constraints>\n"
              "<dependencies>\n"
                "<dependency type=\"enable\" setting=\"emby.apikey\" operator=\"!is\"></dependency>\n"
              "</dependencies>\n"
              "<control type=\"list\" format=\"string\" />\n"
            "</setting>\n"
            "<setting id=\"" + SettingDeviceId + "\" type=\"string\">\n"
              "<visible>false</visible>\n"
              "<level>4</level>\n"
              "<default></default>\n"
              "<constraints>\n"
                "<allowempty>true</allowempty>\n"
              "</constraints>\n"
            "</setting>\n"
          "</group>\n"
        "</category>\n"
      "</section>\n"
    "</settings>\n";

  if (!CanImport(source.GetIdentifier()))
    return false;

  auto& settings = source.Settings();
  settings.SetDefinition(SettingsDefinition);

  if (!settings.Load())
    return false;

  // if necessary generate a new device identifier
  if (settings.GetString(SettingDeviceId).empty())
  {
    settings.SetString(SettingDeviceId, StringUtils::CreateUUID());
    settings.Save();
  }

  // ATTENTION: the const_cast() is a bit hacky but since we don't modify the
  // instance in the options filler, it's not that big of a deal
  settings.SetOptionsFiller(SettingUser, SettingOptionsUsersFiller, &source);

  return true;
}

bool CEmbyMediaImporter::UnloadSourceSettings(CMediaImportSource& source) const
{
  if (!CanImport(source.GetIdentifier()))
    return false;

  auto& settings = source.Settings();
  if (!settings.IsLoaded())
    return false;

  // save the settings
  bool result = settings.Save();

  // unload the settings completely
  settings.Unload();

  return result;
}

IMediaImporter* CEmbyMediaImporter::Create(const CMediaImport &import) const
{
  if (!CanImport(import.GetPath()))
    return nullptr;

  return new CEmbyMediaImporter(import);
}

void CEmbyMediaImporter::Start()
{
  m_discovery.Start();
}

bool CEmbyMediaImporter::Import(CMediaImportRetrievalTask* task) const
{
  static const std::vector<std::string> Fields = {
    PropertyItemDateCreated,
    PropertyItemGenres,
    PropertyItemMediaStreams,
    PropertyItemOverview,
    PropertyItemOverview,
    PropertyItemShortOverview,
    PropertyItemPath,
    PropertyItemPeople,
    PropertyItemProviderIds,
    PropertyItemSortName,
    PropertyItemOriginalTitle,
    PropertyItemStudios,
    PropertyItemTaglines,
    PropertyItemProductionLocations,
    PropertyItemTags
  };

  if (task == nullptr)
    return false;

  CURL baseUrl(BuildUserUrl(UrlItems));
  baseUrl.SetOption("Recursive", "true");
  // get additional property fields
  baseUrl.SetOption("Fields", StringUtils::Join(Fields, ","));
  // exclude virtual items and items that can't be accessed from another machine
  baseUrl.SetOption("ExcludeLocationTypes", "Virtual,Offline");
  // limit the requests to be able to abort
  baseUrl.SetOption("Limit", StringUtils::Format("%u", ItemRequestLimit));

  const auto& mediaTypes = GetImport().GetMediaTypes();
  size_t mediaTypeProgress = 0;
  size_t mediaTypeTotal = mediaTypes.size();
  for (const auto& importedMediaType : mediaTypes)
  {
    if (task->ShouldCancel(mediaTypeProgress, mediaTypeTotal))
      return false;

    const auto& embyMediaType = MediaTypeMapping.find(importedMediaType);
    if (embyMediaType == MediaTypeMapping.end())
    {
      CLog::Log(LOGERROR, "CEmbyMediaImporter: cannot import unsupported media type \"%s\"", importedMediaType.c_str());
      return false;
    }

    task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(39068).c_str(), CMediaTypes::GetPluralLocalization(importedMediaType).c_str()));
    
    CURL actualUrl = baseUrl;
    actualUrl.SetOption("IncludeItemTypes", embyMediaType->second);

    XFILE::CCurlFile curl;
    PrepareApiCall(m_apiKey, m_userId, m_deviceId, curl);

    std::vector<CFileItemPtr> items;
    uint32_t totalCount = 0;
    uint32_t startIndex = 0;
    do
    {
      if (task->ShouldCancel(startIndex, std::max(totalCount, 1u)))
        return false;

      actualUrl.SetOption("StartIndex", StringUtils::Format("%u", startIndex));

      std::string result;
      if (!curl.Get(actualUrl.Get(), result) || result.empty())
      {
        CLog::Log(LOGERROR, "CEmbyMediaImporter: failed to retrieve items of media type \"%s\" from %s", importedMediaType.c_str(), CURL::GetRedacted(actualUrl.Get()).c_str());
        return false;
      }

      auto resultObject = CJSONVariantParser::Parse(result);
      if (!resultObject.isObject() || !resultObject.isMember(PropertyItemItems) || !resultObject.isMember(PropertyItemTotalRecordCount))
      {
        CLog::Log(LOGERROR, "CEmbyMediaImporter: invalid response for items of media type \"%s\" from %s", importedMediaType.c_str(), CURL::GetRedacted(actualUrl.Get()).c_str());
        return false;
      }

      totalCount = static_cast<uint32_t>(resultObject[PropertyItemTotalRecordCount].asUnsignedInteger());

      const auto& itemsObject = resultObject[PropertyItemItems];
      for (auto it = itemsObject.begin_array(); it != itemsObject.end_array(); ++it)
      {
        CFileItemPtr item = ToFileItem(*it, importedMediaType);
        if (item == nullptr)
          continue;

        items.push_back(item);
      }

      startIndex += itemsObject.size();
    }
    while (startIndex < totalCount);

    task->AddItems(items, importedMediaType, MediaImportChangesetTypeNone);

    ++mediaTypeProgress;
  }

  return true;
}

bool CEmbyMediaImporter::UpdateOnSource(CMediaImportUpdateTask* task) const
{
  if (task == nullptr || !task->GetItem().IsImported())
    return false;

  const CMediaImportSettings &importSettings = task->GetImport().GetSettings();
  if (!importSettings.UpdatePlaybackMetadataOnSource())
    return false;

  const std::string &importPath = task->GetImport().GetPath();
  if (!CanUpdatePlaycountOnSource(importPath) &&
      !CanUpdateLastPlayedOnSource(importPath) &&
      !CanUpdateResumePositionOnSource(importPath))
    return false;

  const auto item = task->GetItem();

  if (!item.HasVideoInfoTag())
    return false;

  std::string itemId;
  if (!GetItemId(item.GetVideoInfoTag()->GetPath(), itemId))
    return false;

  // get the URL to retrieve all details of the item from the Emby server
  const auto getItemUrl = BuildUserItemUrl(itemId);

  XFILE::CCurlFile curl;
  PrepareApiCall(m_apiKey, m_userId, m_deviceId, curl);

  std::string result;
  // retrieve all details of the item
  if (!curl.Get(getItemUrl, result) || result.empty())
    return false;

  const auto resultItem = CJSONVariantParser::Parse(result);
  if (!resultItem.isObject() || resultItem.empty())
    return false;

  const auto& userData = resultItem[PropertyItemUserData];
  if (!userData.isObject() || userData.empty())
    return false;

  const auto& videoInfo = item.GetVideoInfoTag();

  // check and update playcout/played/lastplayed if necessary
  bool played = videoInfo->m_playCount > 0;
  if (userData[PropertyItemUserDataPlayCount].asInteger() != videoInfo->m_playCount ||
    userData[PropertyItemUserDataPlayed].asBoolean() != played)
  {
    bool success = false;
    if (played)
      success = MarkAsWatched(itemId, videoInfo->m_lastPlayed);
    else
      success = MarkAsUnwatched(itemId);

    if (!success)
      return false;
  }

  // update resume point if necessary
  const auto playbackPositionTicks = SecondsToTicks(static_cast<uint64_t>(videoInfo->m_resumePoint.timeInSeconds));
  if (userData[PropertyItemUserDataPlaybackPositionTicks].asUnsignedInteger() != playbackPositionTicks)
  {
    // TODO: THIS DOESN'T SEEM TO WORK
    if (!UpdateResumePoint(itemId, playbackPositionTicks))
      return false;
  }

  return true;
}

CFileItemPtr CEmbyMediaImporter::ToFileItem(const CVariant& item, const MediaType& mediaType) const
{
  const auto& embyMediaType = MediaTypeMapping.find(mediaType);
  if (embyMediaType == MediaTypeMapping.end())
  {
    CLog::Log(LOGERROR, "CEmbyMediaImporter: cannot import item with unsupported media type \"%s\"", mediaType.c_str());
    return nullptr;
  }

  if (!item.isObject() || !item.isMember(PropertyItemType) || item[PropertyItemType].asString() != embyMediaType->second)
  {
    CLog::Log(LOGERROR, "CEmbyMediaImporter: cannot import item with media type \"%s\" from invalid object", mediaType.c_str());
    return nullptr;
  }

  const std::string itemId = item[PropertyItemId].asString();
  if (itemId.empty())
  {
    CLog::Log(LOGERROR, "CEmbyMediaImporter: cannot import item with media type \"%s\" without an identifier", mediaType.c_str());
    return nullptr;
  }

  bool isFolder = item[PropertyItemIsFolder].asBoolean();
  std::string itemPath;
  if (isFolder)
    itemPath = BuildFolderItemPath(itemId);
  else
    itemPath = BuildPlayableItemPath(item[PropertyItemMediaType].asString(), itemId, item[PropertyItemContainer].asString());

  CFileItemPtr fileItem = std::make_shared<CFileItem>(itemPath, isFolder);
  fileItem->SetLabel(item[PropertyItemName].asString());
  fileItem->m_dateTime.SetFromW3CDateTime(item[PropertyItemPremiereDate].asString());

  // TODO: works only for videos
  FillVideoInfoTag(item, fileItem, mediaType, fileItem->GetVideoInfoTag());
  
  // artwork
  std::map<std::string, std::string> artwork;
  if (item.isMember(PropertyItemImageTags) && item[PropertyItemImageTags].isObject())
  {
    const auto& images = item[PropertyItemImageTags];

    // check primary image
    if (images.isMember(PropertyItemImageTagsPrimary))
      artwork.insert(std::make_pair("poster", BuildImagePath(itemId, PropertyItemImageTagsPrimary, images[PropertyItemImageTagsPrimary].asString())));

    // check logo
    if (images.isMember(PropertyItemImageTagsLogo))
      artwork.insert(std::make_pair("logo", BuildImagePath(itemId, PropertyItemImageTagsLogo, images[PropertyItemImageTagsLogo].asString())));
  }

  // check fanart
  if (item.isMember(PropertyItemBackdropImageTags) && item[PropertyItemBackdropImageTags].isArray() && !item[PropertyItemBackdropImageTags].empty())
    artwork.insert(std::make_pair("fanart", BuildImagePath(itemId, "Backdrop/0", item[PropertyItemBackdropImageTags].begin_array()->asString())));

  if (!artwork.empty())
    fileItem->AppendArt(artwork);

  return fileItem;
}

void CEmbyMediaImporter::FillVideoInfoTag(const CVariant& item, CFileItemPtr fileItem, const MediaType& mediaType, CVideoInfoTag* videoInfo) const
{
  if (videoInfo == nullptr)
    return;

  videoInfo->m_type = mediaType;
  videoInfo->SetPath(item[PropertyItemPath].asString());
  videoInfo->SetFileNameAndPath(fileItem->GetPath());
  videoInfo->SetTitle(fileItem->GetLabel());
  if (mediaType == MediaTypeMovie || mediaType == MediaTypeTvShow)
    videoInfo->SetSortTitle(item[PropertyItemSortName].asString());
  videoInfo->SetOriginalTitle(item[PropertyItemOriginalTitle].asString());
  videoInfo->SetPlot(item[PropertyItemOverview].asString());
  videoInfo->SetPlotOutline(item[PropertyItemShortOverview].asString());
  if (item.isMember(PropertyItemTaglines) && item[PropertyItemTaglines].isArray() && !item[PropertyItemTaglines].empty())
    videoInfo->SetTagLine(item[PropertyItemTaglines].begin_array()->asString());
  videoInfo->m_dateAdded.SetFromW3CDateTime(item[PropertyItemDateCreated].asString());

  if (fileItem->m_dateTime.IsValid())
  {
    if (mediaType == MediaTypeEpisode)
      videoInfo->m_firstAired = fileItem->m_dateTime;
    else
      videoInfo->SetPremiered(fileItem->m_dateTime);
  }
  else
    videoInfo->SetYear(static_cast<int>(item[PropertyItemProductionYear].asInteger()));

  videoInfo->SetRating(item[PropertyItemCommunityRating].asFloat(), static_cast<int>(item[PropertyItemVoteCount].asInteger()), "", true);
  videoInfo->SetMPAARating(item[PropertyItemOfficialRating].asString());
  videoInfo->m_duration = static_cast<int>(TicksToSeconds(item[PropertyItemRunTimeTicks].asUnsignedInteger()));
  videoInfo->m_playCount = static_cast<int>(item[PropertyItemUserData][PropertyItemUserDataPlayCount].asInteger());
  videoInfo->m_lastPlayed.SetFromW3CDateTime(item[PropertyItemUserData][PropertyItemUserDataLastPlayedDate].asString());
  videoInfo->m_resumePoint.timeInSeconds = static_cast<int>(TicksToSeconds(item[PropertyItemUserData][PropertyItemUserDataPlaybackPositionTicks].asUnsignedInteger()));
  if (videoInfo->m_duration > 0 && videoInfo->m_resumePoint.timeInSeconds > 0)
  {
    videoInfo->m_resumePoint.totalTimeInSeconds = videoInfo->m_duration;
    videoInfo->m_resumePoint.type = CBookmark::RESUME;
  }

  if (mediaType == MediaTypeTvShow)
  {
    videoInfo->m_strShowTitle = videoInfo->m_strTitle;
    videoInfo->m_strStatus = item[PropertyItemStatus].asString();
  }
  else if (mediaType == MediaTypeSeason || mediaType == MediaTypeEpisode)
  {
    videoInfo->m_strShowTitle = item[PropertyItemSeriesName].asString();

    if (mediaType == MediaTypeSeason)
      videoInfo->m_iSeason = static_cast<int>(item[PropertyItemIndexNumber].asInteger());
    else
    {
      videoInfo->m_iSeason = static_cast<int>(item[PropertyItemParentIndexNumber].asInteger());
      videoInfo->m_iEpisode = static_cast<int>(item[PropertyItemIndexNumber].asInteger());
    }
  }
  else if (mediaType == MediaTypeMusicVideo)
  {
    if (item.isMember(PropertyItemArtists) && item[PropertyItemArtists].isArray())
    {
      std::vector<std::string> vecArtists;
      const auto& artists = item[PropertyItemArtists];
      for (auto artist = artists.begin_array(); artist != artists.end_array(); ++artist)
        vecArtists.push_back(artist->asString());

      videoInfo->SetArtist(vecArtists);
    }

    videoInfo->SetAlbum(item[PropertyItemAlbum].asString());
  }

  if (item.isMember(PropertyItemGenres) && item[PropertyItemGenres].isArray())
  {
    const auto& genres = item[PropertyItemGenres];
    for (auto genre = genres.begin_array(); genre != genres.end_array(); ++genre)
      videoInfo->m_genre.push_back(genre->asString());
  }

  if (item.isMember(PropertyItemStudios) && item[PropertyItemStudios].isArray())
  {
    const auto& studios = item[PropertyItemStudios];
    for (auto studio = studios.begin_array(); studio != studios.end_array(); ++studio)
      videoInfo->m_studio.push_back((*studio)[PropertyItemName].asString());
  }

  if (item.isMember(PropertyItemProductionLocations) && item[PropertyItemProductionLocations].isArray())
  {
    const auto& countries = item[PropertyItemProductionLocations];
    for (auto country = countries.begin_array(); country != countries.end_array(); ++country)
      videoInfo->m_country.push_back(country->asString());
  }

  if (item.isMember(PropertyItemProviderIds) && item[PropertyItemProviderIds].isObject())
  {
    const auto& uniqueids = item[PropertyItemProviderIds];
    for (auto uniqueid = uniqueids.begin_map(); uniqueid != uniqueids.end_map(); ++uniqueid)
      videoInfo->SetUniqueID(uniqueid->second.asString(), uniqueid->first);
  }

  if (item.isMember(PropertyItemPeople) && item[PropertyItemPeople].isArray())
  {
    const auto& people = item[PropertyItemPeople];
    for (auto personIt = people.begin_array(); personIt != people.end_array(); ++personIt)
    {
      const auto person = *personIt;
      if (!person.isObject())
        continue;

      const auto name = person[PropertyItemName].asString();
      const auto type = person[PropertyItemType].asString();

      if (type == "Actor")
      {
        SActorInfo actor;
        actor.strName = name;
        actor.strRole = person[PropertyItemRole].asString();
        actor.order = std::distance(people.begin_array(), personIt);

        videoInfo->m_cast.push_back(actor);
      }
      else if (type == "Writer")
        videoInfo->m_writingCredits.push_back(name);
      else if (type == "Director")
        videoInfo->m_director.push_back(name);
    }
  }

  if (item.isMember(PropertyItemTags) && item[PropertyItemTags].isArray())
  {
    const auto& tags = item[PropertyItemTags];
    for (auto tag = tags.begin_array(); tag != tags.end_array(); ++tag)
      videoInfo->m_tags.push_back(tag->asString());
  }

  if (item.isMember(PropertyItemMediaStreams) && item[PropertyItemMediaStreams].isArray())
  {
    const auto& streams = item[PropertyItemMediaStreams];
    for (auto streamIt = streams.begin_array(); streamIt != streams.end_array(); ++streamIt)
    {
      const auto stream = *streamIt;
      const auto streamType = stream[PropertyItemMediaStreamType].asString();
      CStreamDetail* streamDetail = nullptr;
      if (streamType == "Video")
      {
        CStreamDetailVideo* videoStream = new CStreamDetailVideo();
        videoStream->m_strCodec = stream[PropertyItemMediaStreamCodec].asString();
        videoStream->m_strLanguage = stream[PropertyItemMediaStreamLanguage].asString();
        videoStream->m_iWidth = static_cast<int>(stream[PropertyItemMediaStreamWidth].asInteger());
        videoStream->m_iHeight = static_cast<int>(stream[PropertyItemMediaStreamHeight].asInteger());
        videoStream->m_iDuration = videoInfo->m_duration;

        streamDetail = videoStream;
      }
      else if (streamType == "Audio")
      {
        CStreamDetailAudio* audioStream = new CStreamDetailAudio();
        audioStream->m_strCodec = stream[PropertyItemMediaStreamCodec].asString();
        audioStream->m_strLanguage = stream[PropertyItemMediaStreamLanguage].asString();
        audioStream->m_iChannels = static_cast<int>(stream[PropertyItemMediaStreamChannels].asInteger());

        streamDetail = audioStream;
      }
      else if (streamType == "Subtitle")
      {
        CStreamDetailSubtitle* subtitleStream = new CStreamDetailSubtitle();
        subtitleStream->m_strLanguage = stream[PropertyItemMediaStreamLanguage].asString();

        streamDetail = subtitleStream;
      }

      if (streamDetail != nullptr)
        videoInfo->m_streamDetails.AddStream(streamDetail);
    }
  }
}

bool CEmbyMediaImporter::MarkAsWatched(const std::string& itemId, CDateTime lastPlayed) const
{
  if (itemId.empty())
    return false;

  // use the current date and time if lastPlayed is invalid
  if (!lastPlayed.IsValid())
    lastPlayed = CDateTime::GetUTCDateTime();

  XFILE::CCurlFile curl;
  PrepareApiCall(m_apiKey, m_userId, m_deviceId, curl);

  // get the URL to updated the item's played state
  CURL url(BuildUserPlayedItemUrl(itemId));
  // and add the DatePlayed URL parameter
  url.SetOption("DatePlayed",
    StringUtils::Format("%04i%02i%02i%02i%02i%02i", lastPlayed.GetYear(), lastPlayed.GetMonth(),
      lastPlayed.GetDay(), lastPlayed.GetHour(), lastPlayed.GetMinute(), lastPlayed.GetSecond()));

  std::string response;
  // execute the POST request
  return curl.Post(url.Get(), "", response);
}

bool CEmbyMediaImporter::MarkAsUnwatched(const std::string& itemId) const
{
  if (itemId.empty())
    return false;

  XFILE::CCurlFile curl;
  PrepareApiCall(m_apiKey, m_userId, m_deviceId, curl);

  // get the URL to updated the item's played state
  const auto url = BuildUserPlayedItemUrl(itemId);

  std::string response;
  // execute the DELETE request
  return curl.Delete(url, response);
}

bool CEmbyMediaImporter::UpdateResumePoint(const std::string& itemId, uint64_t resumePointInTicks) const
{
  if (itemId.empty())
    return false;

  XFILE::CCurlFile curl;
  PrepareApiCall(m_apiKey, m_userId, m_deviceId, curl);

  // get the URL to updated the item's resume point
  const auto url = BuildUrl("Sessions/Playing/Stopped");

  // put together the POST request's body
  CVariant data(CVariant::VariantTypeObject);
  data["itemId"] = itemId;
  data["mediaSourceId"] = itemId;
  data["positionTicks"] = resumePointInTicks;
  data["PlaySessionId"] = ""; // TODO; how do we get this

  // serialize the POST request's body
  const auto postData = CJSONVariantWriter::Write(data, true);

  std::string response;
  // execute the DELETE request
  return curl.Post(url, postData, response);
}

std::string CEmbyMediaImporter::BuildUrl(const std::string& endpoint) const
{
  std::string url = m_url;
  if (!endpoint.empty())
    url = URIUtils::AddFileToFolder(url, endpoint);

  return url;
}

std::string CEmbyMediaImporter::BuildUserUrl(const std::string& endpoint) const
{
  std::string url = m_url;
  if (!m_userId.empty())
  {
    url = URIUtils::AddFileToFolder(url, UrlUsers);
    url = URIUtils::AddFileToFolder(url, m_userId);
  }

  if (!endpoint.empty())
    url = URIUtils::AddFileToFolder(url, endpoint);

  return url;
}

std::string CEmbyMediaImporter::BuildItemUrl(const std::string& itemId) const
{
  std::string url = BuildUrl(UrlItems);

  if (!itemId.empty())
    url = URIUtils::AddFileToFolder(url, itemId);

  return url;
}

std::string CEmbyMediaImporter::BuildUserItemUrl(const std::string& itemId) const
{
  std::string url = BuildUserUrl(UrlItems);

  if (!itemId.empty())
    url = URIUtils::AddFileToFolder(url, itemId);

  return url;
}

std::string CEmbyMediaImporter::BuildUserPlayedItemUrl(const std::string& itemId) const
{
  std::string url = BuildUserUrl(UrlPlayedItems);

  if (!itemId.empty())
    url = URIUtils::AddFileToFolder(url, itemId);

  return url;
}

std::string CEmbyMediaImporter::BuildEmbyPath(const std::string& url) const
{
  CURL fullUrl;
  fullUrl.SetProtocol(EmbyProtocol);

  std::string hostname = m_apiKey;
  if (!m_userId.empty())
    hostname += ":" + m_userId;
  fullUrl.SetHostName(CURL::Encode(hostname));

  fullUrl.SetFileName(CURL::Encode(url));

  return fullUrl.Get();
}

std::string CEmbyMediaImporter::BuildPlayableItemPath(const std::string& mediaType, const std::string& itemId, const std::string& container) const
{
  static const std::map<std::string, std::string> TypeMapping = {
    { "Video", "Videos" },
    { "Audio", "Audio" }
  };

  if (itemId.empty())
    return StringUtils::Empty;

  const auto& type = TypeMapping.find(mediaType);
  if (type == TypeMapping.cend())
    return StringUtils::Empty;

  std::string url = BuildUrl(type->second);
  url = URIUtils::AddFileToFolder(url, itemId);
  url = URIUtils::AddFileToFolder(url, "stream");
  if (!container.empty())
    url += "." + container;

  CURL finalUrl(url);
  finalUrl.SetOption("MediaSourceId", itemId);
  finalUrl.SetOption("static", "true");

  return finalUrl.Get();
}

std::string CEmbyMediaImporter::BuildFolderItemPath(const std::string& itemId) const
{
  if (itemId.empty())
    return StringUtils::Empty;

  std::string url = BuildUserUrl(UrlItems);
  url = URIUtils::AddFileToFolder(url, itemId);

  return url;
}

std::string CEmbyMediaImporter::BuildImagePath(const std::string& itemId, const std::string& imageType, const std::string& imageTag /* = "" */) const
{
  if (itemId.empty() || imageType.empty())
    return StringUtils::Empty;

  std::string url = BuildItemUrl(itemId);
  url = URIUtils::AddFileToFolder(url, "Images");
  url = URIUtils::AddFileToFolder(url, imageType);

  if (!imageTag.empty())
    url += "?tag=" + imageTag;

  return url;
}

bool CEmbyMediaImporter::GetServerId(const std::string& path, std::string& id)
{
  if (path.empty())
    return false;

  CURL embyUrl(path);
  if (!embyUrl.IsProtocol(EmbyProtocol) || embyUrl.GetHostName().empty())
    return false;

  id = embyUrl.GetHostName();

  return true;
}

bool CEmbyMediaImporter::GetItemId(const std::string& path, std::string& id)
{
  if (path.empty() || !URIUtils::IsHTTP(path))
    return false;

  CURL itemUrl(path);
  if (itemUrl.GetHostName().empty() || itemUrl.GetFileName().empty())
    return false;

  const auto itemUrlParts = StringUtils::Split(itemUrl.GetFileName(), itemUrl.GetDirectorySeparator());
  if (itemUrlParts.size() < 2)
    return false;

  id = itemUrlParts.at(1);

  return !id.empty();
}

void CEmbyMediaImporter::SettingOptionsUsersFiller(const CSetting* setting, std::vector<std::pair<std::string, std::string>>& list, std::string& current, void* data)
{
  const auto currentValue = current;

  // add default choice and activate it by default
  list.push_back(std::make_pair(g_localizeStrings.Get(231), ""));
  current = "";

  if (data == nullptr)
    return;

  auto source = reinterpret_cast<CMediaImportSource*>(data);
  if (source == nullptr)
    return;

  // try to parse the source's identifier
  std::string id;
  if (!GetServerId(source->GetIdentifier(), id))
    return;

  // try to figure out the API key from the current settings
  const auto& settings = source->Settings();
  if (!settings.IsLoaded())
    return;

  const std::string apiKey = settings.GetString(SettingApiKey);
  if (apiKey.empty())
    return;

  const std::string& currentUserId = static_cast<const CSettingString*>(setting)->GetValue();

  // put together the url to retrieve all available users
  std::string usersUrl = source->GetBasePath();
  usersUrl = URIUtils::AddFileToFolder(usersUrl, UrlUsers);
  usersUrl = URIUtils::AddFileToFolder(usersUrl, UrlUsersPublic);

  XFILE::CCurlFile curl;
  PrepareApiCall(apiKey, currentUserId, settings.GetString(SettingDeviceId), curl);

  std::string result;
  if (!curl.Get(usersUrl, result) || result.empty())
    return;

  auto resultObject = CJSONVariantParser::Parse(result);
  if (!resultObject.isArray() || resultObject.empty())
    return;

  for (auto userIt = resultObject.begin_array(); userIt != resultObject.end_array(); ++userIt)
  {
    const auto user = *userIt;

    // make sure the "Name" and "Id" properties are available
    if (!user.isObject() || !user.isMember(PropertyUserName) || !user.isMember(PropertyUserId))
      continue;

    std::string name = user[PropertyUserName].asString();
    std::string id = user[PropertyUserId].asString();
    if (name.empty() || id.empty())
      continue;

    // check if the user is disabled
    if (user.isMember(PropertyUserPolicy) && user[PropertyUserPolicy].isObject() &&
        user[PropertyUserPolicy].isMember(PropertyUserIsDisabled) &&
        user[PropertyUserPolicy][PropertyUserIsDisabled].asBoolean())
      continue;

    list.push_back(std::make_pair(name, id));

    // check if this is the currently selected user
    if (id == currentValue)
      current = id;
  }
}

CEmbyMediaImporter::CEmbyServerDiscovery::CEmbyServerDiscovery()
  : CThread("EmbyServerDiscovery")
{ }

void CEmbyMediaImporter::CEmbyServerDiscovery::Start()
{
  CThread::Create();
}

void CEmbyMediaImporter::CEmbyServerDiscovery::Process()
{
  static const int DiscoveryPort = 7359;
  static const char* DiscoveryMessage = "who is EmbyServer?";
  static const int DiscoveryTimeoutMs = 1000;
  static const size_t DiscoveryBufferSize = 1024;

  CLog::Log(LOGINFO, "CEmbyServerDiscovery: looking for Emby servers...");
  
  // create a new UDP socket
  SOCKETS::CUDPSocket* socket = SOCKETS::CSocketFactory::CreateUDPSocket();
  if (socket == nullptr)
  {
    CLog::Log(LOGERROR, "CEmbyServerDiscovery: failed to create a UDP socket");
    return;
  }

  // limit the socket to IPv4
  socket->SetIpv4Only(true);
  // enable broadcasting
  socket->SetBroadcast(true);

  // try to bind the socket to the Emby discovery port
  if (!socket->Bind(false /* TODO */, 0))
  {
    CLog::Log(LOGERROR, "CEmbyServerDiscovery: failed to bind UDP socket to port %d", DiscoveryPort);
    return;
  }

  // add our socket to the select() listener
  SOCKETS::CSocketListener listener;
  listener.AddSocket(socket);

  // allocate buffer for discovery response
  char receiveBuffer[DiscoveryBufferSize] = { 0 };

  SOCKETS::CAddress broadcastAddress("255.255.255.255", DiscoveryPort);

  while (!m_bStop)
  {
    try
    {
      // broadcast the message to all Emby servers
      if (socket->SendTo(broadcastAddress, strlen(DiscoveryMessage), static_cast<const void*>(DiscoveryMessage)) < 0)
        CLog::Log(LOGWARNING, "CEmbyServerDiscovery: failed to broadcast to Emby servers");
    }
    catch (...)
    {
      CLog::Log(LOGERROR, "CEmbyServerDiscovery: error while broadcasting for Emby servers");
      break;
    }

    try
    {
      // start listening until we timeout
      if (listener.Listen(DiscoveryTimeoutMs))
      {
        // clear the receive buffer
        memset(static_cast<void*>(receiveBuffer), 0, sizeof(receiveBuffer));

        // try to receive an answer from an Emby server
        SOCKETS::CAddress serverAddress;
        int receiveSize = 0;
        if ((receiveSize = socket->Read(serverAddress, sizeof(receiveBuffer), static_cast<void*>(receiveBuffer))) > 0)
        {
          std::string result(receiveBuffer, receiveSize);
          EmbyServer newServer;
          if (ToEmbyServer(result, newServer))
            AddEmbyServer(newServer);
        }
      }
    }
    catch (...)
    {
      CLog::Log(LOGERROR, "CEmbyServerDiscovery: error while looking for Emby servers");
      break;
    }

    // check if any discovered emby servers haven't answered for a while
    // TODO: ExpireEmbyServers();

    Sleep(DiscoveryTimeoutMs);
  }

  // cleanup
  listener.Clear();
  delete socket;

  CLog::Log(LOGINFO, "CEmbyServerDiscovery: stopped looking for Emby servers");
}

void CEmbyMediaImporter::CEmbyServerDiscovery::AddEmbyServer(const CEmbyMediaImporter::CEmbyServerDiscovery::EmbyServer& embyServer)
{
  bool registerServer = false;

  auto it = m_servers.find(embyServer.id);
  // if we don't know this Emby server, we add it
  if (it == m_servers.end())
  {
    it = m_servers.insert(std::make_pair(embyServer.id, embyServer)).first;
    registerServer = true;
  }
  // if the Emby server isn't registered or has changed, we update it
  else if (!it->second.registered || it->second.name != embyServer.name || it->second.address != embyServer.address)
  {
    it->second = embyServer;
    registerServer = true;
  }
  // otherwise we update its last seen property
  else
    it->second.lastSeen = embyServer.lastSeen;

  if (!registerServer)
    return;

  const auto sourceId = BuildSourceIdentifier(embyServer.id);
  const auto iconUrl = URIUtils::AddFileToFolder(embyServer.address, "web/touchicon144.png");
  // TODO: AddSource is asynchronous if the source doesn't exist yet
  if (!CMediaImportManager::GetInstance().AddSource(sourceId, embyServer.address, embyServer.name, iconUrl, { MediaTypeMovie, MediaTypeMusicVideo, MediaTypeTvShow, MediaTypeSeason, MediaTypeEpisode }) ||
      !CMediaImportManager::GetInstance().ActivateSource(sourceId, embyServer.address, embyServer.name, iconUrl))
  {
    CLog::Log(LOGWARNING, "CEmbyServerDiscovery: failed to add/activate Emby server \"%s\" (%s)", embyServer.name.c_str(), embyServer.id.c_str());
    it->second.registered = false;
  }
  else
  {
    it->second.registered = true;
    CLog::Log(LOGINFO, "CEmbyServerDiscovery: Emby server \"%s\" (%s) successfully added and activated", embyServer.name.c_str(), embyServer.id.c_str());
  }
}

void CEmbyMediaImporter::CEmbyServerDiscovery::ExpireEmbyServers()
{
  static const int32_t Timeout = 10;

  const CDateTime now = CDateTime::GetCurrentDateTime();

  for (auto&& server : m_servers)
  {
    // if the server is registered and hasn't been active since a while we deactivate it
    if (server.second.registered && (now - server.second.lastSeen).GetSecondsTotal() > Timeout)
    {
      server.second.registered = false;
      CMediaImportManager::GetInstance().DeactivateSource(BuildSourceIdentifier(server.second.id));
      CLog::Log(LOGINFO, "CEmbyServerDiscovery: Emby server \"%s\" (%s) deactivated due to inactivity", server.second.name.c_str(), server.second.id.c_str());
    }
  }
}

bool CEmbyMediaImporter::CEmbyServerDiscovery::ToEmbyServer(const std::string& result, CEmbyMediaImporter::CEmbyServerDiscovery::EmbyServer& embyServer)
{
  static const std::string ServerPropertyId = "Id";
  static const std::string ServerPropertyName = "Name";
  static const std::string ServerPropertyAddress = "Address";

  if (result.empty())
    return false;

  auto data = CJSONVariantParser::Parse(result);
  if (!data.isObject() || !data.isMember(ServerPropertyId) || !data.isMember(ServerPropertyName) || !data.isMember(ServerPropertyAddress))
  {
    CLog::Log(LOGWARNING, "CEmbyServerDiscovery: invalid discovery message received: \"%s\"", result.c_str());
    return false;
  }

  embyServer.id = data[ServerPropertyId].asString();
  embyServer.name = data[ServerPropertyName].asString();
  embyServer.address = data[ServerPropertyAddress].asString();
  embyServer.registered = false;
  embyServer.lastSeen = CDateTime::GetCurrentDateTime();

  return !embyServer.id.empty() && !embyServer.name.empty() && !embyServer.address.empty();
}
