# Installed scheduled-bus sleep animation —0.2.9 (2026-09-13)

Live N241 evidence at04:49 HKT showed two target ETAs,05:05 and05:35, both marked `Scheduled Bus`; device positioning had0/0 samples, both icons were `eta-guess` at the map entrance, so they were not evidence of two physical buses parked at the terminal. Version0.2.9 carries the official scheduled flags into the illustration state and draws a gently moving custom `zZ` only for a scheduled, unmatched ETA guess still at the entrance. Adjacent sleeping buses share one sign cluster. App-only flash/hash verification passed with NVS preserved. Final physical framebuffer at05:02 HKT showed the first forecast no longer scheduled and advancing without `zZ`, while the second remained scheduled at the entrance with `zZ`; two distinct bus bodies, locked layout and labels remained intact. All11 host suites and the final firmware build pass. Not pushed or published.

# Installed Wi-Fi/off-hours correction —0.2.8 (2026-09-13)

Installed app-only at `0x10000`; esptool verified the written data hash and preserved NVS. The native Wi-Fi list now highlights only the SSID actually connected, while still retaining the current and previous successful credentials. Tapping the other saved network reconnects directly with its stored password; unsaved networks still open the password keyboard. The serial diagnostic distinguishes `saved` from `active` without exposing passwords. The persistent bus illustration now survives one transient empty response but clears after two consecutive current successful `no forecast` responses; transport/API failures do not clear it. All 11 host suites and the firmware build pass. Post-flash route92 off-hours produced three consecutive `etaCode=1` states with `animationCount=0`; a fresh native scan reported exactly one active network, and the failed-connect rollback plus saved-current reconnect passed despite the original CH340 handle timing out after the board had already reconnected. The previous iPhone hotspot was not visible for a live second-profile switch-back test. Not pushed or published.

# Two persistent buses —0.2.7 (2026-09-10)

At Sum's request, both of the first two distinct official target forecasts now have guess/hold/recovery behavior, including scheduled forecasts. The existing target response supplies the second ETA; there are no extra network requests. A matching second journey is promoted when the first arrival rolls over. When sprites overlap, artwork is offset within the same map so both remain distinguishable. Official main ETA/alerts and the strict multi-stop matcher are unchanged. App-only installation/digest verification passes; real1A framebuffer verifies two complete distinct bus bodies while both strict matches are unavailable. See [two-bus validation](TWO_BUS_ILLUSTRATION_0.2.7.md).

# Persistent primary bus —0.2.6 (2026-09-10)

At Sum's explicit request, a fresh ETA now seeds a rough primary bus position even without a reliable multi-stop match. After matching, temporary missing/ambiguous/stale data holds the last position; recovery corrects it without blinking away. The main ETA/alerts remain source-based and the map stays labelled估算. Two matcher bugs are also fixed; all host suites and separate personal/OFL builds pass. App-only physical installation/digest verification completed; framebuffer has the little bus while strict matching is unavailable, and subsequent state confirms held visibility. Saved2A, Wi-Fi, main ETA/map and1A BOTH destination stop lists pass without saving. See [audit evidence](POSITION_AUDIT_2026-09-10.md).

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
- Beta0.2.2 adds a faded road continuation after the selected boarding stop. The context-only extension is normally15% of the approach, bounded to150–450m and clipped at the route endpoint; it does not enter the bus animation, ETA or reminder logic.
- Beta0.2.3 immediately retries one observed TLS connection-EOF during KMB ETA validation or map loading. CA verification remains mandatory; all other failures retain the bounded backoff and honest unavailable state.
- Beta0.2.4 can show up to two little buses when the route-wide feed supports two ordered, independently matched live forecast chains. Each needs two provider updates; scheduled, stale, ambiguous and off-map chains remain hidden. Main ETA and reminders still follow only the next bus.
- Beta0.2.5 repairs native route selection: the bundled official route catalog supplies destination choices without a burst of HTTPS connections, while the selected stop list is still verified live. A failed direction or stop request stays on the relevant page with a retry message instead of discarding the search.
- Source simplification checks all intermediate points against~2m deviation and keeps vertices no farther than80m apart; this permits dense92 data to fit in memory. Host fixtures:2A at Ngau Tau Kok has~982m available;92 Pak Sha Wan has3000m.

- Final on-device3km-version checks:2A near route origin displays982.4m,3 markers and31 rendered points; ETA/display ready. Blank tap0tiles, key entry2/300,20s idle16tiles. Route-stop data is reused from validation; failed map loads retry after30s.
