# Public beta status — 2026-09-09

## Verified on one E32R28T-1
- Display, three-point touch calibration, saved configuration.
- Wi-Fi setup, automatic reconnect, certificate-verified KMB ETA after restart.
- Route catalog and boarding-stop selection for92 toward Diamond Hill at Pak Sha Wan.
- Approved smooth UI on the physical display.
- Host tests:12 ETA/time/alert-boundary checks. Firmware builds.

## Still beta / not yet fully verified
- Real-trip alert acceptance, all route/direction/service combinations, long names and unusual geography.
- General true-road geometry and aspect-preserving map fitting (see MAP_DESIGN.md).
- Local-network web-form operation across phone/browser combinations. Saved form selections do not fully pre-populate yet.
- Audio/speech/volume and scheduling are not implemented; reminders are visual and one-shot.
- Other ESP32 boards and operating systems are not device-tested.
- Public builds need local font assets. No precompiled binary or font files are redistributed in this source release.

No changes to the live KMB Brain website are required to use this project.
