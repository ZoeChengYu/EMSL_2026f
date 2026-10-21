#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>

#ifdef __linux__
#include <unistd.h>
#endif

static const int gpio_pins[4] = {466, 392, 397, 255};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , timer(new QTimer(this))
{
    ui->setupUi(this);

    actionState = Action::OFF;
    rep = 0;
    runningState = 0;
    direction = false;
    switchingPhase = 0;
    for (int i = 0; i < 4; i++) {
        lights[i] = false;
    }

    connect(timer, &QTimer::timeout, this, &MainWindow::onTimerTimeout);

    setupGpio();
    UpdateLights();
    SyncCheckboxes();
}

MainWindow::~MainWindow()
{
    if (timer->isActive()) {
        timer->stop();
    }
    cleanupGpio();
    delete ui;
}

void MainWindow::setupGpio() {
#ifdef __linux__
    for (int i = 0; i < 4; ++i) {
        QFile expFile("/sys/class/gpio/export");
        if (expFile.open(QIODevice::WriteOnly)) {
            QByteArray pinStr = QByteArray::number(gpio_pins[i]);
            expFile.write(pinStr);
            expFile.close();
        }
        usleep(50000);

        QString dirPath = QString("/sys/class/gpio/gpio%1/direction").arg(gpio_pins[i]);
        QFile dirFile(dirPath);
        if (dirFile.open(QIODevice::WriteOnly)) {
            dirFile.write("out");
            dirFile.close();
        }
    }
#endif
}

void MainWindow::setGpio(int pinIdx, int val) {
    if (pinIdx < 0 || pinIdx >= 4) return;
#ifdef __linux__
    QString valPath = QString("/sys/class/gpio/gpio%1/value").arg(gpio_pins[pinIdx]);
    QFile valFile(valPath);
    if (valFile.open(QIODevice::WriteOnly)) {
        valFile.write(val ? "1" : "0");
        valFile.close();
    }
#endif
}

void MainWindow::cleanupGpio() {
#ifdef __linux__
    for (int i = 0; i < 4; ++i) {
        setGpio(i, 0);
        QFile unexpFile("/sys/class/gpio/unexport");
        if (unexpFile.open(QIODevice::WriteOnly)) {
            unexpFile.write(QByteArray::number(gpio_pins[i]));
            unexpFile.close();
        }
    }
#endif
}

void MainWindow::SyncCheckboxes() {
    ui->ledCheck0->setChecked(lights[0]);
    ui->ledCheck1->setChecked(lights[1]);
    ui->ledCheck2->setChecked(lights[2]);
    ui->ledCheck3->setChecked(lights[3]);
}

void MainWindow::UpdateLights() {
    static const QPixmap litPixmap(":/new/prefix1/Lit_Redstone_Lamp_JE2_BE1.png");
    static const QPixmap unlitPixmap(":/new/prefix1/Redstone_Lamp_JE3_BE2.png");

    QLabel* labels[4] = {ui->ledLabel0, ui->ledLabel1, ui->ledLabel2, ui->ledLabel3};
    for (int i = 0; i < 4; ++i) {
        labels[i]->setPixmap(lights[i] ? litPixmap : unlitPixmap);
        labels[i]->setHidden(false);
        setGpio(i, lights[i] ? 1 : 0);
    }
}

int MainWindow::GetDelay() {
    int val = ui->speedSlider->value();
    if (val <= 0) val = 1;
    return 50000 / val;
}

void MainWindow::onTimerTimeout() {
    if (actionState == Action::SWITCHING) {
        if (switchingPhase == 0) {
            lights[0] = true;
            lights[1] = true;
            lights[2] = false;
            lights[3] = false;
            UpdateLights();
            SyncCheckboxes();
            switchingPhase = 1;
        } else {
            lights[0] = false;
            lights[1] = false;
            lights[2] = true;
            lights[3] = true;
            UpdateLights();
            SyncCheckboxes();
            switchingPhase = 0;
            rep--;

            if (rep <= 0) {
                timer->stop();
                actionState = Action::OFF;
                for (int i = 0; i < 4; ++i) {
                    lights[i] = false;
                }
                UpdateLights();
                SyncCheckboxes();
            }
        }
    } else if (actionState == Action::RUNNING) {
        if (direction) {
            runningState = (runningState - 1 + 4) % 4;
        } else {
            runningState = (runningState + 1) % 4;
        }
        for (int i = 0; i < 4; ++i) {
            lights[i] = (i == runningState);
        }
        UpdateLights();
        SyncCheckboxes();
    } else {
        timer->stop();
    }
}

void MainWindow::on_ledShiningBtn_clicked()
{
    if (actionState != Action::OFF) { return; }
    lights[0] = ui->ledCheck0->isChecked();
    lights[1] = ui->ledCheck1->isChecked();
    lights[2] = ui->ledCheck2->isChecked();
    lights[3] = ui->ledCheck3->isChecked();
    UpdateLights();
}

void MainWindow::on_switchingOnBtn_clicked()
{
    timer->stop();
    actionState = Action::SWITCHING;
    rep = ui->switchingRep->value();
    if (rep <= 0) {
        rep = 1;
        ui->switchingRep->setValue(1);
    }

    lights[0] = true;
    lights[1] = true;
    lights[2] = false;
    lights[3] = false;
    UpdateLights();
    SyncCheckboxes();
    switchingPhase = 1;

    timer->start(GetDelay());
}

void MainWindow::on_switchingOffBtn_clicked()
{
    if (actionState != Action::SWITCHING) { return; }
    timer->stop();
    actionState = Action::OFF;
    for (int i = 0; i < 4; i++) {
        lights[i] = false;
    }
    UpdateLights();
    SyncCheckboxes();
}

void MainWindow::on_runningOnBtn_clicked()
{
    timer->stop();
    actionState = Action::RUNNING;

    for (int i = 0; i < 4; i++) {
        lights[i] = (i == runningState);
    }
    UpdateLights();
    SyncCheckboxes();

    timer->start(GetDelay());
}

void MainWindow::on_runningOffBtn_clicked()
{
    if (actionState != Action::RUNNING) { return; }
    timer->stop();
    actionState = Action::OFF;
    for (int i = 0; i < 4; i++) {
        lights[i] = false;
    }
    UpdateLights();
    SyncCheckboxes();
}

void MainWindow::on_changeDirBtn_clicked()
{
    direction = !direction;
}

void MainWindow::on_speedSlider_valueChanged(int value)
{
    Q_UNUSED(value);
    if (timer->isActive()) {
        timer->setInterval(GetDelay());
    }
}

void MainWindow::on_speedSpinBox_valueChanged(int value)
{
    Q_UNUSED(value);
    if (timer->isActive()) {
        timer->setInterval(GetDelay());
    }
}
