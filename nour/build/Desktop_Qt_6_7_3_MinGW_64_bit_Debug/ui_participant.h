/********************************************************************************
** Form generated from reading UI file 'participant.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PARTICIPANT_H
#define UI_PARTICIPANT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Participant
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *Participant)
    {
        if (Participant->objectName().isEmpty())
            Participant->setObjectName("Participant");
        Participant->resize(1313, 725);
        centralwidget = new QWidget(Participant);
        centralwidget->setObjectName("centralwidget");
        Participant->setCentralWidget(centralwidget);
        menubar = new QMenuBar(Participant);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1313, 26));
        Participant->setMenuBar(menubar);
        statusbar = new QStatusBar(Participant);
        statusbar->setObjectName("statusbar");
        Participant->setStatusBar(statusbar);

        retranslateUi(Participant);

        QMetaObject::connectSlotsByName(Participant);
    } // setupUi

    void retranslateUi(QMainWindow *Participant)
    {
        Participant->setWindowTitle(QCoreApplication::translate("Participant", "Participant", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Participant: public Ui_Participant {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PARTICIPANT_H
