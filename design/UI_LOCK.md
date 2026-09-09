# Approved UI — locked 2026-09-09

Sum approved the actual device photo, after restoration of the smooth renderer and successful live ETA integration. This supersedes earlier mockups and the rejected jagged-font live renderer.

## Canonical reference (maintainer-local archive)
- `approved-2026-09-09/approved-device-photo.jpg` — user-supplied physical acceptance photo.
- `approved-2026-09-09/source/` — corresponding renderer, assets, build settings and asset generators.
- `approved-2026-09-09/approved-firmware.bin` — corresponding built application (not a full-device/NVS backup).
- `approved-2026-09-09/sha256.json` — source and firmware checksums.

The acceptance archive above stays local and is intentionally excluded from the public source repository. Public contributors should use the invariants below and the current renderer/assets generators as the baseline. The local archive may be absent in a fresh clone.

## Preserve
- Landscape 320×240 composition and warm cream / dark green / red palette.
- Upper-left circular red bus-stop plate, tilted left about3degrees, white route-number panel with centred number and station below; no pole.
- Large smooth dark ETA digits under the plate; smaller minutes label.
- Destination along the top, separate dark Settings button at upper-right.
- Right-side compact north-up route map, restrained labels and current-stop emphasis.
- Bottom full-width rounded reminder/dismiss button.
- Smooth antialiased Chinese/Latin fonts, consistent intentional sizing hierarchy; no return to1-bit jagged glyphs or mismatched generic text.
- Honest map/ETA status. Never fabricate a GPS bus position. Preserve existing distinction between sourced92 road map and other-route station-position schematics.

## Changes
Route/stop/ETA/status content must continue to update. Functional additions must fit the existing design or use a separate settings screen. Obtain explicit approval before changing appearance or layout. Compare proposed visual changes against the acceptance photo before flashing; preserve device preferences.

The photographed22-minute arrival is only the acceptance-time value, not a fixture or constant.

## Explicitly approved addition — 2026-09-09

Sum requested restoring the cute bus animation in live mode using ETA estimation. A small marker and the ETA估算 caption are authorized additions; the rest of the approved layout/fonts/plate/map remain locked. Marker movement is illustrative, not measured bus location.

## Approved native settings — 2026-09-09

Sum approved the four-entry settings preview and a numeric/letter route keypad. Native settings may add these subpages without altering the locked main display. Only explicit Back exits; changes are staged until Save.

## Approved route-distance window — 2026-09-09

Sum approved replacing fixed three-stop framing with up to3km measured backwards along the incoming route from the boarding stop. Preserve true orientation; the boarding stop may lie on any side. Show along-route stop markers without a fixed station count; keep name labels selective for readability. Do not add preparation/walking-time calculations. This supersedes the earlier fixed92/three-stop map exception.

## Approved appearance additions — 2026-09-09
Sum requested a warmer cream background, restored Hong Kong clock, compact route-aware letter keys, and sunset-based automatic dark mode. Keep existing plate, ETA and geographic map composition. Clock fits below ETA units; Appearance and brightness settings provide Auto/Day/Night with draft preview and explicit Save.

## Approved layout refinement — 2026-09-09
Sum requested two top-level settings categories (Bus/reminders and Device), explicit Auto/Fixed day/Fixed night modes, a muted blue night map matching a pale blue day map, clock removed from the ETA area, station text matching the top plate heading, and a5–10% larger plate. Implemented120px plate from112px (+7.1%),12px station text with safe-width ellipsis, clock above destination in right header. ETA and minutes stand alone. Supersedes below-ETA clock placement.

## Approved simplicity correction — 2026-09-10
Mode taps immediately persist; use自動/日間/深色, integer brightness and concise labels throughout. Bespoke drawn back arrow/settings mark replace those labels. Preserve map174×140, top clock and primary ETA.
