#include "cheat_settings_page.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>

#include "utils.hpp"
#include "fs.hpp"
#include "constants.hpp"
#include "dialogue_page.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

CheatSettingsPage::CheatSettingsPage() : AppletFrame(true, true)
{
    this->setTitle("menus/cheat_settings/title"_i18n);
    
    // Ensure config file exists before proceeding
    ensureConfigExists();
    
    list = new brls::List();
    
    // Status header
    statusHeaderItem = new brls::ListItem("menus/cheat_settings/current_status"_i18n);
    statusHeaderItem->setValue("menus/cheat_settings/config_ready"_i18n);
    statusHeaderItem->setHeight(60);
    list->addView(statusHeaderItem);
    
    // Auto-enable cheats toggle
    autoEnableItem = new brls::ListItem("menus/cheat_settings/auto_enable_cheats"_i18n);
    autoEnableItem->getClickEvent()->subscribe([this](brls::View* view) {
        bool currentValue = getCurrentSetting("dmnt_cheats_enabled_by_default");
        bool newValue = !currentValue;
        
        if (updateConfigSetting("dmnt_cheats_enabled_by_default", newValue)) {
            autoEnableItem->setValue(newValue ? "menus/common/enabled"_i18n : "menus/common/disabled"_i18n);
        }
    });
    autoEnableItem->setHeight(60);
    autoEnableItem->setValue(getCurrentSetting("dmnt_cheats_enabled_by_default") ? "menus/common/enabled"_i18n : "menus/common/disabled"_i18n);
    list->addView(autoEnableItem);
    
    // Remember cheat state toggle
    rememberStateItem = new brls::ListItem("menus/cheat_settings/remember_state"_i18n);
    rememberStateItem->getClickEvent()->subscribe([this](brls::View* view) {
        bool currentValue = getCurrentSetting("dmnt_always_save_cheat_toggles");
        bool newValue = !currentValue;
        
        if (updateConfigSetting("dmnt_always_save_cheat_toggles", newValue)) {
            rememberStateItem->setValue(newValue ? "menus/common/enabled"_i18n : "menus/common/disabled"_i18n);
        }
    });
    rememberStateItem->setHeight(60);
    rememberStateItem->setValue(getCurrentSetting("dmnt_always_save_cheat_toggles") ? "menus/common/enabled"_i18n : "menus/common/disabled"_i18n);
    list->addView(rememberStateItem);
    
    // Add single non-interactive spacing to push restart and info to bottom
    brls::Label* spacer = new brls::Label(brls::LabelStyle::DESCRIPTION, "", true);
    spacer->setHeight(25);
    list->addView(spacer);
    
    // Add restart to payload button (moved to bottom)
    brls::ListItem* restartItem = new brls::ListItem("menus/cheat_settings/restart_to_payload"_i18n);
    restartItem->getClickEvent()->subscribe([this](brls::View* view) {
        // Show confirmation dialog with delay like Atmosphere downloads
        brls::Application::pushView(new DialoguePage_restart(""));
    });
    restartItem->setHeight(60);
    list->addView(restartItem);
    
    // Add info section (moved to bottom)
    brls::ListItem* infoItem = new brls::ListItem("menus/cheat_settings/info"_i18n);
    infoItem->setValue("menus/cheat_settings/info_desc"_i18n);
    infoItem->setHeight(80);
    list->addView(infoItem);
    
    this->setContentView(list);
    
    // Refresh UI to ensure values are correct after potential config creation
    refreshUI();
}

bool CheatSettingsPage::copyTemplateToConfig()
{
    // Check if template exists
    if (!std::filesystem::exists(TEMPLATE_PATH)) {
        return false;
    }
    
    try {
        // Read template file line by line
        std::ifstream templateFile(TEMPLATE_PATH);
        if (!templateFile.is_open()) {
            return false;
        }
        
        // Write to config file
        std::ofstream configFile(CONFIG_PATH);
        if (!configFile.is_open()) {
            templateFile.close();
            return false;
        }
        
        std::string line;
        bool wroteLines = false;
        while (std::getline(templateFile, line)) {
            configFile << line << std::endl;
            wroteLines = true;
        }
        
        templateFile.close();
        configFile.flush();
        configFile.close();
        
        // If we wrote at least one line and file exists, consider it success
        if (wroteLines && std::filesystem::exists(CONFIG_PATH)) {
            return true;
        }
        
        // Verify the file was created and has content
        if (std::filesystem::exists(CONFIG_PATH)) {
            // Check file size first
            auto fileSize = std::filesystem::file_size(CONFIG_PATH);
            if (fileSize > 0) {
                // File exists and has content - that's good enough
                return true;
            }
            
            // Fallback: try to read first line
            std::ifstream test(CONFIG_PATH);
            if (test.is_open()) {
                std::string firstLine;
                if (std::getline(test, firstLine)) {
                    test.close();
                    return true;
                }
                test.close();
            }
        }
        
        return false;
    } catch (const std::exception&) {
        return false;
    }
}

void CheatSettingsPage::refreshUI()
{
    // Update the toggle values to reflect current config state
    if (autoEnableItem) {
        autoEnableItem->setValue(getCurrentSetting("dmnt_cheats_enabled_by_default") ? "menus/common/enabled"_i18n : "menus/common/disabled"_i18n);
    }
    if (rememberStateItem) {
        rememberStateItem->setValue(getCurrentSetting("dmnt_always_save_cheat_toggles") ? "menus/common/enabled"_i18n : "menus/common/disabled"_i18n);
    }
}

void CheatSettingsPage::debugTemplatePaths()
{
    // Check common template locations
    const char* possiblePaths[] = {
        "/atmosphere/config_templates/system_settings.ini",
        "/atmosphere/config_templates/",
        "/atmosphere/config/",
        "/atmosphere/",
        "sdmc:/atmosphere/config_templates/system_settings.ini",
        "sdmc:/atmosphere/config_templates/",
        "sdmc:/atmosphere/config/",
        "sdmc:/atmosphere/"
    };
    
    for (const char* path : possiblePaths) {
        bool exists = std::filesystem::exists(path);
        if (exists) {
            // Found something, but we can't easily log in homebrew
            // The fact that we made it this far means something exists
            break;
        }
    }
}

bool CheatSettingsPage::ensureConfigExists()
{
    // If config already exists, nothing to do
    if (std::filesystem::exists(CONFIG_PATH)) {
        return true;
    }
    
    // Create config directory if it doesn't exist
    std::filesystem::path configDir = std::filesystem::path(CONFIG_PATH).parent_path();
    if (!std::filesystem::exists(configDir)) {
        try {
            std::filesystem::create_directories(configDir);
        } catch (const std::exception&) {
            return false;
        }
    }
    
    // Debug: Check if template exists and add some logging
    bool templateExists = std::filesystem::exists(TEMPLATE_PATH);
    
    // First try to copy from template using our custom method
    if (templateExists) {
        if (copyTemplateToConfig()) {
            // Template copied successfully
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            return true;
        }
    }
    
    // Also try alternative template paths in case the main one isn't found
    const char* alternativePaths[] = {
        "sdmc:/atmosphere/config_templates/system_settings.ini",
        "/atmosphere/config_templates/system_settings.ini.bak",
        "/atmosphere/config_templates/atmosphere.ini",
        "/atmosphere/config/system_settings.ini.template"
    };
    
    for (const char* altPath : alternativePaths) {
        if (std::filesystem::exists(altPath)) {
            try {
                std::ifstream templateFile(altPath);
                if (templateFile.is_open()) {
                    std::ofstream configFile(CONFIG_PATH);
                    if (configFile.is_open()) {
                        std::string line;
                        while (std::getline(templateFile, line)) {
                            configFile << line << std::endl;
                        }
                        templateFile.close();
                        configFile.flush();
                        configFile.close();
                        
                        // Verify it worked
                        if (std::filesystem::exists(CONFIG_PATH)) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(20));
                            return true; // SUCCESS - exit here to prevent default creation
                        }
                    }
                    templateFile.close();
                }
            } catch (...) {
                // Continue to next alternative
            }
        }
    }
    
    // If no template exists or copy failed, create default config
    bool result = createDefaultConfig();
    
    // Force a longer delay to ensure file system operations complete
    if (result) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        // Double-check that the file exists and is readable
        if (std::filesystem::exists(CONFIG_PATH)) {
            std::ifstream test(CONFIG_PATH);
            if (test.is_open()) {
                std::string firstLine;
                if (std::getline(test, firstLine)) {
                    test.close();
                    return true;
                }
                test.close();
            }
        }
        return false;
    }
    
    return result;
}

bool CheatSettingsPage::createDefaultConfig()
{
    try {
        std::ofstream configFile(CONFIG_PATH);
        if (!configFile.is_open()) {
            return false;
        }
        
        configFile << "; Atmosphere Configuration\n";
        configFile << "; This file was created by AIO-Switch-Updater-Reborn\n";
        configFile << "\n";
        configFile << "[atmosphere]\n";
        configFile << "dmnt_cheats_enabled_by_default = u8!0x0\n";
        configFile << "dmnt_always_save_cheat_toggles = u8!0x1\n";
        
        configFile.flush(); // Ensure data is written
        configFile.close();
        
        // Verify the file was actually written
        return std::filesystem::exists(CONFIG_PATH);
    } catch (...) {
        return false;
    }
}

void CheatSettingsPage::showResultDialog(const std::string& message, bool isError)
{
    util::showDialogBoxInfo(message);
}

bool CheatSettingsPage::getCurrentSetting(const std::string& settingName)
{
    if (!std::filesystem::exists(CONFIG_PATH)) {
        // Default values if config doesn't exist
        if (settingName == "dmnt_cheats_enabled_by_default") return false;
        if (settingName == "dmnt_always_save_cheat_toggles") return true;
        return false;
    }
    
    std::ifstream configFile(CONFIG_PATH);
    if (!configFile.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(configFile, line)) {
        if (line.find(settingName) != std::string::npos) {
            if (line.find("0x1") != std::string::npos) {
                return true;
            } else if (line.find("0x0") != std::string::npos) {
                return false;
            }
        }
    }
    
    // Default values if setting not found
    if (settingName == "dmnt_cheats_enabled_by_default") return false;
    if (settingName == "dmnt_always_save_cheat_toggles") return true;
    return false;
}

bool CheatSettingsPage::updateConfigSetting(const std::string& settingName, bool value)
{
    // Read all lines from the file
    std::vector<std::string> lines;
    
    if (std::filesystem::exists(CONFIG_PATH)) {
        std::ifstream configFile(CONFIG_PATH);
        if (configFile.is_open()) {
            std::string line;
            while (std::getline(configFile, line)) {
                lines.push_back(line);
            }
            configFile.close();
        }
    }
    
    // Find and update the setting
    bool found = false;
    
    for (auto& line : lines) {
        if (line.find(settingName) != std::string::npos) {
            line = settingName + " = u8!" + (value ? "0x1" : "0x0");
            found = true;
            break;
        }
    }
    
    // If setting not found, add it at the end
    if (!found) {
        lines.push_back(settingName + " = u8!" + (value ? "0x1" : "0x0"));
    }
    
    // Write all lines back to the file
    try {
        std::ofstream configFile(CONFIG_PATH);
        if (!configFile.is_open()) {
            return false;
        }
        
        for (const auto& line : lines) {
            configFile << line << std::endl;
        }
        
        configFile.close();
        return true;
    } catch (...) {
        return false;
    }
}

