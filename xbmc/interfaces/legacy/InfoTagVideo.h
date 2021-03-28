/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "AddonClass.h"
#include "Dictionary.h"
#include "Tuple.h"
#include "utils/StreamDetails.h"
#include "utils/Variant.h"
#include "video/VideoInfoTag.h"

namespace XBMCAddon
{
  namespace xbmc
  {
    // TODO(Montellese)
    class Actor : public AddonClass
    {
    public:
      explicit Actor(const String& name = emptyString,
                     const String& role = emptyString,
                     int order = -1,
                     const String& thumbnail = emptyString);

      String getName() const { return m_name; }
      String getRole() const { return m_role; }
      int getOrder() const { return m_order; }
      String getThumbnail() const { return m_thumbnail; }

      void setName(const String& name) { m_name = name; }
      void setRole(const String& role) { m_role = role; }
      void setOrder(int order) { m_order = order; }
      void setThumbnail(const String& thumbnail) { m_thumbnail = thumbnail; }

#ifndef SWIG
      SActorInfo ToActorInfo() const;
#endif

    private:
      String m_name;
      String m_role;
      int m_order;
      String m_thumbnail;
    };

    // TODO(Montellese)
    class VideoStreamDetail : public AddonClass
    {
    public:
      explicit VideoStreamDetail(int width = 0,
                                 int height = 0,
                                 float aspect = 0.0f,
                                 int duration = 0,
                                 const String& codec = emptyString,
                                 const String& stereoMode = emptyString,
                                 const String& language = emptyString);

      int getWidth() const { return m_width; }
      int getHeight() const { return m_height; }
      float getAspect() const { return m_aspect; }
      int getDuration() const { return m_duration; }
      String getCodec() const { return m_codec; }
      String getStereoMode() const { return m_stereoMode; }
      String getLanguage() const { return m_language; }

      void setWidth(int width) { m_width = width; }
      void setHeight(int height) { m_height = height; }
      void setAspect(float aspect) { m_aspect = aspect; }
      void setDuration(int duration) { m_duration = duration; }
      void setCodec(const String& codec) { m_codec = codec; }
      void setStereoMode(const String& stereoMode) { m_stereoMode = stereoMode; }
      void setLanguage(const String& language) { m_language = language; }

#ifndef SWIG
      CStreamDetailVideo* ToStreamDetailVideo() const;
#endif

    private:
      int m_width;
      int m_height;
      float m_aspect;
      int m_duration;
      String m_codec;
      String m_stereoMode;
      String m_language;
    };

    // TODO(Montellese)
    class AudioStreamDetail : public AddonClass
    {
    public:
      explicit AudioStreamDetail(int channels = -1,
                                 const String& codec = emptyString,
                                 const String& language = emptyString);

      int getChannels() const { return m_channels; }
      String getCodec() const { return m_codec; }
      String getLanguage() const { return m_language; }

      void setChannels(int channels) { m_channels = channels; }
      void setCodec(const String& codec) { m_codec = codec; }
      void setLanguage(const String& language) { m_language = language; }

#ifndef SWIG
      CStreamDetailAudio* ToStreamDetailAudio() const;
#endif

    private:
      int m_channels;
      String m_codec;
      String m_language;
    };

    // TODO(Montellese)
    class SubtitleStreamDetail : public AddonClass
    {
    public:
      explicit SubtitleStreamDetail(const String& language = emptyString);

      String getLanguage() const { return m_language; }

      void setLanguage(const String& language) { m_language = language; }

#ifndef SWIG
      CStreamDetailSubtitle* ToStreamDetailSubtitle() const;
#endif

    private:
      String m_language;
    };

    // TODO(Montellese)
    class Fanart : public AddonClass
    {
    public:
      explicit Fanart(const String& image = emptyString,
                      const String& preview = emptyString,
                      const String& colors = emptyString);

      String getImage() const { return m_image; }
      String getPreview() const { return m_preview; }
      String getColors() const { return m_colors; }

      void setImage(const String& image) { m_image = image; }
      void setPreview(const String& preview) { m_preview = preview; }
      void setColors(const String& colors) { m_colors = colors; }

    private:
      String m_image;
      String m_preview;
      String m_colors;
    };

    ///
    /// \defgroup python_InfoTagVideo InfoTagVideo
    /// \ingroup python_xbmc
    /// @{
    /// @brief **Kodi's video info tag class.**
    ///
    /// \python_class{ InfoTagVideo() }
    ///
    /// To get video info tag data of currently played source.
    ///
    /// @note Info tag load is only be possible from present player class.
    ///
    ///
    ///-------------------------------------------------------------------------
    ///
    /// **Example:**
    /// ~~~~~~~~~~~~~{.py}
    /// ...
    /// tag = xbmc.Player().getVideoInfoTag()
    ///
    /// title = tag.getTitle()
    /// file  = tag.getFile()
    /// ...
    /// ~~~~~~~~~~~~~
    ///
    class InfoTagVideo : public AddonClass
    {
    private:
      CVideoInfoTag* infoTag;
      bool offscreen;
      bool owned;

    public:
#ifndef SWIG
      explicit InfoTagVideo(const CVideoInfoTag* tag);
      explicit InfoTagVideo(CVideoInfoTag* tag, bool offscreen = false);
#endif
      explicit InfoTagVideo(bool offscreen = false);
      ~InfoTagVideo() override;

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getDbId() }
      /// Get identification number of tag in database
      ///
      /// @return [integer] database id
      ///
      ///
      ///-----------------------------------------------------------------------
      /// @python_v17 New function added.
      ///
      getDbId();
#else
      int getDbId();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getDirector() }
      /// Get [film director](https://en.wikipedia.org/wiki/Film_director)
      /// who has made the film (if present).
      ///
      /// @return [string] Film director name.
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getDirector();
#else
      String getDirector();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getWritingCredits() }
      /// Get the writing credits if present from video info tag.
      ///
      /// @return [string] Writing credits
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getWritingCredits();
#else
      String getWritingCredits();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getGenre() }
      /// To get the [Video Genre](https://en.wikipedia.org/wiki/Film_genre)
      /// if available.
      ///
      /// @return [string] Genre name
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getGenre();
#else
      String getGenre();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getTagLine() }
      /// Get video tag line if available.
      ///
      /// @return [string] Video tag line
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getTagLine();
#else
      String getTagLine();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getPlotOutline() }
      /// Get the outline plot of the video if present.
      ///
      /// @return [string] Outline plot
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getPlotOutline();
#else
      String getPlotOutline();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getPlot() }
      /// Get the plot of the video if present.
      ///
      /// @return [string] Plot
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getPlot();
#else
      String getPlot();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getPictureURL() }
      /// Get a picture URL of the video to show as screenshot.
      ///
      /// @return [string] Picture URL
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getPictureURL();
#else
      String getPictureURL();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getTitle() }
      /// Get the video title.
      ///
      /// @return [string] Video title
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getTitle();
#else
      String getTitle();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getTVShowTitle() }
      /// Get the video TV show title.
      ///
      /// @return [string] TV show title
      ///
      ///
      ///-----------------------------------------------------------------------
      /// @python_v17 New function added.
      ///
      getTVShowTitle();
#else
      String getTVShowTitle();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getMediaType() }
      /// Get the media type of the video.
      ///
      /// @return [string] media type
      ///
      /// Available strings about media type for video:
      /// | String         | Description                                       |
      /// |---------------:|:--------------------------------------------------|
      /// | video          | For normal video
      /// | set            | For a selection of video
      /// | musicvideo     | To define it as music video
      /// | movie          | To define it as normal movie
      /// | tvshow         | If this is it defined as tvshow
      /// | season         | The type is used as a series season
      /// | episode        | The type is used as a series episode
      ///
      ///-----------------------------------------------------------------------
      /// @python_v17 New function added.
      ///
      getMediaType();
#else
      String getMediaType();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getVotes() }
      /// Get the video votes if available from video info tag.
      ///
      /// @return [string] Votes
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getVotes();
#else
      String getVotes();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getCast() }
      /// To get the cast of the video when available.
      ///
      /// @return [string] Video casts
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getCast();
#else
      String getCast();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getFile() }
      /// To get the video file name.
      ///
      /// @return [string] File name
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getFile();
#else
      String getFile();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getPath() }
      /// To get the path where the video is stored.
      ///
      /// @return [string] Path
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getPath();
#else
      String getPath();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getFilenameAndPath() }
      /// To get the full path with filename where the video is stored.
      ///
      /// @return [string] File name and Path
      ///
      ///
      ///-----------------------------------------------------------------------
      /// @python_v19 New function added.
      ///
      getFilenameAndPath();
#else
      String getFilenameAndPath();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getIMDBNumber() }
      /// To get the [IMDb](https://en.wikipedia.org/wiki/Internet_Movie_Database)
      /// number of the video (if present).
      ///
      /// @return [string] IMDb number
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getIMDBNumber();
#else
      String getIMDBNumber();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getSeason() }
      /// To get season number of a series
      ///
      /// @return [integer] season number
      ///
      ///
      ///-----------------------------------------------------------------------
      /// @python_v17 New function added.
      ///
      getSeason();
#else
      int getSeason();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getEpisode() }
      /// To get episode number of a series
      ///
      /// @return [integer] episode number
      ///
      ///
      ///-----------------------------------------------------------------------
      /// @python_v17 New function added.
      ///
      getEpisode();
#else
      int getEpisode();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getYear() }
      /// Get production year of video if present.
      ///
      /// @return [integer] Production Year
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getYear();
#else
      int getYear();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getRating(type) }
      /// Get the video rating if present as float (double where supported).
      ///
      /// @param type           [opt] string - the type of the rating.
      /// - Some rating type values (any string possible):
      ///  | Label         | Type                                             |
      ///  |---------------|--------------------------------------------------|
      ///  | imdb          | string - type name
      ///  | tvdb          | string - type name
      ///  | tmdb          | string - type name
      ///  | anidb         | string - type name
      ///
      /// @return [float] The rating of the video
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getRating(type);
#else
      double getRating(const String& type = "");
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getUserRating() }
      /// Get the user rating if present as integer.
      ///
      /// @return [integer] The user rating of the video
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getUserRating();
#else
      int getUserRating();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getPlayCount() }
      /// To get the number of plays of the video.
      ///
      /// @return [integer] Play Count
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getPlayCount();
#else
      int getPlayCount();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getLastPlayed() }
      /// Get the last played date / time as string.
      ///
      /// @return [string] Last played date / time
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getLastPlayed();
#else
      String getLastPlayed();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getOriginalTitle() }
      /// To get the original title of the video.
      ///
      /// @return [string] Original title
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getOriginalTitle();
#else
      String getOriginalTitle();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getPremiered() }
      /// To get [premiered](https://en.wikipedia.org/wiki/Premiere) date
      /// of the video, if available.
      ///
      /// @return [string]
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getPremiered();
#else
      String getPremiered();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getFirstAired() }
      /// Returns first aired date as string from info tag.
      ///
      /// @return [string] First aired date
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      ///
      getFirstAired();
#else
      String getFirstAired();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getTrailer() }
      /// To get the path where the trailer is stored.
      ///
      /// @return [string] Trailer path
      ///
      ///
      ///-----------------------------------------------------------------------
      /// @python_v17 New function added.
      ///
      getTrailer();
#else
      String getTrailer();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getArtist() }
      /// To get the artist name (for musicvideos)
      ///
      /// @return [std::vector<std::string>] Artist name
      ///
      ///
      ///-----------------------------------------------------------------------
      /// @python_v18 New function added.
      ///
      getArtist();
#else
      std::vector<std::string> getArtist();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getAlbum() }
      /// To get the album name (for musicvideos)
      ///
      /// @return [string] Album name
      ///
      ///
      ///-----------------------------------------------------------------------
      /// @python_v18 New function added.
      ///
      getAlbum();
#else
      String getAlbum();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getTrack() }
      /// To get the track number (for musicvideos)
      ///
      /// @return [int] Track number
      ///
      ///
      ///-----------------------------------------------------------------------
      /// @python_v18 New function added.
      ///
      getTrack();
#else
      int getTrack();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getDuration() }
      /// To get the duration
      ///
      /// @return [unsigned int] Duration
      ///
      ///
      ///-----------------------------------------------------------------------
      /// @python_v18 New function added.
      ///
      getDuration();
#else
      unsigned int getDuration();
#endif

      // TODO(Montellese)
      double getResumeTime();
      double getResumeTimeTotal();

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ getUniqueID(key) }
      ///-----------------------------------------------------------------------
      /// Get the unique ID of the given key.
      /// A unique ID is an identifier used by a (online) video database used to
      /// identify a video in its database.
      ///
      /// @param key            string - uniqueID name.
      /// - Some default uniqueID values (any string possible):
      ///  | Label         | Type                                             |
      ///  |---------------|--------------------------------------------------|
      ///  | imdb          | string - uniqueid name
      ///  | tvdb          | string - uniqueid name
      ///  | tmdb          | string - uniqueid name
      ///  | anidb         | string - uniqueid name
      ///
      ///
      ///
      ///-----------------------------------------------------------------------
      /// @python_v20 New function added.
      ///
      getUniqueID(key);
#else
      String getUniqueID(const char* key);
#endif

      // TODO(Montellese)
      void setUniqueID(const String& uniqueID, const String& type = "", bool isDefault = false);

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ setUniqueIDs(values, defaultUniqueID) }
      ///-----------------------------------------------------------------------
      /// Set the given unique IDs.
      /// A unique ID is an identifier used by a (online) video database used to
      /// identify a video in its database.
      ///
      /// @param values             dictionary - pairs of `{ 'label': 'value' }`.
      /// @param defaultUniqueID    [opt] string - the name of default uniqueID.
      ///
      ///  - Some example values (any string possible):
      ///  | Label         | Type                                              |
      ///  |:-------------:|:--------------------------------------------------|
      ///  | imdb          | string - uniqueid name
      ///  | tvdb          | string - uniqueid name
      ///  | tmdb          | string - uniqueid name
      ///  | anidb         | string - uniqueid name
      ///
      ///
      ///
      ///-----------------------------------------------------------------------
      /// @python_v20 New function added.
      ///
      setUniqueIDs(...);
#else
      void setUniqueIDs(const std::map<String, String>& uniqueIDs,
                        const String& defaultUniqueID = "");
#endif

      // TODO(Montellese)
      void setDbId(int dbId);
      void setYear(int year);
      void setEpisode(int episode);
      void setSeason(int season);
      void setSortEpisode(int sortEpisode);
      void setSortSeason(int sortSeason);
      void setEpisodeGuide(const String& episodeGuide);
      void setTop250(int top250);
      void setSetId(int setId);
      void setTrackNumber(int trackNumber);
      void setRating(float rating, int votes = 0, const String& type = "", bool isDefault = false);
      void setRatings(const std::map<String, Tuple<float, int>>& ratings,
                      const String& defaultRating = "");
      void setUserRating(int userRating);
      void setPlaycount(int playcount);
      void setMpaa(const String& mpaa);
      void setPlot(const String& plot);
      void setPlotOutline(const String& plotOutline);
      void setTitle(const String& title);
      void setOriginalTitle(const String& originalTitle);
      void setSortTitle(const String& sortTitle);
      void setTagLine(const String& tagLine);
      void setTvShowTitle(const String& tvshowTitle);
      void setTvShowStatus(const String& tvshowStatus);
      void setGenre(std::vector<String> genre);
      void setCountries(std::vector<String> countries);
      void setDirectors(std::vector<String> directors);
      void setStudios(std::vector<String> studios);
      void setWriters(std::vector<String> writers);
      void setDuration(int duration);
      void setPremiered(const String& premiered);
      void setSet(const String& set);
      void setSetOverview(const String& setOverview);
      void setTags(std::vector<String> tags);
      void setProductionCode(const String& productionCode);
      void setFirstAired(const String& firstAired);
      void setLastPlayed(const String& lastPlayed);
      void setAlbum(const String& album);
      void setVotes(int votes);
      void setTrailer(const String& trailer);
      void setPath(const String& path);
      void setFilenameAndPath(const String& filenameAndPath);
      void setIMDBNumber(const String& imdbNumber);
      void setDateAdded(const String& dateAdded);
      void setMediaType(const String& mediaType);
      void setShowLinks(std::vector<String> showLinks);
      void setArtists(std::vector<String> artists);
      void setCast(std::vector<const Actor*> actors);
      void setResumePoint(double time, double totalTime = 0.0);


#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ addSeason(number, name = "") }
      /// Add a season with name. It needs at least the season number.
      ///
      /// @param number     int - the number of the season.
      /// @param name       string - the name of the season. Default "".
      ///
      ///
      ///-----------------------------------------------------------------------
      ///
      /// @python_v20 New function added.
      ///
      /// **Example:**
      /// ~~~~~~~~~~~~~{.py}
      /// ...
      /// # addSeason(number, name))
      /// infotagvideo.addSeason(1, "Murder House")
      /// ...
      /// ~~~~~~~~~~~~~
      ///
      addSeason(...);
#else
      void addSeason(int number, std::string name = "");
#endif

      // TODO(Montellese)
      void addSeasons(std::vector<Tuple<int, std::string>> namedSeasons);

      void addVideoStream(const VideoStreamDetail* stream);
      void addAudioStream(const AudioStreamDetail* stream);
      void addSubtitleStream(const SubtitleStreamDetail* stream);

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ setAvailableFanart(fanart) }
      /// Set available images (needed for video scrapers)
      ///
      /// @param fanart            list of fanart
      ///
      ///
      ///-----------------------------------------------------------------------
      /// @python_v20 New function added.
      ///
      /// **Example:**
      /// ~~~~~~~~~~~~~{.py}
      /// ...
      /// fanart = [
      ///     xbmc.Fanart(image="http://www.someurl.com/someimage.png", preview="http://www.someurl.com/somepreviewimage.png", colors="FFFFFFFF,DDDDDDDD"),
      ///     xbmc.Fanart(image=path_to_image_2, preview=path_to_preview_2, colors="|68,69,59|69,70,58|78,78,68|")
      ///     ]
      /// infotagvideo.setAvailableFanart(fanart)
      /// ...
      /// ~~~~~~~~~~~~~
      ///
      setAvailableFanart(...);
#else
      void setAvailableFanart(const std::vector<const Fanart*>& fanart);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
      ///
      /// \ingroup python_InfoTagVideo
      /// @brief \python_func{ addAvailableArtwork(images) }
      /// Add an image to available artworks (needed for video scrapers)
      ///
      /// @param url            string - image path url
      /// @param art_type       string - image type
      /// @param preview        [opt] string - image preview path url
      /// @param referrer       [opt] string - referrer url
      /// @param cache          [opt] string - filename in cache
      /// @param post           [opt] bool - use post to retrieve the image (default false)
      /// @param isgz           [opt] bool - use gzip to retrieve the image (default false)
      /// @param season         [opt] integer - number of season in case of season thumb
      ///
      ///
      ///-----------------------------------------------------------------------
      /// @python_v20 New function added.
      ///
      /// **Example:**
      /// ~~~~~~~~~~~~~{.py}
      /// ...
      /// infotagvideo.addAvailableArtwork(path_to_image_1, "thumb")
      /// ...
      /// ~~~~~~~~~~~~~
      ///
      addAvailableArtwork(...);
#else
      void addAvailableArtwork(const std::string& url,
        const std::string& art_type = "",
        const std::string& preview = "",
        const std::string& referrer = "",
        const std::string& cache = "",
        bool post = false,
        bool isgz = false,
        int season = -1);
#endif

#ifndef SWIG
      static void setDbIdRaw(CVideoInfoTag* infoTag, int dbId);
      static void setUniqueIDRaw(CVideoInfoTag* infoTag,
                                 const String& uniqueID,
                                 const String& type = "",
                                 bool isDefault = false);
      static void setUniqueIDsRaw(CVideoInfoTag* infoTag,
                                  std::map<String, String> uniqueIDs,
                                  const String& defaultUniqueID = "");
      static void setYearRaw(CVideoInfoTag* infoTag, int year);
      static void setEpisodeRaw(CVideoInfoTag* infoTag, int episode);
      static void setSeasonRaw(CVideoInfoTag* infoTag, int season);
      static void setSortEpisodeRaw(CVideoInfoTag* infoTag, int sortEpisode);
      static void setSortSeasonRaw(CVideoInfoTag* infoTag, int sortSeason);
      static void setEpisodeGuideRaw(CVideoInfoTag* infoTag, const String& episodeGuide);
      static void setTop250Raw(CVideoInfoTag* infoTag, int top250);
      static void setSetIdRaw(CVideoInfoTag* infoTag, int setId);
      static void setTrackNumberRaw(CVideoInfoTag* infoTag, int trackNumber);
      static void setRatingRaw(CVideoInfoTag* infoTag,
                               float rating,
                               int votes = 0,
                               std::string type = "",
                               bool isDefault = false);
      static void setRatingsRaw(CVideoInfoTag* infoTag,
                                const std::map<String, Tuple<float, int>>& ratings,
                                const String& defaultRating = "");
      static void setUserRatingRaw(CVideoInfoTag* infoTag, int userRating);
      static void setPlaycountRaw(CVideoInfoTag* infoTag, int playcount);
      static void setMpaaRaw(CVideoInfoTag* infoTag, const String& mpaa);
      static void setPlotRaw(CVideoInfoTag* infoTag, const String& plot);
      static void setPlotOutlineRaw(CVideoInfoTag* infoTag, const String& plotOutline);
      static void setTitleRaw(CVideoInfoTag* infoTag, const String& title);
      static void setOriginalTitleRaw(CVideoInfoTag* infoTag, const String& originalTitle);
      static void setSortTitleRaw(CVideoInfoTag* infoTag, const String& sortTitle);
      static void setTagLineRaw(CVideoInfoTag* infoTag, const String& tagLine);
      static void setTvShowTitleRaw(CVideoInfoTag* infoTag, const String& tvshowTitle);
      static void setTvShowStatusRaw(CVideoInfoTag* infoTag, const String& tvshowStatus);
      static void setGenreRaw(CVideoInfoTag* infoTag, std::vector<String> genre);
      static void setCountriesRaw(CVideoInfoTag* infoTag, std::vector<String> countries);
      static void setDirectorsRaw(CVideoInfoTag* infoTag, std::vector<String> directors);
      static void setStudiosRaw(CVideoInfoTag* infoTag, std::vector<String> studios);
      static void setWritersRaw(CVideoInfoTag* infoTag, std::vector<String> writers);
      static void setDurationRaw(CVideoInfoTag* infoTag, int duration);
      static void setPremieredRaw(CVideoInfoTag* infoTag, const String& premiered);
      static void setSetRaw(CVideoInfoTag* infoTag, const String& set);
      static void setSetOverviewRaw(CVideoInfoTag* infoTag, const String& setOverview);
      static void setTagsRaw(CVideoInfoTag* infoTag, std::vector<String> tags);
      static void setProductionCodeRaw(CVideoInfoTag* infoTag, const String& productionCode);
      static void setFirstAiredRaw(CVideoInfoTag* infoTag, const String& firstAired);
      static void setLastPlayedRaw(CVideoInfoTag* infoTag, const String& lastPlayed);
      static void setAlbumRaw(CVideoInfoTag* infoTag, const String& album);
      static void setVotesRaw(CVideoInfoTag* infoTag, int votes);
      static void setTrailerRaw(CVideoInfoTag* infoTag, const String& trailer);
      static void setPathRaw(CVideoInfoTag* infoTag, const String& path);
      static void setFilenameAndPathRaw(CVideoInfoTag* infoTag, const String& filenameAndPath);
      static void setIMDBNumberRaw(CVideoInfoTag* infoTag, const String& imdbNumber);
      static void setDateAddedRaw(CVideoInfoTag* infoTag, const String& dateAdded);
      static void setMediaTypeRaw(CVideoInfoTag* infoTag, const String& mediaType);
      static void setShowLinksRaw(CVideoInfoTag* infoTag, std::vector<String> showLinks);
      static void setArtistsRaw(CVideoInfoTag* infoTag, std::vector<String> artists);
      static void setCastRaw(CVideoInfoTag* infoTag, std::vector<SActorInfo> cast);
      static void setResumePointRaw(CVideoInfoTag* infoTag, double time, double totalTime = 0.0);

      static void addSeasonRaw(CVideoInfoTag* infoTag, int number, std::string name = "");
      static void addSeasonsRaw(CVideoInfoTag* infoTag,
                                std::vector<Tuple<int, std::string>> namedSeasons);

      static void addStreamRaw(CVideoInfoTag* infoTag, CStreamDetail* stream);
      static void finalizeStreamsRaw(CVideoInfoTag* infoTag);

      static void addAvailableArtworkRaw(CVideoInfoTag* infoTag,
                                         const std::string& url,
                                         const std::string& art_type = "",
                                         const std::string& preview = "",
                                         const std::string& referrer = "",
                                         const std::string& cache = "",
                                         bool post = false,
                                         bool isgz = false,
                                         int season = -1);
      static void prepareAvailableFanartRaw(CVideoInfoTag* infoTag);
      static void addAvailableFanartRaw(CVideoInfoTag* infoTag,
                                        const std::string& image,
                                        const std::string& preview,
                                        const std::string& colors);
      static void finalizeAvailableFanartRaw(CVideoInfoTag* infoTag);
#endif
    };
  }
}
