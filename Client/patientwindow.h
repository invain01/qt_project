#ifndef PATIENTWINDOW_H
#define PATIENTWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTcpSocket>

class PaymentRecordWindow;

class PatientWindow : public QWidget
{
    Q_OBJECT

public:
    PatientWindow(QTcpSocket *existingSocket, const QString &userId, QWidget *parent = nullptr);
    ~PatientWindow();
    QTcpSocket *socket;
    QString id;

private slots:
    // 诊疗服务槽函数
    void onAppointmentClicked();
    void onDoctorListClicked();
    void onMedicineSearchClicked();
    void onHospitalizationClicked();
    void onHealthAssessmentClicked();
    
    // 个人中心槽函数
    void onPersonalInfoClicked();
    void onPaymentClicked();
    void onPaymentRecordClicked();
    void onMyDoctorClicked();
    void onAppointmentRecordClicked();
    void onPrescriptionClicked();
    void onMedicalRecordClicked();
    
    // 其他槽函数
    void onHospitalInfoClicked();
    void onDepartmentRecommendClicked();
    void onQRCodeClicked();
    void onExitClicked();

private:
    // 创建界面的方法
    void createHeaderSection(QVBoxLayout *mainLayout);
    void createServiceSection(QVBoxLayout *layout);
    void createPersonalSection(QVBoxLayout *layout);
    void createOtherSection(QVBoxLayout *layout);
    void connectSignalsSlots();
    
    QPushButton* createAppButton(const QString &text, const QString &iconText, const QString &color);
    QWidget* createAppItem(const QString &text, const QString &iconText, const QString &color, QPushButton **outButton);
    QWidget* createSectionHeader(const QString &title);
    
    // 顶部组件
    QLabel *lblPatientName;
    QLabel *lblPatientId;
    QPushButton *btnQRCode;
    
    // 诊疗服务按钮
    QPushButton *btnAppointment;
    QPushButton *btnDoctorList;
    QPushButton *btnMedicineSearch;
    QPushButton *btnHospitalization;
    QPushButton *btnHealthAssessment;
    
    // 个人中心按钮
    QPushButton *btnPersonalInfo;
    QPushButton *btnPayment;
    QPushButton *btnPaymentRecord;
    QPushButton *btnMyDoctor;
    QPushButton *btnAppointmentRecord;
    QPushButton *btnPrescription;
    QPushButton *btnMedicalRecord;
    
    // 其他按钮
    QPushButton *btnHospitalInfo;
    QPushButton *btnDepartmentRecommend;
    QPushButton *btnExit;

    // 添加成员变量保存缴费记录窗口引用
    PaymentRecordWindow *m_paymentRecordWindow;
};

#endif // PATIENTWINDOW_H
