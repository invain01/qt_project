#ifndef REGMANAGE_H
#define REGMANAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QTcpSocket>
#include <vector>

// 患者信息结构体
struct Patient {
    QString id;          // 患者ID
    QString name;        // 患者姓名
    QString time;        // 预约时间
    QString status;      // 预约状态
    QString gender;      // 性别
    int age;             // 年龄
    QString dept;        // 科室
    QString symptom;     // 症状
    QString phone;       // 电话
};

class RegManage : public QWidget
{
    Q_OBJECT

public:
    RegManage(QTcpSocket *socket, const QString &doctorId, QWidget *parent = nullptr);
    ~RegManage();

signals:
    void backRequested(); // 返回信号

private:
    QTableWidget *table;       // 患者信息表格
    QPushButton *backBtn;      // 返回按钮
    QTcpSocket *m_socket;      // 网络套接字
    QString m_doctorId;        // 医生ID
    std::vector<Patient> data; // 患者数据

    void initUI();             // 初始化界面
    void loadData();           // 加载患者数据
    void sendRequestToServer(const QString &message); // 发送请求到服务器
    void handleServerResponse(const QByteArray &response); // 处理服务器响应

private slots:
    void onBack();             // 返回按钮点击事件
    void onDetail(int row, int col); // 详情按钮点击事件
    void onReadyRead();        // 接收服务器数据
};

// 患者详情对话框
class DetailDialog : public QWidget
{
    Q_OBJECT

public:
    DetailDialog(Patient p, QWidget *parent = nullptr);

private:
    QPushButton *closeBtn;     // 关闭按钮
};

#endif // REGMANAGE_H
