#include "trackermodule.hpp"
#include "filesystem.hpp"
#include <string>

void TrackerModule::loadModule(){
    std::string libname = OS::libPath + OS::libPrefix + "HTracker" + OS::libSuffix;
    loadLibrary(libname.c_str());
    m_void["init"] = loadFunc<void>("initTracker");
    m_void["setDimensions"] = loadFunc<void>("setDimensions");
    m_void["resetTracker"] = loadFunc<void>("resetTracker");
    m_boxes["trackFrame"] = loadFunc<std::vector<bbox_t>>("trackFrame");

    //m_void["init"](1,1,1,1,5);
}