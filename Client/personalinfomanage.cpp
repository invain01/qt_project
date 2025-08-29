#include "personalinfomanage.h"
#include "ui_personalinfomanage.h"
#include <QJsonObject>      // 添加JSON头文件
#include <QJsonDocument>     // 添加JSON头文件
#include <QRegularExpression> // 用于正则表达式验证
#include <QDebug>

PersonalInfoManage::PersonalInfoManage(QTcpSocket *existingSocket, const QString &userId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PersonalInfoManage)
    , socket(existingSocket) // 初始化socket指针
    , id(userId) // 初始化用户ID
{
    ui->setupUi(this);
    // 显示按钮
    ui->editButton->setVisible(true);
    // 隐藏按钮
    ui->saveButton->setVisible(false);
    ui->cancelButton->setVisible(false);

    // 先断开之前可能存在的连接，避免多次连接
    disconnect(socket, &QTcpSocket::readyRead, this, &PersonalInfoManage::onReadyRead);

    // 连接readyRead信号到槽函数
    connect(socket, &QTcpSocket::readyRead, this, &PersonalInfoManage::onReadyRead);

    QString infoRequest = QString("USERINFO#%1").arg(id);
    socket->write(infoRequest.toUtf8()); // 发送个人信息请求

}

PersonalInfoManage::~PersonalInfoManage()
{
    delete ui;
}

void PersonalInfoManage::on_editButton_clicked()
{
    // 切换按钮状态
    ui->editButton->setVisible(false);
    ui->saveButton->setVisible(true);
    ui->cancelButton->setVisible(true);
    setEditEnabled(true);
}

void PersonalInfoManage::onReadyRead()
{
    if (!socket) return;

    QByteArray data = socket->readAll();
    QString response = QString::fromUtf8(data);
    
    // 处理服务器响应
    handleServerResponse(response);
    
}

void PersonalInfoManage::handleServerResponse(const QString &response)
{
    if (response.startsWith("USERINFO_SUCCESS")) {
        // 提取JSON部分
        QString jsonString = response.section('#', 1);
        QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());

        if (doc.isObject()) {
            QJsonObject json = doc.object();

            // 【新增】更新UI显示：将数据设置到对应的QLineEdit中
            ui->nameEdit->setText(json["real_name"].toString());     // 姓名
            ui->dateEdit->setText(json["birth_date"].toString());    // 生日
            ui->IDEdit->setText(json["id_card"].toString());         // 身份证号
            ui->phoneEdit->setText(json["phone"].toString());         // 电话
            ui->emailEdit->setText(json["email"].toString());         // 邮箱

            // 【新增】初始设置为只读模式
            setEditEnabled(false);

            // 存储原始值用于编辑模式
            originalValues["real_name"] = json["real_name"].toString();
            originalValues["birth_date"] = json["birth_date"].toString();
            originalValues["id_card"] = json["id_card"].toString();
            originalValues["phone"] = json["phone"].toString();
            originalValues["email"] = json["email"].toString();

            // 输出每个字段的值
            qDebug() << "=== 解析后的个人信息 ===";
            qDebug() << "姓名:" << json["real_name"].toString();
            qDebug() << "出生日期:" << json["birth_date"].toString();
            qDebug() << "身份证号:" << json["id_card"].toString();
            qDebug() << "手机号:" << json["phone"].toString();
            qDebug() << "邮箱:" << json["email"].toString();

        } else {
            qDebug() << "Invalid JSON response";
        }
    }
}

void PersonalInfoManage::on_saveButton_clicked()
{
    // 保存个人信息的逻辑
    // 创建JSON对象存储更新的信息
    QJsonObject json;
    json["real_name"] = originalValues["real_name"];
    json["birth_date"] = originalValues["birth_date"];
    json["id_card"] = originalValues["id_card"];
    json["phone"] = originalValues["phone"];
    json["email"] = originalValues["email"];
    
    // 转换为JSON字符串
    QJsonDocument doc(json);
    QString jsonString = doc.toJson(QJsonDocument::Compact);
    
    // 构造保存请求
    QString saveRequest = QString("SAVE_USERINFO#%1#%2").arg(id).arg(jsonString);
    
    if (socket) {
        socket->write(saveRequest.toUtf8());
        QMessageBox::information(this, "成功", "个人信息已保存");
    }
    
    // 切换按钮状态
    ui->saveButton->setVisible(false);
    ui->cancelButton->setVisible(false);
    ui->editButton->setVisible(true);
}

void PersonalInfoManage::on_cancelButton_clicked()
{
    // 取消编辑，恢复原始值
    QMessageBox::information(this, "提示", "已取消编辑");
    
    // 切换按钮状态
    ui->saveButton->setVisible(false);
    ui->cancelButton->setVisible(false);
    ui->editButton->setVisible(true);
}

void PersonalInfoManage::setEditEnabled(bool enabled)
{
    // 设置所有QLineEdit的只读状态 !enabled 表示：
    // 当 enabled为true（编辑模式）时，setReadOnly(false)即可编辑
    // 当 enabled为false（查看模式）时，setReadOnly(true)即只读[7](@ref)
    ui->nameEdit->setReadOnly(!enabled);
    ui->dateEdit->setReadOnly(!enabled);
    ui->IDEdit->setReadOnly(!enabled);
    ui->phoneEdit->setReadOnly(!enabled);
    ui->emailEdit->setReadOnly(!enabled);
}
