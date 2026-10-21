#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QTimer>
#include <QEventLoop>

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#endif

// Jetson TX2 GPIO pin numbers (refer to Lab2 demo2.c)
static const int gpio_pins[4] = {466, 392, 397, 255};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    actionState = Action::OFF;
    rep = 0;
    countdown = 0;
    runningState = 0;
    direction = false;
    for (int i=0; i<4; i++) {lights[i] = false;}

    setupGpio();
    UpdateLights();
}

MainWindow::~MainWindow()
{
    cleanupGpio();
    delete ui;
}

void MainWindow::setupGpio() {
#ifdef __linux__
    char path[128];
    for (int i = 0; i < 4; ++i) {
        // 1. Export GPIO
        int fp = open("/sys/class/gpio/export", O_WRONLY);
        if (fp >= 0) {
            snprintf(path, sizeof(path), "%d", gpio_pins[i]);
            write(fp, path, strlen(path));
            close(fp);
        } else {
            perror("Failed to open GPIO export");
        }

        // 等待 GPIO 節點建立
        usleep(50000);

        // 2. 設定成輸出
        snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", gpio_pins[i]);
        fp = open(path, O_WRONLY);
        if (fp >= 0) {
            write(fp, "out", 3);
            close(fp);
        } else {
            perror("Failed to set GPIO direction");
        }
    }
#endif
}

void MainWindow::setGpio(int pin, int value) {
    if (pin < 0 || pin >= 4) return;
#ifdef __linux__
    char path[128];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", gpio_pins[pin]);
    int fp = open(path, O_WRONLY);
    if (fp >= 0) {
        if (value == 0) {
            write(fp, "0", 1);
        } else {
            write(fp, "1", 1);
        }
        close(fp);
    }
#else
    Q_UNUSED(value);
#endif
}

void MainWindow::cleanupGpio() {
#ifdef __linux__
    char path[128];
    for (int i = 0; i < 4; ++i) {
        setGpio(i, 0);
        int fp = open("/sys/class/gpio/unexport", O_WRONLY);
        if (fp >= 0) {
            snprintf(path, sizeof(path), "%d", gpio_pins[i]);
            write(fp, path, strlen(path));
            close(fp);
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
    ui->ledLabel0->setHidden(!lights[0]);
    ui->ledLabel1->setHidden(!lights[1]);
    ui->ledLabel2->setHidden(!lights[2]);
    ui->ledLabel3->setHidden(!lights[3]);

    for (int i = 0; i < 4; ++i) {
        setGpio(i, lights[i] ? 1 : 0);
    }
}

int MainWindow::GetDelay() {
    return 50000/ui->speedSlider->value();
}

void MainWindow::on_ledShiningBtn_clicked()
{
    if (actionState != Action::OFF) {return;}
    lights[0] = ui->ledCheck0->isChecked();
    lights[1] = ui->ledCheck1->isChecked();
    lights[2] = ui->ledCheck2->isChecked();
    lights[3] = ui->ledCheck3->isChecked();
    UpdateLights();
}


void MainWindow::on_switchingOnBtn_clicked()
{
    actionState = Action::SWITCHING;
    rep = ui->switchingRep->value();

    QEventLoop loop;

    while (rep--) {
        if (actionState != Action::SWITCHING) {break;}
        lights[0] = true;
        lights[1] = true;
        lights[2] = false;
        lights[3] = false;
        UpdateLights();
        SyncCheckboxes();

        QTimer::singleShot(GetDelay(), &loop, &QEventLoop::quit);
        loop.exec();

        if (actionState != Action::SWITCHING) {break;}
        lights[0] = false;
        lights[1] = false;
        lights[2] = true;
        lights[3] = true;
        UpdateLights();
        SyncCheckboxes();

        QTimer::singleShot(GetDelay(), &loop, &QEventLoop::quit);
        loop.exec();
    }
}


void MainWindow::on_switchingOffBtn_clicked()
{
    if (actionState != Action::SWITCHING) {return;}
    actionState = Action::OFF;
    for (int i=0; i<4; i++) {lights[i] = false;}
    UpdateLights();
    SyncCheckboxes();
}


void MainWindow::on_runningOnBtn_clicked()
{
    actionState = Action::RUNNING;
    QEventLoop loop;

    while (true) {
        if (actionState != Action::RUNNING) {break;}
        for (int i=0; i<4; i++) {lights[i] = (i == runningState);}
        UpdateLights();
        SyncCheckboxes();

        QTimer::singleShot(GetDelay(), &loop, &QEventLoop::quit);
        loop.exec();

        if (direction) {runningState--;}
        else {runningState++;}
        if (runningState < 0) {runningState += 4;}
        runningState = runningState % 4;
    }
}


void MainWindow::on_runningOffBtn_clicked()
{
    if (actionState != Action::RUNNING) {return;}
    actionState = Action::OFF;
    for (int i=0; i<4; i++) {lights[i] = false;}
    UpdateLights();
    SyncCheckboxes();
}


void MainWindow::on_changeDirBtn_clicked()
{
    direction = !direction;
}

