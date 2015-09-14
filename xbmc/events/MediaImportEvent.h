#pragma once
/*
 *      Copyright (C) 2015 Team Kodi
 *      http://kodi.tv
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

#include "events/UniqueEvent.h"
#include "media/import/MediaImportSource.h"

class CMediaImportEvent : public CUniqueEvent
{
public:
  CMediaImportEvent(const CMediaImportSource& source, const CVariant& description);
  CMediaImportEvent(const CMediaImportSource& source, const CVariant& description, const CVariant& details);
  CMediaImportEvent(const CMediaImportSource& source, const CVariant& description, const CVariant& details, const CVariant& executionLabel);
  virtual ~CMediaImportEvent() { }

  virtual const char* GetType() const { return "MediaImportEvent"; }
  virtual std::string GetExecutionLabel() const;

  virtual bool CanExecute() const { return true; }
  virtual bool Execute() const;

protected:
  CMediaImportSource m_source;
};
