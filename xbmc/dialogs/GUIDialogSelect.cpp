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

#include "GUIDialogSelect.h"
#include "guilib/GUIWindowManager.h"
#include "FileItem.h"
#include "guilib/GUIEditControl.h"
#include "guilib/Key.h"
#include "guilib/LocalizeStrings.h"
#include "utils/StringUtils.h"

#define CONTROL_HEADING       1
#define CONTROL_LIST          3
#define CONTROL_NUMBEROFFILES 2
#define CONTROL_BUTTON        5
#define CONTROL_DETAILS       6
#define CONTROL_INPUT         7
#define CONTROL_BUTTON_ADD    8

CGUIDialogSelect::CGUIDialogSelect(void)
    : CGUIDialogBoxBase(WINDOW_DIALOG_SELECT, "DialogSelect.xml")
{
  m_bButtonEnabled = false;
  m_buttonString = -1;
  m_useDetails = false;
  m_vecList = new CFileItemList;
  m_unfilteredList = new CFileItemList;
  m_selectedItems = new CFileItemList;
  m_multiSelection = false;
  m_iSelected = -1;
  m_bAllowNewItem = false;
  m_loadType = KEEP_IN_MEMORY;
}

CGUIDialogSelect::~CGUIDialogSelect(void)
{
  delete m_vecList;
  delete m_unfilteredList;
  delete m_selectedItems;
}

bool CGUIDialogSelect::OnMessage(CGUIMessage& message)
{
  switch ( message.GetMessage() )
  {
  case GUI_MSG_WINDOW_DEINIT:
    {
      CGUIDialog::OnMessage(message);
      m_viewControl.Clear();

      m_bButtonEnabled = false;
      m_useDetails = false;
      m_multiSelection = false;
      m_bAllowNewItem = false;

      // construct selected items list
      m_selectedItems->Clear();
      m_iSelected = -1;
      for (int i = 0 ; i < m_unfilteredList->Size() ; i++)
      {
        CFileItemPtr item = m_unfilteredList->Get(i);
        if (item->IsSelected())
        {
          m_selectedItems->Add(item);
          if (m_iSelected == -1)
            m_iSelected = i;
        }
      }

      m_vecList->Clear();
      m_unfilteredList->Clear();

      m_buttonString = -1;
      SET_CONTROL_LABEL(CONTROL_BUTTON, "");
      return true;
    }
    break;

  case GUI_MSG_WINDOW_INIT:
    {
      m_bButtonPressed = false;
      m_bConfirmed = false;
      m_strNewItem.clear();
      CGUIDialog::OnMessage(message);
      return true;
    }
    break;


  case GUI_MSG_CLICKED:
    {
      int iControl = message.GetSenderId();
      if (m_viewControl.HasControl(CONTROL_LIST))
      {
        int iAction = message.GetParam1();
        if (ACTION_SELECT_ITEM == iAction || ACTION_MOUSE_LEFT_CLICK == iAction)
        {
          int iSelected = m_viewControl.GetSelectedItem();
          if(iSelected >= 0 && iSelected < (int)m_vecList->Size())
          {
            CFileItemPtr item(m_vecList->Get(iSelected));
            if (m_multiSelection)
              item->Select(!item->IsSelected());
            else
            {
              for (int i = 0 ; i < m_vecList->Size() ; i++)
                m_vecList->Get(i)->Select(false);
              item->Select(true);
              m_bConfirmed = true;
              Close();
            }
          }
        }
      }
      if (CONTROL_BUTTON == iControl)
      {
        m_iSelected = -1;
        m_bButtonPressed = true;
        if (m_multiSelection)
          m_bConfirmed = true;
        Close();
      }
      else if (iControl == CONTROL_INPUT)
      {
        CGUIEditControl *control = (CGUIEditControl*)GetControl(iControl);
        if (control == NULL)
          break;

        CStdString input = control->GetLabel2();
        m_vecList->Clear();

        for (int index = 0; index < m_unfilteredList->Size(); index++)
        {
          const CFileItemPtr& item = m_unfilteredList->Get(index);
          if (StringUtils::StartsWith(item->GetLabel(), input))
            m_vecList->Add(item);
        }
        m_viewControl.SetItems(*m_vecList);
        CONTROL_ENABLE_ON_CONDITION(CONTROL_BUTTON_ADD, m_bAllowNewItem && !control->GetLabel2().empty());
      }
      else if (iControl == CONTROL_BUTTON_ADD)
      {
        CGUIEditControl *control = (CGUIEditControl*)GetControl(CONTROL_INPUT);
        if (!m_bAllowNewItem || control == NULL || control->GetLabel2().empty())
        {
          CONTROL_DISABLE(CONTROL_BUTTON_ADD);
          break;
        }

        m_strNewItem = control->GetLabel2();
        m_bConfirmed = true;
        Close();
      }
    }
    break;
  case GUI_MSG_SETFOCUS:
    {
      // make sure the additional button is focused in case the list is empty
      // (otherwise it is impossible to navigate to the additional button)
      if (m_vecList->IsEmpty() && m_bButtonEnabled &&
          m_viewControl.HasControl(message.GetControlId()))
      {
        SET_CONTROL_FOCUS(CONTROL_BUTTON, 0);
        return true;
      }
      if (m_viewControl.HasControl(message.GetControlId()) && m_viewControl.GetCurrentControl() != message.GetControlId())
      {
        m_viewControl.SetFocused();
        return true;
      }
    }
    break;
  }

  return CGUIDialog::OnMessage(message);
}

bool CGUIDialogSelect::OnBack(int actionID)
{
  m_iSelected = -1;
  m_selectedItems->Clear();
  m_bConfirmed = false;
  m_strNewItem.clear();
  return CGUIDialog::OnBack(actionID);
}

void CGUIDialogSelect::Reset()
{
  m_bButtonEnabled = false;
  m_useDetails = false;
  m_multiSelection = false;
  m_iSelected = -1;
  m_bAllowNewItem = false;
  m_strNewItem.clear();
  m_vecList->Clear();
  m_unfilteredList->Clear();
  m_selectedItems->Clear();
}

int CGUIDialogSelect::Add(const CStdString& strLabel)
{
  CFileItemPtr pItem(new CFileItem(strLabel));
  m_unfilteredList->Add(pItem);
  return m_unfilteredList->Size() - 1;
}

void CGUIDialogSelect::Add(const CFileItemList& items)
{
  for (int i=0;i<items.Size();++i)
  {
    CFileItemPtr item = items[i];
    Add(item.get());
  }
}

int CGUIDialogSelect::Add(const CFileItem* pItem)
{
  CFileItemPtr item(new CFileItem(*pItem));
  m_unfilteredList->Add(item);
  return m_unfilteredList->Size() - 1;
}

void CGUIDialogSelect::SetItems(CFileItemList* pList)
{
  // need to make internal copy of list to be sure dialog is owner of it
  m_unfilteredList->Clear();
  if (pList)
    m_unfilteredList->Copy(*pList);
}

int CGUIDialogSelect::GetSelectedLabel() const
{
  return m_iSelected;
}

const CFileItemPtr CGUIDialogSelect::GetSelectedItem()
{
  return m_selectedItems->Size() > 0 ? m_selectedItems->Get(0) : CFileItemPtr(new CFileItem);
}

const CStdString& CGUIDialogSelect::GetSelectedLabelText()
{
  return GetSelectedItem()->GetLabel();
}

const CFileItemList& CGUIDialogSelect::GetSelectedItems() const
{
  return *m_selectedItems;
}

void CGUIDialogSelect::EnableButton(bool enable, int string)
{
  m_bButtonEnabled = enable;
  m_buttonString = string;

  if (IsActive())
    SetupButton();
}

bool CGUIDialogSelect::IsButtonPressed()
{
  return m_bButtonPressed;
}

void CGUIDialogSelect::AllowNewItem(bool allow)
{
  m_bAllowNewItem = allow;
  if (!m_bAllowNewItem)
    m_strNewItem.clear();

  if (IsActive())
    SetupButton();
}

void CGUIDialogSelect::Sort(bool bSortOrder /*=true*/)
{
  m_unfilteredList->Sort(SortByLabel, bSortOrder ? SortOrderAscending : SortOrderDescending);
}

void CGUIDialogSelect::SetSelected(int iSelected)
{
  if (iSelected < 0 || iSelected >= (int)m_unfilteredList->Size() ||
      m_unfilteredList->Get(iSelected).get() == NULL) 
    return;

  // only set m_iSelected if there is no multi-select
  // or if it doesn't have a valid value yet
  // or if the current value is bigger than the new one
  // so that we always focus the item nearest to the beginning of the list
  if (!m_multiSelection || m_iSelected < 0 || m_iSelected > iSelected)
    m_iSelected = iSelected;
  m_unfilteredList->Get(iSelected)->Select(true);
  m_selectedItems->Add(m_unfilteredList->Get(iSelected));
}

void CGUIDialogSelect::SetSelected(const CStdString &strSelectedLabel)
{
  if (strSelectedLabel.empty())
    return;

  for (int index = 0; index < m_unfilteredList->Size(); index++)
  {
    if (strSelectedLabel.Equals(m_unfilteredList->Get(index)->GetLabel()))
    {
      SetSelected(index);
      return;
    }
  }
}

void CGUIDialogSelect::SetSelected(std::vector<int> selectedIndexes)
{
  if (selectedIndexes.empty())
    return;

  for (std::vector<int>::const_iterator it = selectedIndexes.begin(); it != selectedIndexes.end(); it++)
    SetSelected(*it);
}

void CGUIDialogSelect::SetSelected(const std::vector<CStdString> &selectedLabels)
{
  if (selectedLabels.empty())
    return;

  for (std::vector<CStdString>::const_iterator it = selectedLabels.begin(); it != selectedLabels.end(); it++)
    SetSelected(*it);
}

void CGUIDialogSelect::SetUseDetails(bool useDetails)
{
  m_useDetails = useDetails;
}

void CGUIDialogSelect::SetMultiSelection(bool multiSelection)
{
  m_multiSelection = multiSelection;
}

CGUIControl *CGUIDialogSelect::GetFirstFocusableControl(int id)
{
  if (m_viewControl.HasControl(id))
    id = m_viewControl.GetCurrentControl();
  return CGUIDialogBoxBase::GetFirstFocusableControl(id);
}

void CGUIDialogSelect::OnWindowLoaded()
{
  CGUIDialogBoxBase::OnWindowLoaded();
  m_viewControl.Reset();
  m_viewControl.SetParentWindow(GetID());
  m_viewControl.AddView(GetControl(CONTROL_LIST));
  m_viewControl.AddView(GetControl(CONTROL_DETAILS));
}

void CGUIDialogSelect::OnInitWindow()
{
  m_vecList->Assign(*m_unfilteredList);
  m_viewControl.SetItems(*m_vecList);
  m_selectedItems->Clear();
  if (m_iSelected == -1)
  {
    for(int i = 0 ; i < m_vecList->Size(); i++)
    {
      if (m_vecList->Get(i)->IsSelected())
      {
        m_iSelected = i;
        break;
      }
    }
  }
  m_viewControl.SetCurrentView(m_useDetails ? CONTROL_DETAILS : CONTROL_LIST);

  CStdString items = StringUtils::Format("%i %s", m_vecList->Size(), g_localizeStrings.Get(127).c_str());
  SET_CONTROL_LABEL(CONTROL_NUMBEROFFILES, items);
  
  if (m_multiSelection)
    EnableButton(true, 186);

  SetupButton();
  CGUIDialogBoxBase::OnInitWindow();

  // reset the value of the input control
  CGUIEditControl *inputControl = (CGUIEditControl*)GetControl(CONTROL_INPUT);
  if (inputControl != NULL)
    inputControl->SetLabel2("");

  // if m_iSelected < 0 focus first item
  m_viewControl.SetSelectedItem(std::max(m_iSelected, 0));
}

void CGUIDialogSelect::OnWindowUnload()
{
  CGUIDialog::OnWindowUnload();
  m_viewControl.Reset();
}

void CGUIDialogSelect::SetupButton()
{
  if (m_bButtonEnabled)
  {
    SET_CONTROL_LABEL(CONTROL_BUTTON, g_localizeStrings.Get(m_buttonString));
    SET_CONTROL_VISIBLE(CONTROL_BUTTON);
  }
  else
    SET_CONTROL_HIDDEN(CONTROL_BUTTON);

  CGUIEditControl *inputControl = (CGUIEditControl*)GetControl(CONTROL_INPUT);
  if (inputControl != NULL)
  {
    SET_CONTROL_LABEL(CONTROL_BUTTON_ADD, g_localizeStrings.Get(15019));
    SET_CONTROL_VISIBLE(CONTROL_BUTTON_ADD);
    CONTROL_ENABLE_ON_CONDITION(CONTROL_BUTTON_ADD, m_bAllowNewItem && !inputControl->GetLabel2().empty());
  }
  else
    SET_CONTROL_HIDDEN(CONTROL_BUTTON_ADD);

}
