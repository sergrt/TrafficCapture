#include "capture.h"
//#include <QTest>
#include <QNetworkInterface>
#include "TLV.h"
#include <random>

////////////////////////////////////////////////////////////////
/// \brief uDelay
/// \param val - delay value, microseconds
///
/// Cross-platform microseconds sleep function, see http://sergrt.com/2016/01/15/5/
/// for details
void uDelay(int val) {
    if (val > 1000 * 1000 || val == 0)
        return;
    //// This does not work under windows - sleeping too long
    //std::this_thread::sleep_for(std::chrono::microseconds(val));
    //return;

    #ifdef WIN32
        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        const double PCFreq = double(li.QuadPart)/1000000.0;
        QueryPerformanceCounter(&li);
        __int64 CounterStart = li.QuadPart;
        while (true) {
            QueryPerformanceCounter(&li);
            const double sleep = double(li.QuadPart-CounterStart)/PCFreq;
            if (sleep > val)
                break;
        }
    #else
        timespec a;
        clock_gettime(CLOCK_REALTIME, &a);
        ldiv_t t = ldiv(a.tv_nsec + val*1000, 1000000000l);
        timespec ts;
        ts.tv_sec = a.tv_sec + t.quot;
        ts.tv_nsec = t.rem;
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL);
    #endif
}

void rmemcpy(unsigned char* dst, const unsigned char* const src, const unsigned int count) {
    for (unsigned int i = 0; i < count; ++i)
        dst[count - 1 - i] = src[i];
}
Capture::Capture(bool* stop, const CaptureSettings& captureSettings)
    : stopThread {stop}, captureSettings(captureSettings) {

}

Capture::~Capture(void) {
}

// Some code commented out as it is not part of LE specification, 
// but it could be useful in future
void Capture::decode_light(vector<unsigned char>& data, SvDecoded& res) {
    // Exclude ethernet header
    int trimFront = 22; //without TPID
    if (data[12] == 0x81 && data[13] == 0x00) // with TPID
        trimFront = 26;

    data.erase(data.begin(), data.begin() + trimFront);
    TLV savPDU(data);

    vector<unsigned char> savPDU_data;
    savPDU.getData(savPDU_data);

    TLV noASDU(savPDU_data);

    vector<unsigned char>& sequenceASDU_data = savPDU_data;
    unsigned int tmp_len = noASDU.getFullLength();
    sequenceASDU_data.erase(sequenceASDU_data.begin(), sequenceASDU_data.begin() + tmp_len);

    // check if security TLV present
    /*
    if ( savPDU_data[tmp_len] == 0x81 ) { //exists
        // need to remove this tag
    }
    */
    TLV sequenceASDU(sequenceASDU_data);

    vector<unsigned char> tmp;
    noASDU.getData(tmp);
    unsigned char noASDU_count = tmp[0];

    sequenceASDU.getData(sequenceASDU_data);

    for (unsigned char i = 0; i < noASDU_count; i++) {
        TLV ASDU(sequenceASDU_data);
        //ASDUs.push_back(ASDU);

        TLV& ASDU_cur = ASDU;
        vector<unsigned char> asdu_data;
        ASDU_cur.getData(asdu_data);
        TLV svID(asdu_data);
        asdu_data.erase(asdu_data.begin(), asdu_data.begin() + svID.getFullLength());

        // check for datset
        /*
        if (asdu_data[ 0 ] == 0x81) {
            //remove
        }
        */

        TLV smpCnt(asdu_data);
        asdu_data.erase(asdu_data.begin(), asdu_data.begin() + smpCnt.getFullLength());
        /*
        CTLV confRev(asdu_data);
        asdu_data.erase(asdu_data.begin(), asdu_data.begin() + confRev.getFullLength());
        */
        asdu_data.erase(asdu_data.begin(), asdu_data.begin() + 2 + asdu_data[1]);

        //check for refrTm
        /*
        if (asdu_data[ 0 ] == 0x84) {
            // remove
        }
        */
        /*
        CTLV smpSynch(asdu_data);
        asdu_data.erase(asdu_data.begin(), asdu_data.begin() + smpSynch.getFullLength());
        */
        asdu_data.erase(asdu_data.begin(), asdu_data.begin() + 2 + asdu_data[1]);
        // check for smpRate
        /*
        if (asdu_data[ 0 ] == 0x86) {
            //remove
        }
        */

        TLV sequenceData(asdu_data);
        //const unsigned char IUPhaseValues = 8; // Ua, Ub, Uc, Un, Ia, Ib, Ic, In
        sequenceData.getData(asdu_data);

        int Ua = 0;
        int Ub = 0;
        int Uc = 0;
        int Un = 0;
        int Ia = 0;
        int Ib = 0;
        int Ic = 0;
        int In = 0;

        int QUa = 0;
        int QUb = 0;
        int QUc = 0;
        int QUn = 0;
        int QIa = 0;
        int QIb = 0;
        int QIc = 0;
        int QIn = 0;

        rmemcpy((unsigned char*)&Ia, asdu_data.data(), 4);
        rmemcpy((unsigned char*)&QIa, asdu_data.data() + 4, 4);

        rmemcpy((unsigned char*)&Ib, asdu_data.data() + 8, 4);
        rmemcpy((unsigned char*)&QIb, asdu_data.data() + 12, 4);

        rmemcpy((unsigned char*)&Ic, asdu_data.data() + 16, 4);
        rmemcpy((unsigned char*)&QIc, asdu_data.data() + 20, 4);

        rmemcpy((unsigned char*)&In, asdu_data.data() + 24, 4);
        rmemcpy((unsigned char*)&QIn, asdu_data.data() + 28, 4);

        rmemcpy((unsigned char*)&Ua, asdu_data.data() + 32, 4);
        rmemcpy((unsigned char*)&QUa, asdu_data.data() + 36, 4);

        rmemcpy((unsigned char*)&Ub, asdu_data.data() + 40, 4);
        rmemcpy((unsigned char*)&QUb, asdu_data.data() + 44, 4);

        rmemcpy((unsigned char*)&Uc, asdu_data.data() + 48, 4);
        rmemcpy((unsigned char*)&QUc, asdu_data.data() + 52, 4);

        rmemcpy((unsigned char*)&Un, asdu_data.data() + 56, 4);
        rmemcpy((unsigned char*)&QUn, asdu_data.data() + 60, 4);

        smpCnt.getData(tmp);
        int counter = 0;
        rmemcpy((unsigned char*)&counter, tmp.data(), tmp.size());

        res.counter.push_back(counter);

        svID.getData(tmp);
        string svID_str;
        for(int x = 0; x < tmp.size(); x++)
            svID_str.push_back(tmp[x]);

        res.svID.push_back(svID_str);

        res.Ua.push_back(Ua/100.0);
        res.Ub.push_back(Ub/100.0);
        res.Uc.push_back(Uc/100.0);
        res.Un.push_back(Un/100.0);

        res.QUa.push_back(QUa);
        res.QUb.push_back(QUb);
        res.QUc.push_back(QUc);
        res.QUn.push_back(QUn);

        res.Ia.push_back(Ia/1000.0);
        res.Ib.push_back(Ib/1000.0);
        res.Ic.push_back(Ic/1000.0);
        res.In.push_back(In/1000.0);

        res.QIa.push_back(QIa);
        res.QIb.push_back(QIb);
        res.QIc.push_back(QIc);
        res.QIn.push_back(QIn);

        unsigned short len = ASDU.getFullLength();
        sequenceASDU_data.erase(sequenceASDU_data.begin(), sequenceASDU_data.begin() + len);
    }
}

void Capture::setCaptureSettings(const CaptureSettings& captureSettings) {
    this->captureSettings = captureSettings;
}

void Capture::run() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t* alldevs;
    pcap_findalldevs(&alldevs, errbuf);
    pcap_if_t *d = alldevs;

    QList<QNetworkInterface> ifs = QNetworkInterface::allInterfaces();
    int ifsID = 0;
    while (ifsID < ifs.count()) {
        if (ifs.at(ifsID).hardwareAddress() == captureSettings.interfaceMac)
            break;
        ifsID++;
    }
    
    QString ifName = ifs.at(ifsID).name();
    QString curID = QString(d->name);
    curID = curID.right(curID.length() - curID.indexOf('{'));
    while (ifName != curID) {
        d = d->next;
        curID = QString(d->name);
        curID = curID.right(curID.length() - curID.indexOf('{'));
    }

    pcapHandle = pcap_open_live(d->name,            // name of the device
        4096,                // portion of the packet to capture
        1,  // promiscuous mode
        1000,               // read timeout
        errbuf              // error buffer
        );

    // compile and set filters
    struct bpf_program filter;
    QString streamFilter = "(ether proto 0x8100 or ether proto 0x88ba) and ether dst " + captureSettings.dstMac;
    char* filter_app = new char[streamFilter.length() + 1];
    memcpy(filter_app, streamFilter.toLocal8Bit().data(), streamFilter.length());
    filter_app[streamFilter.length()] = 0x00;

    bpf_u_int32 mask;
    bpf_u_int32 net;

    //Get net address and interface mask
    pcap_lookupnet(d->name, &net, &mask, errbuf);

    if (pcap_compile(pcapHandle, &filter, filter_app, 0, mask) == -1) {
        //exit(-1);
    }
    delete [] filter_app;

    if (pcap_setfilter(pcapHandle, &filter) == -1) {
        //exit(-1);
    }

    dataMeasured.clear();
    emit setCaptureIsRunning(true);

    bool doAdd = false; // We are capturing first MAX_DATA packets every second
                        // (SV counter == 0 on second tick), so we need this flag
                        // to indicate that we are adding captured packet to data vector

    // Note that every possible data manipulation moved out of capture loop to save
    // some processor time
    const std::string streamName = captureSettings.streamId.toLocal8Bit();
    /*const */int MAX_DATA = 256; // If we fix this value, rms will be the same for static signals
                                  // So we make this value random every time to add some dynamics to GUI
    std::default_random_engine re;
    uniform_int_distribution<> dist {10, 300};
    unsigned int time_ui = unsigned int(time(NULL));
    re.seed(time_ui);

    while(!*stopThread) {
        pcap_pkthdr* pkt_header;
        const u_char* pkt_data;
        int res = pcap_next_ex(pcapHandle, &pkt_header, &pkt_data);

        if (res == 1) { //valid data
            vector<unsigned char> packet_data;
            packet_data.resize(pkt_header->caplen);
            memcpy(packet_data.data(),pkt_data,pkt_header->caplen);

            SvDecoded d;
            decode_light(packet_data, d);

            if (d.svID[0] == streamName) {
                if (!doAdd) {
                    if (d.counter[0] == 0) {
                        doAdd = true;
                        // Initialize MAX_DATA counter, this will cause RMS value to change
                        // because of different data size captured
                        MAX_DATA = dist(re);
                    }
                }
                if (doAdd) {
                    if (dataMeasured.size() < MAX_DATA) {
                        dataMeasured.push_back(d);
                    } else {
                        ensureDataIntegrity();
                        processMeasuredData();
                        doAdd = false;
                    }
                }
            }
        } else if (res == 0) { //timeout - do nothing
            // send we've got so far
            processMeasuredData();
            doAdd = false;
        } else if (res < 0) {
            *stopThread = true;
        }
    }

    pcap_close(pcapHandle);
    emit setCaptureIsRunning(false);
}

void Capture::processMeasuredData() {
    // we do not want nested event loops or callback signals here
    // as we should process as fast as possible, so here is simple
    // loop waiting for flag set that data display is done

    bool ready = false;
    //qDebug() << dataMeasured.size();
    emit displayData(&dataMeasured, &ready);
    while(!ready)
        uDelay(3);
    dataMeasured.clear();
}

bool Capture::ensureDataIntegrity() const {
    bool res = true;
    if (dataMeasured.size() > 0) {
        const int MAX_SV_COUNTER = dataMeasured[0].counter.size() == 1 ? 4000 : 12799;

        int expectedCnt = dataMeasured[0].counter[0] - 1;
        if (expectedCnt < 0)
            expectedCnt = MAX_SV_COUNTER - 1;

        for (const auto& d : dataMeasured) {
            for (const auto& curCnt : d.counter) {
                ++expectedCnt;
                if (expectedCnt == MAX_SV_COUNTER)
                    expectedCnt = 0;
                if (curCnt != expectedCnt) {
                    qDebug() << "Gap detected";
                    res = false;
                }
            }
        }
    }

    return res;
}
