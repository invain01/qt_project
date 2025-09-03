#ifndef VIDEOCALLWINDOW_H
#define VIDEOCALLWINDOW_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVideoWidget>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QAudioOutput>
#include <QSlider>
#include <QTextEdit>
#include <QLineEdit>
#include <QFrame>
#include <QTimer>
#include <QProgressBar>
#include <QTcpSocket>
#include <QGroupBox>
#include <QSplitter>
#include <QMediaDevices>

class VideoCallWindow : public QDialog
{
    Q_OBJECT

public:
    explicit VideoCallWindow(QTcpSocket *socket, const QString &userId, 
                           const QString &targetUserId, const QString &targetUserName, 
                           QWidget *parent = nullptr);
    ~VideoCallWindow();

signals:
    void callEnded();

private slots:
    void onCallButtonClicked();
    void onHangupButtonClicked();
    void onMuteButtonClicked();
    void onVideoToggleClicked();
    void onVolumeChanged(int value);
    void updateCallDuration();
    void onSocketDataReceived();
    void onCameraStateChanged();
    void onCameraError(QCamera::Error error);

private:
    void setupUI();
    void setupVideoWidgets();
    void setupControlButtons();
    void setupStatusBar();
    void initializeCamera();
    void initializeAudio();
    void applyStyles();
    void sendCallRequest();
    void sendCallResponse(bool accept);
    void sendCallEnd();
    void updateCameraStatus();
    void setupRemoteVideoPlaceholder();

    // UI组件
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_videoLayout;
    QHBoxLayout *m_controlLayout;
    QHBoxLayout *m_statusLayout;
    
    // 真实视频相关组件
    QVideoWidget *m_localVideoWidget;
    QVideoWidget *m_remoteVideoWidget;
    QCamera *m_camera;
    QMediaCaptureSession *m_captureSession;
    QAudioInput *m_audioInput;
    QAudioOutput *m_audioOutput;
    
    // 控制按钮
    QPushButton *m_callButton;
    QPushButton *m_hangupButton;
    QPushButton *m_muteButton;
    QPushButton *m_videoToggleButton;
    QPushButton *m_switchCameraButton;
    QSlider *m_volumeSlider;
    QLabel *m_volumeLabel;
    
    // 状态栏
    QLabel *m_statusLabel;
    QLabel *m_durationLabel;
    QLabel *m_cameraStatusLabel;
    QProgressBar *m_connectionProgress;
    
    // 状态变量
    bool m_isCallActive;
    bool m_isMuted;
    bool m_isVideoEnabled;
    bool m_cameraInitialized;
    QTimer *m_callTimer;
    int m_callDurationSeconds;
    int m_currentCameraIndex;
    
    // 网络通信
    QTcpSocket *m_socket;
    QString m_userId;
    QString m_targetUserId;
    QString m_targetUserName;
    
    // 可用摄像头列表
    QList<QCameraDevice> m_availableCameras;
    
    // 分隔线
    QFrame *m_separatorLine;
};

#endif // VIDEOCALLWINDOW_H