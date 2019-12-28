/*
 *  Copyright (C) 2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "MediaImportUpdateItemOnSourceJob.h"

#include "FileItem.h"
#include "media/import/IMediaImporterManager.h"
#include "media/import/MediaImport.h"
#include "media/import/jobs/tasks/MediaImportUpdateTask.h"

#include <memory>

CMediaImportUpdateItemOnSourceJob::CMediaImportUpdateItemOnSourceJob(
    const CMediaImport& import,
    const CFileItem& item,
    const IMediaImporterManager* importerManager,
    IMediaImportTaskCallback* callback)
  : CMediaImportTaskProcessorJob(import.GetSource(), callback, false)
{
  m_task = std::make_shared<CMediaImportUpdateTask>(import, item, importerManager);
}
