/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.13
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *ledShiningBtn;
    QLabel *ledLabel0;
    QCheckBox *ledCheck0;
    QSlider *speedSlider;
    QPushButton *switchingOnBtn;
    QPushButton *switchingOffBtn;
    QPushButton *runningOnBtn;
    QProgressBar *speedProgress;
    QSpinBox *switchingRep;
    QPushButton *runningOffBtn;
    QPushButton *changeDirBtn;
    QLabel *ledLabel1;
    QLabel *ledLabel2;
    QLabel *ledLabel3;
    QCheckBox *ledCheck1;
    QCheckBox *ledCheck2;
    QCheckBox *ledCheck3;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        ledShiningBtn = new QPushButton(centralwidget);
        ledShiningBtn->setObjectName(QString::fromUtf8("ledShiningBtn"));
        ledShiningBtn->setGeometry(QRect(330, 440, 381, 61));
        ledLabel0 = new QLabel(centralwidget);
        ledLabel0->setObjectName(QString::fromUtf8("ledLabel0"));
        ledLabel0->setGeometry(QRect(90, 70, 71, 71));
        ledLabel0->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/Lit_Redstone_Lamp_JE2_BE1.png")));
        ledLabel0->setScaledContents(true);
        ledCheck0 = new QCheckBox(centralwidget);
        ledCheck0->setObjectName(QString::fromUtf8("ledCheck0"));
        ledCheck0->setGeometry(QRect(180, 80, 81, 61));
        speedSlider = new QSlider(centralwidget);
        speedSlider->setObjectName(QString::fromUtf8("speedSlider"));
        speedSlider->setGeometry(QRect(330, 360, 381, 21));
        speedSlider->setMinimum(1);
        speedSlider->setMaximum(100);
        speedSlider->setValue(50);
        speedSlider->setOrientation(Qt::Horizontal);
        switchingOnBtn = new QPushButton(centralwidget);
        switchingOnBtn->setObjectName(QString::fromUtf8("switchingOnBtn"));
        switchingOnBtn->setGeometry(QRect(330, 80, 151, 61));
        switchingOffBtn = new QPushButton(centralwidget);
        switchingOffBtn->setObjectName(QString::fromUtf8("switchingOffBtn"));
        switchingOffBtn->setGeometry(QRect(490, 80, 151, 61));
        runningOnBtn = new QPushButton(centralwidget);
        runningOnBtn->setObjectName(QString::fromUtf8("runningOnBtn"));
        runningOnBtn->setGeometry(QRect(330, 200, 121, 61));
        speedProgress = new QProgressBar(centralwidget);
        speedProgress->setObjectName(QString::fromUtf8("speedProgress"));
        speedProgress->setGeometry(QRect(330, 320, 381, 31));
        speedProgress->setMinimum(1);
        speedProgress->setValue(50);
        switchingRep = new QSpinBox(centralwidget);
        switchingRep->setObjectName(QString::fromUtf8("switchingRep"));
        switchingRep->setGeometry(QRect(650, 80, 61, 61));
        runningOffBtn = new QPushButton(centralwidget);
        runningOffBtn->setObjectName(QString::fromUtf8("runningOffBtn"));
        runningOffBtn->setGeometry(QRect(460, 200, 121, 61));
        changeDirBtn = new QPushButton(centralwidget);
        changeDirBtn->setObjectName(QString::fromUtf8("changeDirBtn"));
        changeDirBtn->setGeometry(QRect(590, 200, 121, 61));
        ledLabel1 = new QLabel(centralwidget);
        ledLabel1->setObjectName(QString::fromUtf8("ledLabel1"));
        ledLabel1->setGeometry(QRect(90, 190, 71, 71));
        ledLabel1->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/Lit_Redstone_Lamp_JE2_BE1.png")));
        ledLabel1->setScaledContents(true);
        ledLabel2 = new QLabel(centralwidget);
        ledLabel2->setObjectName(QString::fromUtf8("ledLabel2"));
        ledLabel2->setGeometry(QRect(90, 310, 71, 71));
        ledLabel2->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/Lit_Redstone_Lamp_JE2_BE1.png")));
        ledLabel2->setScaledContents(true);
        ledLabel3 = new QLabel(centralwidget);
        ledLabel3->setObjectName(QString::fromUtf8("ledLabel3"));
        ledLabel3->setGeometry(QRect(90, 430, 71, 71));
        ledLabel3->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/Lit_Redstone_Lamp_JE2_BE1.png")));
        ledLabel3->setScaledContents(true);
        ledCheck1 = new QCheckBox(centralwidget);
        ledCheck1->setObjectName(QString::fromUtf8("ledCheck1"));
        ledCheck1->setGeometry(QRect(180, 200, 81, 61));
        ledCheck2 = new QCheckBox(centralwidget);
        ledCheck2->setObjectName(QString::fromUtf8("ledCheck2"));
        ledCheck2->setGeometry(QRect(180, 310, 81, 61));
        ledCheck3 = new QCheckBox(centralwidget);
        ledCheck3->setObjectName(QString::fromUtf8("ledCheck3"));
        ledCheck3->setGeometry(QRect(180, 440, 81, 61));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 19));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);
        QObject::connect(speedSlider, SIGNAL(valueChanged(int)), speedProgress, SLOT(setValue(int)));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        ledShiningBtn->setText(QCoreApplication::translate("MainWindow", "LED shining", nullptr));
        ledLabel0->setText(QString());
        ledCheck0->setText(QCoreApplication::translate("MainWindow", "LED 0", nullptr));
        switchingOnBtn->setText(QCoreApplication::translate("MainWindow", "switching on", nullptr));
        switchingOffBtn->setText(QCoreApplication::translate("MainWindow", "switching off", nullptr));
        runningOnBtn->setText(QCoreApplication::translate("MainWindow", "running light on", nullptr));
        runningOffBtn->setText(QCoreApplication::translate("MainWindow", "running light off", nullptr));
        changeDirBtn->setText(QCoreApplication::translate("MainWindow", "change direction", nullptr));
        ledLabel1->setText(QString());
        ledLabel2->setText(QString());
        ledLabel3->setText(QString());
        ledCheck1->setText(QCoreApplication::translate("MainWindow", "LED 1", nullptr));
        ledCheck2->setText(QCoreApplication::translate("MainWindow", "LED 2", nullptr));
        ledCheck3->setText(QCoreApplication::translate("MainWindow", "LED 3", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
