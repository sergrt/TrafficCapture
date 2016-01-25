#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "Capture.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    bool captureStopped;
    std::unique_ptr<Capture> capture;
public slots:
    void onStartStopClicked();
    void onSetCaptureIsRunning(bool isRunning);
    void onDisplayData(vector<SvDecoded>* data, bool* ready);
};

#endif // MAINWINDOW_H
