/*
 *  Copyright (C) 2013-2019 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "MediaImportRemovalTask.h"

#include "guilib/LocalizeStrings.h"
#include "media/MediaType.h"
#include "media/import/IMediaImportHandler.h"
#include "media/import/MediaImportManager.h"
#include "utils/log.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

using namespace fmt::literals;

CMediaImportRemovalTask::CMediaImportRemovalTask(const CMediaImport& import,
                                                 MediaImportHandlerPtr importHandler)
  : IMediaImportTask("CMediaImportRemovalTask", import), m_importHandler(importHandler)
{
}

MediaType CMediaImportRemovalTask::GetMediaType() const
{
  if (m_importHandler == nullptr)
    return MediaTypeNone;

  return m_importHandler->GetMediaType();
}

bool CMediaImportRemovalTask::DoWork()
{
  if (m_importHandler == nullptr)
    return false;

  // prepare the progress bar
  PrepareProgressBarHandle(fmt::format(
      g_localizeStrings.Get(39566), "mediatype"_a = CMediaTypes::ToLabel(m_import.GetMediaTypes()),
      "provider"_a = m_import.GetSource().GetFriendlyName()));
  SetProgressText("");

  m_logger->info("removing imported {} items from {}", m_importHandler->GetMediaType(),
                 m_import.GetSource());

  return m_importHandler->RemoveImportedItems(m_import);
}