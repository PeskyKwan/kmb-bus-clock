# Battery shortlist and independent internet — 2026-09-10

Current target: a few hours of portable use, acceptable thin case, no dependence on a phone hotspot if avoidable. No purchase or seller contact has been made.

## Battery shortlist

Recommendation for the first compact sample: SCCCLTD503450, because the listing explicitly includes PCM protection and the50×34×5mm body is smaller than the board footprint. This is a candidate, not a confirmed plug-in accessory.

Live public Shopify data checked2026-09-10: all three below marked available=true and currencyHKD. All were published2025-10-21. These are ONLINE availability signals only, not a branch-stock confirmation. Prices exclude unverified shipping/connector work.

| Listing | Approximate size L×W×T | Stated capacity | Price | Assessment |
|---|---|---|---|---|
| [503450](https://scccltd.com/products/503450-lipo-battery-3-7v-1000mah) |50×34×5mm|1000mAh|HK$20|PCM explicitly listed; smallest preferred trial candidate; bare wire leads|
| [505060](https://scccltd.com/products/3-7v-2000mah-rechargeable-lipo-battery-505060)|60×50×5mm|2000mAh|HK$20|More runtime at same thickness; protection NOT stated, confirm protected pack before choosing; bare leads|
| [606090](https://scccltd.com/products/3-7v-4000mah-606090-li-po-rechargeable-battery)|93×60×6mm including PCM|4000mAh|HK$35|Protection stated, but exceeds50×86mm board footprint and needs larger case; unnecessary for a couple hours|

Capacity/protection/dimensions are seller claims, not independent tests. Do not infer protection on505060 from the other listings. Cell thickness is not finished-device thickness: insulation, case walls, strain relief and appropriate clearance are additional. All need the correct board BAT connector and polarity checked; none is a ready-to-plug product. Board charging supports nominal3.7V polymer lithium; reject7.4V packs/high-voltage variants.

Runtime MODEL for the current Wi-Fi clock (not an added cellular modem), not a measurement: nominal3.7V×capacity×0.8 effective-energy allowance, divided by an assumed1.0–1.5W whole-device average. This gives about2–3h for1000mAh and4–6h for2000mAh. Neither assumed load nor usable energy fraction has been measured, so do not guarantee a couple hours. Verify actual runtime before finalizing the case or relying on an outing.

Discovery covered localSCCCLTD/Price listings, overseasAdafruit/Pimoroni, manufacturerMOTOMA/Vats/HNF and generic pouch sizes. The2025/26 MOTOMA catalog independently lists comparable5mm cells (503450900mAh,5037591350mAh); it does NOT certify the SCCCLTD packs. No recent independent test of these exact retail cells found. Older/undated/unprotected/dimension-mismatched products were not promoted as confirmed alternatives. This is a bounded shortlist, not an exhaustive quality ranking.

Source recency: six-month search did not provide enough dated retail battery evidence; expanded to two years. Public product published_at timestamps establish2025-10-21; updated_at is NOT treated as a specification-update date. Online price/availability were checked live. Exact board manufacturer spec2025-04-22 remains the electrical reference.

## Internet

The actual E32R28T-1 has Wi-Fi/Bluetooth and a microSD/TF slot, NOT a SIM slot or cellular modem. A SIM card alone cannot add mobile data. Current firmware polls over Wi-Fi; no modem support implemented.

The previous phone-hotspot recommendation prioritized minimum extra hardware. Sum now prioritizes independence from unreliable phone tethering, so the least invasive alternative is a dedicated SIM-equipped mobile Wi-Fi router. It still depends on coverage, its battery and power-saving settings; no promise of uninterrupted service.

| Method | Tradeoff |
|---|---|
| Phone hotspot | No extra router/SIM if plan permits; must be enabled and discoverable; ESP32 cannot activate Apple's Instant Hotspot. Auto-reconnect cannot revive a disabled phone hotspot. |
| Dedicated mobile Wi-Fi + dataSIM | Recommended alternative when phone dependence is unacceptable; board remains unchanged, but one more device/battery and data service. TP-Link HKM7350 is an example, not a market-wide best-buy or branch-stock recommendation. |
| AddLTE modem / redesign with cellular ESP32 | Potential one-piece future product, but new hardware, antenna, power budget, wiring and firmware; not a SIM upgrade to this existing board. |

A hotspot is not guaranteed to disconnect after a fixed duration. No specific cause of Sum's previous disconnects has been diagnosed. Apple guidance supports normal hotspot setup/troubleshooting, not an always-on guarantee. Offline cache remains useful for route/map display but cannot supply fresh ETA.

References checked:
- [Apple hotspot guide,2026-05-07](https://support.apple.com/en-la/111785)
- [TP-Link HKM7350 official listing,published2025-01-22 verified via public productJSON](https://hk.store.tp-link.com/products/tplink-m7350-150mbps-mobile-wifi)
- [Exact board specification,2025-04-22](https://www.lcdwiki.com/res/E32R28T-1/E32R28T-1_E32N28T-1_Specification_V1.0.pdf)
- [MOTOMA V25A manufacturer catalog](https://pub-mediabox-storage.rxweb-prd.com/exhibitor/document/exh-eb14bc36-7aeb-40fa-b253-4c64e4d75d81/7fa8cff1-c1a0-4a47-af7a-c14c7294a066.pdf)

---
Earlier planning, superseded by the shortlist/requirements above:

# Portable plan — 2026-09-10

This supersedes yesterday's home-only/deferred requirement. Sum now wants to carry the clock. Constraints: compact thin enclosure, minimal loose cable, useful live ETA; runtime and budget are not newly fixed. Earlier half-day target is a sizing goal to measure, not a promise.

## Recommended path

Final form: protected single-cell3.7V lithium-polymer pouch inside an insulated case, plus the phone's2.4GHz Wi-Fi hotspot. First use an existing USB power bank to measure actual screen/Wi-Fi/ETA power over a typical trip; then choose capacity and case dimensions. A2000–3000mAh range is only a planning candidate. No pack is certified as plug-compatible or ordered.

| Option | Best use | Remaining checks |
|---|---|---|
| Protected1S pouch, internal | Best compact one-piece final device | Exact dimensions, matching BAT connector AND polarity, protection, charge behavior and measured runtime |
| Nitecore NB Air5000 | Light external test/travel alternative; manufacturer lists89g | Needs short cable; exact board low-load/C-to-C behavior and local stock not verified |
| Nitecore Pocket5 | External alternative with built-in lead | Larger total package; board behavior and local stock not verified |

Category discovery also covered Xiaomi/Anker slim banks and Adafruit/Pimoroni protected pouches. Undated cell listings and connector mismatches prevented an exact current compatible-cell recommendation. This is architecture/candidate research, not a stock-verified purchase list. No shop trip or purchase recommendation.

The exact-board manufacturer document dated2025-04-22 specifies3.7V polymer lithium battery, BAT1.25mm2-pin connection and about290mA charging. Connector pitch alone is insufficient: verify the housing and positive/negative pins before attaching a protected prewired pack. Do not wire5V to BAT. Capacity/rated charge current means a3000mAh pack could need more than10hours before charge taper; no charging or runtime test performed yet.

## Internet and offline behavior

- Phone hotspot is the simplest route to fresh ETA; no extra SIM/router required. On iPhone enable Allow Others to Join and, for this2.4GHz board, Maximize Compatibility; remain on the hotspot screen for initial connection. The clock cannot invoke Apple's Instant Hotspot or switch the phone's hotspot on itself.
- A separate mobile Wi-Fi router is an alternative only if phone independence is needed; it adds another device, data plan and battery.
- Offline-only downloads cannot predict fresh bus arrivals. Current firmware embeds route/stop indexes and saves the selected route, but downloaded road geometry is held in RAM and needs fetching again after a reboot. It already hides stale ETA-based bus animation.
- Proposed next portable software work (NOT implemented in this UI fix): two remembered Wi-Fi profiles (home/hotspot); bounded fallback/reconnect; persistent cache for selected route geometry/labels; offline map visible with ETA shown as unavailable. Use last-update age, never animate old ETA as if live.
- Current firmware still saves one Wi-Fi network. Configuring a hotspot replaces that profile; do not claim home/hotspot auto-switching exists yet. No credentials collected or changed for this research.

## Sources / verification

Checked2026-09-10. Six-month search was sufficient for hotspot guidance and a power-bank review, but insufficient for exact-board battery specifications; expanded hardware evidence to within two years. Cached manufacturer specification verified locally after today's web fetch timed out. No evidence older than two years used.
- Manufacturer spec,2025-04-22: https://www.lcdwiki.com/res/E32R28T-1/E32R28T-1_E32N28T-1_Specification_V1.0.pdf
- Manufacturer2026 candidate guide: https://www.nitecore.com/article/541
- Independent NB Air review,2026-04-10 (portability evidence, not board electrical compatibility): https://www.cleverhiker.com/backpacking/nitecore-nb-air-power-bank-review/
- Apple hotspot guide,2026-05-07: https://support.apple.com/en-la/111785

---
Historical research follows; its home-only status is superseded above.

# Portable power research — 2026-09-09

**Current preference supersedes the earlier target below:** primarily home use, avoiding a permanently connected USB cable. Battery work is deferred; thin travel enclosure/12h is not an active requirement.

Target agreed with Sum: thin integrated enclosure and about12hours without USB power. Battery power does not provide internet: live ETA still requires Wi-Fi/a phone hotspot.

## Recommendation

Prefer a protected single-cell3.7V lithium-polymer pouch pack inside an insulated enclosure. Size capacity from measured average current before selecting a cell; roughly2500–3000mAh is a planning candidate, **not a12h promise**. No exact plug-and-play cell or Hong Kong stock has been verified. Do not order from these notes alone.

The exact-board specification (2025-04-22) specifies a1.25mm2-pin BAT connection and about290mA charge current. Use BAT, not the speaker socket. A matching pitch does not establish connector housing or polarity. Require a prewired protected pack matched to the board's BAT+/GND arrangement; verify electrically before connecting. Do not use loose unprotected cells or assume red/black wire placement guarantees compatibility. At3000mAh, capacity divided by290mA already exceeds10hours before charge taper, so overnight recharge time is a tradeoff.

Source: https://www.lcdwiki.com/res/E32R28T-1/E32R28T-1_E32N28T-1_Specification_V1.0.pdf

## Compared approaches

| Candidate | Benefit | Limitation / verification |
|---|---|---|
| Integrated protected1S pouch pack | One compact device, no external cable while carried | Exact capacity, dimensions, protection, housing, pin polarity and seller availability unresolved; runtime measurement required |
| Nitecore NB Air5000 | External lightweight alternative; manufacturer2026 guide lists89g | Requires short USB-C cable; larger total package than an integrated cell; USB-C-to-C/low-load compatibility and local stock not tested |
| Nitecore Pocket5 | Built-in USB-C lead avoids carrying a separate lead; manufacturer2026 guide lists125g and9mm thickness | Still an external unit; not a slim in-case battery; board compatibility and local stock unverified |

Discovery also covered Xiaomi, GP, inno3C, Momax, UNICO and specialist pouch-cell suppliers. Many product pages were undated, cells had mismatched connectors, or stock could not be established. These were not promoted into a verified buying recommendation. No physical-store trip or purchase is recommended yet.

Manufacturer2026 comparison: https://www.nitecore.com/article/541
Independent portability evidence (not proof of ESP32 electrical compatibility): CleverHiker review published2026-04-10, https://www.cleverhiker.com/backpacking/nitecore-nb-air-power-bank-review/ ; Pack Hacker updated2026-07-28, https://www.packhacker.com/travel-gear/nitecore/nb-air-usb-c-5000mah-power-bank/

Research began with six months; the exact board/complete power specifications required extending to two years. No material older than two years was relied on. Current retailer listings were insufficient to certify an exact in-stock compatible integrated pack.

Next: measure typical/peak power at the chosen brightness with Wi-Fi+ETA+animation, budget reserve for12hours, then fit an insulated protected pack and verify connector polarity. No battery hardware has been installed or tested.
