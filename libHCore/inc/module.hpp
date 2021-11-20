#ifndef MODULE_CPP
#define MODULE_CPP

#include <vector>
#include <string>
#include <unordered_map>
#include "hcore.hpp"
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

template<typename T>
using moduleFunc = T (*)(...);


template<typename T>
struct MFunc{
    std::unordered_map<std::string,moduleFunc<T>> kusi;
    moduleFunc<T>& operator[](const char *id) {return kusi[id]; }

};



class Module{
protected:
#ifdef _WIN32
    HMODULE m_library;
#else
    void *m_library;
#endif
public:

    template<typename T>
    moduleFunc<T> loadFunc(char *funcName){
        moduleFunc<T> func;
    #ifdef _WIN32
        func = (moduleFunc<T>)GetProcAddress(m_library,funcName);
    #else
        func = (moduleFunc<T>)dlsym(m_library,funcName);
    #endif
        if(!func){
            printf("Error loading func %s\n",funcName);
        }
        return func;
    }
    

    MFunc<void*> m_pointer;
    MFunc<void> m_void;
    MFunc<bool> m_bool;
    MFunc<uint8_t> m_uchar;
    MFunc<int8_t> m_char;
    MFunc<uint16_t> m_ushort;
    MFunc<int16_t> m_short;
    MFunc<uint32_t> m_uint;
    MFunc<int32_t> m_int;
    MFunc<uint64_t> m_ulong;
    MFunc<int64_t> m_long;
    MFunc<float> m_float;
    MFunc<double> m_double;

    bool loadLibrary(const char *libname);
    virtual void loadModule(){}

};

#endif