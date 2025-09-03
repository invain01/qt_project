#include "server.h"
#include "ClientHandlerThread.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSet>
#include <QCoreApplication> // 包含QCoreApplication类的定义

Server::Server(QObject *parent) : QObject(parent)
{
    m_server = new QTcpServer(this);
}

void Server::initializeDatabase()
{
    // 设置数据库连接
    QString dbPath = "database.db";
    //QString dbPath = QCoreApplication::applicationDirPath() + "/../../database.db";

    // 添加SQLite驱动
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);


    if (!m_db.open()) {
        qDebug() << "Server: Database error:" << m_db.lastError().text();
        return;
    }

    // 在Server::initializeDatabase()函数中添加以下代码

    // 启用外键支持（SQLite默认关闭）
    if (!m_db.exec("PRAGMA foreign_keys = ON;").isActive()) {
        qDebug() << "启用外键支持失败:" << m_db.lastError().text();
    }

    // 创建必要的表
    QSqlQuery query(m_db);

    // //创建新的user表（添加额外字段）
    query.exec("DROP TABLE IF EXISTS user");
    query.exec("DROP TABLE IF EXISTS patient");
    query.exec("DROP TABLE IF EXISTS doctor");
    query.exec("DROP TABLE IF EXISTS attendance");
    query.exec("DROP TABLE IF EXISTS appointment");
    query.exec("DROP TABLE IF EXISTS leave");
    query.exec("DROP TABLE IF EXISTS message");

    query.exec("CREATE TABLE IF NOT EXISTS user ("
               "id TEXT PRIMARY KEY,"
               "username TEXT NOT NULL,"
               "password TEXT NOT NULL,"
               "avatar_path TEXT NOT NULL,"
               "real_name TEXT NOT NULL CHECK(real_name GLOB '*[一-龥]*'),"  // 确保包含至少一个汉字
               "birth_date TEXT NOT NULL,"  // 格式: YYYY-MM-DD
               "id_card TEXT NOT NULL,"  // 18位身份证号
               "phone TEXT NOT NULL,"  // 11位数字
               "email TEXT NOT NULL"  // 包含@和.
               ")");

    // 插入示例用户（添加额外字段）
    QVector<QStringList> users = {
        // id, username, password, avatar_path, real_name, birth_date, id_card, phone, email
        // 病人用户
        {"110001", "张三", "123", "1.jpeg", "张三", "1990-01-01", "110101199001011234", "13800138001", "zhangsan@example.com"},
        {"110002", "李四", "123", "2.jpeg", "李四", "1992-05-15", "210102199205152345", "13900139002", "lisi@example.com"},
        {"110003", "王五", "123", "3.jpeg", "王五", "1988-11-30", "310103198811303456", "13700137003", "wangwu@example.com"},
        {"110004", "刘六", "123", "7.jpeg", "刘六", "1985-08-12", "420104198508123456", "13600136007", "liuliu@example.com"},
        {"110005", "陈七", "123", "8.jpeg", "陈七", "1978-03-25", "510105197803253456", "13500135008", "chenqi@example.com"},
        {"110006", "赵八", "123", "9.jpeg", "赵八", "1995-11-08", "610106199511083456", "13400134009", "zhaoba@example.com"},
        {"110007", "钱九", "123", "10.jpeg", "钱九", "1982-07-19", "710107198207193456", "13300133010", "qianjiu@example.com"},
        {"110008", "孙十", "123", "11.jpeg", "孙十", "1991-02-14", "810108199102143456", "13200132011", "sunshi@example.com"},
        {"110009", "周十一", "123", "12.jpeg", "周十一", "1987-09-30", "910109198709303456", "13100131012", "zhoushiyi@example.com"},
        {"110010", "吴十二", "123", "13.jpeg", "吴十二", "1980-12-05", "101010198012053456", "13000130013", "wushier@example.com"},

        // 医生用户
        {"120001", "张三1", "123", "4.jpeg", "张医生", "1985-03-22", "420104198503224567", "13600136004", "doctor1@hospital.com"},
        {"120002", "李四1", "123", "5.jpeg", "李医生", "1982-07-18", "510105198207185678", "13500135005", "doctor2@hospital.com"},
        {"120003", "王五1", "123", "6.jpeg", "王医生", "1979-09-09", "610106197909096789", "13400134006", "doctor3@hospital.com"},
        {"120004", "刘六1", "123", "14.jpeg", "刘医生", "1976-04-15", "710107197604156789", "13300133014", "doctor4@hospital.com"},
        {"120005", "陈七1", "123", "15.jpeg", "陈医生", "1980-08-28", "810108198008286789", "13200132015", "doctor5@hospital.com"},
        {"120006", "赵八1", "123", "16.jpeg", "赵医生", "1978-12-10", "910109197812106789", "13100131016", "doctor6@hospital.com"},
        {"120007", "钱九1", "123", "17.jpeg", "钱医生", "1983-06-20", "101010198306206789", "13000130017", "doctor7@hospital.com"},
        {"120008", "孙十1", "123", "18.jpeg", "孙医生", "1975-10-05", "111111197510056789", "12900129018", "doctor8@hospital.com"},
        {"120009", "周十一1", "123", "19.jpeg", "周医生", "1987-02-18", "121212198702186789", "12800128019", "doctor9@hospital.com"},
        {"120010", "吴十二1", "123", "20.jpeg", "吴医生", "1981-07-22", "131313198107226789", "12700127020", "doctor10@hospital.com"}
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



    // 创建病人表
    query.exec("CREATE TABLE IF NOT EXISTS patient ("
               "id TEXT PRIMARY KEY,"
               "case_info TEXT,"
               "FOREIGN KEY(id) REFERENCES user(id) ON DELETE CASCADE"
               ")");

    // 创建医生表
    query.exec("CREATE TABLE IF NOT EXISTS doctor ("
               "id TEXT PRIMARY KEY,"
               "department TEXT NOT NULL,"       // 科室
               "title TEXT NOT NULL,"            // 职称
               "introduction TEXT NOT NULL,"     // 个人简介
               "FOREIGN KEY(id) REFERENCES user(id) ON DELETE CASCADE"
               ")");

    // 创建预约表
    query.exec("CREATE TABLE IF NOT EXISTS appointment ("
               "appointment_id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "patient_id TEXT NOT NULL,"       // 病人ID
               "doctor_id TEXT NOT NULL,"         // 医生ID
               "appointment_date DATETIME NOT NULL," // 预约日期时间
               "status TEXT DEFAULT 'pending',"   // 预约状态
               "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
               "FOREIGN KEY(patient_id) REFERENCES patient(id) ON DELETE CASCADE,"
               "FOREIGN KEY(doctor_id) REFERENCES doctor(id) ON DELETE CASCADE"
               ")");

    // 创建考勤表
    query.exec("CREATE TABLE IF NOT EXISTS attendance ("
               "attendance_id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "doctor_id TEXT NOT NULL,"         // 医生ID
               "date DATE NOT NULL,"              // 考勤日期
               "check_in_time TIME,"              // 签到时间
               "check_out_time TIME,"             // 签退时间
               "status TEXT CHECK(status IN ('normal', 'late', 'early_leave', 'absent')) NOT NULL,"
               "FOREIGN KEY(doctor_id) REFERENCES doctor(id) ON DELETE CASCADE"
               ")");

    // 创建请假表
    query.exec("CREATE TABLE IF NOT EXISTS leave ("
               "leave_id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "doctor_id TEXT NOT NULL,"         // 医生ID
               "leave_type TEXT NOT NULL,"        // 请假类型
               "start_date DATE NOT NULL,"        // 开始日期
               "end_date DATE NOT NULL,"          // 结束日期
               "reason TEXT NOT NULL,"            // 请假原因
               "status TEXT CHECK(status IN ('applied', 'approved', 'rejected')) DEFAULT 'applied',"
               "applied_date DATETIME DEFAULT CURRENT_TIMESTAMP," // 申请日期
               "FOREIGN KEY(doctor_id) REFERENCES doctor(id) ON DELETE CASCADE"
               ")");

    // 创建聊天信息表
    query.exec("CREATE TABLE IF NOT EXISTS message ("
               "message_id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "sender_id TEXT NOT NULL,"         // 发送者ID
               "receiver_id TEXT NOT NULL,"       // 接收者ID
               "content TEXT NOT NULL,"           // 消息内容
               "send_time DATETIME DEFAULT CURRENT_TIMESTAMP," // 发送时间
               "FOREIGN KEY(sender_id) REFERENCES user(id) ON DELETE CASCADE,"
               "FOREIGN KEY(receiver_id) REFERENCES user(id) ON DELETE CASCADE"
               ")");


    // 创建药品表
    query.exec("CREATE TABLE IF NOT EXISTS medicine ("
               "medicine_id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "name TEXT NOT NULL,"              // 药品名称
               "dosage_form TEXT NOT NULL,"       // 剂型
               "specification TEXT NOT NULL,"     // 规格
               "manufacturer TEXT NOT NULL,"      // 生产厂家
               "price REAL NOT NULL,"             // 价格
               "description TEXT NOT NULL,"       // 药品描述
               "usage TEXT NOT NULL,"             // 使用方法
               "indications TEXT NOT NULL,"       // 适应症
               "contraindications TEXT NOT NULL,"  // 禁忌症
               "side_effects TEXT NOT NULL,"      // 副作用
               "storage TEXT NOT NULL DEFAULT ''," // 存储条件
               "expiry_date TEXT NOT NULL DEFAULT '2099-12-31'" // 有效期
               ")");

    query.exec("DROP TABLE IF EXISTS hospitalization_application");
    query.exec("DROP TABLE IF EXISTS payment_items");
    query.exec("DROP TABLE IF EXISTS payment_records");
    // 创建住院申请表
    query.exec("CREATE TABLE IF NOT EXISTS hospitalization_application ("
               "application_id TEXT PRIMARY KEY,"
               "patient_id TEXT NOT NULL,"
               "patient_name TEXT NOT NULL,"
               "department TEXT NOT NULL,"
               "doctor TEXT NOT NULL,"
               "admission_date TEXT NOT NULL,"
               "symptoms TEXT NOT NULL,"
               "diagnosis TEXT NOT NULL,"
               "fee REAL NOT NULL,"
               "status TEXT NOT NULL,"
               "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
               "FOREIGN KEY(patient_id) REFERENCES user(id) ON DELETE CASCADE"
               ")");

    // 创建缴费项目表
    query.exec("CREATE TABLE IF NOT EXISTS payment_items ("
               "item_id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "patient_id TEXT NOT NULL,"
               "description TEXT NOT NULL,"
               "amount REAL NOT NULL,"
               "status TEXT NOT NULL,"
               "type TEXT NOT NULL,"
               "application_id TEXT,"
               "created_at TEXT NOT NULL,"
               "paid_at TEXT,"
               "FOREIGN KEY(patient_id) REFERENCES user(id) ON DELETE CASCADE"
               ")");

    // 创建缴费记录表
    query.exec("CREATE TABLE IF NOT EXISTS payment_records ("
               "record_id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "patient_id TEXT NOT NULL,"
               "total_amount REAL NOT NULL,"
               "payment_time TEXT NOT NULL,"
               "payment_method TEXT NOT NULL,"
               "FOREIGN KEY(patient_id) REFERENCES user(id) ON DELETE CASCADE"
               ")");

    // payment_records
    QSqlQuery checkTable1(m_db);
    if (checkTable1.exec("PRAGMA table_info(payment_records)")) {
        while (checkTable1.next()) {
            qDebug() << "列名:" << checkTable1.value(1).toString()
                << "类型:" << checkTable1.value(2).toString();
        }
    }
    QSqlQuery checkTable2(m_db);
    if (checkTable2.exec("PRAGMA table_info(payment_items)")) {
        while (checkTable2.next()) {
            qDebug() << "列名:" << checkTable2.value(1).toString()
                << "类型:" << checkTable2.value(2).toString();
        }
    }
    QSqlQuery checkTable3(m_db);
    if (checkTable3.exec("PRAGMA table_info(hospitalization_application)")) {
        while (checkTable3.next()) {
            qDebug() << "列名:" << checkTable3.value(1).toString()
                << "类型:" << checkTable3.value(2).toString();
        }
    }

    query.exec("DROP TABLE IF EXISTS prescription");
    query.exec("CREATE TABLE IF NOT EXISTS prescription ("
               "prescription_id INTEGER PRIMARY KEY AUTOINCREMENT,"  // 处方唯一ID，自增
               "patient_id TEXT NOT NULL,"         // 患者ID，外键关联patient表
               "doctor_id TEXT NOT NULL,"          // 医生ID，外键关联doctor表
               "medicine_name TEXT NOT NULL,"      // 药品名称
               "dosage TEXT NOT NULL,"             // 剂量信息
               "usage TEXT NOT NULL,"              // 用法说明
               "frequency TEXT NOT NULL,"          // 服用频次
               "quantity INTEGER NOT NULL DEFAULT 1," // 购买数量
               "notes TEXT NOT NULL DEFAULT '',"   // 备注信息
               "prescribed_date DATETIME DEFAULT CURRENT_TIMESTAMP," // 开具时间，自动记录
               "status TEXT DEFAULT 'active',"     // 处方状态：active/completed/cancelled
               "FOREIGN KEY(patient_id) REFERENCES patient(id) ON DELETE CASCADE,"  // 外键约束
               "FOREIGN KEY(doctor_id) REFERENCES doctor(id) ON DELETE CASCADE"
               ")");

    // ================== 插入测试数据 ==================

    // 插入病人测试数据
    QVector<QStringList> patients = {
        // id, case_info
        {"110001", "高血压病史5年，近期血压不稳定，需要定期服药监测"},
        {"110002", "糖尿病II型，需要定期监测血糖，饮食控制严格"},
        {"110003", "慢性胃炎，需定期复查胃镜，避免辛辣刺激食物"},
        {"110004", "哮喘病史3年，对花粉尘螨过敏，随身携带喷雾剂"},
        {"110005", "腰椎间盘突出，需要物理治疗和定期复查"},
        {"110006", "甲状腺功能亢进，需要定期检查甲状腺激素水平"},
        {"110007", "冠心病，支架术后需要长期服药和定期复查"},
        {"110008", "抑郁症病史，需要定期心理咨询和药物治疗"},
        {"110009", "过敏性鼻炎，季节性发作需要抗过敏治疗"},
        {"110010", "骨质疏松，需要补钙和定期骨密度检查"}
    };

    foreach (const QStringList &patient, patients) {
        query.prepare("INSERT INTO patient (id, case_info) VALUES (:id, :case_info)");
        query.bindValue(":id", patient[0]);
        query.bindValue(":case_info", patient[1]);
        if (!query.exec()) {
            qDebug() << "病人插入失败:" << query.lastError().text();
        }
    }

    // 先检查doctor表结构
    QSqlQuery checkTable(m_db);
    if (checkTable.exec("PRAGMA table_info(doctor)")) {
        while (checkTable.next()) {
            qDebug() << "列名:" << checkTable.value(1).toString()
                << "类型:" << checkTable.value(2).toString();
        }
    }

    // 然后检查registration_fee列是否存在
    QSqlQuery checkColumn(m_db);
    checkColumn.prepare("SELECT COUNT(*) FROM pragma_table_info('doctor') WHERE name='registration_fee'");
    if (checkColumn.exec() && checkColumn.next()) {
        if (checkColumn.value(0).toInt() == 0) {
            qDebug() << "错误: registration_fee列不存在";
            // 可能需要先添加列
            QSqlQuery alterQuery(m_db);
            alterQuery.exec("ALTER TABLE doctor ADD COLUMN registration_fee REAL DEFAULT 50.0");
        }
    }

    // 插入医生测试数据
    QVector<QStringList> doctors = {
        // id, department, title, introduction, registration_fee
        {"120001", "心血管内科", "主任医师", "毕业于XX医科大学，擅长高血压、冠心病诊疗，20年临床经验", "100.0"},
        {"120002", "内分泌科", "副主任医师", "糖尿病专家，发表SCI论文10余篇，擅长糖尿病并发症治疗", "80.0"},
        {"120003", "消化内科", "主治医师", "胃肠镜操作专家，年完成胃镜手术千余例，擅长消化道疾病诊治", "50.0"},
        {"120004", "呼吸内科", "主任医师", "擅长哮喘、COPD等呼吸系统疾病，15年临床经验", "50.0"},
        {"120005", "骨科", "副主任医师", "擅长关节置换和脊柱手术，微创手术专家", "70.0"},
        {"120006", "神经内科", "主治医师", "擅长脑血管疾病和神经系统疑难病症诊治", "200.0"},
        {"120007", "皮肤科", "主任医师", "擅长湿疹、银屑病等皮肤疾病，中西医结合治疗", "85.0"},
        {"120008", "眼科", "副主任医师", "擅长白内障手术和眼底疾病诊治", "50.0"},
        {"120009", "耳鼻喉科", "主治医师", "擅长鼻窦炎、中耳炎等耳鼻喉疾病治疗", "50.0"},
        {"120010", "心理科", "主任医师", "国家二级心理咨询师，擅长抑郁症、焦虑症治疗", "150.0"}
    };

    foreach (const QStringList &doctor, doctors) {
        QSqlQuery query(m_db);
        query.prepare("INSERT INTO doctor (id, department, title, introduction, registration_fee) "
                      "VALUES (:id, :department, :title, :introduction, :registration_fee)");
        query.bindValue(":id", doctor[0]);
        query.bindValue(":department", doctor[1]);
        query.bindValue(":title", doctor[2]);
        query.bindValue(":introduction", doctor[3]);
        query.bindValue(":registration_fee", doctor[4].toDouble());
        if (!query.exec()) {
            qDebug() << "医生插入失败:" << query.lastError().text()
                << "| SQL:" << query.lastQuery()
                << "| Values:" << doctor;
        }
    }

    // 插入预约测试数据
    QVector<QStringList> appointments = {
        // patient_id, doctor_id, appointment_date, status
        {"110001", "120001", "2023-08-15 09:30:00", "pending"},
        {"110002", "120002", "2023-08-16 10:00:00", "pending"},
        {"110003", "120003", "2023-08-17 14:30:00", "cancelled"},
        {"110004", "120004", "2023-08-18 08:30:00", "confirmed"},
        {"110005", "120005", "2023-08-19 14:00:00", "pending"},
        {"110006", "120006", "2023-08-20 10:30:00", "confirmed"},
        {"110007", "120007", "2023-08-21 09:00:00", "cancelled"},
        {"110008", "120008", "2023-08-22 15:30:00", "pending"},
        {"110009", "120009", "2023-08-23 11:00:00", "confirmed"},
        {"110010", "120010", "2023-08-24 16:00:00", "pending"},
        {"110001", "120004", "2023-08-25 09:30:00", "confirmed"},
        {"110002", "120005", "2023-08-26 14:00:00", "pending"},
        {"110003", "120006", "2023-08-27 10:30:00", "confirmed"},
        {"110004", "120007", "2023-08-28 09:00:00", "cancelled"},
        {"110005", "120008", "2023-08-29 15:30:00", "pending"},
        {"110006", "120009", "2023-08-30 11:00:00", "confirmed"},
        {"110007", "120010", "2023-08-31 16:00:00", "pending"},
        {"110008", "120001", "2023-09-01 09:30:00", "confirmed"},
        {"110009", "120002", "2023-09-02 10:00:00", "pending"},
        {"110010", "120003", "2023-09-03 14:30:00", "cancelled"}
    };

    foreach (const QStringList &appointment, appointments) {
        query.prepare("INSERT INTO appointment (patient_id, doctor_id, appointment_date, status) "
                      "VALUES (:patient_id, :doctor_id, :appointment_date, :status)");
        query.bindValue(":patient_id", appointment[0]);
        query.bindValue(":doctor_id", appointment[1]);
        query.bindValue(":appointment_date", appointment[2]);
        query.bindValue(":status", appointment[3]);
        if (!query.exec()) {
            qDebug() << "预约插入失败:" << query.lastError().text();
        }
    }

    // 插入考勤测试数据
    QVector<QStringList> attendances = {
        // doctor_id, date, check_in_time, check_out_time, status
        {"120001", "2025-08-01", "08:05:00", "17:30:00", "normal"},
        {"120002", "2023-08-01", "08:45:00", "17:00:00", "late"},
        {"120003", "2023-08-01", "08:10:00", "16:00:00", "early_leave"},
        {"120004", "2023-08-01", "08:00:00", "17:00:00", "normal"},
        {"120005", "2023-08-01", "08:20:00", "17:15:00", "normal"},
        {"120006", "2023-08-01", "08:30:00", "16:45:00", "normal"},
        {"120007", "2023-08-01", "08:10:00", "17:20:00", "normal"},
        {"120008", "2023-08-01", "08:25:00", "16:50:00", "normal"},
        {"120009", "2023-08-01", "08:15:00", "17:10:00", "normal"},
        {"120010", "2023-08-01", "08:40:00", "17:05:00", "late"},
        {"120001", "2025-08-02", "08:00:00", "17:00:00", "early_leave"},
        {"120002", "2023-08-02", "08:10:00", "16:55:00", "normal"},
        {"120003", "2023-08-02", "08:05:00", "16:30:00", "early_leave"},
        {"120004", "2023-08-02", "08:20:00", "17:10:00", "normal"},
        {"120005", "2023-08-02", "08:30:00", "17:20:00", "normal"},
        {"120006", "2023-08-02", "08:15:00", "16:40:00", "early_leave"},
        {"120007", "2023-08-02", "08:25:00", "17:15:00", "normal"},
        {"120008", "2023-08-02", "08:35:00", "17:05:00", "normal"},
        {"120009", "2023-08-02", "08:10:00", "17:00:00", "normal"},
        {"120010", "2023-08-02", "08:50:00", "17:25:00", "late"}
    };

    foreach (const QStringList &attendance, attendances) {
        query.prepare("INSERT INTO attendance (doctor_id, date, check_in_time, check_out_time, status) "
                      "VALUES (:doctor_id, :date, :check_in_time, :check_out_time, :status)");
        query.bindValue(":doctor_id", attendance[0]);
        query.bindValue(":date", attendance[1]);
        query.bindValue(":check_in_time", attendance[2]);
        query.bindValue(":check_out_time", attendance[3]);
        query.bindValue(":status", attendance[4]);
        if (!query.exec()) {
            qDebug() << "考勤插入失败:" << query.lastError().text();
        }
    }


    // 插入请假测试数据
    QVector<QStringList> leaves = {
        // doctor_id, leave_type, start_date, end_date, reason, status
        {"120001", "年假", "2023-08-10", "2023-08-12", "家庭旅行", "approved"},
        {"120002", "病假", "2023-08-15", "2023-08-16", "重感冒需休息", "applied"},
        {"120003", "事假", "2023-08-20", "2023-08-21", "参加学术会议", "rejected"},
        {"120004", "年假", "2023-08-05", "2023-08-07", "回乡探亲", "approved"},
        {"120005", "病假", "2023-08-12", "2023-08-13", "急性肠胃炎", "approved"},
        {"120006", "事假", "2023-08-18", "2023-08-19", "孩子家长会", "applied"},
        {"120007", "年假", "2023-08-25", "2023-08-27", "短期旅行", "approved"},
        {"120008", "病假", "2023-08-14", "2023-08-15", "牙痛需要治疗", "approved"},
        {"120009", "事假", "2023-08-22", "2023-08-23", "办理房产手续", "rejected"},
        {"120010", "年假", "2023-08-28", "2023-08-30", "个人休息", "applied"},
        {"120001", "病假", "2023-09-05", "2023-09-06", "身体不适需要检查", "applied"},
        {"120002", "事假", "2023-09-10", "2023-09-11", "参加朋友婚礼", "approved"},
        {"120003", "年假", "2023-09-15", "2023-09-17", "短途旅行", "applied"},
        {"120004", "病假", "2023-09-20", "2023-09-21", "感冒发烧", "approved"},
        {"120005", "事假", "2023-09-25", "2023-09-26", "车辆年检", "rejected"},
        {"120006", "年假", "2023-09-28", "2023-09-30", "国庆节前休息", "approved"},
        {"120007", "病假", "2023-10-05", "2023-10-06", "腰部不适需要理疗", "applied"},
        {"120008", "事假", "2023-10-10", "2023-10-11", "办理银行业务", "approved"},
        {"120009", "年假", "2023-10-15", "2023-10-17", "陪伴家人", "applied"},
        {"120010", "病假", "2023-10-20", "2023-10-21", "过敏反应需要休息", "approved"}
    };

    foreach (const QStringList &leave, leaves) {
        query.prepare("INSERT INTO leave (doctor_id, leave_type, start_date, end_date, reason, status) "
                      "VALUES (:doctor_id, :leave_type, :start_date, :end_date, :reason, :status)");
        query.bindValue(":doctor_id", leave[0]);
        query.bindValue(":leave_type", leave[1]);
        query.bindValue(":start_date", leave[2]);
        query.bindValue(":end_date", leave[3]);
        query.bindValue(":reason", leave[4]);
        query.bindValue(":status", leave[5]);
        if (!query.exec()) {
            qDebug() << "请假插入失败:" << query.lastError().text();
        }
    }

    // 插入聊天信息测试数据
    QVector<QStringList> messages = {
        // sender_id, receiver_id, content
        {"110001", "120001", "张医生您好，我昨天血压有点高，150/95，需要调整药物吗？"},
        {"120001", "110001", "收到，建议今天再测量两次，如果持续偏高，可以考虑加半片硝苯地平"},
        {"110002", "120002", "李医生，我今早空腹血糖7.8，需要加药吗？"},
        {"120002", "110002", "血糖7.8稍高，建议先饮食控制，明天再测空腹血糖看看"},
        {"110003", "120003", "王医生，我胃痛又犯了，需要提前来复查吗？"},
        {"120003", "110003", "如果疼痛持续，建议明天来医院做个胃镜检查"},
        {"110004", "120004", "刘医生，我最近哮喘发作比较频繁，需要调整用药吗？"},
        {"120004", "110004", "建议增加喷雾剂使用频率，如果不见好转请来医院复查"},
        {"110005", "120005", "陈医生，我的腰最近又疼了，需要来做理疗吗？"},
        {"120005", "110005", "可以安排本周五下午来做物理治疗，记得带医保卡"},
        {"110006", "120006", "赵医生，我最近头晕的厉害，需要检查什么？"},
        {"120006", "110006", "建议做头颅CT和血压监测，明天上午可以来检查"},
        {"110007", "120007", "钱医生，我皮肤过敏很严重，需要开什么药？"},
        {"120007", "110007", "可以先用氯雷他定，如果不见效明天来医院开处方药"},
        {"110008", "120008", "孙医生，我眼睛最近很干涩，需要用什么眼药水？"},
        {"120008", "110008", "建议使用人工泪液，每天4-6次，避免长时间用眼"},
        {"110009", "120009", "周医生，我鼻炎又犯了，打喷嚏流鼻涕很难受"},
        {"120009", "110009", "可以用鼻喷雾剂控制症状，严重时来医院开口服药"},
        {"110010", "120010", "吴医生，我最近睡眠很差，情绪低落，需要调整药物吗？"},
        {"120010", "110010", "建议本周三下午来复诊，我们需要调整抗抑郁药物的剂量"}
    };

    foreach (const QStringList &message, messages) {
        query.prepare("INSERT INTO message (sender_id, receiver_id, content) "
                      "VALUES (:sender_id, :receiver_id, :content)");
        query.bindValue(":sender_id", message[0]);
        query.bindValue(":receiver_id", message[1]);
        query.bindValue(":content", message[2]);
        if (!query.exec()) {
            qDebug() << "聊天信息插入失败:" << query.lastError().text();
        }
    }
    /*
    // 添加处方测试数据
    QVector<QStringList> prescriptions = {
        // patient_id, doctor_id, medicine_name, dosage, usage, frequency, notes, status
        {"110001", "120001", "阿司匹林肠溶片", "100mg", "口服", "一日1次", "饭后服用", "active"},
        {"110001", "120001", "硝苯地平缓释片", "20mg", "口服", "一日2次", "早晚各一次", "completed"},
        {"110001", "120002", "盐酸二甲双胍片", "500mg", "口服", "一日3次", "随餐服用", "pending"},
        {"110002", "120003", "布洛芬缓释胶囊", "0.3g", "口服", "一日3次", "疼痛时服用", "active"},
        {"110002", "120004", "氯雷他定片", "10mg", "口服", "一日1次", "睡前服用", "completed"}
    };*/

    /*foreach (const QStringList &prescription, prescriptions) {
        query.prepare("INSERT INTO prescription (patient_id, doctor_id, medicine_name, dosage, usage, frequency, notes, status) "
                      "VALUES (:patient_id, :doctor_id, :medicine_name, :dosage, :usage, :frequency, :notes, :status)");
        query.bindValue(":patient_id", prescription[0]);
        query.bindValue(":doctor_id", prescription[1]);
        query.bindValue(":medicine_name", prescription[2]);
        query.bindValue(":dosage", prescription[3]);
        query.bindValue(":usage", prescription[4]);
        query.bindValue(":frequency", prescription[5]);
        query.bindValue(":notes", prescription[6]);
        query.bindValue(":status", prescription[7]);
        if (!query.exec()) {
            qDebug() << "处方插入失败:" << query.lastError().text();
        }
        else {
            qDebug() << "处方插入成功";
        }
    }
    */
    // 检查药品表中是否已有数据，避免重复插入
    query.exec("SELECT COUNT(*) FROM medicine");
    if (query.next()) {
        int medicineCount = query.value(0).toInt();
        qDebug() << "当前药品表中有" << medicineCount << "条记录";
        
        if (medicineCount > 0) {
            // 检查是否有大量重复数据（可能是之前的bug导致的）
            query.exec("SELECT COUNT(DISTINCT name) FROM medicine");
            if (query.next()) {
                int uniqueMedicineCount = query.value(0).toInt();
                qDebug() << "药品表中有" << uniqueMedicineCount << "种不同的药品";
                
                // 如果重复数据过多，清理数据库
                if (medicineCount > uniqueMedicineCount * 2) {
                    qDebug() << "检测到大量重复数据，清理药品表";
                    query.exec("DELETE FROM medicine");
                    qDebug() << "药品表已清空，重新插入数据";
                } else {
                    qDebug() << "药品数据正常，跳过插入";
                    return; // 如果数据正常，跳过插入
                }
            }
        }
    }

    qDebug() << "开始插入药品测试数据";
    QVector<QStringList> medicines = {
        // name, dosage_form, specification, manufacturer, price, description, usage, indications, contraindications, side_effects
        // 心血管类药物
        {"阿司匹林肠溶片", "片剂", "100mg*30片/盒", "拜耳医药保健有限公司", "28.5", "用于降低血栓形成风险，缓解轻至中度疼痛", "口服，一次1片，一日1次", "用于心肌梗死、脑梗死、缺血性脑血管病等", "对阿司匹林过敏者禁用；有出血倾向者禁用", "胃肠道不适、恶心、呕吐、胃痛等"},
        {"硝苯地平缓释片", "缓释片", "20mg*30片/盒", "拜耳医药保健有限公司", "45.0", "用于高血压、冠心病、心绞痛", "口服，一次1片，一日1-2次", "用于高血压、冠心病、慢性稳定型心绞痛", "对硝苯地平过敏者禁用；心源性休克患者禁用", "头痛、面部潮红、下肢水肿、头晕等"},
        {"辛伐他汀片", "片剂", "20mg*28片/盒", "杭州默沙东制药有限公司", "68.0", "用于高胆固醇血症、冠心病", "口服，一次1片，每晚1次", "用于高胆固醇血症、冠心病、脑梗死等", "活动性肝病患者禁用；孕妇及哺乳期妇女禁用", "腹痛、便秘、胃肠胀气、乏力等"},
        {"卡托普利片", "片剂", "25mg*24片/盒", "上海信谊药厂有限公司", "18.8", "血管紧张素转换酶抑制剂，用于高血压治疗", "口服，一次12.5-25mg，一日2-3次", "用于高血压、充血性心力衰竭", "孕妇禁用；对血管紧张素转换酶抑制剂过敏者禁用", "干咳、高钾血症、血管性水肿、皮疹等"},
        {"普罗帕酮片", "片剂", "150mg*20片/盒", "沈阳三生制药有限责任公司", "35.8", "抗心律失常药物，用于治疗室性和房性心律失常", "口服，一次150-300mg，一日3次", "用于室性早搏、房性早搏、阵发性室上性心动过速", "病态窦房结综合征禁用；严重心功能不全禁用", "头晕、视力模糊、恶心、口干、便秘等"},

        // 内分泌类药物
        {"盐酸二甲双胍片", "片剂", "500mg*30片/盒", "中美上海施贵宝制药有限公司", "38.0", "用于2型糖尿病，特别是肥胖型糖尿病患者", "口服，一次1-2片，一日3次，饭后服用", "用于单纯饮食控制不满意的2型糖尿病患者", "严重肝肾功能不全者禁用；糖尿病酮症酸中毒禁用", "恶心、呕吐、腹泻、口中金属味等"},
        {"格列美脲片", "片剂", "2mg*30片/盒", "赛诺菲(杭州)制药有限公司", "58.0", "磺脲类降糖药，用于2型糖尿病", "口服，一次1-2mg，一日1次，早餐前服用", "用于经饮食控制和运动治疗不满意的2型糖尿病", "1型糖尿病禁用；严重肝肾功能不全禁用", "低血糖、恶心、腹痛、腹泻、皮疹等"},
        {"胰岛素注射液", "注射液", "400IU/10ml*1支/盒", "诺和诺德(中国)制药有限公司", "48.5", "用于糖尿病患者的血糖控制", "皮下注射，按血糖情况调整剂量", "用于1型和2型糖尿病的血糖控制", "对胰岛素过敏者禁用", "注射部位反应、低血糖、体重增加等"},
        {"甲巯咪唑片", "片剂", "5mg*50片/盒", "上海信谊药厂有限公司", "12.8", "抗甲状腺药物，用于甲状腺功能亢进", "口服，一次5-10mg，一日3次", "用于甲状腺功能亢进症", "严重肝功能损害禁用；粒细胞缺乏症禁用", "皮疹、发热、关节痛、粒细胞减少等"},

        // 抗过敏类药物
        {"氯雷他定片", "片剂", "10mg*14片/盒", "上海先灵葆雅制药有限公司", "35.0", "用于缓解过敏性鼻炎、荨麻疹等症状", "口服，一次1片，一日1次", "用于过敏性鼻炎、慢性荨麻疹、瘙痒性皮肤病", "对氯雷他定过敏者禁用", "乏力、头痛、嗜睡、口干、胃肠道不适等"},
        {"西替利嗪片", "片剂", "10mg*12片/盒", "西安杨森制药有限公司", "28.0", "第二代抗组胺药，用于过敏性疾病", "口服，一次10mg，一日1次，晚上服用", "用于过敏性鼻炎、慢性荨麻疹", "对西替利嗪过敏者禁用；严重肾功能不全禁用", "嗜睡、疲劳、口干、头痛、腹痛等"},
        {"马来酸氯苯那敏片", "片剂", "4mg*20片/盒", "华润双鹤药业股份有限公司", "8.5", "第一代抗组胺药，用于过敏性疾病", "口服，一次4mg，一日3次", "用于过敏性鼻炎、荨麻疹、皮肤瘙痒症", "新生儿及早产儿禁用；哺乳期妇女禁用", "嗜睡、乏力、口干、便秘、视力模糊等"},

        // 止痛类药物
        {"布洛芬缓释胶囊", "缓释胶囊", "0.3g*20粒/盒", "中美天津史克制药有限公司", "22.5", "用于缓解轻至中度疼痛，如头痛、关节痛", "口服，一次1-2粒，一日3次", "用于缓解轻至中度疼痛，如头痛、关节痛、偏头痛等", "对阿司匹林或其他非甾体抗炎药过敏者禁用", "恶心、呕吐、胃烧灼感或轻度消化不良等"},
        {"对乙酰氨基酚片", "片剂", "500mg*16片/盒", "上海强生制药有限公司", "15.0", "解热镇痛药，用于发热和轻中度疼痛", "口服，一次0.5-1g，一日3-4次", "用于感冒发热、头痛、肌肉痛、关节痛等", "严重肝肾功能不全禁用；对本品过敏者禁用", "偶见皮疹、恶心、呕吐、出汗等"},
        {"双氯芬酸钠肠溶片", "肠溶片", "25mg*30片/盒", "北京诺华制药有限公司", "18.8", "非甾体抗炎药，用于炎症和疼痛", "口服，一次25mg，一日2-3次", "用于风湿性关节炎、类风湿性关节炎等", "对双氯芬酸过敏者禁用；活动性消化性溃疡禁用", "胃肠道反应、头痛、头晕、皮疹等"},

        // 呼吸系统药物
        {"盐酸氨溴索口服溶液", "口服溶液", "30mg/5ml*100ml/瓶", "勃林格殷格翰药业有限公司", "48.0", "用于急性、慢性呼吸道疾病的祛痰治疗", "口服，成人一次10ml，一日3次", "用于急性、慢性支气管炎、支气管哮喘等", "对盐酸氨溴索过敏者禁用", "偶见皮疹、恶心、胃部不适、食欲缺乏等"},
        {"沙丁胺醇气雾剂", "气雾剂", "100μg*200揿/瓶", "葛兰素史克(中国)投资有限公司", "32.0", "β2受体激动剂，用于支气管哮喘", "吸入，一次100-200μg，按需使用", "用于支气管哮喘、慢性阻塞性肺疾病", "对沙丁胺醇过敏者禁用", "震颤、心悸、头痛、肌肉痉挛等"},
        {"茶碱缓释片", "缓释片", "0.1g*20片/盒", "上海信谊药厂有限公司", "25.8", "支气管扩张剂，用于哮喘和慢阻肺", "口服，一次0.1-0.2g，一日2次", "用于支气管哮喘、慢性阻塞性肺疾病", "对茶碱过敏者禁用；严重心律失常禁用", "恶心、呕吐、心悸、头痛、失眠等"},

        // 抗生素类药物
        {"头孢克洛干混悬剂", "干混悬剂", "0.125g*6袋/盒", "礼来苏州制药有限公司", "52.0", "用于敏感菌所致的呼吸系统、泌尿系统感染", "口服，按体重计算剂量，一日3次", "用于中耳炎、上呼吸道感染、下呼吸道感染等", "对头孢菌素类抗生素过敏者禁用", "腹泻、恶心、呕吐、皮疹等"},
        {"阿莫西林胶囊", "胶囊", "250mg*24粒/盒", "华北制药股份有限公司", "18.5", "青霉素类抗生素，用于细菌感染", "口服，一次250-500mg，一日3次", "用于敏感菌引起的上呼吸道感染、皮肤软组织感染", "对青霉素过敏者禁用", "腹泻、恶心、呕吐、皮疹、过敏反应等"},
        {"左氧氟沙星片", "片剂", "0.1g*10片/盒", "第一三共制药(北京)有限公司", "35.8", "喹诺酮类抗菌药，用于细菌感染", "口服，一次0.1-0.2g，一日2次", "用于呼吸系统感染、泌尿生殖系统感染", "18岁以下患者禁用；孕妇及哺乳期妇女禁用", "恶心、腹泻、头晕、失眠、皮疹等"},

        // 胃肠道药物
        {"奥美拉唑肠溶胶囊", "肠溶胶囊", "20mg*14粒/盒", "阿斯利康制药有限公司", "42.0", "质子泵抑制剂，用于胃酸相关疾病", "口服，一次20mg，一日1次，晨起空腹服用", "用于胃溃疡、十二指肠溃疡、反流性食管炎", "对奥美拉唑过敏者禁用", "头痛、腹泻、恶心、腹痛、便秘等"},
        {"雷尼替丁片", "片剂", "150mg*20片/盒", "天津力生制药股份有限公司", "12.8", "H2受体拮抗剂，用于抑制胃酸分泌", "口服，一次150mg，一日2次", "用于胃溃疡、十二指肠溃疡", "对雷尼替丁过敏者禁用；严重肾功能不全禁用", "头痛、便秘、腹泻、皮疹等"},
        {"多潘立酮片", "片剂", "10mg*30片/盒", "西安杨森制药有限公司", "25.0", "胃肠动力药，用于胃肠功能紊乱", "口服，一次10mg，一日3次，餐前服用", "用于消化不良、恶心、呕吐、胃胀等", "胃肠道出血禁用；机械性肠梗阻禁用", "口干、皮疹、乳房胀痛、月经失调等"},

        // 中成药
        {"六味地黄丸", "水蜜丸", "6g*10袋/盒", "北京同仁堂股份有限公司", "28.0", "滋阴补肾中成药", "口服，一次6g，一日2次", "用于肾阴亏损、头晕耳鸣、腰膝酸软", "脾胃虚寒者慎用", "偶见胃肠道不适"},
        {"感冒清热颗粒", "颗粒", "12g*9袋/盒", "广州白云山和记黄埔中药有限公司", "15.8", "疏风散寒、解表清热", "口服，一次1袋，一日2次，开水冲服", "用于风寒感冒，头痛发热，恶寒身痛", "孕妇禁用；高血压、心脏病患者慎用", "偶见恶心、腹胀等"},
        {"板蓝根颗粒", "颗粒", "10g*20袋/盒", "广西华润红草帽制药有限公司", "12.5", "清热解毒、凉血利咽", "口服，一次5-10g，一日3-4次", "用于肺胃热盛所致的咽喉肿痛、口咽干燥", "体虚而无实火热毒者禁用", "偶见腹泻、腹痛等"}
    };

    foreach (const QStringList &medicine, medicines) {
        query.prepare("INSERT INTO medicine (name, dosage_form, specification, manufacturer, price, description, usage, indications, contraindications, side_effects) "
                      "VALUES (:name, :dosage_form, :specification, :manufacturer, :price, :description, :usage, :indications, :contraindications, :side_effects)");
        query.bindValue(":name", medicine[0]);
        query.bindValue(":dosage_form", medicine[1]);
        query.bindValue(":specification", medicine[2]);
        query.bindValue(":manufacturer", medicine[3]);
        query.bindValue(":price", medicine[4].toDouble());
        query.bindValue(":description", medicine[5]);
        query.bindValue(":usage", medicine[6]);
        query.bindValue(":indications", medicine[7]);
        query.bindValue(":contraindications", medicine[8]);
        query.bindValue(":side_effects", medicine[9]);
        if (!query.exec()) {
            qDebug() << "药品插入失败:" << query.lastError().text();
        }
    }
    
    qDebug() << "药品测试数据插入完成，共插入" << medicines.size() << "条记录";

    // 更新部分药品的存储条件和有效期信息
    QVector<QStringList> storageUpdates = {
        // name, storage, expiry_date
        {"阿司匹林肠溶片", "密封，在干燥处保存", "2026-12-31"},
        {"硝苯地平缓释片", "遮光，密封保存", "2026-06-30"},
        {"辛伐他汀片", "密封，在阴凉干燥处保存", "2025-12-31"},
        {"盐酸二甲双胍片", "密封保存", "2026-08-31"},
        {"胰岛素注射液", "2-8℃冷藏保存，勿冷冻", "2025-03-31"},
        {"氯雷他定片", "密封，在干燥处保存", "2026-10-31"},
        {"布洛芬缓释胶囊", "密封保存", "2026-05-31"},
        {"奥美拉唑肠溶胶囊", "遮光，密封，在干燥处保存", "2025-11-30"},
        {"头孢克洛干混悬剂", "密封，在阴凉干燥处保存", "2025-09-30"},
        {"左氧氟沙星片", "遮光，密封保存", "2026-07-31"}
    };

    foreach (const QStringList &update, storageUpdates) {
        query.prepare("UPDATE medicine SET storage = :storage, expiry_date = :expiry_date WHERE name = :name");
        query.bindValue(":name", update[0]);
        query.bindValue(":storage", update[1]);
        query.bindValue(":expiry_date", update[2]);
        if (!query.exec()) {
            qDebug() << "药品存储信息更新失败:" << query.lastError().text();
        }
    }

}

void Server::handleClientData()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    QByteArray data = clientSocket->readAll();

    // 处理TCP粘包问题：按换行符分割消息
    static QByteArray buffer;
    buffer.append(data);

    while (buffer.contains('\n')) {
        int pos = buffer.indexOf('\n');
        QByteArray messageData = buffer.left(pos);
        buffer = buffer.mid(pos + 1);

        QString message = QString::fromUtf8(messageData);
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
    else if (messageType == ("SAVE_USERINFO"))
    {
        handleSaveUserInfo(message, clientSocket);
    }
    else if (messageType == "REGISTER")
    {
        handleRegister(message, clientSocket);
    }
    else if (messageType == "APPOINTMENTS")
    {
        handleAppointmentsRequest(message, clientSocket);
    }
    else if (messageType == "PROCESS_APPOINTMENT")
    {
        handleProcessAppointment(message, clientSocket);
    }
    else if (messageType == "CHECKIN") {
        handleCheckIn(message, clientSocket);
    }
    else if (messageType == "CHECKOUT") {
        handleCheckOut(message, clientSocket);
    }
    else if (messageType == "HISTORY") {
        handleAttendanceHistory(message, clientSocket);
    }
    else if (messageType == "LEAVE") {
        handleLeaveApplication(message, clientSocket);
    }
    else if (messageType == "LEAVE_RECORDS") {
        handleLeaveRecordsRequest(message, clientSocket);
    }
    else if (messageType == "RETURN") {
        handleReturnFromLeave(message, clientSocket);
    }
    else if (messageType == "SEND_MESSAGE") {
        handleSendMessage(message, clientSocket);
    }
    else if (messageType == "SEND_IMAGE") {
        handleSendImage(message, clientSocket);
    }
    else if (messageType == "GET_CHAT_HISTORY") {
        handleGetChatHistory(message, clientSocket);
    }
    else if (messageType == "GET_CONTACT_LIST") {
        handleGetContactList(message, clientSocket);
    }
    else if (messageType == "GET_IMAGE") {
        handleGetImage(message, clientSocket);
    }
    else if (messageType == "MEDICINE_SEARCH") {
        handleMedicineSearch(message, clientSocket);
    }
    else if (messageType == "VIDEO_CALL_REQUEST") {
        handleVideoCallRequest(message, clientSocket);
    }
    else if (messageType == "VIDEO_CALL_RESPONSE") {
        handleVideoCallResponse(message, clientSocket);
    }
    else if (messageType == "VIDEO_CALL_END") {
        handleVideoCallEnd(message, clientSocket);
    }
    else if (messageType == "SUBMIT_PRESCRIPTION") {
        handleSubmitPrescription(message, clientSocket);  // 处理处方提交
    }
    else if (messageType == "GET_PATIENT_PRESCRIPTIONS") {
        handleGetPatientPrescriptions(message, clientSocket);  // 处理患者处方查询
    }
    else if (messageType == "HOSPITALIZATION_APPLY") {
        handleHospitalizationApply(message, clientSocket);
    }
    else if (messageType == "GET_HOSPITALIZATION") {
        handleGetHospitalization(message, clientSocket);
    }
    else if (messageType == "ADD_PAYMENT_ITEM") {
        handleAddPaymentItem(message, clientSocket);
    }
    else if (messageType == "GET_PAYMENT_ITEMS") {
        handleGetPaymentItems(message, clientSocket);
    }
    else if (messageType == "PROCESS_PAYMENT") {
        handleProcessPayment(message, clientSocket);
    }
    else if (messageType == "GET_PAYMENT_RECORDS") {
        handleGetPaymentRecords(message, clientSocket);
    }
    else if (messageType == "GET_DOCTOR_SCHEDULE") {
        handleGetDoctorSchedule(clientSocket);
    }
    else if (messageType == "MAKE_APPOINTMENT") {
        handleMakeAppointment(message, clientSocket);
    }
    else if (messageType == "GET_USER_APPOINTMENTS") {
        handleGetUserAppointments(message, clientSocket);
    }
    }
}

void Server::handleUserInfoRequest(const QString &userId, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleUserInfoRequest";
        clientSocket->write("USERINFO_FAIL#DB_NOT_OPEN");
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM user WHERE id = :userId");
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        qDebug() << "用户信息查询失败:" << query.lastError().text();
        clientSocket->write("USERINFO_FAIL");
        return;
    }

    if (query.next()) {
        QJsonObject userJson;
        userJson["id"] = query.value("id").toString();
        userJson["username"] = query.value("username").toString();
        userJson["real_name"] = query.value("real_name").toString();
        userJson["birth_date"] = query.value("birth_date").toString();
        userJson["id_card"] = query.value("id_card").toString();
        userJson["phone"] = query.value("phone").toString();
        userJson["email"] = query.value("email").toString();
        userJson["avatar_path"] = query.value("avatar_path").toString(); // 添加头像路径

        QJsonDocument doc(userJson);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

        clientSocket->write("USERINFO_SUCCESS#" + jsonData);
        qDebug() << "已发送用户信息给用户:" << userId;
    } else {
        clientSocket->write("USERINFO_FAIL#USER_NOT_FOUND");
        qDebug() << "用户不存在:" << userId;
    }
}

void Server::handleSaveUserInfo(const QString &message, QTcpSocket *clientSocket)
{
    QStringList parts = message.split('#');
    if (parts.size() < 3) {
        clientSocket->write("SAVE_USERINFO_FAIL#INVALID_FORMAT");
        return;
    }

    QString userId = parts[1];
    QString jsonStr = parts[2];

    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        clientSocket->write("SAVE_USERINFO_FAIL#INVALID_JSON");
        return;
    }

    QJsonObject jsonData = doc.object();

    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleSaveUserInfo";
        clientSocket->write("SAVE_USERINFO_FAIL#DB_NOT_OPEN");
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("UPDATE user SET real_name = :real_name, birth_date = :birth_date, "
                  "id_card = :id_card, phone = :phone, email = :email, "
                  "avatar_path = :avatar_path WHERE id = :id"); // 更新avatar_path字段

    query.bindValue(":real_name", jsonData["real_name"].toString());
    query.bindValue(":birth_date", jsonData["birth_date"].toString());
    query.bindValue(":id_card", jsonData["id_card"].toString());
    query.bindValue(":phone", jsonData["phone"].toString());
    query.bindValue(":email", jsonData["email"].toString());
    query.bindValue(":avatar_path", jsonData["avatar_path"].toString()); // 绑定头像路径
    query.bindValue(":id", userId);

    if (query.exec()) {
        if (query.numRowsAffected() > 0) {
            clientSocket->write("SAVE_USERINFO_SUCCESS");
            qDebug() << "用户信息更新成功:" << userId;
        } else {
            clientSocket->write("SAVE_USERINFO_FAIL#NO_ROWS_AFFECTED");
            qDebug() << "用户信息更新失败:" << userId;
        }
    } else {
        clientSocket->write("SAVE_USERINFO_FAIL#DB_ERROR");
        qDebug() << "用户信息更新数据库错误:" << query.lastError().text();
    }
}


// 生成新的用户ID
QString Server::generateUserId(const QString &identity)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in generateUserId";
        return "";
    }

    QString prefix = "11"; // 默认患者
    if (identity == "医生") {
        prefix = "12";
    }

    // 查询当前最大ID
    QSqlQuery query(m_db);
    query.prepare("SELECT MAX(id) FROM user WHERE id LIKE :prefix");
    query.bindValue(":prefix", prefix + "%");

    int maxNumber = 0;
    if (query.exec() && query.next()) {
        QString maxId = query.value(0).toString();
        if (!maxId.isEmpty()) {
            maxNumber = maxId.mid(2).toInt(); // 去掉前缀
        }
    }

    // 生成新的ID
    int newNumber = maxNumber + 1;
    QString newId = prefix + QString::number(newNumber).rightJustified(4, '0');

    return newId;
}

void Server::handleRegister(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleRegister";
        clientSocket->write("REGISTER_FAIL#DB_NOT_OPEN");
        return;
    }

    // 注册请求格式: REGISTER#username#password#identity#real_name#birth_date#id_card#phone#email
    QStringList parts = message.split('#');
    if (parts.size() < 9) {
        clientSocket->write("REGISTER_FAIL#INVALID_FORMAT");
        return;
    }

    QString username = parts[1];
    QString password = parts[2];
    QString identity = parts[3];
    QString real_name = parts[4];
    QString birth_date = parts[5];
    QString id_card = parts[6];
    QString phone = parts[7];
    QString email = parts[8];

    // 生成用户ID
    QString userId = generateUserId(identity);

    // 插入用户到数据库
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO user (id, username, password, avatar_path, real_name, birth_date, id_card, phone, email) "
                  "VALUES (:id, :username, :password, :avatar_path, :real_name, :birth_date, :id_card, :phone, :email)");

    query.bindValue(":id", userId);
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    query.bindValue(":avatar_path", "default_avatar.png");
    query.bindValue(":real_name", real_name);
    query.bindValue(":birth_date", birth_date);
    query.bindValue(":id_card", id_card);
    query.bindValue(":phone", phone);
    query.bindValue(":email", email);

    if (query.exec()) {
        // 根据身份插入到相应的表
        if (identity == "医生") {
            QSqlQuery doctorQuery(m_db);
            doctorQuery.prepare("INSERT INTO doctor (id, department, title, introduction) "
                              "VALUES (:id, :department, :title, :introduction)");
            doctorQuery.bindValue(":id", userId);
            doctorQuery.bindValue(":department", "待分配");
            doctorQuery.bindValue(":title", "医生");
            doctorQuery.bindValue(":introduction", "新注册医生");
            doctorQuery.exec();
        } else {
            QSqlQuery patientQuery(m_db);
            patientQuery.prepare("INSERT INTO patient (id, case_info) "
                                "VALUES (:id, :case_info)");
            patientQuery.bindValue(":id", userId);
            patientQuery.bindValue(":case_info", "新注册患者");
            patientQuery.exec();
        }

        clientSocket->write(("REGISTER_SUCCESS#" + userId).toUtf8());
        qDebug() << "Register success:" << userId << "-" << real_name;
    } else {
        clientSocket->write("REGISTER_FAIL#DB_ERROR");
        qDebug() << "Register failed:" << query.lastError().text();
    }
}

// 处理获取预约请求
void Server::handleAppointmentsRequest(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleAppointmentsRequest";
        clientSocket->write("APPOINTMENTS_FAIL#DB_NOT_OPEN");
        return;
    }

    // 请求格式: APPOINTMENTS#doctorId
    QString doctorId = message.section('#', 1, 1);

    QSqlQuery query(m_db);
    query.prepare("SELECT a.patient_id, u.real_name as patient_name, a.appointment_date, "
                  "d.department, p.case_info as symptom, u.phone, u.id_card, a.status "
                  "FROM appointment a "
                  "JOIN user u ON a.patient_id = u.id "
                  "JOIN patient p ON a.patient_id = p.id "
                  "JOIN doctor d ON a.doctor_id = d.id "
                  "WHERE a.doctor_id = :doctor_id");
    query.bindValue(":doctor_id", doctorId);

    if (query.exec()) {
        QJsonArray appointmentsArray;

        while (query.next()) {
            QJsonObject appointment;
            appointment["patient_id"] = query.value("patient_id").toString();
            appointment["patient_name"] = query.value("patient_name").toString();
            appointment["appointment_date"] = query.value("appointment_date").toString();
            appointment["department"] = query.value("department").toString();
            appointment["symptom"] = query.value("symptom").toString();
            appointment["phone"] = query.value("phone").toString();
            appointment["status"] = query.value("status").toString();

            // 从身份证号推断性别和年龄
            QString idCard = query.value("id_card").toString();
            if (!idCard.isEmpty() && idCard.length() >= 18) {
                // 推断性别：身份证第17位，奇数为男性，偶数为女性
                QChar genderDigit = idCard.at(16);
                QString gender = (genderDigit.digitValue() % 2 == 1) ? "男" : "女";
                appointment["gender"] = gender;

                // 推断年龄：从身份证第7-10位获取出生年份
                QString birthYearStr = idCard.mid(6, 4);
                bool ok;
                int birthYear = birthYearStr.toInt(&ok);
                if (ok && birthYear > 1900) {
                    QDate currentDate = QDate::currentDate();
                    int currentYear = currentDate.year();
                    int age = currentYear - birthYear;

                    // 检查是否已过生日
                    QString birthMonthStr = idCard.mid(10, 2);
                    QString birthDayStr = idCard.mid(12, 2);
                    int birthMonth = birthMonthStr.toInt(&ok);
                    if (ok) {
                        int birthDay = birthDayStr.toInt(&ok);
                        if (ok) {
                            QDate birthDate(birthYear, birthMonth, birthDay);
                            if (currentDate < QDate(currentYear, birthMonth, birthDay)) {
                                age--; // 如果今年生日还没到，年龄减1
                            }
                        }
                    }
                    appointment["age"] = age;
                }
            }

            appointmentsArray.append(appointment);
        }

        QJsonDocument doc(appointmentsArray);
        QString response = "APPOINTMENTS_SUCCESS#" + doc.toJson(QJsonDocument::Compact);
        clientSocket->write(response.toUtf8());
        qDebug() << "发送预约数据给医生:" << doctorId;
    } else {
        clientSocket->write("APPOINTMENTS_FAIL");
        qDebug() << "获取预约数据失败:" << query.lastError().text();
    }
}

// 处理预约请求
void Server::handleProcessAppointment(const QString &message, QTcpSocket *clientSocket)
{
    // 请求格式: PROCESS_APPOINTMENT#patientId#doctorId#status
    QStringList parts = message.split('#');
    if (parts.size() < 4) {
        clientSocket->write("PROCESS_APPOINTMENT_FAIL#INVALID_FORMAT");
        return;
    }

    QString patientId = parts[1];
    QString doctorId = parts[2];
    QString status = parts[3];

    QSqlQuery query(m_db);
    query.prepare("UPDATE appointment SET status = :status "
                  "WHERE patient_id = :patient_id AND doctor_id = :doctor_id");
    query.bindValue(":status", status);
    query.bindValue(":patient_id", patientId);
    query.bindValue(":doctor_id", doctorId);

    if (query.exec() && query.numRowsAffected() > 0) {
        clientSocket->write("PROCESS_APPOINTMENT_SUCCESS");
        qDebug() << "预约处理成功:" << patientId << "-" << doctorId << "-" << status;
    } else {
        clientSocket->write("PROCESS_APPOINTMENT_FAIL#DB_ERROR");
        qDebug() << "预约处理失败:" << query.lastError().text();
    }
}

void Server::handleLogIn(QString message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleLogIn";
        clientSocket->write("LOGIN_FAIL#DB_NOT_OPEN");
        return;
    }

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

// 处理打卡请求
void Server::handleCheckIn(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleCheckIn";
        clientSocket->write("CHECKIN_FAIL#DB_NOT_OPEN");
        return;
    }

    // 请求格式: CHECKIN#doctorId#date
    QStringList parts = message.split('#');
    if (parts.size() < 3) {
        clientSocket->write("CHECKIN_FAIL#INVALID_FORMAT");
        return;
    }

    QString doctorId = parts[1];
    QString date = parts[2];
    QTime currentTime = QTime::currentTime();

    // 检查是否已经签到过
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT * FROM attendance WHERE doctor_id = :doctor_id AND date = :date");
    checkQuery.bindValue(":doctor_id", doctorId);
    checkQuery.bindValue(":date", date);

    if (checkQuery.exec() && checkQuery.next()) {
        clientSocket->write("CHECKIN_FAIL#ALREADY_CHECKED_IN");
        qDebug() << "打卡失败: 医生" << doctorId << "在" << date << "已经签到过";
        return;
    }

    // 判断打卡状态
    QString status = "normal";
    if (currentTime > QTime(8, 30)) {
        status = "late";
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO attendance (doctor_id, date, check_in_time, status) "
                  "VALUES (:doctor_id, :date, :check_in_time, :status)");
    query.bindValue(":doctor_id", doctorId);
    query.bindValue(":date", date);
    query.bindValue(":check_in_time", currentTime.toString("hh:mm:ss"));
    query.bindValue(":status", status);

    if (query.exec()) {
        clientSocket->write("CHECKIN_SUCCESS");
        qDebug() << "打卡成功:" << doctorId << "-" << date << "-" << currentTime.toString("hh:mm:ss");
    } else {
        clientSocket->write("CHECKIN_FAIL#DB_ERROR");
        qDebug() << "打卡失败:" << query.lastError().text();
    }
}

// 处理签出请求
void Server::handleCheckOut(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleCheckOut";
        clientSocket->write("CHECKOUT_FAIL#DB_NOT_OPEN");
        return;
    }

    // 请求格式: CHECKOUT#doctorId#date
    QStringList parts = message.split('#');
    if (parts.size() < 3) {
        clientSocket->write("CHECKOUT_FAIL#INVALID_FORMAT");
        return;
    }

    QString doctorId = parts[1];
    QString date = parts[2];
    QTime currentTime = QTime::currentTime();

    // 检查是否已经签到过
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT * FROM attendance WHERE doctor_id = :doctor_id AND date = :date");
    checkQuery.bindValue(":doctor_id", doctorId);
    checkQuery.bindValue(":date", date);

    if (!checkQuery.exec() || !checkQuery.next()) {
        clientSocket->write("CHECKOUT_FAIL#NOT_CHECKED_IN");
        qDebug() << "签出失败: 医生" << doctorId << "在" << date << "尚未签到";
        return;
    }

    // 检查是否已经签出过
    if (!checkQuery.value("check_out_time").isNull()) {
        clientSocket->write("CHECKOUT_FAIL#ALREADY_CHECKED_OUT");
        qDebug() << "签出失败: 医生" << doctorId << "在" << date << "已经签出过";
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("UPDATE attendance SET check_out_time = :check_out_time "
                  "WHERE doctor_id = :doctor_id AND date = :date");
    query.bindValue(":check_out_time", currentTime.toString("hh:mm:ss"));
    query.bindValue(":doctor_id", doctorId);
    query.bindValue(":date", date);

    if (query.exec() && query.numRowsAffected() > 0) {
        clientSocket->write("CHECKOUT_SUCCESS");
        qDebug() << "签出成功:" << doctorId << "-" << date << "-" << currentTime.toString("hh:mm:ss");
    } else {
        clientSocket->write("CHECKOUT_FAIL#DB_ERROR");
        qDebug() << "签出失败:" << query.lastError().text();
    }
}

// 处理考勤历史记录请求
void Server::handleAttendanceHistory(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleAttendanceHistory";
        clientSocket->write("HISTORY_FAIL#DB_NOT_OPEN");
        return;
    }

    // 请求格式: HISTORY#doctorId
    QString doctorId = message.section('#', 1, 1);

    QSqlQuery query(m_db);
    query.prepare("SELECT date, check_in_time, check_out_time, status FROM attendance "
                  "WHERE doctor_id = :doctor_id "
                  "ORDER BY date DESC"); // 近两年记录
    query.bindValue(":doctor_id", doctorId);


    if (query.exec()) {
        QJsonArray historyArray;

        while (query.next()) {
            QJsonObject record;
            record["date"] = query.value("date").toString();
            record["check_in_time"] = query.value("check_in_time").toString();
            record["check_out_time"] = query.value("check_out_time").toString();
            record["status"] = query.value("status").toString();
            historyArray.append(record);
        }

        QJsonDocument doc(historyArray);
        QString response = "HISTORY_SUCCESS#" + doc.toJson(QJsonDocument::Compact);
        clientSocket->write(response.toUtf8());
        qDebug() << "发送考勤历史数据给医生:" << doctorId;
    } else {
        clientSocket->write("HISTORY_FAIL");
        qDebug() << "获取考勤历史失败:" << query.lastError().text();
    }
}

// 处理请假申请
void Server::handleLeaveApplication(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleLeaveApplication";
        clientSocket->write("LEAVE_FAIL#DB_NOT_OPEN");
        return;
    }

    // 请求格式: LEAVE#doctorId#contact#leaveType#startDate#endDate#reason
    QStringList parts = message.split('#');
    if (parts.size() < 7) {
        clientSocket->write("LEAVE_FAIL#INVALID_FORMAT");
        return;
    }

    QString doctorId = parts[1];
    QString contact = parts[2];
    QString leaveType = parts[3];
    QString startDate = parts[4];
    QString endDate = parts[5];
    QString reason = parts[6];

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO leave (doctor_id, leave_type, start_date, end_date, reason) "
                  "VALUES (:doctor_id, :leave_type, :start_date, :end_date, :reason)");
    query.bindValue(":doctor_id", doctorId);
    query.bindValue(":leave_type", leaveType);
    query.bindValue(":start_date", startDate);
    query.bindValue(":end_date", endDate);
    query.bindValue(":reason", reason);

    if (query.exec()) {
        clientSocket->write("LEAVE_SUCCESS");
        qDebug() << "请假申请提交成功:" << doctorId << "-" << leaveType << "-" << startDate << "-" << endDate;
    } else {
        clientSocket->write("LEAVE_FAIL#DB_ERROR");
        qDebug() << "请假申请提交失败:" << query.lastError().text();
    }
}

// 处理请假记录请求
void Server::handleLeaveRecordsRequest(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleLeaveRecordsRequest";
        clientSocket->write("LEAVE_RECORDS_FAIL#DB_NOT_OPEN");
        return;
    }

    // 请求格式: LEAVE_RECORDS#doctorId
    QString doctorId = message.section('#', 1, 1);

    QSqlQuery query(m_db);
    query.prepare("SELECT leave_id, leave_type, start_date, end_date, status "
                  "FROM leave WHERE doctor_id = :doctor_id ORDER BY applied_date DESC");
    query.bindValue(":doctor_id", doctorId);

    if (query.exec()) {
        QJsonArray leaveRecordsArray;

        while (query.next()) {
            QJsonObject record;
            record["id"] = query.value("leave_id").toString();
            record["type"] = query.value("leave_type").toString();
            record["start_date"] = query.value("start_date").toString();
            record["end_date"] = query.value("end_date").toString();
            record["status"] = query.value("status").toString();
            leaveRecordsArray.append(record);
        }

        QJsonDocument doc(leaveRecordsArray);
        QString response = "LEAVE_RECORDS_SUCCESS#" + doc.toJson(QJsonDocument::Compact);
        clientSocket->write(response.toUtf8());
        qDebug() << "发送请假记录数据给医生:" << doctorId;
    } else {
        clientSocket->write("LEAVE_RECORDS_FAIL");
        qDebug() << "获取请假记录失败:" << query.lastError().text();
    }
}

// 处理销假请求
void Server::handleReturnFromLeave(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleReturnFromLeave";
        clientSocket->write("RETURN_FAIL#DB_NOT_OPEN");
        return;
    }

    // 请求格式: RETURN#leaveId
    QString leaveId = message.section('#', 1, 1);

    QSqlQuery query(m_db);
    query.prepare("UPDATE leave SET status = 'rejected' WHERE leave_id = :leave_id AND status = 'approved'");
    query.bindValue(":leave_id", leaveId);

    if (query.exec() && query.numRowsAffected() > 0) {
        clientSocket->write("RETURN_SUCCESS");
        qDebug() << "销假成功:" << leaveId;
    } else {
        clientSocket->write("RETURN_FAIL#DB_ERROR");
        qDebug() << "销假失败:" << query.lastError().text();
    }
}

// ================ 医患沟通相关函数实现 ================

// 处理发送消息
void Server::handleSendMessage(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleSendMessage";
        clientSocket->write("SEND_MESSAGE_FAIL#DB_NOT_OPEN\n");
        return;
    }

    // 消息格式: SEND_MESSAGE#senderId#receiverId#content
    QStringList parts = message.split('#');
    if (parts.size() < 4) {
        clientSocket->write("SEND_MESSAGE_FAIL#INVALID_FORMAT\n");
        return;
    }

    QString senderId = parts[1];
    QString receiverId = parts[2];
    QString content = parts[3];

    // 验证发送者和接收者都存在
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT COUNT(*) FROM user WHERE id = :id");

    checkQuery.bindValue(":id", senderId);
    if (!checkQuery.exec() || !checkQuery.next() || checkQuery.value(0).toInt() == 0) {
        clientSocket->write("SEND_MESSAGE_FAIL#SENDER_NOT_EXISTS\n");
        return;
    }

    checkQuery.bindValue(":id", receiverId);
    if (!checkQuery.exec() || !checkQuery.next() || checkQuery.value(0).toInt() == 0) {
        clientSocket->write("SEND_MESSAGE_FAIL#RECEIVER_NOT_EXISTS\n");
        return;
    }

    // 保存消息到数据库
    QSqlQuery insertQuery(m_db);
    insertQuery.prepare("INSERT INTO message (sender_id, receiver_id, content) VALUES (:sender_id, :receiver_id, :content)");
    insertQuery.bindValue(":sender_id", senderId);
    insertQuery.bindValue(":receiver_id", receiverId);
    insertQuery.bindValue(":content", content);

    if (insertQuery.exec()) {
        // 获取插入的消息ID和时间
        qint64 messageId = insertQuery.lastInsertId().toLongLong();

        QSqlQuery timeQuery(m_db);
        timeQuery.prepare("SELECT send_time FROM message WHERE message_id = :message_id");
        timeQuery.bindValue(":message_id", messageId);

        QString sendTime;
        if (timeQuery.exec() && timeQuery.next()) {
            sendTime = timeQuery.value("send_time").toString();
        }

        // 发送成功响应给发送者
        QString response = QString("SEND_MESSAGE_SUCCESS#%1#%2").arg(messageId).arg(sendTime);
        clientSocket->write(response.toUtf8() + "\n");

        // 实时推送消息给接收者
        QString broadcastData = QString("NEW_MESSAGE#%1#%2#%3#%4").arg(senderId).arg(receiverId).arg(content).arg(sendTime);
        broadcastMessage(receiverId, broadcastData);

        qDebug() << "消息发送成功:" << senderId << "->" << receiverId << ":" << content;
    } else {
        clientSocket->write("SEND_MESSAGE_FAIL#DB_ERROR\n");
        qDebug() << "消息发送失败:" << insertQuery.lastError().text();
    }
}

// 处理发送图片
void Server::handleSendImage(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleSendImage";
        clientSocket->write("SEND_IMAGE_FAIL#DB_NOT_OPEN\n");
        return;
    }

    // 消息格式: SEND_IMAGE#senderId#receiverId#imageName#base64Data
    QStringList parts = message.split('#');
    if (parts.size() < 5) {
        clientSocket->write("SEND_IMAGE_FAIL#INVALID_FORMAT\n");
        return;
    }

    QString senderId = parts[1];
    QString receiverId = parts[2];
    QString imageName = parts[3];
    QString base64Data = parts[4];

    // 验证发送者和接收者都存在
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT COUNT(*) FROM user WHERE id = :id");

    checkQuery.bindValue(":id", senderId);
    if (!checkQuery.exec() || !checkQuery.next() || checkQuery.value(0).toInt() == 0) {
        clientSocket->write("SEND_IMAGE_FAIL#SENDER_NOT_EXISTS\n");
        return;
    }

    checkQuery.bindValue(":id", receiverId);
    if (!checkQuery.exec() || !checkQuery.next() || checkQuery.value(0).toInt() == 0) {
        clientSocket->write("SEND_IMAGE_FAIL#RECEIVER_NOT_EXISTS\n");
        return;
    }

    // 使用服务端应用目录下的 images 子目录，避免工作目录依赖
    QString serverImageDir = QDir(QCoreApplication::applicationDirPath()).filePath("images");
    QDir dir;
    if (!dir.exists(serverImageDir)) {
        bool created = dir.mkpath(serverImageDir);
        qDebug() << "创建服务端 images 目录:" << serverImageDir << "结果:" << created;
    }

    qDebug() << "服务端应用目录:" << QCoreApplication::applicationDirPath();
    qDebug() << "服务端图片目录:" << serverImageDir;

    // 将Base64数据解码并保存为文件
    QByteArray imageData = QByteArray::fromBase64(base64Data.toUtf8());
    QString imagePath = QDir(serverImageDir).filePath(imageName);

    qDebug() << "准备保存图片到:" << imagePath << "数据大小:" << imageData.size() << "字节";

    QFile imageFile(imagePath);
    if (imageFile.open(QIODevice::WriteOnly)) {
        qint64 written = imageFile.write(imageData);
        imageFile.close();

        // 验证文件是否真的保存了
        QFileInfo savedFile(imagePath);
        if (savedFile.exists()) {
            qDebug() << "图片保存成功:" << imagePath << "写入:" << written << "字节，文件大小:" << savedFile.size() << "字节";
        } else {
            qDebug() << "图片保存异常：文件写入后不存在" << imagePath;
            clientSocket->write("SEND_IMAGE_FAIL#SAVE_ERROR\n");
            return;
        }
    } else {
        clientSocket->write("SEND_IMAGE_FAIL#SAVE_ERROR\n");
        qDebug() << "图片保存失败，无法打开文件:" << imagePath << "错误:" << imageFile.errorString();
        return;
    }

    // 保存图片消息到数据库（内容格式：[IMAGE:filename]）
    QString imageMessage = QString("[IMAGE:%1]").arg(imageName);
    QSqlQuery insertQuery(m_db);
    insertQuery.prepare("INSERT INTO message (sender_id, receiver_id, content) VALUES (:sender_id, :receiver_id, :content)");
    insertQuery.bindValue(":sender_id", senderId);
    insertQuery.bindValue(":receiver_id", receiverId);
    insertQuery.bindValue(":content", imageMessage);

    if (insertQuery.exec()) {
        // 获取插入的消息ID和时间
        qint64 messageId = insertQuery.lastInsertId().toLongLong();

        QSqlQuery timeQuery(m_db);
        timeQuery.prepare("SELECT send_time FROM message WHERE message_id = :message_id");
        timeQuery.bindValue(":message_id", messageId);

        QString sendTime;
        if (timeQuery.exec() && timeQuery.next()) {
            sendTime = timeQuery.value("send_time").toString();
        }

        // 发送成功响应给发送者
        QString response = QString("SEND_IMAGE_SUCCESS#%1#%2#%3").arg(messageId).arg(sendTime).arg(imageName);
        clientSocket->write(response.toUtf8() + "\n");

        // 实时推送图片消息给接收者
        QString broadcastData = QString("NEW_IMAGE#%1#%2#%3#%4").arg(senderId).arg(receiverId).arg(imageName).arg(sendTime);
        broadcastMessage(receiverId, broadcastData);

        qDebug() << "图片消息发送成功:" << senderId << "->" << receiverId << ":" << imageName;
    } else {
        clientSocket->write("SEND_IMAGE_FAIL#DB_ERROR\n");
        qDebug() << "图片消息发送失败:" << insertQuery.lastError().text();
    }
}

// 处理获取图片：GET_IMAGE#imageName
void Server::handleGetImage(const QString &message, QTcpSocket *clientSocket)
{
    qDebug() << "服务端收到 GET_IMAGE 请求:" << message;

    // 消息格式: GET_IMAGE#imageName
    QStringList parts = message.split('#');
    if (parts.size() < 2) {
        clientSocket->write("GET_IMAGE_FAIL#INVALID_FORMAT\n");
        qDebug() << "GET_IMAGE: 无效格式" << message;
        return;
    }

    QString imageName = parts[1];
    QString serverImageDir = QDir(QCoreApplication::applicationDirPath()).filePath("images");
    QString imagePath = QDir(serverImageDir).filePath(imageName);

    qDebug() << "GET_IMAGE: 查找图片" << imagePath;

    QFile imageFile(imagePath);
    if (!imageFile.exists()) {
        clientSocket->write("GET_IMAGE_FAIL#NOT_FOUND\n");
        qDebug() << "GET_IMAGE: 文件不存在" << imagePath;

        // 列出 images 目录下的所有文件以便调试
        QDir imageDir(serverImageDir);
        QStringList imageFiles = imageDir.entryList(QStringList() << "*.png" << "*.jpg" << "*.jpeg", QDir::Files);
        qDebug() << "服务端 images 目录现有文件:" << imageFiles;
        return;
    }
    if (!imageFile.open(QIODevice::ReadOnly)) {
        clientSocket->write("GET_IMAGE_FAIL#OPEN_ERROR\n");
        qDebug() << "GET_IMAGE: open error" << imagePath;
        return;
    }

    QByteArray data = imageFile.readAll();
    imageFile.close();

    QString base64 = QString::fromUtf8(data.toBase64());
    QString header = QString("IMAGE_DATA#%1#%2").arg(imageName).arg(base64.size());

    // 写入头部
    clientSocket->write(header.toUtf8() + "\n");
    qDebug() << "GET_IMAGE: 发送头部" << header;

    // 确保头部发送完毕
    clientSocket->flush();

    // 一次性发送完整的 base64 数据，避免分块问题
    QByteArray base64Data = base64.toUtf8();

    // 一次性写入所有 base64 数据和换行符
    clientSocket->write(base64Data + "\n");
    clientSocket->flush();

    // 合理的发送超时：根据测试结果大幅放宽
    qint64 estimatedMB = (base64Data.size() / 1048576) + 1;
    int waitTimeout = qMax(30000, (int)(estimatedMB * 150000)); // 30秒起，每MB增加150秒

    qDebug() << "GET_IMAGE: 极限测试 - 预估文件大小:" << estimatedMB << "MB, 等待发送超时:" << waitTimeout << "ms";
    qDebug() << "GET_IMAGE: Socket状态:" << clientSocket->state() << "错误:" << clientSocket->errorString();

    // 等待确保数据发送完毕
    if (!clientSocket->waitForBytesWritten(waitTimeout)) {
        qDebug() << "GET_IMAGE: 发送超时，Socket状态:" << clientSocket->state()
                 << "错误:" << clientSocket->errorString()
                 << "已写入字节:" << clientSocket->bytesToWrite();
    } else {
        qDebug() << "GET_IMAGE: 数据发送成功完毕";
    }

    qDebug() << "GET_IMAGE: sent" << imageName << "原始字节:" << data.size() << "base64字节:" << base64Data.size() << "分块发送完成";
}

// 处理获取聊天历史
void Server::handleGetChatHistory(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleGetChatHistory";
        clientSocket->write("GET_CHAT_HISTORY_FAIL#DB_NOT_OPEN\n");
        return;
    }

    // 消息格式: GET_CHAT_HISTORY#userId#contactId
    QStringList parts = message.split('#');
    if (parts.size() < 3) {
        clientSocket->write("GET_CHAT_HISTORY_FAIL#INVALID_FORMAT\n");
        return;
    }

    QString userId = parts[1];
    QString contactId = parts[2];

    QSqlQuery query(m_db);
    query.prepare("SELECT message_id, sender_id, receiver_id, content, send_time "
                  "FROM message "
                  "WHERE (sender_id = :user_id AND receiver_id = :contact_id) "
                  "   OR (sender_id = :contact_id AND receiver_id = :user_id) "
                  "ORDER BY send_time ASC");
    query.bindValue(":user_id", userId);
    query.bindValue(":contact_id", contactId);

    if (query.exec()) {
        QJsonArray messagesArray;

        while (query.next()) {
            QJsonObject messageObj;
            messageObj["message_id"] = query.value("message_id").toString();
            messageObj["sender_id"] = query.value("sender_id").toString();
            messageObj["receiver_id"] = query.value("receiver_id").toString();
            messageObj["content"] = query.value("content").toString();
            messageObj["send_time"] = query.value("send_time").toString();
            messagesArray.append(messageObj);
        }

        QJsonDocument doc(messagesArray);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

        clientSocket->write("GET_CHAT_HISTORY_SUCCESS#" + jsonData + "\n");
        qDebug() << "聊天历史发送成功:" << userId << "<->" << contactId;
    } else {
        clientSocket->write("GET_CHAT_HISTORY_FAIL#DB_ERROR\n");
        qDebug() << "获取聊天历史失败:" << query.lastError().text();
    }
}

// 处理获取联系人列表
void Server::handleGetContactList(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleGetContactList";
        clientSocket->write("GET_CONTACT_LIST_FAIL#DB_NOT_OPEN\n");
        return;
    }

    // 消息格式: GET_CONTACT_LIST#userId
    QString userId = message.section('#', 1, 1);

    QSqlQuery query(m_db);
    // 使用简化的查询，分步获取联系人和最后消息
    query.prepare("SELECT DISTINCT "
                  "CASE WHEN m.sender_id = :user_id THEN m.receiver_id ELSE m.sender_id END as contact_id, "
                  "u.real_name "
                  "FROM message m "
                  "JOIN user u ON (CASE WHEN m.sender_id = :user_id THEN m.receiver_id ELSE m.sender_id END) = u.id "
                  "WHERE m.sender_id = :user_id OR m.receiver_id = :user_id "
                  "ORDER BY m.send_time DESC");

    query.bindValue(":user_id", userId);

    if (query.exec()) {
        QJsonArray contactsArray;
        QSet<QString> processedContacts; // 避免重复联系人

        while (query.next()) {
            QString contactId = query.value("contact_id").toString();

            // 如果已经处理过这个联系人，跳过
            if (processedContacts.contains(contactId)) {
                continue;
            }
            processedContacts.insert(contactId);

            // 为每个联系人查询最后一条消息
            QSqlQuery msgQuery(m_db);
            msgQuery.prepare("SELECT content, send_time FROM message "
                           "WHERE (sender_id = :user_id AND receiver_id = :contact_id) "
                           "   OR (sender_id = :contact_id AND receiver_id = :user_id) "
                           "ORDER BY send_time DESC LIMIT 1");
            msgQuery.bindValue(":user_id", userId);
            msgQuery.bindValue(":contact_id", contactId);

            QString lastMessage = "";
            QString lastTime = "";
            if (msgQuery.exec() && msgQuery.next()) {
                lastMessage = msgQuery.value("content").toString();
                lastTime = msgQuery.value("send_time").toString();
            }

            QJsonObject contactObj;
            contactObj["contact_id"] = contactId;
            contactObj["name"] = query.value("real_name").toString();
            contactObj["last_message"] = lastMessage;
            contactObj["last_time"] = lastTime;
            contactsArray.append(contactObj);
        }

        QJsonDocument doc(contactsArray);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

        clientSocket->write("GET_CONTACT_LIST_SUCCESS#" + jsonData + "\n");
        qDebug() << "联系人列表发送成功:" << userId << ", 联系人数量:" << contactsArray.size();
    } else {
        clientSocket->write("GET_CONTACT_LIST_FAIL#DB_ERROR\n");
        qDebug() << "获取联系人列表失败:" << query.lastError().text();
    }
}


// 处理药品搜索请求
void Server::handleMedicineSearch(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleMedicineSearch";
        clientSocket->write("MEDICINE_SEARCH_FAIL#DB_NOT_OPEN\n");
        return;
    }

    // 请求格式: MEDICINE_SEARCH#searchText#[manufacturer#sideEffects#...]
    QStringList parts = message.split('#');
    if (parts.size() < 2) {
        clientSocket->write("MEDICINE_SEARCH_FAIL#INVALID_FORMAT\n");
        return;
    }

    QString searchText = parts[1];
    QString manufacturer = parts.size() > 2 ? parts[2] : "";
    QString sideEffects = parts.size() > 3 ? parts[3] : "";

    // 构建SQL查询语句，实现模糊搜索
    QString queryStr = "SELECT * FROM medicine WHERE 1=1";
    QSqlQuery query(m_db);

    // 添加药品名称模糊搜索条件 - 修正：当搜索文本为"全部"时不添加条件
    if (!searchText.isEmpty() && searchText != "全部") {
        queryStr += " AND name LIKE :name";
    }

    // 添加生产厂家筛选条件
    if (!manufacturer.isEmpty() && manufacturer != "全部") {
        queryStr += " AND manufacturer = :manufacturer";
    }

    // 添加副作用筛选条件
    if (!sideEffects.isEmpty() && sideEffects != "全部") {
        queryStr += " AND side_effects LIKE :sideEffects";
    }

    query.prepare(queryStr);

    if (!searchText.isEmpty() && searchText != "全部") {
        query.bindValue(":name", "%" + searchText + "%");
    }

    if (!manufacturer.isEmpty() && manufacturer != "全部") {
        query.bindValue(":manufacturer", manufacturer);
    }

    if (!sideEffects.isEmpty() && sideEffects != "全部") {
        query.bindValue(":sideEffects", "%" + sideEffects + "%");
    }

    if (query.exec()) {
        QJsonArray medicinesArray;
        int resultCount = 0;

        while (query.next()) {
            resultCount++;
            QJsonObject medicine;
            medicine["medicine_id"] = query.value("medicine_id").toString();
            medicine["name"] = query.value("name").toString();
            medicine["dosage_form"] = query.value("dosage_form").toString();
            medicine["specification"] = query.value("specification").toString();
            medicine["manufacturer"] = query.value("manufacturer").toString();
            medicine["usage"] = query.value("usage").toString();
            // 客户端期望的字段名是 indication（单数），而数据库是 indications（复数）
            medicine["indication"] = query.value("indications").toString();
            // 客户端期望的字段名是 side_effect（单数），而数据库是 side_effects（复数）
            medicine["side_effect"] = query.value("side_effects").toString();
            // 客户端期望的字段名是 contraindication（单数），而数据库是 contraindications（复数）
            medicine["contraindication"] = query.value("contraindications").toString();
            // 添加客户端期望的 notice 字段，可以使用 contraindications 的内容
            medicine["notice"] = query.value("contraindications").toString();
            medicine["storage"] = query.value("storage").toString();
            medicine["expiry_date"] = query.value("expiry_date").toString();
            // 添加价格和描述信息，客户端可能需要这些
            medicine["price"] = query.value("price").toDouble();
            medicine["description"] = query.value("description").toString();

            medicinesArray.append(medicine);
        }

        QJsonDocument doc(medicinesArray);
        QString response = "MEDICINE_SEARCH_SUCCESS#" + doc.toJson(QJsonDocument::Compact);
        clientSocket->write(response.toUtf8() + "\n");
        qDebug() << "发送药品搜索结果给客户端，响应长度：" << response.length();
    } else {
        clientSocket->write("MEDICINE_SEARCH_FAIL#DB_ERROR\n");
        qDebug() << "药品搜索失败:" << query.lastError().text();
    }
}

// 广播消息给指定用户
void Server::broadcastMessage(const QString &receiverId, const QString &messageData)
{
    // 遍历所有连接的客户端，找到接收者并发送消息
    for (auto it = m_connectedClients.begin(); it != m_connectedClients.end(); ++it) {
        if (it.value() == receiverId) {
            QTcpSocket *receiverSocket = it.key();
            if (receiverSocket && receiverSocket->state() == QTcpSocket::ConnectedState) {
                receiverSocket->write(messageData.toUtf8() + "\n");
                qDebug() << "实时消息推送给用户:" << receiverId;
                break;
            }
        }
    }
}

// 处理视频通话请求
void Server::handleVideoCallRequest(const QString &message, QTcpSocket *clientSocket)
{
    QStringList parts = message.split('#');
    if (parts.size() < 3) {
        qDebug() << "视频通话请求格式错误:" << message;
        return;
    }
    
    QString senderId = parts[1];
    QString receiverId = parts[2];
    
    qDebug() << "收到视频通话请求:" << senderId << "呼叫" << receiverId;
    
    // 构造转发消息
    QString forwardMessage = QString("VIDEO_CALL_REQUEST#%1#%2").arg(senderId, receiverId);
    
    // 转发给接收者
    broadcastMessage(receiverId, forwardMessage);
}

// 处理视频通话响应
void Server::handleVideoCallResponse(const QString &message, QTcpSocket *clientSocket)
{
    QStringList parts = message.split('#');
    if (parts.size() < 4) {
        qDebug() << "视频通话响应格式错误:" << message;
        return;
    }
    
    QString senderId = parts[1];
    QString receiverId = parts[2];
    QString accepted = parts[3];
    
    qDebug() << "收到视频通话响应:" << senderId << "回复" << receiverId << ":" << accepted;
    
    // 构造转发消息
    QString forwardMessage = QString("VIDEO_CALL_RESPONSE#%1#%2#%3").arg(senderId, receiverId, accepted);
    
    // 转发给发起者
    broadcastMessage(receiverId, forwardMessage);
}

// 处理视频通话结束
void Server::handleVideoCallEnd(const QString &message, QTcpSocket *clientSocket)
{
    QStringList parts = message.split('#');
    if (parts.size() < 3) {
        qDebug() << "视频通话结束格式错误:" << message;
        return;
    }
    
    QString senderId = parts[1];
    QString receiverId = parts[2];
    
    qDebug() << "收到视频通话结束:" << senderId << "结束与" << receiverId << "的通话";
    
    // 构造转发消息
    QString forwardMessage = QString("VIDEO_CALL_END#%1#%2").arg(senderId, receiverId);
    
    // 转发给对方
    broadcastMessage(receiverId, forwardMessage);
}

// 处理处方提交
void Server::handleSubmitPrescription(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        clientSocket->write("PRESCRIPTION_SUBMIT_FAIL#DB_NOT_OPEN\n");
        return;
    }

    // 请求格式: SUBMIT_PRESCRIPTION#patient_id#doctor_id#medicine_name#dosage#usage#frequency#quantity#notes
    QStringList parts = message.split("#");
    if (parts.size() != 9) {
        clientSocket->write("PRESCRIPTION_SUBMIT_FAIL#INVALID_FORMAT\n");
        qDebug() << "处方提交格式错误，参数数量:" << parts.size() << "预期:9";
        return;
    }

    QString patientId = parts[1];
    QString doctorId = parts[2];
    QString medicineName = parts[3];
    QString dosage = parts[4];
    QString usage = parts[5];
    QString frequency = parts[6];
    QString quantityStr = parts[7];
    QString notes = parts[8];

    // 验证购买数量是否为有效整数
    bool ok;
    int quantity = quantityStr.toInt(&ok);
    if (!ok || quantity <= 0) {
        clientSocket->write("PRESCRIPTION_SUBMIT_FAIL#INVALID_QUANTITY\n");
        qDebug() << "购买数量无效:" << quantityStr;
        return;
    }

    qDebug() << "处方提交信息:";
    qDebug() << "患者ID:" << patientId;
    qDebug() << "医生ID:" << doctorId;
    qDebug() << "药品名称:" << medicineName;
    qDebug() << "剂量:" << dosage;
    qDebug() << "用法:" << usage;
    qDebug() << "频次:" << frequency;
    qDebug() << "购买数量:" << quantity;
    qDebug() << "备注:" << notes;

    // 检查患者是否存在
    QSqlQuery checkPatient(m_db);
    checkPatient.prepare("SELECT COUNT(*) FROM patient WHERE id = :patient_id");
    checkPatient.bindValue(":patient_id", patientId);
    if (!checkPatient.exec() || !checkPatient.next() || checkPatient.value(0).toInt() == 0) {
        clientSocket->write("PRESCRIPTION_SUBMIT_FAIL#PATIENT_NOT_FOUND\n");
        qDebug() << "患者不存在:" << patientId;
        return;
    }

    // 检查医生是否存在
    QSqlQuery checkDoctor(m_db);
    checkDoctor.prepare("SELECT COUNT(*) FROM doctor WHERE id = :doctor_id");
    checkDoctor.bindValue(":doctor_id", doctorId);
    if (!checkDoctor.exec() || !checkDoctor.next() || checkDoctor.value(0).toInt() == 0) {
        clientSocket->write("PRESCRIPTION_SUBMIT_FAIL#DOCTOR_NOT_FOUND\n");
        qDebug() << "医生不存在:" << doctorId;
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO prescription (patient_id, doctor_id, medicine_name, dosage, usage, frequency, quantity, notes) "
                  "VALUES (:patient_id, :doctor_id, :medicine_name, :dosage, :usage, :frequency, :quantity, :notes)");
    query.bindValue(":patient_id", patientId);
    query.bindValue(":doctor_id", doctorId);
    query.bindValue(":medicine_name", medicineName);
    query.bindValue(":dosage", dosage);
    query.bindValue(":usage", usage);
    query.bindValue(":frequency", frequency);
    query.bindValue(":quantity", quantity);
    query.bindValue(":notes", notes);

    if (query.exec()) {
        int prescriptionId = query.lastInsertId().toInt();
        
        // 查询药品价格以计算处方费用 - 使用精确匹配
        QSqlQuery medicineQuery(m_db);
        medicineQuery.prepare("SELECT medicine_id, name, price, specification FROM medicine WHERE name = :medicine_name LIMIT 1");
        medicineQuery.bindValue(":medicine_name", medicineName);
        
        double medicinePrice = 0.0;
        QString medicineSpec = "";
        int medicineId = 0;
        
        if (medicineQuery.exec() && medicineQuery.next()) {
            medicineId = medicineQuery.value("medicine_id").toInt();
            medicinePrice = medicineQuery.value("price").toDouble();
            medicineSpec = medicineQuery.value("specification").toString();
            qDebug() << "找到药品信息:" << medicineName << "价格:" << medicinePrice << "规格:" << medicineSpec;
        } else {
            // 如果精确匹配失败，尝试模糊匹配
            QSqlQuery fuzzyQuery(m_db);
            fuzzyQuery.prepare("SELECT medicine_id, name, price, specification FROM medicine WHERE name LIKE :medicine_pattern LIMIT 1");
            fuzzyQuery.bindValue(":medicine_pattern", QString("%%%1%%").arg(medicineName));
            
            if (fuzzyQuery.exec() && fuzzyQuery.next()) {
                medicineId = fuzzyQuery.value("medicine_id").toInt();
                medicinePrice = fuzzyQuery.value("price").toDouble();
                medicineSpec = fuzzyQuery.value("specification").toString();
                qDebug() << "模糊匹配找到药品:" << fuzzyQuery.value("name").toString() << "价格:" << medicinePrice;
            } else {
                // 如果都找不到，记录错误并使用默认价格
                qDebug() << "警告：药品价格未找到 -" << medicineName << "，使用默认价格";
                medicinePrice = 15.0; // 提高默认价格到15元
            }
        }
        
        // 验证价格的合理性
        if (medicinePrice <= 0) {
            qDebug() << "药品价格异常，使用默认价格:" << medicinePrice << "→ 15.0";
            medicinePrice = 15.0;
        }
        
        // 计算总费用 = 药品单价 * 购买数量
        double totalCost = medicinePrice * quantity;
        
        // 开始事务确保数据一致性
        m_db.transaction();
        
        try {
            // 自动添加处方费用到缴费项目
            QSqlQuery paymentQuery(m_db);
            paymentQuery.prepare("INSERT INTO payment_items "
                               "(patient_id, description, amount, status, type, application_id, created_at) "
                               "VALUES (:patient_id, :description, :amount, :status, :type, :application_id, :created_at)");
            
            QString description = QString("处方费用 - %1 (数量:%2)").arg(medicineName).arg(quantity);
            QString prescriptionRef = QString("PRESC_%1").arg(prescriptionId);
            QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            
            paymentQuery.bindValue(":patient_id", patientId);
            paymentQuery.bindValue(":description", description);
            paymentQuery.bindValue(":amount", totalCost);
            paymentQuery.bindValue(":status", "pending");
            paymentQuery.bindValue(":type", "prescription");
            paymentQuery.bindValue(":application_id", prescriptionRef);
            paymentQuery.bindValue(":created_at", currentTime);
            
            if (!paymentQuery.exec()) {
                throw std::runtime_error("缴费项目添加失败");
            }
            
            // 在缴费记录表中插入待支付记录
            QSqlQuery recordQuery(m_db);
            recordQuery.prepare("INSERT INTO payment_records "
                              "(patient_id, total_amount, payment_time, payment_method) "
                              "VALUES (:patient_id, :total_amount, :payment_time, :payment_method)");
            recordQuery.bindValue(":patient_id", patientId);
            recordQuery.bindValue(":total_amount", totalCost);
            recordQuery.bindValue(":payment_time", currentTime);
            recordQuery.bindValue(":payment_method", "待支付");
            
            if (!recordQuery.exec()) {
                throw std::runtime_error("缴费记录添加失败");
            }
            
            // 提交事务
            m_db.commit();
            
            qDebug() << "处方缴费项目和记录添加成功，费用:" << totalCost;
            
        } catch (const std::exception &e) {
            m_db.rollback();
            qDebug() << "处方缴费处理失败:" << e.what();
        }
        
        QString response = QString("PRESCRIPTION_SUBMIT_SUCCESS#%1").arg(prescriptionId);
        clientSocket->write(response.toUtf8() + "\n");
        qDebug() << "处方提交成功，ID:" << prescriptionId << "费用:" << totalCost;

        // 发送处方更新通知给患者端
        QString notificationMessage = QString("PRESCRIPTION_UPDATE#%1").arg(patientId);
        broadcastMessage(patientId, notificationMessage);
    } else {
        clientSocket->write("PRESCRIPTION_SUBMIT_FAIL#DB_ERROR\n");
        qDebug() << "处方提交失败:" << query.lastError().text();
    }
}

// 处理患者处方查询
void Server::handleGetPatientPrescriptions(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        clientSocket->write("PRESCRIPTION_LIST_FAIL#DB_NOT_OPEN\n");
        return;
    }

    // 请求格式: GET_PATIENT_PRESCRIPTIONS#patient_id
    QStringList parts = message.split("#");
    if (parts.size() != 2) {
        clientSocket->write("PRESCRIPTION_LIST_FAIL#INVALID_FORMAT\n");
        return;
    }

    QString patientId = parts[1];

    QSqlQuery query(m_db);
    query.prepare("SELECT p.prescription_id, p.medicine_name, p.dosage, p.usage, p.frequency, p.quantity, p.notes, "
                  "p.prescribed_date, p.status, u.real_name as doctor_name, d.department "
                  "FROM prescription p "
                  "LEFT JOIN doctor d ON p.doctor_id = d.id "
                  "LEFT JOIN user u ON p.doctor_id = u.id "
                  "WHERE p.patient_id = :patient_id "
                  "ORDER BY p.prescribed_date DESC");
    query.bindValue(":patient_id", patientId);

    if (query.exec()) {
        QJsonArray prescriptionsArray;
        while (query.next()) {
            QJsonObject prescription;
            prescription["prescription_id"] = query.value("prescription_id").toInt();
            prescription["medicine_name"] = query.value("medicine_name").toString();
            prescription["dosage"] = query.value("dosage").toString();
            prescription["usage"] = query.value("usage").toString();
            prescription["frequency"] = query.value("frequency").toString();
            prescription["quantity"] = query.value("quantity").toInt();
            prescription["notes"] = query.value("notes").toString();
            prescription["prescribed_date"] = query.value("prescribed_date").toString();
            prescription["status"] = query.value("status").toString();
            prescription["doctor_name"] = query.value("doctor_name").toString();
            prescription["department"] = query.value("department").toString();

            prescriptionsArray.append(prescription);
        }

        QJsonDocument doc(prescriptionsArray);
        QString response = "PRESCRIPTION_LIST_SUCCESS#" + doc.toJson(QJsonDocument::Compact);
        clientSocket->write(response.toUtf8() + "\n");
        qDebug() << "发送患者处方列表，共" << prescriptionsArray.size() << "条记录";
    } else {
        clientSocket->write("PRESCRIPTION_LIST_FAIL#DB_ERROR\n");
        qDebug() << "患者处方查询失败:" << query.lastError().text();
    }
}
// 处理住院申请
void Server::handleHospitalizationApply(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleHospitalizationApply";
        clientSocket->write("HOSPITALIZATION_APPLY_FAIL#DB_NOT_OPEN\n");
        return;
    }

    // 消息格式: HOSPITALIZATION_APPLY#<json数据>
    QString jsonStr = message.section('#', 1);
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());

    if (doc.isNull() || !doc.isObject()) {
        clientSocket->write("HOSPITALIZATION_APPLY_FAIL#INVALID_JSON\n");
        return;
    }

    QJsonObject application = doc.object();

    // 生成住院申请ID
    QString applicationId = "HOSP" + QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");

    // 插入住院申请到数据库
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO hospitalization_application "
                  "(application_id, patient_id, patient_name, department, doctor, "
                  "admission_date, symptoms, diagnosis, fee, status) "
                  "VALUES (:application_id, :patient_id, :patient_name, :department, :doctor, "
                  ":admission_date, :symptoms, :diagnosis, :fee, :status)");

    query.bindValue(":application_id", applicationId);
    query.bindValue(":patient_id", application["patient_id"].toString());
    query.bindValue(":patient_name", application["patient_name"].toString());
    query.bindValue(":department", application["department"].toString());
    query.bindValue(":doctor", application["doctor"].toString());
    query.bindValue(":admission_date", application["admission_date"].toString());
    query.bindValue(":symptoms", application["symptoms"].toString());
    query.bindValue(":diagnosis", application["diagnosis"].toString());
    query.bindValue(":fee", application["fee"].toDouble());
    query.bindValue(":status", "pending_payment"); // 待支付状态

    if (query.exec()) {
        // 发送成功响应
        clientSocket->write(QString("HOSPITALIZATION_APPLY_SUCCESS#%1\n").arg(applicationId).toUtf8());
        qDebug() << "住院申请提交成功:" << applicationId;
    } else {
        clientSocket->write("HOSPITALIZATION_APPLY_FAIL#DB_ERROR\n");
        qDebug() << "住院申请提交失败:" << query.lastError().text();
    }
}

// 获取住院记录
void Server::handleGetHospitalization(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleGetHospitalization";
        clientSocket->write("GET_HOSPITALIZATION_FAIL#DB_NOT_OPEN\n");
        return;
    }

    // 消息格式: GET_HOSPITALIZATION#<患者ID>
    QString patientId = message.section('#', 1);

    QSqlQuery query(m_db);
    query.prepare("SELECT application_id, admission_date, department, doctor, fee, status "
                  "FROM hospitalization_application WHERE patient_id = :patient_id "
                  "ORDER BY created_at DESC");
    query.bindValue(":patient_id", patientId);

    if (query.exec()) {
        QJsonArray recordsArray;

        while (query.next()) {
            QJsonObject record;
            record["application_id"] = query.value("application_id").toString();
            record["admission_date"] = query.value("admission_date").toString();
            record["department"] = query.value("department").toString();
            record["doctor"] = query.value("doctor").toString();
            record["fee"] = query.value("fee").toDouble();
            record["status"] = query.value("status").toString();

            recordsArray.append(record);
        }

        QJsonDocument doc(recordsArray);
        QString response = "GET_HOSPITALIZATION_SUCCESS#" + doc.toJson(QJsonDocument::Compact);
        clientSocket->write(response.toUtf8() + "\n");
        qDebug() << "发送住院记录数据给患者:" << patientId;
    } else {
        clientSocket->write("GET_HOSPITALIZATION_FAIL#DB_ERROR\n");
        qDebug() << "获取住院记录失败:" << query.lastError().text();
    }
}

// 添加缴费项目
void Server::handleAddPaymentItem(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleAddPaymentItem";
        clientSocket->write("ADD_PAYMENT_ITEM_FAIL#DB_NOT_OPEN\n");
        return;
    }

    // 消息格式: ADD_PAYMENT_ITEM#<json数据>
    QString jsonStr = message.section('#', 1);
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());

    if (doc.isNull() || !doc.isObject()) {
        clientSocket->write("ADD_PAYMENT_ITEM_FAIL#INVALID_JSON\n");
        return;
    }

    QJsonObject paymentItem = doc.object();

    // 插入缴费项目到数据库
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO payment_items "
                  "(patient_id, description, amount, status, type, application_id, created_at) "
                  "VALUES (:patient_id, :description, :amount, :status, :type, :application_id, :created_at)");

    query.bindValue(":patient_id", paymentItem["patient_id"].toString());
    query.bindValue(":description", paymentItem["description"].toString());
    query.bindValue(":amount", paymentItem["amount"].toDouble());
    query.bindValue(":status", paymentItem["status"].toString());
    query.bindValue(":type", paymentItem["type"].toString());
    // 清理application_id，移除可能的换行符和空白字符
    QString applicationId = paymentItem["application_id"].toString().trimmed();
    applicationId = applicationId.remove('\n');
    query.bindValue(":application_id", applicationId);
    query.bindValue(":created_at", paymentItem["created_at"].toString());

    if (query.exec()) {
        clientSocket->write("ADD_PAYMENT_ITEM_SUCCESS\n");
        qDebug() << "缴费项目添加成功:" << paymentItem["description"].toString();
    } else {
        clientSocket->write("ADD_PAYMENT_ITEM_FAIL#DB_ERROR\n");
        qDebug() << "缴费项目添加失败:" << query.lastError().text();
    }
}

// 获取缴费项目（包括待支付和已支付）
void Server::handleGetPaymentItems(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleGetPaymentItems";
        clientSocket->write("GET_PAYMENT_ITEMS_FAIL#DB_NOT_OPEN\n");
        return;
    }

    // 消息格式: GET_PAYMENT_ITEMS#<患者ID>
    QString patientId = message.section('#', 1);

    QSqlQuery query(m_db);
    query.prepare("SELECT item_id, description, amount, created_at, paid_at, status, type, application_id "
                  "FROM payment_items WHERE patient_id = :patient_id "
                  "ORDER BY created_at DESC");
    query.bindValue(":patient_id", patientId);

    if (query.exec()) {
        QJsonArray itemsArray;

        while (query.next()) {
            QJsonObject item;
            item["item_id"] = query.value("item_id").toInt();
            item["description"] = query.value("description").toString();
            item["amount"] = query.value("amount").toDouble();
            item["created_at"] = query.value("created_at").toString();
            item["paid_at"] = query.value("paid_at").toString();
            item["status"] = query.value("status").toString();
            item["type"] = query.value("type").toString();
            item["application_id"] = query.value("application_id").toString();

            itemsArray.append(item);
        }

        QJsonDocument doc(itemsArray);
        QString response = "GET_PAYMENT_ITEMS_SUCCESS#" + doc.toJson(QJsonDocument::Compact);
        clientSocket->write(response.toUtf8() + "\n");
        qDebug() << "发送缴费项目数据给患者:" << patientId << "，共" << itemsArray.size() << "项";
    } else {
        clientSocket->write("GET_PAYMENT_ITEMS_FAIL#DB_ERROR\n");
        qDebug() << "获取缴费项目失败:" << query.lastError().text();
    }
}

// 处理支付
void Server::handleProcessPayment(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleProcessPayment";
        clientSocket->write("PROCESS_PAYMENT_FAIL#DB_NOT_OPEN\n");
        return;
    }

    // 消息格式: PROCESS_PAYMENT#<json数据>
    QString jsonStr = message.section('#', 1);
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());

    if (doc.isNull() || !doc.isObject()) {
        clientSocket->write("PROCESS_PAYMENT_FAIL#INVALID_JSON\n");
        return;
    }

    QJsonObject payment = doc.object();
    QString patientId = payment["patient_id"].toString();
    QString paymentMethod = payment["payment_method"].toString();
    
    // 检查是否是单个项目支付（通过 application_id）
    if (payment.contains("application_id")) {
        QString applicationId = payment["application_id"].toString();
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        
        // 开始事务
        m_db.transaction();
        
        try {
            // 查找待支付的项目
            QSqlQuery findQuery(m_db);
            findQuery.prepare("SELECT item_id, amount, description FROM payment_items "
                            "WHERE patient_id = :patient_id AND application_id = :application_id AND status = 'pending'");
            findQuery.bindValue(":patient_id", patientId);
            findQuery.bindValue(":application_id", applicationId);
            
            if (!findQuery.exec() || !findQuery.next()) {
                m_db.rollback();
                clientSocket->write("PROCESS_PAYMENT_FAIL#ITEM_NOT_FOUND\n");
                return;
            }
            
            int itemId = findQuery.value("item_id").toInt();
            double amount = findQuery.value("amount").toDouble();
            QString description = findQuery.value("description").toString();
            
            // 更新缴费项目状态
            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE payment_items SET status = 'paid', paid_at = :paid_at "
                              "WHERE item_id = :item_id");
            updateQuery.bindValue(":paid_at", currentTime);
            updateQuery.bindValue(":item_id", itemId);
            
            if (!updateQuery.exec()) {
                throw std::runtime_error("更新缴费项目状态失败");
            }
            
            // 根据application_id更新相关业务状态
            if (applicationId.startsWith("APPT_")) {
                // 更新预约状态为已确认
                QString appointmentId = applicationId.mid(5);
                QSqlQuery apptQuery(m_db);
                apptQuery.prepare("UPDATE appointment SET status = 'confirmed' WHERE appointment_id = :appointment_id");
                apptQuery.bindValue(":appointment_id", appointmentId.toInt());
                if (!apptQuery.exec()) {
                    qDebug() << "更新预约状态失败:" << apptQuery.lastError().text();
                }
                qDebug() << "预约支付完成，预约ID:" << appointmentId << "状态更新为已确认";
            } else if (applicationId.startsWith("PRESC_")) {
                // 更新处方状态（可选）
                QString prescriptionId = applicationId.mid(6);
                QSqlQuery prescQuery(m_db);
                prescQuery.prepare("UPDATE prescription SET status = 'paid' WHERE prescription_id = :prescription_id");
                prescQuery.bindValue(":prescription_id", prescriptionId.toInt());
                prescQuery.exec(); // 不强制要求成功，因为原始prescription表可能没有status字段
                qDebug() << "处方支付完成，处方ID:" << prescriptionId;
            } else if (applicationId.startsWith("HOSP_")) {
                // 更新住院申请状态
                QString hospId = applicationId.mid(5);
                QSqlQuery hospQuery(m_db);
                hospQuery.prepare("UPDATE hospitalization_application SET status = 'paid' WHERE application_id = :application_id");
                hospQuery.bindValue(":application_id", hospId);
                if (!hospQuery.exec()) {
                    qDebug() << "更新住院申请状态失败:" << hospQuery.lastError().text();
                }
                qDebug() << "住院费用支付完成，申请ID:" << hospId;
            }
            
            // 添加支付记录
            QSqlQuery recordQuery(m_db);
            recordQuery.prepare("INSERT INTO payment_records "
                              "(patient_id, total_amount, payment_time, payment_method) "
                              "VALUES (:patient_id, :total_amount, :payment_time, :payment_method)");
            recordQuery.bindValue(":patient_id", patientId);
            recordQuery.bindValue(":total_amount", amount);
            recordQuery.bindValue(":payment_time", currentTime);
            recordQuery.bindValue(":payment_method", paymentMethod);
            
            if (!recordQuery.exec()) {
                throw std::runtime_error("添加支付记录失败");
            }
            
            // 提交事务
            m_db.commit();
            
            QString paymentId = recordQuery.lastInsertId().toString();
            clientSocket->write(QString("PROCESS_PAYMENT_SUCCESS#%1\n").arg(paymentId).toUtf8());
            qDebug() << "单项支付处理成功:" << patientId << "-" << amount << "-" << description;
            
        } catch (const std::exception &e) {
            m_db.rollback();
            clientSocket->write("PROCESS_PAYMENT_FAIL#DB_ERROR\n");
            qDebug() << "单项支付处理失败:" << e.what();
        }
        
        return;
    }
    
    // 原有的批量支付逻辑
    double totalAmount = payment["total_amount"].toDouble();
    QString paymentTime = payment["payment_time"].toString();

    // 开始事务
    m_db.transaction();

    try {
        // 更新缴费项目状态
        QJsonArray items = payment["items"].toArray();
        for (const QJsonValue &itemValue : items) {
            QJsonObject item = itemValue.toObject();
            QString description = item["item_name"].toString();
            double amount = item["amount"].toDouble();

            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE payment_items SET status = 'paid', paid_at = :paid_at "
                              "WHERE patient_id = :patient_id AND description = :description AND amount = :amount AND status = 'pending'");
            updateQuery.bindValue(":paid_at", paymentTime);
            updateQuery.bindValue(":patient_id", patientId);
            updateQuery.bindValue(":description", description);
            updateQuery.bindValue(":amount", amount);

            if (!updateQuery.exec()) {
                throw std::runtime_error("更新缴费项目状态失败");
            }

            // 根据application_id更新相关业务状态
            if (item.contains("application_id")) {
                QString applicationId = item["application_id"].toString();
                
                // 处理住院申请费用
                if (applicationId.startsWith("HOSP_")) {
                    QString hospId = applicationId.mid(5); // 移除 "HOSP_" 前缀
                    QSqlQuery hospQuery(m_db);
                    hospQuery.prepare("UPDATE hospitalization_application SET status = 'paid' WHERE application_id = :application_id");
                    hospQuery.bindValue(":application_id", hospId);
                    if (!hospQuery.exec()) {
                        throw std::runtime_error("更新住院申请状态失败");
                    }
                }
                // 处理预约挂号费用
                else if (applicationId.startsWith("APPT_")) {
                    QString appointmentId = applicationId.mid(5); // 移除 "APPT_" 前缀
                    QSqlQuery apptQuery(m_db);
                    apptQuery.prepare("UPDATE appointment SET status = 'confirmed' WHERE appointment_id = :appointment_id");
                    apptQuery.bindValue(":appointment_id", appointmentId.toInt());
                    if (!apptQuery.exec()) {
                        throw std::runtime_error("更新预约状态失败");
                    }
                    qDebug() << "预约状态更新为已确认，预约ID:" << appointmentId;
                }
                // 处理处方费用
                else if (applicationId.startsWith("PRESC_")) {
                    QString prescriptionId = applicationId.mid(6); // 移除 "PRESC_" 前缀
                    // 处方支付完成后可以更新处方状态为已支付（如果需要）
                    QSqlQuery prescQuery(m_db);
                    prescQuery.prepare("UPDATE prescription SET status = 'paid' WHERE prescription_id = :prescription_id");
                    prescQuery.bindValue(":prescription_id", prescriptionId.toInt());
                    prescQuery.exec(); // 这个更新是可选的，不影响主流程
                    qDebug() << "处方支付完成，处方ID:" << prescriptionId;
                }
            }
            // 兼容旧版本的预约状态更新逻辑
            else if (item.contains("type") && item["type"].toString() == "appointment") {
                // 如果是预约费用但没有application_id，尝试从描述中提取
                QString description = item["description"].toString();
                QRegularExpression re("预约号:(\\d+)");
                QRegularExpressionMatch match = re.match(description);

                if (match.hasMatch()) {
                    QString appointmentId = match.captured(1);
                    QSqlQuery updateAppointmentQuery(m_db);
                    updateAppointmentQuery.prepare("UPDATE appointment SET status = 'confirmed' WHERE appointment_id = :appointment_id");
                    updateAppointmentQuery.bindValue(":appointment_id", appointmentId);
                    updateAppointmentQuery.exec();
                    qDebug() << "兼容模式：预约状态更新为已确认，预约ID:" << appointmentId;
                }
            }
        }

        // 添加支付记录
        QSqlQuery recordQuery(m_db);
        recordQuery.prepare("INSERT INTO payment_records "
                          "(patient_id, total_amount, payment_time, payment_method) "
                          "VALUES (:patient_id, :total_amount, :payment_time, :payment_method)");
        recordQuery.bindValue(":patient_id", patientId);
        recordQuery.bindValue(":total_amount", totalAmount);
        recordQuery.bindValue(":payment_time", paymentTime);
        recordQuery.bindValue(":payment_method", "在线支付"); // 可以根据需要修改

        if (!recordQuery.exec()) {
            throw std::runtime_error("添加支付记录失败");
        }

        // 提交事务
        m_db.commit();

        // 发送成功响应
        QString paymentId = recordQuery.lastInsertId().toString();
        clientSocket->write(QString("PROCESS_PAYMENT_SUCCESS#%1\n").arg(paymentId).toUtf8());
        qDebug() << "支付处理成功:" << patientId << "-" << totalAmount;

    } catch (const std::exception &e) {
        // 回滚事务
        m_db.rollback();
        clientSocket->write("PROCESS_PAYMENT_FAIL#DB_ERROR\n");
        qDebug() << "支付处理失败:" << e.what();
    }
}

// 获取缴费记录
void Server::handleGetPaymentRecords(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database not open in handleGetPaymentRecords";
        clientSocket->write("GET_PAYMENT_RECORDS_FAIL#DB_NOT_OPEN\n");
        return;
    }

    // 消息格式: GET_PAYMENT_RECORDS#<患者ID>
    QString patientId = message.section('#', 1);

    QSqlQuery query(m_db);
    query.prepare("SELECT description, amount, paid_at, 'online_payment' as payment_method "
                  "FROM payment_items "
                  "WHERE patient_id = :patient_id AND status = 'paid' "
                  "ORDER BY paid_at DESC");
    query.bindValue(":patient_id", patientId);

    if (query.exec()) {
        QJsonArray recordsArray;

        while (query.next()) {
            QJsonObject record;
            record["description"] = query.value("description").toString();
            record["amount"] = query.value("amount").toDouble();
            record["paid_at"] = query.value("paid_at").toString();
            record["payment_method"] = query.value("payment_method").toString();

            recordsArray.append(record);
        }

        QJsonDocument doc(recordsArray);
        QString response = "GET_PAYMENT_RECORDS_SUCCESS#" + doc.toJson(QJsonDocument::Compact);
        clientSocket->write(response.toUtf8() + "\n");
        qDebug() << "发送缴费记录数据给患者:" << patientId;
    } else {
        clientSocket->write("GET_PAYMENT_RECORDS_FAIL#DB_ERROR\n");
        qDebug() << "获取缴费记录失败:" << query.lastError().text();
    }
}


// 添加新的处理函数
void Server::handleGetDoctorSchedule(QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        clientSocket->write("GET_DOCTOR_SCHEDULE_FAIL#DB_NOT_OPEN\n");
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT d.id, u.real_name as doctor_name, d.department, d.title, "
                  "d.registration_fee, "
                  "(SELECT COUNT(*) FROM appointment a WHERE a.doctor_id = d.id AND "
                  "DATE(a.appointment_date) = DATE('now') AND a.status != 'cancelled') as today_appointments, "
                  "20 as max_daily_appointments " // 假设每个医生每天最多20个预约
                  "FROM doctor d "
                  "JOIN user u ON d.id = u.id"); // 查询所有医生用户

    if (query.exec()) {
        QJsonArray scheduleArray;

        while (query.next()) {
            QJsonObject doctor;
            doctor["doctor_id"] = query.value("id").toString();
            doctor["doctor_name"] = query.value("doctor_name").toString();
            doctor["department"] = query.value("department").toString();
            doctor["title"] = query.value("title").toString();
            doctor["registration_fee"] = query.value("registration_fee").toDouble();

            int todayAppointments = query.value("today_appointments").toInt();
            int maxAppointments = query.value("max_daily_appointments").toInt();
            doctor["remaining_slots"] = maxAppointments - todayAppointments;

            scheduleArray.append(doctor);
        }

        QJsonDocument doc(scheduleArray);
        clientSocket->write("GET_DOCTOR_SCHEDULE_SUCCESS#" + doc.toJson(QJsonDocument::Compact) + "\n");
    } else {
        clientSocket->write("GET_DOCTOR_SCHEDULE_FAIL#DB_ERROR\n");
        qDebug() << "获取医生排班失败:" << query.lastError().text();
    }
}

void Server::handleMakeAppointment(const QString &message, QTcpSocket *clientSocket)
{
    if (!m_db.isOpen()) {
        clientSocket->write("MAKE_APPOINTMENT_FAIL#DB_NOT_OPEN\n");
        return;
    }

    // 消息格式: MAKE_APPOINTMENT#patientId#doctorId#appointmentDate
    QStringList parts = message.split('#');
    if (parts.size() < 4) {
        clientSocket->write("MAKE_APPOINTMENT_FAIL#INVALID_FORMAT\n");
        return;
    }

    QString patientId = parts[1];
    QString doctorId = parts[2];
    QString appointmentDate = parts[3];

    // 开始事务
    m_db.transaction();

    try {
        // 检查医生是否存在和获取挂号费
        QSqlQuery checkDoctorQuery(m_db);
        checkDoctorQuery.prepare("SELECT d.registration_fee, u.real_name, d.department FROM doctor d JOIN user u ON d.id = u.id WHERE d.id = :doctor_id");
        checkDoctorQuery.bindValue(":doctor_id", doctorId);

        if (!checkDoctorQuery.exec() || !checkDoctorQuery.next()) {
            qDebug() << "查询医生信息失败:" << checkDoctorQuery.lastError().text();
            throw std::runtime_error("DOCTOR_NOT_FOUND");
        }

        double registrationFee = checkDoctorQuery.value("registration_fee").toDouble();
        QString doctorName = checkDoctorQuery.value("real_name").toString();
        QString department = checkDoctorQuery.value("department").toString();

        // 检查当天预约数量
        QSqlQuery checkAppointmentQuery(m_db);
        checkAppointmentQuery.prepare("SELECT COUNT(*) as appointment_count FROM appointment "
                                      "WHERE doctor_id = :doctor_id AND DATE(appointment_date) = DATE(:appointment_date) "
                                      "AND status != 'cancelled'");
        checkAppointmentQuery.bindValue(":doctor_id", doctorId);
        checkAppointmentQuery.bindValue(":appointment_date", appointmentDate);

        if (!checkAppointmentQuery.exec() || !checkAppointmentQuery.next()) {
            throw std::runtime_error("DB_ERROR");
        }

        int appointmentCount = checkAppointmentQuery.value("appointment_count").toInt();
        if (appointmentCount >= 20) { // 每天最多20个预约
            throw std::runtime_error("NO_SLOTS_AVAILABLE");
        }

        // 插入预约记录 - 初始状态为待支付
        QSqlQuery insertQuery(m_db);
        insertQuery.prepare("INSERT INTO appointment (patient_id, doctor_id, appointment_date, status) "
                            "VALUES (:patient_id, :doctor_id, :appointment_date, 'pending')");
        insertQuery.bindValue(":patient_id", patientId);
        insertQuery.bindValue(":doctor_id", doctorId);
        insertQuery.bindValue(":appointment_date", appointmentDate + " 09:00:00"); // 添加具体时间

        if (!insertQuery.exec()) {
            throw std::runtime_error("DB_ERROR");
        }

        // 获取新插入的预约ID
        int appointmentId = insertQuery.lastInsertId().toInt();

        // 创建缴费项目 - 统一格式与处方费用保持一致
        QSqlQuery paymentQuery(m_db);
        paymentQuery.prepare("INSERT INTO payment_items "
                             "(patient_id, description, amount, status, type, application_id, created_at) "
                             "VALUES (:patient_id, :description, :amount, :status, :type, :application_id, :created_at)");

        QString description = QString("预约挂号费 - %1医生 (科室:%2)").arg(doctorName).arg(department.isEmpty() ? "未知科室" : department);
        QString appointmentRef = QString("APPT_%1").arg(appointmentId);
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        
        paymentQuery.bindValue(":patient_id", patientId);
        paymentQuery.bindValue(":description", description);
        paymentQuery.bindValue(":amount", registrationFee);
        paymentQuery.bindValue(":status", "pending"); // 统一使用 pending 状态
        paymentQuery.bindValue(":type", "appointment");
        paymentQuery.bindValue(":application_id", appointmentRef);
        paymentQuery.bindValue(":created_at", currentTime);

        if (!paymentQuery.exec()) {
            qDebug() << "创建缴费项目失败:" << paymentQuery.lastError().text();
            qDebug() << "执行的SQL:" << paymentQuery.executedQuery();
            qDebug() << "绑定的值 - patient_id:" << patientId << ", description:" << description 
                     << ", amount:" << registrationFee << ", status: pending, type: appointment"
                     << ", application_id:" << appointmentRef << ", created_at:" << currentTime;
            throw std::runtime_error("PAYMENT_ITEM_ERROR");
        }

        // 提交事务
        m_db.commit();

        clientSocket->write(QString("MAKE_APPOINTMENT_SUCCESS#%1#%2#%3\n").arg(appointmentId).arg(doctorId).arg(registrationFee).toUtf8());
        qDebug() << "预约成功:" << patientId << "预约了医生" << doctorId << "，费用:" << registrationFee;

    } catch (const std::exception &e) {
        // 回滚事务
        m_db.rollback();

        QString errorMsg = e.what();
        if (errorMsg == "DOCTOR_NOT_FOUND") {
            clientSocket->write("MAKE_APPOINTMENT_FAIL#DOCTOR_NOT_FOUND\n");
        } else if (errorMsg == "NO_SLOTS_AVAILABLE") {
            clientSocket->write("MAKE_APPOINTMENT_FAIL#NO_SLOTS_AVAILABLE\n");
        } else if (errorMsg == "PAYMENT_ITEM_ERROR") {
            clientSocket->write("MAKE_APPOINTMENT_FAIL#PAYMENT_ITEM_ERROR\n");
        } else {
            clientSocket->write("MAKE_APPOINTMENT_FAIL#DB_ERROR\n");
        }

        qDebug() << "预约处理失败:" << errorMsg;
    }
}

void Server::handleGetUserAppointments(const QString &message, QTcpSocket *clientSocket) {
    QString patientId = message.section('#', 1, 1);

    QSqlQuery query(m_db);
    query.prepare("SELECT a.appointment_id, a.doctor_id, a.appointment_date, a.status, d.department, d.registration_fee, u.real_name as doctor_name, d.title "
                  "FROM appointment a "
                  "JOIN doctor d ON a.doctor_id = d.id "
                  "JOIN user u ON d.id = u.id "
                  "WHERE a.patient_id = :patient_id AND DATE(a.appointment_date) >= DATE('now') "
                  "ORDER BY a.appointment_date DESC");
    query.bindValue(":patient_id", patientId);

    if (query.exec()) {
        QJsonArray appointmentsArray;

        while (query.next()) {
            QJsonObject appointment;
            appointment["appointment_id"] = query.value("appointment_id").toString();
            appointment["doctor_id"] = query.value("doctor_id").toString();
            appointment["appointment_date"] = query.value("appointment_date").toString();
            appointment["status"] = query.value("status").toString();
            appointment["department"] = query.value("department").toString();
            appointment["doctor_name"] = query.value("doctor_name").toString();
            appointment["doctor_title"] = query.value("title").toString();
            appointment["registration_fee"] = query.value("registration_fee").toDouble();
            appointmentsArray.append(appointment);
        }

        QJsonDocument doc(appointmentsArray);
        QString response = "GET_USER_APPOINTMENTS_SUCCESS#" + doc.toJson(QJsonDocument::Compact);
        clientSocket->write(response.toUtf8() + "\n");
    } else {
        clientSocket->write("GET_USER_APPOINTMENTS_FAIL\n");
    }
}
