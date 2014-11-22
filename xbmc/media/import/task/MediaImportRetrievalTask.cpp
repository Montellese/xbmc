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

#include "MediaImportRetrievalTask.h"
#include "guilib/LocalizeStrings.h"
#include "media/import/IMediaImporter.h"
#include "media/import/IMediaImportHandler.h"
#include "media/import/MediaImportManager.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

CMediaImportRetrievalTask::CMediaImportRetrievalTask(const CMediaImport &import, std::map<MediaType, MediaImportHandlerPtr> importHandlers)
  : IMediaImportTask(import),
    m_importer(),
    m_importHandlers(importHandlers),
    m_retrievedItems(),
    m_localItems()
{
  // pre-fill the item maps with all media types to be retrieved
  for (const auto& mediaType : import.GetMediaTypes())
  {
    m_retrievedItems.insert(std::make_pair(mediaType, ChangesetItems()));
    m_localItems.insert(std::make_pair(mediaType, std::vector<CFileItemPtr>()));
  }
}

CMediaImportRetrievalTask::~CMediaImportRetrievalTask()
{ }

bool CMediaImportRetrievalTask::DoWork()
{
  if (m_importer == nullptr)
  {
    // look for an importer than can handle the given path
    const MediaImporterConstPtr importer = CMediaImportManager::GetInstance().GetImporter(m_import.GetPath());
    if (importer == nullptr)
    {
      CLog::Log(LOGERROR, "CMediaImportRetrievalTask: no importer capable of handling %s (%s) found", m_import.GetSource().GetFriendlyName().c_str(), m_import.GetPath().c_str());
      return false;
    }
    
    // try to create a new instance of the matching importer for this import
    m_importer = MediaImporterPtr(importer->Create(m_import));
    if (m_importer == nullptr)
    {
      CLog::Log(LOGERROR, "CMediaImportRetrievalTask: failed to create importer %s to handle %s", importer->GetIdentification(), m_import.GetPath().c_str());
      return false;
    }
  }

  GetProgressBarHandle(StringUtils::Format(g_localizeStrings.Get(39008).c_str(), m_import.GetSource().GetFriendlyName().c_str()));

  // first get a list of items previously imported from the media import
  for (const auto& importHandler : m_importHandlers)
  {
    if (importHandler.second == nullptr)
      continue;

    std::vector<CFileItemPtr> localItems;
    if (!importHandler.second->GetLocalItems(m_import, localItems))
    {
      CLog::Log(LOGERROR, "CMediaImportRetrievalTask: failed to get previously imported items of type %s from %s", importHandler.first.c_str(), m_import.GetPath().c_str());
      return false;
    }

    m_localItems[importHandler.first] = localItems;
  }

  return m_importer->Import(this);
}

void CMediaImportRetrievalTask::AddItem(const CFileItemPtr& item, const MediaType& mediaType, MediaImportChangesetType changesetType /* = MediaImportChangesetTypeNone */)
{
  auto retrievedItems = m_retrievedItems.find(mediaType);
  if (retrievedItems == m_retrievedItems.end())
    return;

  retrievedItems->second.push_back(std::make_pair(changesetType, item));
}

void CMediaImportRetrievalTask::AddItems(const std::vector<CFileItemPtr>& items, const MediaType& mediaType, MediaImportChangesetType changesetType /* = MediaImportChangesetTypeNone */)
{
  for (const auto& item : items)
    AddItem(item, mediaType, changesetType);
}

void CMediaImportRetrievalTask::SetItems(const ChangesetItems& items, const MediaType& mediaType)
{
  auto retrievedItems = m_retrievedItems.find(mediaType);
  if (retrievedItems == m_retrievedItems.end())
    return;

  retrievedItems->second = items;
}
