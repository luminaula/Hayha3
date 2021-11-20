#include "filesystem.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace OS {
std::vector<std::string> readFolder(std::string path, std::vector<std::string> extensions) {
    std::vector<std::string> files;
    auto iterator = fs::directory_iterator(path);

    for (auto &entry : iterator) {
        if (!entry.is_directory()) {
            fs::path asd = entry.path().extension();
            if(asd.compare("ALL") == 0){
                std::string path = entry.path().u8string();
                files.push_back(path);
            }
            else{
                for(auto &extension : extensions){
                    if (asd.compare(extension) == 0 || asd.compare("ALL") == 0) {
                        std::string path = entry.path().u8string();
                        files.push_back(path);
                        break;
                    }
                }
            }
        }
    }

    return files;
}
std::vector<std::string> readFolder(std::string path) {
    std::vector<std::string> ext;
    return readFolder(path, ext);
}
} // namespace OS