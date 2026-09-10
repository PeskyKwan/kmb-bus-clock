# Multi-stop animation update —0.2.1

The marker now uses up to12 road-distance ETA anchors and up to3 forecasts per anchor. It requires two distinct provider updates and a unique compatible chain near the target. Schedule-only/ambiguous/stale cases fade out; the large ETA and one-shot alert logic are unchanged. Fixed600s progress removed. Host tests cover ranking changes, ambiguity, schedules, history, freshness and malformed streaming input. Physical ground-truth location accuracy is not certified.

Physical update check (2026-09-10): app-only flash/hash verified; saved92 configuration reconnects, target ETA and road load, route-wide stream decodes24 selected forecast rows. Matching was not ready during this observation, so the marker stayed hidden; actual road-position accuracy still needs trip testing. Fixed a TLS allocation failure by removing the eager16KB road-vector reservation before the handshake.

Historical checks follow:

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
- Public builds need local font assets. Public installer binaries now use OFL Noto fonts; see INSTALLER.md.

No changes to the live KMB Brain website are required to use this project.

## Native settings update
- Four native setting pages and route keypad added; numeric/letter input and pagination host checks pass.
- A compact route candidate index is followed by live per-variant validation; up to32 variants and256 stops. New route numbers fall back to the regular service in both directions; regenerate the index to discover newly introduced variants. Stop names use a bundled cache for fast lists, and the selected stop plus predecessors are re-read from the API before staging.
- On-device USB hit-target test passed: numeric keypad92,4 live variants,29 stops, pagination to Pak Sha Wan, live metadata and unsaved-draft protection. Manual stylus/visual acceptance of the new settings pages remains pending. Wi-Fi scan/password entry is now available on-device and via browser USB.

## General roads and flicker fixes
-2A at Ngau Tau Kok: autonomous road download/matching PASS,73 cropped points, live ETA PASS.
-Framebuffer/renderer verified on device: blank key-page tap0 transmitted tiles; entering a key2/300 tiles; idle animation54 tiles over20s (crossing the old15s refresh interval), no full-frame clear. Palette preserves115 exact UI/antialias colors. Manual visual acceptance pending.
-General geometry matches up to3 stops in route order within about150m, keeps intermediate bends and uniform scale; up to2048 source points and256 displayed points. Missing data falls back honestly. Data is fetched again after reboot/route changes; no persistent road cache yet.

## Approved3km approach window
- Fixed3-stop framing replaced by up to3km measured along the incoming road, keeping geographic direction and all matched upstream station dots (up to64 capacity), with up to3 key labels. Nearby origin limits the segment. No walking/preparation logic added.
- Source simplification checks all intermediate points against~2m deviation and keeps vertices no farther than80m apart; this permits dense92 data to fit in memory. Host fixtures:2A at Ngau Tau Kok has~982m available;92 Pak Sha Wan has3000m.

- Final on-device3km-version checks:2A near route origin displays982.4m,3 markers and31 rendered points; ETA/display ready. Blank tap0tiles, key entry2/300,20s idle16tiles. Route-stop data is reused from validation; failed map loads retry after30s.
