# KMB Bus Clock — approved UI is locked

Read `design/UI_LOCK.md` before changing this project. If the maintainer-local `_CONSOLIDATED_HANDOFF.md` exists, read it too; it is intentionally absent from public clones.

- Sum explicitly approved the physical-device UI on 2026-09-09 and instructed: "Pls lock in this UI design and layout".
- Preserve the approved composition, smooth fonts, sizing hierarchy, palette, tilted circular route plate, map appearance, and button placement when adding functionality.
- Do not replace the renderer with visually inferior fonts/primitives to simplify implementation. Adapt implementation to the approved design.
- Changing visual design/layout requires Sum's explicit approval. Routine functional changes that preserve the design do not require new approval.
- Selected route, destination, boarding stop, ETA, connectivity status and alert state are dynamic content; the approval does not freeze them to the photographed values.
- Use the approved device photo and source snapshot in `design/approved-2026-09-09/` as the visual reference. Firmware snapshot is a reference, not an instruction to overwrite newer functionality or saved settings.
- Preserve Wi-Fi credentials, saved route and touch calibration during firmware updates. Never commit credentials or raw device NVS backups.
