

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "bbox.hpp"
#include "framebuffer.hpp"
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QPainter>
#include <QPushButton>
#include <QTime>
#include <QVector>
#include <QWidget>

namespace GUI {

class MainWindow;

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
  public:
    GLWidget();
    ~GLWidget();

  public slots:

  private slots:

  protected:
    void paintGL() override;
    void initializeGL() override;

  private:
    void printStats(Framebuffer &fb);
    void drawDetections(Framebuffer &fb);
    bbox_t translateBox(bbox_t det, Framebuffer &fb);

    void drawTarget(Framebuffer &fb);

    int m_width;
    int m_height;

    std::vector<QImage> images;
    
    QPainter painter;

    QTimer *m_timer;
    MainWindow *mainWindow;
};

class DetectionWindow : public QWidget {
    Q_OBJECT
  public:
    DetectionWindow(MainWindow *mw);

  protected:
  private slots:
    void updateIntervalChanged(int value);

  private:
    GLWidget *m_glWidget;
    QTimer *m_timer;
    MainWindow *mainWindow;
};

} // namespace GUI

#endif
