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
#include "ImageResource.h"
#include "URL.h"
#include "addons/AddonManager.h"
#include "filesystem/File.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"

namespace ADDON
{

CImageResource::CImageResource(const cp_extension_t *ext)
  : CResource(ext)
{
  if (ext != nullptr)
    m_type = CAddonMgr::Get().GetExtValue(ext->configuration, "@type");
}

CImageResource::CImageResource(const CImageResource &rhs)
  : CResource(rhs)
{ }

AddonPtr CImageResource::Clone() const
{
  return AddonPtr(new CImageResource(*this));
}

bool CImageResource::IsAllowed(const std::string &file) const
{
  std::string ext = URIUtils::GetExtension(file);
  return file.empty() ||
         StringUtils::EqualsNoCase(ext, ".png") ||
         StringUtils::EqualsNoCase(ext, ".jpg");
}

std::string CImageResource::GetFullPath(const std::string &filePath) const
{
  // get the usual full path
  std::string fullPath = CResource::GetFullPath(filePath);
  // if it exists use it
  if (XFILE::CFile::Exists(fullPath))
    return fullPath;

  // check if there's an XBT file which might contain the file
  std::string resourcePath = GetResourcePath();
  std::string xbtPath = URIUtils::AddFileToFolder(resourcePath, "Textures.xbt");
  if (!XFILE::CFile::Exists(xbtPath))
    return fullPath;

  // translate it into a xbt:// URL
  CURL xbtUrl = URIUtils::CreateArchivePath("xbt", CURL(xbtPath));

  // append the file path to the xbt:// URL
  return URIUtils::AddFileToFolder(xbtUrl.Get(), filePath);
}

} /* namespace ADDON */
