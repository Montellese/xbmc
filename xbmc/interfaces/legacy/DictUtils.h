/*
 *  Copyright (C) 2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "utils/Variant.h"

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace XBMCAddonUtils
{

class DictUtils
{
public:
  static bool CheckAndGetBoolProperty(const std::string& key, const CVariant& value);
  static int32_t CheckAndGetIntegerProperty(const std::string& key, const CVariant& value);
  static int64_t CheckAndGetInteger64Property(const std::string& key, const CVariant& value);
  static double CheckAndGetDoubleProperty(const std::string& key, const CVariant& value);
  static float CheckAndGetFloatProperty(const std::string& key, const CVariant& value);
  static std::string CheckAndGetStringProperty(const std::string& key, const CVariant& value);
  static std::vector<std::string> CheckAndGetStringArrayProperty(const std::string& key,
                                                                 const CVariant& value);

  typedef struct ObjectProperty
  {
    std::function<void(const std::string&, const CVariant&)> getter;
    bool mandatory = false;
  } ObjectProperty;

  static void ProcessObject(const std::string& key,
                            const CVariant& obj,
                            const std::unordered_map<std::string, ObjectProperty>& properties,
                            bool logUnexpectedProperties = false);
};

} // namespace XBMCAddonUtils
