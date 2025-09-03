// PaymentWindow.cpp
#include "PaymentWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QCheckBox>
#include <QTableWidgetItem>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QTcpSocket>
#include <QBrush>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>

PaymentWindow::PaymentWindow(QTcpSocket *socket, const QString &patientId, QWidget *parent)
    : QWidget(parent), m_patientId(patientId), m_totalAmount(0.0), m_socket(socket)
{
    setWindowTitle("门诊缴费");
    setMinimumSize(900, 600);
    setStyleSheet("background-color: #f8f9fa;");

    // 连接信号槽
    if (m_socket) {
        connect(m_socket, &QTcpSocket::readyRead, this, &PaymentWindow::onReadyRead);
    }

    initUI();
    loadPaymentData();
}

PaymentWindow::~PaymentWindow()
{
}

void PaymentWindow::initUI()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(30);

    // 标题
    QLabel *titleLabel = new QLabel("门诊缴费");
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
    paymentTable = new QTableWidget();
    paymentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    paymentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    paymentTable->setAlternatingRowColors(true);
    paymentTable->setFocusPolicy(Qt::NoFocus);

    // 设置表格列数和表头
    QStringList headers;
    headers << "选择" << "缴费事项" << "金额" << "开具时间" << "状态";
    paymentTable->setColumnCount(headers.size());
    paymentTable->setHorizontalHeaderLabels(headers);

    // 设置表格样式
    paymentTable->setStyleSheet(
        "QTableWidget { color: black; font-size: 14px; background-color: white; border: 1px solid #dee2e6; }"
        "QTableWidget::item { padding: 10px; border: 1px solid #dee2e6; }"
        "QTableWidget::item:selected { background-color: #e3f2fd; color: black; }"
        "QTableWidget::alternating-row-color { background-color: #fafafa; }"
        );

    // 设置表头样式
    paymentTable->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { "
        "background-color: #e9ecef; "
        "color: black; "
        "font-weight: bold; "
        "font-size: 14px; "
        "padding: 10px; "
        "border: 1px solid #dee2e6; "
        "}"
        );

    // 设置列宽
    paymentTable->setColumnWidth(0, 50);   // 选择列
    paymentTable->setColumnWidth(1, 200);  // 缴费事项
    paymentTable->setColumnWidth(2, 100);  // 金额
    paymentTable->setColumnWidth(3, 150);  // 开具时间
    paymentTable->setColumnWidth(4, 100);  // 状态

    // 隐藏垂直表头
    paymentTable->verticalHeader()->setVisible(false);

    // 添加表格到主布局
    mainLayout->addWidget(paymentTable, 1);

    // 底部信息布局
    QHBoxLayout *infoLayout = new QHBoxLayout();

    // 全选按钮
    selectAllButton = new QPushButton("全选");
    selectAllButton->setStyleSheet(
        "QPushButton { background-color: #6c757d; color: white; border: none; padding: 8px 16px; "
        "border-radius: 4px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #5a6268; }"
        );
    connect(selectAllButton, &QPushButton::clicked, this, &PaymentWindow::onSelectAllClicked);

    // 总金额标签
    totalAmountLabel = new QLabel("合计金额: ¥0.00");
    totalAmountLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #e74c3c;");

    infoLayout->addWidget(selectAllButton);
    infoLayout->addStretch();
    infoLayout->addWidget(totalAmountLabel);

    mainLayout->addLayout(infoLayout);

    // 底部按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // 支付按钮
    payButton = new QPushButton("立即支付");
    payButton->setStyleSheet(
        "QPushButton { background-color: #28a745; color: white; border: none; padding: 12px 24px; "
        "border-radius: 4px; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #218838; }"
        "QPushButton:disabled { background-color: #6c757d; }"
        );
    payButton->setEnabled(false); // 初始状态禁用
    connect(payButton, &QPushButton::clicked, this, &PaymentWindow::onPayButtonClicked);

    // 返回按钮
    backButton = new QPushButton("返回");
    backButton->setStyleSheet(
        "QPushButton { background-color: #007bff; color: white; border: none; padding: 12px 24px; "
        "border-radius: 4px; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #0056b3; }"
        );
    connect(backButton, &QPushButton::clicked, this, &PaymentWindow::onBackButtonClicked);

    buttonLayout->addWidget(payButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(backButton);

    mainLayout->addLayout(buttonLayout);

    // 连接表格选择变化信号
    connect(paymentTable, &QTableWidget::itemSelectionChanged,
            this, &PaymentWindow::onItemSelectionChanged);
}

void PaymentWindow::loadPaymentData()
{
    // 如果有socket连接，从服务器获取数据
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        QString request = QString("GET_PAYMENT_ITEMS#%1").arg(m_patientId);
        m_socket->write(request.toUtf8() + "\n");

        // 显示加载中
        paymentTable->setRowCount(1);
        QTableWidgetItem *loadingItem = new QTableWidgetItem("正在加载待支付数据...");
        loadingItem->setTextAlignment(Qt::AlignCenter);
        paymentTable->setItem(0, 0, loadingItem);
        paymentTable->setSpan(0, 0, 1, paymentTable->columnCount());
        return;
    }
    
    // 如果没有socket连接，显示提示信息
    paymentTable->setRowCount(1);
    QTableWidgetItem *noConnectionItem = new QTableWidgetItem("无法连接到服务器，请检查网络连接");
    noConnectionItem->setTextAlignment(Qt::AlignCenter);
    paymentTable->setItem(0, 0, noConnectionItem);
    paymentTable->setSpan(0, 0, 1, paymentTable->columnCount());
}

void PaymentWindow::updateTotalAmount()
{
    m_totalAmount = 0.0;
    int selectedCount = 0;

    // 计算选中的费用总额
    for (int row = 0; row < paymentTable->rowCount(); ++row) {
        QWidget *widget = paymentTable->cellWidget(row, 0);
        if (!widget) continue; // 跳过没有复选框的行
        
        QCheckBox *checkBox = widget->findChild<QCheckBox*>();
        if (!checkBox) continue; // 跳过没有复选框的行

        if (checkBox->isChecked()) {
            // 安全获取金额
            QTableWidgetItem *amountItem = paymentTable->item(row, 2);
            if (amountItem) {
                QString amountStr = amountItem->text();
                amountStr.replace("¥", "");
                bool ok;
                double amount = amountStr.toDouble(&ok);
                if (ok && amount > 0) {
                    m_totalAmount += amount;
                    selectedCount++;
                }
            }
        }
    }

    // 更新总金额标签
    totalAmountLabel->setText(QString("合计金额: ¥%1").arg(m_totalAmount, 0, 'f', 2));

    // 根据是否有选中的项目启用/禁用支付按钮
    payButton->setEnabled(selectedCount > 0 && m_totalAmount > 0);
    
    // 更新全选按钮文本
    bool allSelected = (selectedCount > 0 && selectedCount == paymentTable->rowCount());
    selectAllButton->setText(allSelected ? "取消全选" : "全选");
}

void PaymentWindow::onSelectAllClicked()
{
    // 检查是否有有效的缴费项目
    if (paymentTable->rowCount() == 0) return;
    
    // 检查第一行是否是提示信息（暂无数据等）
    QWidget *firstWidget = paymentTable->cellWidget(0, 0);
    if (!firstWidget) return; // 没有复选框说明是提示信息行

    // 判断当前状态：如果所有可选项都已选中，则取消全选；否则全选
    bool shouldSelectAll = false;
    int validRows = 0;
    int selectedRows = 0;
    
    for (int row = 0; row < paymentTable->rowCount(); ++row) {
        QWidget *widget = paymentTable->cellWidget(row, 0);
        if (!widget) continue;
        
        QCheckBox *checkBox = widget->findChild<QCheckBox*>();
        if (!checkBox) continue;
        
        validRows++;
        if (checkBox->isChecked()) {
            selectedRows++;
        }
    }
    
    // 如果没有全部选中，则全选；如果全部选中，则取消全选
    shouldSelectAll = (selectedRows < validRows);

    // 设置所有复选框状态
    for (int row = 0; row < paymentTable->rowCount(); ++row) {
        QWidget *widget = paymentTable->cellWidget(row, 0);
        if (!widget) continue;
        
        QCheckBox *checkBox = widget->findChild<QCheckBox*>();
        if (!checkBox) continue;

        // 临时断开信号以避免重复调用updateTotalAmount
        checkBox->blockSignals(true);
        checkBox->setChecked(shouldSelectAll);
        checkBox->blockSignals(false);
    }

    // 更新总金额和按钮状态
    updateTotalAmount();
}

void PaymentWindow::onReadyRead()
{
    if (!m_socket) return;

    QByteArray data = m_socket->readAll();
    QString response = QString::fromUtf8(data);

    if (response.startsWith("GET_PAYMENT_ITEMS_SUCCESS")) {
        // 解析缴费项目数据
        QString jsonStr = response.section('#', 1);
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());

        if (!doc.isNull() && doc.isArray()) {
            QJsonArray itemsArray = doc.array();

            // 清空表格
            paymentTable->setRowCount(0);
            rowToApplicationIdMap.clear();

            // 过滤出待支付的项目
            QJsonArray pendingItems;
            for (const QJsonValue &value : itemsArray) {
                QJsonObject item = value.toObject();
                QString status = item["status"].toString();
                if (status == "pending") {
                    pendingItems.append(item);
                }
            }

            // 如果没有待支付项目
            if (pendingItems.isEmpty()) {
                paymentTable->setRowCount(1);
                QTableWidgetItem *emptyItem = new QTableWidgetItem("暂无待支付项目");
                emptyItem->setTextAlignment(Qt::AlignCenter);
                paymentTable->setItem(0, 0, emptyItem);
                paymentTable->setSpan(0, 0, 1, paymentTable->columnCount());
                
                // 重置UI状态
                m_totalAmount = 0.0;
                totalAmountLabel->setText("合计金额: ¥0.00");
                payButton->setEnabled(false);
                payButton->setText("立即支付");
                selectAllButton->setText("全选");
                return;
            }

            // 填充表格数据（只显示待支付项目）
            paymentTable->setRowCount(pendingItems.size());
            for (int row = 0; row < pendingItems.size(); ++row) {
                QJsonObject item = pendingItems[row].toObject();

                // 选择列 - 使用复选框，扩大点击范围
                QWidget *checkWidget = new QWidget();
                checkWidget->setStyleSheet("QWidget:hover { background-color: #f0f0f0; }"); // 悬停效果
                QHBoxLayout *checkLayout = new QHBoxLayout(checkWidget);
                QCheckBox *checkBox = new QCheckBox();
                checkBox->setStyleSheet("QCheckBox::indicator { width: 18px; height: 18px; }"); // 增大复选框尺寸
                connect(checkBox, &QCheckBox::stateChanged, this, &PaymentWindow::updateTotalAmount);
                
                // 让整个widget都能响应点击
                checkWidget->installEventFilter(this);
                checkWidget->setProperty("checkbox", QVariant::fromValue(checkBox));
                
                checkLayout->addWidget(checkBox);
                checkLayout->setAlignment(Qt::AlignCenter);
                checkLayout->setContentsMargins(5, 5, 5, 5); // 增加边距，扩大点击范围
                paymentTable->setCellWidget(row, 0, checkWidget);

                // 缴费事项列
                paymentTable->setItem(row, 1, new QTableWidgetItem(item["description"].toString()));

                // 金额列
                double amount = item["amount"].toDouble();
                paymentTable->setItem(row, 2, new QTableWidgetItem(QString("¥%1").arg(amount, 0, 'f', 2)));

                // 开具时间列
                paymentTable->setItem(row, 3, new QTableWidgetItem(item["created_at"].toString()));

                // 状态列 - 都是待支付状态
                QTableWidgetItem *statusItem = new QTableWidgetItem("待支付");
                statusItem->setForeground(QBrush(QColor("#e74c3c"))); // 红色
                paymentTable->setItem(row, 4, statusItem);

                // 设置所有单元格文本居中
                for (int col = 1; col < paymentTable->columnCount(); ++col) {
                    paymentTable->item(row, col)->setTextAlignment(Qt::AlignCenter);
                }

                // 记录申请ID（如果有的话）
                if (item.contains("application_id")) {
                    rowToApplicationIdMap[row] = item["application_id"].toString();
                }
            }
            
            // 数据加载完成后重置UI状态
            updateTotalAmount(); // 这会重置总金额、按钮状态等
            
        } else {
            qDebug() << "Invalid JSON data format: " << jsonStr;
            paymentTable->setRowCount(1);
            QTableWidgetItem *errorItem = new QTableWidgetItem("数据格式错误");
            errorItem->setTextAlignment(Qt::AlignCenter);
            paymentTable->setItem(0, 0, errorItem);
            paymentTable->setSpan(0, 0, 1, paymentTable->columnCount());
        }
    } else if (response.startsWith("db_error")) {
        // 数据库错误处理
        QString errorMsg = response.section('#', 1);
        QMessageBox::critical(this, "数据库错误", QString("数据库操作失败: %1").arg(errorMsg.isEmpty() ? "未知错误" : errorMsg));
        paymentTable->setRowCount(1);
        QTableWidgetItem *errorItem = new QTableWidgetItem("数据库错误，请稍后再试");
        errorItem->setTextAlignment(Qt::AlignCenter);
        paymentTable->setItem(0, 0, errorItem);
        paymentTable->setSpan(0, 0, 1, paymentTable->columnCount());
    } else if (response.startsWith("PROCESS_PAYMENT_SUCCESS")) {
        // 支付成功处理
        QString paymentId = response.section('#', 1);
        qDebug() << "支付成功，支付ID:" << paymentId;
        
        // 立即重置总金额和相关状态
        m_totalAmount = 0.0;
        if (totalAmountLabel) {
            totalAmountLabel->setText("合计金额: ¥0.00");
        }
        if (payButton) {
            payButton->setEnabled(false);
            payButton->setText("立即支付");
        }
        if (selectAllButton) {
            selectAllButton->setText("全选");
        }
        
        // 显示成功消息（使用模态对话框，更安全）
        QMessageBox::information(this, "支付成功", 
                                QString("支付成功！\n支付编号: %1").arg(paymentId));
        
        // 在消息框关闭后再执行后续操作，增加延迟以确保UI稳定
        QTimer::singleShot(300, this, [this]() {
            // 先发送成功信号
            emit paymentSuccess("general", 0.0, "门诊缴费");
        });
        
        // 延迟更长时间再重新加载数据，避免与信号处理冲突
        QTimer::singleShot(500, this, [this]() {
            // 重新加载数据
            if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
                loadPaymentData();
            }
        });
        
    } else if (response.startsWith("PROCESS_PAYMENT_FAIL")) {
        // 支付失败处理
        QString errorMsg = response.section('#', 1);
        qDebug() << "支付失败:" << errorMsg;
        
        // 重新启用支付按钮
        if (payButton) {
            payButton->setEnabled(true);
            payButton->setText("立即支付");
        }
        
        QMessageBox::warning(this, "支付失败", QString("支付失败: %1").arg(errorMsg));
    }
}

void PaymentWindow::onItemSelectionChanged()
{
    // 当选择变化时更新总金额
    updateTotalAmount();
}

void PaymentWindow::onPayButtonClicked()
{
    // 获取选中的项目
    QList<int> selectedRows;
    for (int row = 0; row < paymentTable->rowCount(); ++row) {
        QWidget *widget = paymentTable->cellWidget(row, 0);
        QCheckBox *checkBox = widget->findChild<QCheckBox*>();

        if (checkBox && checkBox->isChecked()) {
            selectedRows.append(row);
        }
    }

    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择要支付的费用项目");
        return;
    }

    // 显示支付确认对话框
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认支付",
                                  QString("确认支付选中的 %1 项费用，总计 ¥%2？")
                                      .arg(selectedRows.size())
                                      .arg(m_totalAmount, 0, 'f', 2),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 检查是否为单项支付
        if (selectedRows.size() == 1) {
            int row = selectedRows.first();
            if (rowToApplicationIdMap.contains(row)) {
                // 使用单项支付模式
                qDebug() << "使用单项支付模式，application_id:" << rowToApplicationIdMap[row];
                QJsonObject paymentRequest;
                paymentRequest["patient_id"] = m_patientId;
                paymentRequest["application_id"] = rowToApplicationIdMap[row];
                paymentRequest["payment_method"] = "在线支付";

                // 发送单项支付请求
                if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
                    payButton->setEnabled(false);
                    payButton->setText("支付中...");
                    
                    QJsonDocument doc(paymentRequest);
                    QString request = "PROCESS_PAYMENT#" + doc.toJson(QJsonDocument::Compact);
                    qDebug() << "发送单项支付请求:" << request;
                    m_socket->write(request.toUtf8() + "\n");
                    m_socket->flush();
                } else {
                    QMessageBox::warning(this, "连接错误", "与服务器连接已断开，请重新连接后再试");
                }
                return; // 早期返回，避免执行批量支付逻辑
            }
        }
        
        // 批量支付模式
        QJsonArray paymentItems;

        for (int row : selectedRows) {
            QJsonObject item;
            // 检查表格项是否存在
            if (!paymentTable->item(row, 1) || !paymentTable->item(row, 2)) {
                qDebug() << "无效的缴费项目数据，跳过处理"; 
                continue;
            }
            item["item_name"] = paymentTable->item(row, 1)->text();
            QString amountStr = paymentTable->item(row, 2)->text();
            amountStr.replace("¥", "");
            double amount = amountStr.toDouble();
            item["amount"] = amount;
            item["created_at"] = paymentTable->item(row, 3)->text();

            // 添加申请ID（如果有的话）
            if (rowToApplicationIdMap.contains(row)) {
                item["application_id"] = rowToApplicationIdMap[row];
            }

            paymentItems.append(item);
        }

        QJsonObject paymentRequest;
        paymentRequest["patient_id"] = m_patientId;
        paymentRequest["total_amount"] = m_totalAmount;
        paymentRequest["payment_time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        paymentRequest["items"] = paymentItems;

        // 发送支付请求到服务器
        if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
            // 禁用支付按钮，防止重复点击
            payButton->setEnabled(false);
            payButton->setText("支付中...");
            
            QJsonDocument doc(paymentRequest);
            QString jsonString = doc.toJson(QJsonDocument::Compact);
            QString request = QString("PROCESS_PAYMENT#%1").arg(jsonString);
            m_socket->write(request.toUtf8() + "\n");
            m_socket->flush(); // 确保数据立即发送
        } else {
            QMessageBox::warning(this, "连接错误", "与服务器连接已断开，请重新连接后再试");
        }
    }
}

void PaymentWindow::onBackButtonClicked()
{
    emit backRequested();
    this->close();
}

// 事件过滤器实现，用于扩大复选框点击范围
bool PaymentWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QWidget *widget = qobject_cast<QWidget*>(obj);
            if (widget) {
                QVariant checkboxVariant = widget->property("checkbox");
                if (checkboxVariant.isValid()) {
                    QCheckBox *checkBox = checkboxVariant.value<QCheckBox*>();
                    if (checkBox) {
                        // 切换复选框状态
                        checkBox->setChecked(!checkBox->isChecked());
                        return true; // 事件已处理
                    }
                }
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}
