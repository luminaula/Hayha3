#include "module.hpp"
#include "hcore.hpp"

bool Module::loadLibrary(const char *libname){
    HCore::core->log(LOG_INFO,"opening library %s",libname);
    #ifdef _WIN32
    m_library = LoadLibrary(libname);
    if(m_library == NULL){
        HCore::core->log(LOG_ERROR,"Error opening library %x",GetLastError());
        return false;
    }
    #else
    m_library = dlopen(libname, RTLD_LAZY);
    if(m_library == NULL){
        printf("Error opening library %s\n",dlerror());
        return false;
    }
    #endif
    return true;
}