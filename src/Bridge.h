#ifndef BRIDGE_H
#define BRIDGE_H

/*
 * Bridge is the actual controller that bridges Serial <-> MIDI. Create it by passing the
 * the serial & MIDI ports to use (or NULL if that part is not used.)
 *
 * The Bridge manages its own internal state, and can be deleted when you stop bridging and/or
 * recreated with new settings.
 */

#include <QObject>
#include <QTime>
#include <QThread>
#include <QMutex>
#include "RtMidi.h"
#include "QRtMidiIn.h"
#include "qextserialport.h"
#include "PortLatency.h"

const QString TEXT_NOT_CONNECTED = "(Not Connected)";
const QString TEXT_NEW_PORT = "(Create new port)";

#define NAME_MIDI_IN1 "MIDI1->Serial"
#define NAME_MIDI_IN2 "MIDI2->Serial"
#define NAME_MIDI_IN3 "MIDI3->Serial"
#define NAME_MIDI_IN4 "MIDI4->Serial"
#define NAME_MIDI_IN5 "MIDI5->Serial"
#define NAME_MIDI_IN6 "MIDI6->Serial"
#define NAME_MIDI_OUT "Serial->MIDI"

class Bridge : public QObject
{
    Q_OBJECT
public:
    explicit Bridge(bool multiport);

    // Destroying an existing Bridge will cleanup state & release all ports
    ~Bridge();

signals:
    // Signals to push user status messages
    void displayMessage(QString message);
    void debugMessage(QString message);

    // Signals to trigger UI "blinkenlight" updates
    void midiReceived1();
    void midiReceived2();
    void midiReceived3();
    void midiReceived4();
    void midiReceived5();
    void midiReceived6();
    void midiSent();
    void serialTraffic();
private slots:
    void attach(QString serialName, PortSettings serialSettings, int midiInPort1, int midiInPort2, int midiInPort3, int midiInPort4, int midiInPort5, int midiInPort6, int midiOutPort);
    void onMidiIn1(double timeStamp, QByteArray message);
    void onMidiIn2(double timeStamp, QByteArray message);
    void onMidiIn3(double timeStamp, QByteArray message);
    void onMidiIn4(double timeStamp, QByteArray message);
    void onMidiIn5(double timeStamp, QByteArray message);
    void onMidiIn6(double timeStamp, QByteArray message);
    void onSerialAvailable();
private:
    void sendMidiMessage();

    void onDataByte(uint8_t byte);
    void onStatusByte(uint8_t byte);

    QString applyTimeStamp(QString message);
    QString describeMIDI(QByteArray &buf);

    bool bufferStartsWith(uint8_t byte) { return this->msg_data.length() && (uint8_t)msg_data[0] == byte; }

    int running_status;       // if we have a running or current status byte, what is it?
    int data_expected;        // how many bytes of data are we currently waiting on?
    QByteArray msg_data;      // accumulated message data from the serial port
    QRtMidiIn *midiIn1;
    QRtMidiIn *midiIn2;
    QRtMidiIn *midiIn3;
    QRtMidiIn *midiIn4;
    QRtMidiIn *midiIn5;
    QRtMidiIn *midiIn6;
    RtMidiOut *midiOut;
    int midiInPort1;
    int midiInPort2;
    int midiInPort3;
    int midiInPort4;
    int midiInPort5;
    int midiInPort6;
    int midiOutPort;
    QextSerialPort *serial;
    PortLatency *latency;
    QTime attachTime;
    void onMidiIn(int port, QByteArray message);
    void midiReceived(int port);
    QMutex mutex_serialWrite;
    bool multiport;
    int lastMidiInPort;
    double lastMidiInTime;
};

#endif // BRIDGE_H
