#!/usr/bin/env python3
"""
auto_light.py - Lab7 項目四：光敏感測與 LED 自動照明控制

整合項目一的 LED 字元裝置驅動與項目三的光敏電阻 ADC 讀取功能。
持續從 /dev/spi_demo 讀取 ADC 數值，依雙門檻遲滯判斷環境明暗，
透過 Lab7 程式自動控制 LED。

用法：
    sudo python3 auto_light.py \\
        --lab8 "$HOME/Lab7/led_driver/Lab7" \\
        --led LED1 --low 400 --high 600

    # 若暗時 ADC 值較高：
    sudo python3 auto_light.py \\
        --lab8 "$HOME/Lab7/led_driver/Lab7" \\
        --led LED1 --low 400 --high 600 --dark-high
"""

import argparse
import os
import re
import subprocess
import sys
import time


def read_sample(adc):
    """
    Read one complete ADC record from the device.
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


def is_dark(value, previous, low, high, dark_high=False):
    """
    Hysteresis-based darkness determination.

    When dark_high is False (default):
        ADC <= low  -> dark  (LED on)
        ADC >= high -> light (LED off)
        In between  -> keep previous state

    When dark_high is True (reversed wiring):
        ADC >= high -> dark  (LED on)
        ADC <= low  -> light (LED off)
        In between  -> keep previous state
    """
    if value <= low:
        return not dark_high
    if value >= high:
        return dark_high
    return previous


def run_led_command(program, led, state=None):
    """
    Call the Lab7 LED control program.

    Args:
        program: Path to the Lab7 executable.
        led:     LED name (LED1..LED4).
        state:   True for on, False for off, None for query only.

    Returns:
        The readback value (0 or 1) parsed from stdout.
    """
    command = [str(program), led]
    if state is not None:
        command.append("on" if state else "off")

    result = subprocess.run(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
        check=True
    )

    reply = result.stdout.strip()

    # Parse "LEDx Status: N" from the reply
    match = re.search(r'Status:\s*(\d)', reply)
    if match is None:
        raise RuntimeError(
            "Cannot parse LED status from: '{}'".format(reply))

    return int(match.group(1))


def control_led(program, led, dark):
    """Turn LED on (dark=True) or off (dark=False), return readback."""
    return run_led_command(program, led, state=dark)


def query_led(program, led):
    """Query current LED state without changing it, return readback."""
    return run_led_command(program, led, state=None)


def main():
    parser = argparse.ArgumentParser(
        description="Automatic lighting control using ADC + LED driver")
    parser.add_argument("--lab7", "--lab8", dest="program", required=False,
                        help="Path to the Lab7 LED control executable")
    parser.add_argument("--led", default="LED1",
                        help="LED to control (default: LED1)")
    parser.add_argument("--low", type=int, required=True,
                        help="Lower threshold for hysteresis")
    parser.add_argument("--high", type=int, required=True,
                        help="Upper threshold for hysteresis")
    parser.add_argument("--dark-high", action="store_true",
                        help="Set if dark produces higher ADC values")
    parser.add_argument("--device", default="/dev/spi_demo",
                        help="ADC device (default: /dev/spi_demo)")
    parser.add_argument("--interval", type=float, default=0.5,
                        help="Update interval in seconds (default: 0.5)")
    args = parser.parse_args()

    if not args.program:
        # Default fallback to led_driver/Lab7 in current directory
        script_dir = os.path.dirname(os.path.abspath(__file__))
        default_path = os.path.join(script_dir, "led_driver", "Lab7")
        if os.path.isfile(default_path):
            args.program = default_path
        else:
            print("Error: --lab7 or --lab8 argument required", file=sys.stderr)
            sys.exit(1)

    if args.low >= args.high:
        print("Error: --low must be less than --high", file=sys.stderr)
        sys.exit(1)

    program = os.path.expanduser(args.program)
    if not os.path.isfile(program):
        print("Error: LED program not found: {}".format(program),
              file=sys.stderr)
        sys.exit(1)

    led = args.led.upper()
    low = args.low
    high = args.high
    dark_high = args.dark_high

    print("Auto-light: device={}, LED={}, low={}, high={}, "
          "dark_high={}".format(args.device, led, low, high, dark_high))
    print("Press Ctrl+C to stop.\n")

    mode = False        # Initial state: assume LIGHT (LED off)
    first_sample = True

    try:
        with open(args.device, "rb", buffering=0) as adc:
            while True:
                value = read_sample(adc)
                new_dark = is_dark(value, mode, low, high, dark_high)

                if new_dark != mode or first_sample:
                    # State changed or first sample: send control command
                    readback = control_led(program, led, new_dark)
                else:
                    # Stable: query to verify
                    readback = query_led(program, led)

                # Verify readback matches expected state
                if readback != int(new_dark):
                    raise RuntimeError(
                        "LED readback mismatch: expected={}, got={}".format(
                            int(new_dark), readback))

                mode_str = "DARK" if new_dark else "LIGHT"
                led_str = "ON" if new_dark else "OFF"
                print("ADC={:4d} mode={:<5s} {}={:<3s} readback={}".format(
                    value, mode_str, led, led_str, readback),
                    flush=True)

                mode = new_dark
                first_sample = False
                time.sleep(args.interval)

    except KeyboardInterrupt:
        print("\nStopping auto-light...")
        # Try to turn off LED gracefully
        try:
            control_led(program, led, False)
            print("{} turned OFF.".format(led))
        except Exception as e:
            print("Warning: Could not turn off {}: {}".format(led, e),
                  file=sys.stderr)
    except Exception as e:
        print("Error: {}".format(e), file=sys.stderr)
        # Try to turn off LED
        try:
            control_led(program, led, False)
        except Exception:
            pass
        sys.exit(1)


if __name__ == "__main__":
    main()
