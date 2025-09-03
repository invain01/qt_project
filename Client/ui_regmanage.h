/********************************************************************************
** Form generated from reading UI file 'regmanage.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REGMANAGE_H
#define UI_REGMANAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_manageinfor_window
{
public:

    void setupUi(QWidget *manageinfor_window)
    {
        if (manageinfor_window->objectName().isEmpty())
            manageinfor_window->setObjectName("manageinfor_window");
        manageinfor_window->resize(400, 300);

        retranslateUi(manageinfor_window);

        QMetaObject::connectSlotsByName(manageinfor_window);
    } // setupUi

    void retranslateUi(QWidget *manageinfor_window)
    {
        manageinfor_window->setWindowTitle(QCoreApplication::translate("manageinfor_window", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class manageinfor_window: public Ui_manageinfor_window {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REGMANAGE_H
