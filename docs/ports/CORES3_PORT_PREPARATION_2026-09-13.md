# CoreS3 Port Preparation

## Status

Compile-only scaffold prepared for M5Stack CoreS3 K128. No CoreS3 hardware was connected or flashed. It does not display live ETA and is not a release artifact.

## Why CoreS3 is the lower-risk second target

- Same 320×240 landscape resolution as the existing E32R28T-1 UI.
- Capacitive touch, so the interaction model can remain close to the current product.
- ESP32-S3, 16MB flash and 8MB PSRAM provide more headroom.
- Built-in speaker enables a future audible reminder.
- The K128 kit's DinBase contains a 500mAh battery.

The whole K128 set is 69×54×31.5mm and 72.7g. It is smaller than Cardputer-Adv in footprint but considerably thicker than StickS3. CoreS3-Lite is thinner at 54×54×16.5mm but has only a 200mAh battery. Confirm the exact purchased SKU before any flash or battery-runtime claim.

## Prepared build target

```sh
pio run -e cores3
```

The default remains the current board:

```sh
pio run
```

`default_envs = board` and explicit source filters prevent the CoreS3 scaffold from entering the current E32R28T-1 public build.

## Compile verification

- Default `pio run`: PASS, board environment only.
- E32R28T-1 binary: 2,325,776 bytes, SHA-256 `920dfae26b6ed4e5d025eaa75497b8deb1f3eb89761f8945a15c133940d7a0f3`.
- This is byte-identical to the installed, physically verified personal 0.2.9 binary.
- `pio run -e cores3`: PASS.
- CoreS3 scaffold static RAM: 48,584 / 327,680 bytes (14.8%).
- CoreS3 scaffold flash: 950,321 / 3,342,336 bytes (28.4%).
- CoreS3 binary: 950,688 bytes, SHA-256 `3d8ce889972074f4a1157d83e924c81bcb7a1a7b9bd91cd02472b72e13c2971d`.
- The scaffold deliberately uses the conservative 8MB generic ESP32-S3 build map until the physical K128's 16MB flash and safe update partitions are verified.

## What the scaffold proves

- Official M5Stack Arduino/PlatformIO environment resolves.
- M5Unified initializes display, touch, power and board detection APIs at compile time.
- Wi-Fi, TLS/HTTP, WebServer, Preferences and ArduinoJson coexist in the target.
- Shared `eta_logic.h` and `eta_animation.h` compile in the CoreS3 target.
- A 320×240 diagnostic screen and `/status` response are ready for first-device acceptance.

## What it does not prove

- Physical LCD orientation/color/backlight.
- FT6336 touch coordinates and edge accuracy.
- Battery percentage, charging or runtime.
- Speaker output or brownout safety.
- Wi-Fi/TLS, KMB API or road API on physical hardware.
- Current full UI rendering through M5GFX.
- NVS migration or installer compatibility.
- Application-only update offsets.

## First hardware acceptance after purchase

1. Confirm packaging/SKU is CoreS3 K128 and whether the 500mAh DinBase is attached.
2. Build the scaffold and record binary hash/size.
3. Flash only the CoreS3; never connect the current E32R28T-1 to the CoreS3 environment.
4. Verify 320×240 landscape, colors, touch and `/status`.
5. Verify charging, battery reading and safe power cycle.
6. Verify speaker at low volume.
7. Verify home Wi-Fi and iPhone hotspot.
8. Only then connect the shared ETA worker and renderer.

## Intended architecture

The full port should keep shared ETA/route/map/network logic and use separate platform/display implementations:

```text
E32R28T-1 -> Adafruit ST7789 + XPT2046 + current locked UI
CoreS3    -> M5Unified/M5GFX + FT6336 touch + CoreS3 power/audio
```

Do not replace the existing renderer globally. Add a CoreS3 display adapter and prove the current board remains byte/behavior stable before sharing more UI code.

## Sources

- M5Stack, [CoreS3 documentation](https://docs.m5stack.com/en/core/CoreS3), accessed 2026-09-13.
- M5Stack, [CoreS3 official store listing](https://shop.m5stack.com/products/m5stack-cores3-esp32s3-iotdevelopment-kit), accessed 2026-09-13.
- M5Stack, [CoreS3-Lite documentation](https://docs.m5stack.com/en/core/CoreS3-Lite), accessed 2026-09-13.
- M5Stack, [M5Unified releases](https://github.com/m5stack/M5Unified/releases), current release 0.2.19 checked 2026-09-13.
