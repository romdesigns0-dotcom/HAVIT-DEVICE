"""Physical encrypted relay diagnostic for an explicitly selected test unit.

Uses the local backend service implementation with operator Firestore access;
does not substitute for the iPhone/Firebase-token acceptance test.
No credentials, task contents or envelopes are printed.
"""
import asyncio
import argparse
import json
import os
from pathlib import Path
import struct
import subprocess
import sys

from bleak import BleakClient, BleakScanner

IDF = Path(os.environ['IDF_PATH'])
os.environ['IDF_PATH'] = str(IDF)
sys.path.insert(0, str(IDF / 'tools/esp_prov'))
from security.security1 import Security1


async def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--device-name', required=True, help='Exact advertised HAVIT-SYNC-… name')
    parser.add_argument('--device-id', required=True, help='Full identity read from havit-info')
    parser.add_argument('--binding-id', required=True, help='Current binding ID confirmed by its owner')
    parser.add_argument('--app-backend', required=True, type=Path, help='Path to the HAVIT app repository')
    args = parser.parse_args()
    device = await BleakScanner.find_device_by_name(args.device_name, timeout=15)
    if not device:
        raise RuntimeError('Expected BLE sync service not found')
    async with BleakClient(device, timeout=20) as client:
        endpoints = {}
        for service in client.services:
            for char in service.characteristics:
                for desc in char.descriptors:
                    if desc.uuid.lower().startswith('00002901-'):
                        name = bytes(await client.read_gatt_descriptor(desc.handle)).decode().rstrip('\0')
                        endpoints[name] = char
        assert {'prov-session', 'havit-relay'} <= endpoints.keys()
        async def exchange(path, raw):
            await client.write_gatt_char(endpoints[path], raw, response=True)
            return bytes(await client.read_gatt_char(endpoints[path]))
        security = Security1('', False)
        response = None
        while True:
            request = security.security_session(response)
            if request is None:
                break
            response = (await exchange('prov-session', request.encode('latin-1'))).decode('latin-1')
        async def send(packet):
            raw = security.decrypt_data(await exchange('havit-relay', security.encrypt_data(packet)))
            assert raw and raw[0] == 0, 'Relay operation rejected'
            return raw[1:]
        length, = struct.unpack('<I', await send(b'\x01'))
        assert 0 < length < 300000
        raw = bytearray()
        while len(raw) < length:
            chunk = await send(b'\x02' + struct.pack('<I', len(raw)))
            assert 0 < len(chunk) <= 160
            raw.extend(chunk)
        envelope = json.loads(raw)
        assert envelope['deviceId'] == args.device_id
        assert envelope['bindingId'] == args.binding_id
        backend = args.app_backend / 'backend'
        result = await asyncio.to_thread(subprocess.run,
            ['node', str(backend / 'scripts/probe_relay.cjs'), args.device_id, args.binding_id], input=bytes(raw), capture_output=True,
            cwd=backend / 'functions')
        assert result.returncode == 0, 'Backend rejected encrypted proof (details suppressed)'
        reply = json.loads(result.stdout)
        assert reply['nonce'] == envelope['nonce']
        await send(b'\x03' + struct.pack('<I', len(result.stdout)))
        for offset in range(0, len(result.stdout), 160):
            await send(b'\x04' + struct.pack('<I', offset) + result.stdout[offset:offset+160])
        await send(b'\x05')
        print('PASS: BLE Security 1, encrypted device proof, exact identity/binding, backend snapshot and persisted ESP ACK.')
        print('Selected device identity and binding matched.')


if __name__ == '__main__':
    asyncio.run(main())
