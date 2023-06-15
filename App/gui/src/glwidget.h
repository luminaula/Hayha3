

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QTime>
#include <QVector>
#include <QPushButton>
#include <QPainter>
#include <QWidget>
#include "framebuffer.hpp"
#include "bbox.hpp"


namespace GUI{

    class MainWindow;


    class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions{
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
        bbox_t translateBox(bbox_t det,Framebuffer &fb);

        void drawTarget(Framebuffer &fb);

        int m_width;
        int m_height;

        QImage m_image;
        QPainter painter;
    };

    class DetectionWindow : public QWidget{
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

}

#endif
