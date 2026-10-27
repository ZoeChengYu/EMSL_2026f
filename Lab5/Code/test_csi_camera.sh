#!/bin/bash
# ==============================================================================
# 微算機系統實習 - Lab 5 (SPI 控制實習)
# 項目三：測試 CSI Camera (10%)
# 
# 注意事項：
# 1. 在透過 SSH 連線時，請務必啟用 X11 Forwarding (即加上 -Y 參數)：
#    ssh -Y nvidia@<TX2_IP_ADDRESS>
# 2. 執行此腳本即可測試 TX2 上的 CSI 鏡頭即時串流畫面。
# 3. 測試成功後，請截取螢幕影像 (或附上同組可愛合照/錄影截圖) 放入實驗報告中。
# ==============================================================================

echo "========================================================"
echo "  測試 CSI Camera 即時視訊串流 (項目三)"
echo "  正在嘗試透過 GStreamer 開啟主流 nvarguscamerasrc..."
echo "========================================================"

# 主流新版指令 (L4T 32.x / JetPack 4.x+ 適用)
gst-launch-1.0 nvarguscamerasrc ! nvvidconv ! xvimagesink

# 若上述指令回報錯誤或無畫面，請嘗試舊版 nvcamerasrc 指令 (解除下方註解)：
# echo "嘗試舊版 nvcamerasrc 管線..."
# gst-launch-1.0 nvcamerasrc ! 'video/x-raw(memory:NVMM),width=640, height=480, framerate=30/1, format=NV12' ! nvvidconv flip-method=2 ! nvegltransform ! nveglglessink -e
