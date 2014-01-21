#pragma once

/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include <set>
#include <string>

#include "media/MediaType.h"
#include "media/import/MediaImportSource.h"
#include "windows/GUIMediaWindow.h"

class CGUIWindowMediaSourceBrowser : public CGUIMediaWindow
{
public:
  CGUIWindowMediaSourceBrowser();
  virtual ~CGUIWindowMediaSourceBrowser();

  // specialization of CGUIControl
  virtual bool OnMessage(CGUIMessage& message) override;
  
protected:
  // specialization of CGUIMediaWindow
  virtual bool GetDirectory(const std::string &strDirectory, CFileItemList &items) override;
  virtual std::string GetStartFolder(const std::string &dir) override;

  virtual void GetContextButtons(int itemNumber, CContextButtons &buttons) override;
  virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button) override;

  virtual bool OnClick(int iItem, const std::string &player = "") override;
  virtual void UpdateButtons() override;

  static std::string ShowAndGetMediaSourcesToImportFrom(const GroupedMediaTypes &mediaTypes = GroupedMediaTypes());
  static GroupedMediaTypes ShowAndGetMediaTypesToImport(const std::string &sourceId);
  static GroupedMediaTypes ShowAndGetMediaTypesToImport(const CMediaImportSource &source);
  static bool GetMediaTypesToImport(const MediaTypes &availableMediaTypes, CFileItemList &items);

  bool RefreshList(bool keepSelectedItem = true);

  bool OnSourceSynchronise(const CFileItemPtr &item);
  bool OnSourceInfo(const CFileItemPtr &item);
  bool OnSourceDelete(const CFileItemPtr &item);
  bool OnImportSynchronise(const CFileItemPtr &item);
  bool OnImportInfo(const CFileItemPtr &item);
  bool OnImportDelete(const CFileItemPtr &item);

  void OnSourceAdded(const std::string &sourceId);
  void OnSourceUpdated(const std::string &sourceId);
  void OnSourceRemoved(const std::string &sourceId);
  void OnImportAdded(const std::string &importPath, const GroupedMediaTypes &mediaTypes);
  void OnImportUpdated(const std::string &importPath, const GroupedMediaTypes &mediaTypes);
  void OnImportRemoved(const std::string &importPath, const GroupedMediaTypes &mediaTypes);
  void OnSourceIsActiveChanged(const std::string &sourceId, bool isactive);
  
  CFileItemPtr GetImportItem(const std::string &importPath, const GroupedMediaTypes &mediaTypes) const;
  CFileItemPtr GetSourceItem(const std::string &sourceId) const;
};

