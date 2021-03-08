/*
 *  Copyright (C) 2013-2019 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "MediaImportSynchronisationTask.h"

#include "guilib/LocalizeStrings.h"
#include "media/MediaType.h"
#include "media/import/IMediaImportHandler.h"
#include "media/import/MediaImportManager.h"
#include "utils/StringUtils.h"
#include "utils/log.h"

#include <fmt/ostream.h>

using namespace fmt::literals;

CMediaImportSynchronisationTaskBase::CMediaImportSynchronisationTaskBase(
    const std::string& name,
    const CMediaImport& import,
    MediaImportHandlerPtr importHandler)
  : IMediaImportTask(name, import),
    m_importHandler(importHandler)
{
}

bool CMediaImportSynchronisationTaskBase::StartSynchronisation()
{
  if (m_importHandler == nullptr)
    return false;

  if (!m_importHandler->StartSynchronisation(m_import))
  {
    m_logger->info("failed to initialize synchronisation of imported {} items from {}",
                   m_importHandler->GetMediaType(), m_import.GetSource());
    return false;
  }

  return true;
}

bool CMediaImportSynchronisationTaskBase::StartSynchronisationBatch()
{
  if (m_importHandler == nullptr)
    return false;

  if (!m_importHandler->StartSynchronisationBatch(m_import))
  {
    m_logger->info("failed to start batch synchronisation of imported {} items from {}",
      m_importHandler->GetMediaType(), m_import.GetSource());
    return false;
  }

  return true;
}

bool CMediaImportSynchronisationTaskBase::SynchroniseItem(const ChangesetItemPtr& changedItem)
{
  if (m_importHandler == nullptr)
    return false;

  // nothing to do
  if (changedItem.first == MediaImportChangesetType::None)
    return true;

  auto item = changedItem.second.get();

  // get the item label to be used in the progress bar text
  std::string itemLabel = m_importHandler->GetItemLabel(item);

  // process the item depending on its changeset state
  switch (changedItem.first)
  {
    case MediaImportChangesetType::Added:
      SetProgressText(StringUtils::Format(g_localizeStrings.Get(39562), "mediaitem"_a = itemLabel));
      m_importHandler->AddImportedItem(m_import, item);
      break;

    case MediaImportChangesetType::Changed:
      SetProgressText(StringUtils::Format(g_localizeStrings.Get(39563), "mediaitem"_a = itemLabel));
      m_importHandler->UpdateImportedItem(m_import, item);
      break;

    case MediaImportChangesetType::Removed:
      SetProgressText(StringUtils::Format(g_localizeStrings.Get(39564), "mediaitem"_a = itemLabel));
      m_importHandler->RemoveImportedItem(m_import, item);
      break;

    case MediaImportChangesetType::None:
    default:
      m_logger->warn("ignoring imported item with unknown changeset type {}", changedItem.first);
      break;
  }

  return true;
}

bool CMediaImportSynchronisationTaskBase::FinishSynchronisationBatch()
{
  if (m_importHandler == nullptr)
    return false;

  if (!m_importHandler->FinishSynchronisationBatch(m_import))
  {
    m_logger->info("failed to finalize batch synchronisation of imported {} items from {}",
      m_importHandler->GetMediaType(), m_import.GetSource());
    return false;
  }

  return true;
}

bool CMediaImportSynchronisationTaskBase::FinishSynchronisation()
{
  if (m_importHandler == nullptr)
    return false;

  if (!m_importHandler->FinishSynchronisation(m_import))
  {
    m_logger->info("failed to finalize synchronisation of imported {} items from {}",
                   m_importHandler->GetMediaType(), m_import.GetSource());
    return false;
  }

  return true;
}

CMediaImportSynchronisationTask::CMediaImportSynchronisationTask(
  const CMediaImport& import, MediaImportHandlerPtr importHandler, const ChangesetItems& items)
  : CMediaImportSynchronisationTaskBase("CMediaImportSynchronisationTask",
                                        import,
                                        importHandler),
     m_items(items)
{
}

bool CMediaImportSynchronisationTask::DoWork()
{
  // nothing to do if there are no items to synchronise
  if (m_items.empty())
    return true;

  if (!StartSynchronisation())
    return false;

  // prepare the progress bar
  PrepareProgressBarHandle(StringUtils::Format(
      g_localizeStrings.Get(39561), "mediatype"_a = CMediaTypes::ToLabel(m_import.GetMediaTypes()),
      "provider"_a = m_import.GetSource().GetFriendlyName()));
  SetProgressText("");

  m_logger->info("handling {} imported {} items from {}", m_items.size(),
                 m_importHandler->GetMediaType(), m_import.GetSource());
  // handle the imported items of a specific media type
  size_t total = m_items.size();
  size_t progress = 0;
  bool result = true;
  for (const auto& item : m_items)
  {
    // check if we should cancel
    if (ShouldCancel() || !SynchroniseItem(item))
    {
      result = false;
      break;
    }

    ++progress;
    SetProgress(progress, total);
  }

  result &= FinishSynchronisation();
  return result;
}

bool CMediaImportSynchronisationTask::StartSynchronisation()
{
  if (!CMediaImportSynchronisationTask::StartSynchronisation())
    return false;

  if (!StartSynchronisationBatch())
  {
    CMediaImportSynchronisationTaskBase::FinishSynchronisation();
    return false;
  }

  return true;
}

bool CMediaImportSynchronisationTask::FinishSynchronisation()
{
  bool result = FinishSynchronisationBatch();
  result &= CMediaImportSynchronisationTaskBase::FinishSynchronisation();
  return result;
}

CMediaImportSynchronisationAsyncTask::CMediaImportSynchronisationAsyncTask(
    const CMediaImport& import,
    MediaImportHandlerPtr importHandler)
  : CMediaImportSynchronisationTaskBase("CMediaImportSynchronisationAsyncTask",
                                        import,
                                        importHandler)
{
}

void CMediaImportSynchronisationAsyncTask::AddItemsToProcess(const ChangesetItems& items)
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

void CMediaImportSynchronisationAsyncTask::FinalizeSynchronisation()
{
  if (m_finish)
    return;

  {
    CSingleLock lock(m_critical);
    m_finish = true;
  }

  // signal that there are no additional items to process
  m_processItemsEvent.Set();
}

bool CMediaImportSynchronisationAsyncTask::DoWork()
{
  if (!StartSynchronisation())
    return false;

  bool result = true;
  ChangesetItems itemsToProcess;
  bool finish = false;
  size_t total = 0;

  while (!finish)
  {
    // wait for new items to process
    bool eventReceived = m_processItemsEvent.WaitMSec(100);

    // check if we should cancel
    if (ShouldCancel())
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

      finish = m_finish;
    }

    // start batch processing
    if (!StartSynchronisationBatch())
    {
      result = false;
      break;
    }

    // process the items
    for (const auto& item : itemsToProcess)
    {
      // check if we should cancel
      if (ShouldCancel() || !SynchroniseItem(item))
      {
        result = false;
        break;
      }
    }

    itemsToProcess.clear();

    // start batch processing
    if (!FinishSynchronisationBatch())
    {
      result = false;
      break;
    }
  }

  result &= FinishSynchronisation();
  return result;
}
