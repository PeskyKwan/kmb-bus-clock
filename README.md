# KMB Bus Clock 🚌

A small, friendly Hong Kong bus-arrival display for the **LCDWIKI E32R28T-1**: a tilted red stop plate, large ETA, a compact three-stop map, and a touch reminder button.

**Public source beta.** Independent of [KMB Brain](https://github.com/hijapego1/kmb-brain). No integration or deployment dependency on that project. Not affiliated with or endorsed by KMB.

## What works on the tested board

- Live KMB ETA over certificate-verified HTTPS, with Wi-Fi reconnection and stored route settings.
- Route → destination/service variant → boarding-stop selection through a local web form.
- First-use three-point touch calibration, saved across application updates.
- Configurable lead time and screen brightness.
- One-shot **visual** reminder: arm it, receive one alert, tap to dismiss. Arm again for the next trip.
- Smooth Chinese/Latin fonts and the approved compact layout.
- A small animated bus driven by the real ETA countdown, explicitly labelled as an estimate.
- An illustrated road segment for the bundled 92/Pak Sha Wan example. Other locations use a labelled station-position schematic.

**Important beta limits:** audio is not implemented; the board has no attached speaker. No live bus GPS is supplied. General road-map fitting, label collision handling, arbitrary-route visual acceptance, and reminder schedules still need work. Never use this as your only way to decide when to catch a bus.

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

Font raster headers are generated locally and **not redistributed**. On macOS the generator defaults to installed STHeiti Medium and Arial Bold. On another system, provide locally licensed font files with Traditional Chinese coverage:

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

1. Tap the three calibration targets on the display.
2. Run `python setup/server.py` on the USB-connected computer. If auto-detection is ambiguous, set `KMB_SERIAL_PORT` explicitly.
3. Open **http://127.0.0.1:8767** on that computer.
4. Enter a **2.4 GHz** Wi-Fi network, search a route, choose direction/service and boarding stop, then save. Passwords are sent to the device, not written to a computer-side file or log.
5. Once connected, the display works without the computer. Stop the bridge with Ctrl-C and close its browser tab.

The display's **設定** button shows its local-network URL and a per-boot six-digit settings code. Use these to change settings from the same network. The web form is local HTTP, intended only for a trusted LAN; do not port-forward or expose it publicly. The code guards configuration writes. Wi-Fi credentials reside in device NVS, which is not encrypted by this beta.

Leaving the Wi-Fi inputs empty preserves the saved connection. A newly selected route must be searched before saving. Saved values are not yet fully pre-populated in the web form; check your route, direction, stop and reminder settings each time.

## What the display means

- A number is an estimated arrival time, not a guarantee.
- `--` means no usable ETA (offline, waiting, no forecast, failed request or stale data).
- The device normally polls every 30 seconds, backing off on failures. Data older than 120 seconds cannot trigger an alert.
- The bus marker is an **illustrative ETA countdown**, not GPS or a claim that a bus passed a particular stop. The final10minutes map to the displayed path:4minutes is approximately60% along. Longer waits stay at the start. It moves gently, follows ETA revisions, and disappears on stale/null/offline data.
- Notifications are one-shot. `提醒已開` means armed; an alert automatically disarms to avoid repeated alerts from ETA revisions.

See [map design and remaining work](docs/MAP_DESIGN.md), [beta status](docs/BETA_STATUS.md), and [third-party attribution](THIRD_PARTY.md).

## Tests

```sh
c++ -std=c++17 tests/eta_logic_test.cpp -o /tmp/kmb-eta-tests
/tmp/kmb-eta-tests
c++ -std=c++17 tests/eta_animation_test.cpp -o /tmp/kmb-animation-tests
/tmp/kmb-animation-tests
python -m py_compile setup/server.py
pio run
```

The12 ETA checks and16 animation checks cover timezone conversion, invalid/null timestamps, stale/future data, offline state, arming and threshold boundaries. They are not a substitute for device, route and real-trip testing.

## Contributing

Report board model, route/direction/stop, expected vs observed behavior, and a screenshot if useful. **Do not attach Wi-Fi passwords, NVS dumps or full-flash backups.** The visual language and layout are locked as a baseline, not frozen forever: propose improvements separately and get maintainer approval before redesigning. See [UI baseline](design/UI_LOCK.md).

Code is MIT-licensed. Data, fonts and third-party libraries have separate terms.
