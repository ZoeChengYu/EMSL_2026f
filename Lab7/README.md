# 微算機系統實習 - Lab 7：嵌入式系統設備驅動程式與硬體控制

本專案依據 **Lab7-驅動程式結合硬體控制實作設計v2.pdf** 與 **Lecture7_嵌入式系統設備驅動程式與硬體控制.pdf** 規範，於 **NVIDIA Jetson TX2 (Linux 4.9 L4T)** 平台實作完整的 Linux 核心字元裝置驅動、硬體中斷處理與感測自動化控制系統。

---

## 系統架構與檔案結構

```
Lab7/
├── led_driver/              # 項目一：4 顆 LED 字元裝置驅動
│   ├── demo.c               # 核心驅動模組原始碼 (demo.ko)
│   ├── Lab7.c               # 使用者空間控制與狀態查詢程式
│   └── Makefile             # 建置腳本 (產生 demo.ko 與 Lab7)
├── button_driver/           # 項目二：按鍵中斷驅動
│   ├── button_irq.c         # 核心硬體中斷模組原始碼 (button_irq.ko)
│   └── Makefile             # 建置腳本 (產生 button_irq.ko)
├── adc_driver/              # 項目三：GPIO 軟體 SPI MCP3008 ADC 驅動
│   ├── spi_demo.c           # 軟體 SPI 字元驅動模組原始碼 (spi_demo.ko)
│   └── Makefile             # 建置腳本 (產生 spi_demo.ko)
├── adc_watch.py             # 項目三：使用者端 ADC 即時讀取監測程式
├── auto_light.py            # 項目四：光敏感測與 LED 自動照明控制 (雙門檻遲滯演算法)
├── Makefile                 # 頂層一鍵建置 Makefile
├── build_all.sh             # 一鍵編譯所有驅動與程式腳本
├── setup_nodes.sh           # 自動載入模組並建立 /dev 節點腳本
├── cleanup.sh               # 卸載模組、清除節點與釋放 GPIO 腳本
├── Report/                  # 實驗報告目錄
│   ├── generate_report.py   # Word 報告產生腳本
│   └── Lab07_第08組_小組報告.md # Markdown 完整報告
└── README.md                # 本說明文件
```

---

## 硬體接線與腳位對照表 (TX2 J21 40-Pin Header)

### 1. LED1 ~ LED4 接線 (項目一 & 項目四)

| LED 標示 | Linux GPIO 編號 | TX2 J21 實體腳位 (BOARD) | 電路接法說明 |
| :---: | :---: | :---: | :--- |
| **LED 1** | **396** | Pin 12 | 陽極串聯 $220\Omega$ 限流電阻接 Pin 12，陰極接 GND (Pin 14) |
| **LED 2** | **255** | Pin 13 | 陽極串聯 $220\Omega$ 限流電阻接 Pin 13，陰極接 GND (Pin 14) |
| **LED 3** | **296** | Pin 29 | 陽極串聯 $220\Omega$ 限流電阻接 Pin 29，陰極接 GND (Pin 30) |
| **LED 4** | **298** | Pin 31 | 陽極串聯 $220\Omega$ 限流電阻接 Pin 31，陰極接 GND (Pin 30) |

> **說明**：各 GPIO 預設以 high (1) 點亮 LED、low (0) 熄滅 LED。若使用共陽接法，電位邏輯反向。

---

### 2. 按鍵開關模組接線 (項目二)

| 訊號名稱 | Linux GPIO 編號 | TX2 J21 實體腳位 | 接線說明 |
| :---: | :---: | :---: | :--- |
| **按鍵訊號線 (BTN)** | **481** | **Pin 18** | 接按鍵開關一端；模組具備上拉電阻至 3.3V (未按為 High=1) |
| **接地線 (GND)** | - | **Pin 20** (GND) | 接按鍵開關另一端 (按下時導通至 GND，變為 Low=0 下降緣觸發) |

---

### 3. MCP3008 ADC 與光敏電阻接線 (項目三 & 項目四)

| MCP3008 腳位 | 腳位功能 | TX2 J21 實體腳位 | Linux GPIO 編號 | 說明 |
| :--- | :--- | :---: | :---: | :--- |
| **Pin 16 (VDD)** | 晶片工作電源 | Pin 1 | - | 3.3V 電源 |
| **Pin 15 (VREF)** | ADC 參考基準電壓 | Pin 1 | - | 3.3V 電源 |
| **Pin 14 (AGND)** | 類比接地 | Pin 6 | - | 接系統共地 (GND) |
| **Pin 9 (DGND)** | 數位接地 | Pin 9 | - | 接系統共地 (GND) |
| **Pin 13 (CLK)** | SPI 時脈線 (Clock) | **Pin 23** | **427** | TX2 GPIO 輸出時脈 |
| **Pin 12 (DOUT)** | SPI 資料輸出 (MISO) | **Pin 21** | **428** | TX2 GPIO 輸入，讀回 ADC 轉換位元 |
| **Pin 11 (DIN)** | SPI 資料輸入 (MOSI) | **Pin 19** | **429** | TX2 GPIO 輸出，送出通道選取指令 |
| **Pin 10 (CS)** | 晶片選擇 (Chip Select) | **Pin 24** | **430** | TX2 GPIO 輸出，Low 有效致能 |
| **Pin 1 (CH0)** | 類比輸入通道 0 | - | - | 接光敏電阻分壓輸出節點 (Signal) |

---

## 快速上手：一鍵建置與載入

進入 TX2 開發板終端機：

```bash
cd ~/Lab7

# 1. 一鍵編譯所有核心模組與 C 執行檔
make
# 或執行腳本：
chmod +x *.sh
./build_all.sh

# 2. 一鍵載入所有驅動並建立裝置節點 (/dev/demo, /dev/spi_demo)
sudo ./setup_nodes.sh
```

---

## 項目驗收與執行步驟

### 項目一：4 顆 LED 字元裝置驅動程式 (20%)

#### 1. 控制與查詢指令：
```bash
cd ~/Lab7/led_driver

# 點亮 LED1，終端機顯示狀態
sudo ./Lab7 LED1 on
# 輸出：LED1 Status: 1

# 查詢 LED1 狀態 (不改變燈號)
sudo ./Lab7 LED1
# 輸出：LED1 Status: 1

# 熄滅 LED1
sudo ./Lab7 LED1 off
# 輸出：LED1 Status: 0

# 依序測試 LED2, LED3, LED4 的獨立控制
sudo ./Lab7 LED2 on
sudo ./Lab7 LED3 on
sudo ./Lab7 LED4 on
sudo ./Lab7 LED2 off
sudo ./Lab7 LED3 off
sudo ./Lab7 LED4 off
```

#### 2. 驗證 5 種 file_operations 規定日誌：
查看核心日誌輸出：
```bash
sudo dmesg | tail -n 25
```
預期必須完整出現以下 5 項訊息：
```text
Enter Open function
Enter I/O Control function
Enter Write function
Enter Read function
Enter Release function
```

---

### 項目二：按鍵硬體中斷驅動 (10%)

#### 1. 監看中斷核心訊息：
在終端機開啟即時日誌監控：
```bash
sudo dmesg -w | grep --line-buffered "Button Pressed"
```

#### 2. 操作硬體按鍵：
每按一下硬體按鈕，終端機即時印出中斷提示：
```text
Button Pressed! Interrupt Triggered. count=1
Button Pressed! Interrupt Triggered. count=2
Button Pressed! Interrupt Triggered. count=3
```

#### 3. 檢查系統中斷計數：
```bash
grep lab_button_irq /proc/interrupts
```
可看到 `lab_button_irq` 中斷次數隨每次按壓精確累加，且經過 30ms 軟體時間窗抑制消除機械彈跳。

---

### 項目三：GPIO 軟體 SPI MCP3008 ADC 驅動 (20%)

#### 1. 啟動 ADC 即時監看程式：
```bash
cd ~/Lab7
sudo python3 adc_watch.py --interval 0.2
```

#### 2. 驗收標準：
- **環境光照時**：數值約在 `100 ~ 400`（視分壓電路接法）。
- **以手或厚紙遮蔽光敏電阻時**：數值即時跳變至 `600 ~ 950`。
- 終端機格式化輸出範例：
  ```text
  ADC= 215
  ADC= 220
  ADC= 780   <-- 遮光
  ADC= 810
  ADC= 230   <-- 移開遮蔽
  ```
- 按 `Ctrl+C` 可安全退出。

---

### 項目四：光敏感測與 LED 自動照明控制 (20%)

本項目整合項目一與項目三，使用**雙門檻遲滯控制演算法 (Dual-threshold Hysteresis)**，防止光敏數值在臨界點附近跳動導致 LED 頻繁閃爍。

#### 1. 執行自動照明程式：
```bash
cd ~/Lab7

# 執行自動照明 (若暗時 ADC 較高，加上 --dark-high)：
sudo python3 auto_light.py \
    --lab7 ~/Lab7/led_driver/Lab7 \
    --led LED1 \
    --low 350 \
    --high 650 \
    --dark-high

# 若暗時 ADC 較低，則不加 --dark-high：
sudo python3 auto_light.py \
    --lab7 ~/Lab7/led_driver/Lab7 \
    --led LED1 \
    --low 350 \
    --high 650
```

#### 2. 驗收標準：
- **遮光時**：ADC 超過門檻，自動呼叫 `./Lab7 LED1 on`，LED1 點亮，回讀 `readback=1`。
- **照光時**：ADC 低於門檻，自動呼叫 `./Lab7 LED1 off`，LED1 熄滅，回讀 `readback=0`。
- **介於 350~650 中間區間時**：保持前一次狀態，不反覆切換。
- 終端機輸出範例：
  ```text
  ADC= 220 mode=LIGHT LED1=OFF readback=0
  ADC= 215 mode=LIGHT LED1=OFF readback=0
  ADC= 720 mode=DARK  LED1=ON  readback=1
  ADC= 510 mode=DARK  LED1=ON  readback=1
  ADC= 180 mode=LIGHT LED1=OFF readback=0
  ```
- 按 `Ctrl+C` 停止程式時，程式具備安全退出機制，會自動關閉 LED。

---

## 卸載與環境清理

實驗結束或需重新編譯載入時，執行清理腳本：
```bash
cd ~/Lab7
sudo ./cleanup.sh
```
此腳本會：
1. 終止背景執行的 Python 監控程式。
2. 卸載 `spi_demo.ko`、`button_irq.ko`、`demo.ko`。
3. 刪除 `/dev/demo` 與 `/dev/spi_demo` 節點。
4. 釋放所有被佔用的 sysfs GPIO 腳位。

---

## 常見問題與除錯 (Troubleshooting)

1. **`insmod: ERROR: could not insert module ... Device or resource busy`**：
   - 原因：先前載入的模組未卸載，或 GPIO 被 sysfs export 佔用。
   - 解決方法：執行 `sudo ./cleanup.sh` 後重新載入。

2. **`/dev/demo: No such file or directory`**：
   - 原因：裝置節點尚未建立。
   - 解決方法：執行 `sudo ./setup_nodes.sh`，腳本會自動讀取 `/proc/devices` 中的動態 major 編號並執行 `mknod`。

3. **ADC 讀值固定為 0 或 1023 不變**：
   - 檢查 MCP3008 VDD (Pin 16) 與 VREF (Pin 15) 是否確實接在 3.3V。
   - 檢查 AGND (Pin 14) 與 DGND (Pin 9) 是否確實共地。
   - 檢查 DIN (MOSI) 與 DOUT (MISO) 腳位是否接反。
