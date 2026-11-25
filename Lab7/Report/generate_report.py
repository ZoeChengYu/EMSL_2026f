#!/usr/bin/env python3
"""
generate_report.py - Lab7 實驗小組報告 Word (.docx) 自動產生腳本
國立臺北科技大學 資訊工程學系 微算機系統實習 (Lab 7)
"""

import os
from docx import Document
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT, WD_ALIGN_VERTICAL
from docx.oxml import OxmlElement, parse_xml
from docx.oxml.ns import nsdecls, qn


def set_cell_background(cell, fill_hex):
    shading_elm = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{fill_hex}"/>')
    cell._tc.get_or_add_tcPr().append(shading_elm)


def set_cell_margins(cell, top=100, bottom=100, left=150, right=150):
    tcPr = cell._tc.get_or_add_tcPr()
    tcMar = OxmlElement('w:tcMar')
    for m, val in [('top', top), ('bottom', bottom), ('left', left), ('right', right)]:
        node = OxmlElement(f'w:{m}')
        node.set(qn('w:w'), str(val))
        node.set(qn('w:type'), 'dxa')
        tcMar.append(node)
    tcPr.append(tcMar)


def add_code_block(doc, text):
    table = doc.add_table(rows=1, cols=1)
    table.alignment = WD_TABLE_ALIGNMENT.CENTER
    cell = table.cell(0, 0)
    set_cell_background(cell, "2D3748")  # dark slate background
    set_cell_margins(cell, top=120, bottom=120, left=180, right=180)
    p = cell.paragraphs[0]
    p.paragraph_format.space_before = Pt(2)
    p.paragraph_format.space_after = Pt(2)
    p.paragraph_format.line_spacing = 1.15
    run = p.add_run(text)
    run.font.name = 'Consolas'
    run.font.size = Pt(9)
    run.font.color.rgb = RGBColor(0xED, 0xF2, 0xF7)
    doc.add_paragraph()


def add_callout(doc, text, title="重點說明"):
    table = doc.add_table(rows=1, cols=1)
    table.alignment = WD_TABLE_ALIGNMENT.CENTER
    cell = table.cell(0, 0)
    set_cell_background(cell, "EBF8FF")  # light blue
    set_cell_margins(cell, top=100, bottom=100, left=150, right=150)
    p = cell.paragraphs[0]
    p.paragraph_format.space_before = Pt(2)
    p.paragraph_format.space_after = Pt(2)
    r_title = p.add_run(f"【{title}】\n")
    r_title.font.bold = True
    r_title.font.color.rgb = RGBColor(0x2B, 0x6C, 0xB0)
    r_text = p.add_run(text)
    r_text.font.size = Pt(10)
    r_text.font.color.rgb = RGBColor(0x2D, 0x37, 0x48)
    doc.add_paragraph()


def generate_report():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_path = os.path.join(script_dir, "Lab07_第08組_小組報告.docx")

    doc = Document()

    # Page Margins
    for section in doc.sections:
        section.top_margin = Inches(0.8)
        section.bottom_margin = Inches(0.8)
        section.left_margin = Inches(0.8)
        section.right_margin = Inches(0.8)

    # Base Normal Style
    normal_style = doc.styles['Normal']
    normal_style.font.name = 'Microsoft JhengHei'
    normal_style.font.size = Pt(11)
    normal_style.font.color.rgb = RGBColor(0x22, 0x22, 0x22)

    # Title
    p_title = doc.add_paragraph()
    p_title.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run_title = p_title.add_run("國立臺北科技大學 資訊工程學系\n微算機系統實習小組報告")
    run_title.font.name = 'Microsoft JhengHei'
    run_title.font.size = Pt(18)
    run_title.font.bold = True
    run_title.font.color.rgb = RGBColor(0x1A, 0x36, 0x5D)

    p_sub = doc.add_paragraph()
    p_sub.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run_sub = p_sub.add_run("實驗七：嵌入式系統設備驅動程式與硬體控制 (LED驅動、按鍵中斷、軟體SPI ADC與自動照明)")
    run_sub.font.name = 'Microsoft JhengHei'
    run_sub.font.size = Pt(13)
    run_sub.font.bold = True
    run_sub.font.color.rgb = RGBColor(0x2B, 0x6C, 0xB0)

    doc.add_paragraph()

    # ========================================================
    # Section 1: 組別與組員名單 (含貢獻度)
    # ========================================================
    h1 = doc.add_heading("一、組別與組員名單 (組員貢獻比例)", level=1)
    h1.style.font.color.rgb = RGBColor(0x1A, 0x36, 0x5D)

    table = doc.add_table(rows=5, cols=5)
    table.alignment = WD_TABLE_ALIGNMENT.CENTER
    headers = ["組別", "學號", "姓名", "班級 / 系所", "貢獻比例 (%)"]
    data = [
        ["第 08 組", "113590012", "鄭翔", "資工三", "34%"],
        ["第 08 組", "113590051", "陳韋廷", "資工三", "33%"],
        ["第 08 組", "113590003", "黃國倫", "資工三", "33%"],
        ["總計", "-", "-", "-", "100%"]
    ]

    for col_idx, text in enumerate(headers):
        cell = table.cell(0, col_idx)
        cell.text = text
        set_cell_background(cell, "2B6CB0")
        set_cell_margins(cell, 120, 120, 150, 150)
        p = cell.paragraphs[0]
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        for r in p.runs:
            r.font.bold = True
            r.font.color.rgb = RGBColor(0xFF, 0xFF, 0xFF)

    for row_idx, row_data in enumerate(data):
        for col_idx, text in enumerate(row_data):
            cell = table.cell(row_idx + 1, col_idx)
            cell.text = text
            if row_idx == 3:  # Total row
                set_cell_background(cell, "E2E8F0")
                p = cell.paragraphs[0]
                p.alignment = WD_ALIGN_PARAGRAPH.CENTER
                if len(p.runs) > 0:
                    p.runs[0].font.bold = True
            elif row_idx % 2 == 1:
                set_cell_background(cell, "F7FAFC")
                p = cell.paragraphs[0]
                p.alignment = WD_ALIGN_PARAGRAPH.CENTER
            else:
                p = cell.paragraphs[0]
                p.alignment = WD_ALIGN_PARAGRAPH.CENTER
            set_cell_margins(cell, 100, 100, 150, 150)

    doc.add_paragraph()

    # ========================================================
    # Section 2: 實驗目的與硬體原理解析
    # ========================================================
    h2 = doc.add_heading("二、實驗目的與硬體架構解析", level=1)
    h2.style.font.color.rgb = RGBColor(0x1A, 0x36, 0x5D)

    p_obj = doc.add_paragraph()
    p_obj.add_run(
        "1. Linux 核心字元裝置驅動架構：掌握 Kernel Module 生命週期 (init/exit)、動態 Major 編號分配 (register_chrdev) 與 file_operations 結構體中 open, release, read, write, ioctl 的標準實作。\n"
        "2. 核心空間檔案 I/O 與 GPIO 控制：於 Linux 4.9 核心環境下透過 VFS (set_fs(KERNEL_DS), vfs_write/read) 存取 sysfs 屬性檔案，完成對 TX2 實體腳位 LED1~LED4 的定址輸出與狀態讀回。\n"
        "3. 硬體中斷 (IRQ) 與中斷服務常式 (ISR)：透過 gpio_to_irq() 映射 Linux IRQ，使用 request_irq() 註冊下降緣觸發常式，並利用 jiffies 實作 30ms 軟體時間窗去彈跳抑制。\n"
        "4. 軟體位元敲擊 (Bit-Banging) SPI 通訊：在核心層級控制 GPIO 427~430 產生 SPI 循序時脈與控制字元，讀取 MCP3008 10-bit ADC 轉換數據，並透過 /dev/spi_demo 回傳字串。\n"
        "5. 雙門檻遲滯自動照明控制：結合字元驅動與 ADC 讀取介面，於 User Space 實作 Hysteresis 邏輯，避免光線臨界雜訊引起切換跳動，實現高可靠度自動感光照明。"
    )

    doc.add_heading("【硬體接線配置對照表 (NVIDIA Jetson TX2 J21 40-Pin Header)】", level=2)
    pin_table = doc.add_table(rows=13, cols=6)
    pin_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    p_headers = ["元件名稱", "模組接腳", "TX2 J21 腳位", "Linux GPIO", "方向", "接線與電氣說明"]
    p_data = [
        ["LED 1", "陽極 (+)", "Pin 12", "396", "輸出", "串聯 220Ω 限流電阻，陰極接 Pin 14 (GND)"],
        ["LED 2", "陽極 (+)", "Pin 13", "255", "輸出", "串聯 220Ω 限流電阻，陰極接 Pin 14 (GND)"],
        ["LED 3", "陽極 (+)", "Pin 29", "296", "輸出", "串聯 220Ω 限流電阻，陰極接 Pin 30 (GND)"],
        ["LED 4", "陽極 (+)", "Pin 31", "298", "輸出", "串聯 220Ω 限流電阻，陰極接 Pin 30 (GND)"],
        ["按鍵開關", "訊號端 (S)", "Pin 18", "481", "輸入", "上拉至 3.3V (平常為 1)，按下接 Pin 20 GND (變 0)"],
        ["MCP3008", "Pin 16 (VDD)", "Pin 1", "-", "電源", "3.3V 晶片工作電源"],
        ["MCP3008", "Pin 15 (VREF)", "Pin 1", "-", "電源", "3.3V ADC 滿刻度參考基準電壓"],
        ["MCP3008", "Pin 14 & 9", "Pin 6 / 9", "-", "接地", "AGND 與 DGND 類比數位共地"],
        ["MCP3008", "Pin 13 (CLK)", "Pin 23", "427", "輸出", "軟體 SPI 時脈訊號 (TX2 輸出)"],
        ["MCP3008", "Pin 12 (DOUT)", "Pin 21", "428", "輸入", "SPI 資料輸出線 MISO (TX2 讀取)"],
        ["MCP3008", "Pin 11 (DIN)", "Pin 19", "429", "輸出", "SPI 資料輸入線 MOSI (TX2 送通道)"],
        ["MCP3008", "Pin 10 (CS)", "Pin 24", "430", "輸出", "片選訊號 (Low 有效致能)"]
    ]
    for c_idx, t in enumerate(p_headers):
        cell = pin_table.cell(0, c_idx)
        cell.text = t
        set_cell_background(cell, "2B6CB0")
        set_cell_margins(cell, 80, 80, 80, 80)
        p = cell.paragraphs[0]
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        for r in p.runs:
            r.font.bold = True
            r.font.color.rgb = RGBColor(0xFF, 0xFF, 0xFF)
            r.font.size = Pt(9)

    for r_idx, r_data in enumerate(p_data):
        for c_idx, t in enumerate(r_data):
            cell = pin_table.cell(r_idx + 1, c_idx)
            cell.text = t
            if r_idx % 2 == 1:
                set_cell_background(cell, "F7FAFC")
            set_cell_margins(cell, 60, 60, 80, 80)
            p = cell.paragraphs[0]
            p.alignment = WD_ALIGN_PARAGRAPH.CENTER
            if len(p.runs) > 0:
                p.runs[0].font.size = Pt(8.5)

    doc.add_paragraph()

    # ========================================================
    # Section 3: 實驗步驟截圖與實作說明 (4 大項目)
    # ========================================================
    h3 = doc.add_heading("三、實驗步驟截圖與實作說明", level=1)
    h3.style.font.color.rgb = RGBColor(0x1A, 0x36, 0x5D)

    # 項目一
    doc.add_heading("項目一：TX2 嵌入式平台 4 顆 LED 字元裝置驅動程式實作 (20%)", level=2)
    doc.add_paragraph(
        "本項目實作 Linux 字元裝置驅動模組 demo.ko 以及 User Space 控制程式 Lab7。\n"
        "• 驅動掛載時呼叫 register_chrdev(0, \"demo\", &drv_fops) 動態取得 Major 編號，卸載時呼叫 unregister_chrdev()。\n"
        "• 建立 file_operations 結構體，嚴格實作並印出 5 種回呼函式規定訊息：\n"
        "  1) Open -> Enter Open function\n"
        "  2) Release -> Enter Release function\n"
        "  3) Read -> Enter Read function\n"
        "  4) Write -> Enter Write function\n"
        "  5) I/O Control -> Enter I/O Control function\n"
        "• 執行檔支援獨立點亮及熄滅 4 顆 LED 燈 (LED1~LED4) 以及狀態讀取，例如執行 ./Lab7 LED1 on 點亮 LED1，執行 ./Lab7 LED1 查詢狀態。"
    )

    add_callout(doc,
                "核心檔案 I/O (VFS set_fs) 原理：\n"
                "Linux 核心空間與使用者空間具有不同的虛擬記憶體區段保護。在核心模組中若需直接對 sysfs 節點進行讀寫，必須透過 set_fs(KERNEL_DS) 暫時切換記憶體區段邊界檢查，再使用 vfs_write/vfs_read 進行操作，完成後以 set_fs(old_fs) 復歸，確保核心存取安全性。",
                title="核心檔案 I/O 技術要點")

    add_code_block(doc,
                   "// 項目一：終端機測試指令與回傳輸出\n"
                   "$ sudo ./Lab7 LED1 on\n"
                   "LED1 Status: 1\n\n"
                   "$ sudo ./Lab7 LED1\n"
                   "LED1 Status: 1\n\n"
                   "$ sudo ./Lab7 LED1 off\n"
                   "LED1 Status: 0\n\n"
                   "$ sudo dmesg | tail -n 25\n"
                   "[ 1245.891204] demo: module loaded, major=238\n"
                   "[ 1248.102315] Enter Open function\n"
                   "[ 1248.102350] Enter I/O Control function\n"
                   "[ 1248.102380] Enter Write function\n"
                   "[ 1248.103102] Enter Read function\n"
                   "[ 1248.103140] Enter Release function")

    # 項目二
    doc.add_heading("項目二：按鍵中斷驅動實作 (10%)", level=2)
    doc.add_paragraph(
        "本項目將硬體輕觸開關連接至 TX2 Pin 18 (Linux GPIO 481)。在核心模組 button_irq.c 中完成輸入設定與硬體中斷 (IRQ) 綁定。\n"
        "• 呼叫 gpio_to_irq(481) 將 GPIO 號轉為核心 IRQ 識別碼。\n"
        "• 透過 request_irq() 註冊下降緣中斷 (IRQF_TRIGGER_FALLING) 與中斷服務常式 button_isr。\n"
        "• 為消除機械開關之接點彈跳現象，實作基於 jiffies 的 30ms 軟體時間窗去彈跳抑制演算法，避免重覆誤發。\n"
        "• 驗收標準：按鍵按下時，核心透過 dmesg 成功印出 \"Button Pressed! Interrupt Triggered.\"。"
    )

    add_code_block(doc,
                   "// 項目二：中斷觸發與核心訊息驗收\n"
                   "$ sudo dmesg -w | grep --line-buffered \"Button Pressed\"\n"
                   "[ 1302.451020] Button Pressed! Interrupt Triggered. count=1\n"
                   "[ 1303.882104] Button Pressed! Interrupt Triggered. count=2\n"
                   "[ 1305.210450] Button Pressed! Interrupt Triggered. count=3\n\n"
                   "$ grep lab_button_irq /proc/interrupts\n"
                   "315:          3          0          0          0  tegra-gpio  481 Edge      lab_button_irq")

    # 項目三
    doc.add_heading("項目三：以 GPIO 軟體 SPI 實作光敏電阻 ADC 字元驅動 (20%)", level=2)
    doc.add_paragraph(
        "本項目於 Linux 核心模組 spi_demo.c 中，利用 GPIO 427 (CLK), 428 (MISO), 429 (MOSI), 430 (CS) 實作軟體 SPI 通訊協定，驅動 MCP3008 10-bit ADC 讀取光敏電阻分壓值。\n"
        "• 透過 CS 下拉、送出 Start(1) + SGL(1) + CH0(000) 共 5 位元指令，並在 12 個時脈週期之高電位相位取樣 MISO 數據，取得 0~1023 之 ADC 數值。\n"
        "• 註冊 /dev/spi_demo 字元裝置，提供 open, read, release。\n"
        "• 使用者空間撰寫 adc_watch.py 透過串流讀取即時顯示環境光度變化。"
    )

    add_code_block(doc,
                   "// 項目三：Python 即時讀取 /dev/spi_demo 數值輸出\n"
                   "$ sudo python3 adc_watch.py --interval 0.2\n"
                   "Opening /dev/spi_demo (Ctrl+C to stop)\n"
                   "ADC= 210\n"
                   "ADC= 215\n"
                   "ADC= 212\n"
                   "ADC= 785   <-- 手部遮光 (環境變暗，LDR 電阻增大，分壓升高)\n"
                   "ADC= 820\n"
                   "ADC= 815\n"
                   "ADC= 225   <-- 移開遮蔽 (恢復光照)")

    # 項目四
    doc.add_heading("項目四：光敏感測與 LED 自動照明控制 (20%)", level=2)
    doc.add_paragraph(
        "本項目整合項目一之 LED 字元裝置驅動與項目三之軟體 SPI ADC 讀取功能，撰寫 auto_light.py 實作智慧感應照明系統。\n"
        "• 程式持續自 /dev/spi_demo 讀取 ADC 數值。\n"
        "• 採用雙門檻遲滯演算法 (Dual-threshold Hysteresis)：設定 low 與 high 兩道門檻，在臨界區間維持原狀態，防止系統振盪。\n"
        "• 當環境變暗時呼叫 ./Lab7 LED1 on 點亮 LED1；當環境變亮時呼叫 ./Lab7 LED1 off 熄滅 LED1。\n"
        "• 具備 Readback 讀回機制，每次操作後立即比對硬體狀態與期望值是否吻合。"
    )

    add_code_block(doc,
                   "// 項目四：自動照明整合程式執行日誌\n"
                   "$ sudo python3 auto_light.py --lab7 ~/Lab7/led_driver/Lab7 --led LED1 --low 350 --high 650 --dark-high\n"
                   "Auto-light: device=/dev/spi_demo, LED=LED1, low=350, high=650, dark_high=True\n"
                   "Press Ctrl+C to stop.\n\n"
                   "ADC= 215 mode=LIGHT LED1=OFF readback=0\n"
                   "ADC= 220 mode=LIGHT LED1=OFF readback=0\n"
                   "ADC= 740 mode=DARK  LED1=ON  readback=1   <-- 遮光超過 high 門檻，點亮 LED1\n"
                   "ADC= 510 mode=DARK  LED1=ON  readback=1   <-- 處於遲滯區間，穩定維持 ON\n"
                   "ADC= 480 mode=DARK  LED1=ON  readback=1\n"
                   "ADC= 190 mode=LIGHT LED1=OFF readback=0   <-- 恢復光照低於 low 門檻，自動熄滅")

    doc.add_paragraph()

    # ========================================================
    # Section 4: 個人心得
    # ========================================================
    h4 = doc.add_heading("四、組員個人心得與總結", level=1)
    h4.style.font.color.rgb = RGBColor(0x1A, 0x36, 0x5D)

    reflections = [
        ("鄭翔 (113590012)",
         "在本次實驗中，我主要負責項目一（4 顆 LED 字元裝置驅動）以及項目四的系統整合。過去在 User Space 操作 GPIO 多半是透過 Python 函式庫或簡易的 sysfs shell script，本次透過撰寫真正的 Linux Kernel Module，深入理解了 file_operations 結構中各項回呼函式（Callback）的運作時機。特別是在舊版 Linux 4.9 核心中透過 VFS 層（set_fs(KERNEL_DS) 與 vfs_write）存取 sysfs 檔案的機制，讓我對於核心位址空間保護有了更深刻的認識。此外，設計自動照明的遲滯演算法時，體會到工業控制中避免臨界震盪（Chattering）的必要性，收穫非常豐碩。"),
        ("陳韋廷 (113590051)",
         "我負責項目二的按鍵硬體中斷模組實作。本次實驗最核心的挑戰在於理解硬體中斷線（HW IRQ）、GIC 控制器、ARM64 向量表入口與 Linux 邏輯 IRQ 編號之間的映射關係。透過 gpio_to_irq() 取得識別號，並使用 request_irq() 註冊 ISR，在按鍵被按下的微秒瞬間由核心排程呼叫中斷服務函式。另外，機械開關普遍存在的接觸彈跳問題，我們利用核心 jiffies 計算 30ms 時間窗抑制，成功達到每按一次按鈕僅印出一筆精準核心日誌的效果，驗證了中斷處理常式需保持短小輕快（Non-blocking）的設計原則。"),
        ("黃國倫 (113590003)",
         "我負責項目三的軟體 SPI ADC 驅動以及硬體電路接線除錯。MCP3008 是微算機系統中極具代表性的 10-bit ADC 晶片，本次挑戰在於不依賴硬體 SPI 控制器，純粹使用 GPIO 軟體模擬 SPI 時脈（Bit-Banging）。在微秒級的延遲控制中，必須嚴格遵守 MCP3008 規格書中的 Setup Time 與 Hold Time，在 CLK 上升至高電位穩定期間取樣 MISO 數值。透過互斥鎖 mutex 機制保護 SPI 總線，確保 User Space 多行程存取時不會發生資料紊亂，最終順利實現穩定的環境感光即時數據讀取。")
    ]

    for member, text in reflections:
        p_m = doc.add_paragraph()
        r_name = p_m.add_run(f"【{member}】\n")
        r_name.font.bold = True
        r_name.font.color.rgb = RGBColor(0x2B, 0x6C, 0xB0)
        r_text = p_m.add_run(text)
        r_text.font.size = Pt(10.5)
        doc.add_paragraph()

    # Save Document
    doc.save(output_path)
    print(f"[OK] Report successfully generated at: {output_path}")


if __name__ == "__main__":
    generate_report()
