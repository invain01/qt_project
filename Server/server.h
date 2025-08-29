#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMap>
#include <QFile>
#include <QFileInfo>

class Server : public QObject
{
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);
    void start(); // 启动服务器

private slots:
    void handleNewConnection(); // 处理新客户端连接
    void handleClientData();    // 处理客户端数据
    void handleDisconnection(); // 处理客户端断开连接
    void handleLogIn(QString message, QTcpSocket *clientSocket);  //处理登录
    void handleContactListRequest(QTcpSocket *clientSocket); //处理联系人请求

private:
    void initializeDatabase(); // 初始化数据库
    void sendFileToClient(QTcpSocket *clientSocket, const QString &filePath); // 发送文件到客户端
    void getContactList();

    QTcpServer *m_server; // TCP服务器对象
    QSqlDatabase m_db;    // 数据库连接
    QMap<QTcpSocket*, QString> m_connectedClients; // 存储已连接客户端及其用户名
     void handleUserInfoRequest(const QString &userId, QTcpSocket *clientSocket);//处理个人信息请求
};

#endif // SERVER_H
