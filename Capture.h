// Capture IEC61850-9.2LE stream thread

#pragma once
#include <QThread>
#include <CaptureSettings.h>
#include "SvDecoded.h"

#define WPCAP
#define HAVE_REMOTE
#include "pcap.h"
#undef min

class Capture : public QThread {
    Q_OBJECT

public:
    Capture(bool* stop, const CaptureSettings& captureSettings);
    ~Capture(void);
    void setCaptureSettings(const CaptureSettings& captureSettings);
    void run();
private:
    bool* stopThread;
    CaptureSettings captureSettings;

    pcap_t* pcapHandle;
    void decode_light(vector<unsigned char>& data, SvDecoded& res);  // Faster, but not so self-explaining
    std::vector<SvDecoded> dataMeasured;
    void processMeasuredData();
    bool ensureDataIntegrity() const;
signals:
    void setCaptureIsRunning(bool);
    void displayData(vector<SvDecoded>*,bool* ready);
};
