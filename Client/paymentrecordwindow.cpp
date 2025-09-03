#include "paymentrecordwindow.h"
#include <QFont>
#include <QHeaderView>
#include <QScrollArea>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

PaymentRecordWindow::PaymentRecordWindow(QTcpSocket *socket, const QString &patientId, QWidget *parent)
    : QWidget(parent), patientId(patientId), m_socket(socket)
{
    setWindowTitle("缴费记录");
    setMinimumSize(900, 600);
    setStyleSheet("background-color: #f8f9fa;");
    initUI();
    // 连接socket的readyRead信号到onReadyRead槽函数
    connect(m_socket, &QTcpSocket::readyRead, this, &PaymentRecordWindow::onReadyRead);
    loadPaymentData();
}

PaymentRecordWindow::~PaymentRecordWindow()
{
}

void PaymentRecordWindow::initUI()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(30);
    
    // 标题
    QLabel *titleLabel = new QLabel("缴费记录");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: black; font-weight: bold;");
    mainLayout->addWidget(titleLabel);
    
    // 患者信息
    QLabel *patientInfoLabel = new QLabel(QString("患者ID：%1").arg(patientId));
    patientInfoLabel->setAlignment(Qt::AlignCenter);
    patientInfoLabel->setStyleSheet("color: black; font-size: 14px;");
    mainLayout->addWidget(patientInfoLabel);
    
    // 创建表格
    paymentTable = new QTableWidget();
    paymentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 设置表格不可编辑
    paymentTable->setSelectionBehavior(QAbstractItemView::SelectRows); // 设置选择行为为选中整行
    paymentTable->setAlternatingRowColors(true);  // 设置交替行颜色
    paymentTable->setFocusPolicy(Qt::NoFocus);  // 禁用焦点，移除单元格选中框
    
    // 设置表格列数和表头
    QStringList headers; 
    headers << "缴费事项" << "缴费金额" << "缴费方式" << "缴费时间";
    paymentTable->setColumnCount(headers.size());
    paymentTable->setHorizontalHeaderLabels(headers);
    
    // 设置表格和表头样式
    paymentTable->setStyleSheet(
        "QTableWidget { color: black; font-size: 14px; background-color: white; border: 1px solid #dee2e6; }"
        "QTableWidget::item { padding: 10px; border: 1px solid #dee2e6; }"
        "QTableWidget::item:selected { background-color: #e3f2fd; color: black; }"
        "QTableWidget::alternating-row-color { background-color: #fafafa; }"
    );
    
    // 自动调整列宽
    paymentTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // 设置表头样式，确保列名为黑色
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
    
    // 隐藏垂直表头
    paymentTable->verticalHeader()->setVisible(false);
    
    // 添加表格到主布局
    mainLayout->addWidget(paymentTable, 1);
    
    // 底部按钮布局
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    
    // 返回按钮 - 左下角
    backButton = new QPushButton("返回");
    backButton->setStyleSheet(
        "QPushButton { background-color: #007bff; color: white; border: none; padding: 12px 24px; "
        "border-radius: 4px; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #0056b3; }"
    );
    connect(backButton, &QPushButton::clicked, this, &PaymentRecordWindow::onBackButtonClicked);
    
    bottomLayout->addWidget(backButton);
    bottomLayout->addStretch();
    mainLayout->addLayout(bottomLayout);
}

void PaymentRecordWindow::loadPaymentData()
{
    // 如果有socket连接，从服务器获取数据
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        QString request = QString("GET_PAYMENT_RECORDS#%1").arg(patientId);
        m_socket->write(request.toUtf8() + "\n");

        // 显示加载中
        paymentTable->setRowCount(1);
        QTableWidgetItem *loadingItem = new QTableWidgetItem("正在加载缴费记录...");
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

void PaymentRecordWindow::onReadyRead()
{
    if (!m_socket) return;

    QByteArray data = m_socket->readAll();
    handleServerResponse(data);
}

void PaymentRecordWindow::refreshPaymentRecords()
{
    // 刷新缴费记录
    loadPaymentData();
}

void PaymentRecordWindow::handleServerResponse(const QByteArray &response)
{
    QString responseStr = QString::fromUtf8(response);

    if (responseStr.startsWith("db_error")) {
        // 数据库错误处理
        QString errorMsg = responseStr.section('#', 1);
        QMessageBox::critical(this, "数据库错误", QString("数据库操作失败: %1").arg(errorMsg.isEmpty() ? "未知错误" : errorMsg));
        paymentTable->setRowCount(1);
        QTableWidgetItem *errorItem = new QTableWidgetItem("数据库错误，请稍后再试");
        errorItem->setTextAlignment(Qt::AlignCenter);
        paymentTable->setItem(0, 0, errorItem);
        paymentTable->setSpan(0, 0, 1, paymentTable->columnCount());
    } else if (responseStr.startsWith("GET_PAYMENT_RECORDS_SUCCESS")) {
        // 解析缴费记录数据
        QString jsonStr = responseStr.section('#', 1);
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());

        if (!doc.isNull() && doc.isArray()) {
            QJsonArray recordsArray = doc.array();

            // 清空表格
            paymentTable->setRowCount(0);

            // 如果没有缴费记录
            if (recordsArray.isEmpty()) {
                paymentTable->setRowCount(1);
                QTableWidgetItem *emptyItem = new QTableWidgetItem("暂无缴费记录");
                emptyItem->setTextAlignment(Qt::AlignCenter);
                paymentTable->setItem(0, 0, emptyItem);
                paymentTable->setSpan(0, 0, 1, paymentTable->columnCount());
                return;
            }

            // 填充表格数据
            paymentTable->setRowCount(recordsArray.size());
            for (int row = 0; row < recordsArray.size(); ++row) {
                QJsonObject record = recordsArray[row].toObject();

                // 缴费事项列 - 使用实际的描述
                paymentTable->setItem(row, 0, new QTableWidgetItem(record["description"].toString()));

                // 缴费金额列
                double amount = record["amount"].toDouble();
                paymentTable->setItem(row, 1, new QTableWidgetItem(QString("%1元").arg(amount, 0, 'f', 2)));

                // 缴费方式列
                QString paymentMethod = record["payment_method"].toString();
                if (paymentMethod.isEmpty() || paymentMethod == "online_payment") {
                    paymentMethod = "在线支付";
                }
                paymentTable->setItem(row, 2, new QTableWidgetItem(paymentMethod));

                // 缴费时间列
                paymentTable->setItem(row, 3, new QTableWidgetItem(record["paid_at"].toString()));

                // 设置所有单元格文本居中
                for (int col = 0; col < paymentTable->columnCount(); ++col) {
                    paymentTable->item(row, col)->setTextAlignment(Qt::AlignCenter);
                }
            }
        } else {
            qDebug() << "Invalid JSON data format: " << jsonStr;
            paymentTable->setRowCount(1);
            QTableWidgetItem *errorItem = new QTableWidgetItem("数据格式错误");
            errorItem->setTextAlignment(Qt::AlignCenter);
            paymentTable->setItem(0, 0, errorItem);
            paymentTable->setSpan(0, 0, 1, paymentTable->columnCount());
        }
    }
}

void PaymentRecordWindow::onBackButtonClicked()
{
    emit backRequested();
    this->close();
}
