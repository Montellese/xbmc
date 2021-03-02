/*
 *  Copyright (C) 2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "LibraryQueue.h"
#include "media/import/MediaImportSource.h"
#include "utils/logtypes.h"

#include <memory>
#include <string>

class CGUIDialogProgressBarHandle;
class IMediaImportTask;
class IMediaImportTaskCallback;

class CMediaImportTaskProcessorJob : public CLibraryJob
{
public:
  ~CMediaImportTaskProcessorJob() override;

  const CMediaImportSource& GetSource() const { return m_source; }

  std::shared_ptr<const IMediaImportTask> GetCurrentTask() const { return m_task; }

  /*
   * \brief Prepares a progress indicator with the given title set.
   *
   * \param title Title to be set in the progress indicator.
   */
  CGUIDialogProgressBarHandle* PrepareProgressBarHandle(const std::string& title);

  /*!
   * \brief Get the progress bar handle instance used by the import task
   */
  CGUIDialogProgressBarHandle* GetProgressBarHandle() const { return m_progress; }

  // implementation of CJob
  bool DoWork() override;
  bool operator==(const CJob* job) const override;

protected:
  CMediaImportTaskProcessorJob(const CMediaImportSource& source,
                               IMediaImportTaskCallback* callback,
                               bool hasProgress);

  void SetTask(std::shared_ptr<IMediaImportTask> task);
  void ResetTask();

  virtual bool ProcessTask(std::shared_ptr<IMediaImportTask> task);
  virtual bool OnTaskComplete(bool success, const IMediaImportTask* task);

  static Logger GetLogger();

  CMediaImportSource m_source;
  IMediaImportTaskCallback* m_callback;
  std::shared_ptr<IMediaImportTask> m_task;
  bool m_hasProgress;
  CGUIDialogProgressBarHandle* m_progress;
};
