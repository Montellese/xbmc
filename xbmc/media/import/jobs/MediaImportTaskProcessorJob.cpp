/*
 *  Copyright (C) 2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "MediaImportTaskProcessorJob.h"

#include "ServiceBroker.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "guilib/GUIComponent.h"
#include "guilib/GUIWindowManager.h"
#include "media/import/jobs/tasks/IMediaImportTask.h"
#include "settings/Settings.h"
#include "settings/SettingsComponent.h"
#include "utils/PerformanceMeasurement.h"
#include "utils/log.h"

#include <fmt/ostream.h>

CMediaImportTaskProcessorJob::CMediaImportTaskProcessorJob(const CMediaImportSource& source,
                                                           IMediaImportTaskCallback* callback,
                                                           bool hasProgress)
  : m_source(source),
    m_callback(callback),
    m_task(nullptr),
    m_hasProgress(hasProgress),
    m_progress(nullptr)
{
  if (m_hasProgress && CServiceBroker::GetSettingsComponent()->GetSettings()->GetBool(
                           CSettings::SETTING_VIDEOLIBRARY_BACKGROUNDUPDATE))
    m_hasProgress = false;
}

CMediaImportTaskProcessorJob::~CMediaImportTaskProcessorJob()
{
  if (m_progress != nullptr)
    m_progress->MarkFinished();
}

void CMediaImportTaskProcessorJob::SetTask(MediaImportTaskPtr task)
{
  m_task = task;
  if (m_task != nullptr)
    m_task->SetProcessorJob(this);
}

void CMediaImportTaskProcessorJob::ResetTask()
{
  if (m_task != nullptr)
    m_task->SetProcessorJob(nullptr);

  m_task.reset();
}

CGUIDialogProgressBarHandle* CMediaImportTaskProcessorJob::PrepareProgressBarHandle(
    const std::string& title)
{
  if (!m_hasProgress)
    return nullptr;

  if (m_progress == nullptr)
  {
    auto dialog = static_cast<CGUIDialogExtendedProgressBar*>(
        CServiceBroker::GetGUI()->GetWindowManager().GetWindow(WINDOW_DIALOG_EXT_PROGRESS));
    if (dialog != nullptr)
      m_progress = dialog->GetHandle(title);
  }
  else if (!title.empty())
    m_progress->SetTitle(title);

  return m_progress;
}

bool CMediaImportTaskProcessorJob::DoWork()
{
  // check if no task is set and there are no more task types to be performed
  if (m_task == nullptr)
    return true;

  // let the current task do its work
  return ProcessTask(m_task);
}

bool CMediaImportTaskProcessorJob::operator==(const CJob* job) const
{
  if (strcmp(job->GetType(), GetType()) != 0)
    return false;

  const CMediaImportTaskProcessorJob* rjob = dynamic_cast<const CMediaImportTaskProcessorJob*>(job);
  if (rjob == nullptr)
    return false;

  // compare the base properties
  return m_callback == rjob->m_callback && m_task == rjob->m_task && m_progress == rjob->m_progress;
}

bool CMediaImportTaskProcessorJob::ProcessTask(MediaImportTaskPtr task)
{
  if (task == nullptr)
    return false;

  SetTask(task);

  const auto& import = task->GetImport();

  GetLogger()->debug("processing {} task from {}...",
                     MediaImportTaskTypes::ToString(task->GetType()), import);

  // performance measurement
  CPerformanceMeasurement<> perf;

  // let the current task do its work
  bool success = task->DoWork();

  // the task has been completed
  success &= OnTaskComplete(success, task.get());

  perf.Stop();
  GetLogger()->debug("processing {} task from {} took {} s",
                     MediaImportTaskTypes::ToString(task->GetType()), import,
                     perf.GetDurationInSeconds());

  ResetTask();

  return success;
}

bool CMediaImportTaskProcessorJob::OnTaskComplete(bool success, const IMediaImportTask* task)
{
  if (m_callback == nullptr)
    return true;

  return m_callback->OnTaskComplete(success, task);
}

Logger CMediaImportTaskProcessorJob::GetLogger()
{
  static Logger s_logger = CServiceBroker::GetLogging().GetLogger("CMediaImportTaskProcessorJob");
  return s_logger;
}
