# Job: heap-selfheal (KMB Bus Clock) — spec from Cowork, 2026-10-04 17:05 HKT

You are the builder for Sum's KMB Bus Clock (ESP32 E32R28T-1, personal firmware 0.3.2, saved route 92/O/service1).
Repo = current directory. Read first: `AGENTS.md`, `design/UI_LOCK.md`, top of `_CONSOLIDATED_HANDOFF.md`
(entries "2026-10-04 16:49", "2026-09-20" x2, and the "CURRENT BATON" pending-work bullet about auto-recovery).

## Problem (evidence already gathered by Cowork)
- 2026-10-04 ~16:48 the clock showed 更新失敗. Read-only serial state: connected, clock synced, KMB API fine from Mac,
  but `etaCode=-1`, `apiDiagnostic=-1`, `apiTlsDiagnostic=-32512` (MBEDTLS_ERR_SSL_ALLOC_FAILED), free heap ~114 KB,
  **largestHeap 45044**. After a controlled reset at 17:03 (esptool chip_id + hard_reset, no flash): etaCode 2,
  **largestHeap 49140**. So: heap fragmentation — enough total free memory but the biggest contiguous block becomes
  too small for the TLS handshake after long runtime. 09-20 incident (connect -1, TLS diag 0, largest ~45 KB) is
  probably the same root cause; check.
- Backoff is 30 s × 2^failures (max 8 min); no recovery logic beyond that.

## Sum's direction (he asked Cowork to choose; Cowork chose)
Sum prefers the clock to free/avoid wasted memory rather than restarting. Do it in this order:
**A (prevent)** → **B (soft recovery, no restart)** → **C (last-resort restart, rare, guarded)**.

### Phase 0 — measure first, prove the cause
- Audit runtime allocation churn on the ETA/position/road/native-settings paths: per-request `WiFiClientSecure`/
  `HTTPClient`, `String` building, `DynamicJsonDocument` sizes, position/road decode buffers, route-stop doc
  (a 16 KB doc before TLS caused -32512 once before), anything allocated and freed every poll while other
  long-lived allocations land in between.
- Add hidden telemetry only (serial/LAN state JSON, no on-screen change): uptime seconds, min-ever largest block,
  consecutive ETA failure count, last-success age, soft-recovery count, last restart reason.
- Write down which allocations you believe fragment the heap and why (evidence, not guesses).

### Phase A — stop fragmentation at the source
Pick the smallest change that the evidence supports. Candidates (your call, justify it): allocate the big
long-lived buffers once at boot and reuse; reuse fixed-size JSON documents; avoid per-poll String concatenation;
order allocations so long-lived ones don't sit between transient ones; a reserved contiguous block released just
before the TLS handshake and re-taken after `h.end()` (only if you can show nothing else grabs it meanwhile).
Do not reduce TLS security (keep CA validation).

### Phase B — soft recovery without restart
After e.g. 3 consecutive ETA failures with alloc/connect errors (or largest block below a measured threshold):
pause position/map/native network jobs, close clients, release releasable caches, optionally restart Wi-Fi STA
(keep saved credentials — see existing WIFI_STORAGE_RAM safeguard in handoff, do not break it), then retry soon
instead of waiting the full 8-minute backoff. Bounded attempts with cooldown.

### Phase C — last-resort restart
Only if B failed and ETA has been failing ≥ ~20–30 min: controlled `ESP.restart()`; at most once per 6 h;
boot-loop guard (e.g. RTC-memory counter, no frequent NVS writes); never touch saved route/Wi-Fi/calibration/settings.
Record the reason in telemetry.

## Tests / acceptance
1. All existing host suites PASS; add host tests for the B/C state machine (thresholds, cooldown, boot-loop guard).
2. PlatformIO E32 build PASS (personal original-font build for the device; keep personal and public OFL builds separate).
3. Hidden serial test hook to simulate TLS alloc failure so B and C can be exercised on the real board without
   waiting hours; prove B triggers, and C's guard works (simulate, don't loop-reboot the board).
4. Flash: app-only at 0x10000 with hash verify (standing permission), **never erase / never write NVS**.
   Post-flash: version bumped (e.g. 0.3.3), route 92/O/1, Wi-Fi, lead 90, brightness 190, orientation, theme,
   calibration unchanged; etaCode 2; real framebuffer capture shows locked UI unchanged.
5. Soak: sample LAN `http://<board-ip>/state` (board IP is now .50) every 60 s for ≥ 45 min inside this job;
   then leave a detached sampler (nohup) running 24 h, every 5 min, writing JSONL to the evidence folder
   (strip ssid/ip). Compare largestHeap trend vs old firmware (old: 49140 fresh → 45044 failing).
6. Evidence → `evidence/heap-selfheal-2026-10-04/` (states, soak logs, framebuffer PNG, binary SHA256, notes).

## Rules
- UI is LOCKED: no visual/layout/text change on screen. Telemetry is hidden only.
- Serial: POSIX no-reset reader only (`tests/device_position_audit.py` style); one serial owner at a time; close readers.
  `.venv/bin/esptool.py` shebang is broken (folder moved) → use `.venv/bin/python3 -m esptool`.
- Never commit credentials, NVS dumps, `docs/handoffs/`, `docs/plans/`.
- Commit locally on main with clear message. **Do NOT push** (repo is public), no gh-pages, no installer publish.
- Update `_CONSOLIDATED_HANDOFF.md` with a new top entry (under CURRENT BATON) describing what changed and evidence.
- If evidence shows the cause is NOT fragmentation, stop after Phase 0, do not flash, and explain.

## Reply
Reason in English if you like, but the final reply must be plain HK Cantonese for Sum (non-engineer), short, then:
RESULT: pass/fail · commit id · firmware version + SHA256 · soak summary · where the 24 h sampler writes · anything left open.
