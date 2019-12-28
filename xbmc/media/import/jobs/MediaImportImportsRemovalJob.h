/*
 *  Copyright (C) 2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "media/import/MediaImport.h"
#include "media/import/jobs/MediaImportTaskProcessorJob.h"

class IMediaImportHandlerManager;

class CMediaImportImportsRemovalJob : public CMediaImportTaskProcessorJob
{
public:
  CMediaImportImportsRemovalJob(const CMediaImportSource& source,
                                const std::vector<CMediaImport>& imports,
                                const IMediaImportHandlerManager* importHandlerManager,
                                IMediaImportTaskCallback* callback);

  // implementation of CJob
  const char* GetType() const override { return "MediaImportImportsRemovalJob"; }

  // specialization of CMediaImportTaskProcessorJob
  bool DoWork() override;

private:
  static Logger GetLogger();

  std::vector<CMediaImport> m_imports;
  const IMediaImportHandlerManager* m_importHandlerManager;
};
