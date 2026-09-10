# Position audit — 2026-09-10

0.2.6 adds the evidence-backed matcher fixes below and the user-authorized persistent primary illustration. Host tests and separate personal/OFL builds pass. App-only installation and device digest verification completed after USB recovery; physical regression results are recorded below.

## Updated user requirement: keep the primary bus visible

Sum explicitly superseded the earlier hide-on-uncertainty policy: use a rough guess or let the last position lag instead of disappearing; correct it when better information returns. The primary marker now starts from a fresh official ETA (scheduled included), using a nominal 20 km/h illustration clamped near the map entrance/target. After a supported match, missing or ambiguous data holds that last position, including temporary ETA/network failure. Matching recovery corrects it without a fade-out. A changed route resets it; the next arrival after the old one is due reseeds it. Before the first ETA/position, no location is invented. The optional second marker still needs its own supported chain.

This fallback is separate from the strict matcher. The existing估算 caption applies to every position; no GPS feed exists. The main ETA and alerts still use their original freshness rules. Tests cover guess, hold, recovery, no-blink correction, next-arrival reseeding and reset.

## Reproducible defects and fixes

1. **An exact official target was discarded as ambiguous.** The captured 1A inbound feed at 20:07:53 HKT contained target forecasts 87 seconds apart. Both fell within the old ±90-second lookup, so even a unique exact target erased all matching history. Prefer a unique exact target; retain the existing unique-only ±90-second fallback when no exact row exists. Duplicate exact targets, and exact scheduled/stale targets with a nearby live forecast, are rejected. Upstream ambiguity, ordering and freshness thresholds are unchanged.
2. **An expired, already traversed anchor hid a fresh downstream interval.** In the 20:12:27 HKT replay, 71 seconds later the old stop-30 anchor has expired, but stop 33 and the target still form a fresh bracket; the path's newest timestamp is only 82 seconds old. Validate the current bracketing pair rather than every earlier interval. The path's 90-second expiry, the current anchors' timestamp/ETA limits, and off-map bounds remain enforced.

`tests/eta_position_test.cpp` failed before fix 1. `tests/position_replay_test.cpp` failed before fix 2. Both pass after the fixes. The fixture preserves actual provider ETA/data timestamps as Unix seconds; it does not manufacture successive timestamp updates. Its exact-target scenario uses the route feed's target row, not a separately captured 1A stop-endpoint response.

## Evidence and complete pipeline review

| Area | Finding / validation |
| --- | --- |
| Initial physical state | Fresh no-reset 0.2.5 diagnostics: saved **2A inbound**, stop `BD06457129045F1D`, ETA code 2, route-position code 1, 30 forecast rows, both samples 0, animation count 0; road ready, 72 points, 9 map stops, 3000m + 450m continuation. This differs from the older handoff's saved 92. |
| Official target freshness | Direct target and route endpoints captured together at 20:02:38–39 HKT had matching target forecasts and provider timestamps. Later baseline snapshots show the departed live target disappearing and the earliest target becoming Scheduled Bus. The strict matcher correctly rejected that scheduled target; under the NEW user requirement the primary illustration now uses an ETA guess anyway. Main ETA selection/alerts are unchanged. |
| Route polling | Existing single worker schedules target/position requests at 30 seconds, with target priority and bounded network waits. Baseline proves successful route decoding; no polling change is justified. Position HTTP/TLS diagnostics are now separate from main ETA diagnostics. |
| Scheduled/live parsing | Existing `Scheduled` / `原定` filtering and route/direction/service/destination filters retained. Exact scheduled/stale targets now fail closed instead of borrowing a neighbouring live journey. |
| Target-chain matching | Fix 1 preserves exact source identity. Non-exact ambiguity and duplicate exact timestamps remain rejected. A frame with ambiguous upstream anchors still yields no strict match; the primary illustration now guesses or holds instead. |
| Two-bus ordering | Existing target separation >90s, ordered upstream separation >30s, and independent chain confirmation remain unchanged. The 1A replay at 20:12:52 (25 seconds after the captured frame, between polls) supports two distinct ordered on-map estimates. Crowded/ambiguous optional second journeys remain hidden. This is real-feed host replay, not proof of two sprites on the physical display. |
| Warm-up | Two distinct provider timestamps still required. Repeated cached responses do not advance confirmation. No reset of history merely because another target lies within 90s of an exact match. |
| History expiry | Fix 2 affects only intervals already traversed. Tests retain expired-current-anchor rejection, 90-second path expiry and removed-row history limits. |
| Map fractions | Reproduced the physical 2A road using production simplification/projection: 72 points, 10 ETA anchors (lead seq 7 through target 16), matching the physical 9 visible stops. 1A replay geometry has 67 points and 10 anchors (lead 25 through target 34). Negative lead fractions are allowed as evidence; off-map estimates stay hidden. |
| Approach / continuation | 174×140 map, up-to-3km approach and faded continuation untouched. Animation traverses `approachCount`, excluding continuation. Geometry regression passes. |
| Motion / redraw | Map-restore-then-composite-both-sprites is unchanged. Primary correction preserves opacity; a separate persistent source handles guess/hold. Strict secondary smoothing/fade is retained. |
| Heap / TLS | Baseline idle heap approximately 114–118KB, largest block about 45KB; successful ETA/position/map. CA verification unchanged. Diagnostics stream rows without a large JSON allocation and omit worker-owned frames while busy. Final static RAM 69,496 bytes, +24 bytes from 0.2.5. |
| Native route search / Wi-Fi | No native route-search, route-stop parser, Wi-Fi input, saved configuration or calibration implementation changed. Host suites pass. Post-flash Wi-Fi reconnect, saved2A restoration and native1A BOTH destinations reaching nonempty stop lists passed without Save. |
| USB | POSIX no-reset reader obtained four baseline samples, then failed with `Device not configured` at its next write. Thus pyserial/DTR alone does not explain the failure. Reader closed; no driver resets, flash writes or NVS access attempted. User confirmed physical reconnect; subsequent fresh reads and the full app-only write/digest verification succeeded. A later stale handle failed during route testing, but one fresh no-reset open recovered immediately; the draft was closed without saving. |

## Build / release gate

- Personal original-font candidate and public OFL candidate are built separately; the personal generated assets are restored locally after packaging.
- Host ETA, matching/replay, motion, decoder, route-stop, native input, Wi-Fi, daylight and map suites pass; installer artifact/NVS bounds and mocked flow checks pass.
- Personal persistent firmware installed app-only at `0x10000`; device digest verified. Saved 2A, Wi-Fi, road and official ETA restored; primary state is visible with source `eta-guess`.
- Physical framebuffer verification: 222 exact bus-body pixels inside the map, bounding box (173,90)–(188,111), with saved2A and a five-minute main ETA. Source `eta-guess` and strict `positionReady=false` demonstrate the new fallback. A later fresh physical read shows source `held`, strict match false, primary still visible. No synthetic feed was injected.
- Main ETA/map and native1A BOTH directions pass on the installed build. The route test now uses a fresh no-reset handle per short command and closes the unsaved draft with its own handle, avoiding stale idle handles. Host two-chain replay remains separate from physical two-sprite proof.
- Personal SHA256: `21c4b6fe90bcba1e1fabb8c49e8ba69056a95e8905044ac3f55a501c7d824788`. Public OFL SHA256: `080f52b6b7ab7e702c38e5d87d3e1ba62b84af39596ec03f9781db1c8769eee5`. Personal generated assets restored and the rebuilt personal binary is byte-identical to the installed candidate.

Primary live data sources: [2A target](https://data.etabus.gov.hk/v1/transport/kmb/eta/BD06457129045F1D/2A/1), [2A route](https://data.etabus.gov.hk/v1/transport/kmb/route-eta/2A/1), [1A route](https://data.etabus.gov.hk/v1/transport/kmb/route-eta/1A/1). These URLs are live and will no longer reproduce the capture-time values; the committed fixture preserves the replay evidence.
