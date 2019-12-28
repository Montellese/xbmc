/*
 *  Copyright (C) 2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "media/import/jobs/MediaImportTaskProcessorJob.h"

class CFileItem;
class CMediaImport;
class IMediaImporterManager;

class CMediaImportUpdateItemOnSourceJob : public CMediaImportTaskProcessorJob
{
public:
  CMediaImportUpdateItemOnSourceJob(const CMediaImport& import,
                                    const CFileItem& item,
                                    const IMediaImporterManager* importerManager,
                                    IMediaImportTaskCallback* callback);

  // implementation of CJob
  const char* GetType() const override { return "MediaImportUpdateItemOnSourceJob"; }
};
