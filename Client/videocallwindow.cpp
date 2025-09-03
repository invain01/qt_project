#include "videocallwindow.h"
#include <QMessageBox>
#include <QSplitter>
#include <QGroupBox>
#include <QSpacerItem>
#include <QStyle>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QDateTime>

VideoCallWindow::VideoCallWindow(QTcpSocket *socket, const QString &userId, 
                                 const QString &targetUserId, const QString &targetUserName, 
                                 QWidget *parent)
    : QDialog(parent)
    , m_socket(socket)
    , m_userId(userId)
    , m_targetUserId(targetUserId)
    , m_targetUserName(targetUserName)
    , m_camera(nullptr)
    , m_captureSession(nullptr)
    , m_audioInput(nullptr)
    , m_audioOutput(nullptr)
    , m_isCallActive(false)
    , m_isMuted(false)
    , m_isVideoEnabled(true)
    , m_cameraInitialized(false)
    , m_callDurationSeconds(0)
    , m_currentCameraIndex(0)
{
    // 设置窗口基本属性
    setWindowTitle(QString("智能医疗 - 音视频通话 - %1").arg(m_targetUserName));
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
    // 初始化定时器
    m_callTimer = new QTimer(this);
    connect(m_callTimer, &QTimer::timeout, this, &VideoCallWindow::updateCallDuration);
    
    // 连接socket信号
    if (m_socket) {
        connect(m_socket, &QTcpSocket::readyRead, this, &VideoCallWindow::onSocketDataReceived);
    }
    
    setupUI();
    initializeCamera();
    initializeAudio();
    applyStyles();
}

VideoCallWindow::~VideoCallWindow()
{
    // 如果通话还在进行，发送结束通话消息
    if (m_isCallActive) {
        sendCallEnd();
    }
    
    // 清理资源
    if (m_camera) {
        m_camera->stop();
        delete m_camera;
    }
    if (m_captureSession) {
        delete m_captureSession;
    }
    if (m_audioInput) {
        delete m_audioInput;
    }
    if (m_audioOutput) {
        delete m_audioOutput;
    }
}

void VideoCallWindow::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    
    setupVideoWidgets();
    setupControlButtons();
    setupStatusBar();
}

void VideoCallWindow::setupVideoWidgets()
{
    // 创建视频区域的分组框
    QGroupBox *videoGroup = new QGroupBox("视频通话");
    QVBoxLayout *videoGroupLayout = new QVBoxLayout(videoGroup);
    
    // 创建水平分割器
    QSplitter *videoSplitter = new QSplitter(Qt::Horizontal);
    
    // 本地视频窗口
    QFrame *localFrame = new QFrame();
    localFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    QVBoxLayout *localLayout = new QVBoxLayout(localFrame);
    
    QLabel *localLabel = new QLabel("本地视频 (实时摄像头)");
    localLabel->setAlignment(Qt::AlignCenter);
    localLabel->setStyleSheet("font-weight: bold; color: #2c3e50; padding: 5px;");
    
    // 使用真实的QVideoWidget
    m_localVideoWidget = new QVideoWidget();
    m_localVideoWidget->setMinimumSize(400, 300);
    m_localVideoWidget->setStyleSheet("background-color: #34495e; border: 2px solid #3498db;");
    
    localLayout->addWidget(localLabel);
    localLayout->addWidget(m_localVideoWidget);
    
    // 远程视频窗口
    QFrame *remoteFrame = new QFrame();
    remoteFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    QVBoxLayout *remoteLayout = new QVBoxLayout(remoteFrame);
    
    QLabel *remoteLabel = new QLabel(QString("远程视频 - %1").arg(m_targetUserName));
    remoteLabel->setAlignment(Qt::AlignCenter);
    remoteLabel->setStyleSheet("font-weight: bold; color: #2c3e50; padding: 5px;");
    
    // 使用真实的QVideoWidget
    m_remoteVideoWidget = new QVideoWidget();
    m_remoteVideoWidget->setMinimumSize(400, 300);
    m_remoteVideoWidget->setStyleSheet("background-color: #2c3e50; border: 2px solid #e74c3c;");
    
    remoteLayout->addWidget(remoteLabel);
    remoteLayout->addWidget(m_remoteVideoWidget);
    
    videoSplitter->addWidget(localFrame);
    videoSplitter->addWidget(remoteFrame);
    videoSplitter->setStretchFactor(0, 1);
    videoSplitter->setStretchFactor(1, 1);
    
    videoGroupLayout->addWidget(videoSplitter);
    m_mainLayout->addWidget(videoGroup);
}

void VideoCallWindow::setupControlButtons()
{
    // 创建控制按钮分组框
    QGroupBox *controlGroup = new QGroupBox("通话控制");
    QHBoxLayout *controlGroupLayout = new QHBoxLayout(controlGroup);
    
    m_controlLayout = new QHBoxLayout();
    
    // 拨打电话按钮
    m_callButton = new QPushButton("开始通话");
    m_callButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(m_callButton, &QPushButton::clicked, this, &VideoCallWindow::onCallButtonClicked);
    
    // 挂断电话按钮
    m_hangupButton = new QPushButton("挂断");
    m_hangupButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    m_hangupButton->setEnabled(false);
    connect(m_hangupButton, &QPushButton::clicked, this, &VideoCallWindow::onHangupButtonClicked);
    
    // 静音按钮
    m_muteButton = new QPushButton("静音");
    m_muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    m_muteButton->setCheckable(true);
    connect(m_muteButton, &QPushButton::clicked, this, &VideoCallWindow::onMuteButtonClicked);
    
    // 视频开关按钮
    m_videoToggleButton = new QPushButton("关闭视频");
    m_videoToggleButton->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    m_videoToggleButton->setCheckable(true);
    connect(m_videoToggleButton, &QPushButton::clicked, this, &VideoCallWindow::onVideoToggleClicked);
    
    // 切换摄像头按钮
    m_switchCameraButton = new QPushButton("切换摄像头");
    m_switchCameraButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    connect(m_switchCameraButton, &QPushButton::clicked, [this]() {
        if (m_availableCameras.size() > 1) {
            m_currentCameraIndex = (m_currentCameraIndex + 1) % m_availableCameras.size();
            initializeCamera();
        }
    });
    
    // 音量控制
    m_volumeLabel = new QLabel("音量:");
    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(70);
    m_volumeSlider->setMaximumWidth(150);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &VideoCallWindow::onVolumeChanged);
    
    // 添加弹性空间
    QSpacerItem *spacer1 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem *spacer2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    m_controlLayout->addItem(spacer1);
    m_controlLayout->addWidget(m_callButton);
    m_controlLayout->addWidget(m_hangupButton);
    m_controlLayout->addWidget(m_muteButton);
    m_controlLayout->addWidget(m_videoToggleButton);
    m_controlLayout->addWidget(m_switchCameraButton);
    m_controlLayout->addWidget(m_volumeLabel);
    m_controlLayout->addWidget(m_volumeSlider);
    m_controlLayout->addItem(spacer2);
    
    controlGroupLayout->addLayout(m_controlLayout);
    m_mainLayout->addWidget(controlGroup);
}

void VideoCallWindow::setupStatusBar()
{
    // 创建状态栏
    QFrame *statusFrame = new QFrame();
    statusFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    m_statusLayout = new QHBoxLayout(statusFrame);
    
    m_statusLabel = new QLabel("状态: 就绪");
    m_durationLabel = new QLabel("通话时长: 00:00:00");
    m_cameraStatusLabel = new QLabel("摄像头: 未初始化");
    
    m_connectionProgress = new QProgressBar();
    m_connectionProgress->setVisible(false);
    m_connectionProgress->setMaximumWidth(200);
    
    m_statusLayout->addWidget(m_statusLabel);
    m_statusLayout->addWidget(m_cameraStatusLabel);
    m_statusLayout->addStretch();
    m_statusLayout->addWidget(m_connectionProgress);
    m_statusLayout->addWidget(m_durationLabel);
    
    m_mainLayout->addWidget(statusFrame);
}

void VideoCallWindow::initializeCamera()
{
    // 清理之前的摄像头
    if (m_camera) {
        m_camera->stop();
        delete m_camera;
        m_camera = nullptr;
    }
    
    // 获取可用摄像头
    m_availableCameras = QMediaDevices::videoInputs();
    if (!m_availableCameras.isEmpty()) {
        if (m_currentCameraIndex >= m_availableCameras.size()) {
            m_currentCameraIndex = 0;
        }
        
        try {
            m_camera = new QCamera(m_availableCameras[m_currentCameraIndex], this);
            
            // 连接摄像头信号 - 使用队列连接避免递归调用
            connect(m_camera, &QCamera::activeChanged, this, &VideoCallWindow::updateCameraStatus, Qt::QueuedConnection);
            connect(m_camera, &QCamera::errorOccurred, this, &VideoCallWindow::onCameraError, Qt::QueuedConnection);
            
            if (!m_captureSession) {
                m_captureSession = new QMediaCaptureSession(this);
            }
            
            m_captureSession->setCamera(m_camera);
            m_captureSession->setVideoOutput(m_localVideoWidget);
            
            // 启动摄像头预览
            m_camera->start();
            m_cameraInitialized = true;
            
            // 更新切换摄像头按钮状态
            if (m_switchCameraButton) {
                m_switchCameraButton->setEnabled(m_availableCameras.size() > 1);
            }
            
            updateCameraStatus();
            qDebug() << "摄像头初始化成功:" << m_availableCameras[m_currentCameraIndex].description();
            
            // 模拟远程视频流 - 在实际应用中，这里应该接收来自对方的视频流
            // 目前先显示一个提示，表明这是远程视频区域
            setupRemoteVideoPlaceholder();
            
        } catch (const std::exception& e) {
            qDebug() << "摄像头初始化异常:" << e.what();
            if (m_cameraStatusLabel) {
                m_cameraStatusLabel->setText("摄像头: 初始化失败");
            }
            
            // 显示摄像头权限错误提示
            QMessageBox::warning(this, "摄像头权限错误", 
                               "无法访问摄像头，可能的原因：\n"
                               "1. 摄像头权限被拒绝\n"
                               "2. 摄像头被其他应用占用\n"
                               "3. 摄像头驱动问题\n\n"
                               "请检查系统设置中的摄像头权限，并确保没有其他应用正在使用摄像头。");
        }
    } else {
        if (m_cameraStatusLabel) {
            m_cameraStatusLabel->setText("摄像头: 未找到可用设备");
        }
        if (m_switchCameraButton) {
            m_switchCameraButton->setEnabled(false);
        }
        qDebug() << "未找到可用摄像头";
        
        QMessageBox::warning(this, "摄像头权限错误", 
                           "未找到可用的摄像头设备。\n\n"
                           "请确保：\n"
                           "1. 摄像头硬件连接正常\n"
                           "2. 摄像头驱动已正确安装\n"
                           "3. 系统摄像头权限已开启");
    }
}

void VideoCallWindow::initializeAudio()
{
    // 初始化音频输入和输出
    m_audioInput = new QAudioInput(this);
    m_audioOutput = new QAudioOutput(this);
    
    if (m_captureSession) {
        m_captureSession->setAudioInput(m_audioInput);
        m_captureSession->setAudioOutput(m_audioOutput);
    }
    
    // 设置初始音量
    if (m_audioOutput) {
        m_audioOutput->setVolume(0.7); // 70%
    }
    
    qDebug() << "音频系统初始化完成";
}

void VideoCallWindow::setupRemoteVideoPlaceholder()
{
    // 在远程视频窗口显示占位符，提示用户这是对方的视频区域
    // 在实际的视频通话应用中，这里应该显示来自对方的实时视频流
    
    // 创建一个标签显示提示信息
    QLabel *placeholderLabel = new QLabel("等待对方视频...", m_remoteVideoWidget);
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setStyleSheet(
        "QLabel {"
        "    color: white;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    background-color: rgba(0, 0, 0, 0.7);"
        "    border-radius: 8px;"
        "    padding: 20px;"
        "}"
    );
    
    // 设置标签的大小和位置
    placeholderLabel->setGeometry(50, 100, 300, 100);
    placeholderLabel->show();
    
    qDebug() << "远程视频占位符设置完成";
    
    // 在通话开始后，模拟显示对方的视频
    // 注意：在真实的应用中，这里应该接收并解码来自网络的视频流
    QTimer::singleShot(3000, [this, placeholderLabel]() {
        if (m_isCallActive) {
            placeholderLabel->setText("对方视频连接中...\n\n注意：当前为演示版本\n实际应用需要实现视频流传输");
            placeholderLabel->setStyleSheet(
                "QLabel {"
                "    color: #3498db;"
                "    font-size: 14px;"
                "    font-weight: bold;"
                "    background-color: rgba(255, 255, 255, 0.9);"
                "    border-radius: 8px;"
                "    padding: 20px;"
                "}"
            );
        }
    });
}

void VideoCallWindow::updateCameraStatus()
{
    if (m_camera) {
        QString status = "摄像头: ";
        if (m_camera->isActive()) {
            status += "运行中";
        } else {
            status += "未激活";
        }
        
        if (m_availableCameras.size() > 1) {
            status += QString(" (%1/%2)").arg(m_currentCameraIndex + 1).arg(m_availableCameras.size());
        }
        
        m_cameraStatusLabel->setText(status);
    }
}

void VideoCallWindow::onCameraStateChanged()
{
    updateCameraStatus();
}

void VideoCallWindow::onCameraError(QCamera::Error error)
{
    QString errorString;
    bool showWarning = false;
    
    switch (error) {
    case QCamera::NoError:
        return; // 没有错误，直接返回
    case QCamera::CameraError:
        errorString = "摄像头设备错误";
        showWarning = true;
        break;
    default:
        // 对于其他类型的错误，只在状态栏显示，不弹窗
        errorString = QString("摄像头状态: %1").arg(static_cast<int>(error));
        showWarning = false;
        break;
    }
    
    if (m_cameraStatusLabel) {
        m_cameraStatusLabel->setText(QString("摄像头: %1").arg(errorString));
    }
    
    // 只有严重错误才弹窗提示
    if (showWarning) {
        QMessageBox::warning(this, "摄像头权限错误", 
                           QString("摄像头发生错误: %1\n\n这可能是由于：\n"
                                  "1. 摄像头权限被拒绝\n"
                                  "2. 摄像头被其他程序占用\n"
                                  "3. 摄像头驱动问题\n\n"
                                  "请检查系统设置中的摄像头权限，并确保没有其他应用正在使用摄像头。").arg(errorString));
    }
    
    qDebug() << "摄像头错误:" << errorString << "显示警告:" << showWarning;
}

void VideoCallWindow::applyStyles()
{
    setStyleSheet(
        "QDialog {"
        "    background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, "
        "    stop:0 #f8f9fa, stop:1 #e9ecef);"
        "}"
        "QGroupBox {"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    color: #2c3e50;"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 8px;"
        "    margin: 10px 0px;"
        "    padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top center;"
        "    padding: 0 5px;"
        "    background-color: #ecf0f1;"
        "    border-radius: 4px;"
        "}"
        "QPushButton {"
        "    background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #3498db, stop:1 #2980b9);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 16px;"
        "    font-weight: bold;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #5dade2, stop:1 #3498db);"
        "}"
        "QPushButton:pressed {"
        "    background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #2980b9, stop:1 #1f618d);"
        "}"
        "QPushButton:checked {"
        "    background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #e74c3c, stop:1 #c0392b);"
        "}"
        "QPushButton:disabled {"
        "    background-color: #95a5a6;"
        "    color: #7f8c8d;"
        "}"
        "QSlider::groove:horizontal {"
        "    border: 1px solid #bdc3c7;"
        "    height: 6px;"
        "    background: #ecf0f1;"
        "    border-radius: 3px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: #3498db;"
        "    border: 1px solid #2980b9;"
        "    width: 16px;"
        "    margin: -5px 0;"
        "    border-radius: 8px;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "    background: #5dade2;"
        "}"
        "QProgressBar {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 4px;"
        "    background-color: #ecf0f1;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #27ae60;"
        "    border-radius: 2px;"
        "}"
        "QLabel {"
        "    color: #2c3e50;"
        "    font-size: 12px;"
        "}"
    );
}

void VideoCallWindow::onCallButtonClicked()
{
    if (!m_isCallActive) {
        m_statusLabel->setText("状态: 发起通话中...");
        m_connectionProgress->setVisible(true);
        m_connectionProgress->setRange(0, 0); // 无限进度条
        m_callButton->setEnabled(false);
        
        // 发送通话请求
        sendCallRequest();
    }
}

void VideoCallWindow::onHangupButtonClicked()
{
    if (m_isCallActive) {
        sendCallEnd();
        
        m_isCallActive = false;
        m_callButton->setEnabled(true);
        m_hangupButton->setEnabled(false);
        m_muteButton->setEnabled(false);
        m_videoToggleButton->setEnabled(false);
        m_volumeSlider->setEnabled(false);
        
        m_statusLabel->setText("状态: 通话已结束");
        m_connectionProgress->setVisible(false);
        m_callTimer->stop();
        m_callDurationSeconds = 0;
        m_durationLabel->setText("通话时长: 00:00:00");
        
        emit callEnded();
    }
}

void VideoCallWindow::onMuteButtonClicked()
{
    m_isMuted = !m_isMuted;
    if (m_isMuted) {
        m_muteButton->setText("取消静音");
        m_muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
        // 实际静音处理
        if (m_audioInput) {
            m_audioInput->setVolume(0.0);
        }
    } else {
        m_muteButton->setText("静音");
        m_muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
        // 恢复音量
        if (m_audioInput) {
            m_audioInput->setVolume(1.0);
        }
    }
}

void VideoCallWindow::onVideoToggleClicked()
{
    m_isVideoEnabled = !m_isVideoEnabled;
    if (m_isVideoEnabled) {
        m_videoToggleButton->setText("关闭视频");
        if (m_camera) {
            m_camera->start();
        }
    } else {
        m_videoToggleButton->setText("开启视频");
        if (m_camera) {
            m_camera->stop();
        }
    }
    updateCameraStatus();
}

void VideoCallWindow::onVolumeChanged(int value)
{
    if (m_audioOutput) {
        m_audioOutput->setVolume(value / 100.0);
    }
    m_volumeLabel->setText(QString("音量: %1%").arg(value));
}

void VideoCallWindow::updateCallDuration()
{
    m_callDurationSeconds++;
    int hours = m_callDurationSeconds / 3600;
    int minutes = (m_callDurationSeconds % 3600) / 60;
    int seconds = m_callDurationSeconds % 60;
    
    m_durationLabel->setText(QString("通话时长: %1:%2:%3")
                            .arg(hours, 2, 10, QChar('0'))
                            .arg(minutes, 2, 10, QChar('0'))
                            .arg(seconds, 2, 10, QChar('0')));
}

void VideoCallWindow::onSocketDataReceived()
{
    QByteArray data = m_socket->readAll();
    QString message = QString::fromUtf8(data);
    
    // 处理TCP粘包问题：按换行符分割消息
    static QByteArray buffer;
    buffer.append(data);

    while (buffer.contains('\n')) {
        int pos = buffer.indexOf('\n');
        QByteArray messageData = buffer.left(pos);
        buffer = buffer.mid(pos + 1);

        QString fullMessage = QString::fromUtf8(messageData);
        qDebug() << "VideoCallWindow收到消息:" << fullMessage;

        QStringList parts = fullMessage.split('#');
        if (parts.isEmpty()) continue;
        
        QString messageType = parts[0];
        
        if (messageType == "VIDEO_CALL_REQUEST") {
            // 收到视频通话请求，显示确认对话框
            if (parts.size() >= 3) {
                QString senderId = parts[1];
                QString receiverId = parts[2];
                
                qDebug() << "收到通话请求:" << "发送者=" << senderId << "接收者=" << receiverId;
                qDebug() << "当前用户ID=" << m_userId << "目标用户ID=" << m_targetUserId;
                
                if (receiverId == m_userId) {
                    QMessageBox::StandardButton reply = QMessageBox::question(this, 
                        "视频通话", 
                        QString("%1 邀请您进行视频通话，是否接受？").arg(m_targetUserName),
                        QMessageBox::Yes | QMessageBox::No);
                    
                    qDebug() << "用户选择:" << (reply == QMessageBox::Yes ? "接受" : "拒绝");
                    
                    sendCallResponse(reply == QMessageBox::Yes);
                    
                    if (reply == QMessageBox::Yes) {
                        m_isCallActive = true;
                        m_callButton->setEnabled(false);
                        m_hangupButton->setEnabled(true);
                        m_muteButton->setEnabled(true);
                        m_videoToggleButton->setEnabled(true);
                        m_volumeSlider->setEnabled(true);
                        
                        m_statusLabel->setText("状态: 通话中");
                        m_connectionProgress->setVisible(false);
                        m_callTimer->start(1000);
                    }
                }
            }
        } else if (messageType == "VIDEO_CALL_RESPONSE") {
            if (parts.size() >= 4) {
                QString senderId = parts[1];
                QString receiverId = parts[2];
                QString accepted = parts[3];
                
                qDebug() << "收到通话响应:" << "发送者=" << senderId << "接收者=" << receiverId << "接受=" << accepted;
                qDebug() << "当前用户ID=" << m_userId << "目标用户ID=" << m_targetUserId;
                
                // 检查这个响应是否是针对我发出的请求
                // 如果我是发起者，那么senderId应该是目标用户，receiverId应该是我
                if (receiverId == m_userId && senderId == m_targetUserId) {
                    if (accepted == "true" || accepted == "1") {
                        m_isCallActive = true;
                        m_callButton->setEnabled(false);
                        m_hangupButton->setEnabled(true);
                        m_muteButton->setEnabled(true);
                        m_videoToggleButton->setEnabled(true);
                        m_volumeSlider->setEnabled(true);
                        
                        m_statusLabel->setText("状态: 通话中");
                        m_connectionProgress->setVisible(false);
                        m_callTimer->start(1000);
                        
                        QMessageBox::information(this, "通话", "对方接受了您的通话请求，通话开始！");
                    } else {
                        m_statusLabel->setText("状态: 通话被拒绝");
                        m_connectionProgress->setVisible(false);
                        m_callButton->setEnabled(true);
                        
                        QMessageBox::information(this, "通话", "对方拒绝了您的通话请求");
                    }
                }
            }
        } else if (messageType == "VIDEO_CALL_END") {
            if (parts.size() >= 3) {
                QString senderId = parts[1];
                QString receiverId = parts[2];
                
                if (receiverId == m_userId && m_isCallActive) {
                    onHangupButtonClicked();
                    QMessageBox::information(this, "通话", "对方已结束通话");
                }
            }
        }
    }
}

void VideoCallWindow::sendCallRequest()
{
    QString message = QString("VIDEO_CALL_REQUEST#%1#%2").arg(m_userId, m_targetUserId);
    m_socket->write(message.toUtf8() + "\n");
}

void VideoCallWindow::sendCallResponse(bool accept)
{
    QString acceptStr = accept ? "true" : "false";
    // 发送响应时：我是发送者，目标用户是接收者
    QString message = QString("VIDEO_CALL_RESPONSE#%1#%2#%3").arg(m_userId, m_targetUserId, acceptStr);
    
    qDebug() << "发送通话响应:" << message;
    qDebug() << "响应内容: 发送者=" << m_userId << "接收者=" << m_targetUserId << "接受=" << acceptStr;
    
    m_socket->write(message.toUtf8() + "\n");
}

void VideoCallWindow::sendCallEnd()
{
    QString message = QString("VIDEO_CALL_END#%1#%2").arg(m_userId, m_targetUserId);
    m_socket->write(message.toUtf8() + "\n");
}