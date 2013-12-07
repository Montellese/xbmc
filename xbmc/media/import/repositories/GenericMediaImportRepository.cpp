/*
 *      Copyright (C) 2014 Team XBMC
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

#include "GenericMediaImportRepository.h"
#include "FileItem.h"
#include "dialogs/GUIDialogProgress.h"
#include "guilib/LocalizeStrings.h"
#include "threads/SingleLock.h"
#include "utils/StringUtils.h"

CGenericMediaImportRepository::~CGenericMediaImportRepository()
{
  if (!m_loaded)
    return;

  m_sources.clear();
  m_imports.clear();
}

bool CGenericMediaImportRepository::Initialize()
{
  if (m_loaded)
    return true;

  if (!openRepository())
    return false;

  std::vector<CMediaImportSource> sources = getSources();
  std::vector<CMediaImport> imports = getImports();
  closeRepository();
  
  CSingleLock sourcesLock(m_sourcesLock);
  for (const auto& it : sources)
    m_sources.insert(std::make_pair(it.GetIdentifier(), it));

  {
    CSingleLock importsLock(m_importsLock);
    for (auto& it : imports)
    {
      if (it.GetSource().GetIdentifier().empty())
        continue;

      const auto& itSource = m_sources.find(it.GetSource().GetIdentifier());
      if (itSource == m_sources.end())
        continue;

      it.SetSource(itSource->second);
      m_imports.insert(std::make_pair(GetMediaImportIdentifier(it), it));
    }
  }

  m_loaded = true;
  return true;
}

std::vector<CMediaImport> CGenericMediaImportRepository::GetImports() const
{
  std::vector<CMediaImport> imports;

  if (!m_loaded)
    return imports;

  CSingleLock importsLock(m_importsLock);
  for (const auto& it : m_imports)
    imports.push_back(it.second);

  return imports;
}

std::vector<CMediaImport> CGenericMediaImportRepository::GetImportsBySource(const std::string &sourceIdentifier) const
{
  std::vector<CMediaImport> imports;

  if (!m_loaded || sourceIdentifier.empty())
    return imports;

  CSingleLock importsLock(m_importsLock);
  for (const auto& it : m_imports)
  {
    if (it.second.GetSource().GetIdentifier().compare(sourceIdentifier) == 0)
      imports.push_back(it.second);
  }

  return imports;
}

std::vector<CMediaImport> CGenericMediaImportRepository::GetImportsByMediaType(const GroupedMediaTypes &mediaTypes) const
{
  std::vector<CMediaImport> imports;

  if (!m_loaded || mediaTypes.empty())
    return imports;

  MediaTypes supportedMediaTypes = getSupportedMediaTypes();
  if (!ContainsAllMediaTypes(supportedMediaTypes, mediaTypes))
    return imports;

  CSingleLock importsLock(m_importsLock);
  for (const auto& it : m_imports)
  {
    if (it.first.second == mediaTypes)
      imports.push_back(it.second);
  }

  return imports;
}

bool CGenericMediaImportRepository::GetImport(const std::string &path, const GroupedMediaTypes &mediaTypes, CMediaImport &import) const
{
  if (!m_loaded ||
      path.empty() || mediaTypes.empty())
    return false;
  
  CSingleLock importsLock(m_importsLock);
  const auto& it = m_imports.find(MediaImportIdentifier(path, mediaTypes));
  if (it == m_imports.end())
    return false;

  import = it->second;

  return true;
}

bool CGenericMediaImportRepository::AddImport(const CMediaImport &import)
{
  if (!m_loaded ||
      import.GetSource().GetIdentifier().empty() || import.GetPath().empty() ||
      import.GetMediaTypes().empty())
    return false;

  MediaTypes supportedMediaTypes = getSupportedMediaTypes();
  if (!ContainsAllMediaTypes(supportedMediaTypes, import.GetMediaTypes()))
    return false;

  MediaImportIdentifier importIdentifier = GetMediaImportIdentifier(import);

  CSingleLock importsLock(m_importsLock);
  auto&& itImport = m_imports.find(importIdentifier);
  if (itImport != m_imports.end() && itImport->second == import)
    return true;

  if (!openRepository())
    return false;

  if (itImport == m_imports.end())
  {
    int idImport = addImport(import);
    closeRepository();

    if (idImport < 0)
      return false;

    m_imports.insert(std::make_pair(importIdentifier, import));
  }
  else
  {
    bool ret = updateImport(import);
    closeRepository();

    if (!ret)
      return false;

    itImport->second = import;
  }

  return true;
}

bool CGenericMediaImportRepository::UpdateImport(const CMediaImport &import)
{
  if (!m_loaded ||
      import.GetSource().GetIdentifier().empty() || import.GetPath().empty() ||
      import.GetMediaTypes().empty())
    return false;

  CSingleLock importsLock(m_importsLock);
  auto&& itImport = m_imports.find(GetMediaImportIdentifier(import));
  if (itImport == m_imports.end())
    return false;

  if (!openRepository())
    return false;

  bool ret = updateImport(import);
  closeRepository();

  if (!ret)
    return false;

  itImport->second = import;

  return true;
}

bool CGenericMediaImportRepository::RemoveImport(const CMediaImport &import)
{
  if (!m_loaded ||
      import.GetPath().empty() || import.GetMediaTypes().empty())
    return false;

  CSingleLock importsLock(m_importsLock);
  auto&& itImport = m_imports.find(GetMediaImportIdentifier(import));
  if (itImport == m_imports.end())
    return false;

  if (!openRepository())
    return false;

  removeImport(import);
  closeRepository();

  m_imports.erase(itImport);

  return true;
}

bool CGenericMediaImportRepository::UpdateLastSync(CMediaImport &import)
{
  if (!m_loaded ||
      import.GetPath().empty() || import.GetMediaTypes().empty())
    return false;

  CSingleLock importsLock(m_importsLock);
  auto&& itImport = m_imports.find(GetMediaImportIdentifier(import));
  if (itImport == m_imports.end())
    return false;

  if (!openRepository())
    return false;

  CDateTime lastSynced = CDateTime::GetCurrentDateTime();
  updateLastSync(import, lastSynced);
  closeRepository();

  // update the cached copy of the import
  itImport->second.SetLastSynced(lastSynced);
  importsLock.Leave();

  // update the cached copy of the import's source
  CSingleLock sourcesLock(m_sourcesLock);
  auto&& itSource = m_sources.find(import.GetSource().GetIdentifier());
  if (itSource != m_sources.end())
    itSource->second.SetLastSynced(lastSynced);

  // update the local copy of the import and the source
  import.SetLastSynced(lastSynced);

  return true;
}

std::vector<CMediaImportSource> CGenericMediaImportRepository::GetSources(const GroupedMediaTypes &mediaTypes /* = GroupedMediaTypes() */) const
{
  std::vector<CMediaImportSource> sources;

  if (!m_loaded)
    return sources;

  if (!mediaTypes.empty())
  {
    MediaTypes supportedMediaTypes = getSupportedMediaTypes();
    if (!ContainsAllMediaTypes(supportedMediaTypes, mediaTypes))
      return sources;
  }

  CSingleLock sourcesLock(m_sourcesLock);
  for (const auto& it : m_sources)
  {
    if (mediaTypes.empty() || ContainsAllMediaTypes(it.second.GetAvailableMediaTypes(), mediaTypes))
      sources.push_back(it.second);
  }

  return sources;
}

bool CGenericMediaImportRepository::GetSource(const std::string &identifier, CMediaImportSource &source) const
{
  if (!m_loaded ||
      identifier.empty())
    return false;
  
  CSingleLock sourcesLock(m_sourcesLock);
  const auto& it = m_sources.find(identifier);
  if (it == m_sources.end())
    return false;

  source = it->second;

  return true;
}

bool CGenericMediaImportRepository::AddSource(const CMediaImportSource &source)
{
  if (!m_loaded ||
      source.GetIdentifier().empty() || source.GetFriendlyName().empty())
    return false;  

  MediaTypes supportedMediaTypes = getSupportedMediaTypes();
  bool supported = false;
  const MediaTypes availableMediaTypes = source.GetAvailableMediaTypes();
  for (const auto& mediaType : availableMediaTypes)
  {
    if (supportedMediaTypes.find(mediaType) != supportedMediaTypes.end())
    {
      supported = true;
      break;
    }
  }

  if (!supported)
    return false;
  
  CSingleLock sourcesLock(m_sourcesLock);
  auto&& itSource = m_sources.find(source.GetIdentifier());
  if (itSource != m_sources.end() && itSource->second == source)
    return true;

  if (!openRepository())
    return false;

  if (itSource == m_sources.end())
  {
    int idSource = addSource(source);
    closeRepository();

    if (idSource < 0)
      return false;

    m_sources.insert(std::make_pair(source.GetIdentifier(), source));
  }
  else
  {
    bool ret = updateSource(source);
    closeRepository();

    if (!ret)
      return false;

    itSource->second = source;
  }

  return true;
}

bool CGenericMediaImportRepository::UpdateSource(const CMediaImportSource &source)
{
  if (!m_loaded ||
      source.GetIdentifier().empty() || source.GetFriendlyName().empty())
    return false;
  
  CSingleLock sourcesLock(m_sourcesLock);
  auto&& itSource = m_sources.find(source.GetIdentifier());
  if (itSource == m_sources.end())
    return false;

  if (itSource->second == source)
    return true;

  if (!openRepository())
    return false;

  bool ret = updateSource(source);
  closeRepository();

  if (!ret)
    return false;

  itSource->second = source;

  return true;
}

bool CGenericMediaImportRepository::RemoveSource(const std::string &identifier)
{
  if (!m_loaded ||
      identifier.empty())
    return false;
  
  CSingleLock sourcesLock(m_sourcesLock);
  auto&& it = m_sources.find(identifier);
  if (it == m_sources.end())
    return false;

  if (!openRepository())
    return false;

  removeSource(it->second);
  closeRepository();

  m_sources.erase(it);

  CSingleLock importsLock(m_importsLock);
  for (MediaImportMap::iterator it = m_imports.begin(); it != m_imports.end(); )
  {
    if (it->second.GetSource().GetIdentifier().compare(identifier) == 0)
      m_imports.erase(it++);
    else
      ++it;
  }

  return true;
}

CGenericMediaImportRepository::MediaImportIdentifier CGenericMediaImportRepository::GetMediaImportIdentifier(const CMediaImport &import)
{
  return MediaImportIdentifier(import.GetPath(), import.GetMediaTypes());
}

bool CGenericMediaImportRepository::ContainsAllMediaTypes(const MediaTypes& mediaTypes, const GroupedMediaTypes& groupedMediaTypes)
{
  return std::all_of(groupedMediaTypes.begin(), groupedMediaTypes.end(),
    [&mediaTypes](const MediaType& mediaType) { return mediaTypes.find(mediaType) != mediaTypes.end(); });
}
