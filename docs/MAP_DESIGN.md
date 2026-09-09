# Route map window

## Approved behavior

From the selected boarding stop, follow the incoming route backwards for **up to3km of road distance**. This is not a3km straight-line radius and is not a fixed number of stations. If the route begins closer than3km, show the available section from its origin; do not invent a preceding road.

Keep north at the top and preserve real east/west/north/south orientation, diagonals and bends. North-up does **not** mean the bus moves north. The selected stop can naturally sit on any side of the panel. Use one scale for both axes and include all intervening bends in the bounds.

All matched upstream stops within the section are dots (64-marker safety capacity). Up to3 key names are labelled to keep the small display legible. No preparation-time, walking-time or departure-planning calculation is part of this feature. Existing reminder settings remain separate.

## Implementation

- Candidate TD IDs come from the dated route index; CSDI-derived road geometry is downloaded through HK Bus WayPoints Crawling over verified HTTPS.
- The3 stored nearby stops remain **matching anchors only**, locating the boarding stop on the correct route passage. They do not limit the visible window.
- Validated KMB route-stop order is reused for upstream markers. Stop coordinates/name offsets use the local catalog. Unknown new stop metadata may require a catalog rebuild.
- A backward walk along the polyline finds the3km boundary and interpolates the exact cut within a segment. Source coordinates/rounding mean physical distance remains approximate.
- Streaming simplification checks all pending intermediate points against an approximate2m deviation and preserves vertices no farther than80m apart. This accommodates densely sampled roads without filling ESP32 memory.
- Limits:512KB downloaded geometry,2048 retained geographic vertices,256 rendered points,64 station dots. Unsupported/missing/mismatched data shows an unavailable map and retries after30seconds; it does not silently revert to a fixed3-stop diagram.
- The previous92 static-map exception has been removed. All routes use the same rule.

## Animation is separate

The bus marker is still a labelled ETA countdown illustration. The final10minutes map to the visible path; it is **not GPS, measured speed or proof of stop passage**. Changing the map window does not change the reminder or make the marker's position factual.

## Verification and remaining coverage

Host tests cover vertical, horizontal, reversed horizontal, diagonal, U-shaped, near-origin and exact-distance clipping, plus simplification and tile-change checks. Actual source fixtures show about982m available for2A at Ngau Tau Kok and3000m for92 at Pak Sha Wan. On-device2A loaded the near-origin section with3 markers; display/ETA and partial-refresh tests passed.

Broader variants, unusual loops/repeated passages, very dense station clusters, long names, temporary diversions, missing source data and routes beyond the safety limits remain beta coverage areas.
