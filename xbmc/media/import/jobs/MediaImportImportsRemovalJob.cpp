/*
 *  Copyright (C) 2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "MediaImportImportsRemovalJob.h"

#include "ServiceBroker.h"
#include "media/import/IMediaImportHandlerManager.h"
#include "media/import/jobs/tasks/MediaImportRemovalTask.h"
#include "utils/log.h"

#include <memory>

CMediaImportImportsRemovalJob::CMediaImportImportsRemovalJob(
    const CMediaImportSource& source,
    const std::vector<CMediaImport>& imports,
    const IMediaImportHandlerManager* importHandlerManager,
    IMediaImportTaskCallback* callback)
  : CMediaImportTaskProcessorJob(source, callback, true),
    m_imports(imports),
    m_importHandlerManager(importHandlerManager)
{
}

bool CMediaImportImportsRemovalJob::DoWork()
{
  if (m_importHandlerManager == nullptr)
  {
    GetLogger()->error("invalid media import handler manager implementation");
    return false;
  }

  // go through all imports and their media types in the proper order and remove them
  for (const auto& import : m_imports)
  {
    const auto& mediaTypes = import.GetMediaTypes();
    // go through all media types in the proper order and remove them
    for (auto&& mediaType = mediaTypes.rbegin(); mediaType != mediaTypes.rend(); ++mediaType)
    {
      const auto& importHandler = m_importHandlerManager->GetImportHandler(*mediaType);
      if (importHandler == nullptr)
      {
        GetLogger()->error("failed to get import handler for {}", *mediaType);
        return false;
      }

      auto removalTask = std::make_shared<CMediaImportRemovalTask>(
          import, MediaImportHandlerPtr(importHandler->Create()));

      // if processing the task failed remove the import
      GetLogger()->info("starting import removal task for {} items from {}...", *mediaType, import);
      if (!CMediaImportTaskProcessorJob::ProcessTask(removalTask))
      {
        GetLogger()->warn("import removal task for {} items from {} failed", *mediaType, import);
        return false;
      }
    }
  }

  return true;
}

Logger CMediaImportImportsRemovalJob::GetLogger()
{
  static Logger s_logger = CServiceBroker::GetLogging().GetLogger("CMediaImportImportsRemovalJob");
  return s_logger;
}
