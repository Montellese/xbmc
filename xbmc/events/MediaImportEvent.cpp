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
#include "media/import/MediaImportManager.h"
#include "utils/StringUtils.h"

CMediaImportSourceEvent::CMediaImportSourceEvent(const CMediaImportSource& source, const CVariant& description, EventLevel level /* = EventLevel::Information */)
  : CMediaImportSourceEvent(source, description, false, level)
{ }

CMediaImportSourceEvent::CMediaImportSourceEvent(const CMediaImportSource& source, const CVariant& description, bool removed, EventLevel level /* = EventLevel::Information */)
  : CUniqueEvent(source.GetFriendlyName(), description, source.GetIconUrl(), CVariant{ removed }, level)
  , m_source(source)
{ }

std::string CMediaImportSourceEvent::GetExecutionLabel() const
{
  std::string executionLabel = CUniqueEvent::GetExecutionLabel();
  if (!executionLabel.empty())
    return executionLabel;

  return g_localizeStrings.Get(39052);
}

bool CMediaImportSourceEvent::CanExecute() const
{
  return !m_details.isBoolean() || !m_details.asBoolean();
}

bool CMediaImportSourceEvent::Execute() const
{
  if (!CanExecute())
    return false;

  std::vector<std::string> params;
  params.push_back(StringUtils::Format("import://imports/sources/%s/", CURL::Encode(m_source.GetIdentifier()).c_str()));
  params.push_back("return");
  g_windowManager.ActivateWindow(WINDOW_MEDIASOURCE_BROWSER, params);
  return true;
}

CMediaImportEvent::CMediaImportEvent(const CMediaImport& import, const CVariant& description, EventLevel level /* = EventLevel::Information */)
  : CMediaImportEvent(import, description, false, level)
{ }

CMediaImportEvent::CMediaImportEvent(const CMediaImport& import, const CVariant& description, bool removed, EventLevel level /* = EventLevel::Information */)
  : CUniqueEvent(StringUtils::Format(g_localizeStrings.Get(39065).c_str(), import.GetSource().GetFriendlyName().c_str(), CMediaTypes::ToLabel(import.GetMediaTypes()).c_str()),
      description, import.GetSource().GetIconUrl(), CVariant{ removed }, level)
  , m_import(import)
{ }

std::string CMediaImportEvent::GetExecutionLabel() const
{
  std::string executionLabel = CUniqueEvent::GetExecutionLabel();
  if (!executionLabel.empty())
    return executionLabel;

  return g_localizeStrings.Get(39107);
}

bool CMediaImportEvent::CanExecute() const
{
  return !m_details.isBoolean() || !m_details.asBoolean();
}

bool CMediaImportEvent::Execute() const
{
  if (!CanExecute())
    return false;

  return CMediaImportManager::GetInstance().Import(m_import.GetPath(), m_import.GetMediaTypes());
}
