# performance_check_R4

# Arduino Uno R4 プログラム例集 / Arduino Uno R4 Example Programs  
このリポジトリは、Arduino Uno R4専用のサンプルプログラムをまとめたものです。  
RA4M1の周辺機能、および/またはプログラム性能の評価を目的としています。  
Arduino UNO R4の能力を学習・検証したい初心者から開発者までの活用を意図しています。  
This repository contains example programs specifically designed for Arduino UNO R4.  
The focus is on evaluating peripheral functions and/or program performance of RA4M1.  
It is intended for learners, hobbyists, and developers who want to explore the capabilities of the UNO R4.  

---

## 内容 / Contents
- `led_output_response_test/` :  
スイッチ入力からLED点灯までのレスポンステスト / Test for response from switch input to LED output  
- `ad_conversion_test/` :  
- サンプリング時間を変えてのAD変換テスト / Test for analog-digital conversion with sampling time sweep  
- (preparing)  
---

## 必要な環境 / Requirements
- Arduino IDE（最新版推奨） / Arduino IDE (latest recommended)  
- Arduino Uno R4 MINIMA / Arduino Uno R4 MINIMA  
- 各exampleに必要な部品 / Components required for each example  

---

## ライブラリのインストール / Library Installation
一部のプログラム例では、GPTタイマーとその割り込みの設定のために自作ライブラリが必要です。  
プログラムを実行する前に、以下のリポジトリから取得しインストールしてください：  
Some example requires a custom library for timer GPT and its interrupt handling.  
Please get it from the following repository the program and install before running:  

https://github.com/inteGN/GPT_basicfunction_R4  

---

## License
Copyright (c) 2025 inteGN - MIT License  

