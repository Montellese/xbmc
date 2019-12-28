/*
 *  Copyright (C) 2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "media/import/IMediaImportRepository.h"

#include <vector>

/*!
 * \brief Interface defining a manager capable of returning a list of import repositories.
 */
class IMediaImportRepositoryManager
{
public:
  virtual ~IMediaImportRepositoryManager() = default;

  /*
   * \brief Gets all import repositories.
   *
   * \return List of import repositories
   */
  virtual std::vector<MediaImportRepositoryPtr> GetImportRepositories() const = 0;

protected:
  IMediaImportRepositoryManager() = default;
};
