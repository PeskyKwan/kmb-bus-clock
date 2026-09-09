# Three-stop maps: current behavior and planned fitting

The display's map panel is compact and fixed in the approved layout. The *geographic viewport* must adapt to the selected journey, not force every journey into the Pak Sha Wan example.

## Current beta after automatic-map implementation

- The illustrated 92/Pak Sha Wan section is a static raster made from OSM geometry and official KMB stop coordinates.
- Other selections now look up a candidate TD route ID, download CSDI-derived geometry through HK Bus WayPoints Crawling, match the three stops in travel order, include the intervening bends and fit at one uniform scale. North remains up; direction is route order.2A at Ngau Tau Kok has passed on-device.
- Names are stripped of stop codes and placed with collision checks. No matching source, a match farther than about150m, a source beyond2048 vertices, or missing network/data falls back to the explicitly labelled schematic. The fallback's original aspect limitations remain; the real-road path uses uniform scaling.
- Live mode now includes a user-requested **illustrative ETA-driven bus marker**. The last10minutes are normalized along the visible path; longer waits remain at the start. This is not a measured vehicle position, stop passage, speed, or travel-time model. The caption says ETA估算. Stale/null/offline data hides it; large forecast revisions reposition it. General road fitting remains unfinished.

## Design rules and remaining coverage tests

1. Select the boarding stop and up to two preceding stops from the chosen direction **and service variant**. At the start of a route, show only the stops that actually exist.
2. Obtain the route section connecting those stops, including bends between them. Include the section's geometry in the bounds, not just the three stop points; otherwise a U-turn or long curve can be clipped.
3. Project geographic coordinates consistently (Mercator or local metric projection). Keep north up. Do not stretch latitude/longitude independently to fill the rectangle.
4. Reserve an inset for station dots, bus/direction symbols, text and the north marker. Compute `scale = min(usableWidth / boundsWidth, usableHeight / boundsHeight)` and centre the result. One uniform scale preserves vertical, horizontal and diagonal orientation.
5. Fit once when the route/boarding stop changes; keep the viewport stable while ETA refreshes. Use minimum bounds for coincident or extremely close stops to avoid division by zero or excessive zoom.
6. Place station names with measured text bounds and alternate offsets/leader lines. If labels still overlap, use numbered station dots plus a compact label key rather than shrinking text until unreadable. A long-name/overlap layout needs visual review against the locked UI.
7. Use a direction arrow based on route order. U-shaped and crossing sections must follow the selected trip's sequence, not sort road points by latitude or imply that every journey runs downward.
8. Cache route geometry after selection. If geometry is unavailable, retain the explicit station-position schematic; never invent a road line or show the previous route's map under new labels.

## Cases to verify before release

Vertical, horizontal, both diagonal slopes, far-apart stops, very close/coincident stops, loops/U-turns, road sections extending outside stop-only bounds, long labels, first/second stop, opposite direction, express variant skipping stops, and loss of map data.

The UI panel, plate, fonts and controls remain unchanged by the fitting algorithm. The current marker remains visibly approximate. A more geographically meaningful position model requires separately validated data; a real basemap does not make the vehicle position real.
