/*
 *  Copyright (C) 2017-2021 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "addons/IAddon.h"
#include "threads/Event.h"

#include <string>

class CScriptRunner
{
protected:
  CScriptRunner();
  virtual ~CScriptRunner() = default;

  virtual bool OverrideReuseLanguageInvoker() const { return false; }
  virtual bool OverriddenReuseLanguageInvoker() const { return false; }

  virtual bool IsSuccessful() const = 0;
  virtual bool IsCancelled() const = 0;

  ADDON::AddonPtr GetAddon() const;

  bool StartScript(ADDON::AddonPtr addon, const std::string& path);
  bool RunScript(ADDON::AddonPtr addon, const std::string& path, int handle, bool resume);

  void SetDone();

  static bool ReuseLanguageInvoker(ADDON::AddonPtr addon);

  static int ExecuteScript(ADDON::AddonPtr addon,
                           const std::string& path,
                           bool resume,
                           bool reuseLanguageInvoker);
  static int ExecuteScript(ADDON::AddonPtr addon,
                           const std::string& path,
                           int handle,
                           bool resume,
                           bool reuseLanguageInvoker);

private:
  bool RunScriptInternal(
      ADDON::AddonPtr addon, const std::string& path, int handle, bool resume, bool wait = true);
  bool WaitOnScriptResult(int scriptId, const std::string& path, const std::string& name);

  ADDON::AddonPtr m_addon;

  CEvent m_scriptDone;
};
