# Install without development tools

Open https://peskykwan.github.io/kmb-bus-clock/ on desktop Chrome or Edge.

1. Connect the E32R28T-1 with a USB data cable, confirm the model, press Install and choose USB Serial in the browser's permission dialog.
2. Wait for the verified write to finish. Choose a2.4GHz Wi-Fi network and enter its password on the website, or skip this step and use the screen.
3. On first boot tap the three calibration crosses. Screen: settings gear → 巴士 → 路線 → direction → stop → 儲存.

No GitHub account, AI assistant, terminal or compiler is needed. The browser's USB chooser is required by the browser and cannot be skipped. Safari/iPhone are not the initial USB-install target. A data-capable cable is required; if no serial device appears, install the official driver matching the board's USB-serial chip rather than an arbitrary driver.

## Changing Wi-Fi on the screen

Gear → 裝置 → Wi-Fi → network → password → 連接. 搜尋 rescans, 手動 supports hidden network names. Aa changes case,123/symbol pages provide digits and punctuation. Passwords are masked. Empty password reuses the selected remembered network's saved password; an open network can use an empty password. WPA-Enterprise/captive-login networks are not supported by this simple password workflow.

The device pauses outstanding requests before switching. It saves credentials only after connection succeeds; failure/cancel restores the previous network. It remembers the current and previous successful networks (for example home and phone hotspot). Select either with an empty password to reuse its stored password; automatic roaming between them is not implemented. Changing Wi-Fi does not reset the route, alert, brightness or touch calibration. During scan, the radio may briefly disconnect then return to the saved network.

The website's Wi-Fi helper uses local USB messages, not a server upload. It does not store passwords in browser storage or send them to GitHub. Device NVS is not encrypted in this beta. No credentials or device backup are in the public firmware.

## Installation and recovery

The installer accepts originalESP32/4MB only; the model checkbox additionally identifies the correct display board. It writes four clean build outputs, preserves NVS at0x9000–0xdfff, checks download SHA256 and verifies flash MD5. It does replace existing firmware/partitions; only the documented E32R28T-1 is supported. Updating this project preserves settings. No erase-all operation is used.

If interrupted, reconnect USB and retry this same installer. If connection stalls, hold BOOT, tap RESET, then release BOOT and retry. Only one serial tool may hold the device at a time. The current partition layout has no OTA slot; updates use the same USB website.

Public firmware uses pinned SIL-OFL Noto Sans TC/Noto Sans glyphs. Maintainer-local builds may use the original installed fonts; the layout is the same but glyph shapes can differ. The public binary is built from source, never from a device dump.

## Rebuilding

Install the Python dependencies described in README and run `python3 tools/build_public_release.py`, then `npm ci --ignore-scripts`, `node node_modules/esbuild/install.js`, and `npm run build:installer`. The release script downloads hash-pinned open fonts, regenerates assets and writes installer/manifest.json plus four firmware files. Run `node tests/installer_artifact_test.mjs` before publishing. Serve installer/ over HTTPS (localhost is allowed for development).
