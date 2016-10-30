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

#include <map>

#include "media/MediaType.h"
#include "media/import/IMediaImporter.h"
#include "media/import/IMediaImportHandler.h"
#include "media/import/IMediaImportTask.h"
#include "media/import/MediaImportChangesetTypes.h"

class CMediaImportRetrievalTask : public IMediaImportTask
{
public:
  CMediaImportRetrievalTask(const CMediaImport &import, std::map<MediaType, MediaImportHandlerPtr> importHandlers);
  virtual ~CMediaImportRetrievalTask();

  /*!
   * \brief Get the IMediaImporter instance used by the import job
   */
  const MediaImporterPtr GetImporter() const { return m_importer; }

  /*!
   * \brief Get the media type of the media import
   */
  const GroupedMediaTypes& GetMediaTypes() const { return GetImport().GetMediaTypes(); }

  /*!
   * \brief Get a list of imported items
   *
   * \param mediaType media type of the imported items
   * \return list of imported items
   */
  const ChangesetItems& GetRetrievedItems(const MediaType& mediaType) const { return m_retrievedItems.find(mediaType)->second; }

  /*!
  * \brief Get a list of previously imported items
  *
  * \param mediaType media type of the previously imported items
  * \return list of previously imported items
  */
  const std::vector<CFileItemPtr>& GetLocalItems(const MediaType& mediaType) const { return m_localItems.find(mediaType)->second; }

  /*!
   * \brief Add an imported item of a specific changeset type
   *
   * \param item imported item
   * \param mediaType media type of the item
   * \param changesetType changeset type of the imported item
   */
  void AddItem(const CFileItemPtr& item, const MediaType& mediaType, MediaImportChangesetType changesetType = MediaImportChangesetTypeNone);

  /*!
  * \brief Add a list of imported items of a specific changeset type
  *
  * \param items imported items
   * \param mediaType media type of the items
  * \param changesetType changeset type of the imported items
  */
  void AddItems(const std::vector<CFileItemPtr>& items, const MediaType& mediaType, MediaImportChangesetType changesetType = MediaImportChangesetTypeNone);

  /*!
   * \brief Add a list of imported items of a specific media type
   *
   * \param items imported items
   * \param mediaType media type of the items
   * \param changesetType changeset type of the imported items
   */
  void SetItems(const ChangesetItems& items, const MediaType& mediaType);

  // implementation of IMediaImportTask
  virtual MediaImportTaskType GetType() const override { return MediaImportTaskType::Retrieval; }
  virtual bool DoWork() override;

protected:
  MediaImporterPtr m_importer;
  std::map<MediaType, MediaImportHandlerPtr> m_importHandlers;
  std::map<MediaType, ChangesetItems> m_retrievedItems;
  std::map<MediaType, std::vector<CFileItemPtr>> m_localItems;
};
