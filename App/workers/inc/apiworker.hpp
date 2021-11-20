#ifndef APIWORKER_HPP
#define APIWORKER_HPP

#include "INeuralNet.hpp"
#include "settings.hpp"
#include "sol.hpp"
#include "threadable.hpp"
#include <vector>

class ApiWorker : public Threadable {
  private:
    std::vector<uint32_t> m_team1;
    std::vector<uint32_t> m_team2;
    std::vector<std::vector<uint32_t>> m_teams;
    uint32_t m_targetTeam;
    std::vector<FloatArea_t> m_ignoreAreas;
    sol::state m_luaApi;
    bbox_t m_currentTarget = {0};
    timeStamp m_lastTime;
    std::string m_savePrefix;
    uint32_t m_saveCount;
    bool m_save;

    void clearTeams();
    void newTeam();
    void addTargetId(int id);
    bool loadScript(char *filename);

  public:
    ApiWorker(HCore::HCore *core);
    ~ApiWorker();
    void reload();
    std::vector<FloatArea_t> getIgnores();
    std::vector<uint32_t> getTargetIds();
    void clearIgnores();
    void ignoreArea(float x0, float y0, float w, float h);
    bbox_t getCurrentTarget();
    void saveImage();
    void switchTeams();
    void work();
};

#endif