
#include "workers.hpp"

namespace Workers {

bool running = false;

HDetector *detect;
Capturer *capture;
StatServer *statServer;
ApiWorker *apiworker;
MouseWorker *mouseWorker;
KeyboardWorker *keyboardWorker;
Trainer *trainer;
HCore::HCore *m_core;

void init(HCore::HCore *core) {
    m_core = core;
    apiworker = new ApiWorker(core);
    capture = new Capturer(core);
    detect = new HDetector(core);
    statServer = new StatServer(core);
    mouseWorker = new MouseWorker(core);
    keyboardWorker = new KeyboardWorker(core);
    trainer = new Trainer(core);

    apiworker->reload();
    Settings::train.iters = 100;
}

void setLimit(bool value) {
    capture->setLimit(value);
    detect->setLimit(value);
}

void start() {
    if (!running) {
        if (Settings::capture.async) {
            capture->startEventThread();
        }

        apiworker->startEventThread();
        mouseWorker->startEventThread();
        keyboardWorker->startEventThread();
        detect->startEventThread();
        running = true;
    } else {
        capture->stopThread();
        detect->stopThread();
        apiworker->stopThread();
        mouseWorker->stopThread();
        keyboardWorker->stopThread();
        while (detect->isRunning() || capture->isRunning()) {
            std::this_thread::sleep_for(milliseconds(1));
        }
        running = false;
    }
}

void startTrain() {
    if (trainer->isRunning())
        trainer->stopThread();
    else
        trainer->startEventThread();
}

} // namespace Workers