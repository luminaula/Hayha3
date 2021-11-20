#include "mouseworker.hpp"
#include "HApi.hpp"
#include "mouse.hpp"
#include "settings.hpp"
#include <math.h>

MouseWorker::MouseWorker(HCore::HCore *core) : Threadable(Settings::mouse.rate, "MouseWorker", false, core) {

    accumX = 0.0;
    accumY = 0.0;
    sensitivity = 100;
}

MouseWorker::~MouseWorker() {}

void MouseWorker::push(MouseNode node) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (node.code == MOUSE_MOVE) {
        m_moveQ.push(node);
        return;
    }
    m_mouseQueue.push(node);
}

void MouseWorker::work() {
    HAPI::checkMouseClick();
    if (m_mouseQueue.empty() && m_moveQ.empty())
        return;
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_mouseQueue.empty()) {
        MouseNode node = m_mouseQueue.front();
        m_mouseQueue.pop();
        switch (node.code) {
        /*
        case MOUSE_MOVE:
            accumX = node.x;
            accumY = node.y;
            while(!m_mouseQueue.empty() && m_mouseQueue.front().code ==
        MOUSE_MOVE){ node = m_mouseQueue.front(); m_mouseQueue.pop(); accumX +=
        node.x; accumY += node.y;
            }
            OS::moveMouse(accumX,accumY);
            move = true;
            break;
        */
        case MOUSE_CLICK_DOWN:
            OS::mouseDown();
            break;
        case MOUSE_CLICK_UP:
            OS::mouseUp();
            break;
        default:
            break;
        }
    }

    while (!m_moveQ.empty() && timeTo(m_moveQ.top().m_time) < 0) {
        MouseNode node = m_moveQ.top();
        m_moveQ.pop();
        accumX += node.x * (float)sensitivity / 100.0;
        accumY += node.y * (float)sensitivity / 100.0;
        // OS::moveMouse(node.x,node.y);
    }
    if (abs(accumX) > 1.0 || abs(accumY) > 1.0) {
        int moveX, moveY;
        if (accumX > 0.0)
            moveX = (int)floorf(accumX);
        else
            moveX = (int)ceilf(accumX);

        if (accumY > 0.0)
            moveY = (int)floorf(accumY);
        else
            moveY = (int)ceilf(accumY);
        OS::moveMouse(moveX, moveY);

        accumX -= moveX;
        accumY -= moveY;
    }
}