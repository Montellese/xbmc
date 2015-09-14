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
#include "media/import/MediaImport.h"
#include "media/import/MediaImportSource.h"

class CMediaImportSourceEvent : public CUniqueEvent
{
public:
  CMediaImportSourceEvent(const CMediaImportSource& source, const CVariant& description, EventLevel level = EventLevel::Information);
  CMediaImportSourceEvent(const CMediaImportSource& source, const CVariant& description, bool removed, EventLevel level = EventLevel::Information);
  virtual ~CMediaImportSourceEvent() { }

  virtual const char* GetType() const override { return "MediaImportSourceEvent"; }
  virtual std::string GetExecutionLabel() const override;

  virtual bool CanExecute() const override;
  virtual bool Execute() const override;

protected:
  CMediaImportSource m_source;
};

class CMediaImportEvent : public CUniqueEvent
{
public:
  CMediaImportEvent(const CMediaImport& import, const CVariant& description, EventLevel level = EventLevel::Information);
  CMediaImportEvent(const CMediaImport& import, const CVariant& description, bool removed, EventLevel level = EventLevel::Information);
  virtual ~CMediaImportEvent() { }

  virtual const char* GetType() const override { return "MediaImportEvent"; }
  virtual std::string GetExecutionLabel() const override;

  virtual bool CanExecute() const override;
  virtual bool Execute() const override;

protected:
  CMediaImport m_import;
};
