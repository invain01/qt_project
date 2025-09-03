#ifndef PERSONALINFOMANAGE_H
#define PERSONALINFOMANAGE_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>
#include <QMap>
#include <QRadioButton>
#include "EmailVerificationDialog.h"

namespace Ui
{
    class PersonalInfoManage;
}

class PersonalInfoManage : public QWidget
{
    Q_OBJECT

public:
    explicit PersonalInfoManage(QTcpSocket *existingSocket, const QString &userId, QWidget *parent = nullptr);
    ~PersonalInfoManage();
    QTcpSocket *socket;
    QString id;
    QMap<QString, QString> originalValues;
    QString m_tempAvatarPath;

private slots:
    void on_editButton_clicked();
    void on_saveButton_clicked();
    void on_cancelButton_clicked();
    void onReadyRead();
    void on_uploadButton_clicked();

private:
    Ui::PersonalInfoManage *ui;
    QRadioButton *maleRadioButton;   // 男性单选按钮
    QRadioButton *femaleRadioButton; // 女性单选按钮
    void handleServerResponse(const QString &response);
    void setEditEnabled(bool enabled);
    bool loadAvatarFromPath(const QString &path);
    void setupUI();
    void applyStyles();
    bool requiresEmailVerification(); // 检查是否需要邮箱验证
    bool verifyEmailForSensitiveChange(); // 为敏感操作验证邮箱
};

#endif // PERSONALINFOMANAGE_H
