#include "capture.hpp"
#include "filesystem.hpp"
#include "framebuffer.hpp"


namespace OS{

    HCapture::HCapture(){
        std::string filename = OS::libPath + OS::libPrefix + "HCapture" + OS::libSuffix;
        loadLibrary(filename.c_str());
        m_void["init"] = loadFunc<void>("init");
        m_void["deinit"] = loadFunc<void>("deinit");
        //m_void["captureFrame"] = loadFunc<void>("captureFrame");
        m_pointer["captureFrame"] = loadFunc<void*>("captureFrame");
        m_void["setMode"] = loadFunc<void>("setMode");
        m_void["setTarget"] = loadFunc<void>("setTarget");
        m_void["Error"] = loadFunc<void>("Error");
        m_bool["isAttached"] = loadFunc<bool>("isAttached");
    }

    HCapture *capture;
    std::mutex captureMutex;


    void initCapture(){
        capture = new HCapture();
        capture->m_void["setMode"](Settings::capture.DXGI);
        capture->m_void["setTarget"](Settings::capture.target.c_str());
        capture->m_void["init"](Settings::screen.w,Settings::screen.h);
        
    }


    void* captureFrame(Framebuffer &fb){
        std::lock_guard<std::mutex> lock(captureMutex);
        return capture->m_pointer["captureFrame"]((unsigned char*)fb.data,fb.capture.x,fb.capture.y,fb.width,fb.height);
        
    }

}