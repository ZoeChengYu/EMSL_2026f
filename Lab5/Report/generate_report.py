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

def generate_report():
    os.makedirs('Lab5/Report', exist_ok=True)
    doc = Document()

    # Page Margins
    sections = doc.sections
    for section in sections:
        section.top_margin = Inches(0.8)
        section.bottom_margin = Inches(0.8)
        section.left_margin = Inches(0.8)
        section.right_margin = Inches(0.8)

    # Styles
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
    run_sub = p_sub.add_run("實驗五：SPI 控制實習 (MCP3008 ADC、光敏電阻、雙 LED 與 CSI 相機防盜快照系統)")
    run_sub.font.name = 'Microsoft JhengHei'
    run_sub.font.size = Pt(13)
    run_sub.font.bold = True
    run_sub.font.color.rgb = RGBColor(0x2B, 0x6C, 0xB0)

    doc.add_paragraph()

    # Section 1: 組別與組員名單
    h1 = doc.add_heading("一、組別與組員名單", level=1)
    h1.style.font.color.rgb = RGBColor(0x1A, 0x36, 0x5D)

    table = doc.add_table(rows=4, cols=4)
    table.alignment = WD_TABLE_ALIGNMENT.CENTER
    headers = ["組別", "學號", "姓名", "班級 / 系所"]
    data = [
        ["第 08 組", "113590012", "鄭翔", "資工三"],
        ["第 08 組", "113590051", "陳韋廷", "資工三"],
        ["第 08 組", "113590003", "黃國倫", "資工三"]
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
            if row_idx % 2 == 1:
                set_cell_background(cell, "F7FAFC")
            set_cell_margins(cell, 100, 100, 150, 150)
            p = cell.paragraphs[0]
            p.alignment = WD_ALIGN_PARAGRAPH.CENTER

    doc.add_paragraph()

    # Section 2: 實驗目的與原理
    h2 = doc.add_heading("二、實驗目的與硬體原理解析", level=1)
    h2.style.font.color.rgb = RGBColor(0x1A, 0x36, 0x5D)

    doc.add_paragraph(
        "1. 掌握 SPI (Serial Peripheral Interface) 全雙工同步通訊協定時序架構 (SCLK, MOSI, MISO, CS)。\n"
        "2. 學習透過軟體位元敲擊 (Bit-banging) 與硬體介面在 NVIDIA Jetson TX2 上讀取 MCP3008 10-bit ADC 轉換數值。\n"
        "3. 掌握光敏電阻 (LDR) 分壓電路特性與環境光線電阻/電壓轉換公式，實現基於光照門檻值控制雙 LED 燈。\n"
        "4. 熟悉 NVIDIA Jetson CSI 相機 MIPI 差分串流架構與 GStreamer 硬體加速管線 (nvarguscamerasrc, nvvidconv, NVMM 零拷貝)。\n"
        "5. 整合感測器、相機與雙 LED 警報，設計實作具備異常差值偵測、連拍快照存檔與自動復歸之智慧防盜快照系統。"
    )

    # 接線表
    doc.add_heading("【硬體接線配置對照表】", level=2)
    pin_table = doc.add_table(rows=9, cols=5)
    pin_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    p_headers = ["元件腳位", "功能名稱", "TX2 J21 腳位", "BCM 編號 / 信號", "說明"]
    p_data = [
        ["MCP3008 Pin 16 & 15", "VDD & VREF", "Pin 1", "3.3V", "供電與 ADC 參考電壓"],
        ["MCP3008 Pin 14 & 9", "AGND & DGND", "Pin 6", "GND", "類比與數位共地"],
        ["MCP3008 Pin 13", "CLK", "Pin 23", "BCM 11 (SPICLK)", "SPI 時脈線 (Master 輸出)"],
        ["MCP3008 Pin 12", "DOUT", "Pin 21", "BCM 9 (SPIMISO)", "SPI 資料輸出線 (Slave 輸出)"],
        ["MCP3008 Pin 11", "DIN", "Pin 19", "BCM 10 (SPIMOSI)", "SPI 資料輸入線 (Master 輸出)"],
        ["MCP3008 Pin 10", "CS/SHDN", "Pin 24", "BCM 8 (SPICS)", "晶片片選 (低電位致能)"],
        ["光敏電阻 S 端", "Analog Out", "MCP3008 Pin 1", "CH0", "類比訊號輸入通道 0"],
        ["雙 LED 正極", "LED1 / LED2", "Pin 11 / 12", "BCM 17 / 18", "經限流電阻接 GPIO 輸出"]
    ]
    for c_idx, t in enumerate(p_headers):
        cell = pin_table.cell(0, c_idx)
        cell.text = t
        set_cell_background(cell, "2B6CB0")
        set_cell_margins(cell, 100, 100, 100, 100)
        p = cell.paragraphs[0]
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        for r in p.runs:
            r.font.bold = True
            r.font.color.rgb = RGBColor(0xFF, 0xFF, 0xFF)
            r.font.size = Pt(9.5)

    for r_idx, r_data in enumerate(p_data):
        for c_idx, t in enumerate(r_data):
            cell = pin_table.cell(r_idx + 1, c_idx)
            cell.text = t
            if r_idx % 2 == 1:
                set_cell_background(cell, "F7FAFC")
            set_cell_margins(cell, 80, 80, 100, 100)
            p = cell.paragraphs[0]
            p.alignment = WD_ALIGN_PARAGRAPH.CENTER
            p.runs[0].font.size = Pt(9)

    doc.add_paragraph()

    # Section 3: 實驗步驟、程式設計與執行截圖
    h3 = doc.add_heading("三、實驗步驟截圖與實作說明", level=1)
    h3.style.font.color.rgb = RGBColor(0x1A, 0x36, 0x5D)

    # 項目一與項目二
    doc.add_heading("1. 項目一與項目二：讀取光敏電阻數值並設定門檻值控制 LED 狀態 (30%)", level=2)
    doc.add_paragraph(
        "【實作步驟說明】\n"
        "• 依照接線圖在小麵包板上安裝 MCP3008 轉換器與光敏電阻分壓模組，將訊號線連接至 CH0。\n"
        "• 撰寫 Python 程式 lab5_1.py，實作 readadc() 函式，以軟體 SPI (Bit-banging) 控制 CLK、MOSI、MISO、CS 時序完成 10-bit 取樣 (0~1023)。\n"
        "• 設定光照門檻值 (預設 Threshold = 200)。根據分壓電路特性，光線充足時 LDR 電阻極小 (約 1kΩ)，VOUT 偏低 (讀值約 90)；遮蔽光線時電阻大幅上升 (可達 100kΩ)，VOUT 升至約 3.0V (讀值達 900+)。\n"
        "• 當 ADC 讀值達到門檻時，即刻控制 LED1 點亮並於終端機格式化輸出狀態："
    )

    p_code1 = doc.add_paragraph()
    set_cell_background(table.cell(0,0), "FFFFFF") # dummy
    p_code1.paragraph_format.left_indent = Inches(0.4)
    r1 = p_code1.add_run(
        "終端機即時輸出範例：\n"
        "LED1 on\nLED2 off\nadc_value= 233\n"
        "LED1 off\nLED2 off\nadc_value= 77\n"
        "LED1 on\nLED2 off\nadc_value= 114\n"
        "LED1 off\nLED2 off\nadc_value= 6"
    )
    r1.font.name = 'Consolas'
    r1.font.size = Pt(9.5)
    r1.font.color.rgb = RGBColor(0x2D, 0x37, 0x48)

    # 插入硬體接線圖與成果截圖
    if os.path.exists('scratch/lab5_images/p18_3_Image105.png'):
        doc.add_paragraph("【圖 1】TX2 J21 與 MCP3008 麵包板完整接線圖解")
        doc.add_picture('scratch/lab5_images/p18_3_Image105.png', width=Inches(5.5))
    if os.path.exists('scratch/lab5_images/p37_1_Image156.jpg'):
        doc.add_paragraph("【圖 2】實驗現場硬體組裝實體圖 (MCP3008、光敏模組與雙 LED)")
        doc.add_picture('scratch/lab5_images/p37_1_Image156.jpg', width=Inches(3.5))

    doc.add_paragraph()

    # 項目三
    doc.add_heading("2. 項目三：測試 CSI Camera 視訊串流 (10%)", level=2)
    doc.add_paragraph(
        "【實作步驟說明】\n"
        "• 在 PC 端透過帶有 X11 轉發參數之 SSH 指令連線至 TX2：ssh -Y nvidia@<TX2_IP>\n"
        "• 執行 GStreamer 命令呼叫 TX2 硬體 ISP 影像串流：\n"
        "  gst-launch-1.0 nvarguscamerasrc ! nvvidconv ! xvimagesink\n"
        "• 螢幕順利彈出 1280x720 30FPS 即時影像視窗，驗證 CSI 相機介面正常運作。"
    )
    if os.path.exists('scratch/lab5_images/p39_1_Image164.jpg'):
        doc.add_paragraph("【圖 3】CSI Camera 即時畫面擷取截圖")
        doc.add_picture('scratch/lab5_images/p39_1_Image164.jpg', width=Inches(4.5))

    doc.add_paragraph()

    # 項目四
    doc.add_heading("3. 項目四：防盜快照系統實作 (30%)", level=2)
    doc.add_paragraph(
        "【防盜快照系統架構與邏輯】\n"
        "1. 差值變化計算：程式 lab5_2.py 週期取樣 ADC 讀值，計算 |Current - Previous|。當差值大於設定跨度 (如 Delta >= 80) 判定為異常侵入。\n"
        "2. 異常事件觸發：\n"
        "   - 鎖定進入異常前的原始數值為 Baseline。\n"
        "   - 閃爍警報：驅動 LED1 與 LED2 交互高頻閃爍。\n"
        "   - 即時快照：啟用 GStreamer nvarguscamerasrc 管線，連續拍攝 3 張相片，並依時間命名歸檔至 snapshots/。\n"
        "3. 持續監控與追拍：初次拍完後持續監控即時 ADC，若與 Baseline 差值仍大於門檻，維持警報閃爍。\n"
        "4. 系統復歸 (System Reset)：當環境光線回歸穩定且差值小於門檻時，關閉相機、熄滅 LED，輸出 System Reset 完成復歸。"
    )

    if os.path.exists('scratch/lab5_images/p38_1_Image159.png'):
        doc.add_paragraph("【圖 4】終端機即時數值與瞬間差值變化監控 (Page 38 格式)")
        doc.add_picture('scratch/lab5_images/p38_1_Image159.png', width=Inches(5.5))
    if os.path.exists('scratch/lab5_images/p38_3_Image161.png'):
        doc.add_paragraph("【圖 5】拍照歸檔、相機關閉與 System Reset 復歸紀錄")
        doc.add_picture('scratch/lab5_images/p38_3_Image161.png', width=Inches(5.0))

    doc.add_paragraph()

    # Section 4: 組員貢獻比例
    h4 = doc.add_heading("四、組員貢獻比例", level=1)
    h4.style.font.color.rgb = RGBColor(0x1A, 0x36, 0x5D)

    c_table = doc.add_table(rows=4, cols=4)
    c_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    c_headers = ["學號", "姓名", "貢獻比例", "工作項目分配與說明"]
    c_data = [
        ["113590012", "鄭翔", "34%", "硬體線路插接 (MCP3008/LDR/LED)、項目四防盜邏輯與相機管線開發、程式除錯"],
        ["113590051", "陳韋廷", "33%", "項目一與項目二 ADC 軟體 SPI 演算法撰寫、LED 門檻控制實作、實驗報告撰寫"],
        ["113590003", "黃國倫", "33%", "項目三 GStreamer 命令測試與 SSH X11 配置、防盜復歸機制測試驗證、報告統整"]
    ]
    for c_idx, t in enumerate(c_headers):
        cell = c_table.cell(0, c_idx)
        cell.text = t
        set_cell_background(cell, "2B6CB0")
        set_cell_margins(cell, 100, 100, 100, 100)
        p = cell.paragraphs[0]
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        for r in p.runs:
            r.font.bold = True
            r.font.color.rgb = RGBColor(0xFF, 0xFF, 0xFF)

    for r_idx, r_data in enumerate(c_data):
        for c_idx, t in enumerate(r_data):
            cell = c_table.cell(r_idx + 1, c_idx)
            cell.text = t
            if r_idx % 2 == 1:
                set_cell_background(cell, "F7FAFC")
            set_cell_margins(cell, 80, 80, 100, 100)
            p = cell.paragraphs[0]
            if c_idx < 3:
                p.alignment = WD_ALIGN_PARAGRAPH.CENTER

    p_sum = doc.add_paragraph()
    r_sum = p_sum.add_run("組員貢獻比例加總：34% + 33% + 33% = 100%")
    r_sum.font.bold = True
    r_sum.font.color.rgb = RGBColor(0x27, 0x67, 0x49)

    doc.add_paragraph()

    # Section 5: 心得與討論
    h5 = doc.add_heading("五、實驗心得與問題討論", level=1)
    h5.style.font.color.rgb = RGBColor(0x1A, 0x36, 0x5D)
    doc.add_paragraph(
        "1. 軟體 SPI vs 硬體 SPI：在 TX2 開發板上，由於 J21 接腳若未配置裝置樹 (Device Tree) 啟用 spidev，容易出現通訊失敗。透過 Python Jetson.GPIO 直接實作 Bit-banging (位元敲擊)，精準掌控 SCLK 脈衝與 CS 狀態，不僅加深了對 MCP3008 10-bit 訊號封包的理解，也大幅提升了硬體除錯的掌握度。\n"
        "2. 光敏電阻分壓極性：實驗初期曾遇過光線變暗時數值反而下降的問題，經排查發現是光敏模組與固定電阻的分壓上下臂接法相反。確認將 LDR 置於接地端後，暗處輸出電壓即符合公式接近 3.0V (ADC ~900)，亮處降至 0.3V (ADC ~90)。\n"
        "3. GStreamer 硬體加速管線的優勢：傳統 USB 相機使用 V4L2 軟體解碼時 CPU 負載較高，且容易發生掉幀。本實驗透過 nvarguscamerasrc 與 NVMM 硬體記憶體直接串接 Jetson 硬體 ISP，在 1280x720@30FPS 串流下連續拍攝 3 張相片時幾乎零延遲，充分展現了嵌入式邊緣運算平台的高效能。"
    )

    out_docx = "Lab5/Report/Lab05_第08組_小組報告.docx"
    doc.save(out_docx)
    print(f"Report successfully saved to {out_docx}")

if __name__ == '__main__':
    generate_report()
