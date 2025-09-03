// PaymentWindow.h
#ifndef PAYMENTWINDOW_H
#define PAYMENTWINDOW_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QTcpSocket>
#include <QMap>

class PaymentWindow : public QWidget
{
    Q_OBJECT

public:
    PaymentWindow(QTcpSocket *existingSocket, const QString &userId, QWidget *parent = nullptr);
    ~PaymentWindow();

    // 添加方法用于添加住院待支付费用
    void addHospitalizationPayment(const QString &description, double amount, const QString &applicationId);

signals:
    void backRequested();
    void paymentSuccess(const QString &paymentType, double amount, const QString &description);

private slots:
    void onBackButtonClicked();
    void onPayButtonClicked();
    void onSelectAllClicked();
    void onItemSelectionChanged();
    void onReadyRead();

private:
    void initUI();
    void loadPaymentData();
    void updateTotalAmount();
    bool eventFilter(QObject *obj, QEvent *event) override; // 添加事件过滤器
    QString m_patientId;
    QTableWidget *paymentTable;
    QPushButton *selectAllButton;
    QPushButton *payButton;
    QPushButton *backButton;
    QLabel *totalAmountLabel;

    double m_totalAmount;

    // 存储申请ID与行号的映射
    QMap<int, QString> rowToApplicationIdMap;
    QTcpSocket *m_socket; // 添加socket成员变量用于与服务器通信
};

#endif // PAYMENTWINDOW_H
