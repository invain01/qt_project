#ifndef SIMPLEVOICERECOGNITION_H
#define SIMPLEVOICERECOGNITION_H

#include <QObject>
#include <QPushButton>
#include <QProcess>
#include <QTimer>
#include <QTemporaryFile>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>
#include <QMessageBox>
#include <QDebug>

/**
 * 简化版语音识别类
 * 使用外部录音工具来避免Qt音频权限问题
 */
class SimpleVoiceRecognition : public QObject
{
    Q_OBJECT

public:
    explicit SimpleVoiceRecognition(QObject *parent = nullptr);
    ~SimpleVoiceRecognition();
    
    void startRecording();
    void stopRecording();
    bool isRecording() const { return m_isRecording; }
    bool isVoskModelAvailable() const;

signals:
    void recognitionResult(const QString &text);
    void recognitionError(const QString &error);
    void recordingStateChanged(bool isRecording);

private slots:
    void onRecordingProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onRecognitionProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString getVoskModelPath() const;
    void performRecognition(const QString &audioPath);
    void createRecordingScript(const QString &scriptPath);
    void createRecognitionScript(const QString &scriptPath);
    
    QProcess *m_recordingProcess;
    QProcess *m_recognitionProcess;
    bool m_isRecording;
    QString m_voskModelPath;
    QString m_tempAudioPath;
    QString m_currentAudioFile;
};

/**
 * 简化版语音按钮
 */
class SimpleVoiceButton : public QPushButton
{
    Q_OBJECT

public:
    explicit SimpleVoiceButton(QWidget *parent = nullptr);
    ~SimpleVoiceButton();

signals:
    void voiceTextRecognized(const QString &text);
    void voiceError(const QString &error);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onRecognitionResult(const QString &text);
    void onRecognitionError(const QString &error);
    void onRecordingStateChanged(bool isRecording);
    void updateAnimation();

private:
    void updateButtonAppearance();
    
    SimpleVoiceRecognition *m_voiceRecognition;
    bool m_isPressed;
    bool m_isRecording;
    QTimer *m_animationTimer;
    int m_animationFrame;
};

#endif // SIMPLEVOICERECOGNITION_H
