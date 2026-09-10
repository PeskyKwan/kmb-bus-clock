# KMB Bus Clock 🚌

A small, friendly Hong Kong bus-arrival display for the **LCDWIKI E32R28T-1**: a tilted red stop plate, large ETA, a compact approach map, and a touch reminder button.

**Public beta with a browser installer.** Independent of [KMB Brain](https://github.com/hijapego1/kmb-brain). No integration or deployment dependency on that project. Not affiliated with or endorsed by KMB.

## Installation

**[Open the installer](https://peskykwan.github.io/kmb-bus-clock/)** on desktop Chrome or Edge.

1. Connect the correct E32R28T-1 board with a USB data cable and click Install.
2. Set Wi-Fi on the website or on the screen: gear → 裝置 → Wi-Fi.
3. Tap the first-boot calibration crosses, then choose your route, direction and stop on the screen.

No AI assistant, GitHub account or programming tools needed. The browser asks you to select USB Serial. [Full setup and recovery guide](docs/INSTALLER.md).

## What works on the tested board

- Live KMB ETA over certificate-verified HTTPS, with Wi-Fi reconnection and stored route settings.
- Native touch settings: numeric/letter route keypad → live destination/service variants → paged boarding stops. Settings stay as a draft until Save.
- On-device network scan and full password keyboard; connect/cancel with rollback to the old network on failure. Browser USB Wi-Fi setup is also available.
- First-use three-point touch calibration, saved across application updates.
- Configurable lead time and screen brightness. Warm cream day mode, dark green night mode, automatic sunrise/sunset switching, and a synchronized Hong Kong clock.
- One-shot **visual** reminder: arm it, receive one alert, tap to dismiss. Arm again for the next trip.
- Smooth Chinese/Latin fonts and the approved compact layout.
- A small animated bus positioned from matched multi-stop ETA forecasts and actual road distances, explicitly labelled as an estimate.
- Automatic road sections extending up to3km upstream along the selected bus route, ending at the boarding stop. All matching upstream stops inside the section are drawn as dots, with up to3 key name labels. Bends and east/west/north/south orientation are preserved at uniform scale. A nearby route origin limits the available length. Missing/mismatched/oversized geometry shows an unavailable message and retries.
- Buffered rendering sends only changed16×16 tiles; keypresses and ETA refreshes no longer clear the physical screen first.

**Important beta limits:** audio is not implemented; the board has no attached speaker. No live bus GPS is supplied. General maps have been verified on2A, but broader route/variant coverage, very long routes and reminder schedules still need testing. Never use this as your only way to decide when to catch a bus.

## Hardware

Tested: **E32R28T-1 / ESP32-WROOM-32E**, ST7789P3 240×320 LCD, XPT2046 resistive touch, 4 MB flash, single USB-C. Similar yellow boards can have different pin assignments: this is **not a generic CYD build**.

| Device | ESP32 pins |
|---|---|
| LCD | CLK14, MOSI13, MISO12, CS15, DC2; reset shared with EN |
| Backlight | GPIO21, active high |
| Touch | CLK25, MOSI32, MISO39, CS33, IRQ36 |
| Audio (disabled) | DAC26, amplifier enable4 active low |

See [LCDWIKI model resources](https://www.lcdwiki.com/res/E32R28T-1/) and the [dated model specification](https://www.lcdwiki.com/res/E32R28T-1/E32R28T-1_E32N28T-1_Specification_V1.0.pdf) (2025-04-22). Do not connect a buzzer arbitrarily to the speaker socket.

## Build

The physical beta was built on macOS. Linux/Windows support is not hardware-tested.

```sh
python3 -m venv .venv
# Activate the environment for your shell; Windows: .venv\Scripts\activate
source .venv/bin/activate
pip install -r requirements.txt
python tools/prepare_assets.py
pio run
```

Public installer firmware embeds Noto Sans TC/Noto Sans glyphs under SIL OFL; license files are included. `python tools/build_public_release.py` builds that package reproducibly with pinned font downloads. Custom local raster headers remain generated files. On macOS the generator defaults to installed STHeiti Medium and Arial Bold. On another system, provide locally licensed font files with Traditional Chinese coverage:

```sh
export KMB_CJK_FONT=/path/to/your/traditional-chinese-font.ttf
export KMB_LATIN_FONT=/path/to/your/latin-bold-font.ttf
python tools/prepare_assets.py
```

Different fonts may change the visual appearance; the approved physical reference uses the original fonts. Review before flashing. Font files remain subject to their own licenses, not this repository's MIT license.

The pinned PlatformIO platform/framework downloads its toolchain on first build. The application uses the `huge_app.csv` partition layout (3 MB application); OTA updates are not supported by this layout.

## Back up before installing

Use a data-capable USB cable. Identify the correct serial port (`/dev/cu.usbserial-*` on macOS, `/dev/ttyUSB*` on Linux, `COMx` on Windows). Replace `PORT` below with your port.

```sh
esptool --port PORT flash-id
# For the tested 4 MB board only:
esptool --port PORT --baud 115200 read-flash 0 0x400000 factory.bin
esptool --port PORT --baud 115200 verify-flash 0 factory.bin
```

Keep this backup private: it can contain device configuration. Backups are excluded from Git. On the tested board, 460800-baud reads were unreliable; 115200 worked.

## Install

First install changes boot/application partitions. Stop any serial monitor or setup bridge before flashing.

```sh
esptool --port PORT --baud 115200 write-flash --flash-mode dio --flash-freq 40m --flash-size 4MB \
  0x1000 .pio/build/board/bootloader.bin \
  0x8000 .pio/build/board/partitions.bin \
  0xe000 .platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin \
  0x10000 .pio/build/board/firmware.bin
```

After the initial partition layout is installed, application-only updates write just `0x10000 .pio/build/board/firmware.bin`, preserving Wi-Fi, calibration and route preferences. Do not erase the entire flash for a normal update.

## First setup

Use the installer above, or the screen's gear → 裝置 → Wi-Fi to scan/select a2.4GHz network and enter its password. Aa/123/符號 switch keyboard pages. The new credentials are saved only after a successful connection; failure/cancel restores the old network. The current and previous successful networks are remembered; select either to reconnect with an empty password.

Gear → 巴士 → 路線 opens the route keypad; choose direction/service and boarding stop, then 儲存. Route and brightness edits use Save; 自動/日間/夜間 mode taps save immediately. The legacy same-LAN configuration form and optional `setup/server.py` bridge remain available for development, but are not needed for the normal install flow.

## What the display means

- A number is an estimated arrival time, not a guarantee.
- `--` means no usable ETA (offline, waiting, no forecast, failed request or stale data).
- The device normally polls every 30 seconds, backing off on failures. Data older than 120 seconds cannot trigger an alert.
- Up to two bus markers are **multi-stop ETA position estimates**, not GPS. They match the next two ordered, independently supported upstream forecast chains across two distinct provider updates, then interpolate between stop predictions along the road. Schedule-only, ambiguous, stale or off-map positions are not shown; one reliable chain shows one bus rather than inventing a second. Short data gaps use only still-fresh history. Large revisions reacquire rather than driving backwards. The official first boarding-stop ETA and alerts remain independent. There is no fixed ten-minute journey animation.
- Position polling adds one bounded, streamed route-wide request about every30seconds while ETA/map are available. Actual positioning accuracy still needs journey validation.
- Notifications are one-shot. `提醒已開` means armed; an alert automatically disarms to avoid repeated alerts from ETA revisions.

See [map design and remaining work](docs/MAP_DESIGN.md), [beta status](docs/BETA_STATUS.md), and [third-party attribution](THIRD_PARTY.md).

## Tests

```sh
c++ -std=c++17 tests/eta_logic_test.cpp -o /tmp/kmb-eta-tests
/tmp/kmb-eta-tests
c++ -std=c++17 tests/eta_animation_test.cpp -o /tmp/kmb-animation-tests
/tmp/kmb-animation-tests
c++ -std=c++17 tests/eta_position_test.cpp -o /tmp/kmb-position-tests
/tmp/kmb-position-tests
c++ -std=c++17 -I.pio/libdeps/board/ArduinoJson/src tests/position_decode_test.cpp -o /tmp/kmb-decode-tests
/tmp/kmb-decode-tests
c++ -std=c++17 tests/native_input_test.cpp -o /tmp/kmb-native-input
/tmp/kmb-native-input
c++ -std=c++17 tests/map_render_test.cpp -o /tmp/kmb-map-tests
/tmp/kmb-map-tests
python -m py_compile setup/server.py
pio run
```

The ETA/animation/map/tile checks cover timezone conversion, invalid/null timestamps, stale/future data, offline state, arming and threshold boundaries. They are not a substitute for device, route and real-trip testing.

## Contributing

Report board model, route/direction/stop, expected vs observed behavior, and a screenshot if useful. **Do not attach Wi-Fi passwords, NVS dumps or full-flash backups.** The visual language and layout are locked as a baseline, not frozen forever: propose improvements separately and get maintainer approval before redesigning. See [UI baseline](design/UI_LOCK.md).

For integrated battery planning, see [portable power research](docs/PORTABLE_POWER.md).

The bare board has a battery connector, not a built-in battery pack. If powered only by USB, unplugging turns it off and stops monitoring; saved settings remain. External battery hardware has not been tested in this beta.

Code is MIT-licensed. Data, fonts and third-party libraries have separate terms.
