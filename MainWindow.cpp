#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QNetworkInterface>
#include <algorithm>

const QString MAC_PROPERTY_NAME = "Mac";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    connect(ui->bnStartStop, &QPushButton::clicked, this, &MainWindow::onStartStopClicked);
    onSetCaptureIsRunning(false);

    QList<QNetworkInterface> ifs = QNetworkInterface::allInterfaces();
    for (int i = 0; i < ifs.count(); ++i) {
        if (ifs.at(i).hardwareAddress().length() == 17) { // MAC 6 bytes
            QString ipStr = "---.---.---.---";
            for (int x = 0; x < ifs.at(i).addressEntries().count(); x++) {
                if (ifs.at(i).addressEntries().at(x).ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    ipStr = ifs.at(i).addressEntries().at(x).ip().toString();
                    break;
                }
            }

            // lambda is handy if you need more than one combobox with macs, for example
            // if you use one of yours interfaces as dstMac
            auto setMac = [](QComboBox& e, const QString& mac) {
                e.addItem(mac);
                e.setProperty(QString(MAC_PROPERTY_NAME + "%1").arg(e.count() - 1).toLocal8Bit(), mac);
            };

            QString s = ifs.at(i).hardwareAddress();
            setMac(*ui->cbNetworkInterface, s);
        }
    }

    ui->cbNetworkInterface->setCurrentIndex(0);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onStartStopClicked() {
    if (!capture || (captureStopped && !capture->isRunning())) {
        const QString captureInterfaceMac = ui->cbNetworkInterface->property(QString(MAC_PROPERTY_NAME + "%1").arg(ui->cbNetworkInterface->currentIndex()).toLocal8Bit()).toString();
        const QString dstMac = ui->leDstMac->text();
        const QString streamId = ui->leSvId->text();
        CaptureSettings captureSettings;
        captureSettings.interfaceMac = captureInterfaceMac;
        captureSettings.dstMac = dstMac;
        captureSettings.streamId = streamId;

        if (!capture) {
            capture.reset(new Capture(&captureStopped, captureSettings));
            connect(capture.get(), &Capture::setCaptureIsRunning, this, &MainWindow::onSetCaptureIsRunning);
            connect(capture.get(), &Capture::displayData, this, &MainWindow::onDisplayData);
        } else {
            capture->setCaptureSettings(captureSettings);
        }

        captureStopped = false;
        capture->start();
    } else {
        captureStopped = true;
    }
}

void MainWindow::onSetCaptureIsRunning(bool isRunning) {
    ui->bnStartStop->setText(isRunning ? tr("Остановить") : tr("Запустить"));
}

void MainWindow::onDisplayData(vector<SvDecoded>* data, bool* ready) {
    double sum = 0.0;
    for (const SvDecoded& d : *data)
        sum += std::accumulate(d.Ua.begin(), d.Ua.end(), 0.0);

    const double rms = sum / sqrt(2);
    ui->leRms->setText(QString("%1").arg(rms, 0, 'f', 6, QChar(' ')));
    *ready = true;
}
