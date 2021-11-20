#include "hcore.hpp"

class HTracker : public Threadable {
  private:
  public:
    HTracker(HCore::HCore *core);
    void work() override;
};