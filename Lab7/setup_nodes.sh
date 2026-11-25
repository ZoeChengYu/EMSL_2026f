#!/bin/bash
# Lab7 setup_nodes.sh - 載入驅動模組並建立字元裝置節點 (/dev/demo, /dev/spi_demo)

if [ "$EUID" -ne 0 ]; then
    echo "請使用 sudo 執行此腳本！"
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=== 1. 載入 LED 驅動模組 (demo.ko) ==="
if lsmod | grep -q "^demo "; then
    echo "demo.ko 已在核心中，跳過載入"
else
    insmod "${SCRIPT_DIR}/led_driver/demo.ko"
    echo "demo.ko 載入成功"
fi

echo "=== 2. 建立 /dev/demo 節點 ==="
LED_MAJOR=$(awk '$2=="demo" {print $1}' /proc/devices)
if [ -n "$LED_MAJOR" ]; then
    if [ ! -e /dev/demo ]; then
        mknod /dev/demo c "$LED_MAJOR" 0
        chmod 666 /dev/demo
        echo "/dev/demo 建立成功 (Major: $LED_MAJOR, Minor: 0)"
    else
        echo "/dev/demo 節點已存在"
    fi
else
    echo "警告：在 /proc/devices 中找不到 demo 驅動！"
fi

echo "=== 3. 載入按鍵中斷驅動模組 (button_irq.ko) ==="
# 確保 GPIO 481 未被 sysfs export 佔用
if [ -d /sys/class/gpio/gpio481 ]; then
    echo "釋放 sysfs 中的 gpio481..."
    echo 481 > /sys/class/gpio/unexport 2>/dev/null || true
fi

if lsmod | grep -q "^button_irq "; then
    echo "button_irq.ko 已在核心中，跳過載入"
else
    insmod "${SCRIPT_DIR}/button_driver/button_irq.ko" gpio=481 active_low=1 debounce_ms=30
    echo "button_irq.ko 載入成功 (GPIO=481, 下降緣觸發, 30ms 去彈跳)"
fi

echo "=== 4. 載入軟體 SPI ADC 驅動模組 (spi_demo.ko) ==="
if lsmod | grep -q "^spi_demo "; then
    echo "spi_demo.ko 已在核心中，跳過載入"
else
    insmod "${SCRIPT_DIR}/adc_driver/spi_demo.ko" channel=0
    echo "spi_demo.ko 載入成功 (Channel 0)"
fi

echo "=== 5. 建立 /dev/spi_demo 節點 ==="
ADC_MAJOR=$(awk '$2=="spi_demo" {print $1}' /proc/devices)
if [ -n "$ADC_MAJOR" ]; then
    if [ ! -e /dev/spi_demo ]; then
        mknod /dev/spi_demo c "$ADC_MAJOR" 0
        chmod 666 /dev/spi_demo
        echo "/dev/spi_demo 建立成功 (Major: $ADC_MAJOR, Minor: 0)"
    else
        echo "/dev/spi_demo 節點已存在"
    fi
else
    echo "警告：在 /proc/devices 中找不到 spi_demo 驅動！"
fi

echo "=========================================="
echo " 裝置節點與驅動載入狀態："
echo "=========================================="
ls -l /dev/demo /dev/spi_demo 2>/dev/null || true
echo "中斷註冊狀態："
grep lab_button_irq /proc/interrupts 2>/dev/null || true
echo "完成！"
