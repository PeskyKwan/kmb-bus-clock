# Public beta status — 2026-09-09

## Verified on one E32R28T-1
- Display, three-point touch calibration, saved configuration.
- Wi-Fi setup, automatic reconnect, certificate-verified KMB ETA after restart.
- Route catalog and boarding-stop selection for92 toward Diamond Hill at Pak Sha Wan.
- Approved smooth UI on the physical display.
- Host tests:12 ETA/time/alert-boundary checks plus16 ETA-animation checks. Firmware builds.
- ETA-driven marker added; physical animation acceptance is pending. It is a10-minute normalized countdown illustration, not GPS.

## Still beta / not yet fully verified
- Real-trip alert acceptance, all route/direction/service combinations, long names and unusual geography.
- General true-road geometry and aspect-preserving map fitting (see MAP_DESIGN.md).
- Local-network web-form operation across phone/browser combinations. Saved form selections do not fully pre-populate yet.
- Audio/speech/volume and scheduling are not implemented; reminders are visual and one-shot.
- Other ESP32 boards and operating systems are not device-tested.
- Public builds need local font assets. No precompiled binary or font files are redistributed in this source release.

No changes to the live KMB Brain website are required to use this project.

## Native settings update
- Four native setting pages and route keypad added; numeric/letter input and pagination host checks pass.
- A compact route candidate index is followed by live per-variant validation; up to32 variants and256 stops. New route numbers fall back to the regular service in both directions; regenerate the index to discover newly introduced variants. Stop names use a bundled cache for fast lists, and the selected stop plus predecessors are re-read from the API before staging.
- On-device USB hit-target test passed: numeric keypad92,4 live variants,29 stops, pagination to Pak Sha Wan, live metadata and unsaved-draft protection. Manual stylus/visual acceptance of the new settings pages remains pending. Wi-Fi password entry remains on phone/computer.
