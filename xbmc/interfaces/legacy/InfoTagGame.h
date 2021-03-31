/*
 *  Copyright (C) 2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "AddonClass.h"
#include "utils/Variant.h"
#include "games/tags/GameInfoTag.h"

namespace XBMCAddon
{
namespace xbmc
{
///
/// \defgroup python_InfoTagGame InfoTagGame
/// \ingroup python_xbmc
/// @{
/// @brief **Kodi's game info tag class.**
///
/// \python_class{ InfoTagGame() }
///
/// TODO(Montellese)
///
///-------------------------------------------------------------------------
///
/// **Example:**
/// ~~~~~~~~~~~~~{.py}
/// ...
/// TODO(Montellese)
/// tag = xbmc.Player().getVideoInfoTag()
///
/// title = tag.getTitle()
/// file  = tag.getFile()
/// ...
/// ~~~~~~~~~~~~~
///
class InfoTagGame : public AddonClass
{
 private:
  KODI::GAME::CGameInfoTag* infoTag;
  bool offscreen;
  bool owned;

 public:
#ifndef SWIG
  explicit InfoTagGame(const KODI::GAME::CGameInfoTag* tag);
  explicit InfoTagGame(KODI::GAME::CGameInfoTag* tag, bool offscreen = false);
#endif
  explicit InfoTagGame(bool offscreen = false);
  ~InfoTagGame() override;

  /* TODO(Montellese)
#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_InfoTagGame
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
  /// \ingroup python_InfoTagGame
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
  // TODO(Montellese)
  String getDirector();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_InfoTagGame
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

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_InfoTagGame
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

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_InfoTagGame
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
  */

  void setInfo(const CVariant& info);

#ifndef SWIG
  /* TODO(Montellese)
  static void setDbIdRaw(KODI::GAME::CGameInfoTag* infoTag, int dbId);
  static void setUniqueIDRaw(KODI::GAME::CGameInfoTag* infoTag,
                              const String& uniqueID,
                              const String& type = "",
                              bool isDefault = false);
  static void setUniqueIDsRaw(KODI::GAME::CGameInfoTag* infoTag,
                              std::map<String, String> uniqueIDs,
                              const String& defaultUniqueID = "");
  */

  static void setInfoRaw(KODI::GAME::CGameInfoTag* infoTag, const CVariant& info);
#endif
};

}
}
