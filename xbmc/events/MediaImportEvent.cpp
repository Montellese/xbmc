/*
 *      Copyright (C) 2015 Team Kodi
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "MediaImportEvent.h"
#include "URL.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"
#include "guilib/WindowIDs.h"
#include "utils/StringUtils.h"

CMediaImportEvent::CMediaImportEvent(const CMediaImportSource& source, const CVariant& description)
  : CUniqueEvent(source.GetFriendlyName(), description, source.GetIconUrl()),
    m_source(source)
{ }

CMediaImportEvent::CMediaImportEvent(const CMediaImportSource& source, const CVariant& description, const CVariant& details)
  : CUniqueEvent(source.GetFriendlyName(), description, source.GetIconUrl(), details),
    m_source(source)
{ }

CMediaImportEvent::CMediaImportEvent(const CMediaImportSource& source, const CVariant& description, const CVariant& details, const CVariant& executionLabel)
  : CUniqueEvent(source.GetFriendlyName(), description, source.GetIconUrl(), details, executionLabel),
    m_source(source)
{ }

std::string CMediaImportEvent::GetExecutionLabel() const
{
  std::string executionLabel = CUniqueEvent::GetExecutionLabel();
  if (!executionLabel.empty())
    return executionLabel;

  return g_localizeStrings.Get(39002);
}

bool CMediaImportEvent::Execute() const
{
  if (!CanExecute())
    return false;

  std::vector<std::string> params;
  params.push_back(StringUtils::Format("import://imports/sources/%s/", CURL::Encode(m_source.GetIdentifier()).c_str()));
  params.push_back("return");
  g_windowManager.ActivateWindow(WINDOW_MEDIASOURCE_BROWSER, params);
  return true;
}
