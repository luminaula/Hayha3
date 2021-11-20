#include "keyboardworker.hpp"
#include "keyboard.hpp"
#include "settings.hpp"

KeyboardWorker::KeyboardWorker(HCore::HCore *core) : Threadable(Settings::keyboard.rate, "KeyboardWorker", false, core), m_strict(false) {}
KeyboardWorker::~KeyboardWorker() {}

void KeyboardWorker::setStrict(bool value) { m_strict = value; }

void KeyboardWorker::listen(uint32_t code) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_keystates[code].listen = true;
    m_keystates[code].triggerTime = m_workTimeUsec;
}

void KeyboardWorker::clearListen() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_keystates.clear();
}

bool KeyboardWorker::check(uint32_t code) {
    if (!m_keystates[code].listen) {
        m_core->log(LOG_WARNING, "Tried checking key %d that is not being listened", code);
        if (!m_strict)
            listen(code);
        return false;
    }
    return m_keystates[code].pressed;
}

void KeyboardWorker::work() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto &entry : m_keystates) {
        if (!entry.second.listen)
            continue;
        if (timeSince(entry.second.time) > entry.second.triggerTime) {
            entry.second.pressed = OS::checkIfPressed(entry.first);
            entry.second.time = getCurrentTimeMicro();
        }
    }
}