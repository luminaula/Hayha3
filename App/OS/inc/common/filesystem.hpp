#include "fsutil.hpp"
#include <string>
#include <vector>

namespace OS {
extern std::string hayhaPath;
extern std::string binPath;
extern std::string cfgPath;
extern std::string libPath;
extern std::string profilesPath;
extern std::string scriptPath;
extern std::string libPrefix;
extern std::string libSuffix;
extern std::string routesPath;
extern std::string datasetPath;

void initPaths();

} // namespace OS