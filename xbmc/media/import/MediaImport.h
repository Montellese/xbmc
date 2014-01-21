/*
 *  Copyright (C) 2013-2019 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "XBDateTime.h"
#include "media/MediaType.h"
#include "media/import/MediaImportSource.h"

#include <memory>
#include <set>
#include <string>

enum class MediaImportTrigger
{
  Auto = 0,
  Manual = 1
};

class CMediaImportSettings : public CMediaImportSettingsBase
{
public:
  explicit CMediaImportSettings(const GroupedMediaTypes& mediaTypes,
                                const std::string& settingValues = "");
  CMediaImportSettings(const CMediaImportSettings& other);
  virtual ~CMediaImportSettings() = default;

  MediaImportTrigger GetImportTrigger() const;
  bool SetImportTrigger(MediaImportTrigger importTrigger);
  bool UpdateImportedMediaItems() const;
  bool SetUpdateImportedMediaItems(bool updateImportedMediaItems);
  bool UpdatePlaybackMetadataFromSource() const;
  bool SetUpdatePlaybackMetadataFromSource(bool updatePlaybackMetadataFromSource);
  bool UpdatePlaybackMetadataOnSource() const;
  bool SetUpdatePlaybackMetadataOnSource(bool updatePlaybackMetadataOnSource);

  static const std::string SettingTrigger;
  static const std::string SettingTriggerValueAuto;
  static const std::string SettingTriggerValueManual;
  static const std::string SettingUpdateItems;
  static const std::string SettingUpdatePlaybackMetadataFromSource;
  static const std::string SettingUpdatePlaybackMetadataOnSource;

private:
  static const std::string SettingsDefinition;

  static const std::string SettingConditionHasMediaType;

  void Setup();

  static bool HasMediaType(const std::string& condition,
                           const std::string& value,
                           const std::shared_ptr<const CSetting>& setting,
                           void* data);

  const GroupedMediaTypes m_mediaTypes;
};

using MediaImportSettingsPtr = std::shared_ptr<CMediaImportSettings>;
using MediaImportSettingsConstPtr = std::shared_ptr<const CMediaImportSettings>;

class CMediaImport
{
public:
  explicit CMediaImport(const GroupedMediaTypes& mediaTypes = {},
                        const std::string& sourceIdentifier = "");
  CMediaImport(const GroupedMediaTypes& mediaTypes, const CMediaImportSource& source);
  CMediaImport(const GroupedMediaTypes& mediaTypes,
               const CMediaImportSource& source,
               const CDateTime& lastSynced,
               const std::string& settingValues);
  CMediaImport(const CMediaImport& other);

  ~CMediaImport() = default;

  bool operator==(const CMediaImport& other) const;
  bool operator!=(const CMediaImport& other) const { return !(*this == other); }

  CMediaImport Clone() const;

  bool IsValid() const { return !m_mediaTypes.empty() && m_source.IsValid(); }

  CMediaImportSource& GetSource() { return m_source; }
  const CMediaImportSource& GetSource() const { return m_source; }
  void SetSource(const CMediaImportSource& source)
  {
    if (source.GetIdentifier().empty())
      return;

    m_source = source;
  }

  const GroupedMediaTypes& GetMediaTypes() const { return m_mediaTypes; }
  std::string GetMediaTypesAsString() const;
  void SetMediaTypes(const GroupedMediaTypes& mediaTypes) { m_mediaTypes = mediaTypes; }
  bool ContainsMediaType(const GroupedMediaTypes::value_type mediaType) const;

  const CDateTime& GetLastSynced() const { return m_lastSynced; }
  void SetLastSynced(const CDateTime& lastSynced)
  {
    m_lastSynced = lastSynced;
    m_source.SetLastSynced(lastSynced);
  }

  MediaImportSettingsConstPtr Settings() const { return m_settings; }
  MediaImportSettingsPtr Settings() { return m_settings; }

  bool IsActive() const { return m_source.IsActive(); }
  void SetActive(bool active) { m_source.SetActive(active); }

  bool IsReady() const { return m_source.IsReady(); }
  void SetReady(bool ready) { m_source.SetReady(ready); }

  friend std::ostream& operator<<(std::ostream& os, const CMediaImport& import);

private:
  GroupedMediaTypes m_mediaTypes;
  CMediaImportSource m_source;
  CDateTime m_lastSynced;
  MediaImportSettingsPtr m_settings;
};

using MediaImportPtr = std::shared_ptr<CMediaImport>;
