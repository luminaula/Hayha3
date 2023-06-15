#include <QApplication>
#include <QMainWindow>


#include "mainwindow.h"
#include "hcore.hpp"
#include "workers.hpp"

#include "capture.hpp"
#include "settings.hpp"
#include "hsocket.hpp"

#include "filesystem.hpp"
#include "mouse.hpp"
#include "OScommon.hpp"
#include "buffer.hpp"
#include "gui.hpp"

#ifdef __linux__ 
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "HApi.hpp"

#include "xorstring.hpp"

#include "route.hpp"
#include "modules.hpp"

HCore::HCore *core;

void printWaterMark(){
    core->log(0xFFFFFFFF,XORSTR("Made by Xirrel"));
}



int main(int argc,char **argv){

    QApplication a( argc, argv );

    QCoreApplication::setApplicationName("Hayha 0.2.6");
    QCoreApplication::setOrganizationName("Hebi software");
    QCoreApplication::setApplicationVersion("0.2.6");

    //Entry point
    printf("Loading core\n");

    srand(time(NULL));
#ifdef __linux__ 
    //Set lowest of low priority
    setpriority(PRIO_PROCESS, getpid(), 19);
#endif
    
    //Init all folder locations for absolute paths
    OS::initPaths();
    Settings::loadSettings(OS::cfgPath + "Settings.json");
     
    core = HCore::init(Settings::core.threadCount, Settings::core.logLevel);
    
    Modules::init();
    core->log(LOG_INFO,"Begin init");
    core->log(LOG_INFO,"Init buffer");
    HBuffer::init(32);
    OS::init(core);
    HAPI::init(core);
    Workers::init(core);
    GUI::init(core);
    core->eventHandler->addRecurringEvent(getCurrentTimeMicro(),300 *1000000,printWaterMark);
    
    
    if(Settings::detection.mode != Settings::LOCAL){
        core->log(LOG_INFO,"Init socket");
        OS::socket_init(OS::CLIENT);
        //socket_connect(Settings::detection.IP, 1939);
    }

    Workers::statServer->startEventThread();

    
    core->log(LOG_INFO,"Init done");
    return a.exec();

}
