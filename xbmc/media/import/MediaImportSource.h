#pragma once
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

#include <memory>
#include <set>
#include <string>

#include "XBDateTime.h"
#include "media/MediaType.h"
#include "settings/SettingControl.h"
#include "settings/lib/SettingDefinitions.h"
#include "settings/SettingsBase.h"

class CSettingSection;
class CSettingsManager;

class CMediaImportSource
{
public:
  explicit CMediaImportSource(const std::string& identifier,
    const std::string& basePath = "",
    const std::string& friendlyName = "",
    const std::string& iconUrl = "",
    const MediaTypes& availableMediaTypes = MediaTypes(),
    const CDateTime& lastSynced = CDateTime(),
    const std::string& settingValues = "");

  ~CMediaImportSource() = default;

  bool operator==(const CMediaImportSource &other) const;
  bool operator!=(const CMediaImportSource &other) const { return !(*this == other); }

  const std::string& GetIdentifier() const { return m_identifier; }

  const std::string& GetBasePath() const { return m_basePath; }
  void SetBasePath(const std::string &basePath) { m_basePath = basePath; }

  const std::string& GetFriendlyName() const { return m_friendlyName; }
  void SetFriendlyName(const std::string &friendlyName) { m_friendlyName = friendlyName; }

  const std::string& GetIconUrl() const { return m_iconUrl; }
  void SetIconUrl(const std::string &iconUrl) { m_iconUrl = iconUrl; }

  const MediaTypes& GetAvailableMediaTypes() const { return m_availableMediaTypes; }
  void SetAvailableMediaTypes(const MediaTypes &mediaTypes) { m_availableMediaTypes = mediaTypes; }

  const CDateTime& GetLastSynced() const { return m_lastSynced; }
  void SetLastSynced(const CDateTime &lastSynced) { m_lastSynced = lastSynced; }

  bool IsActive() const { return m_active; }
  void SetActive(bool active) { m_active = active; }

  class CSettings : public CSettingsBase, public CSettingControlCreator
  {
    friend class CMediaImportSource;
  public:
    ~CSettings() = default;

    bool operator==(const CSettings &other) const;
    bool operator!=(const CSettings &other) const { return !(*this == other); }

    // implementations of CSettingsBase
    bool Load() override;
    bool Save() override;

    // specialization of CSettingsBase
    void Unload() override;

    void SetDefinition(const std::string& settingDefinition);

    void SetOptionsFiller(const std::string &settingId, IntegerSettingOptionsFiller optionsFiller, void* data = nullptr);
    void SetOptionsFiller(const std::string &settingId, StringSettingOptionsFiller optionsFiller, void* data = nullptr);

    std::string ToXml() const;

  protected:
    explicit CSettings(const std::string& settingValues = "");
    CSettings(const CSettings& other);

    CSettings& operator=(const CSettings& other);

    // implementation of CSettingsBase
    bool InitializeDefinitions() override;

    // specializations of CSettingsBase
    void InitializeControls() override;

    // hide methods of CSettingsBase
    using CSettingsBase::Initialize;
    using CSettingsBase::SetLoaded;
    using CSettingsBase::Uninitialize;

  private:
    mutable std::string m_settingValues;
    std::string m_settingDefinition;
  };

  const CSettings& Settings() const { return m_settings; }
  CSettings& Settings() { return m_settings; }

private:
  std::string m_identifier;
  std::string m_basePath;
  std::string m_friendlyName;
  std::string m_iconUrl;
  MediaTypes m_availableMediaTypes;
  CDateTime m_lastSynced;
  bool m_active;
  CSettings m_settings;
};
