#pragma once

#include <borealis.hpp>

enum class CheatSettingStatus {
    UNKNOWN,
    ENABLED,
    DISABLED
};

class CheatSettingsPage : public brls::AppletFrame
{
private:
    brls::List* list;
    brls::ListItem* autoEnableItem;
    brls::ListItem* rememberStateItem;
    brls::ListItem* statusHeaderItem;
    
    // File paths
    static constexpr const char* CONFIG_PATH = "/atmosphere/config/system_settings.ini";
    static constexpr const char* TEMPLATE_PATH = "/atmosphere/config_templates/system_settings.ini";
    
    // Helper methods
    bool ensureConfigExists();
    bool createDefaultConfig();
    bool copyTemplateToConfig();
    void refreshUI();
    void debugTemplatePaths();
    void showResultDialog(const std::string& message, bool isError = false);
    bool getCurrentSetting(const std::string& settingName);
    bool updateConfigSetting(const std::string& settingName, bool value);

public:
    CheatSettingsPage();
};