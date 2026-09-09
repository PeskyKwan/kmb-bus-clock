# Third-party materials

The MIT license covers original project code only.

- **KMB public API**: route/stop metadata and ETA come from https://data.etabus.gov.hk/v1/transport/kmb/ . Cached stop names in docs/stops.json seed glyph generation; docs/routes.json seeds candidate route lookup, with live per-variant verification; they are not live ETA. KMB owns its data and marks; follow the provider's applicable usage conditions. This project is unofficial.
- **OpenStreetMap**: docs/map-roads.json and derived road graphics/coordinates contain © OpenStreetMap contributors data, available under the Open Database License (ODbL). Attribution: https://www.openstreetmap.org/copyright . The included Overpass snapshot reports2026-05-31. Keep attribution with derivative map materials.
- **LCDWIKI / QDtech**: board pin assignments and panel initialization were implemented using the model's public documentation. Vendor PDFs are linked in README, not redistributed. Product reference: https://www.lcdwiki.com/res/E32R28T-1/ .
- **Fonts**: no font files or generated font raster headers are distributed. Build scripts use locally supplied/licensed fonts. The maintainer's physical reference used installed macOS STHeiti Medium and Arial Bold; their rights are not covered by MIT. Rendering with another font is supported through environment variables but requires visual review.
- **Libraries/toolchain**: PlatformIO, Espressif Arduino, Adafruit GFX/ST7789, ArduinoJson, esptool and Python packages retain their own licenses and are obtained separately during setup. See their upstream packages.
- **TLS trust**: src/kmb_ca.h contains a public Hongkong Post Root CA3 certificate, not a private key. It was selected from the operating-system trust store after verifying the KMB endpoint's certificate chain.
