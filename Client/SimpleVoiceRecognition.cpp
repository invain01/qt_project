#include "SimpleVoiceRecognition.h"
#include <QMouseEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QUuid>

SimpleVoiceRecognition::SimpleVoiceRecognition(QObject *parent)
    : QObject(parent)
    , m_recordingProcess(nullptr)
    , m_recognitionProcess(nullptr)
    , m_isRecording(false)
{
    m_voskModelPath = getVoskModelPath();
    
    // 创建临时目录
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir().mkpath(tempDir + "/voice_recognition");
    m_tempAudioPath = tempDir + "/voice_recognition";
}

SimpleVoiceRecognition::~SimpleVoiceRecognition()
{
    if (m_isRecording) {
        stopRecording();
    }
    
    if (m_recordingProcess) {
        m_recordingProcess->kill();
        m_recordingProcess->waitForFinished(3000);
        delete m_recordingProcess;
    }
    
    if (m_recognitionProcess) {
        m_recognitionProcess->kill();
        m_recognitionProcess->waitForFinished(3000);
        delete m_recognitionProcess;
    }
}

QString SimpleVoiceRecognition::getVoskModelPath() const
{
    QStringList searchPaths;
    
    QString appDir = QApplication::applicationDirPath();
    
    searchPaths << appDir + "/vosk-model-small-cn-0.22";
    searchPaths << QDir(appDir).absoluteFilePath("../vosk-model-small-cn-0.22");
    searchPaths << QDir(appDir).absoluteFilePath("../../vosk-model-small-cn-0.22");
    searchPaths << QDir(appDir).absoluteFilePath("../../../Client/vosk-model-small-cn-0.22");
    searchPaths << "./vosk-model-small-cn-0.22";
    searchPaths << "../vosk-model-small-cn-0.22";
    
    for (const QString &path : searchPaths) {
        QDir dir(path);
        if (dir.exists()) {
            qDebug() << "找到VOSK模型路径:" << dir.absolutePath();
            return dir.absolutePath();
        }
    }
    
    return QString();
}

bool SimpleVoiceRecognition::isVoskModelAvailable() const
{
    return !m_voskModelPath.isEmpty() && QDir(m_voskModelPath).exists();
}

void SimpleVoiceRecognition::startRecording()
{
    if (m_isRecording) {
        return;
    }
    
    if (!isVoskModelAvailable()) {
        emit recognitionError("VOSK语音识别模型未找到");
        return;
    }
    
    // 生成唯一的音频文件名
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_currentAudioFile = m_tempAudioPath + "/voice_" + uuid + ".wav";
    
    // 创建录音脚本
    QString recordingScript = m_tempAudioPath + "/record_audio.py";
    createRecordingScript(recordingScript);
    
    // 启动录音进程
    if (m_recordingProcess) {
        delete m_recordingProcess;
    }
    
    m_recordingProcess = new QProcess(this);
    connect(m_recordingProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SimpleVoiceRecognition::onRecordingProcessFinished);
    
    QStringList arguments;
    arguments << recordingScript << m_currentAudioFile;
    
    m_recordingProcess->start("python", arguments);
    
    if (m_recordingProcess->waitForStarted()) {
        m_isRecording = true;
        emit recordingStateChanged(true);
        qDebug() << "开始录音:" << m_currentAudioFile;
    } else {
        emit recognitionError("无法启动录音进程，请确保Python已安装");
    }
}

void SimpleVoiceRecognition::stopRecording()
{
    if (!m_isRecording) {
        return;
    }
    
    m_isRecording = false;
    emit recordingStateChanged(false);
    
    if (m_recordingProcess && m_recordingProcess->state() == QProcess::Running) {
        // 发送停止信号给Python脚本
        m_recordingProcess->write("stop\n");
        m_recordingProcess->waitForBytesWritten();
        
        // 等待进程结束
        if (!m_recordingProcess->waitForFinished(5000)) {
            m_recordingProcess->kill();
        }
    }
}

void SimpleVoiceRecognition::createRecordingScript(const QString &scriptPath)
{
    QString script = R"(
import sys
import pyaudio
import wave
import threading
import signal
import time

class AudioRecorder:
    def __init__(self, filename):
        self.filename = filename
        self.is_recording = False
        self.frames = []
        
        # 音频参数
        self.chunk = 1024
        self.format = pyaudio.paInt16
        self.channels = 1
        self.rate = 16000
        
        self.audio = pyaudio.PyAudio()
        self.stream = None
        
    def start_recording(self):
        try:
            self.stream = self.audio.open(
                format=self.format,
                channels=self.channels,
                rate=self.rate,
                input=True,
                frames_per_buffer=self.chunk
            )
            
            self.is_recording = True
            print("开始录音...")
            
            while self.is_recording:
                try:
                    data = self.stream.read(self.chunk, exception_on_overflow=False)
                    self.frames.append(data)
                except Exception as e:
                    print(f"录音错误: {e}")
                    break
                    
        except Exception as e:
            print(f"无法启动录音: {e}")
            return False
            
        return True
        
    def stop_recording(self):
        self.is_recording = False
        
        if self.stream:
            self.stream.stop_stream()
            self.stream.close()
            
        self.audio.terminate()
        
        # 保存音频文件
        try:
            wf = wave.open(self.filename, 'wb')
            wf.setnchannels(self.channels)
            wf.setsampwidth(self.audio.get_sample_size(self.format))
            wf.setframerate(self.rate)
            wf.writeframes(b''.join(self.frames))
            wf.close()
            print(f"音频已保存: {self.filename}")
            return True
        except Exception as e:
            print(f"保存音频失败: {e}")
            return False

def main():
    if len(sys.argv) != 2:
        print("用法: python record_audio.py <输出文件>")
        sys.exit(1)
        
    output_file = sys.argv[1]
    recorder = AudioRecorder(output_file)
    
    # 启动录音
    recording_thread = threading.Thread(target=recorder.start_recording)
    recording_thread.start()
    
    try:
        # 等待停止信号
        while True:
            line = input()
            if line.strip() == "stop":
                break
    except:
        pass
    
    # 停止录音
    recorder.stop_recording()
    recording_thread.join()
    
    print("录音完成")

if __name__ == "__main__":
    main()
)";
    
    QFile file(scriptPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << script;
        file.close();
    }
}

void SimpleVoiceRecognition::createRecognitionScript(const QString &scriptPath)
{
    QString script = R"(
import sys
import json
import wave
import os

try:
    from vosk import Model, KaldiRecognizer, SetLogLevel
except ImportError:
    print(json.dumps({"error": "VOSK not installed. Please install with: pip install vosk"}, ensure_ascii=False))
    sys.exit(1)

def recognize_audio(audio_path, model_path):
    try:
        SetLogLevel(0)
        
        if not os.path.exists(audio_path):
            return {"error": "音频文件不存在"}
            
        if not os.path.exists(model_path):
            return {"error": "VOSK模型不存在"}
        
        wf = wave.open(audio_path, "rb")
        
        if wf.getnchannels() != 1 or wf.getsampwidth() != 2 or wf.getcomptype() != "NONE":
            wf.close()
            return {"error": "音频文件必须是单声道16位PCM WAV格式"}
        
        model = Model(model_path)
        rec = KaldiRecognizer(model, wf.getframerate())
        rec.SetWords(True)
        
        results = []
        
        while True:
            data = wf.readframes(4000)
            if len(data) == 0:
                break
            if rec.AcceptWaveform(data):
                result = json.loads(rec.Result())
                text = result.get("text", "")
                if text:
                    results.append(text)
        
        final_result = json.loads(rec.FinalResult())
        final_text = final_result.get("text", "")
        if final_text:
            results.append(final_text)
        
        wf.close()
        
        full_text = " ".join(results).strip()
        
        if full_text:
            return {"success": True, "text": full_text}
        else:
            return {"error": "未识别到语音内容"}
            
    except Exception as e:
        return {"error": f"识别过程出错: {str(e)}"}

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(json.dumps({"error": "参数错误"}, ensure_ascii=False))
        sys.exit(1)
    
    audio_path = sys.argv[1]
    model_path = sys.argv[2]
    
    result = recognize_audio(audio_path, model_path)
    print(json.dumps(result, ensure_ascii=False))
)";
    
    QFile file(scriptPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << script;
        file.close();
    }
}

void SimpleVoiceRecognition::onRecordingProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    
    qDebug() << "录音进程结束，退出码:" << exitCode;
    
    if (exitCode == 0 && QFile::exists(m_currentAudioFile)) {
        // 录音成功，开始识别
        performRecognition(m_currentAudioFile);
    } else {
        emit recognitionError("录音失败，请检查麦克风设备和权限");
    }
}

void SimpleVoiceRecognition::performRecognition(const QString &audioPath)
{
    QString recognitionScript = m_tempAudioPath + "/voice_recognition.py";
    createRecognitionScript(recognitionScript);
    
    if (m_recognitionProcess) {
        delete m_recognitionProcess;
    }
    
    m_recognitionProcess = new QProcess(this);
    connect(m_recognitionProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &SimpleVoiceRecognition::onRecognitionProcessFinished);
    
    QStringList arguments;
    arguments << recognitionScript << audioPath << m_voskModelPath;
    
    m_recognitionProcess->start("python", arguments);
}

void SimpleVoiceRecognition::onRecognitionProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    
    if (exitCode == 0) {
        QByteArray output = m_recognitionProcess->readAllStandardOutput();
        QString outputStr = QString::fromUtf8(output).trimmed();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(outputStr.toUtf8(), &error);
        
        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            
            if (obj.contains("success") && obj["success"].toBool()) {
                QString text = obj["text"].toString();
                emit recognitionResult(text);
            } else if (obj.contains("error")) {
                QString errorMsg = obj["error"].toString();
                emit recognitionError(errorMsg);
            }
        } else {
            emit recognitionError("识别结果解析失败");
        }
    } else {
        QByteArray errorOutput = m_recognitionProcess->readAllStandardError();
        QString errorStr = QString::fromUtf8(errorOutput);
        emit recognitionError("语音识别失败: " + errorStr);
    }
    
    // 清理临时文件
    QFile::remove(m_currentAudioFile);
}

// SimpleVoiceButton 实现
SimpleVoiceButton::SimpleVoiceButton(QWidget *parent)
    : QPushButton(parent)
    , m_voiceRecognition(new SimpleVoiceRecognition(this))
    , m_isPressed(false)
    , m_isRecording(false)
    , m_animationTimer(new QTimer(this))
    , m_animationFrame(0)
{
    setText("🎤");
    setToolTip("按住说话，松开识别");
    setFixedSize(40, 30);
    
    connect(m_voiceRecognition, &SimpleVoiceRecognition::recognitionResult,
            this, &SimpleVoiceButton::onRecognitionResult);
    connect(m_voiceRecognition, &SimpleVoiceRecognition::recognitionError,
            this, &SimpleVoiceButton::onRecognitionError);
    connect(m_voiceRecognition, &SimpleVoiceRecognition::recordingStateChanged,
            this, &SimpleVoiceButton::onRecordingStateChanged);
    
    connect(m_animationTimer, &QTimer::timeout, this, &SimpleVoiceButton::updateAnimation);
    
    updateButtonAppearance();
}

SimpleVoiceButton::~SimpleVoiceButton()
{
}

void SimpleVoiceButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isPressed = true;
        m_voiceRecognition->startRecording();
    }
    QPushButton::mousePressEvent(event);
}

void SimpleVoiceButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isPressed) {
        m_isPressed = false;
        m_voiceRecognition->stopRecording();
    }
    QPushButton::mouseReleaseEvent(event);
}

void SimpleVoiceButton::updateAnimation()
{
    m_animationFrame = (m_animationFrame + 1) % 3;
    update();
}

void SimpleVoiceButton::updateButtonAppearance()
{
    if (m_isRecording) {
        setStyleSheet("QPushButton { background-color: #ffcccc; border: 2px solid red; border-radius: 5px; }");
        setText("🔴");
        setToolTip("正在录音...");
        m_animationTimer->start(500);
    } else {
        setStyleSheet("QPushButton { background-color: #f0f0f0; border: 1px solid #ccc; border-radius: 5px; }");
        setText("🎤");
        setToolTip("按住说话，松开识别");
        m_animationTimer->stop();
    }
}

void SimpleVoiceButton::onRecognitionResult(const QString &text)
{
    if (!text.isEmpty()) {
        emit voiceTextRecognized(text);
    }
}

void SimpleVoiceButton::onRecognitionError(const QString &error)
{
    emit voiceError(error);
}

void SimpleVoiceButton::onRecordingStateChanged(bool isRecording)
{
    m_isRecording = isRecording;
    updateButtonAppearance();
}
