# Havit OS application backend

This project currently builds the LVGL desktop simulator. The Havit OS code in
`src/havit_app.*` is a platform independent C application layer, not a web API
or cloud service. It owns navigation and application state; LVGL screens are
the presentation layer and ESP-IDF drivers will be separate adapters.

## Current flow

| State | Event | Next state |
| --- | --- | --- |
| First boot | Initialize | Onboarding (`frame_1`) |
| Onboarding | Continue | Sign in (`frame_2`) |
| Sign in | Login started | Sign in, authentication pending |
| Sign in | Verified login success | Welcome back (`frame_3`) |
| Welcome back | Continue | Home placeholder |
| Home / Welcome back | Logout | Sign in |

The authentication result must come from a real provider integration. This
module does not accept passwords or save access tokens. The display name is
held in RAM for the current session.

## Storage boundary

`havit_store_t` is the persistence port. The desktop simulator uses
`havit_store_memory.c`, so onboarding completion resets when the process exits.
The ESP32-S3 target should implement this same interface with ESP-IDF NVS and
link that adapter instead. Keep secrets out of the generic settings store.

## UI integration boundary

The ESP-IDF target in `firmware/` adapts the three exported screens into native
LVGL widgets using the positions from `ui/screens/frame_*.xml` and embedded
assets from `ui/images/` and `ui/fonts/`. It translates taps into the events
above. The Google sign-in is currently a demo success returning `ROM`; replace
that callback with real provider authentication before treating it as a login.
The actual home screen remains unimplemented.

## Build targets

The desktop CMake target includes the portable application core and volatile
store. The ESP-IDF project can reuse `havit_app.c` and provide its own NVS
adapter; `src/main.c` and `src/hal.c` remain simulator-specific.
