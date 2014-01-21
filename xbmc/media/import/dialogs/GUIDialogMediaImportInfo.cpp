/*
 *      Copyright (C) 2014 Team XBMC
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

#include "GUIDialogMediaImportInfo.h"
#include "FileItem.h"
#include "filesystem/MediaImportDirectory.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/WindowIDs.h"
#include "input/Key.h"
#include "media/MediaType.h"
#include "media/import/IMediaImporter.h"
#include "media/import/MediaImportManager.h"
#include "settings/lib/SettingsManager.h"
#include "utils/log.h"
#include "utils/Variant.h"

#define CONTROL_LIST_AREA                                     CONTROL_SETTINGS_CUSTOM + 1

#define SETTING_SYNCHRONISATION_IMPORT_TRIGGER                "synchronisation.importtrigger"
#define SETTING_SYNCHRONISATION_UPDATE_ITEMS                  "synchronisation.updateimporteditems"
#define SETTING_SYNCHRONISATION_UPDATE_PLAYBACK_FROM_SOURCE   "synchronisation.updateplaybackmetadatafromsource"
#define SETTING_SYNCHRONISATION_UPDATE_METADATA_ON_SOURCE     "synchronisation.updatemetadataonsource"
#define SETTING_SYNCHRONISATION_UPDATE_PLAYBACK_ON_SOURCE     "synchronisation.updateplaybackmetadataonsource"

CGUIDialogMediaImportInfo::CGUIDialogMediaImportInfo()
  : CGUIDialogSettingsManualBase(WINDOW_DIALOG_MEDIAIMPORT_INFO, "DialogMediaImportInfo.xml")
  , m_item(std::make_shared<CFileItem>())
  , m_import(nullptr)
  , m_source(nullptr)
  , m_importer(nullptr)
{
  m_loadType = KEEP_IN_MEMORY;
}

CGUIDialogMediaImportInfo::~CGUIDialogMediaImportInfo()
{ }

bool CGUIDialogMediaImportInfo::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
    case GUI_MSG_WINDOW_DEINIT:
    {
      // get rid of the CMediaImport instance
      m_import.reset();
      // get rid of the CMediaImportSource instance
      m_source.reset();
      // get rid of the IMediaImporter instance
      m_importer.reset();

      break;
    }

    case GUI_MSG_CLICKED:
    {
      if (message.GetSenderId() == CONTROL_SETTINGS_CUSTOM_BUTTON)
      {
        OnResetSettings();
        return true;
      }

      break;
    }

    default:
        break;
  }

  return CGUIDialogSettingsManualBase::OnMessage(message);
}

bool CGUIDialogMediaImportInfo::OnAction(const CAction &action)
{
  if (action.GetID() == ACTION_SHOW_INFO)
  {
    OnOkay();
    Close();
    return true;
  }

  return CGUIDialogSettingsManualBase::OnAction(action);
}

bool CGUIDialogMediaImportInfo::ShowForMediaImport(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  CGUIDialogMediaImportInfo* dialog = (CGUIDialogMediaImportInfo*)g_windowManager.GetWindow(WINDOW_DIALOG_MEDIAIMPORT_INFO);
  if (!dialog)
    return false;

  if (!dialog->SetMediaImport(item))
    return false;

  dialog->Open();
  return dialog->IsConfirmed();
}

bool CGUIDialogMediaImportInfo::ShowForMediaImportSource(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  CGUIDialogMediaImportInfo* dialog = (CGUIDialogMediaImportInfo*)g_windowManager.GetWindow(WINDOW_DIALOG_MEDIAIMPORT_INFO);
  if (!dialog)
    return false;

  if (!dialog->SetMediaImportSource(item))
    return false;

  dialog->Open();
  return dialog->IsConfirmed();
}

void CGUIDialogMediaImportInfo::OnSettingChanged(const CSetting *setting)
{
  if (setting == nullptr)
    return;

  CGUIDialogSettingsManualBase::OnSettingChanged(setting);

  if (m_import == nullptr)
    return;

  CMediaImportSettings &importSettings = m_import->GetSettings();

  const std::string &settingId = setting->GetId();
  if (settingId == SETTING_SYNCHRONISATION_IMPORT_TRIGGER)
  {
    m_synchronisationImportTrigger = static_cast<const CSettingInt*>(setting)->GetValue();
    importSettings.SetImportTrigger(static_cast<MediaImportTrigger>(m_synchronisationImportTrigger));
  }
  else if (settingId == SETTING_SYNCHRONISATION_UPDATE_ITEMS)
  {
    m_synchronisationUpdateImportedMediaItems = static_cast<const CSettingBool*>(setting)->GetValue();
    importSettings.SetUpdateImportedMediaItems(m_synchronisationUpdateImportedMediaItems);
  }
  else if (settingId == SETTING_SYNCHRONISATION_UPDATE_PLAYBACK_FROM_SOURCE)
  {
    m_synchronisationUpdatePlaybackMetadataFromSource = static_cast<const CSettingBool*>(setting)->GetValue();
    importSettings.SetUpdatePlaybackMetadataFromSource(m_synchronisationUpdatePlaybackMetadataFromSource);
  }
  else if (settingId == SETTING_SYNCHRONISATION_UPDATE_METADATA_ON_SOURCE)
  {
    m_synchronisationUpdateMetadataOnSource = static_cast<const CSettingBool*>(setting)->GetValue();
    if (m_synchronisationUpdateMetadataOnSource)
      importSettings.SetUpdatePlaybackMetadataOnSource(m_synchronisationUpdatePlaybackMetadataOnSource);
    else
      importSettings.SetUpdatePlaybackMetadataOnSource(false);
  }
  else if (settingId == SETTING_SYNCHRONISATION_UPDATE_PLAYBACK_ON_SOURCE)
  {
    m_synchronisationUpdatePlaybackMetadataOnSource = static_cast<const CSettingBool*>(setting)->GetValue();
    importSettings.SetUpdatePlaybackMetadataOnSource(m_synchronisationUpdatePlaybackMetadataOnSource);
  }
}

void CGUIDialogMediaImportInfo::Save()
{
  if (m_import != nullptr)
  {
    if (CMediaImportManager::GetInstance().UpdateImport(*m_import))
      CLog::Log(LOGINFO, "CGUIDialogMediaImportInfo: settings for import from %s with media types %s saved", m_import->GetPath().c_str(), CMediaTypes::Join(m_import->GetMediaTypes()).c_str());
    else
      CLog::Log(LOGERROR, "CGUIDialogMediaImportInfo: failed to save settings for import from %s with media types %s", m_import->GetPath().c_str(), CMediaTypes::Join(m_import->GetMediaTypes()).c_str());
  }
  else if (m_source != nullptr)
  {
    bool success = false;
    if (m_importer->UnloadSourceSettings(*m_source))
      success = CMediaImportManager::GetInstance().UpdateSource(*m_source);

    if (success)
      CLog::Log(LOGINFO, "CGUIDialogMediaImportInfo: settings for source %s (%s) saved", m_source->GetFriendlyName().c_str(), m_source->GetIdentifier().c_str());
    else
      CLog::Log(LOGINFO, "CGUIDialogMediaImportInfo: failed to save settings for source %s (%s)", m_source->GetFriendlyName().c_str(), m_source->GetIdentifier().c_str());
  }
}

void CGUIDialogMediaImportInfo::SetupView()
{
  CGUIDialogSettingsManualBase::SetupView();

  // set heading
  SetHeading(m_source != nullptr ? 39200 : 39201);

  // set control labels
  SET_CONTROL_LABEL(CONTROL_SETTINGS_OKAY_BUTTON, 186);
  SET_CONTROL_LABEL(CONTROL_SETTINGS_CANCEL_BUTTON, 222);
  SET_CONTROL_LABEL(CONTROL_SETTINGS_CUSTOM_BUTTON, 409);
}

CSettingSection* CGUIDialogMediaImportInfo::GetSection()
{
  // for media sources with settings we provide a custom section
  if (m_source != nullptr && m_source->Settings().IsLoaded())
  {
    const auto sections = m_source->Settings().GetSections();
    if (!sections.empty())
      return sections.front();
  }

  return CGUIDialogSettingsManualBase::GetSection();
}

void CGUIDialogMediaImportInfo::InitializeSettings()
{
  // ATTENTION: do not generally call the base class method as for
  // media sources we want to provide our own settings section

  if (m_import != nullptr)
    InitializeMediaImportSettings();
  else if (m_source != nullptr)
    InitializeMediaImportSourceSettings();
}

CSettingsManager* CGUIDialogMediaImportInfo::GetSettingsManager() const
{
  // for media sources with settings we provide a custom section
  if (m_source != nullptr && m_source->Settings().IsLoaded())
    return m_source->Settings().GetSettingsManager();

  return CGUIDialogSettingsManualBase::GetSettingsManager();
}

void CGUIDialogMediaImportInfo::InitializeMediaImportSettings()
{
  CGUIDialogSettingsManualBase::InitializeSettings();

  if (m_importer == nullptr)
    return;

  CSettingCategory *categorySynchronisation = AddCategory("synchronisation", 39250);
  if (categorySynchronisation == nullptr)
  {
    CLog::Log(LOGERROR, "CGUIDialogMediaImportInfo: unable to setup settings");
    return;
  }

  CSettingGroup *groupImport = AddGroup(categorySynchronisation);
  if (groupImport == nullptr)
  {
    CLog::Log(LOGERROR, "CGUIDialogMediaImportInfo: unable to setup settings");
    return;
  }

  const CMediaImportSettings &importSettings = m_import->GetSettings();
  m_synchronisationImportTrigger = static_cast<int>(importSettings.GetImportTrigger());
  m_synchronisationUpdateImportedMediaItems = importSettings.UpdateImportedMediaItems();
  m_synchronisationUpdatePlaybackMetadataFromSource = importSettings.UpdatePlaybackMetadataFromSource();
  m_synchronisationUpdatePlaybackMetadataOnSource = importSettings.UpdatePlaybackMetadataOnSource();

  StaticIntegerSettingOptions entries;
  entries.push_back(std::make_pair(39305, MediaImportTriggerAuto));
  entries.push_back(std::make_pair(39306, MediaImportTriggerManual));
  AddSpinner(groupImport, SETTING_SYNCHRONISATION_IMPORT_TRIGGER, 39304, 0, m_synchronisationImportTrigger, entries);

  CSettingGroup *groupSynchronisation = AddGroup(categorySynchronisation);
  if (groupSynchronisation == nullptr)
  {
    CLog::Log(LOGERROR, "CGUIDialogMediaImportInfo: unable to setup settings");
    return;
  }

  AddToggle(groupSynchronisation, SETTING_SYNCHRONISATION_UPDATE_ITEMS, 39300, 0, m_synchronisationUpdateImportedMediaItems);

  CSettingBool *settingPlaybackFromSource =
    AddToggle(groupSynchronisation, SETTING_SYNCHRONISATION_UPDATE_PLAYBACK_FROM_SOURCE, 39301, 0, m_synchronisationUpdatePlaybackMetadataFromSource);
  // set the parent setting to SETTING_SYNCHRONISATION_UPDATE_ITEMS
  settingPlaybackFromSource->SetParent(SETTING_SYNCHRONISATION_UPDATE_ITEMS);
  // add a dependency on SETTING_SYNCHRONISATION_UPDATE_ITEMS (parent setting) being enabled
  CSettingDependency dependencySynchronisationUpdateItemsEnabled(SettingDependencyTypeEnable, GetSettingsManager());
  dependencySynchronisationUpdateItemsEnabled.And()
    ->Add(CSettingDependencyConditionPtr(new CSettingDependencyCondition(SETTING_SYNCHRONISATION_UPDATE_ITEMS, "true", SettingDependencyOperatorEquals, false, GetSettingsManager())));
  SettingDependencies depsSynchronisationUpdateItemsEnabled;
  depsSynchronisationUpdateItemsEnabled.push_back(dependencySynchronisationUpdateItemsEnabled);
  settingPlaybackFromSource->SetDependencies(depsSynchronisationUpdateItemsEnabled);

  const std::string &importPath = m_import->GetPath();

  bool canUpdatePlaybackMetadataOnSource = m_importer->CanUpdatePlaycountOnSource(importPath) ||
    m_importer->CanUpdateLastPlayedOnSource(importPath) ||
    m_importer->CanUpdateResumePositionOnSource(importPath);

  CSettingBool *settingUpdateMetadataOnSource =
    AddToggle(groupSynchronisation, SETTING_SYNCHRONISATION_UPDATE_METADATA_ON_SOURCE, 39302, 0, m_synchronisationUpdatePlaybackMetadataOnSource);
  settingUpdateMetadataOnSource->SetEnabled(canUpdatePlaybackMetadataOnSource);

  CSettingDependency dependencySynchronisationUpdateMetadataOnSourceEnabled(SettingDependencyTypeEnable, GetSettingsManager());
  dependencySynchronisationUpdateMetadataOnSourceEnabled.And()
    ->Add(CSettingDependencyConditionPtr(new CSettingDependencyCondition(SETTING_SYNCHRONISATION_UPDATE_METADATA_ON_SOURCE, "true", SettingDependencyOperatorEquals, false, GetSettingsManager())));
  SettingDependencies depsSynchronisationUpdateMetadataOnSourceEnabled;
  depsSynchronisationUpdateMetadataOnSourceEnabled.push_back(dependencySynchronisationUpdateMetadataOnSourceEnabled);

  CSettingBool *settingPlaycountOnSource =
    AddToggle(groupSynchronisation, SETTING_SYNCHRONISATION_UPDATE_PLAYBACK_ON_SOURCE, 39303, 0, m_synchronisationUpdatePlaybackMetadataOnSource);
  // set the parent setting to SETTING_SYNCHRONISATION_UPDATE_METADATA_ON_SOURCE
  settingPlaycountOnSource->SetParent(SETTING_SYNCHRONISATION_UPDATE_METADATA_ON_SOURCE);
  // disable the setting if the importer doesn't support this
  if (!canUpdatePlaybackMetadataOnSource)
    settingPlaycountOnSource->SetEnabled(false);
  else
  {
    // add a dependency on SETTING_SYNCHRONISATION_UPDATE_PLAYBACK_ON_SOURCE (parent setting) being enabled
    settingPlaycountOnSource->SetDependencies(depsSynchronisationUpdateMetadataOnSourceEnabled);
  }
}

void CGUIDialogMediaImportInfo::InitializeMediaImportSourceSettings()
{
  if (m_importer == nullptr)
    return;

  if (!m_importer->LoadSourceSettings(*m_source))
    return;

  // TODO
}

bool CGUIDialogMediaImportInfo::SetMediaImport(const CFileItemPtr &item)
{
  if (!item->HasProperty(PROPERTY_SOURCE_IDENTIFIER) ||
      !item->HasProperty(PROPERTY_IMPORT_PATH) ||
      !item->HasProperty(PROPERTY_IMPORT_MEDIATYPES))
    return false;

  std::string importPath = item->GetProperty(PROPERTY_IMPORT_PATH).asString();
  GroupedMediaTypes mediaTypes = CMediaTypes::Split(item->GetProperty(PROPERTY_IMPORT_MEDIATYPES).asString());
  if (importPath.empty() || mediaTypes.empty())
    return false;

  // get the import details
  m_import = std::make_shared<CMediaImport>();
  if (!CMediaImportManager::GetInstance().GetImport(importPath, mediaTypes, *m_import))
  {
    m_import.reset();
    return false;
  }

  // get a matching importer
  const auto importer = CMediaImportManager::GetInstance().GetImporter(m_import->GetPath());
  if (importer == nullptr)
  {
    m_import.reset();
    return false;
  }

  // create a specific importer
  m_importer.reset(importer->Create(*m_import));

  // copy the given item
  *m_item = *item;

  return true;
}

bool CGUIDialogMediaImportInfo::SetMediaImportSource(const CFileItemPtr &item)
{
  std::string sourceId = item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString();
  if (sourceId.empty())
    return false;

  m_source = std::make_shared<CMediaImportSource>(sourceId);
  if (!CMediaImportManager::GetInstance().GetSource(sourceId, *m_source))
  {
    m_source.reset();
    return false;
  }

  // get a matching importer
  const auto importer = CMediaImportManager::GetInstance().GetImporter(sourceId);
  if (importer == nullptr)
  {
    m_source.reset();
    return false;
  }

  // TODO: this is a bit hacky
  m_importer.reset(importer->Create(CMediaImport(m_source->GetIdentifier(), *m_source)));

  // copy the given item
  *m_item = *item;

  return true;
}
