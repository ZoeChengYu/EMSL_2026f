#!/bin/bash
# Lab7 cleanup.sh - 卸載驅動模組並清除裝置節點

if [ "$EUID" -ne 0 ]; then
    echo "請使用 sudo 執行此腳本！"
    exit 1
fi

echo "=== 停止相關使用者空間程式 (如有) ==="
pkill -f "auto_light.py" 2>/dev/null || true
pkill -f "adc_watch.py" 2>/dev/null || true

echo "=== 卸載核心模組 ==="
if lsmod | grep -q "^spi_demo "; then
    rmmod spi_demo
    echo "已卸載 spi_demo.ko"
fi

if lsmod | grep -q "^button_irq "; then
    rmmod button_irq
    echo "已卸載 button_irq.ko"
fi

if lsmod | grep -q "^demo "; then
    rmmod demo
    echo "已卸載 demo.ko"
fi

echo "=== 移除裝置節點 ==="
rm -f /dev/demo /dev/spi_demo

echo "=== 釋放可能殘留的 sysfs GPIO ==="
for g in 396 255 296 298 481 427 428 429 430; do
    if [ -d "/sys/class/gpio/gpio${g}" ]; then
        echo "$g" > /sys/class/gpio/unexport 2>/dev/null || true
    fi
done

echo "清理完成！"
