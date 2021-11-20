#include "route.hpp"
#include <fstream>
#include <sstream>

namespace Util {

RouteHandler routeHandler;

Route::Route() { reset(); }

Route::~Route() {}

void Route::addPoint(float x, float y, uint32_t utime) {
    RoutePoint point;
    point.x = x;
    point.y = y;
    point.utime = utime;
    m_points.push_back(point);
}

void Route::addPoint(RoutePoint point) {
    m_points.push_back(point);
    // HCore::log(LOG_DEBUG,"%f %f
    // %u",m_points.back().x,m_points.back().y,m_points.back().utime);
}

void Route::clear() { m_points.clear(); }

void Route::reset() {
    m_currentLocation = {0};
    m_iterator = m_points.begin();
}

void Route::travel(uint32_t utime) {
    RoutePoint next;
    // m_iterator = m_points.begin();
    do {
        m_iterator++;
        next = *m_iterator;
        // HCore::log(LOG_DEBUG,"%f %f %u",next.x, next.y, next.utime);
        if (utime >= next.utime - m_currentLocation.utime) {
            float ratio = (float)m_currentLocation.utime / next.utime;
            m_currentLocation.x += next.x - (next.x * ratio);
            m_currentLocation.y += next.y - (next.y * ratio);
            utime -= next.utime - m_currentLocation.utime;
            m_currentLocation.utime = 0;
        } else {
            float ratio = (float)utime / next.utime;
            m_currentLocation.x += next.x * ratio;
            m_currentLocation.y += next.y * ratio;
            m_currentLocation.utime += utime;
            m_iterator--;
            break;
        }
    } while (utime > next.utime - m_currentLocation.utime);
    // HCore::log(LOG_DEBUG,"%f %f %d",m_currentLocation.x, m_currentLocation.y,
    // m_currentLocation.utime);
}

void RouteHandler::readRoute(const std::string &filename) {
    std::ifstream routeFile(filename);

    std::string line;
    std::string name;
    std::vector<RoutePoint> points;
    int lineNumber = 0;
    // HCore::log(LOG_DEBUG,"Reading route %s",filename.c_str());
    while (std::getline(routeFile, line)) {
        lineNumber++;
        if (line.compare("[NAME]") == 0) {
            if (!std::getline(routeFile, name)) {
                // HCore::log(LOG_ERROR,"%s line
                // %d",filename.c_str(),lineNumber);
                break;
            } else {
                // HCore::log(LOG_DEBUG,"[NAME] %s %d",name.c_str(),lineNumber);
                addRoute(name);
                lineNumber++;
            }
        } else if (line.compare("[POINT]") == 0) {
            while (std::getline(routeFile, line)) {
                if (line.compare("[/POINT]") == 0)
                    break;
                lineNumber++;
                RoutePoint tmpPoint;
                std::stringstream ss(line);
                if (!(ss >> tmpPoint.x >> tmpPoint.y >> tmpPoint.utime)) {
                    // HCore::log(LOG_ERROR,"%s line
                    // %s",filename.c_str(),lineNumber);
                } else {
                    // HCore::log(LOG_DEBUG,"[POINT] x = %f y = %f time = %uus
                    // %d",tmpPoint.x,tmpPoint.y,tmpPoint.utime,lineNumber);
                    addPoint(name, tmpPoint);
                }
            }
            RoutePoint tmpPoint = {0};
            tmpPoint.x = 0.0;
            tmpPoint.y = 0.0;
            tmpPoint.utime = 0xFFFFFFFF;
            addPoint(name, tmpPoint);
        }
    }

    reset(name);
}

} // namespace Util