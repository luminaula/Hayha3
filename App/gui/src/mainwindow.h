
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "glwidget.h"
#include <QCloseEvent>
#include <QGridLayout>
#include <QMainWindow>
#include <QSpinBox>
#include <QTimer>

namespace GUI {

QT_FORWARD_DECLARE_CLASS(QOpenGLWidget)

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow();

    void resizeEvent(QResizeEvent *);

  private slots:
    void loadSettings();
    void updateDetectionRate(int value);
    void updateCaptureRate(int value);
    void updateThresh(int value);
    void updateSense(int value);
    void updateSlider(int value);
    void updateSliderTwo(int value);
    void setLimit(bool value);
    void startButton();
    void switchButton();
    void reloadButton();
    void benchmarkButton();

  private:
    void closeEvent(QCloseEvent *event);
    QGridLayout *m_layout;
};

} // namespace GUI

#endif
