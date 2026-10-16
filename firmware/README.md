# Havit OS firmware target

This is the ESP-IDF firmware target for the Waveshare
ESP32-S3-Touch-LCD-1.85 Rev 2.0 on COM5. The existing root CMake project stays
the LVGL Pro desktop simulator.

## Hardware profile

- ESP32-S3, 16 MB flash, octal PSRAM
- 360 × 360 ST77916 display over QSPI
- CST816 capacitive touch
- Display and touch reset lines go through the onboard TCA9554 I/O expander
- USB Serial/JTAG console is enabled for this board's USB connection

The board driver is the Espressif `esp32_display_panel` component with its
Waveshare 1.85-inch board profile. The shared application state machine is
compiled from `../src/havit_app.c`; this target supplies NVS persistence.
The three exported screens are recreated as native LVGL objects using their
XML layouts and embedded assets. The gradient rings, Google mark, divider,
arrows, pill, and Instrument Sans font are included in the app image. The
partition table reserves 3 MB for the app.

Current flow: Frame 1 continues to Frame 2; tapping the Google mark opens
account pairing. Google sign-in takes place on the HAVIT web page. The watch
receives its own revocable credential and synchronizes the user's name and
active task count. Frame 3 displays the linked name; Continue opens the home
placeholder.

## Wi-Fi and account pairing

Open https://havit--havit-c8c1d.us-central1.hosted.app/dispositivos and sign in.
For first-time Wi-Fi setup, use the page's Bluetooth button in Chrome/Edge
and configure a 2.4 GHz network. Bluetooth uses Improv and can stop after
Wi-Fi is configured; this does not mean the watch lost internet or its account.
A previously configured watch reconnects to its saved Wi-Fi without Bluetooth.

When the watch obtains an IP, it requests a pairing PIN even if its pairing
screen is already open. Enter the displayed PIN in step 02 of the page. The
PIN expires in ten minutes. The watch's Reintentar / pedir PIN button requests
another one and serializes requests so two pairing pollers cannot overlap.

The Wi-Fi control at the top of every main screen opens Conexiones, including
after login. Conexiones shows Wi-Fi connection, network name, IP, and availability of the
Bluetooth setup service. It can activate/stop Bluetooth, disconnect/reconnect
Wi-Fi, or forget saved Wi-Fi networks after on-device confirmation. Forgetting
Wi-Fi preserves the account credential. Wi-Fi manual shows fallback AP
credentials and its configuration portal. HTTP transport/server failures are
shown on the pairing screen instead of leaving it at generic Wi-Fi instructions.

TLS allocations use PSRAM (CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC); certificate
validation stays enabled. Internal-only TLS allocation can exhaust internal RAM
with BLE and LVGL active and fail with MBEDTLS_ERR_X509_ALLOC_FAILED (-0x2880).
CST816S reads are triggered by its interrupt to avoid polling an asleep touch
controller. All LVGL rendering stays on the LVGL task; worker/event tasks only
request updates. Saved onboarding and account credentials survive resets.

The most recent hardware boot capture is build/connection-boot.log. It records
Wi-Fi acquisition, PIN creation or saved-account synchronization, and transport
errors. It never logs account tokens or the PIN value.

## Toolchain and commands

The installed ESP-IDF is 5.4.2 (Waveshare requires 5.3.1 or newer). In an
ESP-IDF terminal, run these from this folder:

```powershell
idf.py set-target esp32s3
idf.py build
idf.py -p COM5 flash monitor
```

The first build downloads the pinned component dependencies. Converted UI
assets are in `main/assets/`; replace them from `../ui/` when updating the
exported design, then rebuild and flash.
