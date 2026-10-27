# 微算機系統實習 - Lab 5 (SPI 控制實習)

本專案實作 **Lab 5：SPI 通訊協定、MCP3008 ADC 類比數位轉換、光敏電阻感測器、GPIO 雙 LED 控制與 CSI 相機防盜快照系統** 的完整功能。

---

## 目錄結構與檔案說明

依據簡報規定之母資料夾與子資料夾規範：
```
Lab5/
├── Code/                          # 存放專案程式碼
│   ├── lab5_1.py                  # 項目一與項目二整合程式 (光敏電阻讀取 + LED 門檻控制)
│   ├── lab5_2.py                  # 項目四程式 (防盜快照系統：差值警報 + 雙 LED 交互閃爍 + 連續快照歸檔 + 復歸)
│   ├── test_csi_camera.sh         # 項目三測試指令腳本 (測試 CSI 相機畫面)
│   ├── requirements.txt           # Python 套件依賴
│   └── snapshots/                 # 項目四拍照自動歸檔目錄
├── Report/                        # 存放實驗報告
│   ├── Lab05_第08組_小組報告.docx   # 符合規範之 Word 小組報告
│   └── Lab05_第08組_小組報告.md     # Markdown 完整報告版本
└── README.md                      # 本說明文件
```

---

## 實驗器材與接線對照表

### 1. 器材清單
- NVIDIA Jetson TX2 開發板
- MCP3008 10-bit ADC 晶片
- 光敏電阻感測模組 (含分壓電路)
- 麵包板與杜邦線
- LED 燈 2 顆 (LED1, LED2)
- CSI 介面相機 (或 USB 相機)

### 2. TX2 J21 擴充接腳與麵包板接線對照

| 元件腳位 | 元件功能 | TX2 J21 實體腳位 (BOARD) | BCM 編號 / 名稱 | 備註說明 |
| :--- | :--- | :--- | :--- | :--- |
| **MCP3008 Pin 16** | VDD (晶片供電) | Pin 1 | 3.3V | 接麵包板正極軌 (Red) |
| **MCP3008 Pin 15** | VREF (參考電壓) | Pin 1 | 3.3V | 接麵包板正極軌 (Red) |
| **MCP3008 Pin 14** | AGND (類比接地) | Pin 6 | GND | 接麵包板負極軌 (Black) |
| **MCP3008 Pin 9** | DGND (數位接地) | Pin 6 | GND | 接麵包板負極軌 (Black) |
| **MCP3008 Pin 13** | CLK (SPI 時脈) | Pin 23 | BCM 11 (SPICLK) | 時脈控制 |
| **MCP3008 Pin 12** | DOUT (資料輸出/MISO) | Pin 21 | BCM 9 (SPIMISO) | ADC 傳回數位數據 |
| **MCP3008 Pin 11** | DIN (資料輸入/MOSI) | Pin 19 | BCM 10 (SPIMOSI) | 送出通道設定指令 |
| **MCP3008 Pin 10** | CS/SHDN (片選/致能) | Pin 24 | BCM 8 (SPICS / CE0) | 低電位有效 |
| **光敏電阻模組 S** | 類比訊號輸出 | MCP3008 Pin 1 | CH0 | 類比輸入通道 0 |
| **光敏電阻模組 +** | 供電端 | Pin 1 | 3.3V | 接 3.3V |
| **光敏電阻模組 -** | 接地端 | Pin 6 | GND | 接 GND (若數值反向請交換 - 與 S) |
| **LED 1 (正極)** | 警示/狀態燈 1 | Pin 11 | BCM 17 (gpio466) | 經限流電阻接 Pin 11，負極接 GND |
| **LED 2 (正極)** | 警示/狀態燈 2 | Pin 12 | BCM 18 (gpio392) | 經限流電阻接 Pin 12，負極接 GND |

---

## 各項目執行指南

### 項目一與項目二：讀取光敏電阻數值 & 設定門檻值控制 LED (30%)
此項目將項目一與項目二整合於同一程式 `lab5_1.py`：
- 即時從 MCP3008 通道 0 讀取 10-bit ADC 數值 (0~1023)。
- 依據設定之光線門檻值控制 LED1 與 LED2 開關狀態。
- 終端機格式化輸出 LED 燈與光敏電阻狀態。

```bash
cd Lab5/Code

# 在 Jetson TX2 上執行 (預設門檻 200，可視現場光線調整)
python3 lab5_1.py --threshold 200

# 自訂取樣頻率與門檻值範例：
python3 lab5_1.py --threshold 250 --delay 0.5

# (可選) 離線/無硬體測試模擬模式：
python3 lab5_1.py --mock
```

**終端機輸出範例：**
```text
LED1 on
LED2 off
adc_value= 233
LED1 off
LED2 off
adc_value= 77
```

---

### 項目三：測試 CSI Camera (10%)
項目三為 Linux 指令測試，不需另寫程式。

1. **連線注意事項**：使用 SSH 連線至 TX2 時，務必加上 `-Y` 參數啟用 X11 轉發：
   ```bash
   ssh -Y nvidia@192.168.55.1
   ```
2. **執行測試指令**：
   ```bash
   cd Lab5/Code
   chmod +x test_csi_camera.sh
   ./test_csi_camera.sh
   ```
   或直接執行 GStreamer 指令：
   ```bash
   gst-launch-1.0 nvarguscamerasrc ! nvvidconv ! xvimagesink
   ```
   *(若因 JetPack 版本使用舊版核心，可改用 `gst-launch-1.0 nvcamerasrc ! ...` 備用指令)*
3. **成果截圖**：截取鏡頭彈出的即時視訊畫面 (附上同組可愛合照) 貼入報告中。

---

### 項目四：防盜快照系統 (30%)
執行防盜監控主程式 `lab5_2.py`：
1. 即時計算目前與前次 ADC 數值之瞬間差值。
2. 當差值超過警報門檻 (例如瞬間遮光或開燈變化 `delta >= 80`)：
   - 兩顆 LED 交互閃爍發出警報。
   - 啟動 CSI 相機 (透過 GStreamer 硬體加速管線)，連續自動拍攝 3 張快照。
   - 快照自動以時間戳記命名歸檔至 `snapshots/` 目錄。
3. **持續異常監測**：拍攝後持續追蹤即時數值與「進入異常前的原始基準值」之差距。若異常尚未解除，維持警報閃爍與追拍。
4. **系統復歸**：當即時數值回歸原始基準值 (差距小於門檻) 時，自動釋放相機資源、熄滅 LED，復歸正常監控狀態。

```bash
cd Lab5/Code

# 在 Jetson TX2 上執行 (預設變化門檻 80)
python3 lab5_2.py --delta 80

# 若使用 USB Webcam 代替 CSI 鏡頭：
python3 lab5_2.py --delta 80 --usb

# (可選) 模擬模式測試：
python3 lab5_2.py --mock
```

**終端機輸出範例：**
```text
Current value: 219 | Previous value: 207 | Instantaneous change: 12
Current value: 205 | Previous value: 219 | Instantaneous change: 14
Current value: 274 | Previous value: 175 | Instantaneous change: 99

[ALERT] 偵測到異常光線劇烈變化！差值 99 >= 門檻 80
save to snapshots/intruder_20261028_102818_shot1.jpg
save to snapshots/intruder_20261028_102818_shot2.jpg
save to snapshots/intruder_20261028_102818_shot3.jpg
GST_ARGUS: Cleaning up
CONSUMER: Done Success
GST_ARGUS: Done Success
close camera
System Reset
```

---

## 核心原理解析

### 1. MCP3008 SPI 通訊協定 (位元敲擊 Bit-banging)
MCP3008 是 10-bit 8-channel 的類比數位轉換晶片，通訊採 SPI Mode (0,0)：
- MCU 送出 5 個位元指令：`Start bit (1)` + `SGL/DIFF (1)` + `通道位元 (D2, D1, D0)`。
- MCP3008 回傳：1 個 null bit 加上 10 個 ADC 數位取樣位元 (0 ~ 1023)。
- 量化電壓計算：
  $$V_{IN} = \frac{\text{ADC Value} \times V_{REF}}{1024} = \frac{\text{ADC Value} \times 3.3\text{V}}{1024}$$

### 2. 光敏電阻 (LDR) 分壓電路特性
- 依照分壓電路配置，$10\text{ k}\Omega$ 固定電阻在上，LDR 在接地端：
  $$V_{OUT} = 3.3\text{V} \times \frac{R_{LDR}}{10\text{k}\Omega + R_{LDR}}$$
- 強光時：LDR 電阻急劇下降 ($< 1\text{ k}\Omega$)，$V_{OUT}$ 降低，ADC 讀值偏小 (約 50 ~ 150)。
- 遮光/暗處時：LDR 電阻大幅升高 ($> 100\text{ k}\Omega$)，$V_{OUT}$ 升高接近 3.3V，ADC 讀值偏大 (約 800 ~ 950)。

### 3. GStreamer 零拷貝 (Zero-Copy) 硬體加速管線
- `nvarguscamerasrc`：透過 libargus 直接存取 TX2 晶片硬體 ISP，進行 RAW Bayer 解碼、自動對焦與色彩校正。
- `video/x-raw(memory:NVMM)`：影像停留在 NVMM 硬體記憶體，繞過 CPU 減輕負載。
- `nvvidconv`：硬體 VIC 處理色彩空間轉換與畫面旋轉 (flip-method)。
- `appsink`：將標準 BGR 格式高幀率影格直接交付 Python OpenCV 處理。
