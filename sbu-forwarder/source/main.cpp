#include <filesystem>
#include <string>

#include <switch.h>

#define PATH        "/switch/switchbros-updater/"
#define FULL_PATH   "/switch/switchbros-updater/switchbros-updater.nro"
#define CONFIG_PATH "/config/switchbros-updater/switch/switchbros-updater/switchbros-updater.nro"
#define PREFIX      "/switch/switchbros-updater/switchbros-updater-v"
#define FORWARDER_PATH      "/config/switchbros-updater/sbu-forwarder.nro"
#define CONFIG_SWITCH       "/config/switchbros-updater/switch/"
#define HIDDEN_FILE "/config/switchbros-updater/.switchbros-updater"

int removeDir(const char* path)
{
    Result ret = 0;
    FsFileSystem *fs = fsdevGetDeviceFileSystem("sdmc");
    if (R_FAILED(ret = fsFsDeleteDirectoryRecursively(fs, path))) {
        return ret;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    std::filesystem::create_directory(PATH);
    for (const auto & entry : std::filesystem::directory_iterator(PATH)){
        if(entry.path().string().find(PREFIX) != std::string::npos) {
            std::filesystem::remove(entry.path().string());
            std::filesystem::remove(entry.path().string() + ".star");
        }
    }
    std::filesystem::remove(HIDDEN_FILE);

    if(std::filesystem::exists(CONFIG_PATH)){
        std::filesystem::create_directory(PATH);
        std::filesystem::remove(FULL_PATH);
        std::filesystem::rename(CONFIG_PATH, FULL_PATH);
        removeDir(CONFIG_SWITCH);
    }

    std::filesystem::remove(FORWARDER_PATH);

    envSetNextLoad(FULL_PATH, FULL_PATH);
    return 0;
}
