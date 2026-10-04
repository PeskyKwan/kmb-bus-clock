# Heap self-heal — evidence notes (2026-10-04)

Board: E32R28T-1, personal original-font build, route 92/O/service1. All serial access was POSIX no-reset; network identity (SSID/IP) stripped from every saved record.

## Phase 0 — cause

### Toolchain facts
- Arduino-ESP32 2.0.17 / ESP-IDF 4.4 / mbedTLS 2.28.7. `CONFIG_MBEDTLS_SSL_MAX_CONTENT_LEN=16384`, asymmetric/variable buffers off → every HTTPS session callocs **two 16,717-byte record buffers** (`mbedtls_ssl_setup`, after TCP connect).
- `ESP.getMaxAllocHeap()` (`largestHeap`) = `heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL)`, which **includes the 32-bit-only IRAM region**. On this firmware that region is 46,248 bytes with a 45,044-byte free block (`largestIram` = 45044, measured). mbedTLS cannot use it (8-bit calloc).

### Measured on the old allocation pattern (probe build `0.3.3-p0` = 0.3.2 + hidden telemetry only; `probe/`)
- Byte heap (`heap8`, MALLOC_CAP_8BIT|INTERNAL) idle: 69.4–71.8 KB free, largest block **49,140** — the only big free block, in region 0x3ffe4350 (all other regions ~full; see `probe/probe-states.jsonl` region dumps).
- Just before ETA TLS: largest **40,948 = 49,140 − 8,192** — the 8 KB ETA JSON document is carved from that same block before the handshake.
- During TLS: byte heap free fell to **8.9 KB**, largest 4.6–7.9 KB; low-water since boot 6,240 bytes.
- `largestHeap` read **45044 whenever the byte heap was busy** (6 samples, e.g. heap8 9,600 / largest8 7,924 / largestHeap 45,044). So the 16:48 incident value 45044 was the IRAM block: the real usable largest block was ≤45 KB and unknown, not "45 KB available".

### Conclusion
Fragmentation, not a leak: total free stayed ~114 KB, but one handshake needs 8 KB + 2×16.7 KB (+ handshake allocations) from a single ~49 KB block. Any long-lived allocation landing inside that block (lwIP/Wi-Fi/runtime objects allocated while a session holds the block) leaves pieces too small → `MBEDTLS_ERR_SSL_ALLOC_FAILED` (-32512), matching the incident (`apiDiagnostic -1`, `apiTlsDiagnostic -32512`). A restart restores the 49 KB block, matching the 17:03 recovery.

09-20 incident (`-1`, TLS diag 0, largestHeap ~45 KB): TLS diag 0 means failure before mbedTLS (socket/DNS/TCP connect). Its 45 KB reading is consistent with a fragmented byte heap (likely the IRAM block again) but the old telemetry cannot prove it; it is classified as a device-side transport failure and is covered by the same soft recovery.

### Allocation audit (per request)
| Path | Allocation | Note |
|---|---|---|
| every HTTPS | 2 × 16,717 B mbedTLS record buffers | needs 2 large contiguous blocks — **root cause** |
| ETA (every 30 s) | `DynamicJsonDocument(8192)` before TLS | takes 8 KB of the same big block; live 3-row response uses 1,146 B |
| every HTTPS | `WiFiClientSecure`→`sslclient_context`, CA parse, peer chain, lwIP PCB, Wi-Fi RX buffers (1,618 B) | small; best-fit into holes |
| position (30 s) | `meta(768)`, `row(1536)`, prefix 512 after TLS | small |
| road (once/route) | `std::vector<GeoPoint>` growth during TLS stream | boot peak |
| loop, every 7 s | `DynamicJsonDocument(2048)` state + Strings | ran concurrently with worker TLS |

## Phase A — prevention (final 0.3.3)
1. `src/tls_pool.h`: the two record buffers (16,896 B slabs) are reserved once at boot (retried when idle) and lent through `mbedtls_platform_set_calloc_free`; other sizes use the identical heap_caps fallback. CA validation unchanged.
2. ETA document 8 KB → 4 KB (3.6× measured need).
3. State JSON (USB + LAN) streamed by `src/json_out.h` — no heap document every 7 s.

Rejected candidate (kept in `superseded/candidate-a-static-docs-panic.jsonl`): static 8 KB ETA + 3 KB state documents on top of the slabs. It turned transient memory into permanent memory; boot-time road download then exhausted the byte heap (low-water 1,136 B, 11 failed 1,618-byte Wi-Fi RX allocations, one **panic reboot**). Reverted before any further testing.

## Phase B/C — recovery
`src/heal_policy.h` (host-tested) + `src/self_heal.inc`: 3 consecutive device-side ETA failures (HTTP/TLS client error or JSON NoMemory) → soft recovery (90 s pause of position/map/native jobs, drop route-stop scratch list, prompt retry); attempts 2–3 also restart Wi-Fi STA with saved credentials; 3-min cooldown; ≤3 per episode. Restart only when soft is exhausted, its cooldown passed, failure ≥25 min, clock synced and ≥6 h since the last guarded restart (RTC_NOINIT record with checksum; no NVS writes).

## Results (final binary)
- Personal 0.3.3: 2,335,264 bytes, SHA256 `3423e1ecee027dfb135ed99830f29ccd2b4e85e5ace92c1b9a04fc73c4ab1221` (`candidate/`, not committed: *.bin ignored). Static RAM 69,736 (0.3.2: 69,536). App-only 0x10000 write, `Hash of data verified` (`flash-log.txt`). NVS untouched.
- Post-flash (`post-flash-serial.jsonl`): version 0.3.3, route 92/O/1, stop 5089C69E080B7A43, lead 90, brightness 190, theme 0, normal orientation, armed false, connected, ETA code 2, road ready, two buses, displayReady. Idle byte heap 40.4 KB / largest 16.4 KB, low-water **10,504** (old 6,240), failed allocs 0, slab hits 14 / misses 0, ETA doc usage 1,146 B.
- On-board self-heal (`selfheal-*.jsonl`, `tests/device_selfheal_smoke.py`): soft PASS; dry run soft → soft+STA ×2 (real STA disconnect/got-IP counters) → would-restart at 25 min → second blocked by 6 h guard PASS; one real guarded restart (`rst:0xc SW_CPU_RESET`, `restartReason=self-heal`), ETA healthy after reboot, second restart blocked, test record cleared PASS.
- Framebuffer (`framebuffer/physical-framebuffer.png`): locked layout unchanged; two bus illustrations.
- Soak (`soak-45min.jsonl`): 45-min LAN soak 17:44–18:30 HKT, 60 s × 47 samples, 0 sample errors: ETA code 2 in 46, one KMB stale-data `-3` (server timestamp, not device; streak stayed 0); idle largest byte block flat at 18,420 B the whole run (no downward drift); byte heap free 24.4–35.2 KB; low-water 4,716 B set during boot road download and never lowered; failed allocs 0; TLS slab hits 370 / misses 0; no soft recovery or restart during soak. Old firmware comparison: old `largestHeap` 49140 fresh → 45044 failing; on 0.3.3 that metric is pinned at the IRAM block (45044) so the meaningful trend is `largest8`, flat at 18,420 with TLS no longer needing large blocks.
- 24 h detached sampler: `soak-24h.jsonl`, every 5 min until ~2026-10-05 17:44 HKT (started after final flash; still running at commit time).
- Board boot during soak was the test restart at 17:43 HKT, so `restartReason` reads `self-heal` (test), not an outage.
