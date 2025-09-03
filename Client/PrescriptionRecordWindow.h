// PrescriptionRecordWindow.h
#ifndef PRESCRIPTIONRECORDWINDOW_H
#define PRESCRIPTIONRECORDWINDOW_H

#include <QWidget>
#include <QDialog>
#include <QTcpSocket>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QJsonObject>
#include <QJsonArray>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>

// 处方详情对话框
class PrescriptionDetailDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PrescriptionDetailDialog(const QJsonObject &prescriptionData, QTcpSocket *socket, const QString &patientId, QWidget *parent = nullptr);

private slots:
    void onReadyRead();          // 处理服务器响应

private:
    void initUI();
    void loadPaymentInfo();      // 加载缴费信息（仅查看）
    void handleServerResponse(const QByteArray &response);

    // UI组件
    QLabel *prescriptionIdLabel;
    QLabel *prescriptionDateLabel;
    QLabel *doctorNameLabel;
    QLabel *doctorDepartmentLabel;
    QLabel *medicineNameLabel;
    QLabel *dosageLabel;
    QLabel *usageLabel;
    QLabel *frequencyLabel;
    QLabel *quantityLabel;        // 购买数量标签
    QLabel *costLabel;           // 费用标签
    QLabel *paymentStatusLabel;  // 支付状态标签
    QLabel *statusLabel;
    QTextEdit *notesTextEdit;
    
    // 数据成员
    QJsonObject m_prescriptionData;
    QTcpSocket *m_socket;
    QString m_patientId;
    QString m_prescriptionRef;   // 处方引用ID
};

class PrescriptionRecordWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PrescriptionRecordWindow(QTcpSocket *socket, const QString &patientId, QWidget *parent = nullptr);
    ~PrescriptionRecordWindow();

signals:
    void backRequested();

private slots:
    void onBackButtonClicked();
    void onReadyRead();
    void onRowDoubleClicked(QTableWidgetItem* item);

private:
    void initUI();
    void loadPrescriptionData();
    void handleServerResponse(const QByteArray &response);

    QTcpSocket *m_socket;
    QString m_patientId;
    QJsonArray m_prescriptionData; // 存储处方数据用于详情查看

    QTableWidget *prescriptionTable;
    QPushButton *backButton;
};

#endif // PRESCRIPTIONRECORDWINDOW_H
