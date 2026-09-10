# Multi-stop ETA positioning — feasibility study, 2026-09-10

Original feasibility study. The0.2.1 implementation now follows a bounded, conservative matching approach described below; this document does not constitute measured location-accuracy validation.

## Verdict

A multi-stop, temporally tracked ETA model is feasible on this board and is a better-supported design than the current fixed ten-minute progress animation. It cannot deliver measured vehicle location or guaranteed stop-passage detection from this feed. Actual accuracy improvement needs validation against observed journeys, not merely a more convincing animation.

## Verified evidence

- Current eta_animation.h uses1−remainingSeconds/600, clamped0–1. Thus position is not inferred from upstream stops, road length or traffic.
- Official API specifications dated2024-10-23 document route-eta/{route}/{service_type}: one response covers all stops. It can include other service types at shared stops; filter carefully by route/direction/service/destination and stop sequence.
- Live92 snapshots at16:00:29 and16:01:46 HKT on2026-09-10 were47,457 and46,247bytes,165/164rows. A2A snapshot was47,757bytes/183rows. Observed fields include route,dir,service_type,seq,eta_seq,eta,rmk_* anddata_timestamp. No vehicle/trip identity or vehicle coordinates are provided by these responses.
- At16:01:46,92 outbound origin's first ETA was16:20, while Pak Sha Wan(seq8)'s first ETA was16:07:54. Matching “first ETA” across stops would join different services or inconsistent forecasts. The origin's16:20 entry had shifted fromrank2 to rank1 between snapshots, while the target16:07:54 remainedrank1.
- In the first92 snapshot, ranks1–3 for stops1–8 were marked原定班次. The next snapshot had blank remarks for most approaching first entries while many ETA timestamps stayed exactly the same. A fresh response, new data timestamp or blank remark is not proof of newly measured position.
-2A included “Moving slowly” and past ETA entries. Do not equate disappearance, expiry, a remark change or a rank shift with confirmed physical stop passage.
- Small research excerpts are saved in research/eta-position-samples-2026-09-10.json. They are historical observations, not current arrivals or ground truth.

Primary references:
https://data.etabus.gov.hk/datagovhk/kmb_eta_api_specification.pdf
https://data.etabus.gov.hk/v1/transport/kmb/route-eta/92/1
https://data.etabus.gov.hk/v1/transport/kmb/route-eta/2A/1

Freshness: a bounded six-month search did not find adequate detailed official schema documentation, so this study uses the2024-10-23 specification within two years plus live data. An older data dictionary returned by the provider is excluded from the evidence basis; no older dictionary claims are required here.

## Proposed model

1. Keep the large boarding-stop ETA and alerts driven by the provider's target-stop forecast. Position inference must never silently rewrite that ETA or trigger a new location-based alarm.
2. Fetch a full-route ETA response around the current30s polling cadence initially. A single request reduces client-side cross-request skew, but does not guarantee all rows share an observation time. Check each row's data timestamp and limit skew.
3. Retain stop sequence and along-road distance with visible map stops. Include one time anchor before the3km boundary where available. Keep geographic north-up geometry and the current layout.
4. Seed from the earliest usable target arrival. Match plausible upstream forecast chains using ALL available ranks, compatible variants/destinations, forward travel order, plausible interval times and consistency with prior snapshots. Maintain competing candidates; do not hardcode that equal ranks identify the same bus or assume vehicles never overtake.
5. Retain a short history. Forecasts crossing the current time and upstream list changes can support a *likely* progress interval, but are not passage observations. Missed updates, cancelled services, schedule-only data and overtaking must lower confidence rather than force a match.
6. With a coherent chain, estimate an interval between stops and interpolate along actual road distance using estimated inter-stop time. Dwell/traffic uncertainty remains; don't simply divide target ETA by an assumed constant speed. Between API refreshes, smooth movement is still interpolation.
7. On major revisions/reassignment, fade and reacquire instead of drawing the old bus racing backwards. With insufficient evidence show a subdued/paused marker or hide it; do not secretly fall back to a solid “live” ten-minute animation. If the candidate is before the map boundary, indicate off-map rather than parking it on the first displayed stop.

Correlated ETA forecasts are not independent location sensors. Even perfect matching cannot uniquely invert future arrival predictions into current coordinates. No justified metre-error or “within one stop” guarantee can be made yet.

## Device and network cost

No AI model or new hardware is necessary. Use streaming per-row parsing and a small fixed retained set (for example up to8–12 ETA anchors ×3 forecasts), plus a short history. Do not load a full route into the current small ArduinoJson document or merely increase it until TLS/framebuffer memory fails. Bound bytes/time and preserve target-ETA fallback if a route response is too large.

At the observed47KB payload and two reads/minute, response bodies alone are about5.7MB/hour, excludingTLS and other traffic. Long routes/variants can cost more. Measure cadence and payloads; do not claim per-second telemetry or multiply separate HTTPS handshakes for every stop.

## Validation before replacing the animation

Build a replayable prototype first, retaining the old UI. Compare against the existing fixed600s baseline on quiet92, denser routes, bus bunching, dwell, original schedules, cancelled/null entries, rank changes and reconnection. Record independently observed stop-arrival/passage times when available; otherwise tests establish only consistency, not actual position accuracy.

Evaluate target-service switches, wrong-vehicle matches, inferred segment error where ground truth exists, coverage/confidence, update lag and false “passed stop” assertions. Confidence must be calibrated from these results, not invented as a percentage. Release only if data-backed improvement and honest low-confidence behavior are demonstrated.

UI wording can stay short: 估算. Website wording may become “位置按多站ETA推算”. The statement that this is not GPS remains true.
