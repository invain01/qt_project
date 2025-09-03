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
#include <QDir>
#include <QCoreApplication>
#include <QThread>

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

private:
    void initializeDatabase(); // 初始化数据库
    void sendFileToClient(QTcpSocket *clientSocket, const QString &filePath); // 发送文件到客户端
    void getContactList();
    QString generateUserId(const QString &identity); // 生成用户ID
    void handleRegister(const QString &message, QTcpSocket *clientSocket); // 处理注册

    QTcpServer *m_server; // TCP服务器对象
    QSqlDatabase m_db;    // 数据库连接
    QMap<QTcpSocket*, QString> m_connectedClients; // 存储已连接客户端及其用户名
    void handleUserInfoRequest(const QString &userId, QTcpSocket *clientSocket);//处理个人信息请求
    void handleSaveUserInfo(const QString &message, QTcpSocket *clientSocket);//保存个人信息

    void handleAppointmentsRequest(const QString &message, QTcpSocket *clientSocket);
    void handleProcessAppointment(const QString &message, QTcpSocket *clientSocket);
    
    // 考勤管理相关函数
    void handleCheckIn(const QString &message, QTcpSocket *clientSocket);
    void handleCheckOut(const QString &message, QTcpSocket *clientSocket);
    void handleAttendanceHistory(const QString &message, QTcpSocket *clientSocket);
    void handleLeaveApplication(const QString &message, QTcpSocket *clientSocket);
    void handleLeaveRecordsRequest(const QString &message, QTcpSocket *clientSocket);
    void handleReturnFromLeave(const QString &message, QTcpSocket *clientSocket);
    
    // 医患沟通相关函数
    void handleSendMessage(const QString &message, QTcpSocket *clientSocket);
    void handleSendImage(const QString &message, QTcpSocket *clientSocket);
    void handleGetChatHistory(const QString &message, QTcpSocket *clientSocket);
    void handleGetContactList(const QString &message, QTcpSocket *clientSocket);
    void broadcastMessage(const QString &receiverId, const QString &messageData);

    // 图片拉取
    void handleGetImage(const QString &message, QTcpSocket *clientSocket);


    // 声明药品搜索处理函数
    void handleMedicineSearch(const QString &message, QTcpSocket *clientSocket);
    
    // 视频通话相关函数
    void handleVideoCallRequest(const QString &message, QTcpSocket *clientSocket);
    void handleVideoCallResponse(const QString &message, QTcpSocket *clientSocket);
    void handleVideoCallEnd(const QString &message, QTcpSocket *clientSocket);

    //住院缴费
    void handleHospitalizationApply(const QString &message, QTcpSocket *clientSocket);
    void handleGetHospitalization(const QString &message, QTcpSocket *clientSocket);
    void handleAddPaymentItem(const QString &message, QTcpSocket *clientSocket);
    void handleGetPaymentItems(const QString &message, QTcpSocket *clientSocket);
    void handleProcessPayment(const QString &message, QTcpSocket *clientSocket);
    void handleGetPaymentRecords(const QString &message, QTcpSocket *clientSocket);

    //预约挂号
    void handleGetDoctorSchedule(QTcpSocket *clientSocket);
    void handleMakeAppointment(const QString &message, QTcpSocket *clientSocket);
    void handleGetUserAppointments(const QString &message, QTcpSocket *clientSocket);

    // 处方管理相关函数
    void handleSubmitPrescription(const QString &message, QTcpSocket *clientSocket);
    void handleGetPatientPrescriptions(const QString &message, QTcpSocket *clientSocket);
};

#endif // SERVER_H
