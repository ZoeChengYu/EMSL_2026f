#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QApplication>
#include <QShortcut>
#include <QLineEdit>
#include <cstdlib>
#include <ctime>

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#endif

// Jetson TX2 GPIO pin numbers (參考 Lab2 demo2.c: int gpio[6] = {466, 392, 397, 255, 296, 481};)
static const int gpio_pins[4] = {466, 392, 397, 255};

// 圓形立體球體風格按鈕樣式 (依據 Lab3_5_QT.pdf 示意圖)
static const char* kBtnNormalStyle =
    "QPushButton { background: qradialgradient(cx:0.4, cy:0.4, radius:0.6, fx:0.3, fy:0.3, "
    "stop:0 #93c5fd, stop:0.75 #3b82f6, stop:1 #1d4ed8); color: white; border: 3px solid #60a5fa; "
    "border-radius: 35px; font-family: serif; } "
    "QPushButton:hover { background: qradialgradient(cx:0.4, cy:0.4, radius:0.6, fx:0.3, fy:0.3, "
    "stop:0 #bfdbfe, stop:0.75 #60a5fa, stop:1 #2563eb); }";

static const char* kBtnGlowStyle =
    "QPushButton { background: qradialgradient(cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, "
    "stop:0 #ffffff, stop:0.25 #fde047, stop:0.65 #f97316, stop:1 #ea580c); "
    "color: #7c2d12; border: 4px solid #facc15; border-radius: 35px; font-family: serif; }";

static const char* kBtnErrorStyle =
    "QPushButton { background: qradialgradient(cx:0.4, cy:0.4, radius:0.6, fx:0.3, fy:0.3, "
    "stop:0 #fca5a5, stop:0.75 #ef4444, stop:1 #991b1b); "
    "color: white; border: 3px solid #dc2626; border-radius: 35px; font-family: serif; }";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , timer(new QTimer(this))
    , qteIsRunning(false)
    , qteScore(0)
    , qteTarget(0)
    , qteBlinkPhase(false)
    , qteRemainingTimeMs(3000)
    , qteBlinkTimer(new QTimer(this))
    , qteReactionTimer(new QTimer(this))
{
    ui->setupUi(this);

    // 跨版本通用連線 (Slider <-> SpinBox <-> ProgressBar 連動，相容 Qt 5 / Qt 6)
    connect(ui->speedSlider, SIGNAL(valueChanged(int)), ui->speedProgress, SLOT(setValue(int)));
    connect(ui->speedSlider, SIGNAL(valueChanged(int)), ui->speedSpinBox, SLOT(setValue(int)));
    connect(ui->speedSpinBox, SIGNAL(valueChanged(int)), ui->speedSlider, SLOT(setValue(int)));
    connect(ui->speedSpinBox, SIGNAL(valueChanged(int)), ui->speedProgress, SLOT(setValue(int)));

    // 亂數種子初始化 (相容 Qt 5.5 / Qt 6)
    srand(static_cast<unsigned int>(time(nullptr)));

    actionState = Action::OFF;
    rep = 0;
    runningState = 0;
    direction = false;
    switchingPhase = 0;
    for (int i = 0; i < 4; i++) {
        lights[i] = false;
    }

    // 基礎功能定時器連接
    connect(timer, &QTimer::timeout, this, &MainWindow::onTimerTimeout);

    // QTE 定時器連接
    connect(qteBlinkTimer, &QTimer::timeout, this, &MainWindow::onQteBlinkTimeout);
    connect(qteReactionTimer, &QTimer::timeout, this, &MainWindow::onQteReactionTick);

    // 全域安裝事件過濾器，確保鍵盤按鍵在任何焦點下都能即時觸發
    qApp->installEventFilter(this);

    setupGpio();
    UpdateLights();
    SyncCheckboxes();
    initQteUi();
}

MainWindow::~MainWindow()
{
    if (timer->isActive()) {
        timer->stop();
    }
    if (qteBlinkTimer->isActive()) {
        qteBlinkTimer->stop();
    }
    if (qteReactionTimer->isActive()) {
        qteReactionTimer->stop();
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
            ssize_t ret = write(fp, path, strlen(path));
            Q_UNUSED(ret);
            ::close(fp);
        } else {
            perror("Failed to open GPIO export");
        }

        // 等待 GPIO 節點建立 (demo2.c: usleep)
        usleep(50000);

        // 2. 設定成輸出方向
        snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", gpio_pins[i]);
        fp = open(path, O_WRONLY);
        if (fp >= 0) {
            ssize_t ret = write(fp, "out", 3);
            Q_UNUSED(ret);
            ::close(fp);
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
        ssize_t ret = 0;
        if (val == 0) {
            ret = write(fp, "0", 1);
        } else {
            ret = write(fp, "1", 1);
        }
        Q_UNUSED(ret);
        ::close(fp);
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
            ssize_t ret = write(fp, path, strlen(path));
            Q_UNUSED(ret);
            ::close(fp);
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
            if (rep <= 0) {
                timer->stop();
                actionState = Action::OFF;
                for (int i = 0; i < 4; ++i) {
                    lights[i] = false;
                }
                UpdateLights();
                SyncCheckboxes();
                return;
            }

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
    if (qteIsRunning) {
        qteGameOver(false);
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
    if (qteIsRunning) {
        qteGameOver(false);
    }
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
    if (qteIsRunning) {
        qteGameOver(false);
    }
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

// =========================================================================
// 進階題：LED QTE (Quick Time Event) 模組實作 (參考 Lab3_5_QT.pdf)
// =========================================================================

void MainWindow::initQteUi() {
    static const QPixmap unlitPixmap(":/new/prefix1/Redstone_Lamp_JE3_BE2.png");
    QLabel* qteLeds[4] = {ui->qteLed0, ui->qteLed1, ui->qteLed2, ui->qteLed3};
    for (int i = 0; i < 4; ++i) {
        qteLeds[i]->setPixmap(unlitPixmap);
    }
    resetButtonStyles();
    ui->qteScoreBox->setText("0000");
    ui->qteBonusLabel->setText("");
    ui->qteTimeBar->setRange(0, 3000);
    ui->qteTimeBar->setValue(3000);
    ui->qteStatusLabel->setText("請按下『開始』啟動 LED QTE 挑戰！");
}

void MainWindow::resetButtonStyles() {
    QPushButton* btns[4] = {ui->qteBtn0, ui->qteBtn1, ui->qteBtn2, ui->qteBtn3};
    for (int i = 0; i < 4; ++i) {
        btns[i]->setStyleSheet(kBtnNormalStyle);
    }
}

void MainWindow::setQteLedVisual(int index, bool on) {
    if (index < 0 || index >= 4) return;
    static const QPixmap litPixmap(":/new/prefix1/Lit_Redstone_Lamp_JE2_BE1.png");
    static const QPixmap unlitPixmap(":/new/prefix1/Redstone_Lamp_JE3_BE2.png");
    QLabel* qteLeds[4] = {ui->qteLed0, ui->qteLed1, ui->qteLed2, ui->qteLed3};
    qteLeds[index]->setPixmap(on ? litPixmap : unlitPixmap);
}

void MainWindow::updateQteScoreDisplay(int chosenBtnIndex) {
    // 依據示意圖格式化顯示 4 位數：0000, 0020, 0040, 0060, 0080, 0100
    ui->qteScoreBox->setText(QString("%1").arg(qteScore, 4, 10, QChar('0')));

    if (chosenBtnIndex >= 0 && chosenBtnIndex < 4) {
        // 依示意圖：+20 浮現在被點擊且發光的按鈕正上方
        QPushButton* btns[4] = {ui->qteBtn0, ui->qteBtn1, ui->qteBtn2, ui->qteBtn3};
        ui->qteBonusLabel->setGeometry(btns[chosenBtnIndex]->x(),
                                       btns[chosenBtnIndex]->y() - 38,
                                       btns[chosenBtnIndex]->width(),
                                       35);
        ui->qteBonusLabel->setText("+20");
    } else {
        ui->qteBonusLabel->setText("");
    }
}

void MainWindow::startQteRound() {
    resetButtonStyles();

    // 隨機選取 0~3 其中一顆 LED 閃爍
    int nextTarget = rand() % 4;
    if (nextTarget == qteTarget && (rand() % 2 == 0)) {
        nextTarget = (nextTarget + 1 + (rand() % 3)) % 4;
    }
    qteTarget = nextTarget;
    qteBlinkPhase = true;

    // 每題時限 3 秒 (3000ms)
    qteRemainingTimeMs = 3000;
    ui->qteTimeBar->setRange(0, 3000);
    ui->qteTimeBar->setValue(3000);

    // 熄滅所有其他 LED (畫面與 GPIO)
    for (int i = 0; i < 4; ++i) {
        if (i != qteTarget) {
            setGpio(i, 0);
            setQteLedVisual(i, false);
        }
    }

    // 點亮當前目標 LED
    setGpio(qteTarget, 1);
    setQteLedVisual(qteTarget, true);

    int roundNum = (qteScore / 20) + 1;
    ui->qteStatusLabel->setText(QString("【第 %1 / 5 關】注意！LED %2 正在閃爍，請立即選擇按鈕 %2 或快捷鍵 %2！")
                                    .arg(roundNum).arg(qteTarget + 1));

    qteBlinkTimer->start(200);      // 200ms 快速閃爍
    qteReactionTimer->start(50);   // 每 50ms 更新反應倒數進度條
}

void MainWindow::onQteBlinkTimeout() {
    if (!qteIsRunning) return;
    qteBlinkPhase = !qteBlinkPhase;
    setGpio(qteTarget, qteBlinkPhase ? 1 : 0);
    setQteLedVisual(qteTarget, qteBlinkPhase);
}

void MainWindow::onQteReactionTick() {
    if (!qteIsRunning) return;
    qteRemainingTimeMs -= 50;
    if (qteRemainingTimeMs <= 0) {
        ui->qteTimeBar->setValue(0);
        qteGameOver(true); // 逾時判定失敗
    } else {
        ui->qteTimeBar->setValue(qteRemainingTimeMs);
    }
}

void MainWindow::handleQteChoice(int choiceIndex) {
    if (!qteIsRunning) {
        return;
    }

    QPushButton* btns[4] = {ui->qteBtn0, ui->qteBtn1, ui->qteBtn2, ui->qteBtn3};

    if (choiceIndex == qteTarget) {
        // QTE 成功：對應按鈕呈現示意圖中的發光太陽芒星橘光效果
        btns[choiceIndex]->setStyleSheet(kBtnGlowStyle);
        qteScore += 20;
        updateQteScoreDisplay(choiceIndex);

        // 停止本回合閃爍計時，稍作短暫展示後推進
        qteBlinkTimer->stop();
        qteReactionTimer->stop();

        if (qteScore >= 100) {
            // 達到 100 分即通過
            QTimer::singleShot(350, this, [this]() {
                qteVictory();
            });
        } else {
            // 進入下一回合
            QTimer::singleShot(350, this, [this]() {
                if (qteIsRunning) {
                    startQteRound();
                }
            });
        }
    } else {
        // QTE 失敗：按錯按鈕以紅色警示
        btns[choiceIndex]->setStyleSheet(kBtnErrorStyle);
        qteGameOver(false);
    }
}

void MainWindow::qteGameOver(bool isTimeout) {
    qteIsRunning = false;
    qteBlinkTimer->stop();
    qteReactionTimer->stop();

    // 關閉實體與畫面的所有 LED
    for (int i = 0; i < 4; ++i) {
        setGpio(i, 0);
        setQteLedVisual(i, false);
    }

    // QTE 失敗後，分數歸零，程式終止，再進行一次
    qteScore = 0;
    ui->qteScoreBox->setText("0000");
    ui->qteBonusLabel->setText("");
    ui->qteTimeBar->setValue(0);

    if (isTimeout) {
        ui->qteStatusLabel->setText("❌ 逾時未作答！QTE 失敗，分數歸零，程式終止。請按『再玩一次』重新挑戰！");
    } else {
        ui->qteStatusLabel->setText("❌ 選擇錯誤！QTE 失敗，分數歸零，程式終止。請按『再玩一次』重新挑戰！");
    }
}

void MainWindow::qteVictory() {
    qteIsRunning = false;
    qteBlinkTimer->stop();
    qteReactionTimer->stop();
    resetButtonStyles();

    // 通過時所有燈全亮慶祝並停止
    for (int i = 0; i < 4; ++i) {
        setGpio(i, 1);
        setQteLedVisual(i, true);
    }

    ui->qteScoreBox->setText("0100");
    ui->qteBonusLabel->setGeometry(ui->qteScoreBox->x(), ui->qteScoreBox->y() - 35, ui->qteScoreBox->width(), 30);
    ui->qteBonusLabel->setText("PASS!");
    ui->qteTimeBar->setValue(ui->qteTimeBar->maximum());
    ui->qteStatusLabel->setText("🎉 恭喜通過！獲得 100 分滿分！(100分即通過，可點擊『再玩一次』繼續挑戰)");
}

void MainWindow::on_qteStartBtn_clicked() {
    // 停止基礎功能定時器與流水燈
    if (timer->isActive()) timer->stop();
    actionState = Action::OFF;
    for (int i = 0; i < 4; ++i) {
        lights[i] = false;
        setGpio(i, 0);
    }
    UpdateLights();
    SyncCheckboxes();

    // 重設 QTE 分數與狀態
    qteScore = 0;
    updateQteScoreDisplay(-1);
    qteIsRunning = true;
    startQteRound();
}

void MainWindow::on_qteRestartBtn_clicked() {
    on_qteStartBtn_clicked();
}

void MainWindow::on_qteBtn0_clicked() { handleQteChoice(0); }
void MainWindow::on_qteBtn1_clicked() { handleQteChoice(1); }
void MainWindow::on_qteBtn2_clicked() { handleQteChoice(2); }
void MainWindow::on_qteBtn3_clicked() { handleQteChoice(3); }

void MainWindow::on_tabWidget_currentChanged(int index) {
    if (index == 0) {
        // 切換到基礎功能頁面：停止 QTE
        if (qteIsRunning) {
            qteGameOver(false);
            ui->qteStatusLabel->setText("已切換回基礎功能，QTE 挑戰暫停。");
        }
    } else if (index == 1) {
        // 切換到 QTE 頁面：停止基礎功能
        if (timer->isActive()) timer->stop();
        actionState = Action::OFF;
        for (int i = 0; i < 4; ++i) {
            lights[i] = false;
            setGpio(i, 0);
        }
        UpdateLights();
        SyncCheckboxes();
    }
}

// =========================================================================
// 鍵盤事件過濾器與快捷鍵處理
// =========================================================================

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (!keyEvent->isAutoRepeat()) {
            bool isLineEdit = obj && obj->inherits("QLineEdit");
            int key = keyEvent->key();

            // 1. 若目前在 QTE 頁籤或 QTE 正在運行：4 個快捷鍵 (1, 2, 3, 4) 對應 4 個 LED 按鈕
            if (ui->tabWidget->currentIndex() == 1 || qteIsRunning) {
                if (key == Qt::Key_1 && !isLineEdit) {
                    ui->qteBtn0->animateClick();
                    return true;
                } else if (key == Qt::Key_2 && !isLineEdit) {
                    ui->qteBtn1->animateClick();
                    return true;
                } else if (key == Qt::Key_3 && !isLineEdit) {
                    ui->qteBtn2->animateClick();
                    return true;
                } else if (key == Qt::Key_4 && !isLineEdit) {
                    ui->qteBtn3->animateClick();
                    return true;
                } else if ((key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Space) && !isLineEdit) {
                    if (!qteIsRunning) {
                        ui->qteStartBtn->animateClick();
                        return true;
                    }
                }
            } else {
                // 2. 基礎模式下快捷鍵
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
                } else if (key == Qt::Key_S && !isLineEdit) {
                    ui->ledShiningBtn->animateClick();
                    return true;
                } else if (key == Qt::Key_O && !isLineEdit) {
                    ui->switchingOnBtn->animateClick();
                    return true;
                } else if (key == Qt::Key_F && !isLineEdit) {
                    ui->switchingOffBtn->animateClick();
                    return true;
                } else if (key == Qt::Key_R && !isLineEdit) {
                    ui->runningOnBtn->animateClick();
                    return true;
                } else if (key == Qt::Key_X && !isLineEdit) {
                    ui->runningOffBtn->animateClick();
                    return true;
                } else if (key == Qt::Key_D && !isLineEdit) {
                    ui->changeDirBtn->animateClick();
                    return true;
                } else if (key == Qt::Key_Plus || key == Qt::Key_Equal || key == Qt::Key_Up) {
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
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    if (ui->tabWidget->currentIndex() == 1 || qteIsRunning) {
        switch (event->key()) {
        case Qt::Key_1:
            ui->qteBtn0->animateClick();
            event->accept();
            return;
        case Qt::Key_2:
            ui->qteBtn1->animateClick();
            event->accept();
            return;
        case Qt::Key_3:
            ui->qteBtn2->animateClick();
            event->accept();
            return;
        case Qt::Key_4:
            ui->qteBtn3->animateClick();
            event->accept();
            return;
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Space:
            if (!qteIsRunning) {
                ui->qteStartBtn->animateClick();
                event->accept();
                return;
            }
            break;
        default:
            break;
        }
    } else {
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
            break;
        }
    }

    QMainWindow::keyPressEvent(event);
}
