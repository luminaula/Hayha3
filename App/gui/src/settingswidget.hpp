#include <QWidget>

#include "mainwindow.h"

namespace GUI {

class SettingsWidget : public QWidget {
    Q_OBJECT
  public:
    SettingsWidget(MainWindow *main);
  private slots:
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
    void trainButton();

  private:
};

} // namespace GUI