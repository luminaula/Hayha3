#include "OScommon.hpp"
#include "captureWin.hpp"
#include "mouse.hpp"


namespace OS{
    void init(HCore::HCore *core){
        m_core = core;
        m_core->log(LOG_INFO,"Init win");
        initCapture();
        initMouse();
    }
}