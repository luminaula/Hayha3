#include "bbox.hpp"
#include "module.hpp"

class TrackerModule : public Module {
  private:
  public:
    MFunc<std::vector<bbox_t>> m_boxes;

    TrackerModule() {}
    void loadModule() override;
};