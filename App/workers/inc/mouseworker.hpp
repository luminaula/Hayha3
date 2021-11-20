#include "hcore.hpp"
#include <queue>

#define MOUSE_MOVE 1
#define MOUSE_CLICK_DOWN 2
#define MOUSE_CLICK_UP 3

struct MouseNode {
    int code;
    float x, y;
    timeStamp m_time;

    bool operator()(const MouseNode &left, const MouseNode &right) { return left.m_time > right.m_time; }
};

class MouseWorker : public Threadable {
  private:
    std::queue<MouseNode> m_mouseQueue;
    std::priority_queue<MouseNode, std::vector<MouseNode>, MouseNode> m_moveQ;
    float accumX;
    float accumY;
    int sensitivity;

  public:
    MouseWorker(HCore::HCore *core);
    ~MouseWorker();

    void push(MouseNode node);

    void setSensitivity(int sense) { sensitivity = sense; }

    void work();
};