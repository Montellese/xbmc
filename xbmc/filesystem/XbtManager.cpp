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

#include "XbtManager.h"
#include "URL.h"
#include "guilib/XBTF.h"
#include "guilib/XBTFReader.h"

namespace XFILE
{

CXbtManager::CXbtManager()
{ }

CXbtManager::~CXbtManager()
{ }

CXbtManager& CXbtManager::GetInstance()
{
  static CXbtManager xbtManager;
  return xbtManager;
}

bool CXbtManager::HasFiles(const CURL& path) const
{
  return ProcessFile(path) != m_readers.end();
}

bool CXbtManager::GetFiles(const CURL& path, std::vector<CXBTFFile>& files) const
{
  const auto& reader = ProcessFile(path);
  if (reader == m_readers.end())
    return false;

  files = reader->second->GetFiles();
  return true;
}

bool CXbtManager::GetReader(const CURL& path, CXBTFReaderPtr& reader) const
{
  const auto& it = ProcessFile(path);
  if (it == m_readers.end())
    return false;

  reader = it->second;
  return true;
}

void CXbtManager::Release(const CURL& path)
{
  const auto& it = GetReader(path);
  if (it == m_readers.end())
    return;

  // close the reader
  it->second->Close();

  // and remove it from the map
  m_readers.erase(it);
}

CXbtManager::XBTFReaders::iterator CXbtManager::GetReader(const CURL& path) const
{
  return GetReader(NormalizePath(path));
}

CXbtManager::XBTFReaders::iterator CXbtManager::GetReader(const std::string& path) const
{
  if (path.empty())
    return m_readers.end();

  return m_readers.find(path);
}

CXbtManager::XBTFReaders::const_iterator CXbtManager::ProcessFile(const CURL& path) const
{
  std::string filePath = NormalizePath(path);

  // check if the file is known
  const auto& it = GetReader(filePath);
  if (it != m_readers.end())
    return it;

  // try to read the file
  CXBTFReaderPtr reader(new CXBTFReader());
  if (!reader->Open(filePath))
    return m_readers.end();

  std::pair<XBTFReaders::iterator, bool> result = m_readers.insert(std::make_pair(filePath, reader));
  return result.first;
}

std::string CXbtManager::NormalizePath(const CURL& path)
{
  if (path.IsProtocol("xbt"))
    return path.GetHostName();

  return path.Get();
}

}
