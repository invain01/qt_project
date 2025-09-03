#include "regmanage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollArea>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QColor>

// 主界面构造函数
RegManage::RegManage(QTcpSocket *socket, const QString &doctorId, QWidget *parent)
    : QWidget(parent), m_socket(socket), m_doctorId(doctorId)
{
    setWindowTitle("挂号管理");
    setMinimumSize(800, 600);
    
    // 连接信号槽 - 先断开之前的连接，避免信号被多个对象处理
    if (m_socket) {
        disconnect(m_socket, &QTcpSocket::readyRead, nullptr, nullptr);
        connect(m_socket, &QTcpSocket::readyRead, this, &RegManage::onReadyRead);
    }
    
    initUI();
    loadData();
}

// 析构函数
RegManage::~RegManage()
{
}

// 初始化界面
void RegManage::initUI()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 表格设置
    table = new QTableWidget();
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels(QStringList() << "患者ID" << "患者姓名" << "预约时间" << "预约状态" << "操作");
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers); // 设置表格为只读

    // 连接表格点击信号
    connect(table, &QTableWidget::cellClicked, this, &RegManage::onDetail);

    // 返回按钮（改为蓝色样式）
    backBtn = new QPushButton("返回上一级");
    backBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #0d6efd; /* 蓝色主色调 */
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 12px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #0b5ed7; /* 悬停深色 */
        }
        QPushButton:pressed {
            background-color: #0a53be; /* 点击更深色 */
        }
    )");
    connect(backBtn, &QPushButton::clicked, this, &RegManage::onBack);

    // 底部布局
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    bottomLayout->addWidget(backBtn);

    // 添加到主布局
    mainLayout->addWidget(table);
    mainLayout->addLayout(bottomLayout);
}

// 加载患者数据
void RegManage::loadData()
{
    // 清空现有数据
    data.clear();
    table->setRowCount(0);
    
    // 发送请求获取医生的预约信息
    QString request = "APPOINTMENTS#" + m_doctorId;
    sendRequestToServer(request);
    
    // 显示加载提示
    table->setRowCount(1);
    QTableWidgetItem *loadingItem = new QTableWidgetItem("正在加载预约数据...");
    loadingItem->setTextAlignment(Qt::AlignCenter);
    table->setItem(0, 0, loadingItem);
    table->setSpan(0, 0, 1, 5);
}

// 发送请求到服务器
void RegManage::sendRequestToServer(const QString &message)
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        // 添加换行符分隔消息，避免TCP粘包
        QByteArray data = message.toUtf8() + "\n";
        m_socket->write(data);
        qDebug() << "发送请求到服务器:" << message;
    } else {
        QMessageBox::warning(this, "网络错误", "未连接到服务器");
    }
}

// 处理服务器响应
void RegManage::handleServerResponse(const QByteArray &response)
{
    QString responseStr = QString::fromUtf8(response);
    qDebug() << "收到服务器响应:" << responseStr;
    
    if (responseStr.startsWith("APPOINTMENTS_SUCCESS")) {
        // 解析预约数据
        QString jsonStr = responseStr.section('#', 1);
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        
        if (!doc.isNull() && doc.isArray()) {
            QJsonArray appointments = doc.array();
            
            // 清空表格
            table->setRowCount(0);
            data.clear();
            
            // 填充预约数据
            for (const QJsonValue &value : appointments) {
                QJsonObject appointment = value.toObject();
                
                Patient patient;
                patient.id = appointment["patient_id"].toString();
                patient.name = appointment["patient_name"].toString();
                patient.time = appointment["appointment_date"].toString();
                patient.status = appointment["status"].toString();
                patient.gender = appointment["gender"].toString();
                patient.age = appointment["age"].toInt();
                patient.dept = appointment["department"].toString();
                patient.symptom = appointment["symptom"].toString();
                patient.phone = appointment["phone"].toString();
                
                data.push_back(patient);
            }
            
            // 更新表格显示
            table->setRowCount(data.size());
            for (int i = 0; i < data.size(); ++i) {
                table->setItem(i, 0, new QTableWidgetItem(data[i].id));
                table->setItem(i, 1, new QTableWidgetItem(data[i].name));
                table->setItem(i, 2, new QTableWidgetItem(data[i].time));
                
                // 预约状态列 - 本地化显示
                QString statusText;
                QColor statusColor;
                
                if (data[i].status == "pending") {
                    statusText = "待处理";
                    statusColor = QColor("#ffc107"); // 黄色
                } else if (data[i].status == "confirmed") {
                    statusText = "已确认";
                    statusColor = QColor("#28a745"); // 绿色
                } else if (data[i].status == "cancelled") {
                    statusText = "已取消";
                    statusColor = QColor("#dc3545"); // 红色
                } else {
                    statusText = data[i].status;
                    statusColor = QColor("#6c757d"); // 灰色
                }
                
                QTableWidgetItem *statusItem = new QTableWidgetItem(statusText);
                statusItem->setForeground(QBrush(statusColor));
                statusItem->setTextAlignment(Qt::AlignCenter);
                table->setItem(i, 3, statusItem);

                // 操作列容器
                QWidget *operationWidget = new QWidget();
                QHBoxLayout *operationLayout = new QHBoxLayout(operationWidget);
                operationLayout->setContentsMargins(5, 2, 5, 2);
                operationLayout->setSpacing(5);

                // 查看患者信息按钮（蓝色样式）
                QPushButton *viewInfoBtn = new QPushButton("查看信息");
                viewInfoBtn->setStyleSheet(R"(
                    QPushButton {
                        background-color: #0d6efd; /* 蓝色主色调 */
                        color: white;
                        border: none;
                        border-radius: 4px;
                        padding: 4px 8px;
                        font-size: 12px;
                        min-width: 60px;
                    }
                    QPushButton:hover {
                        background-color: #0b5ed7; /* 悬停深色 */
                    }
                    QPushButton:pressed {
                        background-color: #0a53be; /* 点击更深色 */
                    }
                )");
                connect(viewInfoBtn, &QPushButton::clicked, [this, i]() {
                    // 显示详情对话框
                    DetailDialog *dialog = new DetailDialog(data[i], this);
                    dialog->show();
                });

                // 处理预约申请按钮（绿色样式）
                QPushButton *processAppointmentBtn = new QPushButton("处理预约");
                processAppointmentBtn->setStyleSheet(R"(
                    QPushButton {
                        background-color: #198754; /* 绿色主色调 */
                        color: white;
                        border: none;
                        border-radius: 4px;
                        padding: 4px 8px;
                        font-size: 12px;
                        min-width: 60px;
                    }
                    QPushButton:hover {
                        background-color: #157347; /* 悬停深色 */
                    }
                    QPushButton:pressed {
                        background-color: #146c43; /* 点击更深色 */
                    }
                )");
                connect(processAppointmentBtn, &QPushButton::clicked, [this, i]() {
                    // 处理预约申请逻辑
                    QString request = "PROCESS_APPOINTMENT#" + data[i].id + "#" + m_doctorId + "#confirmed";
                    sendRequestToServer(request);
                });

                operationLayout->addWidget(viewInfoBtn);
                operationLayout->addWidget(processAppointmentBtn);
                operationWidget->setLayout(operationLayout);
                table->setCellWidget(i, 4, operationWidget);
            }
        }
    } else if (responseStr.startsWith("APPOINTMENTS_FAIL")) {
        QMessageBox::warning(this, "加载失败", "获取预约数据失败");
    } else if (responseStr.startsWith("PROCESS_APPOINTMENT_SUCCESS")) {
        QMessageBox::information(this, "成功", "预约处理成功");
        // 重新加载数据
        loadData();
    } else if (responseStr.startsWith("PROCESS_APPOINTMENT_FAIL")) {
        QMessageBox::warning(this, "失败", "预约处理失败");
    }
}

// 返回按钮点击事件
void RegManage::onBack()
{
    emit backRequested();
    close();
}

// 详情按钮点击事件
void RegManage::onDetail(int row, int col)
{
    // 保留原有的单元格点击事件处理，但主要功能已通过按钮的单独连接实现
}

// 接收服务器数据
void RegManage::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    handleServerResponse(data);
}

// 详情对话框构造函数
DetailDialog::DetailDialog(Patient p, QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("患者详情");
    setMinimumSize(400, 300);

    // 创建滚动区域，适应较多内容
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);

    // 添加患者信息（优化标签样式）
    QLabel *nameLabel = new QLabel("患者姓名: " + p.name);
    QLabel *genderLabel = new QLabel("性别: " + p.gender);
    QLabel *ageLabel = new QLabel("年龄: " + QString::number(p.age));
    QLabel *timeLabel = new QLabel("预约时间: " + p.time);
    QLabel *deptLabel = new QLabel("就诊科室: " + p.dept);
    QLabel *symptomLabel = new QLabel("症状描述: " + p.symptom);
    QLabel *phoneLabel = new QLabel("联系电话: " + p.phone);

    // 统一信息标签样式
    QList<QLabel*> infoLabels = {nameLabel, genderLabel, ageLabel, timeLabel, deptLabel, symptomLabel, phoneLabel};
    foreach (QLabel *label, infoLabels) {
        label->setStyleSheet("font-size: 14px; padding: 4px 0;");
        contentLayout->addWidget(label);
    }

    contentLayout->addStretch();

    // 关闭按钮（与返回按钮颜色统一，使用蓝色）
    closeBtn = new QPushButton("关闭");
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #0d6efd; /* 蓝色主色调，与返回按钮保持一致 */
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 12px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #0b5ed7;
        }
        QPushButton:pressed {
            background-color: #0a53be;
        }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &DetailDialog::close);
    contentLayout->addWidget(closeBtn);

    // 设置滚动区域
    scrollArea->setWidget(contentWidget);
    scrollArea->setWidgetResizable(true);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(scrollArea);
}
