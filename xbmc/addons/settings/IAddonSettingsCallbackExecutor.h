/*
 *  Copyright (C) 2019 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "settings/lib/SettingDefinitions.h"

#include <memory>
#include <string>

namespace ADDON
{

class IAddonSettingsCallbackExecutor
{
public:
  virtual ~IAddonSettingsCallbackExecutor() = default;

  virtual void OnSettingAction(std::shared_ptr<const CSetting> setting,
                               const std::string& callback,
                               void* data)
  {
  }

  virtual void OnIntegerSettingOptionsFiller(std::shared_ptr<const CSetting> setting,
                                             const std::string& callback,
                                             IntegerSettingOptions& list,
                                             int& current,
                                             void* data)
  {
  }
  virtual bool SetIntegerSettingOptions(const std::string& settingId,
                                        const IntegerSettingOptions& list,
                                        void* data)
  {
    return false;
  }

  virtual void OnStringSettingOptionsFiller(std::shared_ptr<const CSetting> setting,
                                            const std::string& callback,
                                            StringSettingOptions& list,
                                            std::string& current,
                                            void* data)
  {
  }
  virtual bool SetStringSettingOptions(const std::string& settingId,
                                       const StringSettingOptions& list,
                                       void* data)
  {
    return false;
  }

protected:
  IAddonSettingsCallbackExecutor() = default;
};

} // namespace ADDON
