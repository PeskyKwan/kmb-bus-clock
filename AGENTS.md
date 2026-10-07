# KMB Bus Clock — approved UI is locked

## SHARED CHANGE LOG — every thread / agent (Sum, 2026-10-05)
- Threads can't read each other's chats; only files are shared. At the start of work read the last 30 lines of
  `/Volumes/Sum-AI-Projects/Codex-Claude Workspaces/_SHARED/SHARED_CHANGELOG.md` and `_SHARED/MACHINE_LOG.md`.
- The moment you change anything shared — any Mac / NAS / router (OS update, reboot, settings, permissions, network, Wi-Fi),
  Hermes agents / gateways / cron, logins / credentials, shared scripts — append ONE line to SHARED_CHANGELOG.md
  (`YYYY-MM-DD HH:MM HKT | who | machine/system | what changed | details file`). No line = the change didn't happen.
- Write only inside your own project folder and `_SHARED`. Never edit another project's files; read them only when needed.

## SESSION CLOSE — commit + push (Sum, 2026-10-06)
- At the end of every session, commit and push this project's changes to its private/own origin.
- Never commit or push passwords, tokens, `.env` files or keys — check the staged diff first.

Read `design/UI_LOCK.md` before changing this project. If the maintainer-local `_CONSOLIDATED_HANDOFF.md` exists, read it too; it is intentionally absent from public clones.

- Sum explicitly approved the physical-device UI on 2026-09-09 and instructed: "Pls lock in this UI design and layout".
- Preserve the approved composition, smooth fonts, sizing hierarchy, palette, tilted circular route plate, map appearance, and button placement when adding functionality.
- Do not replace the renderer with visually inferior fonts/primitives to simplify implementation. Adapt implementation to the approved design.
- Changing visual design/layout requires Sum's explicit approval. Routine functional changes that preserve the design do not require new approval.
- Selected route, destination, boarding stop, ETA, connectivity status and alert state are dynamic content; the approval does not freeze them to the photographed values.
- Use the approved device photo and source snapshot in `design/approved-2026-09-09/` as the visual reference. Firmware snapshot is a reference, not an instruction to overwrite newer functionality or saved settings.
- Preserve Wi-Fi credentials, saved route and touch calibration during firmware updates. Never commit credentials or raw device NVS backups.

## Board UI simplicity — user rule 2026-09-10
- Minimal on-board text everywhere. Prefer short Chinese labels (自動/日間/夜間), whole-number percentages, no redundant explanations. Use bespoke simple drawn symbols for unambiguous actions; no stock icons or emoji (only exception: the gear icon, see map detail below). Keep explanations in documentation, not the small screen.
- Mode selection saves immediately; returning to main must preserve it. Route drafts still require Save. Keep current174×140 map area and clock position; ETA remains visually primary.

## User-approved map detail —2026-09-10
Night-mode label: 夜間. Gear icon: filled toothed ring that reads clearly as a gear (stock icon allowed if needed). Subtle decorative grid and a red travel-direction arrow authorized; preserve 174×140 map bounds. Arrow follows route order, not fixed north/south or GPS.
