#!/usr/bin/env python3
"""
adc_watch.py - Lab7 項目三：ADC 數值即時監看程式

持續從 /dev/spi_demo 讀取光敏電阻 ADC 數值並顯示。
用於觀察照光與遮光時的數值變化。

用法：
    sudo python3 adc_watch.py --interval 0.2
"""

import argparse
import sys
import time


def read_sample(adc):
    """
    Read one complete ADC record from the device.
    The kernel driver returns a numeric value terminated by newline.
    Supports the kernel's short-read interface.
    """
    line = bytearray()
    while True:
        ch = adc.read(1)
        if not ch:
            raise EOFError("ADC EOF")
        if ch == b"\n":
            break
        line.extend(ch)
        if len(line) > 4:
            raise ValueError("long record")
    value = int(line.decode("ascii"))
    if not 0 <= value <= 1023:
        raise ValueError("ADC range: {}".format(value))
    return value


def main():
    parser = argparse.ArgumentParser(
        description="Watch ADC values from /dev/spi_demo")
    parser.add_argument("--device", default="/dev/spi_demo",
                        help="Device file path (default: /dev/spi_demo)")
    parser.add_argument("--interval", type=float, default=0.2,
                        help="Polling interval in seconds (default: 0.2)")
    args = parser.parse_args()

    print("Opening {}  (Ctrl+C to stop)".format(args.device))

    try:
        with open(args.device, "rb", buffering=0) as adc:
            while True:
                value = read_sample(adc)
                print("ADC={:4d}".format(value), flush=True)
                time.sleep(args.interval)
    except KeyboardInterrupt:
        print("\nStopped.")
    except Exception as e:
        print("Error: {}".format(e), file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
