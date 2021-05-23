/*
 *  Copyright (C) 2017-2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "commons/Exception.h"
#include "interfaces/legacy/AddonClass.h"
#include "interfaces/legacy/AddonString.h"
#include "interfaces/legacy/Exception.h"
#include "interfaces/legacy/Tuple.h"
#include "settings/lib/ISettingCallback.h"
#include "settings/lib/SettingDefinitions.h"

#include <memory>
#include <string>
#include <vector>

namespace ADDON
{
class IAddonSettingsCallbackExecutor;
}

class CSettingsBase;

namespace XBMCAddon
{
namespace xbmcaddon
{

XBMCCOMMONS_STANDARD_EXCEPTION(SettingCallbacksNotSupportedException);

//
/// \defgroup python_settings Settings
/// \ingroup python_settings
/// @{
/// @brief **Add-on settings**
///
/// \python_class{ Settings() }
///
/// This wrapper provides access to the settings specific to an add-on.
/// It supports reading and writing specific setting values.
///
///-----------------------------------------------------------------------
/// @python_v20 New class added.
///
/// **Example:**
/// ~~~~~~~~~~~~~{.py}
/// ...
/// settings = xbmc.Addon('id').getSettings()
/// ...
/// ~~~~~~~~~~~~~
class Settings : public AddonClass
#ifndef SWIG
  , protected ISettingCallback
#endif
{
public:
#if !defined SWIG && !defined DOXYGEN_SHOULD_SKIP_THIS
  std::shared_ptr<CSettingsBase> settings;
#endif

#ifndef SWIG
  Settings(std::shared_ptr<CSettingsBase> settings,
           const std::string& addonId,
           ADDON::IAddonSettingsCallbackExecutor* callbackExecutor = nullptr,
           void* callbackData = nullptr);
#endif

  virtual ~Settings() = default;

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ load() }
  ///-----------------------------------------------------------------------
  /// Loads the setting definitions and any saved values.
  ///
  /// @return [bool] True if the settings were loaded.
  ///
  ///
  ///-----------------------------------------------------------------------
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// result = settings.load()
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  load(...);
#else
  bool load();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ save() }
  ///-----------------------------------------------------------------------
  /// Saves any changed values of the settings.
  ///
  /// @return [bool] True if the settings were saved.
  ///
  ///
  ///-----------------------------------------------------------------------
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// result = settings.save()
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  save(...);
#else
  bool save();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ setLoaded() }
  ///-----------------------------------------------------------------------
  /// Tells the core logic that the add-on has performed all necessary
  /// actions registered option fillers or action callbacks to load
  /// the settings logic.
  ///
  ///
  ///-----------------------------------------------------------------------
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// settings.setLoaded()
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  setLoaded(...);
#else
  void setLoaded();
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ getBool(id) }
  ///-----------------------------------------------------------------------
  /// Returns the value of a setting as a boolean.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @return                       bool - Setting as a boolean
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// enabled = settings.getBool('enabled')
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  getBool(...);
#else
  bool getBool(const char* id) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ getInt(id) }
  ///-----------------------------------------------------------------------
  /// Returns the value of a setting as an integer.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @return                       integer - Setting as an integer
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// max = settings.getInt('max')
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  getInt(...);
#else
  int getInt(const char* id) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ getNumber(id) }
  ///-----------------------------------------------------------------------
  /// Returns the value of a setting as a floating point number.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @return                       float - Setting as a floating point number
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// max = settings.getNumber('max')
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  getNumber(...);
#else
  double getNumber(const char* id) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ getString(id) }
  ///-----------------------------------------------------------------------
  /// Returns the value of a setting as a unicode string.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @return                       string - Setting as a unicode string
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// apikey = settings.getString('apikey')
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  getString(...);
#else
  String getString(const char* id) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ getBoolList(id) }
  ///-----------------------------------------------------------------------
  /// Returns the value of a setting as a list of booleans.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @return                       list - Setting as a list of booleans
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// enabled = settings.getBoolList('enabled')
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  getBoolList(...);
#else
  std::vector<bool> getBoolList(const char* id) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ getIntList(id) }
  ///-----------------------------------------------------------------------
  /// Returns the value of a setting as a list of integers.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @return                       list - Setting as a list of integers
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// ids = settings.getIntList('ids')
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  getIntList(...);
#else
  std::vector<int> getIntList(const char* id) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ getNumberList(id) }
  ///-----------------------------------------------------------------------
  /// Returns the value of a setting as a list of floating point numbers.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @return                       list - Setting as a list of floating point numbers
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// max = settings.getNumberList('max')
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  getNumberList(...);
#else
  std::vector<double> getNumberList(const char* id) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ getStringList(id) }
  ///-----------------------------------------------------------------------
  /// Returns the value of a setting as a list of unicode strings.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @return                       list - Setting as a list of unicode strings
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// views = settings.getStringList('views')
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  getStringList(...);
#else
  std::vector<String> getStringList(const char* id) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ setBool(id, value) }
  ///-----------------------------------------------------------------------
  /// Sets the value of a setting.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @param value                  bool - value of the setting.
  /// @return                       bool - True if the value of the setting was set, false otherwise
  ///
  ///
  /// @note You can use the above as keywords for arguments.
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// settings.setBool(id='enabled', value=True)
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  setBool(...);
#else
  void setBool(const char* id, bool value) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ setInt(id, value) }
  ///-----------------------------------------------------------------------
  /// Sets the value of a setting.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @param value                  integer - value of the setting.
  /// @return                       bool - True if the value of the setting was set, false otherwise
  ///
  ///
  /// @note You can use the above as keywords for arguments.
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// settings.setInt(id='max', value=5)
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  setInt(...);
#else
  void setInt(const char* id, int value) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ setNumber(id, value) }
  ///-----------------------------------------------------------------------
  /// Sets the value of a setting.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @param value                  float - value of the setting.
  /// @return                       bool - True if the value of the setting was set, false otherwise
  ///
  ///
  /// @note You can use the above as keywords for arguments.
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// settings.setNumber(id='max', value=5.5)
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  setNumber(...);
#else
  void setNumber(const char* id, double value) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ setString(id, value) }
  ///-----------------------------------------------------------------------
  /// Sets the value of a setting.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @param value                  string or unicode - value of the setting.
  /// @return                       bool - True if the value of the setting was set, false otherwise
  ///
  ///
  /// @note You can use the above as keywords for arguments.
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// settings.setString(id='username', value='teamkodi')
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  setString(...);
#else
  void setString(const char* id, const String& value) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ setBoolList(id, values) }
  ///-----------------------------------------------------------------------
  /// Sets the boolean values of a list setting.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @param values                 list of boolean - values of the setting.
  /// @return                       bool - True if the values of the setting were set, false otherwise
  ///
  ///
  /// @note You can use the above as keywords for arguments.
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// settings.setBoolList(id='enabled', values=[ True, False ])
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  setBoolList(...);
#else
  void setBoolList(const char* id,
                   const std::vector<bool>& values) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ setIntList(id, value) }
  ///-----------------------------------------------------------------------
  /// Sets the integer values of a list setting.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @param values                 list of int - values of the setting.
  /// @return                       bool - True if the values of the setting were set, false otherwise
  ///
  ///
  /// @note You can use the above as keywords for arguments.
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// settings.setIntList(id='max', values=[ 5, 23 ])
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  setIntList(...);
#else
  void setIntList(const char* id,
                  const std::vector<int>& values) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ setNumberList(id, value) }
  ///-----------------------------------------------------------------------
  /// Sets the floating point values of a list setting.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @param values                 list of float - values of the setting.
  /// @return                       bool - True if the values of the setting were set, false otherwise
  ///
  ///
  /// @note You can use the above as keywords for arguments.
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// settings.setNumberList(id='max', values=[ 5.5, 5.8 ])
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  setNumberList(...);
#else
  void setNumberList(const char* id,
                     const std::vector<double>& values) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ setStringList(id, value) }
  ///-----------------------------------------------------------------------
  /// Sets the string values of a list setting.
  ///
  /// @param id                     string - id of the setting that the module needs to access.
  /// @param values                 list of string or unicode - values of the setting.
  /// @return                       bool - True if the values of the setting were set, false otherwise
  ///
  ///
  /// @note You can use the above as keywords for arguments.
  ///
  ///
  ///-----------------------------------------------------------------------
  /// @python_v20 New function added.
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// settings.setStringList(id='username', values=[ 'team', 'kodi' ])
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  setStringList(...);
#else
  void setStringList(const char* id,
                     const std::vector<String>& values) throw(XBMCAddon::WrongTypeException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ registerActionCallback(settingId, callback) }
  ///-----------------------------------------------------------------------
  /// Register an action callback for the given setting which is called when
  /// the action setting is triggered.
  ///
  /// @param settingId              string - id of the setting.
  /// @param callback               string - name of the callback to execute.
  /// @return                       bool - True if the callback is registered, false otherwise.
  ///
  ///
  /// @note You can use the above as keywords for arguments.
  ///
  ///
  ///-----------------------------------------------------------------------
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// settings.registerActionCallback('foo', 'onSettingAction')
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  registerActionCallback(...);
#else
  bool registerActionCallback(const String& settingId,
                              const String& callback) throw(SettingCallbacksNotSupportedException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ registerOptionsFillerCallback(settingId, callback) }
  ///-----------------------------------------------------------------------
  /// Register an options filler callback for the given setting which is called
  /// when the possible options of the setting are required.
  ///
  /// @param settingId              string - id of the setting.
  /// @param callback               string - name of the callback to execute.
  /// @return                       bool - True if the callback is registered, false otherwise.
  ///
  ///
  ///-----------------------------------------------------------------------
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// settings.registerActionCallback('foo', 'fooOptionFiller')
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  registerOptionsFillerCallback(...);
#else
  bool registerOptionsFillerCallback(const String& settingId, const String& callback) throw(
      SettingCallbacksNotSupportedException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ setIntegerOptions(settingId, options) }
  ///-----------------------------------------------------------------------
  /// Set the given options for the given integer settings from a registered
  /// options filler callback.
  ///
  /// @param settingId              string - id of the integer setting.
  /// @param options                list - List of tuple containing the label and integer value of the option.
  /// @return                       bool - True if the options have been set, false otherwise.
  ///
  ///
  ///-----------------------------------------------------------------------
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// result = setIntegerOptions('foo', [('Zero', 0), ('One', 1), ('Two', 2)])
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  setIntegerOptions(...);
#else
  bool setIntegerOptions(
      const String& settingId,
      const std::vector<Tuple<String, int>>& options) throw(SettingCallbacksNotSupportedException);
#endif

#ifdef DOXYGEN_SHOULD_USE_THIS
  ///
  /// \ingroup python_settings
  /// @brief \python_func{ setStringOptions(settingId, options) }
  ///-----------------------------------------------------------------------
  /// Set the given options for the given string settings from a registered
  /// options filler callback.
  ///
  /// @param settingId              string - id of the string setting.
  /// @param options                list - List of tuple containing the label and string value of the option.
  /// @return                       bool - True if the options have been set, false otherwise.
  ///
  ///
  ///-----------------------------------------------------------------------
  ///
  /// **Example:**
  /// ~~~~~~~~~~~~~{.py}
  /// ..
  /// result = setStringOptions('foo', [('Hello', 'hello'), ('World', 'world')])
  /// ..
  /// ~~~~~~~~~~~~~
  ///
  setStringOptions(...);
#else
  bool setStringOptions(const String& settingId,
                        const std::vector<Tuple<String, String>>&
                            options) throw(SettingCallbacksNotSupportedException);
#endif

#ifndef SWIG
protected:
  void OnSettingAction(const std::shared_ptr<const CSetting>& setting) override;

private:
  using CallbackMap = std::map<std::string, std::string>;

  std::shared_ptr<CSetting> GetSetting(const std::string& settingId) const;

  static void IntegerSettingOptionsFiller(const std::shared_ptr<const CSetting>& setting,
                                          IntegerSettingOptions& list,
                                          int& current,
                                          void* data);
  static void StringSettingOptionsFiller(const std::shared_ptr<const CSetting>& setting,
                                         StringSettingOptions& list,
                                         std::string& current,
                                         void* data);

  const std::string m_addonId;

  ADDON::IAddonSettingsCallbackExecutor* m_callbackExecutor;
  void* m_callbackData;
  CallbackMap m_actionCallbacks;
  CallbackMap m_optionsFillerCallbacks;
#endif
};

} // namespace xbmcaddon
} // namespace XBMCAddon
