

#include "mainwindow.h"

#include "fancyslider.h"
#include <QApplication>
#include <QCheckBox>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QMenuBar>
#include <QSlider>
#include <QtWidgets>

#include "filesystem.hpp"
#include "filewidget.hpp"
#include "glwidget.h"
#include "settings.hpp"
#include "settingswidget.hpp"
#include "workers.hpp"

namespace GUI {

MainWindow::MainWindow() {
    setFixedSize(640, 600);

    QMenuBar *menuBar = new QMenuBar;
    QMenu *fileMenu = new QMenu(this);
    menuBar->addMenu(fileMenu)->setText(tr("&File"));
    // fileMenu->addAction(tr("&Load Settings"),this,
    // &MainWindow::loadSettings);
    fileMenu->addAction("E&xit", this, &QWidget::close);

    QVBoxLayout *container = new QVBoxLayout;

    SettingsWidget *settei = new SettingsWidget(this);
    FileWidget *files = new FileWidget(this);
    container->addWidget(settei);
    container->addWidget(files);

    QWidget *w = new QWidget;
    w->setLayout(container);
    w->setParent(this);
    w->setAttribute(Qt::WA_QuitOnClose);

    setMenuBar(menuBar);
}

void MainWindow::closeEvent(QCloseEvent *event) { QApplication::quit(); }

static void initializeSettingFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode) {
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        dialog.setDirectory(QString::fromStdString(OS::hayhaPath));
    }

    dialog.setDefaultSuffix("cfg");
}

void MainWindow::loadSettings() {
    QFileDialog dialog(this, tr("Open File"));
    initializeSettingFileDialog(dialog, QFileDialog::AcceptOpen);

    if (dialog.exec() != QDialog::Accepted)
        return;
    QByteArray ba = dialog.selectedFiles().first().toLatin1();
    Settings::loadSettings(ba.data());
}

void MainWindow::resizeEvent(QResizeEvent *) {}

void MainWindow::updateDetectionRate(int value) {
    value = (std::max)(1, value);
    Workers::detect->setRate(value);
}
void MainWindow::updateCaptureRate(int value) {
    value = (std::max)(1, value);
    Workers::capture->setRate(value);
}
void MainWindow::updateThresh(int value) { Settings::detection.thresh = value; }

void MainWindow::updateSense(int value) {}

void MainWindow::updateSlider(int value) {
    Settings::net.h = value * 32;
    Settings::processSettings();
}

void MainWindow::updateSliderTwo(int value) {
    Settings::net.w = value * 32;
    Settings::processSettings();
}

void MainWindow::setLimit(bool value) { Workers::setLimit(value); }

void MainWindow::startButton() { Workers::start(); }

void MainWindow::switchButton() { Workers::apiworker->switchTeams(); }

void MainWindow::reloadButton() { Workers::apiworker->reload(); }

void MainWindow::benchmarkButton() { Workers::detect->benchmark(); }

} // namespace GUI