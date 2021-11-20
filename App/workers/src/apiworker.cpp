#include "json.hpp"

#include "apiworker.hpp"
#include "workers.hpp"

#include "HApi.hpp"
#include "buffer.hpp"
#include "framebuffer.hpp"
#include "keyboard.hpp"
#include "mouse.hpp"
#include "settings.hpp"

#include "filesystem.hpp"

using json = nlohmann::json;

ApiWorker::ApiWorker(HCore::HCore *core) : Threadable(2000, "ApiWorker", false, core) {

    m_targetTeam = 0;
    m_saveCount = 0;
    m_save = false;
    m_savePrefix = std::to_string(unixTime(getCurrentTimeMicro())) + std::to_string(rand());
}

ApiWorker::~ApiWorker() {}

void ApiWorker::clearTeams() { m_teams.clear(); }

void ApiWorker::newTeam() { m_teams.push_back(std::vector<uint32_t>()); }

void ApiWorker::addTargetId(int id) { m_teams.back().push_back(id); }

std::vector<FloatArea_t> ApiWorker::getIgnores() { return m_ignoreAreas; }

void ApiWorker::clearIgnores() { m_ignoreAreas.clear(); }

void ApiWorker::ignoreArea(float x0, float y0, float w, float h) {
    FloatArea_t ignore;
    ignore.x = x0;
    ignore.y = y0;
    ignore.w = w;
    ignore.h = h;
    m_ignoreAreas.push_back(ignore);
}

std::vector<uint32_t> ApiWorker::getTargetIds() {
    if (m_teams.empty()) {
        return std::vector<uint32_t>(0);
    }
    return m_teams[m_targetTeam];
}

bbox_t ApiWorker::getCurrentTarget() { return m_currentTarget; }

void ApiWorker::switchTeams() {
    if (m_targetTeam == m_teams.size() - 1)
        m_targetTeam = 0;
    else
        m_targetTeam++;
}

bool ApiWorker::loadScript(char *filename) {
    sol::load_result lr = m_luaApi.load_file(filename);
    if (!lr.valid()) {
        sol::error err = lr;
        m_core->log(LOG_ERROR, "%s API %s", filename, err.what());
        return false;
    } else {
        sol::protected_function_result result = lr();
        if (!result.valid()) {
            sol::error err = result;
            m_core->log(LOG_ERROR, "%s %s", filename, err.what());
            return false;
        }
    }
    return true;
}

void ApiWorker::reload() {
    std::lock_guard<std::mutex> lock(m_mutex);

    m_luaApi = sol::state();

    m_luaApi.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);
    // m_luaApi.open_libraries(sol::lib::base, sol::lib::package,
    // sol::lib::math); Set Useful functions

    m_luaApi.new_usertype<bbox_t>("bbox", "x", &bbox_t::x, "y", &bbox_t::y, "w", &bbox_t::w, "h", &bbox_t::h, "obj_id", &bbox_t::obj_id,
                                  "prob", &bbox_t::prob);

    m_luaApi.new_usertype<OS::PointerLocation>("pointer", "x", &OS::PointerLocation::x, "y", &OS::PointerLocation::y);

    m_luaApi.new_usertype<Util::RoutePoint>("Route", "x", &Util::RoutePoint::x, "y", &Util::RoutePoint::y, "time",
                                            &Util::RoutePoint::utime);

    m_luaApi.set_function("usleep", &HAPI::sleep);
    m_luaApi.set_function("checkKey", &HAPI::checkKey);

    m_luaApi.set_function("listen", &HAPI::listenKey);
    m_luaApi.set_function("clear", &HAPI::clearKeys);
    m_luaApi.set_function("checkKey", &HAPI::checkKey);
    m_luaApi.set_function("usleep", &HAPI::sleep);

    // Api
    m_luaApi.set_function("pointerLocation", &HAPI::pointerLocation);
    m_luaApi.set_function("center", &HAPI::centerPointer);
    m_luaApi.set_function("click", &HAPI::click);
    m_luaApi.set_function("moveMouse", &HAPI::moveMouse);
    m_luaApi.set_function("timeClicked", &HAPI::timeClicked);
    m_luaApi.set_function("setDecay", &HAPI::setDecay);
    m_luaApi.set_function("setIncrement", &HAPI::setIncrement);
    m_luaApi.set_function("setMouseRate", &HAPI::setMouseRate);
    m_luaApi.set_function("clickState", &HAPI::getClickState);
    m_luaApi.set_function("log", &HAPI::log);
    m_luaApi.set_function("time", &HAPI::time);
    m_luaApi.set_function("ignore", &HAPI::ignoreArea);
    m_luaApi.set_function("clearIgnore", &HAPI::clearIgnore);

    m_luaApi.set_function("setKeyboardRate", &HAPI::setKeyboardRate);
    m_luaApi.set_function("strict", &HAPI::strictKeyboard);

    m_luaApi.set_function("clearTeams", &ApiWorker::clearTeams, this);
    m_luaApi.set_function("newTeam", &ApiWorker::newTeam, this);
    m_luaApi.set_function("targetID", &ApiWorker::addTargetId, this);

    m_luaApi.set_function("readRouteFile", &HAPI::readRoute);
    m_luaApi.set_function("clearRoutes", &HAPI::clearRoutes);
    m_luaApi.set_function("recoil", &HAPI::getLocation);
    m_luaApi.set_function("resetRecoil", &HAPI::startRoute);
    m_luaApi.set_function("travel", &HAPI::travelRoute);
    m_luaApi.set_function("setRecoil", &HAPI::setRoute);
    m_luaApi.set_function("travelCurrent", &HAPI::travelCurrentRoute);
    m_luaApi.set_function("resetRecoilCurrent", &HAPI::startCurrentRoute);
    m_luaApi.set_function("recoilCurrent", &HAPI::getCurrentLocation);

    m_luaApi.set_function("setCaptureOffset", &HAPI::setCaptureOffset);
    m_luaApi.set_function("setCaptureCenter", &HAPI::setCaptureCenter);
    m_luaApi.set_function("setDetectionNetRatio", &HAPI::setDetectionNetRatio);
    m_luaApi.set_function("setNetworkResolution", &HAPI::setNetworkResolution);

    m_luaApi.set_function("getWeapon", &HAPI::getWeapon);
    m_luaApi.set_function("getWeaponPath", &HAPI::getWeaponPath);
    m_luaApi.set_function("getUtilPath", &HAPI::getUtilPath);
    m_luaApi.set_function("setWeapon", &HAPI::setWeapon);
    m_luaApi.set_function("scriptsPath", &HAPI::scriptsPath);
    m_luaApi.set_function("save", &ApiWorker::saveImage, this);

    m_luaApi.set_function("refreshTrainData", &HAPI::refreshTrainData);
    m_luaApi.set_function("train", &HAPI::train);
    m_luaApi.set_function("training", &HAPI::training);

    m_lastTime = startTime;

    loadScript(Settings::scripts.entry);
}

void ApiWorker::saveImage() { m_save = true; }

void ApiWorker::work() {

    Framebuffer &fb = HBuffer::getFramebuffer(HBuffer::PRESENT);
    if (fb.m_targeted)
        return;

    std::lock_guard<std::mutex> lock(m_mutex);

    OS::PointerLocation p = HAPI::pointerLocation();

    sol::function targetingFunction = m_luaApi["target"];
    sol::function aimingFunction = m_luaApi["aim"];
    sol::function keytest = m_luaApi["keyMain"];

    int32_t detectionTime = getTimeDifference(fb.m_endDetect, fb.m_beginDetect);
    int32_t captureTime = getTimeDifference(fb.m_beginProcess, fb.m_beginCapture);
    int32_t lastTime = getTimeDifference(getCurrentTimeMicro(), m_lastTime);

    m_luaApi["mx"] = p.x;
    m_luaApi["my"] = p.y;
    m_luaApi["cursor"] = p;
    m_luaApi["detectionTime"] = detectionTime;
    m_luaApi["captureTime"] = captureTime;
    m_luaApi["detections"] = fb.m_detections;
    m_luaApi["lastTime"] = lastTime;

    if (!m_teams.empty())
        m_luaApi["tid"] = m_teams[m_targetTeam];

    int closest = targetingFunction();
    if (closest != 0) {
        m_currentTarget = fb.m_detections[closest - 1];
        aimingFunction(fb.m_detections[closest - 1]);
    } else {
        m_currentTarget = {0};
    }

    fb.m_targeted = true;

    keytest();

    m_lastTime = startTime;

    if (m_save) {
        m_save = false;
        std::string imageName = m_savePrefix + std::to_string(m_saveCount) + ".jpg";
        std::string imageNameFull = OS::datasetPath + "images/" + imageName;
        std::string annotationName = OS::datasetPath + "annotations/" + m_savePrefix + std::to_string(m_saveCount) + ".xml";
        std::string annotationNameYolo = OS::datasetPath + "images/" + m_savePrefix + std::to_string(m_saveCount) + ".txt";
        fb.save(imageNameFull);
        fb.saveAnnotation(annotationName, imageName);
        fb.saveAnnotationYOLO(annotationNameYolo);
        m_saveCount++;
    }
}