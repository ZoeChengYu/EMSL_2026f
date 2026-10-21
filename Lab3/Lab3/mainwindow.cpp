#include "mainwindow.h"
#include "./ui_mainwindow.h"

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#endif

// Jetson TX2 GPIO pin numbers (refer to Lab2 demo2.c: int gpio[6] = {466, 392, 397, 255, 296, 481};)
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

// 參考 Lab2 demo2.c 中的 setup_gpio()
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

        // 等待 GPIO 節點建立 (demo2.c: usleep)
        usleep(50000);

        // 2. 設定成輸出方向
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

// 參考 Lab2 demo2.c 中的 gpio_control(int pin, int value)
void MainWindow::setGpio(int pinIdx, int val) {
    if (pinIdx < 0 || pinIdx >= 4) return;
#ifdef __linux__
    char path[128];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", gpio_pins[pinIdx]);
    int fp = open(path, O_WRONLY);
    if (fp >= 0) {
        if (val == 0) {
            write(fp, "0", 1);
        } else {
            write(fp, "1", 1);
        }
        close(fp);
    }
#else
    Q_UNUSED(val);
#endif
}

// 程式結束或清理時將各腳位熄滅並釋放 (unexport)
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
    static const QPixmap litPixmap(":/new/prefix1/Lit_Redstone_Lamp_JE2_BE1.png");

    QLabel* labels[4] = {ui->ledLabel0, ui->ledLabel1, ui->ledLabel2, ui->ledLabel3};
    for (int i = 0; i < 4; ++i) {
        labels[i]->setPixmap(litPixmap);
        labels[i]->setVisible(lights[i]);
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

// 支援鍵盤快捷鍵調整 Switching Speed (例如: +, -, Up, Down, W, Q 等按鍵)
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal || event->key() == Qt::Key_Up) {
        ui->speedSlider->setValue(qMin(100, ui->speedSlider->value() + 5));
    } else if (event->key() == Qt::Key_Minus || event->key() == Qt::Key_Underscore || event->key() == Qt::Key_Down) {
        ui->speedSlider->setValue(qMax(1, ui->speedSlider->value() - 5));
    } else if (event->key() == Qt::Key_W) {
        ui->speedSlider->setValue(qMin(100, ui->speedSlider->value() + 5));
    } else if (event->key() == Qt::Key_Q) {
        ui->speedSlider->setValue(qMax(1, ui->speedSlider->value() - 5));
    } else {
        QMainWindow::keyPressEvent(event);
    }
}
