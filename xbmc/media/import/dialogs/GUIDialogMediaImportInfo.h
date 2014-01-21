#pragma once
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

#include <memory>

#include "settings/dialogs/GUIDialogSettingsManualBase.h"
#include "view/GUIViewControl.h"

class CFileItemList;
class CMediaImport;
class CMediaImportSource;
class IMediaImporter;

class CGUIDialogMediaImportInfo : public CGUIDialogSettingsManualBase
{
public:
  CGUIDialogMediaImportInfo();
  virtual ~CGUIDialogMediaImportInfo();

  // specialization of CGUIControl
  virtual bool OnMessage(CGUIMessage& message) override;
  virtual bool OnAction(const CAction &action) override;

  static bool ShowForMediaImport(const CFileItemPtr &item);
  static bool ShowForMediaImportSource(const CFileItemPtr &item);

protected:
  // implementations of ISettingCallback
  virtual void OnSettingChanged(const CSetting *setting) override;

  // specialization of CGUIDialogSettingsBase
  virtual bool AllowResettingSettings() const override { return false; }
  virtual void Save() override;
  virtual void SetupView() override;

  // specialization of CGUIDialogSettingsManualBase
  virtual CSettingSection* GetSection() override;
  virtual void InitializeSettings() override;
  virtual CSettingsManager* GetSettingsManager() const override;

  void InitializeMediaImportSettings();
  void InitializeMediaImportSourceSettings();

  bool SetMediaImport(const CFileItemPtr &item);
  bool SetMediaImportSource(const CFileItemPtr &item);
  
  CFileItemPtr m_item;
  std::shared_ptr<CMediaImport> m_import;
  std::shared_ptr<CMediaImportSource> m_source;
  std::shared_ptr<const IMediaImporter> m_importer;

  // media import settings related
  int m_synchronisationImportTrigger;
  bool m_synchronisationUpdateImportedMediaItems;
  bool m_synchronisationUpdatePlaybackMetadataFromSource;
  bool m_synchronisationUpdateMetadataOnSource;
  bool m_synchronisationUpdatePlaybackMetadataOnSource;
};

