#include "settingswidget.hpp"
#include "settings.hpp"
#include "workers.hpp"

#include "fancyslider.h"
#include <QApplication>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>

namespace GUI {

SettingsWidget::SettingsWidget(MainWindow *main) : QWidget(main) {

    QHBoxLayout *container = new QHBoxLayout;

    QGroupBox *ticks = new QGroupBox;
    QGridLayout *tickLayout = new QGridLayout;

    QGroupBox *buttons = new QGroupBox;
    QGridLayout *buttonLayout = new QGridLayout;

    QGroupBox *sliders = new QGroupBox;
    QGridLayout *sliderLayout = new QGridLayout;

    tickLayout->setAlignment(Qt::AlignTop);
    buttonLayout->setAlignment(Qt::AlignTop);

    QSpinBox *updateInterval = new QSpinBox(this);
    updateInterval->setRange(1, 2500);
    updateInterval->setSuffix(" fps");
    updateInterval->setValue((int)Workers::detect->getRate());

    connect(updateInterval, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsWidget::updateDetectionRate);

    tickLayout->addWidget(new QLabel(tr("Detection rate")), 0, 0);
    tickLayout->addWidget(updateInterval, 0, 1);

    updateInterval = new QSpinBox(this);
    updateInterval->setRange(1, 2500);
    updateInterval->setSuffix(" fps");
    updateInterval->setValue((int)Workers::capture->getRate());

    connect(updateInterval, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsWidget::updateCaptureRate);

    tickLayout->addWidget(new QLabel(tr("Capture rate")), 1, 0);
    tickLayout->addWidget(updateInterval, 1, 1);

    QSpinBox *threshBox = new QSpinBox(this);
    threshBox->setRange(1, 100);
    threshBox->setSuffix(" %");
    threshBox->setValue(Settings::detection.thresh);

    connect(threshBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsWidget::updateThresh);

    tickLayout->addWidget(new QLabel(tr("Thresh")), 2, 0);
    tickLayout->addWidget(threshBox, 2, 1);

    QCheckBox *checker = new QCheckBox(tr("&Set limit"));

    connect(checker, &QAbstractButton::toggled, this, &SettingsWidget::setLimit);

    tickLayout->addWidget(new QLabel(tr("Limit")), 3, 0);
    tickLayout->addWidget(checker, 3, 1);

    QSpinBox *senseBox = new QSpinBox(this);
    senseBox->setRange(1, 10000);
    senseBox->setValue(100);

    connect(senseBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsWidget::updateSense);

    tickLayout->addWidget(new QLabel(tr("Mouse sensitivity")), 4, 0);
    tickLayout->addWidget(senseBox, 4, 1);

    ticks->setLayout(tickLayout);

    QPushButton *button = new QPushButton(tr("Start"));
    connect(button, &QPushButton::clicked, this, &SettingsWidget::startButton);
    buttonLayout->addWidget(button, 0, 0);

    button = new QPushButton(tr("Switch"));
    connect(button, &QPushButton::clicked, this, &SettingsWidget::switchButton);
    buttonLayout->addWidget(button, 1, 0);

    button = new QPushButton(tr("Reload API"));
    connect(button, &QPushButton::clicked, this, &SettingsWidget::reloadButton);
    buttonLayout->addWidget(button, 2, 0);

    button = new QPushButton(tr("Benchmark"));
    connect(button, &QPushButton::clicked, this, &SettingsWidget::benchmarkButton);
    buttonLayout->addWidget(button, 3, 0);

    button = new QPushButton(tr("Train"));
    connect(button, &QPushButton::clicked, this, &SettingsWidget::trainButton);
    buttonLayout->addWidget(button, 4, 0);

    buttons->setLayout(buttonLayout);

    FancySlider *slider = new FancySlider(Qt::Vertical);
    slider->setFocusPolicy(Qt::StrongFocus);
    slider->setTickPosition(QSlider::TicksBothSides);
    slider->setTickInterval(10);
    slider->setSingleStep(10);
    slider->setMaximum(20);
    slider->setMinimum(3);
    slider->setSliderPosition(Settings::net.h / 32);

    connect(slider, QOverload<int>::of(&QSlider::valueChanged), this, &SettingsWidget::updateSlider);

    sliderLayout->addWidget(slider, 0, 0);

    slider = new FancySlider(Qt::Vertical);
    slider->setFocusPolicy(Qt::StrongFocus);
    slider->setTickPosition(QSlider::TicksBothSides);
    slider->setTickInterval(10);
    slider->setSingleStep(10);
    slider->setMaximum(20);
    slider->setMinimum(3);
    slider->setSliderPosition(Settings::net.w / 32);

    connect(slider, QOverload<int>::of(&QSlider::valueChanged), this, &SettingsWidget::updateSliderTwo);

    sliderLayout->addWidget(slider, 0, 1);

    sliders->setLayout(sliderLayout);

    container->addWidget(ticks);
    container->addWidget(buttons);
    container->addWidget(sliders);

    setLayout(container);
}

void SettingsWidget::updateDetectionRate(int value) {
    value = (std::max)(1, value);
    Workers::detect->setRate(value);
}
void SettingsWidget::updateCaptureRate(int value) {
    value = (std::max)(1, value);
    Workers::capture->setRate(value);
}
void SettingsWidget::updateThresh(int value) { Settings::detection.thresh = value; }

void SettingsWidget::updateSense(int value) { Workers::mouseWorker->setSensitivity(value); }

void SettingsWidget::updateSlider(int value) {
    Settings::net.h = value * 32;
    Settings::processSettings();
}

void SettingsWidget::updateSliderTwo(int value) {
    Settings::net.w = value * 32;
    Settings::processSettings();
}

void SettingsWidget::setLimit(bool value) { Workers::setLimit(value); }

void SettingsWidget::startButton() { Workers::start(); }

void SettingsWidget::switchButton() { Workers::apiworker->switchTeams(); }

void SettingsWidget::reloadButton() { Workers::apiworker->reload(); }

void SettingsWidget::benchmarkButton() { Workers::detect->benchmark(); }

void SettingsWidget::trainButton() { Workers::startTrain(); }

} // namespace GUI