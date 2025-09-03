#ifndef PAYMENTRECORDWINDOW_H
#define PAYMENTRECORDWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDate>
#include <QString>
#include <QTcpSocket>

class PaymentRecordWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PaymentRecordWindow(QTcpSocket *socket, const QString &patientId, QWidget *parent = nullptr);
    ~PaymentRecordWindow();

signals:
    void backRequested();

    public slots:
    void refreshPaymentRecords();

private slots:
    void onBackButtonClicked();
    void onReadyRead();

private:
    QString patientId;  // 患者ID
    QTableWidget *paymentTable;  // 缴费记录表格
    QPushButton *backButton;     // 返回按钮
    QTcpSocket *m_socket;        // 添加socket成员变量用于与服务器通信

    void initUI();      // 初始化界面
    void loadPaymentData();  // 加载缴费记录数据
    void handleServerResponse(const QByteArray &response); // 处理服务器响应
};

#endif // PAYMENTRECORDWINDOW_H
