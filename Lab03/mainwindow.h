#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QKeyEvent>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

enum class Action {
    OFF, SWITCHING, RUNNING, QTE
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    Action actionState;
    bool lights[4];   // state of each led
    int rep;          // reps left for switching
    bool direction;   // false being non-inverted (0->1->2->3), true being inverted
    int runningState; // which led is on when running

    void SyncCheckboxes();
    void UpdateLights();
    int GetDelay();

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 基礎功能 Slots (項目一 ~ 四)
    void onTimerTimeout();
    void on_ledShiningBtn_clicked();
    void on_switchingOnBtn_clicked();
    void on_switchingOffBtn_clicked();
    void on_runningOnBtn_clicked();
    void on_runningOffBtn_clicked();
    void on_changeDirBtn_clicked();
    void on_speedSlider_valueChanged(int value);
    void on_speedSpinBox_valueChanged(int value);

    // 進階題：LED QTE Slots (Lab3_5_QT.pdf)
    void onQteBlinkTimeout();
    void onQteReactionTick();
    void on_qteStartBtn_clicked();
    void on_qteRestartBtn_clicked();
    void on_qteBtn0_clicked();
    void on_qteBtn1_clicked();
    void on_qteBtn2_clicked();
    void on_qteBtn3_clicked();
    void on_tabWidget_currentChanged(int index);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    int switchingPhase; // 0: LEDs 0,1 on; 1: LEDs 2,3 on

    // QTE 成員變數
    bool qteIsRunning;
    int qteScore;
    int qteTarget;            // 0..3: 當前隨機閃爍的目標 LED
    bool qteBlinkPhase;       // 閃爍亮/滅狀態
    int qteRemainingTimeMs;   // 當前 QTE 回合剩餘時間 (毫秒)
    QTimer *qteBlinkTimer;    // 控制 QTE 目標 LED 閃爍 (200ms)
    QTimer *qteReactionTimer; // 控制反應時限倒數 (3000ms)

    void initQteUi();
    void setQteLedVisual(int index, bool on);
    void resetButtonStyles();
    void startQteRound();
    void handleQteChoice(int choiceIndex);
    void qteGameOver(bool isTimeout);
    void qteVictory();
    void updateQteScoreDisplay(int chosenBtnIndex);

    void setupGpio();
    void setGpio(int pinIdx, int val);
    void cleanupGpio();
};
#endif // MAINWINDOW_H
