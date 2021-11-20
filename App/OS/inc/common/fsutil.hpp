#include <string>
#include <vector>

namespace OS {
std::vector<std::string> readFolder(std::string path);
std::vector<std::string> readFolder(std::string path, std::vector<std::string> extensions);
} // namespace OS