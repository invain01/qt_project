#include "server.h"
#include "ClientHandlerThread.h"
#include <QJsonObject>
#include <QJsonDocument>

Server::Server(QObject *parent) : QObject(parent)
{
    m_server = new QTcpServer(this);
}

void Server::initializeDatabase()
{
    // 设置数据库连接
    m_db = QSqlDatabase::addDatabase("QSQLITE", "server_connection");
    QString dbPath = "C:\\Users\\invain\\Desktop\\smart_medical_care-master\\Server\\database.db";

    m_db.setDatabaseName(dbPath);


    if (!m_db.open()) {
        qDebug() << "Server: Database error:" << m_db.lastError().text();
        return;
    }

    // 创建必要的表
    QSqlQuery query(m_db);

    // 创建新的user表（添加额外字段）
    query.exec("DROP TABLE IF EXISTS user");
    query.exec("CREATE TABLE IF NOT EXISTS user ("
               "id TEXT PRIMARY KEY,"
               "username TEXT NOT NULL,"
               "password TEXT NOT NULL,"
               "avatar_path TEXT NOT NULL,"
               "real_name TEXT NOT NULL CHECK(real_name GLOB '*[一-龥]*'),"  // 确保包含至少一个汉字
               "birth_date TEXT NOT NULL,"  // 格式: YYYY-MM-DD
               "id_card TEXT NOT NULL UNIQUE CHECK(LENGTH(id_card) = 18),"  // 18位身份证号
               "phone TEXT NOT NULL UNIQUE CHECK(LENGTH(phone) = 11 AND phone GLOB '[0-9]*'),"  // 11位数字
               "email TEXT NOT NULL UNIQUE CHECK(email LIKE '%@%.%')"  // 包含@和.
               ")");

    // 插入示例用户（添加额外字段）
    QVector<QStringList> users = {
        // id, username, password, avatar_path, real_name, birth_date, id_card, phone, email
        {"110001", "张三", "123", "1.jpeg", "张三", "1990-01-01", "110101199001011234", "13800138001", "zhangsan@example.com"},
        {"110002", "李四", "123", "2.jpeg", "李四", "1992-05-15", "210102199205152345", "13900139002", "lisi@example.com"},
        {"110003", "王五", "123", "3.jpeg", "王五", "1988-11-30", "310103198811303456", "13700137003", "wangwu@example.com"},
        {"120001", "张三1", "123", "4.jpeg", "张医生", "1985-03-22", "420104198503224567", "13600136004", "doctor1@hospital.com"},
        {"120002", "李四1", "123", "5.jpeg", "李医生", "1982-07-18", "510105198207185678", "13500135005", "doctor2@hospital.com"},
        {"120003", "王五1", "123", "6.jpeg", "王医生", "1979-09-09", "610106197909096789", "13400134006", "doctor3@hospital.com"}
    };

    foreach (const QStringList &user, users) {
        QSqlQuery insertQuery(m_db);
        insertQuery.prepare("INSERT INTO user "
                            "(id, username, password, avatar_path, real_name, birth_date, id_card, phone, email) "
                            "VALUES (:id, :username, :password, :avatar_path, :real_name, :birth_date, :id_card, :phone, :email)");

        insertQuery.bindValue(":id", user[0]);
        insertQuery.bindValue(":username", user[1]);
        insertQuery.bindValue(":password", user[2]);
        insertQuery.bindValue(":avatar_path", user[3]);
        insertQuery.bindValue(":real_name", user[4]);    // 真实姓名（汉字）
        insertQuery.bindValue(":birth_date", user[5]);   // 出生日期
        insertQuery.bindValue(":id_card", user[6]);       // 身份证号
        insertQuery.bindValue(":phone", user[7]);         // 手机号
        insertQuery.bindValue(":email", user[8]);        // 邮箱

        if (!insertQuery.exec()) {
            qDebug() << "用户插入失败:" << insertQuery.lastError().text()
                << "| SQL:" << insertQuery.lastQuery();
        }
    }


}

void Server::handleClientData()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    QByteArray data = clientSocket->readAll();
    QString message = QString::fromUtf8(data);

    qDebug() << "Received from client:" << message;

    // 解析消息类型
    QString messageType = message.section('#', 0, 0);

    if (messageType == "LOGIN") {
        handleLogIn(message, clientSocket);
    }
    else if (messageType == "USERINFO") {
        QString userId = message.section('#', 1, 1);
        handleUserInfoRequest(userId, clientSocket);
    }
    else if (m_connectedClients.contains(clientSocket)) {
        // 获取发送者用户名
        QString sender = m_connectedClients[clientSocket];

        if (messageType == "0" || messageType == "1" || messageType == "2") {
            // 消息格式: type#sender#content#receiver
            QString receiver = message.section('#', 3, 3);
            QString content = message.section('#', 2, 2);

            // 存储到数据库
            QSqlQuery query(m_db);
            query.prepare("INSERT INTO chat_history (sender, receiver, content, contentType) "
                          "VALUES (:sender, :receiver, :content, :type)");
            query.bindValue(":sender", sender);
            query.bindValue(":receiver", receiver);
            query.bindValue(":content", content);
            query.bindValue(":type", messageType);
            query.exec();

            // 转发给目标客户端
            foreach (QTcpSocket *socket, m_connectedClients.keys()) {
                if (m_connectedClients[socket] == receiver) {
                    socket->write(data);
                    qDebug() << "Message forwarded to:" << receiver;
                    break;
                }
            }
        }
    }
}

void Server::handleUserInfoRequest(const QString &userId, QTcpSocket *clientSocket)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM user WHERE id = :userId");
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        qDebug() << "用户信息查询失败:" << query.lastError().text();
        clientSocket->write("USERINFO_FAIL");
        return;
    }

    if (query.next()) {
        // 创建JSON对象
        QJsonObject userJson;
        userJson["id"] = query.value("id").toString();
        userJson["username"] = query.value("username").toString();
        userJson["real_name"] = query.value("real_name").toString();
        userJson["birth_date"] = query.value("birth_date").toString();
        userJson["id_card"] = query.value("id_card").toString();
        userJson["phone"] = query.value("phone").toString();
        userJson["email"] = query.value("email").toString();

        // 转换为JSON文档
        QJsonDocument doc(userJson);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

        // 发送响应
        clientSocket->write("USERINFO_SUCCESS#" + jsonData);
        qDebug() << "已发送用户信息给用户:" << userId;
    } else {
        clientSocket->write("USERINFO_FAIL#USER_NOT_FOUND");
        qDebug() << "用户不存在:" << userId;
    }
}

void Server::handleContactListRequest(QTcpSocket *clientSocket)
{
    // 获取当前登录用户的ID
    QString userId = m_connectedClients.value(clientSocket);
    if (userId.isEmpty()) {
        qDebug() << "未找到用户ID，无法获取联系人列表";
        return;
    }

    QSqlQuery query(m_db);
    // 修改为JOIN查询，获取联系人的详细信息
    query.prepare(
        "SELECT u.username, u.avatar_path "
        "FROM contacts c "
        "JOIN user u ON c.contact_user_id = u.id "
        "WHERE c.user_id = :userId"
        );
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        qDebug() << "联系人查询失败:" << query.lastError().text();
        return;
    }

    QStringList contacts;
    while (query.next()) {
        QString username = query.value(0).toString();
        QString avatarPath = query.value(1).toString();
        // 格式：用户名,头像路径
        contacts.append(QString("%1,%2").arg(username).arg(avatarPath));
    }

    QString response = "CONTACTS#" + contacts.join("|");
    clientSocket->write(response.toUtf8());
    qDebug() << "已发送联系人列表给用户" << userId << "数量:" << contacts.size();
}

void Server::handleLogIn(QString message, QTcpSocket *clientSocket)
{
    // 登录请求格式: LOGIN#id#password
    QString id = message.section('#', 1, 1);
    QString password = message.section('#', 2, 2);

    // 验证用户
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM user WHERE id = :id AND password = :password");
    query.bindValue(":id", id);
    query.bindValue(":password", password);

    if (query.exec() && query.next()) {
        m_connectedClients.insert(clientSocket, id);
        clientSocket->write("LOGIN_SUCCESS");
        qDebug() << "Login success:" << id;
    } else {
        clientSocket->write("LOGIN_FAIL");
        qDebug() << "Login failed for:" << id;
    }
}

void Server::start()
{
    initializeDatabase();  // 初始化数据库

    quint16 port = 8888;
    if (!m_server->listen(QHostAddress::Any, port)) {
        qDebug() << "Server could not start:" << m_server->errorString();
        return;
    }

    qDebug() << "Server listening on port" << port;

    connect(m_server, &QTcpServer::newConnection, this, &Server::handleNewConnection);
}

void Server::handleNewConnection()
{
    QTcpSocket *clientSocket = m_server->nextPendingConnection();
    if (!clientSocket) return;

    qDebug() << "New connection from:" << clientSocket->peerAddress().toString();

    connect(clientSocket, &QTcpSocket::readyRead, this, &Server::handleClientData);
    connect(clientSocket, &QTcpSocket::disconnected, this, &Server::handleDisconnection);
}



void Server::handleDisconnection()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    QString username = m_connectedClients.value(clientSocket, "Unknown");
    qDebug() << "Client disconnected:" << username;

    m_connectedClients.remove(clientSocket);
    clientSocket->deleteLater();
}
