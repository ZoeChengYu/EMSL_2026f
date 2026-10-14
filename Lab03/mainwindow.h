#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    bool lights[4]; // state of each led

    bool shiningState;
    int rep; // reps left

    float countdown; // in seconds
    bool direction; // false being non-inverted

    int runningState; // which led is on when running

    void SyncCheckboxes();
    void UpdateLights();
    int GetDelay();

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_ledShiningBtn_clicked();

    void on_switchingOnBtn_clicked();

    void on_switchingOffBtn_clicked();

    void on_runningOnBtn_clicked();

    void on_runningOffBtn_clicked();

    void on_changeDirBtn_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
