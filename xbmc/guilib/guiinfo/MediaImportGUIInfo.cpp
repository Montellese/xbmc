/*
 *  Copyright (C) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "guilib/guiinfo/MediaImportGUIInfo.h"

#include "FileItem.h"
#include "ServiceBroker.h"
#include "guilib/guiinfo/GUIInfo.h"
#include "guilib/guiinfo/GUIInfoLabels.h"
#include "media/import/MediaImportManager.h"

using namespace KODI::GUILIB;
using namespace KODI::GUILIB::GUIINFO;

bool CMediaImportGUIInfo::InitCurrentItem(CFileItem* item)
{
  return false;
}

bool CMediaImportGUIInfo::GetLabel(std::string& value,
                                   const CFileItem* item,
                                   int contextWindow,
                                   const CGUIInfo& info,
                                   std::string* fallback) const
{
  switch (info.m_info)
  {
    case LISTITEM_MEDIAIMPORTER:
    {
      if (item->IsImported())
      {
        const auto& mediaImportManager = CServiceBroker::GetMediaImportManager();
        CMediaImportSource source(item->GetSource());
        if (!mediaImportManager.GetSource(source.GetIdentifier(), source))
          return false;

        value = source.GetImporterId();
        return true;
      }
    }
  }

  return false;
}

bool CMediaImportGUIInfo::GetInt(int& value,
                                 const CGUIListItem* gitem,
                                 int contextWindow,
                                 const CGUIInfo& info) const
{
  return false;
}

bool CMediaImportGUIInfo::GetBool(bool& value,
                                  const CGUIListItem* gitem,
                                  int contextWindow,
                                  const CGUIInfo& info) const
{
  if (!gitem->IsFileItem())
    return false;

  const CFileItem* item = static_cast<const CFileItem*>(gitem);

  switch (info.m_info)
  {
    case LISTITEM_IS_IMPORTED:
      value = item->IsImported();
      return true;
  }

  return false;
}
