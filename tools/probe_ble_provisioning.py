"""Read-only BLE discovery/GATT/protocol check. Requires: pip install bleak.

Does not send Wi-Fi credentials, read the PoP, or modify NVS.
Usage: python tools/probe_ble_provisioning.py [HAVIT-XXXX]
"""
import asyncio
import json
import sys
from bleak import BleakClient, BleakScanner

SERVICE_UUID = "1775244d-6b43-439b-877c-060f2d9bed07"
EXPECTED = {"prov-session", "prov-config", "prov-scan", "prov-ctrl", "proto-ver", "havit-info"}


async def main():
    target = sys.argv[1] if len(sys.argv) > 1 else None
    found = await BleakScanner.discover(timeout=8, return_adv=True)
    matches = [(d, a) for d, a in found.values()
               if (a.local_name or d.name or "").startswith("HAVIT-")
               and (target is None or target == (a.local_name or d.name))]
    if len(matches) != 1:
        raise RuntimeError(f"Expected one HAVIT, found {len(matches)}. Specify its displayed name.")
    device, advert = matches[0]
    print("Found:", advert.local_name or device.name)
    assert SERVICE_UUID in [uuid.lower() for uuid in advert.service_uuids], advert.service_uuids
    endpoints = {}
    async with BleakClient(device, timeout=20) as client:
        service = client.services.get_service(SERVICE_UUID)
        assert service, "Provisioning GATT service missing"
        for char in service.characteristics:
            for descriptor in char.descriptors:
                if descriptor.uuid.lower().startswith("00002901-"):
                    name = bytes(await client.read_gatt_descriptor(descriptor.handle)).decode().rstrip("\x00")
                    endpoints[name] = char
        assert EXPECTED <= endpoints.keys(), f"Missing endpoints: {EXPECTED - endpoints.keys()}"
        print("Endpoints:", ", ".join(sorted(endpoints)))
        await client.write_gatt_char(endpoints["proto-ver"], b"ESP", response=True)
        raw = await client.read_gatt_char(endpoints["proto-ver"])
        version = json.loads(bytes(raw).rstrip(b"\0"))
        assert version["prov"]["sec_ver"] == 1, version
        assert "no_sec" not in version["prov"]["cap"], version
        assert "no_pop" not in version["prov"]["cap"], version
        print("Protocol:", json.dumps(version))
        print("PASS: BLE advertisement, GATT endpoints and Security 1 + PoP capability")


if __name__ == "__main__":
    asyncio.run(main())
