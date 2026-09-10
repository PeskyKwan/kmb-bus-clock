# Two persistent illustrated buses —0.2.7

Sum asked to see two little buses after the primary-only persistent fallback in 0.2.6. Fresh device evidence on the user-selected 1A showed two matched markers at 20:42 HKT but only one at 20:46 while the second chain warmed up again. This release extends the authorized approximate/held behavior to the second arrival.

- The existing official target ETA response supplies the earliest distinct following arrival, with the same route/direction/service/stop and timestamp checks. Scheduled forecasts can seed an illustration. The main ETA and alert selection are unchanged; no extra requests or large buffers are added.
- Each bus can use an ETA guess, a supported matched estimate, or a held position. The strict matcher remains unchanged. A matched second estimate must refer to the same target ETA as the following official forecast; otherwise its illustration guesses/holds.
- When the first arrival is due and the following journey becomes first, promote its saved illustration and reseed the second from the next supplied forecast. Do not leave a duplicate of the promoted journey. No second bus is invented if none has been supplied.
- If both 22×26 sprites overlap, offset their artwork by 12px either side of a bounded midpoint; their underlying route fractions are unchanged. Both sprites remain inside the locked 174×140 map. This also prevents two distant forecasts at the map entrance from covering one another.
- The existing估算 caption remains. These are illustrations, not GPS positions; the main ETA/alert freshness rules remain authoritative.

`tests/two_bus_illustration_test.cpp` covers second forecast selection, duplicates/stale/out-of-range rows, two guesses, holding, recovery without blinking, arrival promotion, no invented second, reset, and exhaustive sampled overlap/boundary cases. All 11 host C++ suites and firmware build pass. The physical smoke test now checks distinct connected bus-body pixel components, not only `animationCount`.

USB screenshot transfer now also supports lossless per-row RLE (`settings/capture-rle`). This keeps the framebuffer coherent within one command, uses no snapshot allocation, and reduces serial traffic; the old capture command and Wi-Fi-screen protection are retained. The smoke decoder requires every complete row and counts disconnected bus bodies.

Public publication is not authorized yet. Personal-font device firmware and public OFL artifacts remain separate. Final personal firmware installed at app-only0x10000 and device digest verified; NVS/Wi-Fi/route/calibration retained. The real1A framebuffer at20:58 HKT contains TWO complete bus bodies (444 exact body pixels total), separate boxes(195,92)–(210,113) and(198,129)–(213,150). Both position sources wereeta-guess, with both strict positionReady flagsfalse: this verifies the requested fallback on the physical renderer, without injecting a fake feed.

Personal SHA256: `7232b8eadb4e18ace2ad1725510c24532fc9035a47fb1e4c6b57ce083329d495`. Public OFL SHA256: `aefe979827c9ac054b3bec38bc27a9703a1e11fb8e82de4d83a231d2ae1f88db`. Personal assets restored and rebuilt firmware is byte-identical. Static RAM69,520 bytes; flash2,317,445 bytes.
