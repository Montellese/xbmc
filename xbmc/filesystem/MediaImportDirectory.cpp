/*
 *      Copyright (C) 2011-2013 Team XBMC
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

#include "MediaImportDirectory.h"
#include "DatabaseManager.h"
#include "DbUrl.h"
#include "FileItem.h"
#include "guilib/LocalizeStrings.h"
#include "filesystem/Directory.h"
#include "media/MediaType.h"
#include "media/import/IMediaImportRepository.h"
#include "media/import/MediaImportManager.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "video/VideoDbUrl.h"

using namespace std;
using namespace XFILE;

CMediaImportDirectory::CMediaImportDirectory(void)
{ }

CMediaImportDirectory::~CMediaImportDirectory(void)
{ }

bool CMediaImportDirectory::GetDirectory(const CURL& url, CFileItemList &items)
{
  const std::string& strPath = url.Get();
  if (url.GetHostName() == "imports")
  {
    std::string filename = url.GetFileName();
    URIUtils::RemoveSlashAtEnd(filename);
    if (filename.empty())
    {
      // All
      if (CMediaImportManager::GetInstance().HasImports())
      {
        CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "all"), true));
        item->SetLabel(g_localizeStrings.Get(39020));
        item->SetSpecialSort(SortSpecialOnTop);
        items.Add(item);
      }

        // by media provider
      if (CMediaImportManager::GetInstance().HasSources())
      {
        CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "sources"), true));
        item->SetLabel(g_localizeStrings.Get(39021));
        item->SetSpecialSort(SortSpecialOnTop);
        items.Add(item);
      }

      // by media types
      if (CMediaImportManager::GetInstance().HasImports())
      {
        CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "mediatypes"), true));
        item->SetLabel(g_localizeStrings.Get(39022));
        item->SetSpecialSort(SortSpecialOnTop);
        items.Add(item);
      }

      items.SetLabel(g_localizeStrings.Get(39101));
      return true;
    }
    else if (filename == "all" || StringUtils::StartsWith(filename, "sources") || StringUtils::StartsWith(filename, "mediatypes"))
    {
      if (filename == "all" ||
          StringUtils::StartsWith(filename, "sources/") ||
          StringUtils::StartsWith(filename, "mediatypes/"))
      {
        std::vector<CMediaImport> imports;
        if (filename == "all")
        {
          items.SetLabel(g_localizeStrings.Get(39020));
          imports = CMediaImportManager::GetInstance().GetImports();
        }
        else if (StringUtils::StartsWith(filename, "sources/"))
        {
          std::string sourceID = StringUtils::Mid(filename, 8);
          URIUtils::RemoveSlashAtEnd(sourceID);
          if (sourceID.find('/') != std::string::npos)
            return false;

          sourceID = CURL::Decode(sourceID);
          CMediaImportSource source(sourceID);
          if (!CMediaImportManager::GetInstance().GetSource(sourceID, source))
            return false;

          items.SetLabel(source.GetFriendlyName());
          imports = CMediaImportManager::GetInstance().GetImportsBySource(sourceID);
        }
        else if (StringUtils::StartsWith(filename, "mediatypes/"))
        {
          std::string strMediaTypes = StringUtils::Mid(filename, 11);
          URIUtils::RemoveSlashAtEnd(strMediaTypes);
          strMediaTypes = CURL::Decode(strMediaTypes);
          if (strMediaTypes.find('/') != std::string::npos)
            return false;

          GroupedMediaTypes mediaTypes = CMediaTypes::Split(strMediaTypes);
          if (mediaTypes.empty())
            return false;

          items.SetLabel(MediaTypesToLabel(mediaTypes));
          imports = CMediaImportManager::GetInstance().GetImportsByMediaType(mediaTypes);
        }
        else
          return false;

        HandleImports(strPath, imports, items);
        return true;
      }
      else if (filename == "sources")
      {
        items.SetLabel(g_localizeStrings.Get(39021));
        HandleSources(strPath, CMediaImportManager::GetInstance().GetSources(), items, true);
        return true;
      }
      else if (filename == "mediatypes")
      {
        items.SetLabel(g_localizeStrings.Get(39022));
        std::vector<GroupedMediaTypes> groupedMediaTypes = CMediaImportManager::GetInstance().GetSupportedMediaTypesGrouped(CMediaImportManager::GetInstance().GetSupportedMediaTypes());
        for (const auto& mediaTypes : groupedMediaTypes)
        {
          std::string path = URIUtils::AddFileToFolder(strPath, CURL::Encode(CMediaTypes::Join(mediaTypes)));
          CFileItemPtr item(new CFileItem(path, true));
          item->SetLabel(MediaTypesToLabel(mediaTypes));

          items.Add(item);
        }

        return true;
      }
    }
  }
  else if (url.GetHostName() == "sources")
  {
    std::string filename = url.GetFileName();
    URIUtils::RemoveSlashAtEnd(filename);
    if (filename.empty())
    {
      // All
      if (CMediaImportManager::GetInstance().HasSources())
      {
        {
          CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "all"), true));
          item->SetLabel(g_localizeStrings.Get(39023));
          item->SetSpecialSort(SortSpecialOnTop);
          items.Add(item);
        }

        // Active
        if (CMediaImportManager::GetInstance().HasSources(true))
        {
          CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "active"), true));
          item->SetLabel(g_localizeStrings.Get(39024));
          items.Add(item);
        }

        // Inactive
        if (CMediaImportManager::GetInstance().HasSources(false))
        {
          CFileItemPtr item(new CFileItem(URIUtils::AddFileToFolder(strPath, "inactive"), true));
          item->SetLabel(g_localizeStrings.Get(39025));
          items.Add(item);
        }
      }

      items.SetLabel(g_localizeStrings.Get(39100));
      return true;
    }
    else if (filename == "all" || filename == "active" || filename == "inactive")
    {
      std::vector<CMediaImportSource> sources;
      if (filename == "all")
      {
        items.SetLabel(g_localizeStrings.Get(39023));
        sources = CMediaImportManager::GetInstance().GetSources();
      }
      else
      {
        if (filename == "active")
          items.SetLabel(g_localizeStrings.Get(39024));
        else
          items.SetLabel(g_localizeStrings.Get(39025));

        sources = CMediaImportManager::GetInstance().GetSources(filename == "active");
      }

      HandleSources(strPath, sources, items);
      return true;
    }
  }

  return false;
}

void CMediaImportDirectory::HandleSources(const std::string &strPath, const std::vector<CMediaImportSource> &sources, CFileItemList &items, bool asFolder /* = false */)
{
  for (std::vector<CMediaImportSource>::const_iterator itSource = sources.begin(); itSource != sources.end(); ++itSource)
  {
    CFileItemPtr item = FileItemFromMediaImportSource(*itSource, strPath, asFolder);
    if (item != NULL)
      items.Add(item);
  }

  items.SetContent("sources");
}

CFileItemPtr CMediaImportDirectory::FileItemFromMediaImportSource(const CMediaImportSource &source, const std::string &basePath, bool asFolder /* = false */)
{
  if (source.GetIdentifier().empty() || source.GetFriendlyName().empty())
    return CFileItemPtr();
  
  // prepare the path
  std::string path = URIUtils::AddFileToFolder(basePath, CURL::Encode(source.GetIdentifier()));
  if (asFolder)
    URIUtils::AddSlashAtEnd(path);

  CFileItemPtr item(new CFileItem(path, asFolder));
  item->SetLabel(source.GetFriendlyName());
  item->m_dateTime = source.GetLastSynced();

  if (!source.GetIconUrl().empty())
    item->SetArt("thumb", source.GetIconUrl());

  item->SetProperty(PROPERTY_SOURCE_IDENTIFIER, source.GetIdentifier());
  item->SetProperty(PROPERTY_SOURCE_NAME, source.GetFriendlyName());
  item->SetProperty(PROPERTY_SOURCE_ISACTIVE, source.IsActive());
  item->SetProperty(PROPERTY_SOURCE_ISACTIVE_LABEL, source.IsActive() ? g_localizeStrings.Get(39026) : g_localizeStrings.Get(39027));

  return item;
}

void CMediaImportDirectory::HandleImports(const std::string &strPath, const std::vector<CMediaImport> &imports, CFileItemList &items, bool bySource /* = false */)
{
  for (std::vector<CMediaImport>::const_iterator itImport = imports.begin(); itImport != imports.end(); ++itImport)
  {
    CFileItemPtr item = FileItemFromMediaImport(*itImport, strPath, bySource);
    if (item != NULL)
      items.Add(item);
  }

  items.SetContent("imports");
}

CFileItemPtr CMediaImportDirectory::FileItemFromMediaImport(const CMediaImport &import, const std::string &basePath, bool bySource /* = false */)
{
  if (import.GetPath().empty() || import.GetMediaTypes().empty())
    return CFileItemPtr();

  const CMediaImportSource &source = import.GetSource();

  CURL url(URIUtils::AddFileToFolder(basePath, CURL::Encode(import.GetPath())));
  std::string mediaTypes = CMediaTypes::Join(import.GetMediaTypes());
  url.SetOption("mediatypes", mediaTypes);
  std::string path = url.Get();
  std::string mediaTypesLabel = MediaTypesToLabel(import.GetMediaTypes());
  std::string label = mediaTypesLabel;
  if (!bySource)
    label = StringUtils::Format(g_localizeStrings.Get(39015).c_str(), source.GetFriendlyName().c_str(), label.c_str());

  CFileItemPtr item(new CFileItem(path, false));
  item->SetLabel(label);
  item->m_dateTime = import.GetLastSynced();

  if (!source.GetIconUrl().empty())
    item->SetArt("thumb", source.GetIconUrl());

  item->SetProperty(PROPERTY_IMPORT_PATH, import.GetPath());
  item->SetProperty(PROPERTY_IMPORT_MEDIATYPES, mediaTypes);
  item->SetProperty(PROPERTY_IMPORT_MEDIATYPES_LABEL, mediaTypesLabel);
  item->SetProperty(PROPERTY_IMPORT_NAME, StringUtils::Format(g_localizeStrings.Get(39015).c_str(),
    source.GetFriendlyName().c_str(), mediaTypesLabel.c_str()));
  item->SetProperty(PROPERTY_SOURCE_IDENTIFIER, source.GetIdentifier());
  item->SetProperty(PROPERTY_SOURCE_NAME, source.GetFriendlyName());
  item->SetProperty(PROPERTY_SOURCE_ISACTIVE, import.IsActive());
  item->SetProperty(PROPERTY_SOURCE_ISACTIVE_LABEL, import.IsActive() ? g_localizeStrings.Get(39026) : g_localizeStrings.Get(39027));

  return item;
}
