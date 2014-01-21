#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
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

#include <set>
#include <string>

#include "XBDateTime.h"
#include "media/MediaType.h"
#include "media/import/MediaImportSource.h"

typedef enum MediaImportTrigger
{
  MediaImportTriggerAuto = 0,
  MediaImportTriggerManual = 1
} MediaImportTrigger;

class CMediaImportSettings
{
public:
  CMediaImportSettings()
    : m_importTrigger(MediaImportTriggerAuto)
    , m_updateImportedMediaItems(true)
    , m_updatePlaybackMetadataFromSource(true)
    , m_updatePlaybackMetadataOnSource(true)
  { }

  std::string Serialize() const;
  bool Deserialize(const std::string &xmlData);

  MediaImportTrigger GetImportTrigger() const { return m_importTrigger; }
  void SetImportTrigger(MediaImportTrigger importTrigger) { m_importTrigger = importTrigger; }
  bool UpdateImportedMediaItems() const { return m_updateImportedMediaItems; }
  void SetUpdateImportedMediaItems(bool updateImportedMediaItems) { m_updateImportedMediaItems = updateImportedMediaItems; }
  bool UpdatePlaybackMetadataFromSource() const { return m_updatePlaybackMetadataFromSource; }
  void SetUpdatePlaybackMetadataFromSource(bool updatePlaybackMetadataFromSource) { m_updatePlaybackMetadataFromSource = updatePlaybackMetadataFromSource; }
  bool UpdatePlaybackMetadataOnSource() const { return m_updatePlaybackMetadataOnSource; }
  void SetUpdatePlaybackMetadataOnSource(bool updatePlaybackMetadataOnSource) { m_updatePlaybackMetadataOnSource = updatePlaybackMetadataOnSource; }

private:
  MediaImportTrigger m_importTrigger;
  bool m_updateImportedMediaItems;
  bool m_updatePlaybackMetadataFromSource;
  bool m_updatePlaybackMetadataOnSource;
};

class CMediaImport
{
public:
  explicit CMediaImport(const std::string& importPath = "")
    : CMediaImport(importPath, CMediaImportSource(importPath))
  { }
  CMediaImport(const std::string& importPath, const CMediaImportSource& source)
    : CMediaImport(importPath, source, {}, true, CDateTime())
  { }

  static CMediaImport CreateRecursive(const std::string& importPath, const GroupedMediaTypes& importedMediaTypes,
                                      const CMediaImportSource& source, const CDateTime& lastSynced = CDateTime())
  {
    return CMediaImport(importPath, source, importedMediaTypes, true, lastSynced);
  }
  static CMediaImport CreateSelective(const std::string& importPath,
                                      const GroupedMediaTypes& importedMediaTypes,
                                      const CMediaImportSource& source, const CDateTime& lastSynced = CDateTime())
  {
    return CMediaImport(importPath, source, importedMediaTypes, false, lastSynced);
  }

  bool operator==(const CMediaImport& other) const
  {
    if (m_importPath.compare(other.m_importPath) != 0 ||
        m_source != other.m_source ||
        m_mediaTypes != other.m_mediaTypes ||
        m_recursive != other.m_recursive ||
        m_lastSynced != other.m_lastSynced)
      return false;

    return true;
  }
  bool operator!=(const CMediaImport& other) const { return !(*this == other); }

  const std::string& GetPath() const { return m_importPath; }

  const CMediaImportSource& GetSource() const { return m_source; }
  void SetSource(const CMediaImportSource& source)
  {
    if (source.GetIdentifier().empty())
      return;

    m_source = source;
  }

  const GroupedMediaTypes& GetMediaTypes() const { return m_mediaTypes; }
  void SetMediaTypes(const GroupedMediaTypes& mediaTypes) { m_mediaTypes = mediaTypes; }

  bool IsRecursive() const { return m_recursive; }
  void SetRecursive(bool recursive) { m_recursive = recursive; }

  const CDateTime& GetLastSynced() const { return m_lastSynced; }
  void SetLastSynced(const CDateTime& lastSynced)
  {
    m_lastSynced = lastSynced;
    m_source.SetLastSynced(lastSynced);
  }

  bool IsActive() const { return m_source.IsActive(); }
  void SetActive(bool active) { m_source.SetActive(active); }
  
  const CMediaImportSettings& GetSettings() const { return m_settings; }
  CMediaImportSettings& GetSettings() { return m_settings; }

private:
  CMediaImport(const std::string& importPath, const CMediaImportSource& source, 
    const GroupedMediaTypes& importedMediaTypes,
    bool recursive, const CDateTime& lastSynced)
    : m_importPath(importPath)
    , m_mediaTypes(importedMediaTypes)
    , m_source(source)
    , m_recursive(recursive)
    , m_lastSynced(lastSynced)
  { }

  std::string m_importPath;
  GroupedMediaTypes m_mediaTypes;
  CMediaImportSource m_source;
  bool m_recursive;
  CDateTime m_lastSynced;
  CMediaImportSettings m_settings;
};
