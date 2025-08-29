#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QDebug>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class LoginWindow; }
QT_END_NAMESPACE

class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    LoginWindow(const QString &serverAddress, quint16 port, QWidget *parent = nullptr);
    ~LoginWindow();
    QTcpSocket *socket;  // 成员变量
signals:
    void loginSuccessful(const QString &id);

private slots:
    void on_loginButton_clicked();

private:
    Ui::LoginWindow *ui;
    QString m_serverAddress;
    quint16 m_port;

    void authenticateUser(const QString &id, const QString &password);
};

#endif // LOGINWINDOW_H
