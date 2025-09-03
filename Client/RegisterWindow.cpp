#include "RegisterWindow.h"
#include "SocketThread.h"
#include "EmailVerificationDialog.h"
#include <QPixmap>
#include <QMessageBox>
#include <QTcpSocket>
#include <QDebug>

RegisterWindow::RegisterWindow(QWidget *parent)
    : QWidget(parent)
    , m_emailVerified(false)
{
    setWindowTitle("智能医疗护理系统 - 注册");
    setFixedSize(550, 580);  // 进一步增大窗口尺寸，确保所有控件完整显示
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);  // 只保留关闭按钮
    
    // 设置整体背景样式
    setStyleSheet("QWidget {"
                  "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                  "stop:0 #667eea, stop:1 #764ba2);"
                  "}");

    // ========== 1. 初始化控件 ==========
    // 主容器
    QWidget *mainContainer = new QWidget(this);
    mainContainer->setGeometry(30, 20, 490, 540);  // 增大容器尺寸，确保有足够的空间显示所有控件
    mainContainer->setStyleSheet("QWidget {"
                                "background: rgba(255, 255, 255, 0.95);"
                                "border-radius: 15px;"
                                "border: 1px solid rgba(255, 255, 255, 0.2);"
                                "}");

    // 标题
    QLabel *titleLabel = new QLabel("智能医疗护理系统", mainContainer);
    titleLabel->setGeometry(20, 20, 450, 40);  // 增加宽度适应新的容器尺寸
    titleLabel->setStyleSheet("QLabel {"
                             "color: #2c3e50;"
                             "font-size: 20px;"
                             "font-weight: bold;"
                             "font-family: 'Microsoft YaHei UI';"
                             "}");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *subtitleLabel = new QLabel("创建您的账户", mainContainer);
    subtitleLabel->setGeometry(20, 55, 450, 20);  // 增加宽度适应新的容器尺寸
    subtitleLabel->setStyleSheet("QLabel {"
                                "color: #7f8c8d;"
                                "font-size: 14px;"
                                "font-family: 'Microsoft YaHei UI';"
                                "}");
    subtitleLabel->setAlignment(Qt::AlignCenter);

    // 注册信息（必填）区域
    lblRegTitle = new QLabel("注册信息(必填)", mainContainer);
    lblRegTitle->setGeometry(50, 90, 200, 25);
    lblRegTitle->setStyleSheet("QLabel {"
                              "color: #2c3e50;"
                              "font-size: 16px;"
                              "font-weight: bold;"
                              "font-family: 'Microsoft YaHei UI';"
                              "}");

    lblUsername = new QLabel("真实姓名:", mainContainer);
    editUsername = new QLineEdit(mainContainer);
    editUsername->setPlaceholderText("请输入真实姓名");
    
    lblPassword = new QLabel("注册密码:", mainContainer);
    editPassword = new QLineEdit(mainContainer);
    editPassword->setEchoMode(QLineEdit::Password);
    editPassword->setPlaceholderText("请输入密码");
    
    lblIdentity = new QLabel("身份:", mainContainer);
    rdbDoctor = new QRadioButton("医生", mainContainer);
    rdbPatient = new QRadioButton("病患", mainContainer);
    identityGroup = new QButtonGroup(this);
    identityGroup->addButton(rdbDoctor);
    identityGroup->addButton(rdbPatient);

    // 其他信息（选填）区域
    lblOtherTitle = new QLabel("其他信息(选填)", mainContainer);
    lblOtherTitle->setStyleSheet("QLabel {"
                                "color: #2c3e50;"
                                "font-size: 16px;"
                                "font-weight: bold;"
                                "font-family: 'Microsoft YaHei UI';"
                                "}");

    lblAge = new QLabel("年龄:", mainContainer);
    editAge = new QLineEdit(mainContainer);
    editAge->setPlaceholderText("请输入年龄");
    
    lblIDCard = new QLabel("身份证号:", mainContainer);
    editIDCard = new QLineEdit(mainContainer);
    editIDCard->setPlaceholderText("请输入18位身份证号");
    
    lblEmail = new QLabel("邮箱:", mainContainer);
    editEmail = new QLineEdit(mainContainer);
    editEmail->setPlaceholderText("请输入邮箱地址");
    
    btnVerifyEmail = new QPushButton("📧 验证邮箱", mainContainer);
    btnVerifyEmail->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #3498db, stop:0.5 #2980b9, stop:1 #1f618d);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    font-size: 12px;"
        "    font-weight: bold;"
        "    font-family: 'Microsoft YaHei UI';"
        "    padding: 8px 12px;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #2980b9, stop:0.5 #1f618d, stop:1 #154360);"
        "    transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #1f618d, stop:0.5 #154360, stop:1 #0e2f44);"
        "    transform: translateY(1px);"
        "}"
        "QPushButton:disabled {"
        "    background: #bdc3c7;"
        "    color: #7f8c8d;"
        "}"
    );
    
    lblEmailVerified = new QLabel("", mainContainer);
    lblEmailVerified->setStyleSheet("font-size: 12px; font-weight: bold; font-family: 'Microsoft YaHei UI';");
    
    lblPhone = new QLabel("电话:", mainContainer);
    editPhone = new QLineEdit(mainContainer);
    editPhone->setPlaceholderText("请输入11位手机号");

    // 设置通用的输入框样式
    QString lineEditStyle = "QLineEdit {"
                           "border: 2px solid #e1e8ed;"
                           "border-radius: 8px;"
                           "padding: 10px 12px;"
                           "font-size: 14px;"
                           "font-family: 'Microsoft YaHei UI';"
                           "background-color: white;"
                           "color: #2c3e50;"
                           "min-height: 16px;"
                           "}"
                           "QLineEdit:focus {"
                           "border: 2px solid #667eea;"
                           "outline: none;"
                           "}"
                           "QLineEdit:hover {"
                           "border: 2px solid #a8b8e6;"
                           "}";

    editUsername->setStyleSheet(lineEditStyle);
    editPassword->setStyleSheet(lineEditStyle);
    editAge->setStyleSheet(lineEditStyle);
    editIDCard->setStyleSheet(lineEditStyle);
    editEmail->setStyleSheet(lineEditStyle);
    editPhone->setStyleSheet(lineEditStyle);

    // 设置标签样式
    QString labelStyle = "QLabel {"
                        "color: #34495e;"
                        "font-size: 14px;"
                        "font-family: 'Microsoft YaHei UI';"
                        "}";
    lblUsername->setStyleSheet(labelStyle);
    lblPassword->setStyleSheet(labelStyle);
    lblIdentity->setStyleSheet(labelStyle);
    lblAge->setStyleSheet(labelStyle);
    lblIDCard->setStyleSheet(labelStyle);
    lblEmail->setStyleSheet(labelStyle);
    lblPhone->setStyleSheet(labelStyle);

    // 设置单选按钮样式
    QString radioStyle = "QRadioButton {"
                        "color: #34495e;"
                        "font-size: 14px;"
                        "font-family: 'Microsoft YaHei UI';"
                        "spacing: 5px;"
                        "}"
                        "QRadioButton::indicator {"
                        "width: 18px;"
                        "height: 18px;"
                        "}"
                        "QRadioButton::indicator:unchecked {"
                        "border: 2px solid #bdc3c7;"
                        "border-radius: 9px;"
                        "background-color: white;"
                        "}"
                        "QRadioButton::indicator:checked {"
                        "border: 2px solid #667eea;"
                        "border-radius: 9px;"
                        "background-color: #667eea;"
                        "}";
    rdbDoctor->setStyleSheet(radioStyle);
    rdbPatient->setStyleSheet(radioStyle);

    btnRegister = new QPushButton("注册", mainContainer);
    btnRegister->setStyleSheet("QPushButton {"
                              "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                              "stop:0 #667eea, stop:1 #764ba2);"
                              "border: none;"
                              "border-radius: 8px;"
                              "color: white;"
                              "font-size: 16px;"
                              "font-weight: bold;"
                              "font-family: 'Microsoft YaHei UI';"
                              "padding: 10px;"
                              "}"
                              "QPushButton:hover {"
                              "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                              "stop:0 #5a6fd8, stop:1 #6a4190);"
                              "}"
                              "QPushButton:pressed {"
                              "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                              "stop:0 #4a5fc6, stop:1 #5a377e);"
                              "}");
    
    btnExit = new QPushButton("取消", mainContainer);
    btnExit->setStyleSheet("QPushButton {"
                          "background: transparent;"
                          "border: 2px solid #667eea;"
                          "border-radius: 8px;"
                          "color: #667eea;"
                          "font-size: 16px;"
                          "font-weight: bold;"
                          "font-family: 'Microsoft YaHei UI';"
                          "padding: 10px;"
                          "}"
                          "QPushButton:hover {"
                          "background: rgba(102, 126, 234, 0.1);"
                          "border: 2px solid #5a6fd8;"
                          "color: #5a6fd8;"
                          "}"
                          "QPushButton:pressed {"
                          "background: rgba(102, 126, 234, 0.2);"
                          "border: 2px solid #4a5fc6;"
                          "color: #4a5fc6;"
                          "}");

    // ========== 2. 设置控件位置（使用绝对定位，单列布局） ==========
    // 必填信息区域
    lblRegTitle->setGeometry(50, 90, 250, 20);
    
    lblUsername->setGeometry(50, 120, 80, 20);
    editUsername->setGeometry(140, 115, 300, 36);
    
    lblPassword->setGeometry(50, 160, 80, 20);
    editPassword->setGeometry(140, 155, 300, 36);
    
    lblIdentity->setGeometry(50, 195, 60, 20);
    rdbDoctor->setGeometry(120, 195, 60, 20);
    rdbPatient->setGeometry(190, 195, 60, 20);

    // 选填信息区域
    lblOtherTitle->setGeometry(50, 230, 250, 20);
    
    lblAge->setGeometry(50, 260, 60, 20);
    editAge->setGeometry(140, 255, 300, 36);
    
    lblIDCard->setGeometry(50, 300, 80, 20);
    editIDCard->setGeometry(140, 295, 300, 36);
    
    lblEmail->setGeometry(50, 340, 60, 20);
    editEmail->setGeometry(140, 335, 210, 36);
    btnVerifyEmail->setGeometry(360, 335, 80, 36);
    lblEmailVerified->setGeometry(140, 375, 300, 20);
    
    lblPhone->setGeometry(50, 405, 60, 20);
    editPhone->setGeometry(140, 400, 300, 36);

    // 按钮区域
    btnRegister->setGeometry(175, 480, 100, 35);
    btnExit->setGeometry(295, 480, 100, 35);
    
    // 设置焦点到真实姓名输入框
    editUsername->setFocus();

    // ========== 3. 连接信号与槽 ==========
    connect(btnRegister, &QPushButton::clicked, this, &RegisterWindow::onRegisterClicked);
    connect(btnExit, &QPushButton::clicked, this, &RegisterWindow::onExitClicked);
    connect(btnVerifyEmail, &QPushButton::clicked, this, &RegisterWindow::onVerifyEmailClicked);
    
    // 邮箱输入变化时重置验证状态
    connect(editEmail, &QLineEdit::textChanged, [this]() {
        m_emailVerified = false;
        lblEmailVerified->setText("");
        btnVerifyEmail->setEnabled(!editEmail->text().isEmpty());
    });
    
    // 连接回车键事件到注册功能
    connect(editUsername, &QLineEdit::returnPressed, this, &RegisterWindow::onRegisterClicked);
    connect(editPassword, &QLineEdit::returnPressed, this, &RegisterWindow::onRegisterClicked);
    connect(editAge, &QLineEdit::returnPressed, this, &RegisterWindow::onRegisterClicked);
    connect(editIDCard, &QLineEdit::returnPressed, this, &RegisterWindow::onRegisterClicked);
    connect(editEmail, &QLineEdit::returnPressed, this, &RegisterWindow::onRegisterClicked);
    connect(editPhone, &QLineEdit::returnPressed, this, &RegisterWindow::onRegisterClicked);
}

RegisterWindow::~RegisterWindow()
{
    // 控件由布局管理，自动释放，无需手动删除
}

void RegisterWindow::onRegisterClicked()
{
    qDebug() << "Register button clicked";
    
    // 获取用户输入
    QString real_name = editUsername->text();
    QString password = editPassword->text();
    
    // 验证必填字段
    if (real_name.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "注册失败", "真实姓名和密码不能为空！");
        return;
    }
    
    // 验证身份选择
    if (!rdbDoctor->isChecked() && !rdbPatient->isChecked()) {
        QMessageBox::warning(this, "注册失败", "请选择身份（医生或病患）！");
        return;
    }
    
    QString identity = rdbDoctor->isChecked() ? "医生" : "病患";
    QString age = editAge->text();
    QString id_card = editIDCard->text();
    QString email = editEmail->text();
    QString phone = editPhone->text();
    
    // 验证身份证号格式（如果填写了就必须是18位数字）
    if (!id_card.isEmpty() && (id_card.length() != 18 || !id_card.toLongLong())) {
        QMessageBox::warning(this, "注册失败", "身份证号必须是18位数字！");
        return;
    }
    
    // 验证手机号格式（如果填写了就必须是11位数字）
    if (!phone.isEmpty() && (phone.length() != 11 || !phone.toLongLong())) {
        QMessageBox::warning(this, "注册失败", "手机号必须是11位数字！");
        return;
    }
    
    // 验证邮箱格式（如果填写了就必须包含@和.com）
    if (!email.isEmpty() && (!email.contains('@') || !email.contains(".com"))) {
        QMessageBox::warning(this, "注册失败", "邮箱格式不正确，必须包含@和.com！");
        return;
    }
    
    // 如果填写了邮箱，必须进行验证
    if (!email.isEmpty() && !m_emailVerified) {
        QMessageBox::warning(this, "注册失败", "请先验证邮箱地址！");
        return;
    }
    
    // 创建Socket连接
    QTcpSocket *socket = new QTcpSocket(this);
    socket->connectToHost("127.0.0.1", 8888);
    
    if (!socket->waitForConnected(1000)) {
        QMessageBox::warning(this, "连接错误", "无法连接到服务器");
        delete socket;
        return;
    }
    
    // 构造注册请求报文：REGISTER#username#password#identity#real_name#birth_date#id_card#phone#email
    // 注意：这里使用real_name作为username，birth_date暂时使用空字符串
    QString registerRequest = QString("REGISTER#%1#%2#%3#%4##%5#%6#%7")
                                .arg(real_name)
                                .arg(password)
                                .arg(identity)
                                .arg(real_name)
                                .arg(id_card)
                                .arg(phone)
                                .arg(email);
    
    socket->write(registerRequest.toUtf8() + "\n");
    
    // 等待服务器响应
    if (socket->waitForReadyRead(3000)) {
        QByteArray response = socket->readAll();
        QString responseStr = QString::fromUtf8(response);
        
        if (responseStr.startsWith("REGISTER_SUCCESS#")) {
            QString userId = responseStr.section('#', 1, 1);
            QMessageBox::information(this, "注册成功", 
                                    QString("用户 %1 注册成功！\n您的用户ID是：%2")
                                        .arg(real_name).arg(userId));
            emit registered(userId);  // 发射注册成功信号，传递用户ID
            emit closed();
            close();
        } else {
            QMessageBox::warning(this, "注册失败", "注册失败，请重试！");
        }
    } else {
        QMessageBox::warning(this, "超时", "服务器响应超时");
    }
    
    socket->disconnectFromHost();
    socket->deleteLater();
}

void RegisterWindow::onExitClicked()
{
    emit closed(); // 发射自定义关闭信号
    close(); // 恢复使用close()，依赖WA_DeleteOnClose属性删除窗口
}

void RegisterWindow::onVerifyEmailClicked()
{
    QString email = editEmail->text().trimmed();
    
    // 验证邮箱格式
    if (email.isEmpty()) {
        QMessageBox::warning(this, "验证失败", "请先输入邮箱地址！");
        return;
    }
    
    if (!email.contains('@') || !email.contains(".com")) {
        QMessageBox::warning(this, "验证失败", "邮箱格式不正确，必须包含@和.com！");
        return;
    }
    
    // 显示邮箱验证对话框
    if (EmailVerificationDialog::verifyEmail(email, "注册验证", this)) {
        m_emailVerified = true;
        lblEmailVerified->setText("✅ 已验证");
        lblEmailVerified->setStyleSheet("color: #27ae60; font-size: 12px; font-weight: bold;");
        btnVerifyEmail->setEnabled(false);
        QMessageBox::information(this, "验证成功", "邮箱验证成功！");
    } else {
        m_emailVerified = false;
        lblEmailVerified->setText("❌ 验证失败");
        lblEmailVerified->setStyleSheet("color: #e74c3c; font-size: 12px; font-weight: bold;");
    }
}
