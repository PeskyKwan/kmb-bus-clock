# Third-party materials

The MIT license covers original project code only.

- **KMB public API**: route/stop metadata and ETA come from https://data.etabus.gov.hk/v1/transport/kmb/ . Cached stop names in docs/stops.json seed glyph generation; docs/routes.json seeds candidate route lookup, with live per-variant verification; they are not live ETA. KMB owns its data and marks; follow the provider's applicable usage conditions. This project is unofficial.
- **OpenStreetMap**: docs/map-roads.json and derived road graphics/coordinates contain © OpenStreetMap contributors data, available under the Open Database License (ODbL). Attribution: https://www.openstreetmap.org/copyright . The included Overpass snapshot reports2026-05-31. Keep attribution with derivative map materials.
- **LCDWIKI / QDtech**: board pin assignments and panel initialization were implemented using the model's public documentation. Vendor PDFs are linked in README, not redistributed. Product reference: https://www.lcdwiki.com/res/E32R28T-1/ .
- **Fonts**: public installer firmware contains raster glyphs derived from Noto Sans TC/Noto Sans, SIL Open Font License1.1; see fonts/*-OFL.txt and pinned fonts/sources.json. Maintainer-local original STHeiti/Arial glyphs are not part of the public installer.
- **Browser installer**: esptool-js0.6.1 Apache-2.0, SparkMD5 MIT, pako MIT/Zlib; bundled notices are in installer/licenses/.

- **Libraries/toolchain**: PlatformIO, Espressif Arduino, Adafruit GFX/ST7789, ArduinoJson, esptool and Python packages retain their own licenses and are obtained separately during setup. See their upstream packages.
- **TLS trust**: src/kmb_ca.h contains a public Hongkong Post Root CA3 certificate, not a private key. It was selected from the operating-system trust store after verifying the KMB endpoint's certificate chain.

- **Dynamic road geometry**: HK Bus Crawling@2021, https://github.com/hkbus/route-waypoints . The provider sources CSDI road lines; dataset branch verified2026-09-01 and README updated2026-08-01. Firmware downloads route geometry on demand with verified HTTPS. docs/map-sources.json maps route numbers/directions to TD IDs using the official2026-08-28 resource https://static.data.gov.hk/td/routes-fares-geojson/JSON_BUS.json . That official JSON contains stop points; it is not itself the line geometry. Upstream data terms apply separately from the code license.
