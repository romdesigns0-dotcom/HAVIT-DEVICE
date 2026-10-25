"""Local development touch control; coordinates are native 360x360 pixels."""
import argparse
import re
import time

import serial


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("status")
    commands.add_parser("tasks")
    commands.add_parser("task-test-create")
    commands.add_parser("factory-reset", help="erase Wi-Fi, claim and task data; preserve factory identity")
    tap = commands.add_parser("tap")
    tap.add_argument("x", type=int)
    tap.add_argument("y", type=int)
    swipe = commands.add_parser("swipe")
    for field in ("x1", "y1", "x2", "y2", "duration_ms"):
        swipe.add_argument(field, type=int)
    args = parser.parse_args()
    if args.command == "tap":
        values = (args.x, args.y)
    elif args.command == "swipe":
        values = (args.x1, args.y1, args.x2, args.y2)
    else:
        values = ()
    if any(value < 0 or value >= 360 for value in values):
        parser.error("Coordinates must be 0..359")
    if args.command == "swipe" and not 50 <= args.duration_ms <= 5000:
        parser.error("Duration must be 50..5000 ms")
    command = "havit " + args.command
    if values:
        command += " " + " ".join(map(str, values))
    if args.command == "swipe":
        command += " " + str(args.duration_ms)
    device = serial.Serial(port=None, baudrate=115200, timeout=0.1)
    device.dtr = False
    device.rts = False
    device.port = args.port
    with device:
        device.write((command + "\n").encode("ascii"))
        device.flush()
        deadline = time.monotonic() + (args.duration_ms / 1000 if args.command == "swipe" else 0) + (25 if args.command == "factory-reset" else 3)
        accepted = False
        finished = args.command in ("status", "tasks", "task-test-create", "factory-reset")
        while time.monotonic() < deadline:
            line = device.readline().decode("utf-8", errors="replace").strip()
            if not line:
                continue
            # No credential, provisioning-code or authentication payload output.
            if re.search(r"Bearer|device_token|claim_secret|password|passphrase|\bPoP\b|\bPIN\b", line, re.I):
                continue
            if any(label in line for label in ("USB command", "USB status", "USB task", "USB gesture", "havit_eyes", "havit_login")):
                print(line)
            accepted |= "USB command ESP_OK" in line
            finished |= "USB gesture finished" in line
        if not accepted or not finished:
            raise SystemExit("USB command was not confirmed; inspect status before retrying input")


if __name__ == "__main__":
    main()
