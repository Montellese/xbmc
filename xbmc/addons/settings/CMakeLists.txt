set(SOURCES AddonSettings.cpp
            SettingUrlEncodedString.cpp)

set(HEADERS AddonSettings.h
            IAddonSettingsCallbackExecutor.h
            SettingUrlEncodedString.h)

core_add_library(addons_settings)
