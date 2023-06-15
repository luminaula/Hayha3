#ifndef WORKERS_HPP
#define WORKERS_HPP

#include "capturer.hpp"
#include "hdetector.hpp"
#include "statserver.hpp"
#include "apiworker.hpp"
#include "mouseworker.hpp"
#include "hcore.hpp"
#include "keyboardworker.hpp"

extern HCore::HCore *mainCore;

namespace Workers{

    extern HDetector *detect;
    extern Capturer *capture;
    extern StatServer *statServer;
    extern ApiWorker *apiworker;
    extern MouseWorker *mouseWorker;
    extern KeyboardWorker *keyboardWorker;

    extern HCore::HCore *m_core;

    void init(HCore::HCore *core);
    void setLimit(bool value);

    void start();
}
#endif