#ifndef WORKERS_HPP
#define WORKERS_HPP

#include "apiworker.hpp"
#include "capturer.hpp"
#include "hcore.hpp"
#include "hdetector.hpp"
#include "keyboardworker.hpp"
#include "mouseworker.hpp"
#include "statserver.hpp"
#include "trainer.hpp"

extern HCore::HCore *mainCore;

namespace Workers {

extern bool running;

extern HDetector *detect;
extern Capturer *capture;
extern StatServer *statServer;
extern ApiWorker *apiworker;
extern MouseWorker *mouseWorker;
extern KeyboardWorker *keyboardWorker;
extern Trainer *trainer;

extern HCore::HCore *m_core;

void init(HCore::HCore *core);
void setLimit(bool value);

void start();
void startTrain();
} // namespace Workers
#endif