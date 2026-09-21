# M5Stack StickS3 Port Feasibility

## Executive verdict

**Build a StickS3 edition as a second portable target, not as a drop-in replacement for the current E32R28T-1 firmware.** The KMB ETA, route, map, Wi-Fi, alert and bus-motion logic are portable. The existing hardware layer, renderer, 320×240 layout, touch calibration and touch-driven settings are not.

StickS3 is technically strong enough: ESP32-S3 at 240MHz, 8MB flash, 8MB PSRAM, 2.4GHz Wi-Fi, an ST7789-family 135×240 display, supported Arduino/PlatformIO libraries and a built-in speaker.^1 A compile-only probe using M5Stack's documented PlatformIO configuration successfully linked the display, PMIC, Wi-Fi/TLS, WebServer, Preferences and ArduinoJson stack.^2

The two material constraints are physical:

1. **No touch screen.** The current device's route keypad, stop selector, Wi-Fi keyboard, theme and reminder pages cannot simply be scaled down. StickS3 has two normal user buttons; its power/reset button has system semantics and should not be treated as a third ordinary UI key.^3
2. **Only a 250mAh battery.** This is suitable for a compact short-trip device, not a safe assumption of all-day, always-on Wi-Fi use. Runtime must be measured on real hardware. Until then, plan around roughly one to two hours of continuous bright-screen/Wi-Fi operation, with two hours as a target rather than a promise.

The best product shape is therefore:

- E32R28T-1 remains the full-size touch-screen desk clock.
- StickS3 becomes a compact, button-driven travel display.
- Full route/Wi-Fi configuration moves to the browser installer or local phone page.
- On-device buttons handle only frequent actions: reminder/dismiss, page change, brightness and switching between two already-saved networks.
- The portable UI is redesigned for 240×135 landscape rather than proportionally shrinking the 320×240 screen.

## Decision criteria

The hardware decision is based on the intended outcome rather than the novelty of the board.

| Criterion | Required outcome | StickS3 result |
|---|---|---|
| Portability | Pocketable, integrated battery | Excellent: 48×24×15mm, 20g, 250mAh |
| Live ETA | Stable 2.4GHz Wi-Fi, TLS, JSON | Feasible; same ESP32 Arduino network stack |
| Legibility | Route and ETA readable at a glance | Feasible with a new information hierarchy |
| Map animation | At least one useful moving bus | Feasible; two may be shown when spacing permits |
| Configuration | A beginner can install and set a route | Feasible through WebSerial/web setup, not current touch UI |
| Daily control | Reminder/dismiss and basic navigation | Feasible with two user buttons |
| Audio alert | Audible reminder without extra hardware | Better than current board: built-in codec, amp and speaker |
| Battery | Useful for a trip away from USB | Plausible for short use; not yet proven |
| Maintainability | Avoid a forked duplicate codebase | Requires shared core plus two board/UI targets |
| Public installation | Wrong-board flashing must fail closed | Requires a separate StickS3 manifest and chip/board guard |

## Hardware evidence

StickS3 K150 uses an ESP32-S3-PICO-1-N8R8 with 8MB flash and 8MB Octal PSRAM. Its 1.14-inch ST7789P3 display is 135×240 pixels; landscape orientation is therefore 240×135. It includes a 250mAh battery, M5PM1 power management, two user buttons on GPIO11/GPIO12, built-in ES8311 audio, an AW8737 amplifier and 1W speaker.^1

The official pin map is completely different from E32R28T-1: StickS3 LCD MOSI/SCK/DC/CS/RST/backlight are GPIO39/40/45/41/21/38. The current board directly initializes a different SPI bus and its XPT2046 touch controller on board-specific pins. Reusing the current `hardware.inc` would therefore be unsafe.

M5Stack documents a PlatformIO target based on `esp32-s3-devkitc-1`, `default_8MB.csv`, `qio_opi`, PSRAM and native USB CDC flags, with M5Unified and M5PM1.^4 M5Unified 0.2.19 is current within the six-month research window and explicitly supports M5StickS3; recent releases also include StickS3/M5PM1 power fixes and APIs.^5

This is a relatively new platform. Recent M5Unified issues show that power-button and power-off behavior needed StickS3-specific fixes, and an open July 2026 issue still discusses explicit L3A/L3B low-power control.^3 That does not block an always-on MVP, but it means battery and wake behavior must be validated on the exact library versions rather than inferred from older M5Stick products.

## What can be reused

The following code is mostly board-independent and should remain shared:

- ISO-8601 parsing, ETA freshness and alert rules
- first/second official ETA selection
- scheduled-bus classification
- persistent guess/hold/recovery state
- strict multi-stop position matcher
- streamed KMB route-ETA decoding
- road acquisition, simplification and route-fraction calculations
- day/night calculation
- route/stop catalog and validation
- two saved Wi-Fi credential slots
- safe configuration serialization excluding credentials
- certificate-verified KMB and road HTTPS requests
- public-installer safety rules around NVS and board identity

The existing host test suite already isolates much of this logic from the physical display. That is the strongest reason a second target is realistic.

## What must be replaced or split

### Hardware abstraction

The current firmware directly owns:

- `Adafruit_ST7789`
- fixed HSPI/VSPI pins
- LCD vendor-init commands
- XPT2046 touch sampling and calibration
- direct PWM backlight
- amplifier-disable GPIO
- 320×240 tile transmission

StickS3 should use M5Unified/M5GFX for LCD, buttons, speaker, battery and PMIC. Do not add another set of scattered `#ifdef STICKS3` statements throughout `main.cpp`. Introduce narrow interfaces:

```text
DisplayBackend   width, height, pixel, fill, present, brightness
InputBackend     events: next, select, back, dismiss
PowerBackend     battery %, charging, dim, sleep, power-off
AudioBackend     short alert tone, volume cap
BoardProfile     model ID, installer chip/flash layout, capabilities
```

### Renderer

The current `BufferedDisplay` is hard-coded to 320×240, 20×15 tiles and 320-byte indexed rows. Nineteen direct 320/240/174/140 layout references remain across the renderer/settings files. The StickS3 target needs a 240×135 framebuffer and its own layout constants.

The smaller framebuffer is not a memory problem: an 8-bit 240×135 surface needs 32,400 bytes, compared with 76,800 bytes now. PSRAM gives additional margin, although display/TLS-critical buffers should be deliberately placed and tested rather than assuming every allocation automatically uses PSRAM.

### Input and settings

StickS3 has no touch panel. Shrinking the current on-screen keyboard would create an unusable interface. Two-button entry of arbitrary route names, Wi-Fi symbols and passwords is also the wrong design.

Recommended split:

- Browser installer: board confirmation, firmware, Wi-Fi, route, direction, stop, reminder and theme.
- Same-LAN phone page: later configuration changes.
- StickS3 Button A short: cycle summary/map/battery pages.
- Button A long: brightness menu.
- Button B short: arm/cancel reminder.
- Button B while alarming: dismiss.
- Button B long: switch between the two saved Wi-Fi profiles, with confirmation.
- Both buttons long: show local setup URL/PIN or enter a bounded setup mode.
- Power button: retain documented reset/off/download behavior; do not overload it.

This removes the feature-parity illusion. StickS3 would share capability, not interaction layout.

## Proposed 240×135 UI

The display should use landscape 240×135 and keep only the information needed at a glance.

```text
┌──────────────────────────────────────┐
│ N241   往長宏邨      05:12     78%  │
│ ┌───────────── map ────────────────┐ │
│ │  ·────🚌────·────────🚌 zZ────●  │ │
│ └─────────────────────────────────┘ │
│  8 分鐘到 · 2.2km估算       提醒○  │
└──────────────────────────────────────┘
```

Design rules:

- Route number and ETA are primary; destination is one truncated line.
- Map occupies most of the width, but labels are limited to current stop plus one upstream landmark.
- Prefer one bus at full clarity; show a second only when the two remain distinguishable.
- Scheduled `zZ` remains a small status illustration.
- Battery percentage replaces low-priority decorative detail.
- No touch-looking buttons. Button hints appear briefly after input, not permanently.
- Preserve warm cream/dark green/red identity, but this is a new approved layout for another device rather than a modification of the locked 320×240 composition.

## Firmware and build architecture

Recommended repository structure:

```text
src/core/             ETA, route, matcher, Wi-Fi/config logic
src/platform/e32/     current SPI, XPT2046, backlight, display backend
src/platform/sticks3/ M5Unified, M5PM1, buttons, battery, audio
src/ui/e32/           locked 320×240 touch UI
src/ui/sticks3/       new 240×135 button UI
platformio.ini        env:e32r28t and env:sticks3
installer/manifests/  one signed/hashed manifest per exact board
```

The current E32R28T-1 behavior must remain the regression baseline. Shared logic moves only after existing host tests are green. Each extraction should be mechanical and verified before changing behavior.

### Compile evidence

An isolated probe used M5Stack's documented PlatformIO settings with:

- M5Unified 0.2.19
- M5GFX 0.2.28
- M5PM1 1.0.7
- Wi-Fi, WiFiClientSecure, HTTPClient, WebServer, Preferences and ArduinoJson

It built successfully at 48,348 bytes static RAM and 960,965 bytes flash.^2 This proves the supported toolchain and required libraries can coexist. It does not prove physical LCD orientation, PSRAM, battery, buttons, audio, WebSerial reset behavior or the full KMB firmware.

The current KMB application is about 2.32MB. The StickS3 documented default app partition allows about 3.34MB in the compile probe. M5Unified adds meaningful code size, so the full target could approach that limit. Because the board has 8MB flash, a larger no-OTA partition is technically possible; the better first step is to measure the real port before choosing a partition. Do not sacrifice rollback/install safety pre-emptively.

## Public installer implications

The current public installer must never offer the E32R28T-1 binary to StickS3. StickS3 is ESP32-S3, uses a different flash layout, native USB behavior and board initialization.

Required installer changes:

- Board choice before WebSerial: E32R28T-1 or StickS3 K150.
- Separate manifests and binaries.
- Verify ESP32 versus ESP32-S3 chip family.
- Verify expected flash size.
- Keep NVS/config partitions outside update writes.
- Use M5Stack's download-mode instructions for recovery.
- Test Chrome/Edge reconnection after native USB reset.
- Beginner path collects Wi-Fi and route because StickS3 cannot type them comfortably.
- Advanced path documents `git clone`, `pio run -e sticks3` and serial monitor commands.

M5Stack community evidence indicates direct USB flashing is the dependable baseline; launcher/web workflows vary by firmware ecosystem.^6 The existing browser installer is therefore still a sensible path, but it needs real StickS3 hardware testing.

## Battery analysis

The 250mAh cell stores roughly 0.9Wh nominal. Official measurements list 36.69mA for an active L3A state and 519mA at full load, but the published table does not establish the current of this specific always-on LCD/Wi-Fi/TLS workload.^1 L3A describes the active ESP32-S3 power level; display and peripheral rails are controlled separately.^7

Consequently:

- `250mAh ÷ 36.69mA ≈ 6.8h` is an unrealistic upper bound for the complete bus clock.
- Cardputer-Adv's official Wi-Fi operating current is 132.3mA. Using that only as a rough same-family reference gives `250 ÷ 132.3 ≈ 1.9h` before conversion losses and reserve.^8
- Recent owners independently flag the 250mAh capacity as the main limitation and use deep sleep or motion wake for intermittent workloads.^9

**Planning position:** expect one to two hours for the first always-on prototype. Treat two hours as a success target, not a specification. Measure at 25%, 50% and 75% backlight with 30-second KMB polling, two HTTPS endpoints, animation and periodic audio. Report time to low-battery warning and shutdown, not only percentage estimates.

Ways to improve runtime:

1. Dim backlight aggressively on battery.
2. Pause route-wide positioning when the screen is off; retain target ETA polling.
3. Use 60-second polling when ETA is far away and 30 seconds near arrival.
4. Turn off speaker amp except during an alert; M5Stack recommends keeping battery-powered volume below 75%.^1
5. Add a button-driven screen-off mode with periodic audio alert.
6. Consider light/deep sleep only for a glanceable mode. A permanently visible clock cannot gain deep-sleep runtime without changing the product behavior.

If the actual requirement becomes four to eight continuous hours, StickS3 is the wrong battery platform unless paired with external power.

## Audio opportunity

StickS3 solves the current board's missing-speaker problem. The ES8311/AW8737/1W speaker is supported through M5Unified. A short chime or simple Cantonese cue is feasible.

For the first target:

- Use a short local tone, not speech synthesis.
- Keep volume at or below 75% on battery per M5Stack's warning.^1
- Power the amplifier only while sounding.
- Test for brownouts during Wi-Fi transmit plus audio.
- Keep the visual reminder independent so audio failure never changes ETA truth.

## Alternative hardware

The market contains meaningfully different trade-offs.

| Device | Display/input | Battery | Size/weight | Current official price/stock | Port judgment |
|---|---|---:|---|---|---|
| StickS3 K150 | 240×135, two user buttons, no touch | 250mAh | 48×24×15mm, 20g | US$21.50, 10+ listed | Best size; weakest input/runtime |
| Cardputer-Adv | 240×135, 56-key keyboard | 1750mAh | 84×54×19.6mm, 81g | US$29.90, 10+ listed | Best portable usability/runtime; much larger |
| CoreS3 | 320×240 capacitive touch | 500mAh in included base | core 54×54×15.5mm; full set thicker | US$59.90, 10+ listed | Easiest UI port; not truly stick-sized |

Live stock is the official global store listing, not Hong Kong landed stock or delivery time.^10 DigiKey Hong Kong listed K150 as active but zero in stock at the checked moment, so no local trip recommendation is justified.^11

### Recommendation among the three

- Choose **StickS3** if pocket size is the main objective and short runtime is acceptable.
- Choose **Cardputer-Adv** if this must work for hours and route/Wi-Fi entry on the device matters. It is the stronger self-contained field unit.
- Choose **CoreS3** if preserving the current touch workflow matters more than minimum size.

For the stated goal—very small, battery-powered and fun—the StickS3 is worth one prototype. It should not become the only supported device until battery and button-only interaction are proven.

## Recommended implementation sequence

### Phase 0 — hardware acceptance

Buy one unit only. Before porting application code, verify:

- exact K150 identity
- LCD orientation, offsets, color order and backlight
- Button A/B short and long press
- battery percentage, charging and clean power-off
- speaker at safe volume
- native USB flash, reset and serial reconnect
- 2.4GHz home Wi-Fi and iPhone hotspot reception

### Phase 1 — architecture split

- Keep current E32 target green.
- Extract shared core without behavior changes.
- Add `sticks3` PlatformIO environment pinned to known library versions.
- Implement M5Unified display/button/power/audio backends.

### Phase 2 — portable MVP

- Preconfigured route and Wi-Fi through browser/serial.
- 240×135 main screen.
- one bus marker first; second marker only after legibility QA.
- Button A page cycle; Button B reminder/dismiss.
- battery indicator and low-battery warning.

### Phase 3 — configuration and power

- Extend beginner installer to collect configuration.
- Add two-saved-network toggle.
- Add same-LAN phone settings.
- Measure always-on runtime.
- Add dim/screen-off modes only after baseline measurement.

### Phase 4 — audio and public release

- short alert tone with brownout/volume tests
- separate StickS3 public manifest
- wrong-board rejection tests
- recovery instructions
- real-device screenshots and runtime disclosure

## Acceptance gates

Do not call the port complete until all are true:

- Current E32R28T-1 0.2.9 behavior remains unchanged.
- StickS3 cold boot, Wi-Fi, NTP, KMB TLS and map TLS pass repeatedly.
- No credential appears in logs or firmware artifacts.
- Route/stop selected through beginner setup persists after power-off.
- Home/hotspot switching passes without retyping saved passwords.
- 240×135 screen remains readable in day and night mode.
- Scheduled `zZ`, no-service clearing and one/two-bus transitions are visually verified.
- Reminder/dismiss works with buttons.
- Battery runtime is measured, with backlight and polling settings recorded.
- Low battery and audio cannot cause a reboot loop.
- Browser installer refuses the wrong board and preserves configuration partitions.

## Final recommendation

Proceed with a **separate StickS3 prototype target** after obtaining one physical unit. Do not begin by scaling every coordinate or copying `main.cpp`. First split core/platform/UI boundaries, then build a deliberately simpler 240×135 button interface.

The StickS3 is a good match for a pocketable short-trip bus clock and adds a useful speaker. It is not a good match for the existing touch-based setup experience or an all-day always-on assumption. Cardputer-Adv is the stronger long-runtime portable device; CoreS3 is the lower-risk touch-compatible port. StickS3 wins only when smallest size is the decisive requirement.

## Evidence freshness and limits

The first search pass used only material from the previous six months: current store inventory, M5Unified releases/issues, current official repositories and recent independent user reports. This was insufficient for the original product specification and launch facts. The search therefore expanded to the permitted two-year window and used M5Stack's January–February 2026 launch/PDF materials. No source older than two years was used.

No physical StickS3 was available. Battery runtime, display orientation, PSRAM behavior, RF performance, speaker brownout behavior and native USB recovery remain unverified. Store stock is current online listing only; Hong Kong delivery, tax and exact landed cost were not checked.

## Sources

1. M5Stack, [StickS3 product documentation](https://docs.m5stack.com/en/core/StickS3) and [dated StickS3 product PDF](https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/docs/static/pdf/static/ja/core/StickS3.pdf), updated February 27, 2026. Hardware, pin map, power, dimensions and battery.
2. KMB Bus Clock, [StickS3 compile probe evidence](../../evidence/reports/STICKS3_COMPILE_PROBE_2026-09-13.md), September 13, 2026.
3. M5Stack M5Unified, [StickS3 power-button issue #233](https://github.com/m5stack/M5Unified/issues/233), February 2026, and [low-power issue list](https://github.com/m5stack/M5Unified/issues), July 2026.
4. M5Stack, [StickS3 PlatformIO configuration](https://docs.m5stack.com/en/core/StickS3#platformio), accessed September 2026.
5. M5Stack, [M5Unified releases](https://github.com/m5stack/M5Unified/releases), releases 0.2.14–0.2.19, April–July 2026; [M5Unified repository](https://github.com/m5stack/M5Unified).
6. SimonBear03, [m5stack-codex-companion](https://github.com/SimonBear03/m5stack-codex-companion), 2026. Recent independent StickS3/Cardputer build and flashing experience.
7. M5Stack, [StickS3 M5PM1 power-level documentation](https://docs.m5stack.com/en/arduino/m5sticks3/m5pm1), accessed September 2026.
8. M5Stack, [Cardputer-Adv documentation](https://docs.m5stack.com/en/core/Cardputer-Adv), accessed September 2026. Battery, operating/Wi-Fi current, display and keyboard.
9. M5Stack community, [recent StickS3 deployment discussion](https://www.reddit.com/r/M5Stack/comments/1w1of46/first_time_buying_this_many_sticks3_boards_for_a/), August 2026; [StickS3 low-power counter project](https://www.reddit.com/r/M5Stack/comments/1tqjx2j/m5sticks3_and_dual_button_unit_esphome_mqtt/), May 2026. Anecdotal evidence only.
10. M5Stack Store, [StickS3](https://shop.m5stack.com/products/m5sticks3-esp32s3-mini-iot-dev-kit), [Cardputer-Adv](https://shop.m5stack.com/products/m5stack-cardputer-adv-version-esp32-s3), and [CoreS3](https://shop.m5stack.com/products/m5stack-cores3-esp32s3-iotdevelopment-kit), live listings checked September 13, 2026.
11. DigiKey Hong Kong, [M5Stack K150 listing](https://www.digikey.hk/en/products/detail/m5stack-technology-co-ltd/K150/28739297), live listing checked September 13, 2026.
