#include <QWidget>
#include <QDebug>
#include <QDesktopWidget>
#include <QPushButton>
#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include <QTimer>
#include <vector>
#include "bbox.hpp"
#include "framebuffer.hpp"


class TransWindow : public QWidget{
    Q_OBJECT
public:
    explicit TransWindow(QWidget *parent = 0);
    void drawBoxes(std::vector<bbox_t> boxes);

private:

    void printStats(Framebuffer &fb);

    QPainter painter;
    QTimer *m_timer;
    int m_width,m_height;


protected:
    void paintEvent(QPaintEvent *event);
};