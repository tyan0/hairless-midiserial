#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qextserialenumerator.h"
#include "Settings.h"
#include "settingsdialog.h"
#include "aboutdialog.h"

const int LIST_REFRESH_RATE =20; // Hz

static void selectIfAvailable(QComboBox *box, QString itemText)
{
    for(int i = 0; i < box->count(); i++) {
        if(box->itemText(i) == itemText) {
            box->setCurrentIndex(i);
            return;
        }
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    bridge(NULL),
    workerThread(NULL),
    debugListTimer(),
    debugListMessages(),
    pendingStartBridge(false),
    disableOnValueChanged(false)
{
#if defined(__MACOSX_CORE__)
    dummyMidiIn = new RtMidiIn;
#endif
    ui->setupUi(this);
    // Fixed width, minimum height
    this->setMinimumSize(this->size());
    this->setMaximumSize(this->size().width(), 2000);
#ifdef Q_OS_LINUX
    this->setWindowIcon(QIcon(":/images/images/icon48.png"));
#endif

    // Plumb event filter for focus events
    ui->cmbMidiIn1->installEventFilter(this);
    ui->cmbMidiIn2->installEventFilter(this);
    ui->cmbMidiIn3->installEventFilter(this);
    ui->cmbMidiIn4->installEventFilter(this);
    ui->cmbMidiIn5->installEventFilter(this);
    ui->cmbMidiIn6->installEventFilter(this);
    ui->cmbMidiOut->installEventFilter(this);
    ui->cmbSerial->installEventFilter(this);

    // Multiport ON, OFF
    stretchSize = ui->cmbMidiIn6->pos() - ui->cmbMidiIn1->pos() - QPoint(0,15);
    ui->chk_multiport->setChecked(true);
    connect(ui->chk_multiport, SIGNAL(toggled(bool)), ui->cmbMidiIn2, SLOT(setVisible(bool)));
    connect(ui->chk_multiport, SIGNAL(toggled(bool)), ui->cmbMidiIn3, SLOT(setVisible(bool)));
    connect(ui->chk_multiport, SIGNAL(toggled(bool)), ui->cmbMidiIn4, SLOT(setVisible(bool)));
    connect(ui->chk_multiport, SIGNAL(toggled(bool)), ui->cmbMidiIn5, SLOT(setVisible(bool)));
    connect(ui->chk_multiport, SIGNAL(toggled(bool)), ui->cmbMidiIn6, SLOT(setVisible(bool)));
    connect(ui->chk_multiport, SIGNAL(toggled(bool)), ui->led_midiin2, SLOT(setVisible(bool)));
    connect(ui->chk_multiport, SIGNAL(toggled(bool)), ui->led_midiin3, SLOT(setVisible(bool)));
    connect(ui->chk_multiport, SIGNAL(toggled(bool)), ui->led_midiin4, SLOT(setVisible(bool)));
    connect(ui->chk_multiport, SIGNAL(toggled(bool)), ui->led_midiin5, SLOT(setVisible(bool)));
    connect(ui->chk_multiport, SIGNAL(toggled(bool)), ui->led_midiin6, SLOT(setVisible(bool)));
    connect(ui->chk_multiport, SIGNAL(toggled(bool)), this, SLOT(stretchWindow(bool)));

    // Load initial state
    this->workerThread = new QThread();
    this->workerThread->start(QThread::HighestPriority);
    refresh();
    scrollbackSize=Settings::getScrollbackSize();
    ui->chk_debug->setChecked( Settings::getDebug() );
    ui->chk_multiport->setChecked( Settings::getMultiport() );
    selectIfAvailable(ui->cmbMidiIn1, Settings::getLastMidiIn1());
    selectIfAvailable(ui->cmbMidiIn2, Settings::getLastMidiIn2());
    selectIfAvailable(ui->cmbMidiIn3, Settings::getLastMidiIn3());
    selectIfAvailable(ui->cmbMidiIn4, Settings::getLastMidiIn4());
    selectIfAvailable(ui->cmbMidiIn5, Settings::getLastMidiIn5());
    selectIfAvailable(ui->cmbMidiIn6, Settings::getLastMidiIn6());
    selectIfAvailable(ui->cmbMidiOut, Settings::getLastMidiOut());
    selectIfAvailable(ui->cmbSerial, Settings::getLastSerialPort());

    // Set up timer for the display list
    debugListTimer.setSingleShot(true);
    debugListTimer.setInterval(1000 / LIST_REFRESH_RATE);

    // Plumb signals & slots
    connect(ui->cmbMidiIn1, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->cmbMidiIn2, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->cmbMidiIn3, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->cmbMidiIn4, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->cmbMidiIn5, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->cmbMidiIn6, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->cmbMidiOut, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->cmbSerial, SIGNAL(currentIndexChanged(int)), SLOT(onValueChanged()));
    connect(ui->chk_on, SIGNAL(clicked()), SLOT(onValueChanged()));
    connect(ui->chk_debug, SIGNAL(clicked(bool)), SLOT(onDebugClicked(bool)));
    connect(ui->chk_multiport, SIGNAL(clicked(bool)), SLOT(onMultiportClicked(bool)));
    connect(&debugListTimer, SIGNAL(timeout()), SLOT(refreshDebugList()));


    // Menu items
    connect(ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->actionAbout, SIGNAL(triggered()), SLOT(showAboutBox()));
    connect(ui->actionPreferences, SIGNAL(triggered()), SLOT(showPreferences()));

    // Get started
    qRegisterMetaType<PortSettings>("PortSettings");
    startBridge();

#ifdef Q_OS_MAC
    // hack: avoid an empty dummy File menu on OS X
    // there might be a better way to do this, but hide() and clear() don't work.
    ui->menuFile->setTitle(""); // Doesn't do anything on OS X
#endif

}


MainWindow::~MainWindow()
{
    if (bridge) {
        QEventLoop w;
        connect(bridge, SIGNAL(destroyed()), &w, SLOT(quit()));
        bridge->deleteLater();
        w.exec();
    }
    delete ui;
#if defined(__MACOSX_CORE__)
    delete dummyMidiIn;
#endif
}

void MainWindow::showPreferences()
{
    SettingsDialog dialog;
    if(dialog.exec() == QDialog::Accepted) {
        scrollbackSize=Settings::getScrollbackSize();
        onValueChanged();
    }
}

void MainWindow::showAboutBox()
{
    AboutDialog().exec();
}

bool MainWindow::eventFilter(QObject *object, QEvent *event) {
    // If user is triggering a dropdown, refresh it live
    if(event->type() == QEvent::MouseButtonPress || event->type() == QEvent::KeyPress) {
        disableOnValueChanged = true;
        if (object == ui->cmbMidiIn1) {
            RtMidiIn in;
            refreshMidi(ui->cmbMidiIn1, &in);
        } else if (object == ui->cmbMidiIn2) {
            RtMidiIn in;
            refreshMidi(ui->cmbMidiIn2, &in);
        } else if (object == ui->cmbMidiIn3) {
            RtMidiIn in;
            refreshMidi(ui->cmbMidiIn3, &in);
        } else if (object == ui->cmbMidiIn4) {
            RtMidiIn in;
            refreshMidi(ui->cmbMidiIn4, &in);
        } else if (object == ui->cmbMidiIn5) {
            RtMidiIn in;
            refreshMidi(ui->cmbMidiIn5, &in);
        } else if (object == ui->cmbMidiIn6) {
            RtMidiIn in;
            refreshMidi(ui->cmbMidiIn6, &in);
        } else if (object == ui->cmbMidiOut) {
            RtMidiOut out;
            refreshMidi(ui->cmbMidiOut, &out);
        } else if (object == ui->cmbSerial) {
            refreshSerial();
        }
        disableOnValueChanged = false;
    }
    return false;
}

void MainWindow::refresh()
{
    refreshSerial();
    refreshMidiIn();
    refreshMidiOut();
}

void MainWindow::refreshMidiIn()
{
    RtMidiIn in = RtMidiIn();
    refreshMidi(ui->cmbMidiIn1, &in);
    refreshMidi(ui->cmbMidiIn2, &in);
    refreshMidi(ui->cmbMidiIn3, &in);
    refreshMidi(ui->cmbMidiIn4, &in);
    refreshMidi(ui->cmbMidiIn5, &in);
    refreshMidi(ui->cmbMidiIn6, &in);
}

void MainWindow::refreshMidiOut()
{
    RtMidiOut out = RtMidiOut();
    refreshMidi(ui->cmbMidiOut, &out);
}


void MainWindow::refreshMidi(QComboBox *combo, RtMidi *midi)
{
    QString current = combo->currentText();
    combo->clear();
    try
    {
      int ports = midi->getPortCount();
      combo->addItem(TEXT_NOT_CONNECTED);
      for (int i = 0; i < ports; i++ ) {
        QString name = QString::fromUtf8(midi->getPortName(i).c_str());
        combo->addItem(name);
            if(current == name) {
                combo->setCurrentIndex(combo->count() - 1);
           }
        }
    }
    catch (RtMidiError &err) {
        ui->lst_debug->addItem("Failed to scan for MIDI ports:");
        ui->lst_debug->addItem(QString::fromStdString(err.getMessage()));
        ui->lst_debug->scrollToBottom();
    }
}

void MainWindow::refreshSerial()
{
    QString current = ui->cmbSerial->currentText();
    ui->cmbSerial->clear();
    ui->cmbSerial->addItem(TEXT_NOT_CONNECTED);
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    for(QList<QextPortInfo>::iterator it = ports.begin(); it != ports.end(); it++) {
        QString label = it->friendName.isEmpty() ? it->portName : it->friendName;
#ifdef Q_OS_LINUX
        QString portName = it->physName; // Bug workaround, Linux needs the /dev/ in front of port name
#else
        QString portName = it->portName;
#endif
        ui->cmbSerial->addItem(label, QVariant(portName));
        if(current == label) {
            ui->cmbSerial->setCurrentIndex(ui->cmbSerial->count() - 1);
        }
    }
}

void MainWindow::onDebugClicked(bool value)
{
    Settings::setDebug(value);
}

void MainWindow::onMultiportClicked(bool value)
{
    Settings::setMultiport(value);
    onValueChanged();
}

void MainWindow::startBridge()
{
    Settings::setLastMidiIn1(ui->cmbMidiIn1->currentText());
    Settings::setLastMidiIn2(ui->cmbMidiIn2->currentText());
    Settings::setLastMidiIn3(ui->cmbMidiIn3->currentText());
    Settings::setLastMidiIn4(ui->cmbMidiIn4->currentText());
    Settings::setLastMidiIn5(ui->cmbMidiIn5->currentText());
    Settings::setLastMidiIn6(ui->cmbMidiIn6->currentText());
    Settings::setLastMidiOut(ui->cmbMidiOut->currentText());
    Settings::setLastSerialPort(ui->cmbSerial->currentText());
    if(!ui->chk_on->isChecked()
            || ( ui->cmbSerial->currentIndex() == 0
                    && ui->cmbMidiIn1->currentIndex() == 0
                    && ui->cmbMidiIn2->currentIndex() == 0
                    && ui->cmbMidiIn3->currentIndex() == 0
                    && ui->cmbMidiIn4->currentIndex() == 0
                    && ui->cmbMidiIn5->currentIndex() == 0
                    && ui->cmbMidiIn6->currentIndex() == 0
                    && ui->cmbMidiOut->currentIndex() == 0 )) {
        pendingStartBridge = false;
        return;
    }
    refreshDebugList();
    ui->lst_debug->clear();
    int midiIn1 =ui->cmbMidiIn1->currentIndex()-1;
    int midiIn2 =ui->cmbMidiIn2->currentIndex()-1;
    int midiIn3 =ui->cmbMidiIn3->currentIndex()-1;
    int midiIn4 =ui->cmbMidiIn4->currentIndex()-1;
    int midiIn5 =ui->cmbMidiIn5->currentIndex()-1;
    int midiIn6 =ui->cmbMidiIn6->currentIndex()-1;
    int midiOut = ui->cmbMidiOut->currentIndex()-1;
    ui->lst_debug->addItem("Starting MIDI<->Serial Bridge...");
    bridge = new Bridge(ui->chk_multiport->isChecked());
    bridge->moveToThread(workerThread);
    connect(bridge, SIGNAL(debugMessage(QString)), SLOT(onDebugMessage(QString)));
    connect(bridge, SIGNAL(displayMessage(QString)), SLOT(onDisplayMessage(QString)));
    connect(bridge, SIGNAL(midiReceived1()), ui->led_midiin1, SLOT(blinkOn()));
    connect(bridge, SIGNAL(midiReceived2()), ui->led_midiin2, SLOT(blinkOn()));
    connect(bridge, SIGNAL(midiReceived3()), ui->led_midiin3, SLOT(blinkOn()));
    connect(bridge, SIGNAL(midiReceived4()), ui->led_midiin4, SLOT(blinkOn()));
    connect(bridge, SIGNAL(midiReceived5()), ui->led_midiin5, SLOT(blinkOn()));
    connect(bridge, SIGNAL(midiReceived6()), ui->led_midiin6, SLOT(blinkOn()));
    connect(bridge, SIGNAL(midiSent()), ui->led_midiout, SLOT(blinkOn()));
    connect(bridge, SIGNAL(serialTraffic()), ui->led_serial, SLOT(blinkOn()));
    QMetaObject::invokeMethod(bridge, "attach", Qt::QueuedConnection,
        Q_ARG(QString, ui->cmbSerial->itemData(ui->cmbSerial->currentIndex()).toString()),
        Q_ARG(PortSettings, Settings::getPortSettings()),
        Q_ARG(int, midiIn1),
        Q_ARG(int, midiIn2),
        Q_ARG(int, midiIn3),
        Q_ARG(int, midiIn4),
        Q_ARG(int, midiIn5),
        Q_ARG(int, midiIn6),
        Q_ARG(int, midiOut) );
    pendingStartBridge = false;
}

void MainWindow::onValueChanged()
{
    if (disableOnValueChanged) return;
    while (pendingStartBridge) {
        QCoreApplication::processEvents();
    }
    if(bridge) {
        pendingStartBridge = true;
        Bridge *old_bridge = bridge;
        bridge = NULL;
        connect(old_bridge, SIGNAL(destroyed()), this, SLOT(startBridge()));
        old_bridge->deleteLater();
    } else {
        startBridge();
    }
}

void MainWindow::onDisplayMessage(QString message)
{
    if(debugListMessages.size() == scrollbackSize)
        debugListMessages.removeFirst();
    debugListMessages.append(message);
    if(!debugListTimer.isActive())
        debugListTimer.start();
}

void MainWindow::onDebugMessage(QString message)
{
    if(ui->chk_debug->isChecked())
        onDisplayMessage(message);
}

/*
 * When the timer (started in onDisplayMessage) fires, we update lst_debug with the
 * contents of debugListMessages.
 *
 * This happens in the timer event in order to rate limit it to a number of redraws per second
 * (redrawing, and especially scrolling the list view, can be quite resource intensive.)
 */
void MainWindow::refreshDebugList()
{
    QListWidget *lst = ui->lst_debug;
    while(lst->count() + debugListMessages.size() - scrollbackSize > 0 && lst->count() > 0) {
      delete lst->item(0);
    }
    lst->addItems(debugListMessages);
    debugListMessages.clear();
    lst->scrollToBottom();
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    const int margin = 5;
    QRect geo = ui->lst_debug->geometry();
    geo.setBottom(ui->centralWidget->height() - margin);
    ui->lst_debug->setGeometry(geo);
}

void MainWindow::stretchWindow(bool multiport)
{
    QSize stretchSize2 = QSize(stretchSize.x(), stretchSize.y());
    if (multiport) {
        ui->chk_debug->move(ui->chk_debug->pos() + stretchSize);
        ui->lst_debug->move(ui->lst_debug->pos() + stretchSize);
        this->resize(this->size() + stretchSize2);
        this->setMinimumSize(this->minimumSize() + stretchSize2);
    } else {
        ui->chk_debug->move(ui->chk_debug->pos() - stretchSize);
        ui->lst_debug->move(ui->lst_debug->pos() - stretchSize);
        this->setMinimumSize(this->minimumSize() - stretchSize2);
        this->resize(this->size() - stretchSize2);
    }
}
