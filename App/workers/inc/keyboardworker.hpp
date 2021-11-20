#include "hcore.hpp"
#include <map>
#include <queue>

struct KeyboardNode {
    bool listen;
    bool pressed;
    timeStamp time;
    uint32_t triggerTime;
};

class KeyboardWorker : public Threadable {
  private:
    std::map<uint32_t, KeyboardNode> m_keystates;
    bool m_strict;

  public:
    KeyboardWorker(HCore::HCore *core);
    ~KeyboardWorker();
    void listen(uint32_t code);
    void setStrict(bool value);
    void clearListen();
    bool check(uint32_t code);
    void work() override;
};