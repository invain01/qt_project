#include "loginwindow.h"
#include "ui_loginwindow.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QTcpSocket>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    ui->usernameLineEdit->setPlaceholderText(tr("用户id")); // 设置提示文字为“用户id”
    ui->usernameLineEdit->setStyleSheet("background-color: white; color:black;");
    ui->passwordLineEdit->setPlaceholderText(tr("密码")); // 设置提示文字为“密码”
    ui->passwordLineEdit->setStyleSheet("background-color: white; color:black;");
}

// 新增带参数的构造函数
LoginWindow::LoginWindow(const QString &serverAddress, quint16 port, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWindow)
    , m_serverAddress(serverAddress)
    , m_port(port)
{
    ui->setupUi(this);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::on_loginButton_clicked()
{
    QString id = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    authenticateUser(id, password);
}

void LoginWindow::authenticateUser(const QString &id, const QString &password)
{

    if (!socket->waitForConnected(1000)) { // 等待连接建立（1秒超时）
        QMessageBox::warning(this, "Connection Error", "无法连接到服务器");
        delete socket;
        return;
    }

    // 构造登录请求报文：LOGIN#id#password
    QString loginRequest = QString("LOGIN#%1#%2").arg(id).arg(password);
    socket->write(loginRequest.toUtf8()); // 发送登录请求

    // 等待服务器响应（1秒超时）
    if (socket->waitForReadyRead(1000)) {
        QByteArray response = socket->readAll();
        QString responseStr = QString::fromUtf8(response);

        if (responseStr == "LOGIN_SUCCESS") {
            emit loginSuccessful(id);
            this->close(); // 关闭登录窗口
        } else {
            QMessageBox::warning(this, "Login Failed", "用户id或密码错误");
        }
    } else {
        QMessageBox::warning(this, "Timeout", "服务器响应超时");
    }
}
