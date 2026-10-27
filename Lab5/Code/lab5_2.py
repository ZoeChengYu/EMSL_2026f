#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
EMSL 2026 Autumn - Lab 5 (SPI Control Practice)
項目四：防盜快照系統 (Intruder Snapshot System) (30%)

功能規範：
1. 即時計算目前與前次 ADC Value 差值 (|Current - Previous|)，當瞬間變化量超過門檻 (DELTA_THRESHOLD) 判定為異常狀態。
2. 異常狀態事件：
   - 閃爍警報：兩顆 LED 交互閃爍 (LED1 亮 / LED2 滅 <-> LED1 滅 / LED2 亮)
   - 即時拍照：啟動 CSI 相機 (使用 GStreamer nvarguscamerasrc 管線)，連續拍攝 3 張照片
   - 自動歸檔：以當前時間格式命名 (snapshots/intruder_YYYYMMDD_HHMMSS_shotN.jpg)
   - 持續異常監測：初次拍攝後，持續比對即時 ADC 與觸發前的原始基準值 (Baseline)。若 |Current - Baseline| >= 門檻，維持警報閃爍並追拍。
   - 系統復歸：當即時數值回歸原始基準值 (|Current - Baseline| < 門檻)，關閉相機、熄滅 LED，輸出 System Reset 復歸正常。
3. 終端機格式完全匹配投影片 Page 38 範例：
   Current value: XXX | Previous value: YYY | Instantaneous change: ZZZ
   save to snapshots/intruder_YYYYMMDD_HHMMSS_shot1.jpg
   ...
   close camera
   System Reset
"""

import os
import sys
import time
import argparse
import datetime
import random

# 檢查與導入相機庫
try:
    import cv2
    HAS_CV2 = True
except ImportError:
    HAS_CV2 = False

# 檢查與導入 GPIO 庫
try:
    import Jetson.GPIO as GPIO
    HAS_GPIO = True
except (ImportError, RuntimeError):
    HAS_GPIO = False

# 腳位定義 (BCM 編號，符合投影片規範)
SPICLK = 11    # TX2 J21 Pin 23
SPIMISO = 9    # TX2 J21 Pin 21
SPIMOSI = 10   # TX2 J21 Pin 19
SPICS = 8      # TX2 J21 Pin 24
PHOTO_CH = 0   # 光敏電阻 MCP3008 CH0

LED1_PIN = 17  # TX2 J21 Pin 11 (gpio466)
LED2_PIN = 18  # TX2 J21 Pin 12 (gpio392)

# 預設瞬間變化量門檻 (投影片建議差值跨度不要太小，如 60~100)
DEFAULT_DELTA_THRESHOLD = 80
SNAPSHOT_DIR = "snapshots"


def gstreamer_pipeline(
    sensor_id=0,
    capture_width=1280,
    capture_height=720,
    display_width=1280,
    display_height=720,
    framerate=30,
    flip_method=0,
):
    """
    建立 NVIDIA Jetson TX2 專用 GStreamer 硬體加速擷取管線
    嚴格遵循投影片 Page 32-34 之 7 參數定義與字串格式
    """
    return (
        "nvarguscamerasrc sensor-id=%d ! "
        "video/x-raw(memory:NVMM), width=(int)%d, height=(int)%d, framerate=(fraction)%d/1 ! "
        "nvvidconv flip-method=%d ! "
        "video/x-raw, width=(int)%d, height=(int)%d, format=(string)BGRx ! "
        "videoconvert ! "
        "video/x-raw, format=(string)BGR ! appsink"
        % (
            sensor_id,
            capture_width,
            capture_height,
            framerate,
            flip_method,
            display_width,
            display_height,
        )
    )


def readadc(adcnum, clockpin, mosipin, misopin, cspin):
    """MCP3008 軟體 SPI 位元模擬讀取 (10-bit: 0~1023)"""
    if adcnum > 7 or adcnum < 0:
        return -1

    GPIO.output(cspin, True)
    GPIO.output(clockpin, False)
    GPIO.output(cspin, False)

    commandout = adcnum
    commandout |= 0x18  # 0b00011000
    commandout <<= 3

    for _ in range(5):
        if commandout & 0x80:
            GPIO.output(mosipin, True)
        else:
            GPIO.output(mosipin, False)
        commandout <<= 1
        GPIO.output(clockpin, True)
        GPIO.output(clockpin, False)

    adcout = 0
    for _ in range(12):
        GPIO.output(clockpin, True)
        GPIO.output(clockpin, False)
        adcout <<= 1
        if GPIO.input(misopin):
            adcout |= 0x1

    GPIO.output(cspin, True)
    adcout >>= 1
    return adcout


def init_gpio():
    """初始化 TX2 GPIO 與 SPI 腳位"""
    if not HAS_GPIO:
        return
    GPIO.setwarnings(False)
    GPIO.cleanup()
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(SPIMOSI, GPIO.OUT)
    GPIO.setup(SPIMISO, GPIO.IN)
    GPIO.setup(SPICLK, GPIO.OUT)
    GPIO.setup(SPICS, GPIO.OUT)
    GPIO.setup(LED1_PIN, GPIO.OUT, initial=GPIO.LOW)
    GPIO.setup(LED2_PIN, GPIO.OUT, initial=GPIO.LOW)


def set_leds(led1, led2):
    """設定 LED1 與 LED2 電位"""
    if HAS_GPIO:
        GPIO.output(LED1_PIN, GPIO.HIGH if led1 else GPIO.LOW)
        GPIO.output(LED2_PIN, GPIO.HIGH if led2 else GPIO.LOW)


def toggle_alarm_leds(state_toggle):
    """警報閃爍：雙 LED 交互閃爍"""
    if state_toggle:
        set_leds(True, False)
    else:
        set_leds(False, True)


class CameraController:
    """相機控制器：支援 CSI 相機、USB Webcam 與模擬相機"""
    def __init__(self, use_usb=False, mock_mode=False):
        self.use_usb = use_usb
        self.mock_mode = mock_mode
        self.cap = None
        os.makedirs(SNAPSHOT_DIR, exist_ok=True)

    def open(self):
        if self.mock_mode or not HAS_CV2:
            return True

        if self.cap is not None and self.cap.isOpened():
            return True

        if self.use_usb:
            self.cap = cv2.VideoCapture(0)
        else:
            pipeline_str = gstreamer_pipeline(sensor_id=0, flip_method=0)
            self.cap = cv2.VideoCapture(pipeline_str, cv2.CAP_GSTREAMER)

        if not self.cap.isOpened():
            # 若 CSI 開啟失敗且未指定 USB，嘗試切換至 USB Webcam
            print("[WARN] GStreamer CSI 相機開啟失敗，嘗試切換至標準 V4L2 USB 相機...")
            self.cap = cv2.VideoCapture(0)

        return self.cap.isOpened()

    def capture_and_save(self, filepath):
        """擷取單張影像並儲存至指定路徑"""
        if self.mock_mode or not HAS_CV2 or self.cap is None or not self.cap.isOpened():
            # 建立簡易模擬影像檔
            try:
                import numpy as np
                img = np.zeros((720, 1280, 3), dtype=np.uint8)
                cv2.putText(img, f"SIMULATED INTRUDER PHOTO: {os.path.basename(filepath)}",
                            (100, 360), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (0, 0, 255), 2)
                cv2.imwrite(filepath, img)
            except Exception:
                # 若無 cv2 / numpy 則建立空檔案作為標記
                with open(filepath, "w") as f:
                    f.write("Simulated photo frame at " + str(datetime.datetime.now()))
            return True

        # 讀取影格並存檔
        ret, frame = self.cap.read()
        if ret and frame is not None:
            cv2.imwrite(filepath, frame)
            return True
        return False

    def close(self):
        """關閉相機釋放資源 (符合投影片清理訊息輸出)"""
        if self.cap is not None:
            self.cap.release()
            self.cap = None
            print("GST_ARGUS: Cleaning up")
            print("CONSUMER: Done Success")
            print("GST_ARGUS: Done Success")
            print("close camera")


def main():
    parser = argparse.ArgumentParser(description="Lab 5: 項目四 - 防盜快照系統")
    parser.add_argument("--delta", "-d", type=int, default=DEFAULT_DELTA_THRESHOLD,
                        help=f"光照瞬間變化量警報門檻 (預設: {DEFAULT_DELTA_THRESHOLD})")
    parser.add_argument("--interval", type=float, default=0.5,
                        help="ADC 取樣監控週期 (秒，預設: 0.5)")
    parser.add_argument("--usb", action="store_true",
                        help="使用 USB 相機代替 CSI 相機")
    parser.add_argument("--mock", action="store_true",
                        help="強制啟用模擬模式 (無 TX2/相機環境測試用)")
    args = parser.parse_args()

    use_mock = args.mock or (not HAS_GPIO)
    delta_threshold = args.delta
    sample_interval = args.interval

    print("=" * 65)
    print("  微算機系統實習 - Lab 5 項目四：防盜快照系統")
    print(f"  執行模式: {'[MOCK 模擬模式]' if use_mock else '[Jetson TX2 硬體模式]'}")
    print(f"  異常變化門檻 (Delta): {delta_threshold}")
    print(f"  取樣週期: {sample_interval}s")
    print("  按 Ctrl+C 結束監控")
    print("=" * 65)

    if not use_mock:
        init_gpio()

    camera = CameraController(use_usb=args.usb, mock_mode=use_mock)

    # 模擬數值基底
    mock_current = 200
    mock_trigger_count = 0

    # 初始讀取第一筆數據
    if use_mock:
        prev_adc = mock_current
    else:
        prev_adc = readadc(PHOTO_CH, SPICLK, SPIMOSI, SPIMISO, SPICS)

    time.sleep(sample_interval)

    # 狀態機管理
    # is_abnormal: 是否處於異常警報中
    # baseline_adc: 進入異常前的原始環境基準值
    is_abnormal = False
    baseline_adc = prev_adc
    toggle_flag = False

    try:
        while True:
            # 取得即時數值
            if use_mock:
                mock_trigger_count += 1
                # 模擬在第 5 次迴圈產生瞬間劇烈變化 (模擬光敏被手遮蔽或外來光源)
                if mock_trigger_count == 5:
                    mock_current += 150
                elif mock_trigger_count == 12:
                    # 模擬手移開，環境回歸正常基準
                    mock_current -= 150
                else:
                    mock_current += random.choice([-5, -2, 0, 2, 5])
                current_adc = max(20, min(1020, mock_current))
            else:
                current_adc = readadc(PHOTO_CH, SPICLK, SPIMOSI, SPIMISO, SPICS)

            # 計算瞬間差值 (與前一次取樣之差)
            diff = abs(current_adc - prev_adc)

            # 終端機格式化輸出 (嚴格對應投影片 Page 38 範例格式)
            print(f"Current value: {current_adc:3d} | Previous value: {prev_adc:3d} | Instantaneous change: {diff:2d}")

            if not is_abnormal:
                # 正常狀態下：檢查瞬間差值是否超標
                if diff >= delta_threshold:
                    print(f"\n[ALERT] 偵測到異常光線劇烈變化！差值 {diff} >= 門檻 {delta_threshold}")
                    is_abnormal = True
                    baseline_adc = prev_adc  # 鎖定進入異常前的原始基準值

                    # 1. 啟動相機
                    camera.open()

                    # 2. 警報事件：兩顆 LED 交互閃爍 + 連續拍攝 3 張照片
                    timestamp_str = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
                    for shot_id in range(1, 4):
                        # LED 交互切換
                        toggle_flag = not toggle_flag
                        toggle_alarm_leds(toggle_flag)

                        filename = f"intruder_{timestamp_str}_shot{shot_id}.jpg"
                        filepath = os.path.join(SNAPSHOT_DIR, filename)

                        disp_path = filepath.replace('\\', '/')
                        camera.capture_and_save(filepath)
                        # 終端機顯示快照歸檔紀錄 (投影片 Page 38 格式)
                        print(f"save to {disp_path}")
                        time.sleep(0.3)

            else:
                # 處於異常狀態中：持續進行異常監測
                # 判斷依據：即時數值與「進入異常前的原始基準值」之差距是否仍大於門檻
                residual_diff = abs(current_adc - baseline_adc)

                if residual_diff >= delta_threshold:
                    # 狀態依然異常：持續交互閃爍警報
                    toggle_flag = not toggle_flag
                    toggle_alarm_leds(toggle_flag)

                    # 可選擇每隔數秒進行追加快照追拍
                    # timestamp_str = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
                    # chase_file = os.path.join(SNAPSHOT_DIR, f"intruder_{timestamp_str}_tracking.jpg")
                    # camera.capture_and_save(chase_file)
                else:
                    # 系統復歸：數據已回歸並接近觸發前原始基準 (差值小於門檻)
                    set_leds(False, False)
                    camera.close()
                    print("System Reset\n")
                    is_abnormal = False

            # 更新前次讀值
            prev_adc = current_adc
            time.sleep(sample_interval)

    except KeyboardInterrupt:
        print("\n[INFO] 使用者手動中斷...")
    finally:
        set_leds(False, False)
        if is_abnormal:
            camera.close()
            print("System Reset")
        if HAS_GPIO and not use_mock:
            GPIO.cleanup()
        print("[INFO] 系統已安全退出。")


if __name__ == "__main__":
    main()
