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
    OFF, SWITCHING, RUNNING
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
    void onTimerTimeout();

    void on_ledShiningBtn_clicked();

    void on_switchingOnBtn_clicked();

    void on_switchingOffBtn_clicked();

    void on_runningOnBtn_clicked();

    void on_runningOffBtn_clicked();

    void on_changeDirBtn_clicked();

    void on_speedSlider_valueChanged(int value);

    void on_speedSpinBox_valueChanged(int value);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    int switchingPhase; // 0: LEDs 0,1 on; 1: LEDs 2,3 on

    void setupGpio();
    void setGpio(int pinIdx, int val);
    void cleanupGpio();
};
#endif // MAINWINDOW_H
