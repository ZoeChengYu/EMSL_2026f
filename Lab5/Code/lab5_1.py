#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
EMSL 2026 Autumn - Lab 5 (SPI Control Practice)
項目一：讀取光敏電阻數值 (20%)
項目二：設定門檻值控制 LED 燈狀態 (10%)
(項目一與項目二寫於同一個程式)

硬體連接說明 (TX2 J21 Pin / BCM 編號):
- MCP3008 VDD, VREF  -> TX2 Pin 1 (3.3V)
- MCP3008 AGND, DGND -> TX2 Pin 6 (GND)
- MCP3008 CLK        -> TX2 Pin 23 (BCM 11, SPICLK)
- MCP3008 DOUT       -> TX2 Pin 21 (BCM 9,  SPIMISO)
- MCP3008 DIN        -> TX2 Pin 19 (BCM 10, SPIMOSI)
- MCP3008 CS/SHDN    -> TX2 Pin 24 (BCM 8,  SPICS)
- 光敏電阻模組 Signal  -> MCP3008 CH0 (Pin 1)
- LED1 (警示燈/狀態燈1) -> TX2 Pin 11 (BCM 17 / gpio466)
- LED2 (警示燈/狀態燈2) -> TX2 Pin 12 (BCM 18 / gpio392)
"""

import sys
import time
import argparse
import random

# 嘗試載入 Jetson.GPIO，若非在 Jetson 環境則啟用模擬模式
try:
    import Jetson.GPIO as GPIO
    HAS_GPIO = True
except (ImportError, RuntimeError):
    HAS_GPIO = False

# 腳位定義 (預設使用 BCM 編碼，與投影片 Page 22 一致)
SPICLK = 11    # TX2 J21 Pin 23
SPIMISO = 9    # TX2 J21 Pin 21
SPIMOSI = 10   # TX2 J21 Pin 19
SPICS = 8      # TX2 J21 Pin 24
PHOTO_CH = 0   # MCP3008 通道 0 接光敏電阻

# LED 輸出腳位 (BCM 17: Pin 11, BCM 18: Pin 12)
DEFAULT_LED1_PIN = 17
DEFAULT_LED2_PIN = 18

# 預設門檻值 (可根據現場光線微調，例如亮處 < 200，遮住光線 > 300)
DEFAULT_THRESHOLD = 200


def readadc(adcnum, clockpin, mosipin, misopin, cspin):
    """
    透過軟體模擬 SPI (Bit-banging) 從 MCP3008 讀取類比轉換數值 (10-bit: 0~1023)
    演算法完全遵循投影片 Page 23-24 之標準實作
    """
    if adcnum > 7 or adcnum < 0:
        return -1

    # CS 拉高後拉低，通知 MCP3008 開始進行通訊
    GPIO.output(cspin, True)
    GPIO.output(clockpin, False)  # 時脈初始為低電位
    GPIO.output(cspin, False)     # CS 置低，開始通訊

    # 送出指令: Start bit (1) + Single-ended bit (1) + 3-bit 通道編號
    commandout = adcnum
    commandout |= 0x18  # 0b00011000: Start bit (1) + SGL (1)
    commandout <<= 3    # 左移 3 位，使最高位對齊 0x80 (共 5 個有效位元)

    for _ in range(5):
        if commandout & 0x80:
            GPIO.output(mosipin, True)
        else:
            GPIO.output(mosipin, False)
        commandout <<= 1
        # 產生 SCLK 時脈脈衝 (上升緣 MCP3008 取樣)
        GPIO.output(clockpin, True)
        GPIO.output(clockpin, False)

    adcout = 0
    # 讀取轉換結果: 包含 1 個空白位元、1 個 null bit 與 10 個 ADC 資料位元 (共 12 bits)
    for _ in range(12):
        GPIO.output(clockpin, True)
        GPIO.output(clockpin, False)
        adcout <<= 1
        if GPIO.input(misopin):
            adcout |= 0x1

    # 結束通訊，將 CS 拉高
    GPIO.output(cspin, True)

    # 捨棄第一個 null bit，取得最終 10-bit 數值 (0 ~ 1023)
    adcout >>= 1
    return adcout


def init_gpio(led1_pin, led2_pin):
    """初始化 GPIO 與 SPI 介面腳位"""
    if not HAS_GPIO:
        print("[INFO] Jetson.GPIO 未偵測到，自動進入 Mock 模擬模式。")
        return

    # 設定編號系統為 BCM
    GPIO.setwarnings(False)
    GPIO.cleanup()
    GPIO.setmode(GPIO.BCM)

    # 設定 SPI 軟體腳位
    GPIO.setup(SPIMOSI, GPIO.OUT)
    GPIO.setup(SPIMISO, GPIO.IN)
    GPIO.setup(SPICLK, GPIO.OUT)
    GPIO.setup(SPICS, GPIO.OUT)

    # 設定 LED 輸出腳位，初始為低電位 (熄滅)
    GPIO.setup(led1_pin, GPIO.OUT, initial=GPIO.LOW)
    GPIO.setup(led2_pin, GPIO.OUT, initial=GPIO.LOW)


def set_leds(led1_pin, led2_pin, led1_state, led2_state):
    """控制 2 顆 LED 燈開關狀態"""
    if HAS_GPIO:
        GPIO.output(led1_pin, GPIO.HIGH if led1_state else GPIO.LOW)
        GPIO.output(led2_pin, GPIO.HIGH if led2_state else GPIO.LOW)


def main():
    parser = argparse.ArgumentParser(description="Lab 5: 項目一與項目二 - 光敏電阻讀取與 LED 門檻控制")
    parser.add_argument("--threshold", "-t", type=int, default=DEFAULT_THRESHOLD,
                        help=f"LED 控制門檻值 (預設: {DEFAULT_THRESHOLD})")
    parser.add_argument("--led1", type=int, default=DEFAULT_LED1_PIN,
                        help=f"LED 1 BCM 腳位 (預設: {DEFAULT_LED1_PIN}, J21 Pin 11)")
    parser.add_argument("--led2", type=int, default=DEFAULT_LED2_PIN,
                        help=f"LED 2 BCM 腳位 (預設: {DEFAULT_LED2_PIN}, J21 Pin 12)")
    parser.add_argument("--delay", "-d", type=float, default=1.0,
                        help="取樣延遲間隔 (秒，預設: 1.0)")
    parser.add_argument("--mock", action="store_true",
                        help="強制啟用模擬模式 (無 TX2 硬體時測試用)")
    args = parser.parse_args()

    use_mock = args.mock or (not HAS_GPIO)
    threshold = args.threshold
    led1_pin = args.led1
    led2_pin = args.led2
    delay = args.delay

    print("=" * 60)
    print("  微算機系統實習 - Lab 5 (SPI 控制實習)")
    print("  項目一：讀取光敏電阻數值 & 項目二：設定門檻值控制 LED 狀態")
    print(f"  模式: {'[MOCK 模擬模式]' if use_mock else '[Jetson TX2 硬體模式]'}")
    print(f"  門檻值設定: {threshold}")
    print(f"  LED 1 腳位: BCM {led1_pin} (TX2 Pin 11)")
    print(f"  LED 2 腳位: BCM {led2_pin} (TX2 Pin 12)")
    print(f"  SPI 腳位: CLK={SPICLK}, MISO={SPIMISO}, MOSI={SPIMOSI}, CS={SPICS}")
    print("  操作說明: 按下 Ctrl+C 可隨時中斷程式並安全清理 GPIO")
    print("=" * 60)

    if not use_mock:
        init_gpio(led1_pin, led2_pin)

    # 模擬數值狀態 (若在 mock 模式下)
    mock_val = 150

    try:
        while True:
            if use_mock:
                # 隨機波動模擬環境光線變化
                mock_val += random.choice([-25, -10, 0, 10, 25, 40])
                mock_val = max(50, min(950, mock_val))
                adc_value = mock_val
            else:
                adc_value = readadc(PHOTO_CH, SPICLK, SPIMOSI, SPIMISO, SPICS)

            # 項目二邏輯：依照門檻值控制 2 顆 LED 狀態
            # 根據分壓電路特性：光線變暗時 LDR 電阻增大，ADC 讀值升高
            # 當 adc_value >= threshold 表示環境偏暗 (或遮蔽光線)，點亮 LED 進行照明/警示
            if adc_value >= threshold:
                led1_on = True
                led2_on = False  # 可設定 LED1 亮、LED2 暗，或依門檻階梯點亮
            else:
                led1_on = False
                led2_on = True

            set_leds(led1_pin, led2_pin, led1_on, led2_on)

            # 終端機顯示 LED 燈與光敏電阻狀態 (嚴格符合投影片 Page 26-27 範例格式)
            print(f"LED1 {'on' if led1_on else 'off'}")
            print(f"LED2 {'on' if led2_on else 'off'}")
            print(f"adc_value= {adc_value}")

            time.sleep(delay)

    except KeyboardInterrupt:
        print("\n[INFO] 接收到中斷訊號，正在清理 GPIO 腳位並關閉程式...")
    finally:
        if HAS_GPIO and not use_mock:
            set_leds(led1_pin, led2_pin, False, False)
            GPIO.cleanup()
        print("[INFO] 程式正常結束。")


if __name__ == "__main__":
    main()
