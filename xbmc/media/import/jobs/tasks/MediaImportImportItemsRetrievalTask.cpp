/*
 *  Copyright (C) 2013-2019 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "MediaImportImportItemsRetrievalTask.h"

#include "guilib/LocalizeStrings.h"
#include "media/import/IMediaImporter.h"
#include "media/import/MediaImportManager.h"
#include "threads/SingleLock.h"
#include "utils/StringUtils.h"
#include "utils/log.h"

#include <fmt/ostream.h>

using namespace fmt::literals;

CMediaImportImportItemsRetrievalTask::CMediaImportImportItemsRetrievalTask(
    const CMediaImport& import,
    const IMediaImporterManager* importerManager,
    IMediaImportItemsRetrievalObserver* observer /* = nullptr */)
  : IMediaImportTask("CMediaImportImportItemsRetrievalTask", import),
    m_importerManager(importerManager),
    m_importer(),
    m_retrievedItems(),
    m_isChangeset(false),
    m_itemsRetrievalObserver(observer)
{
  // pre-fill the item maps with all media types to be retrieved
  for (const auto& mediaType : import.GetMediaTypes())
  {
    m_localItems.insert(std::make_pair(mediaType, std::vector<CFileItemPtr>()));
    m_retrievedItems.insert(std::make_pair(mediaType, ChangesetItems()));
  }
}

bool CMediaImportImportItemsRetrievalTask::DoWork()
{
  if (m_importer == nullptr)
  {
    if (m_importerManager == nullptr)
    {
      m_logger->error("invalid media importer manager implementation");
      return false;
    }

    // look for an importer than can handle the given path
    m_importer = m_importerManager->GetImporterBySource(m_import.GetSource());
    if (m_importer == nullptr)
    {
      m_logger->error("no importer capable of handling source {} found", m_import.GetSource());
      return false;
    }
  }

  PrepareProgressBarHandle(StringUtils::Format(g_localizeStrings.Get(39558),
                                               "provider"_a = m_import.GetSource().GetFriendlyName()));

  return m_importer->Import(this);
}

std::vector<CFileItemPtr> CMediaImportImportItemsRetrievalTask::GetLocalItems(
    const MediaType& mediaType)
{
  auto localItems = m_localItems.find(mediaType);
  if (localItems == m_localItems.end())
    return {};

  return localItems->second;
}

void CMediaImportImportItemsRetrievalTask::SetLocalItems(const std::vector<CFileItemPtr>& items,
                                                         const MediaType& mediaType)
{
  auto localItems = m_localItems.find(mediaType);
  if (localItems == m_localItems.end())
    return;

  localItems->second = items;
}

ChangesetItems CMediaImportImportItemsRetrievalTask::GetRetrievedItems(
    const MediaType& mediaType) const
{
  CSingleLock lock(m_critical);

  auto retrievedItems = m_retrievedItems.find(mediaType);
  if (retrievedItems == m_retrievedItems.end())
    return {};

  return retrievedItems->second;
}

ChangesetItems CMediaImportImportItemsRetrievalTask::GetAndClearRetrievedItems(
    const MediaType& mediaType)
{
  CSingleLock lock(m_critical);

  auto retrievedItems = m_retrievedItems.find(mediaType);
  if (retrievedItems == m_retrievedItems.end())
    return {};

  const auto items = retrievedItems->second;
  retrievedItems->second.clear();

  return items;
}

void CMediaImportImportItemsRetrievalTask::AddItem(
    const CFileItemPtr& item,
    const MediaType& mediaType,
    MediaImportChangesetType changesetType /* = MediaImportChangesetTypeNone */)
{
  AddItems({ item }, mediaType, changesetType);
}

void CMediaImportImportItemsRetrievalTask::AddItems(
    const std::vector<CFileItemPtr>& items,
    const MediaType& mediaType,
    MediaImportChangesetType changesetType /* = MediaImportChangesetTypeNone */)
{
  {
    CSingleLock lock(m_critical);

    auto retrievedItems = m_retrievedItems.find(mediaType);
    if (retrievedItems == m_retrievedItems.end())
      return;

    for (const auto& item : items)
      retrievedItems->second.push_back(std::make_pair(changesetType, item));
  }

  NotifyItemsRetrievedObserver(mediaType);
}

void CMediaImportImportItemsRetrievalTask::SetItems(const ChangesetItems& items,
                                                    const MediaType& mediaType)
{
  {
    CSingleLock lock(m_critical);

    auto retrievedItems = m_retrievedItems.find(mediaType);
    if (retrievedItems == m_retrievedItems.end())
      return;

    retrievedItems->second = items;
  }

  NotifyItemsRetrievedObserver(mediaType);
}

void CMediaImportImportItemsRetrievalTask::NotifyItemsRetrievedObserver(const MediaType& mediaType)
{
  if (m_itemsRetrievalObserver != nullptr)
    m_itemsRetrievalObserver->ItemsRetrieved(mediaType);
}
