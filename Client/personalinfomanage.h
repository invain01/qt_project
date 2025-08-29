#ifndef PERSONALINFOMANAGE_H
#define PERSONALINFOMANAGE_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>

namespace Ui {
class PersonalInfoManage;
}

class PersonalInfoManage : public QWidget
{
    Q_OBJECT

public:
    explicit PersonalInfoManage(QTcpSocket *existingSocket, const QString &userId, QWidget *parent = nullptr);
    ~PersonalInfoManage();
    QTcpSocket *socket;  // 成员变量
    QString id;
    QMap<QString, QString> originalValues; // 存储原始值的成员变量

private slots:
    void on_editButton_clicked();
    void on_saveButton_clicked();
    void on_cancelButton_clicked();
    void onReadyRead(); // 处理socket的readyRead信号

private:
    Ui::PersonalInfoManage *ui;
    void handleServerResponse(const QString &response);
    void setEditEnabled(bool enabled);
};

#endif // PERSONALINFOMANAGE_H
