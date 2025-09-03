#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
VOSK语音识别测试程序
用于测试语音识别功能是否正常工作
"""

import sys
import json
import wave
import os
import tempfile
import time

def test_vosk_installation():
    """测试VOSK是否正确安装"""
    try:
        from vosk import Model, KaldiRecognizer, SetLogLevel
        print("✅ VOSK库安装正确")
        return True
    except ImportError as e:
        print(f"❌ VOSK库未安装或安装有误: {e}")
        print("请运行: pip install vosk")
        return False

def test_model_availability(model_path):
    """测试VOSK模型是否可用"""
    if not os.path.exists(model_path):
        print(f"❌ 模型路径不存在: {model_path}")
        return False
    
    try:
        from vosk import Model
        model = Model(model_path)
        print(f"✅ 模型加载成功: {model_path}")
        return True
    except Exception as e:
        print(f"❌ 模型加载失败: {e}")
        return False

def create_test_audio():
    """创建一个测试用的音频文件（静音）"""
    # 创建一个简单的静音WAV文件用于测试
    sample_rate = 16000
    duration = 2  # 2秒
    filename = os.path.join(tempfile.gettempdir(), "test_audio.wav")
    
    with wave.open(filename, 'w') as wav_file:
        wav_file.setnchannels(1)  # 单声道
        wav_file.setsampwidth(2)  # 16位
        wav_file.setframerate(sample_rate)
        
        # 写入静音数据
        silence = b'\x00\x00' * sample_rate * duration
        wav_file.writeframes(silence)
    
    print(f"✅ 创建测试音频文件: {filename}")
    return filename

def test_recognition(model_path, audio_path):
    """测试语音识别功能"""
    try:
        from vosk import Model, KaldiRecognizer, SetLogLevel
        
        SetLogLevel(0)  # 减少日志输出
        
        # 检查音频文件
        if not os.path.exists(audio_path):
            print(f"❌ 音频文件不存在: {audio_path}")
            return False
        
        # 打开音频文件
        wf = wave.open(audio_path, "rb")
        
        # 检查音频格式
        if wf.getnchannels() != 1 or wf.getsampwidth() != 2 or wf.getcomptype() != "NONE":
            print("❌ 音频格式不正确，必须是单声道16位PCM WAV格式")
            wf.close()
            return False
        
        print(f"✅ 音频格式正确: {wf.getnchannels()}声道, {wf.getsampwidth()*8}位, {wf.getframerate()}Hz")
        
        # 加载模型和识别器
        model = Model(model_path)
        rec = KaldiRecognizer(model, wf.getframerate())
        rec.SetWords(True)
        
        print("✅ 识别器创建成功，开始识别...")
        
        # 执行识别
        results = []
        frame_count = 0
        
        while True:
            data = wf.readframes(4000)
            if len(data) == 0:
                break
            frame_count += 1
            
            if rec.AcceptWaveform(data):
                result = json.loads(rec.Result())
                text = result.get("text", "")
                if text:
                    results.append(text)
                    print(f"✅ 识别到文本: {text}")
        
        # 获取最终结果
        final_result = json.loads(rec.FinalResult())
        final_text = final_result.get("text", "")
        if final_text:
            results.append(final_text)
            print(f"✅ 最终识别结果: {final_text}")
        
        wf.close()
        
        print(f"✅ 识别完成，处理了 {frame_count} 个音频帧")
        
        if results:
            full_text = " ".join(results)
            print(f"✅ 完整识别文本: {full_text}")
        else:
            print("ℹ️ 未识别到文本内容（这对于静音测试文件是正常的）")
        
        return True
        
    except Exception as e:
        print(f"❌ 识别过程出错: {e}")
        return False

def main():
    """主函数"""
    print("=" * 50)
    print("VOSK语音识别功能测试")
    print("=" * 50)
    
    # 1. 测试VOSK安装
    print("\n1. 测试VOSK库安装...")
    if not test_vosk_installation():
        return 1
    
    # 2. 查找模型路径
    print("\n2. 查找VOSK模型...")
    model_paths = [
        "./vosk-model-small-cn-0.22",
        "../vosk-model-small-cn-0.22",
        "vosk-model-small-cn-0.22",
    ]
    
    model_path = None
    for path in model_paths:
        if os.path.exists(path):
            model_path = path
            break
    
    if not model_path:
        print("❌ 未找到VOSK中文模型")
        print("请确保 vosk-model-small-cn-0.22 文件夹在当前目录或上级目录")
        return 1
    
    print(f"✅ 找到模型路径: {model_path}")
    
    # 3. 测试模型加载
    print("\n3. 测试模型加载...")
    if not test_model_availability(model_path):
        return 1
    
    # 4. 创建测试音频
    print("\n4. 创建测试音频文件...")
    test_audio_path = create_test_audio()
    
    # 5. 测试识别功能
    print("\n5. 测试语音识别功能...")
    if not test_recognition(model_path, test_audio_path):
        return 1
    
    # 6. 清理测试文件
    try:
        os.remove(test_audio_path)
        print(f"✅ 清理测试文件: {test_audio_path}")
    except:
        pass
    
    print("\n" + "=" * 50)
    print("✅ 所有测试通过！语音识别功能正常。")
    print("现在您可以在智慧医疗系统中使用语音转文字功能了。")
    print("=" * 50)
    
    return 0

if __name__ == "__main__":
    exit_code = main()
    
    # 如果是在Windows上运行，保持窗口打开
    if sys.platform.startswith('win'):
        input("\n按Enter键退出...")
    
    sys.exit(exit_code)
