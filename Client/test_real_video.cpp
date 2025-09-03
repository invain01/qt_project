#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QTcpSocket>
#include "EnhancedVideoCallDialog.h"
#include "videocallwindow.h"

class TestMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    TestMainWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
    }

private slots:
    void openEnhancedVideoCall()
    {
        // 创建模拟的socket连接（实际使用中应该是真实的连接）
        QTcpSocket *socket = new QTcpSocket(this);
        
        EnhancedVideoCallDialog *dialog = new EnhancedVideoCallDialog(
            socket, "doctor001", "patient002", "张患者", this);
        dialog->show();
    }
    
    void openVideoCallWindow()
    {
        // 创建模拟的socket连接
        QTcpSocket *socket = new QTcpSocket(this);
        
        VideoCallWindow *window = new VideoCallWindow(
            socket, "doctor001", "patient003", "李患者", this);
        window->show();
    }

private:
    void setupUI()
    {
        setWindowTitle("智能医疗 - 音视频通话测试");
        resize(400, 300);
        
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        
        QLabel *titleLabel = new QLabel("音视频通话功能测试");
        titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin: 20px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        
        QLabel *infoLabel = new QLabel(
            "本测试程序演示了真实音视频获取功能：\n"
            "• 支持真实摄像头预览\n"
            "• 支持音频输入输出控制\n"
            "• 支持多摄像头切换\n"
            "• 支持实时音量调节\n"
            "• 集成了test目录中的真实音视频功能"
        );
        infoLabel->setStyleSheet("color: #34495e; margin: 10px; line-height: 1.5;");
        
        QPushButton *enhancedBtn = new QPushButton("打开增强版音视频通话");
        enhancedBtn->setStyleSheet(
            "QPushButton {"
            "    background-color: #3498db;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 8px;"
            "    padding: 12px 24px;"
            "    font-size: 14px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    background-color: #2980b9;"
            "}"
        );
        connect(enhancedBtn, &QPushButton::clicked, this, &TestMainWindow::openEnhancedVideoCall);
        
        QPushButton *standardBtn = new QPushButton("打开标准音视频通话");
        standardBtn->setStyleSheet(
            "QPushButton {"
            "    background-color: #27ae60;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 8px;"
            "    padding: 12px 24px;"
            "    font-size: 14px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    background-color: #229954;"
            "}"
        );
        connect(standardBtn, &QPushButton::clicked, this, &TestMainWindow::openVideoCallWindow);
        
        QLabel *noteLabel = new QLabel(
            "注意：首次使用时系统可能会要求摄像头和麦克风权限"
        );
        noteLabel->setStyleSheet("color: #e74c3c; font-style: italic; margin: 10px;");
        noteLabel->setAlignment(Qt::AlignCenter);
        
        mainLayout->addWidget(titleLabel);
        mainLayout->addWidget(infoLabel);
        mainLayout->addStretch();
        mainLayout->addWidget(enhancedBtn);
        mainLayout->addWidget(standardBtn);
        mainLayout->addWidget(noteLabel);
        mainLayout->addStretch();
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    TestMainWindow window;
    window.show();
    
    return app.exec();
}

#include "test_real_video.moc"
