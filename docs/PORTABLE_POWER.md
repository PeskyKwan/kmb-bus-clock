# Portable power research — 2026-09-09

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
