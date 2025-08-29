/********************************************************************************
** Form generated from reading UI file 'personalinfomanage.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PERSONALINFOMANAGE_H
#define UI_PERSONALINFOMANAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PersonalInfoManage
{
public:
    QPushButton *editButton;
    QPushButton *saveButton;
    QPushButton *cancelButton;
    QLineEdit *IDEdit;
    QLabel *label;
    QLabel *label_2;
    QLineEdit *emailEdit;
    QLabel *label_3;
    QLineEdit *phoneEdit;
    QLabel *label_4;
    QLineEdit *dateEdit;
    QLabel *label_5;
    QLineEdit *nameEdit;

    void setupUi(QWidget *PersonalInfoManage)
    {
        if (PersonalInfoManage->objectName().isEmpty())
            PersonalInfoManage->setObjectName("PersonalInfoManage");
        PersonalInfoManage->resize(842, 412);
        editButton = new QPushButton(PersonalInfoManage);
        editButton->setObjectName("editButton");
        editButton->setGeometry(QRect(370, 339, 91, 31));
        saveButton = new QPushButton(PersonalInfoManage);
        saveButton->setObjectName("saveButton");
        saveButton->setGeometry(QRect(260, 339, 101, 31));
        cancelButton = new QPushButton(PersonalInfoManage);
        cancelButton->setObjectName("cancelButton");
        cancelButton->setGeometry(QRect(470, 339, 91, 31));
        IDEdit = new QLineEdit(PersonalInfoManage);
        IDEdit->setObjectName("IDEdit");
        IDEdit->setGeometry(QRect(560, 280, 211, 31));
        label = new QLabel(PersonalInfoManage);
        label->setObjectName("label");
        label->setGeometry(QRect(510, 290, 51, 20));
        label_2 = new QLabel(PersonalInfoManage);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(530, 250, 40, 12));
        emailEdit = new QLineEdit(PersonalInfoManage);
        emailEdit->setObjectName("emailEdit");
        emailEdit->setGeometry(QRect(560, 240, 211, 31));
        label_3 = new QLabel(PersonalInfoManage);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(530, 210, 40, 12));
        phoneEdit = new QLineEdit(PersonalInfoManage);
        phoneEdit->setObjectName("phoneEdit");
        phoneEdit->setGeometry(QRect(560, 200, 211, 31));
        label_4 = new QLabel(PersonalInfoManage);
        label_4->setObjectName("label_4");
        label_4->setGeometry(QRect(530, 170, 40, 12));
        dateEdit = new QLineEdit(PersonalInfoManage);
        dateEdit->setObjectName("dateEdit");
        dateEdit->setGeometry(QRect(560, 160, 211, 31));
        label_5 = new QLabel(PersonalInfoManage);
        label_5->setObjectName("label_5");
        label_5->setGeometry(QRect(60, 290, 40, 12));
        nameEdit = new QLineEdit(PersonalInfoManage);
        nameEdit->setObjectName("nameEdit");
        nameEdit->setGeometry(QRect(90, 280, 211, 31));

        retranslateUi(PersonalInfoManage);

        QMetaObject::connectSlotsByName(PersonalInfoManage);
    } // setupUi

    void retranslateUi(QWidget *PersonalInfoManage)
    {
        PersonalInfoManage->setWindowTitle(QCoreApplication::translate("PersonalInfoManage", "Form", nullptr));
        editButton->setText(QCoreApplication::translate("PersonalInfoManage", "\347\274\226\350\276\221\344\270\252\344\272\272\344\277\241\346\201\257", nullptr));
        saveButton->setText(QCoreApplication::translate("PersonalInfoManage", "\344\277\235\345\255\230\346\233\264\346\224\271", nullptr));
        cancelButton->setText(QCoreApplication::translate("PersonalInfoManage", "\345\217\226\346\266\210\347\274\226\350\276\221", nullptr));
        label->setText(QCoreApplication::translate("PersonalInfoManage", "\350\272\253\344\273\275\350\257\201\345\217\267", nullptr));
        label_2->setText(QCoreApplication::translate("PersonalInfoManage", "\351\202\256\347\256\261", nullptr));
        label_3->setText(QCoreApplication::translate("PersonalInfoManage", "\347\224\265\350\257\235", nullptr));
        label_4->setText(QCoreApplication::translate("PersonalInfoManage", "\347\224\237\346\227\245", nullptr));
        label_5->setText(QCoreApplication::translate("PersonalInfoManage", "\345\247\223\345\220\215", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PersonalInfoManage: public Ui_PersonalInfoManage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PERSONALINFOMANAGE_H
