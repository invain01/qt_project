/********************************************************************************
** Form generated from reading UI file 'casepresc.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CASEPRESC_H
#define UI_CASEPRESC_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_casepresc
{
public:

    void setupUi(QWidget *casepresc)
    {
        if (casepresc->objectName().isEmpty())
            casepresc->setObjectName("casepresc");
        casepresc->resize(400, 300);

        retranslateUi(casepresc);

        QMetaObject::connectSlotsByName(casepresc);
    } // setupUi

    void retranslateUi(QWidget *casepresc)
    {
        casepresc->setWindowTitle(QCoreApplication::translate("casepresc", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class casepresc: public Ui_casepresc {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CASEPRESC_H
