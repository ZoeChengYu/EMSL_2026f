/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
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
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QTabWidget *tabWidget;
    QWidget *tabBasic;
    QPushButton *ledShiningBtn;
    QLabel *ledLabel0;
    QCheckBox *ledCheck0;
    QSlider *speedSlider;
    QPushButton *switchingOnBtn;
    QPushButton *switchingOffBtn;
    QPushButton *runningOnBtn;
    QLabel *speedLabel;
    QProgressBar *speedProgress;
    QSpinBox *speedSpinBox;
    QSpinBox *switchingRep;
    QPushButton *runningOffBtn;
    QPushButton *changeDirBtn;
    QLabel *ledLabel1;
    QLabel *ledLabel2;
    QLabel *ledLabel3;
    QCheckBox *ledCheck1;
    QCheckBox *ledCheck2;
    QCheckBox *ledCheck3;
    QWidget *tabQTE;
    QLabel *qteHeaderLabel;
    QLabel *qteDescLabel;
    QLabel *qteLedText0;
    QLabel *qteLed0;
    QLabel *qteLedText1;
    QLabel *qteLed1;
    QLabel *qteLedText2;
    QLabel *qteLed2;
    QLabel *qteLedText3;
    QLabel *qteLed3;
    QLabel *qteScoreBox;
    QLabel *qteBonusLabel;
    QLabel *qteTimeTitle;
    QProgressBar *qteTimeBar;
    QLabel *qteBtnRowLabel;
    QPushButton *qteBtn0;
    QPushButton *qteBtn1;
    QPushButton *qteBtn2;
    QPushButton *qteBtn3;
    QPushButton *qteStartBtn;
    QPushButton *qteRestartBtn;
    QLabel *qteStatusLabel;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(820, 630);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setGeometry(QRect(10, 5, 800, 580));
        QFont font;
        font.setPointSize(10);
        tabWidget->setFont(font);
        tabBasic = new QWidget();
        tabBasic->setObjectName("tabBasic");
        ledShiningBtn = new QPushButton(tabBasic);
        ledShiningBtn->setObjectName("ledShiningBtn");
        ledShiningBtn->setGeometry(QRect(330, 440, 381, 61));
        ledLabel0 = new QLabel(tabBasic);
        ledLabel0->setObjectName("ledLabel0");
        ledLabel0->setGeometry(QRect(90, 50, 71, 71));
        ledLabel0->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/Lit_Redstone_Lamp_JE2_BE1.png")));
        ledLabel0->setScaledContents(true);
        ledCheck0 = new QCheckBox(tabBasic);
        ledCheck0->setObjectName("ledCheck0");
        ledCheck0->setGeometry(QRect(180, 60, 81, 61));
        speedSlider = new QSlider(tabBasic);
        speedSlider->setObjectName("speedSlider");
        speedSlider->setGeometry(QRect(330, 360, 300, 25));
        speedSlider->setMinimum(1);
        speedSlider->setMaximum(100);
        speedSlider->setValue(50);
        speedSlider->setOrientation(Qt::Horizontal);
        switchingOnBtn = new QPushButton(tabBasic);
        switchingOnBtn->setObjectName("switchingOnBtn");
        switchingOnBtn->setGeometry(QRect(330, 60, 151, 61));
        switchingOffBtn = new QPushButton(tabBasic);
        switchingOffBtn->setObjectName("switchingOffBtn");
        switchingOffBtn->setGeometry(QRect(490, 60, 151, 61));
        runningOnBtn = new QPushButton(tabBasic);
        runningOnBtn->setObjectName("runningOnBtn");
        runningOnBtn->setGeometry(QRect(330, 180, 121, 61));
        speedLabel = new QLabel(tabBasic);
        speedLabel->setObjectName("speedLabel");
        speedLabel->setGeometry(QRect(330, 260, 200, 31));
        QFont font1;
        font1.setPointSize(11);
        font1.setBold(true);
        speedLabel->setFont(font1);
        speedProgress = new QProgressBar(tabBasic);
        speedProgress->setObjectName("speedProgress");
        speedProgress->setGeometry(QRect(330, 300, 300, 31));
        speedProgress->setMinimum(1);
        speedProgress->setValue(50);
        speedSpinBox = new QSpinBox(tabBasic);
        speedSpinBox->setObjectName("speedSpinBox");
        speedSpinBox->setGeometry(QRect(645, 300, 66, 65));
        speedSpinBox->setMinimum(1);
        speedSpinBox->setMaximum(100);
        speedSpinBox->setValue(50);
        switchingRep = new QSpinBox(tabBasic);
        switchingRep->setObjectName("switchingRep");
        switchingRep->setGeometry(QRect(650, 60, 61, 61));
        switchingRep->setMinimum(1);
        switchingRep->setMaximum(999);
        switchingRep->setValue(5);
        runningOffBtn = new QPushButton(tabBasic);
        runningOffBtn->setObjectName("runningOffBtn");
        runningOffBtn->setGeometry(QRect(460, 180, 121, 61));
        changeDirBtn = new QPushButton(tabBasic);
        changeDirBtn->setObjectName("changeDirBtn");
        changeDirBtn->setGeometry(QRect(590, 180, 121, 61));
        ledLabel1 = new QLabel(tabBasic);
        ledLabel1->setObjectName("ledLabel1");
        ledLabel1->setGeometry(QRect(90, 170, 71, 71));
        ledLabel1->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/Lit_Redstone_Lamp_JE2_BE1.png")));
        ledLabel1->setScaledContents(true);
        ledLabel2 = new QLabel(tabBasic);
        ledLabel2->setObjectName("ledLabel2");
        ledLabel2->setGeometry(QRect(90, 290, 71, 71));
        ledLabel2->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/Lit_Redstone_Lamp_JE2_BE1.png")));
        ledLabel2->setScaledContents(true);
        ledLabel3 = new QLabel(tabBasic);
        ledLabel3->setObjectName("ledLabel3");
        ledLabel3->setGeometry(QRect(90, 410, 71, 71));
        ledLabel3->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/Lit_Redstone_Lamp_JE2_BE1.png")));
        ledLabel3->setScaledContents(true);
        ledCheck1 = new QCheckBox(tabBasic);
        ledCheck1->setObjectName("ledCheck1");
        ledCheck1->setGeometry(QRect(180, 180, 81, 61));
        ledCheck2 = new QCheckBox(tabBasic);
        ledCheck2->setObjectName("ledCheck2");
        ledCheck2->setGeometry(QRect(180, 290, 81, 61));
        ledCheck3 = new QCheckBox(tabBasic);
        ledCheck3->setObjectName("ledCheck3");
        ledCheck3->setGeometry(QRect(180, 420, 81, 61));
        tabWidget->addTab(tabBasic, QString());
        tabQTE = new QWidget();
        tabQTE->setObjectName("tabQTE");
        qteHeaderLabel = new QLabel(tabQTE);
        qteHeaderLabel->setObjectName("qteHeaderLabel");
        qteHeaderLabel->setGeometry(QRect(30, 6, 740, 26));
        QFont font2;
        font2.setPointSize(13);
        font2.setBold(true);
        qteHeaderLabel->setFont(font2);
        qteHeaderLabel->setStyleSheet(QString::fromUtf8("color: #0f172a;"));
        qteHeaderLabel->setAlignment(Qt::AlignCenter);
        qteDescLabel = new QLabel(tabQTE);
        qteDescLabel->setObjectName("qteDescLabel");
        qteDescLabel->setGeometry(QRect(30, 32, 740, 20));
        QFont font3;
        font3.setPointSize(9);
        qteDescLabel->setFont(font3);
        qteDescLabel->setStyleSheet(QString::fromUtf8("color: #475569;"));
        qteDescLabel->setAlignment(Qt::AlignCenter);
        qteLedText0 = new QLabel(tabQTE);
        qteLedText0->setObjectName("qteLedText0");
        qteLedText0->setGeometry(QRect(115, 58, 70, 20));
        QFont font4;
        font4.setPointSize(12);
        font4.setBold(true);
        qteLedText0->setFont(font4);
        qteLedText0->setStyleSheet(QString::fromUtf8("color: #334155;"));
        qteLedText0->setAlignment(Qt::AlignCenter);
        qteLed0 = new QLabel(tabQTE);
        qteLed0->setObjectName("qteLed0");
        qteLed0->setGeometry(QRect(115, 80, 70, 70));
        qteLed0->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/Redstone_Lamp_JE3_BE2.png")));
        qteLed0->setScaledContents(true);
        qteLedText1 = new QLabel(tabQTE);
        qteLedText1->setObjectName("qteLedText1");
        qteLedText1->setGeometry(QRect(275, 58, 70, 20));
        qteLedText1->setFont(font4);
        qteLedText1->setStyleSheet(QString::fromUtf8("color: #334155;"));
        qteLedText1->setAlignment(Qt::AlignCenter);
        qteLed1 = new QLabel(tabQTE);
        qteLed1->setObjectName("qteLed1");
        qteLed1->setGeometry(QRect(275, 80, 70, 70));
        qteLed1->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/Redstone_Lamp_JE3_BE2.png")));
        qteLed1->setScaledContents(true);
        qteLedText2 = new QLabel(tabQTE);
        qteLedText2->setObjectName("qteLedText2");
        qteLedText2->setGeometry(QRect(435, 58, 70, 20));
        qteLedText2->setFont(font4);
        qteLedText2->setStyleSheet(QString::fromUtf8("color: #334155;"));
        qteLedText2->setAlignment(Qt::AlignCenter);
        qteLed2 = new QLabel(tabQTE);
        qteLed2->setObjectName("qteLed2");
        qteLed2->setGeometry(QRect(435, 80, 70, 70));
        qteLed2->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/Redstone_Lamp_JE3_BE2.png")));
        qteLed2->setScaledContents(true);
        qteLedText3 = new QLabel(tabQTE);
        qteLedText3->setObjectName("qteLedText3");
        qteLedText3->setGeometry(QRect(595, 58, 70, 20));
        qteLedText3->setFont(font4);
        qteLedText3->setStyleSheet(QString::fromUtf8("color: #334155;"));
        qteLedText3->setAlignment(Qt::AlignCenter);
        qteLed3 = new QLabel(tabQTE);
        qteLed3->setObjectName("qteLed3");
        qteLed3->setGeometry(QRect(595, 80, 70, 70));
        qteLed3->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/Redstone_Lamp_JE3_BE2.png")));
        qteLed3->setScaledContents(true);
        qteScoreBox = new QLabel(tabQTE);
        qteScoreBox->setObjectName("qteScoreBox");
        qteScoreBox->setGeometry(QRect(290, 165, 200, 56));
        QFont font5;
        font5.setPointSize(28);
        font5.setBold(true);
        qteScoreBox->setFont(font5);
        qteScoreBox->setStyleSheet(QString::fromUtf8("background-color: #000000; color: #ff1111; border: 4px solid #38bdf8; border-radius: 6px; font-family: Consolas, monospace; padding: 2px;"));
        qteScoreBox->setAlignment(Qt::AlignCenter);
        qteBonusLabel = new QLabel(tabQTE);
        qteBonusLabel->setObjectName("qteBonusLabel");
        qteBonusLabel->setGeometry(QRect(275, 245, 70, 35));
        QFont font6;
        font6.setPointSize(22);
        font6.setBold(true);
        qteBonusLabel->setFont(font6);
        qteBonusLabel->setStyleSheet(QString::fromUtf8("color: #ea580c; font-weight: bold; font-family: serif;"));
        qteBonusLabel->setAlignment(Qt::AlignCenter);
        qteTimeTitle = new QLabel(tabQTE);
        qteTimeTitle->setObjectName("qteTimeTitle");
        qteTimeTitle->setGeometry(QRect(180, 230, 80, 20));
        QFont font7;
        font7.setPointSize(9);
        font7.setBold(true);
        qteTimeTitle->setFont(font7);
        qteTimeTitle->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
        qteTimeBar = new QProgressBar(tabQTE);
        qteTimeBar->setObjectName("qteTimeBar");
        qteTimeBar->setGeometry(QRect(265, 230, 270, 18));
        qteTimeBar->setMaximum(3000);
        qteTimeBar->setValue(3000);
        qteTimeBar->setTextVisible(false);
        qteTimeBar->setStyleSheet(QString::fromUtf8("QProgressBar { border: 1px solid #94a3b8; border-radius: 4px; background: #e2e8f0; } QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #10b981, stop:0.7 #f59e0b, stop:1 #ef4444); border-radius: 3px; }"));
        qteBtnRowLabel = new QLabel(tabQTE);
        qteBtnRowLabel->setObjectName("qteBtnRowLabel");
        qteBtnRowLabel->setGeometry(QRect(680, 305, 85, 35));
        qteBtnRowLabel->setFont(font2);
        qteBtnRowLabel->setStyleSheet(QString::fromUtf8("color: #1e293b;"));
        qteBtnRowLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        qteBtn0 = new QPushButton(tabQTE);
        qteBtn0->setObjectName("qteBtn0");
        qteBtn0->setGeometry(QRect(115, 285, 70, 70));
        QFont font8;
        font8.setPointSize(20);
        font8.setBold(true);
        qteBtn0->setFont(font8);
        qteBtn0->setStyleSheet(QString::fromUtf8("QPushButton { background: qradialgradient(cx:0.4, cy:0.4, radius:0.6, fx:0.3, fy:0.3, stop:0 #93c5fd, stop:0.75 #3b82f6, stop:1 #1d4ed8); color: white; border: 3px solid #60a5fa; border-radius: 35px; font-family: serif; } QPushButton:hover { background: qradialgradient(cx:0.4, cy:0.4, radius:0.6, fx:0.3, fy:0.3, stop:0 #bfdbfe, stop:0.75 #60a5fa, stop:1 #2563eb); } QPushButton:pressed { background: #1d4ed8; }"));
        qteBtn1 = new QPushButton(tabQTE);
        qteBtn1->setObjectName("qteBtn1");
        qteBtn1->setGeometry(QRect(275, 285, 70, 70));
        qteBtn1->setFont(font8);
        qteBtn1->setStyleSheet(QString::fromUtf8("QPushButton { background: qradialgradient(cx:0.4, cy:0.4, radius:0.6, fx:0.3, fy:0.3, stop:0 #93c5fd, stop:0.75 #3b82f6, stop:1 #1d4ed8); color: white; border: 3px solid #60a5fa; border-radius: 35px; font-family: serif; } QPushButton:hover { background: qradialgradient(cx:0.4, cy:0.4, radius:0.6, fx:0.3, fy:0.3, stop:0 #bfdbfe, stop:0.75 #60a5fa, stop:1 #2563eb); } QPushButton:pressed { background: #1d4ed8; }"));
        qteBtn2 = new QPushButton(tabQTE);
        qteBtn2->setObjectName("qteBtn2");
        qteBtn2->setGeometry(QRect(435, 285, 70, 70));
        qteBtn2->setFont(font8);
        qteBtn2->setStyleSheet(QString::fromUtf8("QPushButton { background: qradialgradient(cx:0.4, cy:0.4, radius:0.6, fx:0.3, fy:0.3, stop:0 #93c5fd, stop:0.75 #3b82f6, stop:1 #1d4ed8); color: white; border: 3px solid #60a5fa; border-radius: 35px; font-family: serif; } QPushButton:hover { background: qradialgradient(cx:0.4, cy:0.4, radius:0.6, fx:0.3, fy:0.3, stop:0 #bfdbfe, stop:0.75 #60a5fa, stop:1 #2563eb); } QPushButton:pressed { background: #1d4ed8; }"));
        qteBtn3 = new QPushButton(tabQTE);
        qteBtn3->setObjectName("qteBtn3");
        qteBtn3->setGeometry(QRect(595, 285, 70, 70));
        qteBtn3->setFont(font8);
        qteBtn3->setStyleSheet(QString::fromUtf8("QPushButton { background: qradialgradient(cx:0.4, cy:0.4, radius:0.6, fx:0.3, fy:0.3, stop:0 #93c5fd, stop:0.75 #3b82f6, stop:1 #1d4ed8); color: white; border: 3px solid #60a5fa; border-radius: 35px; font-family: serif; } QPushButton:hover { background: qradialgradient(cx:0.4, cy:0.4, radius:0.6, fx:0.3, fy:0.3, stop:0 #bfdbfe, stop:0.75 #60a5fa, stop:1 #2563eb); } QPushButton:pressed { background: #1d4ed8; }"));
        qteStartBtn = new QPushButton(tabQTE);
        qteStartBtn->setObjectName("qteStartBtn");
        qteStartBtn->setGeometry(QRect(180, 390, 200, 48));
        qteStartBtn->setFont(font4);
        qteStartBtn->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #10b981; color: white; border-radius: 10px; } QPushButton:hover { background-color: #059669; } QPushButton:pressed { background-color: #047857; }"));
        qteRestartBtn = new QPushButton(tabQTE);
        qteRestartBtn->setObjectName("qteRestartBtn");
        qteRestartBtn->setGeometry(QRect(420, 390, 200, 48));
        qteRestartBtn->setFont(font4);
        qteRestartBtn->setStyleSheet(QString::fromUtf8("QPushButton { background-color: #f59e0b; color: white; border-radius: 10px; } QPushButton:hover { background-color: #d97706; } QPushButton:pressed { background-color: #b45309; }"));
        qteStatusLabel = new QLabel(tabQTE);
        qteStatusLabel->setObjectName("qteStatusLabel");
        qteStatusLabel->setGeometry(QRect(40, 460, 720, 45));
        qteStatusLabel->setFont(font1);
        qteStatusLabel->setStyleSheet(QString::fromUtf8("color: #1e293b; background-color: #f8fafc; border: 1px solid #cbd5e1; border-radius: 8px; padding: 4px;"));
        qteStatusLabel->setAlignment(Qt::AlignCenter);
        tabWidget->addTab(tabQTE, QString());
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 820, 20));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Lab3 - GPIO LED Control & QTE Game", nullptr));
#if QT_CONFIG(shortcut)
        ledShiningBtn->setShortcut(QCoreApplication::translate("MainWindow", "S", nullptr));
#endif // QT_CONFIG(shortcut)
        ledShiningBtn->setText(QCoreApplication::translate("MainWindow", "LED Shining", nullptr));
        ledLabel0->setText(QString());
#if QT_CONFIG(shortcut)
        ledCheck0->setShortcut(QCoreApplication::translate("MainWindow", "1", nullptr));
#endif // QT_CONFIG(shortcut)
        ledCheck0->setText(QCoreApplication::translate("MainWindow", "LED1", nullptr));
#if QT_CONFIG(shortcut)
        switchingOnBtn->setShortcut(QCoreApplication::translate("MainWindow", "O", nullptr));
#endif // QT_CONFIG(shortcut)
        switchingOnBtn->setText(QCoreApplication::translate("MainWindow", "Switching on", nullptr));
#if QT_CONFIG(shortcut)
        switchingOffBtn->setShortcut(QCoreApplication::translate("MainWindow", "F", nullptr));
#endif // QT_CONFIG(shortcut)
        switchingOffBtn->setText(QCoreApplication::translate("MainWindow", "Switching off", nullptr));
#if QT_CONFIG(shortcut)
        runningOnBtn->setShortcut(QCoreApplication::translate("MainWindow", "R", nullptr));
#endif // QT_CONFIG(shortcut)
        runningOnBtn->setText(QCoreApplication::translate("MainWindow", "Running Light on", nullptr));
        speedLabel->setText(QCoreApplication::translate("MainWindow", "Switching Speed", nullptr));
        speedSpinBox->setSuffix(QCoreApplication::translate("MainWindow", "%", nullptr));
#if QT_CONFIG(shortcut)
        runningOffBtn->setShortcut(QCoreApplication::translate("MainWindow", "X", nullptr));
#endif // QT_CONFIG(shortcut)
        runningOffBtn->setText(QCoreApplication::translate("MainWindow", "Running Light off", nullptr));
#if QT_CONFIG(shortcut)
        changeDirBtn->setShortcut(QCoreApplication::translate("MainWindow", "D", nullptr));
#endif // QT_CONFIG(shortcut)
        changeDirBtn->setText(QCoreApplication::translate("MainWindow", "Change Direction", nullptr));
        ledLabel1->setText(QString());
        ledLabel2->setText(QString());
        ledLabel3->setText(QString());
#if QT_CONFIG(shortcut)
        ledCheck1->setShortcut(QCoreApplication::translate("MainWindow", "2", nullptr));
#endif // QT_CONFIG(shortcut)
        ledCheck1->setText(QCoreApplication::translate("MainWindow", "LED2", nullptr));
#if QT_CONFIG(shortcut)
        ledCheck2->setShortcut(QCoreApplication::translate("MainWindow", "3", nullptr));
#endif // QT_CONFIG(shortcut)
        ledCheck2->setText(QCoreApplication::translate("MainWindow", "LED3", nullptr));
#if QT_CONFIG(shortcut)
        ledCheck3->setShortcut(QCoreApplication::translate("MainWindow", "4", nullptr));
#endif // QT_CONFIG(shortcut)
        ledCheck3->setText(QCoreApplication::translate("MainWindow", "LED4", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabBasic), QCoreApplication::translate("MainWindow", "\345\237\272\347\244\216\345\212\237\350\203\275 (\351\240\205\347\233\256\344\270\200 ~ \345\233\233)", nullptr));
        qteHeaderLabel->setText(QCoreApplication::translate("MainWindow", "\351\200\262\351\232\216\351\241\214\357\274\232LED QTE (Quick Time Event) \345\277\253\351\200\237\345\217\215\346\207\211\344\272\213\344\273\266\346\214\221\346\210\260 (20%)", nullptr));
        qteDescLabel->setText(QCoreApplication::translate("MainWindow", "\346\214\211\344\270\213\351\226\213\345\247\213\357\274\214\347\225\253\351\235\242\350\210\207 TX2 \344\270\212\347\232\204 LED \351\232\250\346\251\237\351\226\203\347\210\215\357\274\214\350\253\213\351\201\270\351\226\203\347\210\215\344\270\255\347\232\204 LED\343\200\202\346\257\217\346\210\220\345\212\237\345\212\240 20 \345\210\206\357\274\214\351\201\224 100 \345\210\206\345\215\263\351\200\232\351\201\216\357\274\201\345\244\261\346\225\227\345\211\207\346\255\270\351\233\266\347\265\202\346\255\242\343\200\202", nullptr));
        qteLedText0->setText(QCoreApplication::translate("MainWindow", "1", nullptr));
        qteLed0->setText(QString());
        qteLedText1->setText(QCoreApplication::translate("MainWindow", "2", nullptr));
        qteLed1->setText(QString());
        qteLedText2->setText(QCoreApplication::translate("MainWindow", "3", nullptr));
        qteLed2->setText(QString());
        qteLedText3->setText(QCoreApplication::translate("MainWindow", "4", nullptr));
        qteLed3->setText(QString());
        qteScoreBox->setText(QCoreApplication::translate("MainWindow", "0000", nullptr));
        qteBonusLabel->setText(QString());
        qteTimeTitle->setText(QCoreApplication::translate("MainWindow", "\346\231\202\351\231\220\345\200\222\346\225\270\357\274\232", nullptr));
        qteBtnRowLabel->setText(QCoreApplication::translate("MainWindow", "\346\214\211\351\210\225\345\210\227", nullptr));
        qteBtn0->setText(QCoreApplication::translate("MainWindow", "1", nullptr));
#if QT_CONFIG(shortcut)
        qteBtn0->setShortcut(QCoreApplication::translate("MainWindow", "1", nullptr));
#endif // QT_CONFIG(shortcut)
        qteBtn1->setText(QCoreApplication::translate("MainWindow", "2", nullptr));
#if QT_CONFIG(shortcut)
        qteBtn1->setShortcut(QCoreApplication::translate("MainWindow", "2", nullptr));
#endif // QT_CONFIG(shortcut)
        qteBtn2->setText(QCoreApplication::translate("MainWindow", "3", nullptr));
#if QT_CONFIG(shortcut)
        qteBtn2->setShortcut(QCoreApplication::translate("MainWindow", "3", nullptr));
#endif // QT_CONFIG(shortcut)
        qteBtn3->setText(QCoreApplication::translate("MainWindow", "4", nullptr));
#if QT_CONFIG(shortcut)
        qteBtn3->setShortcut(QCoreApplication::translate("MainWindow", "4", nullptr));
#endif // QT_CONFIG(shortcut)
        qteStartBtn->setText(QCoreApplication::translate("MainWindow", "\351\226\213\345\247\213 (Start)", nullptr));
        qteRestartBtn->setText(QCoreApplication::translate("MainWindow", "\345\206\215\347\216\251\344\270\200\346\254\241 (Restart)", nullptr));
        qteStatusLabel->setText(QCoreApplication::translate("MainWindow", "\350\253\213\346\214\211\344\270\213\343\200\216\351\226\213\345\247\213\343\200\217\345\225\237\345\213\225 LED QTE \346\214\221\346\210\260\357\274\201", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabQTE), QCoreApplication::translate("MainWindow", "\351\200\262\351\232\216\351\241\214\357\274\232LED QTE (20%)", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
