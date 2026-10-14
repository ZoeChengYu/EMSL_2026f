#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QTimer>
#include <QEventLoop>

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
}

MainWindow::~MainWindow()
{
    delete ui;
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

