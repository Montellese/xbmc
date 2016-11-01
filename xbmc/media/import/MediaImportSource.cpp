/*
 *      Copyright (C) 2016 Team XBMC
 *      http://xbmc.org
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

#include "MediaImportSource.h"
#include "settings/lib/SettingsManager.h"
#include "utils/log.h"
#include "utils/XBMCTinyXML.h"

CMediaImportSource::CMediaImportSource(const std::string& identifier,
  const std::string& basePath /* = "" */,
  const std::string& friendlyName /* = "" */,
  const std::string& iconUrl /* = "" */ ,
  const MediaTypes& availableMediaTypes /* = MediaTypes() */ ,
  const CDateTime& lastSynced /* = CDateTime() */ ,
  const std::string& settingValues /* = "" */)
  : m_identifier(identifier)
  , m_basePath(basePath)
  , m_friendlyName(friendlyName)
  , m_iconUrl(iconUrl)
  , m_availableMediaTypes(availableMediaTypes)
  , m_lastSynced(lastSynced)
  , m_active(false)
  , m_settings(settingValues)
{ }


bool CMediaImportSource::operator==(const CMediaImportSource &other) const
{
  if (m_identifier.compare(other.m_identifier) != 0 ||
      m_basePath.compare(other.m_basePath) != 0 ||
      m_friendlyName.compare(other.m_friendlyName) != 0 ||
      m_iconUrl.compare(other.m_iconUrl) != 0 ||
      m_availableMediaTypes != other.m_availableMediaTypes ||
      m_lastSynced != other.m_lastSynced ||
      m_settings != other.m_settings)
    return false;

  return true;
}

CMediaImportSource::CSettings::CSettings(const std::string& settingValues /* = "" */)
  : CSettingsBase()
  , m_settingValues(settingValues)
{ }

CMediaImportSource::CSettings::CSettings(const CMediaImportSource::CSettings& other)
  : CSettings(other.m_settingValues)
{
  SetDefinition(other.m_settingDefinition);

  if (other.IsLoaded())
    Load();
}

bool CMediaImportSource::CSettings::operator==(const CSettings &other) const
{
  if (ToXml() != other.ToXml())
    return false;

  if (IsLoaded() != other.IsLoaded())
    return true;

  return m_settingDefinition == other.m_settingDefinition;  
}

CMediaImportSource::CSettings& CMediaImportSource::CSettings::operator=(const CMediaImportSource::CSettings& other)
{
  if (this == &other)
    return *this;

  if (IsLoaded())
    Unload();

  m_settingValues = other.m_settingValues;
  m_settingDefinition = other.m_settingDefinition;

  if (other.IsLoaded())
    Load();

  return *this;
}

bool CMediaImportSource::CSettings::Load()
{
  // try to initialize the settings by loading its definitions
  if (!Initialize())
  {
    CLog::Log(LOGERROR, "CMediaImportSource: failed to initialize settings");
    return false;
  }

  // if available try to load the setting's values
  if (!m_settingValues.empty())
  {
    CXBMCTinyXML xmlValues;
    if (!xmlValues.Parse(m_settingValues, TIXML_ENCODING_UTF8))
    {
      CLog::Log(LOGERROR, "CMediaImportSource: error loading setting values, Line %d\n%s", xmlValues.ErrorRow(), xmlValues.ErrorDesc());
      Uninitialize();
      return false;
    }

    if (xmlValues.RootElement() == nullptr)
    {
      Uninitialize();
      return false;
    }

    if (!LoadValuesFromXml(xmlValues))
    {
      CLog::Log(LOGERROR, "CMediaImportSource: failed to load setting values");
      Uninitialize();
      return false;
    }
  }

  // we are done with loading
  SetLoaded();

  return true;
}

bool CMediaImportSource::CSettings::Save()
{
  if (!IsLoaded())
    return false;

  m_settingValues = ToXml();
  return true;
}

void CMediaImportSource::CSettings::Unload()
{
  CSettingsBase::Unload();
  CSettingsBase::Uninitialize();
}

void CMediaImportSource::CSettings::SetDefinition(const std::string& settingDefinition)
{
  m_settingDefinition = settingDefinition;
}

void CMediaImportSource::CSettings::AddSimpleCondition(const std::string& condition)
{
  m_simpleConditions.insert(condition);
}

void CMediaImportSource::CSettings::AddComplexCondition(const std::string& name, const SettingConditionCheck& condition)
{
  m_complexConditions.insert(std::make_pair(name, condition));
}

void CMediaImportSource::CSettings::SetOptionsFiller(const std::string &settingId, IntegerSettingOptionsFiller optionsFiller, void* data /* = nullptr */)
{
  if (!IsLoaded() || settingId.empty())
    return;

  auto setting = GetSetting(settingId);
  if (setting == nullptr || setting->GetType() != SettingTypeInteger)
    return;

  static_cast<CSettingInt*>(setting)->SetOptionsFiller(optionsFiller, data);
}

void CMediaImportSource::CSettings::SetOptionsFiller(const std::string &settingId, StringSettingOptionsFiller optionsFiller, void* data /* = nullptr */)
{
  if (!IsLoaded() || settingId.empty())
    return;

  auto setting = GetSetting(settingId);
  if (setting == nullptr || setting->GetType() != SettingTypeString)
    return;

  static_cast<CSettingString*>(setting)->SetOptionsFiller(optionsFiller, data);
}

std::string CMediaImportSource::CSettings::ToXml() const
{
  if (!IsLoaded())
    return m_settingValues;

  CXBMCTinyXML xmlValues;
  if (!SaveValuesToXml(xmlValues))
    return m_settingValues;

  TiXmlPrinter printer;
  xmlValues.Accept(&printer);

  return printer.Str();
}

bool CMediaImportSource::CSettings::InitializeDefinitions()
{
  if (m_settingDefinition.empty())
    return false;

  CXBMCTinyXML xmlDefinition;
  if (!xmlDefinition.Parse(m_settingDefinition, TIXML_ENCODING_UTF8))
  {
    CLog::Log(LOGERROR, "CMediaImportSource: error loading settings definition, Line %d\n%s", xmlDefinition.ErrorRow(), xmlDefinition.ErrorDesc());
    return false;
  }

  if (xmlDefinition.RootElement() == nullptr)
    return false;

  return InitializeDefinitionsFromXml(xmlDefinition);
}

void CMediaImportSource::CSettings::InitializeControls()
{
  GetSettingsManager()->RegisterSettingControl("toggle", this);
  GetSettingsManager()->RegisterSettingControl("spinner", this);
  GetSettingsManager()->RegisterSettingControl("edit", this);
  GetSettingsManager()->RegisterSettingControl("button", this);
  GetSettingsManager()->RegisterSettingControl("list", this);
  GetSettingsManager()->RegisterSettingControl("slider", this);
  GetSettingsManager()->RegisterSettingControl("range", this);
  GetSettingsManager()->RegisterSettingControl("title", this);
}

void CMediaImportSource::CSettings::InitializeConditions()
{
  // add simple conditions
  for (const auto condition : m_simpleConditions)
    GetSettingsManager()->AddCondition(condition);

  // add more complex conditions
  for (const auto condition : m_complexConditions)
    GetSettingsManager()->AddCondition(condition.first, condition.second);
}
