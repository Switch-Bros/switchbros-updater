#include "list_download_tab.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

#include "app_page.hpp"
#include "confirm_page.hpp"
#include "current_cfw.hpp"
#include "dialogue_page.hpp"
#include "download.hpp"
#include "extract.hpp"
#include "fs.hpp"
#include "utils.hpp"
#include "worker_page.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

// --- Version Comparison Helpers ---

std::vector<int> parseVersion(const std::string& versionString)
{
    std::vector<int> version;
    std::stringstream ss(versionString);
    std::string segment;
    while (std::getline(ss, segment, '.')) {
        if (segment.empty()) continue;
        try {
            size_t idx;
            int part = std::stoi(segment, &idx);
            version.push_back(part);
        } catch (...) {
            break; 
        }
    }
    return version;
}

bool isVersionLower(const std::string& target, const std::string& current)
{
    std::vector<int> vTarget = parseVersion(target);
    std::vector<int> vCurrent = parseVersion(current);

    size_t len = std::max(vTarget.size(), vCurrent.size());

    for (size_t i = 0; i < len; ++i) {
        int t = (i < vTarget.size()) ? vTarget[i] : 0;
        int c = (i < vCurrent.size()) ? vCurrent[i] : 0;

        if (t < c) return true;
        if (t > c) return false;
    }
    return false;
}

// --- Downgrade Warning Dialogue ---

class DowngradeDialogue : public DialoguePage
{
private:
    std::function<void()> onDownload;
    std::chrono::system_clock::time_point startTime;
    std::string currentVer;
    std::string targetVer;

public:
    DowngradeDialogue(const std::string& current, const std::string& target, std::function<void()> cb) 
        : onDownload(cb), currentVer(current), targetVer(target)
    {
        // 5 seconds timer
        this->startTime = std::chrono::high_resolution_clock::now() + std::chrono::seconds(5);
        this->CreateView();
    }

    void instantiateButtons() override
    {
        // Warning Label using the requested localization key
        this->label = new brls::Label(
            brls::LabelStyle::DIALOG,
            fmt::format("menus/common/low_fw_warning"_i18n, targetVer, currentVer),
            true
        );

        // Button 1: Start Download (Action)
        this->button1->setLabel("menus/common/start_download"_i18n);
        
        // Copy callback to keep it valid
        auto cb_copy = this->onDownload;
        
        this->button1->getClickEvent()->subscribe([cb_copy](brls::View* view) {
            // Safe execution: just open the next view on top.
            // DO NOT call popView() here to avoid crashing.
            cb_copy(); 
        });

        // Button 2: Cancel (Safety)
        this->button2->setLabel("menus/common/cancel"_i18n);
        this->button2->getClickEvent()->subscribe([](brls::View* view) {
            brls::Application::popView();
        });
    }

    void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override
    {
        this->label->frame(ctx);
        this->button2->frame(ctx); // Cancel button always active

        auto now = std::chrono::high_resolution_clock::now();
        auto missing = std::max(0l, std::chrono::duration_cast<std::chrono::seconds>(this->startTime - now).count());

        if (missing > 0) {
            this->button1->setLabel(fmt::format("{} ({})", "menus/common/start_download"_i18n, missing));
            this->button1->setState(brls::ButtonState::DISABLED);
        } else {
            this->button1->setLabel("menus/common/start_download"_i18n);
            this->button1->setState(brls::ButtonState::ENABLED);
        }
        
        this->button1->invalidate();
        this->button1->frame(ctx);
    }

    brls::View* getDefaultFocus() override
    {
        return this->button2;
    }
};

// --- ListDownloadTab Implementation ---

ListDownloadTab::ListDownloadTab(const contentType type, const nlohmann::ordered_json& nxlinks) : brls::List(), type(type), nxlinks(nxlinks)
{
    this->setDescription();

    this->createList();

    if (this->type == contentType::cheats) {
        brls::Label* cheatsLabel = new brls::Label(
            brls::LabelStyle::DESCRIPTION,
            "menus/cheats/cheats_label"_i18n,
            true);
        this->addView(cheatsLabel);
        this->createGbatempItem();
        this->createGfxItem();
        this->createCheatSlipItem();
    }

    if (this->type == contentType::bootloaders) {
        this->setDescription(contentType::hekate_ipl);
        this->createList(contentType::hekate_ipl);
        this->setDescription(contentType::payloads);
        this->createList(contentType::payloads);
    }
}

void ListDownloadTab::createList()
{
    ListDownloadTab::createList(this->type);
}

void ListDownloadTab::createList(contentType type)
{
    std::vector<std::pair<std::string, std::string>> links;
    if (type == contentType::cheats && this->newCheatsVer != "") {
        links.push_back(std::make_pair(fmt::format("menus/main/get_cheats"_i18n, this->newCheatsVer), CurrentCfw::running_cfw == CFW::sxos ? CHEATS_URL_TITLES : CHEATS_URL_CONTENTS));
        links.push_back(std::make_pair("menus/main/get_cheats_gfx"_i18n, CurrentCfw::running_cfw == CFW::sxos ? GFX_CHEATS_URL_TITLES : GFX_CHEATS_URL_CONTENTS));
    }
    else
        links = download::getLinksFromJson(util::getValueFromKey(this->nxlinks, contentTypeNames[(int)type].data()));

    if (links.size()) {
        for (const auto& link : links) {
            const std::string title = link.first;
            const std::string url = link.second;
            const std::string text("menus/common/download"_i18n + link.first + "menus/common/from"_i18n + url);
            listItem = new brls::ListItem(link.first);
            listItem->setHeight(LISTITEM_HEIGHT);
            
            listItem->getClickEvent()->subscribe([this, type, text, url, title](brls::View* view) {
                // Lambda to encapsulate the actual download logic
                auto startDownloadAction = [this, type, text, url, title]() {
                    brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
                    stagedFrame->setTitle(fmt::format("menus/main/getting"_i18n, contentTypeNames[(int)type].data()));
                    stagedFrame->addStage(new ConfirmPage(stagedFrame, text));
                    if (type != contentType::payloads && type != contentType::hekate_ipl) {
                        if (type != contentType::cheats || (this->newCheatsVer != this->currentCheatsVer && this->newCheatsVer != "offline")) {
                            stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/downloading"_i18n, [this, type, url]() { util::downloadArchive(url, type); }));
                        }
                        stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/extracting"_i18n, [this, type]() { util::extractArchive(type, this->newCheatsVer); }));
                    }
                    else if (type == contentType::payloads) {
                        fs::createTree(BOOTLOADER_PL_PATH);
                        std::string path = std::string(BOOTLOADER_PL_PATH) + title;
                        stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/downloading"_i18n, [url, path]() { download::downloadFile(url, path, OFF); }));
                    }
                    else if (type == contentType::hekate_ipl) {
                        fs::createTree(BOOTLOADER_PATH);
                        std::string path = std::string(BOOTLOADER_PATH) + title;
                        stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/downloading"_i18n, [url, path]() { download::downloadFile(url, path, OFF); }));
                    }

                    std::string doneMsg = "menus/common/all_done"_i18n;
                    switch (type) {
                        case contentType::fw: {
                            std::string contentsPath = util::getContentsPath();
                            if (std::filesystem::exists(DAYBREAK_PATH)) {
                                stagedFrame->addStage(new DialoguePage_fw(stagedFrame, doneMsg));
                            }
                            else {
                                stagedFrame->addStage(new ConfirmPage_Done(stagedFrame, doneMsg));
                            }
                            break;
                        }
                        default:
                            stagedFrame->addStage(new ConfirmPage_Done(stagedFrame, doneMsg));
                            break;
                    }
                    brls::Application::pushView(stagedFrame);
                };

                // Check for downgrade if downloading firmware
                if (type == contentType::fw) {
                    SetSysFirmwareVersion ver;
                    if (R_SUCCEEDED(setsysGetFirmwareVersion(&ver))) {
                        std::string currentSysVer = ver.display_version;
                        // title usually contains the version (e.g. "13.2.1")
                        if (isVersionLower(title, currentSysVer)) {
                            brls::Application::pushView(new DowngradeDialogue(currentSysVer, title, startDownloadAction));
                            return; 
                        }
                    }
                }

                // Normal execution
                startDownloadAction();
            });
            this->addView(listItem);
        }
    }
    else {
        this->displayNotFound();
    }
}

void ListDownloadTab::displayNotFound()
{
    brls::Label* notFound = new brls::Label(
        brls::LabelStyle::SMALL,
        "menus/main/links_not_found"_i18n,
        true);
    notFound->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->addView(notFound);
}

void ListDownloadTab::setDescription()
{
    this->setDescription(this->type);
}

void ListDownloadTab::setDescription(contentType type)
{
    brls::Label* description = new brls::Label(brls::LabelStyle::DESCRIPTION, "", true);

    switch (type) {
        case contentType::fw: {
            SetSysFirmwareVersion ver;

            brls::Label* fwText = new brls::Label(
            brls::LabelStyle::DESCRIPTION,
                fmt::format("menus/main/firmware_text"_i18n),
                true);
            fwText->setHorizontalAlign(NVG_ALIGN_LEFT);
            this->addView(fwText);

            brls::Label* fwVersion = new brls::Label(
            brls::LabelStyle::MEDIUM,
                fmt::format("{}{}", "menus/main/firmware_version"_i18n, R_SUCCEEDED(setsysGetFirmwareVersion(&ver)) ? ver.display_version : "menus/main/not_found"_i18n),
                true);
            fwVersion->setHorizontalAlign(NVG_ALIGN_LEFT);
            this->addView(fwVersion);

            break;
        }
        case contentType::bootloaders:
            description->setText(
                "menus/main/bootloaders_text"_i18n);
            break;
        case contentType::cheats:
            this->newCheatsVer = util::getCheatsVersion();
            this->currentCheatsVer = util::readFile(CHEATS_VERSION);
            description->setText("menus/main/cheats_text"_i18n + this->currentCheatsVer);
            break;
        case contentType::payloads:
            description->setText(fmt::format("menus/main/payloads_label"_i18n, BOOTLOADER_PL_PATH));
            break;
        case contentType::hekate_ipl:
            description->setText("menus/main/hekate_ipl_label"_i18n);
            break;
        default:
            break;
    }

    this->addView(description);
}

void ListDownloadTab::createCheatSlipItem()
{
    brls::ListItem* cheatslipsItem = new brls::ListItem("menus/cheats/get_cheatslips"_i18n);
    cheatslipsItem->setHeight(LISTITEM_HEIGHT);
    cheatslipsItem->getClickEvent()->subscribe([](brls::View* view) {
        if (std::filesystem::exists(TOKEN_PATH)) {
            brls::Application::pushView(new AppPage_CheatSlips());
        }
        else {
            std::string usr, pwd;
            // Result rc = swkbdCreate(&kbd, 0);
            brls::Swkbd::openForText([&usr](std::string text) { usr = text; }, "cheatslips.com e-mail", "", 64, "", 0, "Submit", "cheatslips.com e-mail");
            brls::Swkbd::openForText([&pwd](std::string text) { pwd = text; }, "cheatslips.com password", "", 64, "", 0, "Submit", "cheatslips.com password", true);
            std::string body = "{\"email\":\"" + std::string(usr) + "\",\"password\":\"" + std::string(pwd) + "\"}";
            nlohmann::ordered_json token;
            download::getRequest(CHEATSLIPS_TOKEN_URL, token,
                                 {"Accept: application/json",
                                  "Content-Type: application/json",
                                  "charset: utf-8"},
                                 body);
            if (token.find("token") != token.end()) {
                std::ofstream tokenFile(TOKEN_PATH);
                tokenFile << token.dump();
                tokenFile.close();
                brls::Application::pushView(new AppPage_CheatSlips());
            }
            else {
                util::showDialogBoxInfo("menus/cheats/cheatslips_wrong_id"_i18n + "\n" + "menus/cheats/kb_error"_i18n);
            }
        }
        return true;
    });
    this->addView(cheatslipsItem);
}

void ListDownloadTab::createGbatempItem()
{
    brls::ListItem* gbatempItem = new brls::ListItem("menus/cheats/get_gbatemp"_i18n);
    gbatempItem->setHeight(LISTITEM_HEIGHT);
    gbatempItem->getClickEvent()->subscribe([](brls::View* view) {
        brls::Application::pushView(new AppPage_Gbatemp());
        return true;
    });
    this->addView(gbatempItem);
}

void ListDownloadTab::createGfxItem()
{
    brls::ListItem* gfxItem = new brls::ListItem("menus/cheats/get_gfx"_i18n);
    gfxItem->setHeight(LISTITEM_HEIGHT);
    gfxItem->getClickEvent()->subscribe([](brls::View* view) {
        brls::Application::pushView(new AppPage_Gfx());
        return true;
    });
    this->addView(gfxItem);
}