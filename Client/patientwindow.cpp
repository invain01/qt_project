#include "patientwindow.h"
#include "AppointmentDialog.h"
#include "personalinfomanage.h"
#include "medicalrecordwindow.h"
#include "medicalrecordlistwindow.h"
#include "paymentrecordwindow.h"
#include "HealthAssessmentWindow.h"
#include "doctorlistdialog.h"
#include "doctorchatdialog.h"
#include "hospitalizationwindow.h"
#include "PaymentWindow.h"
#include "PrescriptionRecordWindow.h"
#include "AppointmentRecordWindow.h"
#include "MedicineSearchDialog.h"
#include <QPixmap>
#include <QMessageBox>
#include <QStyle>
#include <QGraphicsEffect>
#include <QFrame>
#include <QGridLayout>
#include <QScrollArea>
#include <QDateTime>
#include <QTimer>

PatientWindow::PatientWindow(QTcpSocket *existingSocket, const QString &userId, QWidget *parent)
    : QWidget(parent)
    , socket(existingSocket)
    , id(userId)
{
    // 设置窗口基本属性
    setWindowTitle("智能医疗护理系统 - 患者端");
    setFixedSize(1000, 700);
    setStyleSheet("QWidget { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                  "stop:0 #f8f9fa, stop:1 #e9ecef); }");

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(0);

    // ========== 1. 创建顶部区域 ==========
    createHeaderSection(mainLayout);

    // ========== 2. 创建滚动区域和内容区域 ==========
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    QWidget *scrollContent = new QWidget;
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 20, 0, 20);
    scrollLayout->setSpacing(30);

    // ========== 3. 创建各个应用栏 ==========
    createServiceSection(scrollLayout);    // 诊疗服务
    createPersonalSection(scrollLayout);   // 个人中心
    createOtherSection(scrollLayout);      // 其他

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

    // 连接信号槽
    connectSignalsSlots();
}

PatientWindow::~PatientWindow()
{
    // 控件由布局管理，自动释放
}

// 创建顶部区域
void PatientWindow::createHeaderSection(QVBoxLayout *mainLayout)
{
    QWidget *headerWidget = new QWidget;
    headerWidget->setFixedHeight(90);
    headerWidget->setStyleSheet("QWidget { "
                               "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                               "stop:0 #667eea, stop:1 #764ba2); "
                               "border-radius: 14px; "
                               "box-shadow: 0 6px 18px rgba(0,0,0,.12);"
                               "}");

    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(24, 16, 24, 16);

    // 左侧患者信息
    QVBoxLayout *patientInfoLayout = new QVBoxLayout;
    
    lblPatientName = new QLabel("患者姓名：加载中...");
    lblPatientName->setStyleSheet("QLabel { "
                                 "color: white; "
                                 "font-size: 18px; "
                                 "font-weight: bold; "
                                 "font-family: 'Microsoft YaHei UI'; "
                                 "}");
    
    lblPatientId = new QLabel(QString("患者ID：%1").arg(id));
    lblPatientId->setStyleSheet("QLabel { "
                               "color: rgba(255, 255, 255, 0.9); "
                               "font-size: 14px; "
                               "font-family: 'Microsoft YaHei UI'; "
                               "}");
    
    patientInfoLayout->addWidget(lblPatientName);
    patientInfoLayout->addWidget(lblPatientId);
    patientInfoLayout->addStretch();
    
    headerLayout->addLayout(patientInfoLayout);
    headerLayout->addStretch();

    // 右侧二维码按钮
    btnQRCode = new QPushButton("出示就诊二维码");
    btnQRCode->setFixedSize(150, 50);
    btnQRCode->setStyleSheet("QPushButton { "
                            "background: rgba(255, 255, 255, 0.2); "
                            "border: 2px solid rgba(255, 255, 255, 0.5); "
                            "border-radius: 25px; "
                            "color: white; "
                            "font-size: 14px; "
                            "font-weight: bold; "
                            "font-family: 'Microsoft YaHei UI'; "
                            "} "
                            "QPushButton:hover { "
                            "background: rgba(255, 255, 255, 0.3); "
                            "border: 2px solid white; "
                            "} "
                            "QPushButton:pressed { "
                            "background: rgba(255, 255, 255, 0.1); "
                            "}");
    
    headerLayout->addWidget(btnQRCode);
    mainLayout->addWidget(headerWidget);
}

// 创建应用图标按钮
QPushButton* PatientWindow::createAppButton(const QString &text, const QString &iconText, const QString &color)
{
    // 为兼容旧接口，内部委托到 createAppItem
    QPushButton *btn = nullptr;
    QWidget *item = createAppItem(text, iconText, color, &btn);
    item->deleteLater();
    return btn;
}

QWidget* PatientWindow::createAppItem(const QString &text, const QString &iconText, const QString &color, QPushButton **outButton)
{
    QWidget *container = new QWidget;
    container->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *vl = new QVBoxLayout(container);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(10);

    QPushButton *btn = new QPushButton;
    btn->setFixedSize(84, 84);
    btn->setText(iconText);
    btn->setStyleSheet(QString(
        "QPushButton {" 
        "background: %1;"
        "border: none;"
        "outline: none;"
        "border-radius: 16px;"
        "color: white;" 
        "font-size: 30px;"
        "font-weight: bold;" 
        "font-family: 'Microsoft YaHei UI';"
        "}"
        "QPushButton:focus { outline: none; border: none; }"
        "QPushButton:hover {" 
        "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %1, stop:1 rgba(255,255,255,0.15));"
        "margin-top: -2px;"
        "}"
        "QPushButton:pressed {"
        "background: %1;"
        "margin-top: 0px;"
        "}").arg(color));

    QLabel *label = new QLabel(text);
    label->setAlignment(Qt::AlignHCenter);
    label->setStyleSheet("QLabel{color:#1f2937;font-size:15px;font-family:'Microsoft YaHei UI';background:transparent;border:none;}");

    vl->addWidget(btn, 0, Qt::AlignHCenter);
    vl->addWidget(label);

    if (outButton) *outButton = btn;
    return container;
}

// 创建应用栏标题
QWidget* PatientWindow::createSectionHeader(const QString &title)
{
    QWidget *headerWidget = new QWidget;
    headerWidget->setFixedHeight(50);
    
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("QLabel { "
                             "color: #111827; "
                             "font-size: 24px; "
                             "font-weight: 800; "
                             "letter-spacing: 1px;"
                             "font-family: 'Microsoft YaHei UI'; "
                             "background: transparent;"
        "border: none;" 
                             "}");
    
    QFrame *line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("QFrame { "
                       "background: #e5e7eb; "
                       "border: none; "
                       "min-height: 3px; "
                       "max-height: 3px; "
                       "border-radius: 2px; "
                       "}");
    
    headerLayout->addWidget(titleLabel);
    headerLayout->addSpacing(12);
    headerLayout->addWidget(line, 1);
    
    return headerWidget;
}

// 创建诊疗服务区域
void PatientWindow::createServiceSection(QVBoxLayout *layout)
{
    layout->addWidget(createSectionHeader("诊疗服务"));
    
    QWidget *serviceWidget = new QWidget;
    serviceWidget->setStyleSheet("QWidget { "
                                "background: white; "
                                "border-radius: 15px; "
                                "border: 1px solid #e9ecef; "
                                "}");
    
    QGridLayout *serviceLayout = new QGridLayout(serviceWidget);
    serviceLayout->setContentsMargins(30, 30, 30, 30);
    serviceLayout->setSpacing(20);
    
    // 创建诊疗服务按钮（图标与名称分离）
    QWidget *wAppointment = createAppItem("预约挂号", "📅", "#4CAF50", &btnAppointment);
    QWidget *wDoctorList = createAppItem("医生一览", "👨‍⚕️", "#2196F3", &btnDoctorList);
    QWidget *wMedicineSearch = createAppItem("药品搜索", "💊", "#FF9800", &btnMedicineSearch);
    QWidget *wHospitalization = createAppItem("住院服务", "🏥", "#9C27B0", &btnHospitalization);
    QWidget *wHealthAssessment = createAppItem("健康评估", "📊", "#00BCD4", &btnHealthAssessment);
    
    // 第一行：5个应用，占满一行
    serviceLayout->addWidget(wAppointment, 0, 0);
    serviceLayout->addWidget(wDoctorList, 0, 1);
    serviceLayout->addWidget(wMedicineSearch, 0, 2);
    serviceLayout->addWidget(wHospitalization, 0, 3);
    serviceLayout->addWidget(wHealthAssessment, 0, 4);
    
    // 设置列均匀拉伸
    for (int i = 0; i < 5; i++) {
        serviceLayout->setColumnStretch(i, 1);
    }
    
    layout->addWidget(serviceWidget);
}

// 创建个人中心区域
void PatientWindow::createPersonalSection(QVBoxLayout *layout)
{
    layout->addWidget(createSectionHeader("个人中心"));
    
    QWidget *personalWidget = new QWidget;
    personalWidget->setStyleSheet("QWidget { "
                                 "background: white; "
                                 "border-radius: 15px; "
                                 "border: 1px solid #e9ecef; "
                                 "}");
    
    QGridLayout *personalLayout = new QGridLayout(personalWidget);
    personalLayout->setContentsMargins(30, 30, 30, 30);
    personalLayout->setSpacing(20);
    
    // 创建个人中心按钮（图标与名称分离）
    QWidget *wPersonalInfo = createAppItem("个人信息", "👤", "#6C63FF", &btnPersonalInfo);
    QWidget *wPayment = createAppItem("门诊缴费", "💳", "#FF6B6B", &btnPayment);
    QWidget *wPaymentRecord = createAppItem("缴费记录", "📋", "#4ECDC4", &btnPaymentRecord);
    QWidget *wMyDoctor = createAppItem("我的医生", "👩‍⚕️", "#45B7D1", &btnMyDoctor);
    QWidget *wAppointmentRecord = createAppItem("预约记录", "📝", "#FFA726", &btnAppointmentRecord);
    QWidget *wPrescription = createAppItem("处方查询", "💉", "#AB47BC", &btnPrescription);
    QWidget *wMedicalRecord = createAppItem("病例记录", "📄", "#26A69A", &btnMedicalRecord);
    
    // 第一行：5个应用
    personalLayout->addWidget(wPersonalInfo, 0, 0);
    personalLayout->addWidget(wPayment, 0, 1);
    personalLayout->addWidget(wPaymentRecord, 0, 2);
    personalLayout->addWidget(wMyDoctor, 0, 3);
    personalLayout->addWidget(wAppointmentRecord, 0, 4);
    
    // 第二行：3个应用（左对齐，右边空2个位置）
    personalLayout->addWidget(wPrescription, 1, 0);
    personalLayout->addWidget(wMedicalRecord, 1, 1);
    
    // 设置列均匀拉伸
    for (int i = 0; i < 5; i++) {
        personalLayout->setColumnStretch(i, 1);
    }
    
    layout->addWidget(personalWidget);
}

// 创建其他区域
void PatientWindow::createOtherSection(QVBoxLayout *layout)
{
    layout->addWidget(createSectionHeader("其他"));
    
    QWidget *otherWidget = new QWidget;
    otherWidget->setStyleSheet("QWidget { "
                              "background: white; "
                              "border-radius: 15px; "
                              "border: 1px solid #e9ecef; "
                              "}");
    
    QGridLayout *otherLayout = new QGridLayout(otherWidget);
    otherLayout->setContentsMargins(30, 30, 30, 30);
    otherLayout->setSpacing(20);
    
    // 创建其他按钮（图标与名称分离）
    QWidget *wHospitalInfo = createAppItem("医院介绍", "🏥", "#795548", &btnHospitalInfo);
    QWidget *wDepartmentRecommend = createAppItem("科室推荐", "🏢", "#607D8B", &btnDepartmentRecommend);
    QWidget *wExit = createAppItem("退出系统", "🚪", "#F44336", &btnExit);
    
    // 一行3个应用（左对齐，右边空2个位置）
    otherLayout->addWidget(wHospitalInfo, 0, 0);
    otherLayout->addWidget(wDepartmentRecommend, 0, 1);
    otherLayout->addWidget(wExit, 0, 2);
    
    // 设置列均匀拉伸
    for (int i = 0; i < 5; i++) {
        otherLayout->setColumnStretch(i, 1);
    }
    
    layout->addWidget(otherWidget);
}

// 连接信号槽
void PatientWindow::connectSignalsSlots()
{
    // 诊疗服务
    connect(btnAppointment, &QPushButton::clicked, this, &PatientWindow::onAppointmentClicked);
    connect(btnDoctorList, &QPushButton::clicked, this, &PatientWindow::onDoctorListClicked);
    connect(btnMedicineSearch, &QPushButton::clicked, this, &PatientWindow::onMedicineSearchClicked);
    connect(btnHospitalization, &QPushButton::clicked, this, &PatientWindow::onHospitalizationClicked);
    connect(btnHealthAssessment, &QPushButton::clicked, this, &PatientWindow::onHealthAssessmentClicked);
    
    // 个人中心
    connect(btnPersonalInfo, &QPushButton::clicked, this, &PatientWindow::onPersonalInfoClicked);
    connect(btnPayment, &QPushButton::clicked, this, &PatientWindow::onPaymentClicked);
    connect(btnPaymentRecord, &QPushButton::clicked, this, &PatientWindow::onPaymentRecordClicked);
    connect(btnMyDoctor, &QPushButton::clicked, this, &PatientWindow::onMyDoctorClicked);
    connect(btnAppointmentRecord, &QPushButton::clicked, this, &PatientWindow::onAppointmentRecordClicked);
    connect(btnPrescription, &QPushButton::clicked, this, &PatientWindow::onPrescriptionClicked);
    connect(btnMedicalRecord, &QPushButton::clicked, this, &PatientWindow::onMedicalRecordClicked);
    
    // 其他
    connect(btnHospitalInfo, &QPushButton::clicked, this, &PatientWindow::onHospitalInfoClicked);
    connect(btnDepartmentRecommend, &QPushButton::clicked, this, &PatientWindow::onDepartmentRecommendClicked);
    connect(btnExit, &QPushButton::clicked, this, &PatientWindow::onExitClicked);

    // 二维码按钮
    connect(btnQRCode, &QPushButton::clicked, this, &PatientWindow::onQRCodeClicked);
}

// ========== 诊疗服务槽函数 ==========
void PatientWindow::onAppointmentClicked()
{
    AppointmentDialog *appointmentDialog = new AppointmentDialog(socket, id, this);
    appointmentDialog->setAttribute(Qt::WA_DeleteOnClose);
    // 连接预约创建信号到当前窗口（如果需要广播到所有打开的预约记录窗口）
    connect(appointmentDialog, &AppointmentDialog::appointmentCreated, this, [=](const QJsonObject &newAppointment) {
        // 这里可以广播给所有打开的预约记录窗口
        // 或者通过单例模式管理预约记录窗口
        qDebug() << "新预约创建:" << newAppointment;
    });
    appointmentDialog->show();
}

void PatientWindow::onDoctorListClicked()
{
    DoctorListDialog *doctorListDialog = new DoctorListDialog(this);
    doctorListDialog->show();
}

void PatientWindow::onMedicineSearchClicked()
{
    MedicineSearchDialog *medicineSearchDialog = new MedicineSearchDialog(socket, this);
    medicineSearchDialog->setAttribute(Qt::WA_DeleteOnClose);
    medicineSearchDialog->show();
}

void PatientWindow::onHospitalizationClicked()
{
    HospitalizationWindow *hospitalizationWindow = new HospitalizationWindow(socket, id);
    connect(hospitalizationWindow, &HospitalizationWindow::backRequested, this, [=]() {
        // 可以在这里添加返回时的处理逻辑
    });
    hospitalizationWindow->setAttribute(Qt::WA_DeleteOnClose);
    hospitalizationWindow->show();
}

void PatientWindow::onHealthAssessmentClicked()
{
    HealthAssessmentWindow *healthAssessmentWindow = new HealthAssessmentWindow(socket, id);
    healthAssessmentWindow->show();
}

// ========== 个人中心槽函数 ==========
void PatientWindow::onPersonalInfoClicked()
{
    PersonalInfoManage *personalInfoManageWindow = new PersonalInfoManage(socket, id);
    personalInfoManageWindow->show();
}

void PatientWindow::onPaymentClicked()
{
    // 修改：传递socket参数给PaymentWindow
    // 创建独立顶级窗口，不指定父窗口以避免被隐藏
    PaymentWindow *paymentWindow = new PaymentWindow(socket, id);
    connect(paymentWindow, &PaymentWindow::backRequested, this, [=]() {
        this->show();
        QTimer::singleShot(100, paymentWindow, &PaymentWindow::close);
    });

    connect(paymentWindow, &PaymentWindow::paymentSuccess, this, [=](const QString &paymentType, double amount, const QString &description) {
        // 刷新缴费记录（延迟执行以避免并发问题）
        QTimer::singleShot(200, this, [=]() {
            if (m_paymentRecordWindow) {
                m_paymentRecordWindow->refreshPaymentRecords();
            }
        });
    });
    paymentWindow->setAttribute(Qt::WA_DeleteOnClose);
    paymentWindow->show();
    this->hide();
}

void PatientWindow::onPaymentRecordClicked()
{
    // 创建独立顶级窗口，不指定父窗口以避免被隐藏
    PaymentRecordWindow *paymentRecordWindow = new PaymentRecordWindow(socket, id);
    m_paymentRecordWindow = paymentRecordWindow; // 保存引用以便刷新

    connect(paymentRecordWindow, &PaymentRecordWindow::backRequested, this, [=]() {
        paymentRecordWindow->close();
        this->show();
    });

    paymentRecordWindow->setAttribute(Qt::WA_DeleteOnClose);
    paymentRecordWindow->show();
    this->hide();
}

void PatientWindow::onMyDoctorClicked()
{
    // 示例：打开与指定医生的聊天对话框
    // 这里使用示例医生ID：120001
    QString doctorId = "120001";
    QString doctorName = "张医生";
    
    DoctorChatDialog *chatDialog = new DoctorChatDialog(socket, id, doctorId, doctorName, this);
    chatDialog->setAttribute(Qt::WA_DeleteOnClose);
    chatDialog->show();
}

void PatientWindow::onAppointmentRecordClicked()
{
    AppointmentRecordWindow *appointmentRecordWindow = new AppointmentRecordWindow(socket, id);

    // 连接返回信号
    connect(appointmentRecordWindow, &AppointmentRecordWindow::backRequested, this, [=]() {
        appointmentRecordWindow->close();
        this->show();
    });

    appointmentRecordWindow->setAttribute(Qt::WA_DeleteOnClose);
    appointmentRecordWindow->show();
    this->hide();
}

void PatientWindow::onPrescriptionClicked()
{
    PrescriptionRecordWindow *prescriptionWindow = new PrescriptionRecordWindow(socket, id);
    connect(prescriptionWindow, &PrescriptionRecordWindow::backRequested, this, [=]() {
        // 可以在这里添加返回时的处理逻辑
    });
    prescriptionWindow->setAttribute(Qt::WA_DeleteOnClose);
    prescriptionWindow->show();
}

void PatientWindow::onMedicalRecordClicked()
{
    MedicalRecordListWindow *medicalRecordListWindow = new MedicalRecordListWindow(id);
    connect(medicalRecordListWindow, &MedicalRecordListWindow::backRequested, this, [=]() {
        // 可以在这里添加返回时的处理逻辑
    });
    medicalRecordListWindow->show();
}

// ========== 其他槽函数 ==========
void PatientWindow::onHospitalInfoClicked()
{
    QMessageBox::information(this, "医院介绍", 
        "智能医疗护理系统\n\n"
        "📍 地址：医疗科技园区\n"
        "📞 电话：400-123-4567\n"
        "🌐 网址：www.smartmedical.com\n\n"
        "我们致力于为患者提供最优质的医疗服务！");
}

void PatientWindow::onDepartmentRecommendClicked()
{
    QMessageBox::information(this, "科室推荐", 
        "热门科室推荐：\n\n"
        "🩺 内科 - 常见疾病诊疗\n"
        "🦴 外科 - 手术及创伤治疗\n"
        "👶 儿科 - 儿童专科医疗\n"
        "👩‍⚕️ 妇科 - 妇女健康服务\n"
        "🧠 神经科 - 神经系统疾病\n"
        "❤️ 心血管科 - 心脏疾病专科");
}

void PatientWindow::onQRCodeClicked()
{
    QMessageBox::information(this, "就诊二维码", 
        QString("患者就诊二维码\n\n"
               "患者ID：%1\n"
               "生成时间：%2\n\n"
               "请在就诊时出示此信息").arg(id).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
}

void PatientWindow::onExitClicked()
{
    close();
}