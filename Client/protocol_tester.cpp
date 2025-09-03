#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>
#include <QMessageBox>

class VideoCallProtocolTester : public QWidget
{
    Q_OBJECT

public:
    VideoCallProtocolTester(QWidget *parent = nullptr) : QWidget(parent)
    {
        setupUI();
        setupServer();
    }

private slots:
    void sendRequest()
    {
        QString fromId = m_fromEdit->text().trimmed();
        QString toId = m_toEdit->text().trimmed();
        
        if (fromId.isEmpty() || toId.isEmpty()) {
            QMessageBox::warning(this, "错误", "请填写发送者和接收者ID");
            return;
        }
        
        QString message = QString("VIDEO_CALL_REQUEST#%1#%2").arg(fromId, toId);
        sendMessage(message);
    }
    
    void sendAcceptResponse()
    {
        QString fromId = m_fromEdit->text().trimmed();
        QString toId = m_toEdit->text().trimmed();
        
        QString message = QString("VIDEO_CALL_RESPONSE#%1#%2#true").arg(fromId, toId);
        sendMessage(message);
    }
    
    void sendRejectResponse()
    {
        QString fromId = m_fromEdit->text().trimmed();
        QString toId = m_toEdit->text().trimmed();
        
        QString message = QString("VIDEO_CALL_RESPONSE#%1#%2#false").arg(fromId, toId);
        sendMessage(message);
    }
    
    void sendEndCall()
    {
        QString fromId = m_fromEdit->text().trimmed();
        QString toId = m_toEdit->text().trimmed();
        
        QString message = QString("VIDEO_CALL_END#%1#%2").arg(fromId, toId);
        sendMessage(message);
    }
    
    void connectToServer()
    {
        if (m_socket && m_socket->state() == QTcpSocket::ConnectedState) {
            m_socket->disconnectFromHost();
            m_connectBtn->setText("连接到服务器");
            m_statusLabel->setText("状态: 已断开");
            return;
        }
        
        if (!m_socket) {
            m_socket = new QTcpSocket(this);
            connect(m_socket, &QTcpSocket::connected, [this]() {
                m_connectBtn->setText("断开连接");
                m_statusLabel->setText("状态: 已连接");
                addLog("✅ 连接到服务器成功");
            });
            
            connect(m_socket, &QTcpSocket::disconnected, [this]() {
                m_connectBtn->setText("连接到服务器");
                m_statusLabel->setText("状态: 已断开");
                addLog("❌ 与服务器断开连接");
            });
            
            connect(m_socket, &QTcpSocket::readyRead, [this]() {
                QByteArray data = m_socket->readAll();
                QString message = QString::fromUtf8(data);
                addLog(QString("📥 收到: %1").arg(message.trimmed()));
            });
            
            connect(m_socket, &QTcpSocket::errorOccurred, [this](QAbstractSocket::SocketError error) {
                addLog(QString("❌ 连接错误: %1").arg(static_cast<int>(error)));
            });
        }
        
        QString host = m_hostEdit->text().trimmed();
        int port = m_portEdit->text().toInt();
        
        if (host.isEmpty()) host = "127.0.0.1";
        if (port <= 0) port = 8888;
        
        m_socket->connectToHost(host, port);
        addLog(QString("🔗 正在连接到 %1:%2...").arg(host).arg(port));
    }
    
    void onNewConnection()
    {
        QTcpSocket *clientSocket = m_server->nextPendingConnection();
        m_clients.append(clientSocket);
        
        connect(clientSocket, &QTcpSocket::readyRead, [this, clientSocket]() {
            QByteArray data = clientSocket->readAll();
            QString message = QString::fromUtf8(data);
            addLog(QString("📥 服务器收到: %1").arg(message.trimmed()));
            
            // 回显消息到所有其他客户端
            for (QTcpSocket *other : m_clients) {
                if (other != clientSocket && other->state() == QTcpSocket::ConnectedState) {
                    other->write(data);
                }
            }
        });
        
        connect(clientSocket, &QTcpSocket::disconnected, [this, clientSocket]() {
            m_clients.removeAll(clientSocket);
            clientSocket->deleteLater();
            addLog("❌ 客户端断开连接");
        });
        
        addLog("✅ 新客户端连接");
    }

private:
    void setupUI()
    {
        setWindowTitle("视频通话协议测试工具");
        resize(700, 500);
        
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        
        // 连接设置
        QHBoxLayout *connLayout = new QHBoxLayout();
        connLayout->addWidget(new QLabel("服务器:"));
        m_hostEdit = new QLineEdit("127.0.0.1");
        m_hostEdit->setMaximumWidth(100);
        connLayout->addWidget(m_hostEdit);
        
        connLayout->addWidget(new QLabel("端口:"));
        m_portEdit = new QLineEdit("8888");
        m_portEdit->setMaximumWidth(60);
        connLayout->addWidget(m_portEdit);
        
        m_connectBtn = new QPushButton("连接到服务器");
        connect(m_connectBtn, &QPushButton::clicked, this, &VideoCallProtocolTester::connectToServer);
        connLayout->addWidget(m_connectBtn);
        
        m_statusLabel = new QLabel("状态: 未连接");
        connLayout->addWidget(m_statusLabel);
        connLayout->addStretch();
        
        mainLayout->addLayout(connLayout);
        
        // 消息发送
        QHBoxLayout *msgLayout = new QHBoxLayout();
        msgLayout->addWidget(new QLabel("发送者ID:"));
        m_fromEdit = new QLineEdit("doctor001");
        m_fromEdit->setMaximumWidth(100);
        msgLayout->addWidget(m_fromEdit);
        
        msgLayout->addWidget(new QLabel("接收者ID:"));
        m_toEdit = new QLineEdit("patient001");
        m_toEdit->setMaximumWidth(100);
        msgLayout->addWidget(m_toEdit);
        msgLayout->addStretch();
        
        mainLayout->addLayout(msgLayout);
        
        // 按钮
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        
        QPushButton *requestBtn = new QPushButton("发送通话请求");
        connect(requestBtn, &QPushButton::clicked, this, &VideoCallProtocolTester::sendRequest);
        buttonLayout->addWidget(requestBtn);
        
        QPushButton *acceptBtn = new QPushButton("接受请求");
        connect(acceptBtn, &QPushButton::clicked, this, &VideoCallProtocolTester::sendAcceptResponse);
        buttonLayout->addWidget(acceptBtn);
        
        QPushButton *rejectBtn = new QPushButton("拒绝请求");
        connect(rejectBtn, &QPushButton::clicked, this, &VideoCallProtocolTester::sendRejectResponse);
        buttonLayout->addWidget(rejectBtn);
        
        QPushButton *endBtn = new QPushButton("结束通话");
        connect(endBtn, &QPushButton::clicked, this, &VideoCallProtocolTester::sendEndCall);
        buttonLayout->addWidget(endBtn);
        
        mainLayout->addLayout(buttonLayout);
        
        // 日志
        mainLayout->addWidget(new QLabel("通信日志:"));
        m_logEdit = new QTextEdit();
        m_logEdit->setFont(QFont("Consolas", 9));
        mainLayout->addWidget(m_logEdit);
        
        QPushButton *clearBtn = new QPushButton("清空日志");
        connect(clearBtn, &QPushButton::clicked, m_logEdit, &QTextEdit::clear);
        mainLayout->addWidget(clearBtn);
    }
    
    void setupServer()
    {
        m_server = new QTcpServer(this);
        connect(m_server, &QTcpServer::newConnection, this, &VideoCallProtocolTester::onNewConnection);
        
        if (m_server->listen(QHostAddress::Any, 8888)) {
            addLog("🚀 测试服务器启动成功，监听端口: 8888");
        } else {
            addLog("❌ 测试服务器启动失败");
        }
    }
    
    void sendMessage(const QString &message)
    {
        if (!m_socket || m_socket->state() != QTcpSocket::ConnectedState) {
            QMessageBox::warning(this, "错误", "未连接到服务器");
            return;
        }
        
        m_socket->write(message.toUtf8() + "\n");
        addLog(QString("📤 发送: %1").arg(message));
    }
    
    void addLog(const QString &message)
    {
        m_logEdit->append(QString("[%1] %2")
                         .arg(QTime::currentTime().toString("hh:mm:ss"))
                         .arg(message));
    }

private:
    QLineEdit *m_hostEdit;
    QLineEdit *m_portEdit;
    QLineEdit *m_fromEdit;
    QLineEdit *m_toEdit;
    QPushButton *m_connectBtn;
    QLabel *m_statusLabel;
    QTextEdit *m_logEdit;
    
    QTcpSocket *m_socket = nullptr;
    QTcpServer *m_server = nullptr;
    QList<QTcpSocket*> m_clients;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    VideoCallProtocolTester tester;
    tester.show();
    
    return app.exec();
}

#include "protocol_tester.moc"
