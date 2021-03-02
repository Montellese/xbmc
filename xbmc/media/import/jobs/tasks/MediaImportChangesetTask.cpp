/*
 *  Copyright (C) 2013-2019 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "MediaImportChangesetTask.h"

#include "guilib/LocalizeStrings.h"
#include "media/import/IMediaImportHandler.h"
#include "threads/SingleLock.h"
#include "utils/StringUtils.h"
#include "utils/log.h"

#include <fmt/ostream.h>

#include <algorithm>

using namespace fmt::literals;

CMediaImportChangesetTaskBase::CMediaImportChangesetTaskBase(
    const std::string& name,
    const CMediaImport& import,
    MediaImportHandlerPtr importHandler,
    const std::vector<CFileItemPtr>& localItems)
  : IMediaImportTask(name, import),
    m_importHandler(importHandler),
    m_localItems(localItems)
{
}

bool CMediaImportChangesetTaskBase::StartChangeset()
{
  return true;
}

MediaImportChangesetType CMediaImportChangesetTaskBase::DetermineChangeset(
    MediaImportChangesetType changesetType,
    CFileItemPtr item,
    CFileItemPtr& matchingLocalItem) const
{
  if (item == nullptr)
    return MediaImportChangesetType::None;

  // try to find a local item matching the retrieved item
  matchingLocalItem = m_importHandler->FindMatchingLocalItem(m_import, item.get(), m_localItems);

  // no matching local item found
  if (matchingLocalItem == nullptr)
  {
    // we don't know the changeset or it has already been set to added
    if (changesetType == MediaImportChangesetType::None ||
        changesetType == MediaImportChangesetType::Added)
      return MediaImportChangesetType::Added;

    // cannot change or remove an imported item without a matching local item
    if (changesetType == MediaImportChangesetType::Changed)
    {
      m_logger->warn("unable to change item {} from {} because there's no matching local item",
                     item->GetPath(), m_import);
    }

    return MediaImportChangesetType::None;
  }

  // we can't add an item that has already been imported so we'll update it
  if (changesetType == MediaImportChangesetType::None ||
      changesetType == MediaImportChangesetType::Added)
    return MediaImportChangesetType::Changed;

  // if the item should be removed we need to replace it with the matching local item
  if (changesetType == MediaImportChangesetType::Removed)
    item = matchingLocalItem;
  else
  {
    // nothing to do if we don't need to update imported items
    if (!m_import.Settings()->UpdateImportedMediaItems())
      return MediaImportChangesetType::None;

    // determine the changeset state of the item
    changesetType = m_importHandler->DetermineChangeset(m_import, item.get(), matchingLocalItem);

    // if the imported item has changed prepare it for updating
    if (changesetType != MediaImportChangesetType::None)
      m_importHandler->PrepareImportedItem(m_import, item.get(), matchingLocalItem);
  }

  return changesetType;
}

MediaImportChangesetType CMediaImportChangesetTaskBase::ProcessImportedItem(
    MediaImportChangesetType changesetType,
    CFileItemPtr item)
{
  CFileItemPtr matchingLocalItem;
  changesetType = DetermineChangeset(changesetType, item, matchingLocalItem);

  if (matchingLocalItem != nullptr)
  {
    // remove the matching item from the local list so that the imported item is not considered non-existant
    m_localItems.erase(
        std::remove(m_localItems.begin(), m_localItems.end(), matchingLocalItem),
        m_localItems.end());
  }

  return changesetType;
}

ChangesetItems CMediaImportChangesetTaskBase::FinishChangeset(bool partialChangeset)
{
  ChangesetItems items;
  if (!partialChangeset)
  {
    // all local items left need to be removed
    for (const auto& item : m_localItems)
      items.emplace_back(MediaImportChangesetType::Removed, item);
  }

  m_localItems.clear();

  return items;
}

CMediaImportChangesetTask::CMediaImportChangesetTask(const CMediaImport& import,
                                                     MediaImportHandlerPtr importHandler,
                                                     const std::vector<CFileItemPtr>& localItems,
                                                     const ChangesetItems& retrievedItems,
                                                     bool partialChangeset /* = false */)
  : CMediaImportChangesetTaskBase("CMediaImportChangesetTask", import, importHandler, localItems),
    m_retrievedItems(retrievedItems),
    m_partialChangeset(partialChangeset)
{
}

bool CMediaImportChangesetTask::DoWork()
{
  if (!StartChangeset())
    return false;

  size_t total = m_retrievedItems.size();
  size_t progress = 0;

  // prepare the progress bar
  PrepareProgressBarHandle(StringUtils::Format(g_localizeStrings.Get(39559),
                                               "provider"_a = m_import.GetSource().GetFriendlyName()));
  SetProgressText(StringUtils::Format(
      g_localizeStrings.Get(39560),
      "mediatype"_a = CMediaTypes::GetPluralLocalization(m_importHandler->GetMediaType())));

  const auto& settings = m_import.Settings();

  bool result = true;
  for (ChangesetItems::iterator item = m_retrievedItems.begin(); item != m_retrievedItems.end();)
  {
    // check if we should cancel
    if (ShouldCancel(progress, total))
    {
      result = false;
      break;
    }

    item->first = ProcessImportedItem(item->first, item->second);

    // if the changeset state couldn't be determined, ignore the item
    if (item->first == MediaImportChangesetType::None)
      item = m_retrievedItems.erase(item);
    else
      ++item;

    ++progress;
    SetProgress(progress, total);
  }

  auto removedItems = FinishChangeset(m_partialChangeset);
  m_retrievedItems.reserve(m_retrievedItems.size() + removedItems.size());
  std::move(removedItems.begin(), removedItems.end(), std::back_inserter(m_retrievedItems));

  return result;
}

CMediaImportChangesetAsyncTask::CMediaImportChangesetAsyncTask(
    const CMediaImport& import,
    MediaImportHandlerPtr importHandler,
    const std::vector<CFileItemPtr>& localItems,
    IMediaImportChangesetItemsObserver* observer)
  : CMediaImportChangesetTaskBase("CMediaImportChangesetAsyncTask",
                                  import,
                                  importHandler,
                                  localItems),
    m_changesetItemsObserver(observer),
    m_processItemsEvent()
{
}

void CMediaImportChangesetAsyncTask::AddItemsToProcess(const ChangesetItems & items)
{
  if (items.empty())
    return;

  {
    // copy the items to the list of items to process
    CSingleLock lock(m_critical);
    m_itemsToProcess.insert(m_itemsToProcess.end(), items.begin(), items.end());
  }

  // signal that new items are available
  m_processItemsEvent.Set();
}

void CMediaImportChangesetAsyncTask::FinalizeChangeset(bool partialChangeset)
{
  if (m_finish)
    return;

  {
    CSingleLock lock(m_critical);
    m_partialChangeset = partialChangeset;
    m_finish = true;
  }

  // signal that there are no additional items to process
  m_processItemsEvent.Set();
}

bool CMediaImportChangesetAsyncTask::DoWork()
{
  if (!StartChangeset())
    return false;

  const auto& settings = m_import.Settings();

  bool result = true;
  ChangesetItems itemsToProcess;
  bool partialChangeset = false;
  bool finish = false;
  size_t total = 0;

  while (!finish)
  {
    // wait for new items to process
    bool eventReceived = m_processItemsEvent.WaitMSec(100);

    // check if we should cancel
    // TODO(Montellese): don't report progress
    if (ShouldCancel(m_countProcessedItems, total))
    {
      result = false;
      break;
    }

    if (!eventReceived)
      continue;

    // retrieve new items to process
    {
      CSingleLock lock(m_critical);
      std::move(m_itemsToProcess.begin(), m_itemsToProcess.end(), std::back_inserter(itemsToProcess));
      m_itemsToProcess.clear();
      total += itemsToProcess.size();

      partialChangeset = m_partialChangeset;
      finish = m_finish;
    }

    // process the items
    for (ChangesetItems::iterator item = itemsToProcess.begin(); item != itemsToProcess.end();)
    {
      // check if we should cancel
      if (ShouldCancel(m_countProcessedItems, total))
      {
        result = false;
        break;
      }

      item->first = ProcessImportedItem(item->first, item->second);

      // if the changeset state couldn't be determined, ignore the item
      if (item->first == MediaImportChangesetType::None)
        item = itemsToProcess.erase(item);
      else
        ++item;

      ++m_countProcessedItems;
    }

    // notify the observer about the processed items
    NotifyChangesetItemsObserver(itemsToProcess);
    itemsToProcess.clear();
  }

  // finish the changeset
  auto removedItems = FinishChangeset(partialChangeset);
  std::move(removedItems.begin(), removedItems.end(), std::back_inserter(itemsToProcess));

  // notify the observer about the remaining items
  NotifyChangesetItemsObserver(itemsToProcess);

  return result;
}

void CMediaImportChangesetAsyncTask::NotifyChangesetItemsObserver(
    const ChangesetItems& changesetItems)
{
  if (!changesetItems.empty() && m_changesetItemsObserver != nullptr)
    m_changesetItemsObserver->ChangesetDetermined(m_importHandler->GetMediaType(), changesetItems);
}
