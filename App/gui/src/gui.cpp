#include "gui.hpp"
#include "settings.hpp"

namespace GUI{

    DetectionWindow *dwin;
    MainWindow *mw;
    TransWindow *tw;

    HCore::HCore *m_core;

    void init(HCore::HCore *core){
        m_core = core;
        m_core->log(LOG_INFO,"Init main window");
        mw = new MainWindow();
        
        

        mw->show();



        if(Settings::display.enabled){
            m_core->log(LOG_INFO,"Init detection window");
            dwin = new DetectionWindow(NULL);
            dwin->show();
        }
        
    
        if(Settings::overlay.enabled){
            m_core->log(LOG_INFO,"Init overlay");
            tw = new TransWindow(NULL);
            tw->show();
        }

    }
}