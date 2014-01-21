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

#include <algorithm>

#include "GUIWindowMediaSourceBrowser.h"
#include "FileItem.h"
#include "GUIUserMessages.h"
#include "URL.h"
#include "dialogs/GUIDialogOK.h"
#include "dialogs/GUIDialogProgress.h"
#include "dialogs/GUIDialogSelect.h"
#include "dialogs/GUIDialogYesNo.h"
#include "filesystem/MediaImportDirectory.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"
#include "guilib/WindowIDs.h"
#include "input/Key.h"
#include "media/MediaType.h"
#include "media/import/MediaImportManager.h"
#include "media/import/dialogs/GUIDialogMediaImportInfo.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"

using namespace std;

CGUIWindowMediaSourceBrowser::CGUIWindowMediaSourceBrowser()
: CGUIMediaWindow(WINDOW_MEDIASOURCE_BROWSER, "MediaSourceBrowser.xml")
{ }

CGUIWindowMediaSourceBrowser::~CGUIWindowMediaSourceBrowser()
{ }

std::string CGUIWindowMediaSourceBrowser::ShowAndGetMediaSourcesToImportFrom(const GroupedMediaTypes &mediaTypes /* = GroupedMediaTypes() */)
{
  // get all available sources
  std::vector<CMediaImportSource> sources = CMediaImportManager::GetInstance().GetSources();

  for (std::vector<CMediaImportSource>::iterator source = sources.begin(); source != sources.end();)
  {
    // get all imports for the source
    auto imports = CMediaImportManager::GetInstance().GetImportsBySource(source->GetIdentifier());

    // put together all already imported media types for the source
    std::set<MediaType> importedMediaTypes;
    for (const auto& import : imports)
      importedMediaTypes.insert(import.GetMediaTypes().begin(), import.GetMediaTypes().end());

    const auto& availableMediaTypes = source->GetAvailableMediaTypes();
    if (std::all_of(availableMediaTypes.begin(), availableMediaTypes.end(),
          [&importedMediaTypes](const MediaType& mediaType) -> bool
          {
            return importedMediaTypes.find(mediaType) != importedMediaTypes.end();
          }))
    {
      sources.erase(source++);
      continue;
    }

    ++source;
  }

  if (sources.empty())
    return "";

  // show the select dialog with all the sources from which we can import
  CGUIDialogSelect* selectDialog = (CGUIDialogSelect*)g_windowManager.GetWindow(WINDOW_DIALOG_SELECT);
  if (selectDialog == nullptr)
    return "";

  selectDialog->Reset();
  selectDialog->SetHeading(39100);
  selectDialog->SetMultiSelection(false);

  for (const auto& source : sources)
  {
    CFileItem item(source.GetFriendlyName());
    item.SetPath(source.GetIdentifier()); // abuse the path for the media source's identification

    selectDialog->Add(item);
  }

  selectDialog->Open();

  if (!selectDialog->IsConfirmed() || selectDialog->GetSelectedFileItem() == nullptr)
    return "";

  return selectDialog->GetSelectedFileItem()->GetPath();
}

GroupedMediaTypes CGUIWindowMediaSourceBrowser::ShowAndGetMediaTypesToImport(const std::string &sourceID)
{
  GroupedMediaTypes mediaTypesToImport;

  if (sourceID.empty())
    return mediaTypesToImport;

  CMediaImportSource source(sourceID);
  if (!CMediaImportManager::GetInstance().GetSource(sourceID, source))
    return mediaTypesToImport;

  return ShowAndGetMediaTypesToImport(source);
}

GroupedMediaTypes CGUIWindowMediaSourceBrowser::ShowAndGetMediaTypesToImport(const CMediaImportSource &source)
{
  GroupedMediaTypes mediaTypesToImport;

  if (source.GetIdentifier().empty() || source.GetFriendlyName().empty() || source.GetAvailableMediaTypes().empty())
    return mediaTypesToImport;

  // put together a list of media types that are available for the source and haven't been imported yet
  MediaTypes unimportedMediaTypes = source.GetAvailableMediaTypes();
  std::vector<CMediaImport> imports = CMediaImportManager::GetInstance().GetImportsBySource(source.GetIdentifier());
  for (const auto& import : imports)
  {
    for (const auto& mediaType : import.GetMediaTypes())
      unimportedMediaTypes.erase(mediaType);
  }

  // show the select dialog with all the media types available for import
  CGUIDialogSelect* selectDialog = (CGUIDialogSelect*)g_windowManager.GetWindow(WINDOW_DIALOG_SELECT);
  if (selectDialog != nullptr)
  {
    selectDialog->Reset();
    selectDialog->SetHeading(39055);
    selectDialog->SetMultiSelection(false);

    CFileItemList items;
    if (!GetMediaTypesToImport(unimportedMediaTypes, items))
      return mediaTypesToImport;

    selectDialog->SetItems(items);
    selectDialog->Open();

    if (selectDialog->IsConfirmed() && selectDialog->GetSelectedFileItem() != nullptr)
      return StringUtils::Split(selectDialog->GetSelectedFileItem()->GetPath(), ",");
  }

  return mediaTypesToImport;
}

bool CGUIWindowMediaSourceBrowser::GetMediaTypesToImport(const MediaTypes &availableMediaTypes, CFileItemList &items)
{
  if (availableMediaTypes.empty())
    return false;

  std::vector<GroupedMediaTypes> supportedMediaTypes = CMediaImportManager::GetInstance().GetSupportedMediaTypesGrouped(availableMediaTypes);
  if (supportedMediaTypes.empty())
    return false;

  for (const auto& itSupportedMediaType : supportedMediaTypes)
  {
    CFileItemPtr pItem(new CFileItem(CMediaTypes::ToLabel(itSupportedMediaType)));
    pItem->SetPath(CMediaTypes::Join(itSupportedMediaType)); // abuse the path for the media type's identification

    items.Add(pItem);
  }

  return true;
}

bool CGUIWindowMediaSourceBrowser::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
    case GUI_MSG_WINDOW_INIT:
    {
      m_rootDir.AllowNonLocalSources(false);

      // is this the first time the window is opened?
      if (m_vecItems->GetPath() == "?" && message.GetStringParam().empty())
        m_vecItems->SetPath("");
      break;
    }

    case GUI_MSG_WINDOW_DEINIT:
    {
      break;
    }

    case GUI_MSG_CLICKED:
    {
      int iControl = message.GetSenderId();
      if (m_viewControl.HasControl(iControl))  // list/thumb control
      {
        // get selected item
        int itemIndex = m_viewControl.GetSelectedItem();
        int actionId = message.GetParam1();

        if (actionId == ACTION_SHOW_INFO ||
            actionId == ACTION_DELETE_ITEM)
        {
          CFileItemPtr item = m_vecItems->Get(itemIndex);
          if (item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).empty())
            return false;

          if (item->GetProperty(PROPERTY_IMPORT_PATH).empty())
          {
            if (actionId == ACTION_SHOW_INFO)
              return OnSourceInfo(item);
            else if (actionId == ACTION_DELETE_ITEM)
              return OnSourceDelete(item);
          }
          else
          {
            if (actionId == ACTION_SHOW_INFO)
              return OnImportInfo(item);
            else if (actionId == ACTION_DELETE_ITEM)
              return OnImportDelete(item);
          }
        }
      }
      break;
    }

    case GUI_MSG_NOTIFY_ALL:
    {
      CGUIListItemPtr item = message.GetItem();
      if (item == nullptr)
        break;

      switch (message.GetParam1())
      {
        case GUI_MSG_SOURCE_ADDED:
        {
          OnSourceAdded(message.GetItem()->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString());
          return true;
        }

        case GUI_MSG_SOURCE_UPDATED:
        {
          OnSourceUpdated(message.GetItem()->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString());
          return true;
        }

        case GUI_MSG_SOURCE_REMOVED:
        {
          OnSourceRemoved(message.GetItem()->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString());
          return true;
        }

        case GUI_MSG_SOURCE_ACTIVE_CHANGED:
        {
          OnSourceIsActiveChanged(message.GetItem()->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString(), message.GetParam1() > 0);
          return true;
        }

        case GUI_MSG_IMPORT_ADDED:
        {
          OnImportAdded(item->GetProperty(PROPERTY_IMPORT_PATH).asString(), CMediaTypes::Split(item->GetProperty(PROPERTY_IMPORT_MEDIATYPES).asString()));
          return true;
        }

        case GUI_MSG_IMPORT_UPDATED:
        {
          OnImportUpdated(item->GetProperty(PROPERTY_IMPORT_PATH).asString(), CMediaTypes::Split(item->GetProperty(PROPERTY_IMPORT_MEDIATYPES).asString()));
          return true;
        }

        case GUI_MSG_IMPORT_REMOVED:
        {
          OnImportRemoved(item->GetProperty(PROPERTY_IMPORT_PATH).asString(), CMediaTypes::Split(item->GetProperty(PROPERTY_IMPORT_MEDIATYPES).asString()));
          return true;
        }

        default:
          break;
      }
    }

    default:
      break;
  }
  return CGUIMediaWindow::OnMessage(message);
}

bool CGUIWindowMediaSourceBrowser::RefreshList(bool keepSelectedItem /* = true */)
{
  std::string currentItemPath;
  if (keepSelectedItem)
  {
    int itemIndex = m_viewControl.GetSelectedItem();
    if (itemIndex >= 0 && itemIndex < m_vecItems->Size())
      currentItemPath = m_vecItems->Get(itemIndex)->GetPath();
  }

  bool result = Refresh(true);

  if (keepSelectedItem && !currentItemPath.empty())
    m_viewControl.SetSelectedItem(currentItemPath);

  return result;
}

bool CGUIWindowMediaSourceBrowser::OnSourceSynchronise(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  if (CMediaImportManager::GetInstance().Import(item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString()))
    return true;

  CGUIDialogOK::ShowAndGetInput(39110, StringUtils::Format(g_localizeStrings.Get(39111).c_str(), item->GetLabel().c_str()));
  return false;
}

bool CGUIWindowMediaSourceBrowser::OnSourceInfo(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  return CGUIDialogMediaImportInfo::ShowForMediaImportSource(item);
}

bool CGUIWindowMediaSourceBrowser::OnSourceDelete(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  CGUIDialogYesNo* pDialog = (CGUIDialogYesNo*)g_windowManager.GetWindow(WINDOW_DIALOG_YES_NO);
  if (pDialog == nullptr)
    return false;

  pDialog->SetHeading(39102);
  pDialog->SetText(StringUtils::Format(g_localizeStrings.Get(39103).c_str(), item->GetLabel().c_str()));

  pDialog->Open();

  if (!pDialog->IsConfirmed())
    return false;

  CMediaImportManager::GetInstance().RemoveSource(item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString());
  return true;
}

bool CGUIWindowMediaSourceBrowser::OnImportSynchronise(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  if (CMediaImportManager::GetInstance().Import(item->GetProperty(PROPERTY_IMPORT_PATH).asString(),
    CMediaTypes::Split(item->GetProperty(PROPERTY_IMPORT_MEDIATYPES).asString())))
    return true;

  CGUIDialogOK::ShowAndGetInput(39112, StringUtils::Format(g_localizeStrings.Get(39113).c_str(), item->GetLabel().c_str()));
  return false;
}

bool CGUIWindowMediaSourceBrowser::OnImportInfo(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  return CGUIDialogMediaImportInfo::ShowForMediaImport(item);
}

bool CGUIWindowMediaSourceBrowser::OnImportDelete(const CFileItemPtr &item)
{
  if (item == nullptr)
    return false;

  CGUIDialogYesNo* pDialog = (CGUIDialogYesNo*)g_windowManager.GetWindow(WINDOW_DIALOG_YES_NO);
  if (pDialog == nullptr)
    return false;

  std::string sourceID = item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString();
  std::string importPath = item->GetProperty(PROPERTY_IMPORT_PATH).asString();
  GroupedMediaTypes mediaTypes = CMediaTypes::Split(item->GetProperty(PROPERTY_IMPORT_MEDIATYPES).asString());

  pDialog->SetHeading(39104);
  pDialog->SetText(StringUtils::Format(g_localizeStrings.Get(39103).c_str(), item->GetLabel().c_str()));

  pDialog->Open();

  if (!pDialog->IsConfirmed())
    return false;

  CMediaImportManager::GetInstance().RemoveImport(importPath, mediaTypes);
  return true;
}

void CGUIWindowMediaSourceBrowser::OnSourceAdded(const std::string &sourceId)
{
  if (m_vecItems->GetContent() == "sources")
    RefreshList(true);
}

void CGUIWindowMediaSourceBrowser::OnSourceUpdated(const std::string &sourceId)
{
  if (m_vecItems->GetContent() == "sources")
  {
    // only refresh the list if the updated source is part of it
    CFileItemPtr item = GetSourceItem(sourceId);
    if (item != nullptr)
      RefreshList(true);
  }
}

void CGUIWindowMediaSourceBrowser::OnSourceRemoved(const std::string &sourceId)
{
  if (m_vecItems->GetContent() == "sources")
  {
    // only refresh the list if the removed source is part of it
    CFileItemPtr item = GetSourceItem(sourceId);
    if (item != nullptr)
    {
      int selectedItemIndex = m_viewControl.GetSelectedItem();
      // only update the selected item index when the deleted item is focused
      if (m_vecItems->Get(selectedItemIndex) != item)
        selectedItemIndex = -1;

      RefreshList(false);

      if (selectedItemIndex >= 0)
        m_viewControl.SetSelectedItem(selectedItemIndex);
    }
  }
}

void CGUIWindowMediaSourceBrowser::OnImportAdded(const std::string &importPath, const GroupedMediaTypes &mediaTypes)
{
  if (m_vecItems->GetContent() == "imports")
    RefreshList(true);
}

void CGUIWindowMediaSourceBrowser::OnImportUpdated(const std::string &importPath, const GroupedMediaTypes &mediaTypes)
{
  if (m_vecItems->GetContent() == "imports")
  {
    // only refresh the list if the updated import is part of it
    CFileItemPtr item = GetImportItem(importPath, mediaTypes);
    if (item != nullptr)
      RefreshList(true);
  }
}

void CGUIWindowMediaSourceBrowser::OnImportRemoved(const std::string &importPath, const GroupedMediaTypes &mediaTypes)
{
  if (m_vecItems->GetContent() == "imports")
  {
    // only refresh the list if the removed import is part of it
    CFileItemPtr item = GetImportItem(importPath, mediaTypes);
    if (item != nullptr)
    {
      int selectedItemIndex = m_viewControl.GetSelectedItem();
      // only update the selected item index when the deleted item is focused
      if (m_vecItems->Get(selectedItemIndex) != item)
        selectedItemIndex = -1;

      RefreshList(false);

      if (selectedItemIndex >= 0)
        m_viewControl.SetSelectedItem(selectedItemIndex);
    }
  }
}

void CGUIWindowMediaSourceBrowser::OnSourceIsActiveChanged(const std::string &sourceId, bool isactive)
{
  if (m_vecItems->GetContent() == "sources")
  {
    CFileItemPtr item = GetSourceItem(sourceId);
    if (item != nullptr)
      item->SetProperty(PROPERTY_SOURCE_ISACTIVE, isactive);
    else
      RefreshList(true);
  }
  else if (m_vecItems->GetContent() == "imports")
  {
    for (int index = 0; index < m_vecItems->Size(); index++)
    {
      CFileItemPtr item = m_vecItems->Get(index);
      if (item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString() == sourceId)
        item->SetProperty(PROPERTY_SOURCE_ISACTIVE, isactive);
    }
  }
}

CFileItemPtr CGUIWindowMediaSourceBrowser::GetImportItem(const std::string &importPath, const GroupedMediaTypes &mediaTypes) const
{
  for (int index = 0; index < m_vecItems->Size(); index++)
  {
    CFileItemPtr item = m_vecItems->Get(index);
    if (item->IsParentFolder())
      continue;

    if (item->GetProperty(PROPERTY_IMPORT_PATH).asString() == importPath &&
        CMediaTypes::Split(item->GetProperty(PROPERTY_IMPORT_MEDIATYPES).asString()) == mediaTypes)
      return item;
  }

  return CFileItemPtr();
}

CFileItemPtr CGUIWindowMediaSourceBrowser::GetSourceItem(const std::string &sourceId) const
{
  for (int index = 0; index < m_vecItems->Size(); index++)
  {
    CFileItemPtr item = m_vecItems->Get(index);
    if (item->IsParentFolder())
      continue;

    if (item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString() == sourceId)
      return item;
  }

  return CFileItemPtr();
}

bool CGUIWindowMediaSourceBrowser::GetDirectory(const std::string& strDirectory, CFileItemList& items)
{
  if (!CGUIMediaWindow::GetDirectory(strDirectory, items))
    return false;

  CURL url(strDirectory);
  // "Add import" button for imports by media provider views
  if (items.GetContent() == "imports")
  {
    bool addNewImport = false;
    std::string addImportPath = "newimport://";
    if (StringUtils::StartsWith(url.GetFileName(), "all") || StringUtils::StartsWith(url.GetFileName(), "mediatypes"))
    {
      GroupedMediaTypes mediaTypes;

      bool hasMediaTypes = StringUtils::StartsWith(url.GetFileName(), "mediatypes");
      if (hasMediaTypes)
      {
        std::string mediaTypesDir = strDirectory;
        URIUtils::RemoveSlashAtEnd(mediaTypesDir);
        std::string strMediaTypes = URIUtils::GetFileName(mediaTypesDir);
        strMediaTypes = CURL::Decode(strMediaTypes);
        mediaTypes = CMediaTypes::Split(strMediaTypes);
      }

      if (!mediaTypes.empty())
        addImportPath += "mediatypes/" + CURL::Encode(CMediaTypes::Join(mediaTypes));

      auto sources = CMediaImportManager::GetInstance().GetSources();
      for (const auto& source : sources)
      {
        auto availableMediaTypes = source.GetAvailableMediaTypes();
        // check if the source supports the media types
        bool allMediaTypesSupported = std::all_of(mediaTypes.begin(), mediaTypes.end(),
          [&availableMediaTypes](const MediaType& mediaType) -> bool
          {
            return availableMediaTypes.find(mediaType) != availableMediaTypes.end();
          });

        if (!allMediaTypesSupported)
          continue;

        auto imports = CMediaImportManager::GetInstance().GetImportsBySource(source.GetIdentifier());
        if (hasMediaTypes)
        {
          bool allMediaTypesImported = false;
          for (const auto& import : imports)
          {
            // check if the source supports the media types
            allMediaTypesImported |= std::any_of(mediaTypes.begin(), mediaTypes.end(),
              [&import](const MediaType& mediaType) -> bool
            {
              return std::find(import.GetMediaTypes().begin(), import.GetMediaTypes().end(), mediaType) != import.GetMediaTypes().end();
            });

            if (allMediaTypesImported)
              break;
          }

          if (!allMediaTypesImported)
          {
            addNewImport = true;
            break;
          }
        }
        else
        {
          std::set<MediaType> importedMediaTypes;
          for (const auto& import : imports)
            importedMediaTypes.insert(import.GetMediaTypes().begin(), import.GetMediaTypes().end());

          if (importedMediaTypes != availableMediaTypes)
          {
            addNewImport = true;
            break;
          }
        }
      }
    }
    else if (StringUtils::StartsWith(url.GetFileName(), "sources"))
    {
      std::string sourceDir = strDirectory;
      URIUtils::RemoveSlashAtEnd(sourceDir);
      std::string sourceID = URIUtils::GetFileName(sourceDir);
      sourceID = CURL::Decode(sourceID);

      addImportPath += "source/" + CURL::Encode(sourceID);
      CMediaImportSource source(sourceID);
      if (CMediaImportManager::GetInstance().GetSource(sourceID, source))
      {
        // count the number of media types already being imported for a source
        size_t importedMediaTypesCount = 0;
        for (const auto& import : CMediaImportManager::GetInstance().GetImportsBySource(sourceID))
          importedMediaTypesCount += import.GetMediaTypes().size();

        // check if all media types are already being imported or not
        if (importedMediaTypesCount < source.GetAvailableMediaTypes().size())
          addNewImport = true;
      }
    }

    if (addNewImport && !items.Contains(addImportPath))
    {
      CFileItemPtr addImport(new CFileItem(addImportPath, false));
      addImport->SetLabel(g_localizeStrings.Get(39106));
      addImport->SetLabelPreformated(true);
      addImport->SetSpecialSort(SortSpecialOnBottom);
      items.Add(addImport);
    }
  }

  return true;
}

std::string CGUIWindowMediaSourceBrowser::GetStartFolder(const std::string &dir)
{
  if (StringUtils::StartsWithNoCase(dir, "import://"))
    return dir;

  return CGUIMediaWindow::GetStartFolder(dir);
}

void CGUIWindowMediaSourceBrowser::GetContextButtons(int itemNumber, CContextButtons& buttons)
{
  CFileItemPtr item = m_vecItems->Get(itemNumber);
  bool isSource = !item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).empty() && !item->HasProperty(PROPERTY_IMPORT_PATH);
  bool isImport = !item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).empty() && item->HasProperty(PROPERTY_IMPORT_PATH) && item->HasProperty(PROPERTY_IMPORT_MEDIATYPES);

  if (isSource || isImport)
  {
    // only allow synchronisation of the source is active and has imports
    if (item->GetProperty(PROPERTY_SOURCE_ISACTIVE).asBoolean() &&
       ((isSource && CMediaImportManager::GetInstance().HasImports(item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).asString())) || isImport))
       buttons.Add(CONTEXT_BUTTON_SCAN, g_localizeStrings.Get(39107));
    buttons.Add(CONTEXT_BUTTON_INFO, g_localizeStrings.Get(19033));
    buttons.Add(CONTEXT_BUTTON_DELETE, g_localizeStrings.Get(15015));
  }
}

bool CGUIWindowMediaSourceBrowser::OnContextButton(int itemNumber, CONTEXT_BUTTON button)
{
  CFileItemPtr item = m_vecItems->Get(itemNumber);
  bool isSource = !item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).empty() && !item->HasProperty(PROPERTY_IMPORT_PATH);

  switch (button)
  {
    case CONTEXT_BUTTON_SCAN:
      if (item->GetProperty(PROPERTY_SOURCE_ISACTIVE).asBoolean())
      {
        if (isSource)
          return OnSourceSynchronise(item);
        else
          return OnImportSynchronise(item);
      }
      break;

    case CONTEXT_BUTTON_INFO:
      if (isSource)
        return OnSourceInfo(item);
      else
        return OnImportInfo(item);
      break;

    case CONTEXT_BUTTON_DELETE:
    {
      if (isSource)
        return OnSourceDelete(item);
      else
        return OnImportDelete(item);
      break;
    }

    default:
      break;
  }

  return CGUIMediaWindow::OnContextButton(itemNumber, button);
}

bool CGUIWindowMediaSourceBrowser::OnClick(int iItem, const std::string &player /* = "" */)
{
  CFileItemPtr item = m_vecItems->Get(iItem);
  CURL url(item->GetPath());

  if (url.GetProtocol() == "newimport")
  {
    std::string sourceID;
    GroupedMediaTypes mediaTypesToImport;

    // parse potential URL options
    if (url.GetHostName() == "source")
      sourceID = CURL::Decode(url.GetFileName());
    else if (url.GetHostName() == "mediatypes")
      mediaTypesToImport = CMediaTypes::Split(CURL::Decode(url.GetFileName()));

    if (sourceID.empty())
    {
      // ask the user from which media source to import
      sourceID = ShowAndGetMediaSourcesToImportFrom(mediaTypesToImport);

      if (sourceID.empty())
        return true;
    }

    if (mediaTypesToImport.empty())
    {
      // ask the user what media type to add as an import
      mediaTypesToImport = ShowAndGetMediaTypesToImport(sourceID);

      if (mediaTypesToImport.empty())
        return true;
    }

    // add the new import (they aren't automatically synchronised)
    if (!CMediaImportManager::GetInstance().AddRecursiveImport(sourceID, sourceID, mediaTypesToImport))
    {
      CGUIDialogOK::ShowAndGetInput(39108, g_localizeStrings.Get(39109));
      return false;
    }

    // show the info dialog for the new import
    CMediaImport import;
    if (CMediaImportManager::GetInstance().GetImport(sourceID, mediaTypesToImport, import))
    {
      CFileItemPtr importItem = XFILE::CMediaImportDirectory::FileItemFromMediaImport(import, m_vecItems->GetPath());
      if (importItem != nullptr)
      {
        // refresh the list so that the new import is listed
        RefreshList(false);

        // now select the new import so that it will be available to skins in the import info dialog
        m_viewControl.SetSelectedItem(importItem->GetPath());

        // and now finally open the import info dialog
        OnImportInfo(importItem);

        // now see if the add import button is still there and if so re-select it
        if (m_vecItems->Contains(item->GetPath()))
          m_viewControl.SetSelectedItem(item->GetPath());
      }
    }

    // now try to synchronise the newly added import
    CMediaImportManager::GetInstance().Import(sourceID, mediaTypesToImport);

    return true;
  }

  if (!item->m_bIsFolder)
  {
    // if the item is a media import, show the info dialog
    if (!item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).empty() && item->HasProperty(PROPERTY_IMPORT_PATH) && item->HasProperty(PROPERTY_IMPORT_MEDIATYPES))
      return OnImportInfo(item);
    // if the item is a media provider, show the info dialog
    else if (!item->GetProperty(PROPERTY_SOURCE_IDENTIFIER).empty() && !item->HasProperty(PROPERTY_IMPORT_PATH))
      return OnSourceInfo(item);
  }

  return CGUIMediaWindow::OnClick(iItem);
}

void CGUIWindowMediaSourceBrowser::UpdateButtons()
{
  CGUIMediaWindow::UpdateButtons();
}
