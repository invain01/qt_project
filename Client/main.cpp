#include <QApplication>
#include "loginwindow.h"
#include "personalinfomanage.h"
#include <QTcpSocket>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 创建TCP套接字连接服务器
    QTcpSocket *socket = new QTcpSocket();
    //socket->connectToHost("192.168.43.173", 8888); // 连接到服务器地址和端口
    socket->connectToHost("127.0.0.1", 8888); // 连接到服务器地址和端口

    LoginWindow *loginWindow = new LoginWindow();
    loginWindow->socket = socket;

    // 连接登录成功信号
    QObject::connect(loginWindow, &LoginWindow::loginSuccessful,
                     [=](const QString &id) {
                         // 在登录成功后才创建个人信息管理窗口
                         PersonalInfoManage *personalInfoManageWindow = new PersonalInfoManage(socket,id);

                         personalInfoManageWindow->show();

                         // 关闭登录窗口，但不立即删除
                         loginWindow->close();
                         // 使用deleteLater确保安全删除
                         loginWindow->deleteLater();
                     });

    // 显示登录窗口
    loginWindow->show();

    return a.exec();
}
