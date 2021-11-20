#include <QApplication>
#include <QMainWindow>

#include "hcore.hpp"
#include "mainwindow.h"
#include "workers.hpp"

#include "capture.hpp"
#include "hsocket.hpp"
#include "settings.hpp"

#include "OScommon.hpp"
#include "buffer.hpp"
#include "filesystem.hpp"
#include "gui.hpp"
#include "mouse.hpp"

#ifdef __linux__
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "HApi.hpp"

#include "xorstring.hpp"

#include "modules.hpp"
#include "route.hpp"
#include "colorspaces.hpp"

HCore::HCore *core;

void printWaterMark() { core->log(rand()+0x100, XORSTR("Made by Xirrel")); }

void handtest(int i){
    core->log(LOG_WARNING, "%d",i);
}

int main(int argc, char **argv) {

    QApplication a(argc, argv);
    
    //HEBI::Colors::checkColors();
    //HEBI::Colors::findMinMax();


    //HEBI::Colors::checkColors();

    QCoreApplication::setApplicationName("Hayha 0.2.6");
    QCoreApplication::setOrganizationName("Hebi software");
    QCoreApplication::setApplicationVersion("0.2.6");

    // Entry point
    printf("Loading core\n");

    srand(time(NULL));
#ifdef __linux__
    // Set lowest of low priority
    //No I mean high
    setpriority(PRIO_PROCESS, getpid(), -20);
#endif

    // Init all folder locations for absolute paths
    OS::initPaths();
    Settings::loadSettings(OS::cfgPath + "Settings.json");

    core = HCore::init(Settings::core.threadCount, Settings::core.logLevel);
    printWaterMark();

    

    Modules::init();
    core->log(LOG_INFO, "Begin init");
    core->log(LOG_INFO, "Init colors");
    HEBI::Colors::init();
    core->log(LOG_INFO, "Init buffer");
    HBuffer::init(32);
    OS::init(core);
    HAPI::init(core);
    Workers::init(core);
    GUI::init(core);
    core->eventHandler->addRecurringEvent(getCurrentTimeMicro(), 60*30 * 1000000, printWaterMark);

    if (Settings::detection.mode != Settings::LOCAL) {
        core->log(LOG_INFO, "Init socket");
        OS::socket_init(OS::CLIENT);
        // socket_connect(Settings::detection.IP, 1939);
    }

    Workers::statServer->startEventThread();

    core->log(LOG_INFO, "Init done");
    return a.exec();
}
