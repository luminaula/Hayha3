#include "bbox.hpp"
#include "framebuffer.hpp"
#include <QDebug>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QTimer>
#include <QWidget>
#include <vector>

class TransWindow : public QWidget {
    Q_OBJECT
  public:
    explicit TransWindow(QWidget *parent = 0);
    void drawBoxes(std::vector<bbox_t> boxes);

  private:
    void printStats(Framebuffer &fb);

    QPainter painter;
    QTimer *m_timer;
    int m_width, m_height;

  protected:
    void paintEvent(QPaintEvent *event);
};