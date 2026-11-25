#!/bin/bash
# Lab7 build_all.sh - 一鍵編譯所有驅動模組與使用者空間程式

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=========================================="
echo " [Lab7] Building all modules and binaries "
echo "=========================================="

echo "-> Building led_driver (demo.ko, Lab7)..."
cd "${SCRIPT_DIR}/led_driver"
make clean
make

echo "-> Building button_driver (button_irq.ko)..."
cd "${SCRIPT_DIR}/button_driver"
make clean
make

echo "-> Building adc_driver (spi_demo.ko)..."
cd "${SCRIPT_DIR}/adc_driver"
make clean
make

echo "=========================================="
echo " [Lab7] All builds completed successfully! "
echo "=========================================="
ls -lh "${SCRIPT_DIR}/led_driver/demo.ko" "${SCRIPT_DIR}/led_driver/Lab7"
ls -lh "${SCRIPT_DIR}/button_driver/button_irq.ko"
ls -lh "${SCRIPT_DIR}/adc_driver/spi_demo.ko"
