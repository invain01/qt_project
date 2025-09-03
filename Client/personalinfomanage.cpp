#include "personalinfomanage.h"
#include "ui_personalinfomanage.h"
#include "EmailVerificationDialog.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QDebug>
#include <QFileDialog>
#include <QPainter>
#include <QStyleFactory>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>

PersonalInfoManage::PersonalInfoManage(QTcpSocket *existingSocket, const QString &userId, QWidget *parent)
    : QWidget(parent), ui(new Ui::PersonalInfoManage), socket(existingSocket), id(userId)
{
    ui->setupUi(this);
    setupUI();
    applyStyles();

    // 初始按钮状态
    ui->editButton->setVisible(true);
    ui->saveButton->setVisible(false);
    ui->cancelButton->setVisible(false);
    ui->uploadButton->setVisible(false);

    // 设置头像Label属性
    ui->avatarLabel->setScaledContents(true);
    ui->avatarLabel->setAlignment(Qt::AlignCenter);
    ui->avatarLabel->setMinimumSize(120, 120);

    // 连接信号
    disconnect(socket, &QTcpSocket::readyRead, this, &PersonalInfoManage::onReadyRead);
    connect(socket, &QTcpSocket::readyRead, this, &PersonalInfoManage::onReadyRead);

    QString infoRequest = QString("USERINFO#%1").arg(id);
    socket->write(infoRequest.toUtf8());
}

PersonalInfoManage::~PersonalInfoManage()
{
    delete ui;
}

void PersonalInfoManage::setupUI()
{
    // 设置窗口标题和图标
    setWindowTitle("个人信息管理");
    setWindowIcon(QIcon(":/icons/user-profile.png"));
    // 增加窗口最小宽度，确保有足够空间显示所有内容
    setMinimumSize(700, 550);

    // 设置输入框提示文本
    ui->nameEdit->setPlaceholderText("请输入真实姓名");
    ui->dateEdit->setPlaceholderText("格式：YYYY-MM-DD");
    ui->IDEdit->setPlaceholderText("18位身份证号码");
    ui->phoneEdit->setPlaceholderText("11位手机号码");
    ui->emailEdit->setPlaceholderText("例如：user@example.com");

    // 设置按钮图标
    ui->editButton->setIcon(QIcon(":/icons/edit.png"));
    ui->saveButton->setIcon(QIcon(":/icons/save.png"));
    ui->cancelButton->setIcon(QIcon(":/icons/cancel.png"));
    ui->uploadButton->setIcon(QIcon(":/icons/upload.png"));

    // 设置按钮大小
    ui->editButton->setFixedSize(100, 40);
    ui->saveButton->setFixedSize(100, 40);
    ui->cancelButton->setFixedSize(100, 40);
    ui->uploadButton->setFixedSize(120, 40);

    // 重置所有控件的几何位置，使用布局管理器重新排列
    QLayout *oldLayout = layout();
    if (oldLayout)
    {
        delete oldLayout;
    }

    // 创建主垂直布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // 创建水平布局用于头像和表单
    QHBoxLayout *avatarAndFormLayout = new QHBoxLayout();
    avatarAndFormLayout->setSpacing(40);

    // 头像部分
    QVBoxLayout *avatarLayout = new QVBoxLayout();
    avatarLayout->setAlignment(Qt::AlignCenter);
    avatarLayout->setSpacing(15);

    // 调整头像大小和属性
    ui->avatarLabel->setScaledContents(true);
    ui->avatarLabel->setAlignment(Qt::AlignCenter);
    ui->avatarLabel->setMinimumSize(150, 150);
    ui->avatarLabel->setMaximumSize(150, 150);
    ui->avatarLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    avatarLayout->addWidget(ui->avatarLabel);
    avatarLayout->addWidget(ui->uploadButton);

    // 表单部分
    QWidget *formWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout(formWidget);

    // 设置表单布局属性
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    formLayout->setVerticalSpacing(25);
    formLayout->setHorizontalSpacing(20);

    // 设置标签样式
    QFont labelFont = font();
    labelFont.setPointSize(12);
    labelFont.setBold(true);

    // 添加表单项
    ui->label_5->setFont(labelFont);
    ui->label_4->setFont(labelFont);
    ui->label_3->setFont(labelFont);
    ui->label_2->setFont(labelFont);
    ui->label->setFont(labelFont);

    // 创建性别标签和单选按钮
    QLabel *genderLabel = new QLabel("性别");
    genderLabel->setFont(labelFont);
    genderLabel->setMinimumWidth(80);
    genderLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // 创建性别选择的容器
    QWidget *genderWidget = new QWidget();
    QHBoxLayout *genderLayout = new QHBoxLayout(genderWidget);
    genderLayout->setSpacing(20);

    // 创建单选按钮
    maleRadioButton = new QRadioButton("男");
    femaleRadioButton = new QRadioButton("女");

    // 设置单选按钮字体
    QFont radioFont = font();
    radioFont.setPointSize(11);
    maleRadioButton->setFont(radioFont);
    femaleRadioButton->setFont(radioFont);

    // 将单选按钮添加到按钮组
    QButtonGroup *genderGroup = new QButtonGroup(this);
    genderGroup->addButton(maleRadioButton);
    genderGroup->addButton(femaleRadioButton);

    // 添加到布局
    genderLayout->addWidget(maleRadioButton);
    genderLayout->addWidget(femaleRadioButton);
    genderLayout->addStretch();

    // 确保标签有足够的宽度显示完整文本
    ui->label_5->setMinimumWidth(80);
    ui->label_4->setMinimumWidth(80);
    ui->label_3->setMinimumWidth(80);
    ui->label_2->setMinimumWidth(80);
    ui->label->setMinimumWidth(80);

    // 设置标签垂直居中
    ui->label_5->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->label_4->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->label_3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->label_2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // 添加表单项到布局
    formLayout->addRow(ui->label_5, ui->nameEdit);
    formLayout->addRow(genderLabel, genderWidget);
    formLayout->addRow(ui->label_4, ui->dateEdit);
    formLayout->addRow(ui->label_3, ui->phoneEdit);
    formLayout->addRow(ui->label_2, ui->emailEdit);
    formLayout->addRow(ui->label, ui->IDEdit);

    // 设置输入框最小宽度
    ui->nameEdit->setMinimumWidth(250);
    ui->dateEdit->setMinimumWidth(250);
    ui->phoneEdit->setMinimumWidth(250);
    ui->emailEdit->setMinimumWidth(250);
    ui->IDEdit->setMinimumWidth(250);

    // 添加头像布局和表单布局到水平布局
    avatarAndFormLayout->addLayout(avatarLayout);
    avatarAndFormLayout->addWidget(formWidget);

    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setAlignment(Qt::AlignCenter);
    buttonLayout->setSpacing(30);
    buttonLayout->addWidget(ui->saveButton);
    buttonLayout->addWidget(ui->editButton);
    buttonLayout->addWidget(ui->cancelButton);

    // 添加所有布局到主布局
    mainLayout->addLayout(avatarAndFormLayout);
    mainLayout->addLayout(buttonLayout);
}

void PersonalInfoManage::applyStyles()
{
    // 应用Fusion样式
    qApp->setStyle(QStyleFactory::create("Fusion"));

    // 主窗口样式
    this->setStyleSheet(
        "QWidget { background-color: #f5f7fa; font-family: 'Microsoft YaHei UI'; }"
        "QLabel { color: #2c3e50; font-size: 12pt; padding: 5px; }" // 增加内边距
        "QGroupBox { border: 1px solid #dcdde1; border-radius: 8px; margin-top: 10px; "
        "           font-weight: bold; color: #34495e; padding: 15px; }" // 增加内边距
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }");

    // 输入框样式
    QString lineEditStyle =
        "QLineEdit { border: 1px solid #dcdde1; border-radius: 4px; padding: 8px; "
        "            background: white; selection-background-color: #3498db; "
        "            min-width: 250px; }" // 增加输入框最小宽度
        "QLineEdit:disabled { background: #f1f2f6; }";

    ui->nameEdit->setStyleSheet(lineEditStyle);
    ui->dateEdit->setStyleSheet(lineEditStyle);
    ui->IDEdit->setStyleSheet(lineEditStyle);
    ui->phoneEdit->setStyleSheet(lineEditStyle);
    ui->emailEdit->setStyleSheet(lineEditStyle);

    // 按钮样式
    ui->editButton->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; border-radius: 4px; "
        "              font-weight: bold; padding: 5px; }"
        "QPushButton:hover { background-color: #2980b9; }"
        "QPushButton:pressed { background-color: #1d6fa5; }");

    ui->saveButton->setStyleSheet(
        "QPushButton { background-color: #2ecc71; color: white; border-radius: 4px; "
        "              font-weight: bold; padding: 5px; }"
        "QPushButton:hover { background-color: #27ae60; }"
        "QPushButton:pressed { background-color: #219653; }");

    ui->cancelButton->setStyleSheet(
        "QPushButton { background-color: #95a5a6; color: white; border-radius: 4px; "
        "              font-weight: bold; padding: 5px; }"
        "QPushButton:hover { background-color: #7f8c8d; }"
        "QPushButton:pressed { background-color: #6c7a89; }");

    ui->uploadButton->setStyleSheet(
        "QPushButton { background-color: #9b59b6; color: white; border-radius: 4px; "
        "              font-weight: bold; padding: 5px; }"
        "QPushButton:hover { background-color: #8e44ad; }"
        "QPushButton:pressed { background-color: #7d3c98; }");

    // 头像框样式
    ui->avatarLabel->setStyleSheet(
        "QLabel { background-color: white; border: 2px solid #dcdde1; border-radius: 60px; }");
}

void PersonalInfoManage::on_editButton_clicked()
{
    ui->editButton->setVisible(false);
    ui->saveButton->setVisible(true);
    ui->cancelButton->setVisible(true);
    ui->uploadButton->setVisible(true);
    setEditEnabled(true);
}

void PersonalInfoManage::onReadyRead()
{
    if (!socket)
        return;

    QByteArray data = socket->readAll();
    QString response = QString::fromUtf8(data);
    handleServerResponse(response);
}

void PersonalInfoManage::handleServerResponse(const QString &response)
{
    qDebug() << "服务器响应: " << response;

    if (response.startsWith("USERINFO#"))
    {
        QString jsonString = response.mid(9); // 去掉前缀
        QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
        QJsonObject obj = doc.object();

        // 更新UI和原始值
        ui->nameEdit->setText(obj["name"].toString());
        ui->dateEdit->setText(obj["birthday"].toString());
        ui->phoneEdit->setText(obj["phone"].toString());
        ui->emailEdit->setText(obj["email"].toString());
        ui->IDEdit->setText(obj["id_card"].toString());

        // 处理性别信息
        QString gender = obj["gender"].toString();
        if (gender == "男")
        {
            maleRadioButton->setChecked(true);
        }
        else if (gender == "女")
        {
            femaleRadioButton->setChecked(true);
        }

        // 保存原始值，用于取消编辑时恢复
        originalValues["name"] = obj["name"].toString();
        originalValues["birthday"] = obj["birthday"].toString();
        originalValues["phone"] = obj["phone"].toString();
        originalValues["email"] = obj["email"].toString();
        originalValues["id_card"] = obj["id_card"].toString();
        originalValues["gender"] = gender;

        // 加载头像
        QString avatarBase64 = obj["avatar"].toString();
        if (!avatarBase64.isEmpty())
        {
            QByteArray avatarData = QByteArray::fromBase64(avatarBase64.toUtf8());
            QPixmap pixmap;
            if (pixmap.loadFromData(avatarData))
            {
                // 调整为圆形头像
                QPixmap rounded(pixmap.size());
                rounded.fill(Qt::transparent);
                QPainter painter(&rounded);
                painter.setRenderHint(QPainter::Antialiasing);
                painter.setBrush(QBrush(pixmap));
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(pixmap.rect());
                ui->avatarLabel->setPixmap(rounded.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
        }

        // 恢复按钮状态
        ui->editButton->setVisible(true);
        ui->saveButton->setVisible(false);
        ui->cancelButton->setVisible(false);
        ui->uploadButton->setVisible(false);

        // 设置输入框为只读
        setEditEnabled(false);
    }
    else if (response == "USERINFO_UPDATE_SUCCESS")
    {
        QMessageBox::information(this, "成功", "个人信息更新成功！");
        ui->editButton->setVisible(true);
        ui->saveButton->setVisible(false);
        ui->cancelButton->setVisible(false);
        ui->uploadButton->setVisible(false);
        setEditEnabled(false);
    }
    else if (response == "USERINFO_UPDATE_FAILED")
    {
        QMessageBox::warning(this, "失败", "个人信息更新失败，请重试！");
    }
}

void PersonalInfoManage::on_saveButton_clicked()
{
    // 检查是否需要邮箱验证（身份证号修改等敏感操作）
    if (requiresEmailVerification()) {
        if (!verifyEmailForSensitiveChange()) {
            return; // 验证失败，中止保存
        }
    }
    
    // 构建用户信息JSON对象
    QJsonObject userInfo;
    userInfo["id"] = id;
    userInfo["name"] = ui->nameEdit->text();
    userInfo["birthday"] = ui->dateEdit->text();
    userInfo["phone"] = ui->phoneEdit->text();
    userInfo["email"] = ui->emailEdit->text();
    userInfo["id_card"] = ui->IDEdit->text();

    // 添加性别信息
    if (maleRadioButton->isChecked())
    {
        userInfo["gender"] = "男";
    }
    else if (femaleRadioButton->isChecked())
    {
        userInfo["gender"] = "女";
    }
    else
    {
        userInfo["gender"] = "";
    }

    // 检查是否有头像更新
    if (!m_tempAvatarPath.isEmpty())
    {
        QFile file(m_tempAvatarPath);
        if (file.open(QIODevice::ReadOnly))
        {
            QByteArray avatarData = file.readAll();
            userInfo["avatar"] = QString(avatarData.toBase64());
            file.close();
        }
    }

    // 转换为JSON字符串
    QJsonDocument doc(userInfo);
    QString jsonString = doc.toJson(QJsonDocument::Compact);

    // 发送请求
    QString request = QString("UPDATE_USERINFO#%1").arg(jsonString);
    socket->write(request.toUtf8());
}

void PersonalInfoManage::on_cancelButton_clicked()
{
    // 恢复原始值
    ui->nameEdit->setText(originalValues["name"]);
    ui->dateEdit->setText(originalValues["birthday"]);
    ui->phoneEdit->setText(originalValues["phone"]);
    ui->emailEdit->setText(originalValues["email"]);
    ui->IDEdit->setText(originalValues["id_card"]);

    // 恢复原始性别选择
    QString gender = originalValues["gender"];
    if (gender == "男")
    {
        maleRadioButton->setChecked(true);
    }
    else if (gender == "女")
    {
        femaleRadioButton->setChecked(true);
    }
    else
    {
        maleRadioButton->setChecked(false);
        femaleRadioButton->setChecked(false);
    }

    // 恢复按钮状态
    ui->editButton->setVisible(true);
    ui->saveButton->setVisible(false);
    ui->cancelButton->setVisible(false);
    ui->uploadButton->setVisible(false);

    // 设置输入框为只读
    setEditEnabled(false);
}

void PersonalInfoManage::setEditEnabled(bool enabled)
{
    // 设置输入框只读状态
    ui->nameEdit->setReadOnly(!enabled);
    ui->dateEdit->setReadOnly(!enabled);
    ui->phoneEdit->setReadOnly(!enabled);
    ui->emailEdit->setReadOnly(!enabled);
    ui->IDEdit->setReadOnly(!enabled);

    // 设置性别单选按钮的可用状态
    maleRadioButton->setEnabled(enabled);
    femaleRadioButton->setEnabled(enabled);

    // 设置输入框样式
    if (!enabled)
    {
        ui->nameEdit->setStyleSheet("background-color: #f0f0f0; color: #666;");
        ui->dateEdit->setStyleSheet("background-color: #f0f0f0; color: #666;");
        ui->phoneEdit->setStyleSheet("background-color: #f0f0f0; color: #666;");
        ui->emailEdit->setStyleSheet("background-color: #f0f0f0; color: #666;");
        ui->IDEdit->setStyleSheet("background-color: #f0f0f0; color: #666;");
    }
    else
    {
        ui->nameEdit->setStyleSheet("background-color: white; color: black;");
        ui->dateEdit->setStyleSheet("background-color: white; color: black;");
        ui->phoneEdit->setStyleSheet("background-color: white; color: black;");
        ui->emailEdit->setStyleSheet("background-color: white; color: black;");
        ui->IDEdit->setStyleSheet("background-color: white; color: black;");
    }
}

void PersonalInfoManage::on_uploadButton_clicked()
{
    QString imagePath = QFileDialog::getOpenFileName(
        this,
        tr("选择头像图片"),
        "",
        tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)"));

    if (imagePath.isEmpty())
    {
        return;
    }

    if (loadAvatarFromPath(imagePath))
    {
        m_tempAvatarPath = imagePath;
        QMessageBox::information(this, "提示", "头像选择成功，点击保存后生效");
    }
    else
    {
        QMessageBox::warning(this, "错误", "无法加载选择的图片文件");
    }
}

bool PersonalInfoManage::loadAvatarFromPath(const QString &path)
{
    QPixmap pixmap;
    if (!pixmap.load(path))
    {
        return false;
    }

    // 创建圆形头像
    QPixmap rounded(120, 120);
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(pixmap.scaled(120, 120, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, 120, 120);
    painter.end();

    ui->avatarLabel->setPixmap(rounded);
    return true;
}

bool PersonalInfoManage::requiresEmailVerification()
{
    // 检查身份证号是否发生了修改
    QString currentIdCard = ui->IDEdit->text().trimmed();
    QString originalIdCard = originalValues["id_card"];
    
    // 如果身份证号发生了变化，需要进行邮箱验证
    return (currentIdCard != originalIdCard);
}

bool PersonalInfoManage::verifyEmailForSensitiveChange()
{
    QString email = ui->emailEdit->text().trimmed();
    
    // 检查是否有邮箱地址
    if (email.isEmpty()) {
        QMessageBox::warning(this, "验证失败", "修改身份证号等敏感信息需要邮箱验证，请先设置邮箱地址！");
        return false;
    }
    
    // 验证邮箱格式
    if (!email.contains('@') || !email.contains(".com")) {
        QMessageBox::warning(this, "验证失败", "邮箱格式不正确，无法进行验证！");
        return false;
    }
    
    // 显示邮箱验证对话框
    if (EmailVerificationDialog::verifyEmail(email, "身份证号修改验证", this)) {
        QMessageBox::information(this, "验证成功", "邮箱验证成功，可以保存修改！");
        return true;
    } else {
        QMessageBox::warning(this, "验证失败", "邮箱验证失败，无法保存敏感信息的修改！");
        return false;
    }
}
