# Appearance and route entry

Day mode uses a slightly warmer cream background; this is software styling, not measured panel calibration. Photo exposure, viewing angle and the physical panel may still affect perceived colour. Night mode retains the red/cream stop plate and uses dark green surfaces with a muted blue map corresponding to the pale blue day map. The tilted plate is enlarged by7.1%; station text matches the12px heading, with an ellipsis for names that still exceed the safe circular width.

The Hong Kong HH:MM clock sits at the top of the right column above the destination, next to Settings and updates each minute. Until NTP synchronizes after power-on it shows --:--. No battery-backed real-time clock is assumed.

Settings first separates Bus and reminders from Device settings. Device settings → Display mode offers 自動 / 日間 / 夜間. Brightness and Wi-Fi have their own entries. Mode selection saves immediately; Back to main preserves it. Brightness/route edits retain their explicit Save workflow. Auto defaults on and uses the selected boarding-stop latitude/longitude and date, approximate solar declination/equation of time with the standard horizon correction. Switches at calculated sunrise/sunset, not a fixed evening hour. No weather request, GPS tracking, or walking-time calculation. Offline after time sync continues using the ESP32 system clock. Accuracy is within a few minutes, not an astronomical instrument.

Numeric keys keep their positions. ABC now lists only valid next letters from the bundled KMB/LWB route catalog for the current prefix (92 → R). Delete/clear always remain available. Route/variant selection is still checked against the live KMB API. Newly introduced letters/routes require a refreshed catalog or entry through the existing phone/Mac form; this is not a full live keyboard index.

Research checked 2026-09-09: official APP1933 Google Play listing, Updated Jul21 2026, explicitly describes an intelligent route-input keyboard. It does not document the full filtering algorithm, so our catalog-prefix implementation is an adaptation, not a claimed exact reproduction. https://play.google.com/store/apps/details?hl=en&id=com.kmb.app1933

Solar verification: bounded six-month search did not yield dated primary ephemeris documentation; expanded to the 2026 HKO Almanac (published within two years). September9 reference06:08/18:33; local approximation06:08/18:34. https://www.hko.gov.hk/en/gts/astron2026/files/HKO_almanac_2026.pdf

The map uses a faint decorative grid and one red arrow following the road segment in travel order. On a crowded map the arrow uses the caption margin with the final approach direction. A single-point route origin has no inferred direction arrow. The settings mark is a filled toothed ring with a centre hole; it is not a sun icon.
