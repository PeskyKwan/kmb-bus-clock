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
- Right-side compact north-up three-stop map, restrained labels and current-stop emphasis.
- Bottom full-width rounded reminder/dismiss button.
- Smooth antialiased Chinese/Latin fonts, consistent intentional sizing hierarchy; no return to1-bit jagged glyphs or mismatched generic text.
- Honest map/ETA status. Never fabricate a GPS bus position. Preserve existing distinction between sourced92 road map and other-route station-position schematics.

## Changes
Route/stop/ETA/status content must continue to update. Functional additions must fit the existing design or use a separate settings screen. Obtain explicit approval before changing appearance or layout. Compare proposed visual changes against the acceptance photo before flashing; preserve device preferences.

The photographed22-minute arrival is only the acceptance-time value, not a fixture or constant.
