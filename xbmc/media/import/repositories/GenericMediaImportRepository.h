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
#include <set>

#include "media/import/IMediaImportRepository.h"
#include "threads/CriticalSection.h"

class CGenericMediaImportRepository : public IMediaImportRepository
{
public:
  CGenericMediaImportRepository()
    : m_loaded(false)
  { }
  virtual ~CGenericMediaImportRepository();

  virtual bool Initialize() override;

  virtual std::vector<CMediaImport> GetImports() const override;
  virtual std::vector<CMediaImport> GetImportsBySource(const std::string &sourceIdentifier) const override;
  virtual std::vector<CMediaImport> GetImportsByMediaType(const GroupedMediaTypes &mediaTypes) const override;
  virtual bool GetImport(const std::string &path, const GroupedMediaTypes &mediaTypes, CMediaImport &import) const override;

  virtual bool AddImport(const CMediaImport &import) override;
  virtual bool UpdateImport(const CMediaImport &import) override;
  virtual bool RemoveImport(const CMediaImport &import) override;
  
  virtual bool UpdateLastSync(CMediaImport &import) override;

  virtual std::vector<CMediaImportSource> GetSources(const GroupedMediaTypes &mediaTypes = GroupedMediaTypes()) const override;
  virtual bool GetSource(const std::string &identifier, CMediaImportSource &source) const override;

  virtual bool AddSource(const CMediaImportSource &source) override;
  virtual bool UpdateSource(const CMediaImportSource &source) override;
  virtual bool RemoveSource(const std::string &identifier) override;

protected:
  virtual MediaTypes getSupportedMediaTypes() const = 0;

  virtual bool openRepository() = 0;
  virtual void closeRepository() = 0;

  virtual std::vector<CMediaImportSource> getSources() const = 0;
  virtual int addSource(const CMediaImportSource &source) = 0;
  virtual bool updateSource(const CMediaImportSource &source) = 0;
  virtual void removeSource(const CMediaImportSource &source) = 0;
  virtual std::vector<CMediaImport> getImports() const = 0;
  virtual int addImport(const CMediaImport &import) = 0;
  virtual bool updateImport(const CMediaImport &import) = 0;
  virtual void removeImport(const CMediaImport &import) = 0;
  virtual void updateLastSync(const CMediaImport &import, const CDateTime &lastSync) = 0;

  typedef std::pair<std::string, GroupedMediaTypes> MediaImportIdentifier;
  static MediaImportIdentifier GetMediaImportIdentifier(const CMediaImport &import);

  static bool ContainsAllMediaTypes(const MediaTypes& mediaTypes, const GroupedMediaTypes& groupedMediaTypes);

  bool m_loaded;

  typedef std::map<MediaImportIdentifier, CMediaImport> MediaImportMap;
  CCriticalSection m_importsLock;
  MediaImportMap m_imports;

  typedef std::map<std::string, CMediaImportSource> MediaImportSourceMap;
  CCriticalSection m_sourcesLock;
  MediaImportSourceMap m_sources;
};
