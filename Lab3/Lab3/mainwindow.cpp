#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QApplication>
#include <QShortcut>
#include <QLineEdit>

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

    // 全域安裝事件過濾器，確保鍵盤按鍵 1~4、S、O、F、R、X、D 在任何焦點下都能即時觸發
    qApp->installEventFilter(this);

    // 同步配置全域 QShortcut (ApplicationShortcut 級別) 作為雙重保障
    new QShortcut(QKeySequence(Qt::Key_1), this, [this]() { ui->ledCheck0->toggle(); }, Qt::ApplicationShortcut);
    new QShortcut(QKeySequence(Qt::Key_2), this, [this]() { ui->ledCheck1->toggle(); }, Qt::ApplicationShortcut);
    new QShortcut(QKeySequence(Qt::Key_3), this, [this]() { ui->ledCheck2->toggle(); }, Qt::ApplicationShortcut);
    new QShortcut(QKeySequence(Qt::Key_4), this, [this]() { ui->ledCheck3->toggle(); }, Qt::ApplicationShortcut);
    new QShortcut(QKeySequence(Qt::Key_S), this, [this]() { ui->ledShiningBtn->animateClick(); }, Qt::ApplicationShortcut);
    new QShortcut(QKeySequence(Qt::Key_O), this, [this]() { ui->switchingOnBtn->animateClick(); }, Qt::ApplicationShortcut);
    new QShortcut(QKeySequence(Qt::Key_F), this, [this]() { ui->switchingOffBtn->animateClick(); }, Qt::ApplicationShortcut);
    new QShortcut(QKeySequence(Qt::Key_R), this, [this]() { ui->runningOnBtn->animateClick(); }, Qt::ApplicationShortcut);
    new QShortcut(QKeySequence(Qt::Key_X), this, [this]() { ui->runningOffBtn->animateClick(); }, Qt::ApplicationShortcut);
    new QShortcut(QKeySequence(Qt::Key_D), this, [this]() { ui->changeDirBtn->animateClick(); }, Qt::ApplicationShortcut);

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
    if (timer->isActive()) {
        timer->stop();
    }
    actionState = Action::OFF;
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
        rep = 5;
        ui->switchingRep->setValue(5);
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
    if (timer->isActive()) {
        timer->stop();
    }
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
    if (timer->isActive()) {
        timer->stop();
    }
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

// 全域事件過濾器：確保 1~4、S、O、F、R、X、D 在任何焦點下都能 100% 觸發
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (!keyEvent->isAutoRepeat()) {
            bool isLineEdit = obj && obj->inherits("QLineEdit");
            int key = keyEvent->key();

            // 1. 可任意選取 LED1 至 LED4 (按鍵 1、2、3、4 切換核取狀態)
            if (key == Qt::Key_1 && !isLineEdit) {
                ui->ledCheck0->toggle();
                return true;
            } else if (key == Qt::Key_2 && !isLineEdit) {
                ui->ledCheck1->toggle();
                return true;
            } else if (key == Qt::Key_3 && !isLineEdit) {
                ui->ledCheck2->toggle();
                return true;
            } else if (key == Qt::Key_4 && !isLineEdit) {
                ui->ledCheck3->toggle();
                return true;
            }
            // 2. LED Shining (燈亮) (按鍵 S)
            else if (key == Qt::Key_S && !isLineEdit) {
                ui->ledShiningBtn->animateClick();
                return true;
            }
            // 4. Switching on (LED閃爍) (按鍵 O)
            else if (key == Qt::Key_O && !isLineEdit) {
                ui->switchingOnBtn->animateClick();
                return true;
            }
            // 5. Switching off (LED熄滅) (按鍵 F)
            else if (key == Qt::Key_F && !isLineEdit) {
                ui->switchingOffBtn->animateClick();
                return true;
            }
            // 6. Running Light on (LED流水燈) (按鍵 R)
            else if (key == Qt::Key_R && !isLineEdit) {
                ui->runningOnBtn->animateClick();
                return true;
            }
            // 7. Running Light off (LED熄滅) (按鍵 X)
            else if (key == Qt::Key_X && !isLineEdit) {
                ui->runningOffBtn->animateClick();
                return true;
            }
            // 8. Change Direction (LED方向) (按鍵 D)
            else if (key == Qt::Key_D && !isLineEdit) {
                ui->changeDirBtn->animateClick();
                return true;
            }
            // Switching Speed (控制閃爍速度) (按鍵 +, -, Up, Down, W, Q)
            else if (key == Qt::Key_Plus || key == Qt::Key_Equal || key == Qt::Key_Up) {
                ui->speedSlider->setValue(qMin(100, ui->speedSlider->value() + 5));
                return true;
            } else if (key == Qt::Key_Minus || key == Qt::Key_Underscore || key == Qt::Key_Down) {
                ui->speedSlider->setValue(qMax(1, ui->speedSlider->value() - 5));
                return true;
            } else if (key == Qt::Key_W && !isLineEdit) {
                ui->speedSlider->setValue(qMin(100, ui->speedSlider->value() + 5));
                return true;
            } else if (key == Qt::Key_Q && !isLineEdit) {
                ui->speedSlider->setValue(qMax(1, ui->speedSlider->value() - 5));
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_1:
        ui->ledCheck0->toggle();
        event->accept();
        return;
    case Qt::Key_2:
        ui->ledCheck1->toggle();
        event->accept();
        return;
    case Qt::Key_3:
        ui->ledCheck2->toggle();
        event->accept();
        return;
    case Qt::Key_4:
        ui->ledCheck3->toggle();
        event->accept();
        return;
    case Qt::Key_S:
        ui->ledShiningBtn->animateClick();
        event->accept();
        return;
    case Qt::Key_O:
        ui->switchingOnBtn->animateClick();
        event->accept();
        return;
    case Qt::Key_F:
        ui->switchingOffBtn->animateClick();
        event->accept();
        return;
    case Qt::Key_R:
        ui->runningOnBtn->animateClick();
        event->accept();
        return;
    case Qt::Key_X:
        ui->runningOffBtn->animateClick();
        event->accept();
        return;
    case Qt::Key_D:
        ui->changeDirBtn->animateClick();
        event->accept();
        return;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
    case Qt::Key_Up:
    case Qt::Key_W:
        ui->speedSlider->setValue(qMin(100, ui->speedSlider->value() + 5));
        event->accept();
        return;
    case Qt::Key_Minus:
    case Qt::Key_Underscore:
    case Qt::Key_Down:
    case Qt::Key_Q:
        ui->speedSlider->setValue(qMax(1, ui->speedSlider->value() - 5));
        event->accept();
        return;
    default:
        QMainWindow::keyPressEvent(event);
        break;
    }
}
