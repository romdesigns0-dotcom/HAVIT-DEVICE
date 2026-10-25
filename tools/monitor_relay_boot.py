"""Read boot/relay errors without toggling reset or emitting credentials."""
import re
import time
import serial

p = serial.Serial(port=None, baudrate=115200, timeout=.1)
p.dtr = False
p.rts = False
p.port = 'COM5'
p.open()
deadline = time.monotonic() + 18
while time.monotonic() < deadline:
    line = p.readline().decode('utf8', 'replace').strip()
    if re.search(r'Guru|Backtrace|assert|overflow|BLE relay|nimble|NimBLE|abort|ELF|havit_net|Boot|rst:|wifi_init|wifi:.*[Mm]em|wifi:.*alloc|heap|E \(', line):
        if not re.search(r'password|Bearer|device_token|claim_secret|PoP|PIN', line, re.I):
            print(line, flush=True)
p.close()
