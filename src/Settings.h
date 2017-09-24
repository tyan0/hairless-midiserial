#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include "qextserialport.h"

// Note that main.cpp sets QCoreApplication::applicationName and such, allowing us to use a
// plain QSettings() constructor

class Settings {
public:
    static QString getLastSerialPort() { return QSettings().value("lastSerialPort", "").toString(); }
    static void setLastSerialPort(QString port) { QSettings().setValue("lastSerialPort", port); }

    static QString getLastMidiIn1() { return QSettings().value("lastMidiIn1", "").toString(); }
    static QString getLastMidiIn2() { return QSettings().value("lastMidiIn2", "").toString(); }
    static QString getLastMidiIn3() { return QSettings().value("lastMidiIn3", "").toString(); }
    static QString getLastMidiIn4() { return QSettings().value("lastMidiIn4", "").toString(); }
    static QString getLastMidiIn5() { return QSettings().value("lastMidiIn5", "").toString(); }
    static QString getLastMidiIn6() { return QSettings().value("lastMidiIn6", "").toString(); }
    static void setLastMidiIn1(QString port) { QSettings().setValue("lastMidiIn1", port); }
    static void setLastMidiIn2(QString port) { QSettings().setValue("lastMidiIn2", port); }
    static void setLastMidiIn3(QString port) { QSettings().setValue("lastMidiIn3", port); }
    static void setLastMidiIn4(QString port) { QSettings().setValue("lastMidiIn4", port); }
    static void setLastMidiIn5(QString port) { QSettings().setValue("lastMidiIn5", port); }
    static void setLastMidiIn6(QString port) { QSettings().setValue("lastMidiIn6", port); }

    static QString getLastMidiOut() { return QSettings().value("lastMidiOut", "").toString(); }
    static void setLastMidiOut(QString port) { QSettings().setValue("lastMidiOut", port); }

    static int getScrollbackSize() { return QSettings().value("scrollbackSize", 75).toInt(); }
    static void setScrollbackSize(int newSize) { QSettings().setValue("scrollbackSize", newSize); }

    static bool getDebug() { return QSettings().value("debug", false).toBool(); }
    static void setDebug(bool debug) { QSettings().setValue("debug", debug); }

    static bool getMultiport() { return QSettings().value("multiport", false).toBool(); }
    static void setMultiport(bool multiport) { QSettings().setValue("multiport", multiport); }

    static PortSettings getPortSettings() {
        PortSettings result;
        QSettings settings;
        result.BaudRate = (BaudRateType) settings.value("baudRate", (int)BAUD115200).toInt();
        result.DataBits = (DataBitsType) settings.value("dataBits", (int)DATA_8).toInt();
        result.FlowControl = (FlowType)  settings.value("flowControl", (int)FLOW_OFF).toInt();
        result.Parity = (ParityType) settings.value("parity", (int)PAR_NONE).toInt();
        result.StopBits = (StopBitsType) settings.value("stopBits", (int)STOP_1).toInt();
        result.Timeout_Millisec = -1; // not used when qextserialport is event-driven, anyhow
        return result;
    }

    static void setPortSettings(PortSettings newSettings) {
        QSettings settings;
        settings.setValue("baudRate", (int)newSettings.BaudRate);
        settings.setValue("dataBits", (int)newSettings.DataBits);
        settings.setValue("flowControl", (int)newSettings.FlowControl);
        settings.setValue("parity", (int)newSettings.Parity);
        settings.setValue("stopBits", (int)newSettings.StopBits);
    }

};


#endif // SETTINGS_H
