/*
 *      Copyright (C) 2011 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "AddonsOperations.h"
#include "JSONUtils.h"
#include "addons/AddonManager.h"
#include "addons/AddonDatabase.h"
#include "Application.h"

using namespace std;
using namespace JSONRPC;
using namespace ADDON;

JSONRPC_STATUS CAddonsOperations::GetAddons(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  TYPE addonType = TranslateType(parameterObject["type"].asString());
  CVariant enabled = parameterObject["enabled"];
  VECADDONS addons;
  if (addonType == ADDON_UNKNOWN)
  {
    if (!enabled.isBoolean())
    {
      CAddonMgr::Get().GetAllAddons(addons, false);
      VECADDONS enabledAddons;
      CAddonMgr::Get().GetAllAddons(enabledAddons, true);
      addons.insert(addons.begin(), enabledAddons.begin(), enabledAddons.end());
    }
    else
      CAddonMgr::Get().GetAllAddons(addons, enabled.asBoolean());
  }
  else
  {
    if (!enabled.isBoolean())
    {
      CAddonMgr::Get().GetAddons(addonType, addons, false);
      VECADDONS enabledAddons;
      CAddonMgr::Get().GetAddons(addonType, enabledAddons, true);
      addons.insert(addons.begin(), enabledAddons.begin(), enabledAddons.end());
    }
    else
      CAddonMgr::Get().GetAddons(addonType, addons, enabled.asBoolean());
  }

  int start, end;
  HandleLimits(parameterObject, result, addons.size(), start, end);
  
  for (int index = start; index < end; index++)
    FillDetails(addons.at(index), parameterObject["properties"], result["addons"], true);
  
  return OK;
}

JSONRPC_STATUS CAddonsOperations::GetAddonDetails(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CAddonDatabase addondatabase;
  if (!addondatabase.Open())
    return InternalError;

  string id = parameterObject["addonid"].asString();
  AddonPtr addon;
  if (!addondatabase.GetAddon(id, addon) || addon.get() == NULL)
    return InvalidParams;
    
  FillDetails(addon, parameterObject["properties"], result["addon"]);

  addondatabase.Close();
  return OK;
}

JSONRPC_STATUS CAddonsOperations::SetAddonEnabled(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  CAddonDatabase addondatabase;
  if (!addondatabase.Open())
    return InternalError;

  string id = parameterObject["addonid"].asString();
  if (!addondatabase.DisableAddon(id, !parameterObject["enabled"].asBoolean()))
    return InvalidParams;

  addondatabase.Close();
  return ACK;
}

JSONRPC_STATUS CAddonsOperations::ExecuteAddon(const CStdString &method, ITransportLayer *transport, IClient *client, const CVariant &parameterObject, CVariant &result)
{
  string id = parameterObject["addonid"].asString();
  AddonPtr addon;
  if (!CAddonMgr::Get().GetAddon(id, addon) || addon.get() == NULL ||
      addon->Type() < ADDON_VIZ || addon->Type() >= ADDON_VIZ_LIBRARY)
    return InvalidParams;
    
  string argv;
  CVariant params = parameterObject["params"];
  if (params.isObject())
  {
    for (CVariant::const_iterator_map it = params.begin_map(); it != params.end_map(); it++)
      argv += it->first + "=" + it->second.asString() + ",";
    argv = argv.erase(argv.size() - 1);
  }
  else if (params.isArray())
  {
    for (CVariant::const_iterator_array it = params.begin_array(); it != params.end_array(); it++)
      argv += it->asString() + ",";
    argv = argv.erase(argv.size() - 1);
  }
  
  CStdString cmd;
  if (params.size() == 0)
    cmd.Format("RunAddon(%s)", id.c_str());
  else
    cmd.Format("RunAddon(%s, %s)", id.c_str(), argv.c_str());
  g_application.getApplicationMessenger().ExecBuiltIn(cmd, parameterObject["wait"].asBoolean());
  
  return ACK;
}

void CAddonsOperations::FillDetails(AddonPtr addon, const CVariant& fields, CVariant &result, bool append /* = false */)
{
  if (addon.get() == NULL)
    return;
  
  CVariant addonInfo;
  addon->Props().Serialize(addonInfo);

  CVariant object;
  object["addonid"] = addonInfo["addonid"];
  object["type"] = addonInfo["type"];
  
  for (unsigned int index = 0; index < fields.size(); index++)
  {
    string field = fields[index].asString();
    
    if (field == "enabled")
      object[field] = ((CAddon*)addon.get())->Enabled();
    else if (addonInfo.isMember(field))
      object[field] = addonInfo[field];
  }
  
  if (append)
    result.append(object);
  else
    result = object;
}
