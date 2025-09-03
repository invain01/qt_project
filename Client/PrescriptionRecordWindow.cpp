// PrescriptionRecordWindow.cpp
#include "PrescriptionRecordWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPushButton>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QTextEdit>
#include <QBrush>
#include <QTableWidgetItem>

PrescriptionRecordWindow::PrescriptionRecordWindow(QTcpSocket *socket, const QString &patientId, QWidget *parent)
    : QWidget(parent), m_socket(socket), m_patientId(patientId)
{
    setWindowTitle("处方记录");
    setMinimumSize(900, 600);
    setStyleSheet("background-color: #f8f9fa;");

    // 连接信号槽
    if (m_socket) {
        disconnect(m_socket, &QTcpSocket::readyRead, nullptr, nullptr);
        connect(m_socket, &QTcpSocket::readyRead, this, &PrescriptionRecordWindow::onReadyRead);
    }

    initUI();
    loadPrescriptionData();
}

void PrescriptionRecordWindow::initUI()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(30);

    // 标题
    QLabel *titleLabel = new QLabel("处方记录");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: black; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    // 患者信息
    QLabel *patientInfoLabel = new QLabel(QString("患者ID：%1").arg(m_patientId));
    patientInfoLabel->setAlignment(Qt::AlignCenter);
    patientInfoLabel->setStyleSheet("color: black; font-size: 14px;");
    mainLayout->addWidget(patientInfoLabel);

    // 创建表格
    prescriptionTable = new QTableWidget();
    prescriptionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    prescriptionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    prescriptionTable->setAlternatingRowColors(true);
    prescriptionTable->setFocusPolicy(Qt::NoFocus);

    // 设置表格列数和表头
    QStringList headers;
    headers << "处方号" << "开具时间" << "开具医生" << "药品名称" << "用法用量" << "购买数量" << "状态"; // 添加购买数量列
    prescriptionTable->setColumnCount(headers.size());
    prescriptionTable->setHorizontalHeaderLabels(headers);
    
    // 连接双击事件
    connect(prescriptionTable, &QTableWidget::itemDoubleClicked, this, &PrescriptionRecordWindow::onRowDoubleClicked);

    // 设置表格样式
    prescriptionTable->setStyleSheet(
        "QTableWidget { color: black; font-size: 14px; background-color: white; border: 1px solid #dee2e6; }"
        "QTableWidget::item { padding: 10px; border: 1px solid #dee2e6; }"
        "QTableWidget::item:selected { background-color: #e3f2fd; color: black; }"
        "QTableWidget::alternating-row-color { background-color: #fafafa; }"
        );

    // 设置表头样式
    prescriptionTable->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { "
        "background-color: #e9ecef; "
        "color: black; "
        "font-weight: bold; "
        "font-size: 14px; "
        "padding: 10px; "
        "border: 1px solid #dee2e6; "
        "}"
        );

    // 自动调整列宽
    prescriptionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 隐藏垂直表头
    prescriptionTable->verticalHeader()->setVisible(false);

    // 添加表格到主布局
    mainLayout->addWidget(prescriptionTable, 1);
    
    // 提示标签
    QLabel *hintLabel = new QLabel("💡 提示：双击任意处方行可查看详细信息");
    hintLabel->setAlignment(Qt::AlignCenter);
    hintLabel->setStyleSheet("color: #6c757d; font-size: 14px; font-style: italic; margin: 10px;");
    mainLayout->addWidget(hintLabel);

    // 底部按钮布局
    QHBoxLayout *bottomLayout = new QHBoxLayout();

    // 返回按钮
    backButton = new QPushButton("返回");
    backButton->setStyleSheet(
        "QPushButton { background-color: #007bff; color: white; border: none; padding: 12px 24px; "
        "border-radius: 4px; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #0056b3; }"
        );
    connect(backButton, &QPushButton::clicked, this, &PrescriptionRecordWindow::onBackButtonClicked);

    bottomLayout->addStretch();
    bottomLayout->addWidget(backButton);
    mainLayout->addLayout(bottomLayout);
}

void PrescriptionRecordWindow::loadPrescriptionData()
{
    // 发送请求获取处方数据
    QString request = QString("GET_PATIENT_PRESCRIPTIONS#%1").arg(m_patientId);
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(request.toUtf8() + "\n");
        m_socket->flush();
    }

    // 临时显示加载中提示
    prescriptionTable->setRowCount(1);
    QTableWidgetItem *loadingItem = new QTableWidgetItem("正在加载处方数据...");
    loadingItem->setTextAlignment(Qt::AlignCenter);
    prescriptionTable->setItem(0, 0, loadingItem);
    prescriptionTable->setSpan(0, 0, 1, prescriptionTable->columnCount());
}

void PrescriptionRecordWindow::onBackButtonClicked()
{
    emit backRequested();
    this->close();
}

void PrescriptionRecordWindow::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    handleServerResponse(data);
}

void PrescriptionRecordWindow::handleServerResponse(const QByteArray &response)
{
    QString responseStr = QString::fromUtf8(response);

    if (responseStr.startsWith("PRESCRIPTION_LIST_SUCCESS")) {
        // 解析处方数据
        QString jsonStr = responseStr.section('#', 1);
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());

        if (!doc.isNull() && doc.isArray()) {
            m_prescriptionData = doc.array(); // 保存处方数据

            // 清空表格
            prescriptionTable->setRowCount(0);

            // 填充处方数据
            prescriptionTable->setRowCount(m_prescriptionData.size());
            for (int i = 0; i < m_prescriptionData.size(); ++i) {
                QJsonObject record = m_prescriptionData[i].toObject();

                prescriptionTable->setItem(i, 0, new QTableWidgetItem(QString::number(record["prescription_id"].toInt())));
                prescriptionTable->setItem(i, 1, new QTableWidgetItem(record["prescribed_date"].toString()));
                prescriptionTable->setItem(i, 2, new QTableWidgetItem(record["doctor_name"].toString() + " (" + record["department"].toString() + ")"));
                prescriptionTable->setItem(i, 3, new QTableWidgetItem(record["medicine_name"].toString()));
                prescriptionTable->setItem(i, 4, new QTableWidgetItem(record["dosage"].toString() + " | " + record["usage"].toString() + " | " + record["frequency"].toString()));
                prescriptionTable->setItem(i, 5, new QTableWidgetItem(QString::number(record["quantity"].toInt()))); // 购买数量列

                // 状态列
                QString status = record["status"].toString();
                QTableWidgetItem *statusItem = new QTableWidgetItem();
                if (status == "completed") {
                    statusItem->setText("已完成");
                    statusItem->setForeground(QBrush(QColor("#28a745"))); // 绿色
                } else if (status == "active") {
                    statusItem->setText("进行中");
                    statusItem->setForeground(QBrush(QColor("#007bff"))); // 蓝色
                } else if (status == "cancelled") {
                    statusItem->setText("已取消");
                    statusItem->setForeground(QBrush(QColor("#dc3545"))); // 红色
                } else {
                    statusItem->setText(status);
                }
                prescriptionTable->setItem(i, 6, statusItem);  // 状态列现在是第6列

                // 设置所有单元格文本居中
                for (int col = 0; col < prescriptionTable->columnCount(); ++col) {
                    if (prescriptionTable->item(i, col)) {
                        prescriptionTable->item(i, col)->setTextAlignment(Qt::AlignCenter);
                    }
                }
            }
            
            // 调整列宽
            prescriptionTable->resizeColumnsToContents();
            prescriptionTable->horizontalHeader()->setStretchLastSection(true);
        }
    } else if (responseStr.startsWith("PRESCRIPTION_LIST_FAIL")) {
        QMessageBox::warning(this, "加载失败", "获取处方数据失败");
        
        // 清空表格并显示错误信息
        prescriptionTable->setRowCount(1);
        QTableWidgetItem *errorItem = new QTableWidgetItem("暂无处方数据");
        errorItem->setTextAlignment(Qt::AlignCenter);
        prescriptionTable->setItem(0, 0, errorItem);
        prescriptionTable->setSpan(0, 0, 1, prescriptionTable->columnCount());
    }
}

PrescriptionRecordWindow::~PrescriptionRecordWindow()
{
    // socket 由父对象管理，不需要手动删除
}

void PrescriptionRecordWindow::onRowDoubleClicked(QTableWidgetItem* item)
{
    if (!item) return;
    
    int row = item->row();
    if (row >= 0 && row < m_prescriptionData.size()) {
        QJsonObject prescriptionData = m_prescriptionData[row].toObject();
        
        // 创建并显示处方详情对话框，传递socket和患者ID
        PrescriptionDetailDialog *dialog = new PrescriptionDetailDialog(prescriptionData, m_socket, m_patientId, this);
        dialog->exec();
        dialog->deleteLater();
    }
}

// PrescriptionDetailDialog implementation
PrescriptionDetailDialog::PrescriptionDetailDialog(const QJsonObject &prescriptionData, QTcpSocket *socket, const QString &patientId, QWidget *parent)
    : QDialog(parent), m_prescriptionData(prescriptionData), m_socket(socket), m_patientId(patientId)
{
    m_prescriptionRef = QString("PRESC_%1").arg(prescriptionData["prescription_id"].toInt());
    
    // 连接信号槽处理服务器响应
    if (m_socket) {
        connect(m_socket, &QTcpSocket::readyRead, this, &PrescriptionDetailDialog::onReadyRead);
    }
    
    initUI();
    
    // 直接在构造函数中填充数据
    prescriptionIdLabel->setText(QString::number(prescriptionData["prescription_id"].toInt()));
    prescriptionDateLabel->setText(prescriptionData["prescribed_date"].toString());
    doctorNameLabel->setText(prescriptionData["doctor_name"].toString());
    doctorDepartmentLabel->setText(prescriptionData["department"].toString());
    
    medicineNameLabel->setText(prescriptionData["medicine_name"].toString());
    dosageLabel->setText(prescriptionData["dosage"].toString());
    usageLabel->setText(prescriptionData["usage"].toString());
    frequencyLabel->setText(prescriptionData["frequency"].toString());
    quantityLabel->setText(QString::number(prescriptionData["quantity"].toInt()));  // 显示购买数量
    
    // 设置状态显示
    QString status = prescriptionData["status"].toString();
    if (status == "completed") {
        statusLabel->setText("已完成");
        statusLabel->setStyleSheet("color: #27ae60; font-weight: bold;");
    } else if (status == "active") {
        statusLabel->setText("进行中");
        statusLabel->setStyleSheet("color: #3498db; font-weight: bold;");
    } else if (status == "cancelled") {
        statusLabel->setText("已取消");
        statusLabel->setStyleSheet("color: #e74c3c; font-weight: bold;");
    } else {
        statusLabel->setText(status);
        statusLabel->setStyleSheet("color: #34495e; font-weight: bold;");
    }
    
    notesTextEdit->setText(prescriptionData["notes"].toString());
    
    // 加载缴费信息
    loadPaymentInfo();
}

void PrescriptionDetailDialog::initUI()
{
    setWindowTitle("处方详情");
    setFixedSize(500, 600);
    setModal(true);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 标题
    QLabel *titleLabel = new QLabel("处方详细信息");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2c3e50; margin-bottom: 20px;");
    mainLayout->addWidget(titleLabel);

    // 创建滚动区域
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(10);

    // 基本信息区域
    QGroupBox *basicGroup = new QGroupBox("基本信息");
    basicGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #34495e; }");
    QGridLayout *basicLayout = new QGridLayout(basicGroup);

    prescriptionIdLabel = new QLabel();
    prescriptionDateLabel = new QLabel();
    doctorNameLabel = new QLabel();
    doctorDepartmentLabel = new QLabel();

    basicLayout->addWidget(new QLabel("处方编号:"), 0, 0);
    basicLayout->addWidget(prescriptionIdLabel, 0, 1);
    basicLayout->addWidget(new QLabel("开方日期:"), 1, 0);
    basicLayout->addWidget(prescriptionDateLabel, 1, 1);
    basicLayout->addWidget(new QLabel("医生姓名:"), 2, 0);
    basicLayout->addWidget(doctorNameLabel, 2, 1);
    basicLayout->addWidget(new QLabel("科室:"), 3, 0);
    basicLayout->addWidget(doctorDepartmentLabel, 3, 1);

    contentLayout->addWidget(basicGroup);

    // 药品信息区域
    QGroupBox *medicineGroup = new QGroupBox("药品信息");
    medicineGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #34495e; }");
    QGridLayout *medicineLayout = new QGridLayout(medicineGroup);

    medicineNameLabel = new QLabel();
    dosageLabel = new QLabel();
    usageLabel = new QLabel();
    frequencyLabel = new QLabel();
    quantityLabel = new QLabel();  // 初始化购买数量标签
    costLabel = new QLabel();      // 初始化费用标签

    medicineLayout->addWidget(new QLabel("药品名称:"), 0, 0);
    medicineLayout->addWidget(medicineNameLabel, 0, 1);
    medicineLayout->addWidget(new QLabel("剂量:"), 1, 0);
    medicineLayout->addWidget(dosageLabel, 1, 1);
    medicineLayout->addWidget(new QLabel("用法:"), 2, 0);
    medicineLayout->addWidget(usageLabel, 2, 1);
    medicineLayout->addWidget(new QLabel("频次:"), 3, 0);
    medicineLayout->addWidget(frequencyLabel, 3, 1);
    medicineLayout->addWidget(new QLabel("购买数量:"), 4, 0);
    medicineLayout->addWidget(quantityLabel, 4, 1);
    medicineLayout->addWidget(new QLabel("费用:"), 5, 0);
    medicineLayout->addWidget(costLabel, 5, 1);

    contentLayout->addWidget(medicineGroup);

    // 状态信息区域
    QGroupBox *statusGroup = new QGroupBox("状态信息");
    statusGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #34495e; }");
    QGridLayout *statusLayout = new QGridLayout(statusGroup);

    statusLabel = new QLabel();
    paymentStatusLabel = new QLabel();
    statusLayout->addWidget(new QLabel("处方状态:"), 0, 0);
    statusLayout->addWidget(statusLabel, 0, 1);
    statusLayout->addWidget(new QLabel("支付状态:"), 1, 0);
    statusLayout->addWidget(paymentStatusLabel, 1, 1);

    contentLayout->addWidget(statusGroup);

    // 备注信息区域
    QGroupBox *notesGroup = new QGroupBox("备注信息");
    notesGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #34495e; }");
    QVBoxLayout *notesLayout = new QVBoxLayout(notesGroup);

    notesTextEdit = new QTextEdit();
    notesTextEdit->setMaximumHeight(100);
    notesTextEdit->setReadOnly(true);
    notesTextEdit->setStyleSheet("background-color: #ecf0f1; border: 1px solid #bdc3c7; padding: 5px;");
    notesLayout->addWidget(notesTextEdit);

    contentLayout->addWidget(notesGroup);

    scrollArea->setWidget(contentWidget);
    scrollArea->setWidgetResizable(true);
    mainLayout->addWidget(scrollArea);

    // 只保留关闭按钮
    QPushButton *closeButton = new QPushButton("关闭");
    closeButton->setStyleSheet("QPushButton { "
        "background-color: #95a5a6; "
        "color: white; "
        "border: none; "
        "padding: 10px 20px; "
        "border-radius: 5px; "
        "font-size: 14px; "
        "} "
        "QPushButton:hover { "
        "background-color: #7f8c8d; "
        "}");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
}

// 加载缴费信息（仅查看，不支付）
void PrescriptionDetailDialog::loadPaymentInfo()
{
    if (!m_socket || m_socket->state() != QTcpSocket::ConnectedState) {
        costLabel->setText("无法获取费用信息");
        paymentStatusLabel->setText("网络连接错误");
        return;
    }
    
    // 发送请求获取特定处方的缴费信息
    QString request = QString("GET_PAYMENT_ITEMS#%1").arg(m_patientId);
    m_socket->write(request.toUtf8() + "\n");
    m_socket->flush();
}

// 处理服务器响应
void PrescriptionDetailDialog::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    handleServerResponse(data);
}

// 处理服务器响应
void PrescriptionDetailDialog::handleServerResponse(const QByteArray &response)
{
    QString responseStr = QString::fromUtf8(response);
    qDebug() << "处方详情收到服务器响应:" << responseStr;
    
    if (responseStr.startsWith("GET_PAYMENT_ITEMS_SUCCESS")) {
        // 解析缴费项目数据
        QString jsonStr = responseStr.section('#', 1);
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        
        if (!doc.isNull() && doc.isArray()) {
            QJsonArray paymentItems = doc.array();
            
            // 查找与当前处方相关的缴费项目
            for (const QJsonValue &value : paymentItems) {
                QJsonObject item = value.toObject();
                QString applicationId = item["application_id"].toString();
                
                if (applicationId == m_prescriptionRef) {
                    // 找到匹配的缴费项目
                    double amount = item["amount"].toDouble();
                    QString status = item["status"].toString();
                    
                    costLabel->setText(QString("¥%1").arg(amount, 0, 'f', 2));
                    
                    if (status == "pending") {
                        paymentStatusLabel->setText("待支付");
                        paymentStatusLabel->setStyleSheet("color: #e74c3c; font-weight: bold;");
                    } else if (status == "paid") {
                        paymentStatusLabel->setText("已支付");
                        paymentStatusLabel->setStyleSheet("color: #27ae60; font-weight: bold;");
                    } else {
                        paymentStatusLabel->setText(status);
                        paymentStatusLabel->setStyleSheet("color: #34495e; font-weight: bold;");
                    }
                    return;
                }
            }
            
            // 未找到匹配的缴费项目
            costLabel->setText("费用信息暂未生成");
            paymentStatusLabel->setText("无需支付");
            paymentStatusLabel->setStyleSheet("color: #95a5a6; font-weight: bold;");
        }
    }
}
