#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QPushButton>
#include <QButtonGroup>
#include "EmailVerificationDialog.h"
#include "ConfigManager.h"

class RegisterWindow : public QWidget
{
    Q_OBJECT

public:
    RegisterWindow(QWidget *parent = nullptr);
    ~RegisterWindow();

signals:
    void closed();  // 自定义关闭信号
    void registered(const QString &userId);  // 注册成功信号，传递用户ID

private slots:
    void onRegisterClicked();  // 注册按钮点击槽函数
    void onExitClicked();      // 退出按钮点击槽函数
    void onVerifyEmailClicked(); // 验证邮箱按钮点击槽函数

private:
    // 移除了主布局，改用绝对定位

    // 注册信息（必填）区域控件
    QLabel *lblRegTitle;
    QLabel *lblUsername;
    QLineEdit *editUsername;
    QLabel *lblPassword;
    QLineEdit *editPassword;
    QLabel *lblIdentity;
    QRadioButton *rdbDoctor;
    QRadioButton *rdbPatient;
    QButtonGroup *identityGroup;

    // 其他信息（选填）区域控件
    QLabel *lblOtherTitle;
    QLabel *lblAge;
    QLineEdit *editAge;
    QLabel *lblIDCard;    // 添加身份证字段
    QLineEdit *editIDCard;
    QLabel *lblEmail;     // 添加邮箱字段
    QLineEdit *editEmail;
    QPushButton *btnVerifyEmail; // 邮箱验证按钮
    QLabel *lblEmailVerified;    // 邮箱验证状态标签
    QLabel *lblPhone;
    QLineEdit *editPhone;

    // 右侧图片和按钮区域控件
    QLabel *lblLogo;
    QPushButton *btnRegister;
    QPushButton *btnExit;
    
    // 验证状态
    bool m_emailVerified;
};

#endif // REGISTERWINDOW_H