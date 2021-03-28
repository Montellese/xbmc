/*
 *  Copyright (C) 2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "DictUtils.h"

#include "interfaces/legacy/Exception.h"

using namespace XBMCAddon;

namespace XBMCAddonUtils
{

bool DictUtils::CheckAndGetBoolProperty(const std::string& key, const CVariant& value)
{
  if (!value.isBoolean())
    throw WrongTypeException("%s expects an boolean", key.c_str());

  return value.asBoolean();
}

int32_t DictUtils::CheckAndGetIntegerProperty(const std::string& key, const CVariant& value)
{
  if (!value.isInteger())
    throw WrongTypeException("%s expects an integer", key.c_str());

  return value.asInteger32();
}

int64_t DictUtils::CheckAndGetInteger64Property(const std::string& key, const CVariant& value)
{
  if (!value.isInteger())
    throw WrongTypeException("%s expects a 64-bit integer", key.c_str());

  return value.asInteger();
}

double DictUtils::CheckAndGetDoubleProperty(const std::string& key, const CVariant& value)
{
  if (value.isDouble())
    return value.asDouble();
  else if (value.isInteger())
    return static_cast<double>(value.asInteger());

  throw WrongTypeException("%s expects an float", key.c_str());
}

float DictUtils::CheckAndGetFloatProperty(const std::string& key, const CVariant& value)
{
  return static_cast<float>(CheckAndGetDoubleProperty(key, value));
}

std::string DictUtils::CheckAndGetStringProperty(const std::string& key, const CVariant& value)
{
  if (!value.isString())
    throw WrongTypeException("%s expects an string", key.c_str());

  return value.asString();
}

std::vector<std::string> DictUtils::CheckAndGetStringArrayProperty(const std::string& key,
                                                                   const CVariant& value)
{
  if (!value.isArray())
    throw WrongTypeException("%s expects a list", key.c_str());

  std::vector<std::string> values;
  values.reserve(value.size());
  for (auto val = value.begin_array(); val != value.end_array(); ++val)
    values.push_back(CheckAndGetStringProperty(key, *val));

  return values;
}

void DictUtils::ProcessObject(const std::string& key,
                              const CVariant& obj,
                              const std::unordered_map<std::string, ObjectProperty>& properties,
                              bool logUnexpectedProperties /* = false */)
{
  if (!obj.isObject())
    throw WrongTypeException("%s expects a dict", key.c_str());

  if (properties.empty())
    return;

  std::set<std::string> processedProperties;
  for (const auto& member : obj.asMap())
  {
    const auto memberKey = StringUtils::ToLower(member.first);
    const auto& value = member.second;

    // look for the property
    const auto prop = properties.find(memberKey);
    if (prop != properties.end())
    {
      prop->second.getter(memberKey, value);
      processedProperties.insert(memberKey);
    }
    else if (logUnexpectedProperties)
      CLog::Log(LOGWARNING, "\"{}\" contains an unexpected property: {}", key, memberKey);
  }

  for (const auto& prop : properties)
  {
    if (prop.second.mandatory && processedProperties.find(prop.first) == processedProperties.end())
      throw WrongTypeException("%s expects a \"%s\" property", key.c_str(), prop.first.c_str());
  }
}
} // namespace XBMCAddonUtils
