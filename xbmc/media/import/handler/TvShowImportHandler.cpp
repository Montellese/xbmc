/*
 *      Copyright (C) 2013 Team XBMC
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

#include "TvShowImportHandler.h"
#include "FileItem.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "media/import/IMediaImportTask.h"
#include "utils/StringUtils.h"
#include "video/VideoDatabase.h"
#include "video/VideoThumbLoader.h"

/*!
 * Checks whether two tvshows are the same by comparing them by title and year
 */
static bool IsSameTVShow(CVideoInfoTag& left, CVideoInfoTag& right)
{
  return left.m_strShowTitle == right.m_strShowTitle
      && left.m_iYear        == right.m_iYear;
}

bool CTvShowImportHandler::HandleImportedItems(CVideoDatabase &videodb, const CMediaImport &import, const CFileItemList &items, IMediaImportTask *task)
{
  bool checkCancelled = task != NULL;
  if (checkCancelled && task->ShouldCancel(0, items.Size()))
    return false;
  
  task->SetProgressTitle("Importing tvshows from " + import.GetSource().GetFriendlyName()); // TODO: localization
  task->SetProgressText("");

  CFileItemList storedItems;
  videodb.GetTvShowsByWhere("videodb://tvshows/titles/", GetFilter(import), storedItems, SortDescription(), true);

  CVideoThumbLoader thumbLoader;
  thumbLoader.OnLoaderStart();
  
  int progress = 0;
  int total = storedItems.Size() + items.Size();
  CFileItemList newItems; newItems.Copy(items);
  for (int i = 0; i < storedItems.Size(); i++)
  {
    if (checkCancelled && task->ShouldCancel(progress, items.Size()))
      return false;

    CFileItemPtr& oldItem = storedItems[i];
    bool found = false;
    for (int j = 0; j < newItems.Size() ; j++)
    {
      if (checkCancelled && task->ShouldCancel(progress, items.Size()))
        return false;

      CFileItemPtr& newItem = newItems[j];
      task->SetProgressText("Checking " + newItem->GetVideoInfoTag()->m_strTitle); // TODO: localization

      if (IsSameTVShow(*oldItem->GetVideoInfoTag(), *newItem->GetVideoInfoTag()))
      {
        // get rid of items we already have from the new items list
        newItems.Remove(j);
        total--;
        found = true;

        thumbLoader.LoadItem(oldItem.get());
        
        // check if we need to update (db writing is expensive)
        if (!Compare(oldItem.get(), newItem.get()))
        {
          task->SetProgressText("Updating " + newItem->GetVideoInfoTag()->m_strTitle); // TODO: localization

          PrepareItem(newItem.get(), oldItem.get());

          std::map<int, std::map<std::string, std::string> > seasonArt; // TODO: get season art
          videodb.SetDetailsForTvShow(newItem->GetPath(), *(newItem->GetVideoInfoTag()), newItem->GetArt(), seasonArt, newItem->GetVideoInfoTag()->m_iDbId);
        }
        break;
      }
    }

    // delete items that are not in newItems
    if (!found)
    {
      task->SetProgressText("Removing " + oldItem->GetVideoInfoTag()->m_strTitle); // TODO: localization
      videodb.DeleteTvShow(oldItem->GetVideoInfoTag()->m_iDbId);
    }

    task->SetProgress(progress++, total);
  }

  thumbLoader.OnLoaderFinish();
  
  // add any (remaining) new items
  for (int i = 0; i < newItems.Size(); i++)
  {
    if (checkCancelled && task->ShouldCancel(progress, items.Size()))
      return false;

    CFileItemPtr& pItem = newItems[i];
    PrepareItem(import, pItem.get(), videodb);

    task->SetProgressText("Adding " + pItem->GetVideoInfoTag()->m_strTitle); // TODO: localization

    std::map<int, std::map<std::string, std::string> > seasonArt;
    videodb.SetDetailsForTvShow(pItem->GetPath(), *(pItem->GetVideoInfoTag()), pItem->GetArt(), seasonArt);

    task->SetProgress(progress++, total);
  }

  return true;
}
