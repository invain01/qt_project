/********************************************************************************
** Form generated from reading UI file 'loginwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINWINDOW_H
#define UI_LOGINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoginWindow
{
public:
    QWidget *mainContainer;
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QLineEdit *usernameLineEdit;
    QLineEdit *passwordLineEdit;
    QPushButton *loginButton;
    QPushButton *loginButton_2;

    void setupUi(QWidget *LoginWindow)
    {
        if (LoginWindow->objectName().isEmpty())
            LoginWindow->setObjectName("LoginWindow");
        LoginWindow->resize(500, 400);
        LoginWindow->setStyleSheet(QString::fromUtf8("QWidget#LoginWindow {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 #667eea, stop:1 #764ba2);\n"
"}"));
        mainContainer = new QWidget(LoginWindow);
        mainContainer->setObjectName("mainContainer");
        mainContainer->setGeometry(QRect(50, 50, 400, 300));
        mainContainer->setStyleSheet(QString::fromUtf8("QWidget#mainContainer {\n"
"    background: rgba(255, 255, 255, 0.95);\n"
"    border-radius: 15px;\n"
"    border: 1px solid rgba(255, 255, 255, 0.2);\n"
"}"));
        titleLabel = new QLabel(mainContainer);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setGeometry(QRect(50, 30, 300, 40));
        titleLabel->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    color: #2c3e50;\n"
"    font-size: 24px;\n"
"    font-weight: bold;\n"
"    font-family: \"Microsoft YaHei UI\";\n"
"}"));
        titleLabel->setAlignment(Qt::AlignCenter);
        subtitleLabel = new QLabel(mainContainer);
        subtitleLabel->setObjectName("subtitleLabel");
        subtitleLabel->setGeometry(QRect(50, 70, 300, 20));
        subtitleLabel->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    color: #7f8c8d;\n"
"    font-size: 14px;\n"
"    font-family: \"Microsoft YaHei UI\";\n"
"}"));
        subtitleLabel->setAlignment(Qt::AlignCenter);
        usernameLineEdit = new QLineEdit(mainContainer);
        usernameLineEdit->setObjectName("usernameLineEdit");
        usernameLineEdit->setGeometry(QRect(50, 120, 300, 45));
        usernameLineEdit->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    border: 2px solid #e1e8ed;\n"
"    border-radius: 22px;\n"
"    padding: 0 20px;\n"
"    font-size: 14px;\n"
"    font-family: \"Microsoft YaHei UI\";\n"
"    background-color: white;\n"
"    color: #2c3e50;\n"
"}\n"
"\n"
"QLineEdit:focus {\n"
"    border: 2px solid #667eea;\n"
"    outline: none;\n"
"}\n"
"\n"
"QLineEdit:hover {\n"
"    border: 2px solid #a8b8e6;\n"
"}"));
        passwordLineEdit = new QLineEdit(mainContainer);
        passwordLineEdit->setObjectName("passwordLineEdit");
        passwordLineEdit->setGeometry(QRect(50, 180, 300, 45));
        passwordLineEdit->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    border: 2px solid #e1e8ed;\n"
"    border-radius: 22px;\n"
"    padding: 0 20px;\n"
"    font-size: 14px;\n"
"    font-family: \"Microsoft YaHei UI\";\n"
"    background-color: white;\n"
"    color: #2c3e50;\n"
"}\n"
"\n"
"QLineEdit:focus {\n"
"    border: 2px solid #667eea;\n"
"    outline: none;\n"
"}\n"
"\n"
"QLineEdit:hover {\n"
"    border: 2px solid #a8b8e6;\n"
"}"));
        passwordLineEdit->setEchoMode(QLineEdit::EchoMode::Password);
        loginButton = new QPushButton(mainContainer);
        loginButton->setObjectName("loginButton");
        loginButton->setGeometry(QRect(50, 245, 140, 40));
        loginButton->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,\n"
"        stop:0 #667eea, stop:1 #764ba2);\n"
"    border: none;\n"
"    border-radius: 20px;\n"
"    color: white;\n"
"    font-size: 16px;\n"
"    font-weight: bold;\n"
"    font-family: \"Microsoft YaHei UI\";\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,\n"
"        stop:0 #5a6fd8, stop:1 #6a4190);\n"
"    margin-top: -2px;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,\n"
"        stop:0 #4a5fc6, stop:1 #5a377e);\n"
"}"));
        loginButton_2 = new QPushButton(mainContainer);
        loginButton_2->setObjectName("loginButton_2");
        loginButton_2->setGeometry(QRect(210, 245, 140, 40));
        loginButton_2->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background: transparent;\n"
"    border: 2px solid #667eea;\n"
"    border-radius: 20px;\n"
"    color: #667eea;\n"
"    font-size: 16px;\n"
"    font-weight: bold;\n"
"    font-family: \"Microsoft YaHei UI\";\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    background: rgba(102, 126, 234, 0.1);\n"
"    border: 2px solid #5a6fd8;\n"
"    color: #5a6fd8;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background: rgba(102, 126, 234, 0.2);\n"
"    border: 2px solid #4a5fc6;\n"
"    color: #4a5fc6;\n"
"}"));

        retranslateUi(LoginWindow);

        QMetaObject::connectSlotsByName(LoginWindow);
    } // setupUi

    void retranslateUi(QWidget *LoginWindow)
    {
        LoginWindow->setWindowTitle(QCoreApplication::translate("LoginWindow", "\346\231\272\350\203\275\345\214\273\347\226\227\346\212\244\347\220\206\347\263\273\347\273\237 - \347\231\273\345\275\225", nullptr));
        titleLabel->setText(QCoreApplication::translate("LoginWindow", "\346\231\272\350\203\275\345\214\273\347\226\227\346\212\244\347\220\206\347\263\273\347\273\237", nullptr));
        subtitleLabel->setText(QCoreApplication::translate("LoginWindow", "\347\231\273\345\275\225\346\202\250\347\232\204\350\264\246\346\210\267", nullptr));
        loginButton->setText(QCoreApplication::translate("LoginWindow", "\347\231\273\345\275\225", nullptr));
        loginButton_2->setText(QCoreApplication::translate("LoginWindow", "\346\263\250\345\206\214", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoginWindow: public Ui_LoginWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINWINDOW_H
