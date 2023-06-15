#ifndef ROUTE_HPP
#define ROUTE_HPP

#include <list>
#include <vector>
#include <string>
#include <map>
#include "hcore.hpp"

namespace Util{

    struct RoutePoint{
        float x,y;
        uint32_t utime;
    };

    class Route{
    private:
        std::list<RoutePoint> m_points;
        std::list<RoutePoint>::iterator m_iterator;
        RoutePoint m_currentLocation;
    public:
        Route();
        ~Route();

        void addPoint(float x,float y, uint32_t utime);
        void addPoint(RoutePoint point);
        void travel(uint32_t utime);
        RoutePoint location(){ return m_currentLocation; }
        void reset();
        void clear();
        void clear(uint32_t index);
    };

    class RouteHandler{
    private:
        std::map<std::string,Route> m_routes;
        std::string m_currentRoute;
        
    public:
        RouteHandler(){}
        ~RouteHandler(){}

        Route &getRoute(const std::string &name){
            return m_routes[name];
        }

        void addRoute(const std::string &name){
            m_routes[name] = Route();
            addPoint(name,0.0,0.0,0);
        }

        void clear(){
            m_routes.clear();
        }

        void select(const std::string &name){
            m_currentRoute = name;
        }

        void reset(const std::string &name){
            m_routes[name].reset();
        }

        void resetCurrent(){
            reset(m_currentRoute);
        }

        void addPoint(const std::string &name,float x,float y,uint32_t utime){
            m_routes[name].addPoint(x,y,utime);
        }

        void addPoint(const std::string &name, RoutePoint point){
            m_routes[name].addPoint(point);
        }

        void addPointCurrent(float x,float y, uint32_t utime){
            addPoint(m_currentRoute,x,y,utime);
        }

        void travel(const std::string &name, uint32_t utime){
            m_routes[name].travel(utime);
        }
        
        void travelCurrent(uint32_t utime){
            travel(m_currentRoute,utime);
        }

        void readRoute(const std::string &filename);


    };

    extern RouteHandler routeHandler;

}

#endif