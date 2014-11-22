#pragma once
/*
 *      Copyright (C) 2015 Team XBMC
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

#include "media/MediaType.h"

class IMediaImportHandler;
typedef std::shared_ptr<const IMediaImportHandler> MediaImportHandlerConstPtr;

/*!
* \brief TODO
*/
class IMediaImportHandlerManager
{
public:
  virtual ~IMediaImportHandlerManager() { }

  /*!
   * \brief Returns the media import handler implementation capable of handling
   * imported media items of the given media type.
   *
   * \param mediaType Media type of an imported item
   * \pram Media import handler implementation
   */
  virtual MediaImportHandlerConstPtr GetImportHandler(const MediaType& mediaType) const = 0;

protected:
  IMediaImportHandlerManager() { }
};
