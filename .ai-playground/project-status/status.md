# Project Status Document

## Overview
**Project Name:** dum-e
**Purpose:** UWB (ultra-wideband) positioning project using a Makerfabs MaUWB_AOA Gen2 Anchor + Tag pair. First milestone: connect the anchor to the laptop and prove it works with a heartbeat monitor POC.
**Tech Stack:** Python (uv, pyserial) on macOS/Windows. Hardware: MaUWB_AOA Gen2 Anchor (STM32F103 + DW3000), Gen2 Tag module (JL-U2-CA), USB-UART adapter, Makerfabs MaUWB Assistant on Windows. CAD: Onshape, PLA on Ender-3 V3 SE.

---

## Current Status
**Last Updated:** 2026-10-04
**Current Phase:** Tag mount design (anchor bring-up still waiting on a CH340 adapter)
**Active Branch:** main tracking `origin/main` (`https://github.com/omers2008/dum-e.git`)

### What We're Working On Now
- Next session: design a 3D-printed mount for the bare tag module, using the datasheet dimensions below. | s4
- The tag needs power wiring only: 3.3V to pin 16, GND to pin 1. No UART, no configuration. | s4
- Anchor: Makerfabs says it must be set up via a CH340 adapter before use; not bought yet. | ref
- Open question: Makerfabs sold us their own CP2102 adapter with the anchor, which contradicts "CH340 required". Follow-up email offered, not sent. | s4

### Active Side Quests
- Untracked `cad/tag-mount-v3/` (pogo-contact bench fixtures) already exists; decide whether to build on it or start fresh. | s4

---

## Architecture Decisions

### Active Decisions
| Decision | Ref |
|----------|-----|
| Anchor-first bring-up, tag later (tag now unblocked: power only) | s1, s4 |
| Heartbeat monitor POC as first software deliverable | s1 |
| Python + pyserial via uv for serial tooling | s1 |
| Do not replace the CP2102 on hardware-fault grounds: Windows loopback proved it works | s3 |
| Validate with Makerfabs Assistant before deeper custom parsing | s3 |
| Power the tag from ESP32 3.3V pin or an AMS1117-3.3 board, never a CH340 3.3V pin | ref |

---

## Completed Work

### Session Archive
Full session-by-session history lives in `.ai-playground/project-status/archive/` - append-only files capped at 300 lines each. Use the Lines range to read just the entry you need; the last row's end line is that file's current size. Open an archive file only when a "What is inside" summary matches what you need; do not bulk-read the folder.

#### archive/sessions-001.md
| Session | Date | Author | Lines | What is inside |
|---------|------|--------|-------|----------------|
| 1 | 2026-09-04 | shlomi | 1-7 | Anchor detected via CP2102 on macOS; throughput test shows baud ignored (84,150 B/s at any baud); failed decode attempts not to retry. |
| 2 | 2026-09-04 | shlomi | 9-18 | Silicon Labs v6.0.3 macOS driver: 63-byte replay at ~525 KB/s, impossible for UART; Makerfabs docs say any USB-TTL works; decision to cross-check on Windows. |
| 3 | 2026-09-04 | shlomi | 20-31 | Windows CP210x driver install (COM5), 0 bytes from anchor at all bauds, loopback passes, wiring colors validated, Makerfabs Assistant shows empty port list. |
| ref | 2026-10-04 | shlomi | 33-41 | Ported reference: Makerfabs CH340 reply, tag label/antenna details, anchor and tag pin maps, full 81-byte positioning frame layout, datasheet sources, driver environment. |
| 4 | 2026-10-04 | shlomi | 43-56 | Tag datasheet dimensions (13x23x2, 1.27 pitch, pad rows 2.0-10.89 mm from pad end, antenna end clear), power-only wiring, order #000027927 (2 tags, Makerfabs CP2102), existing cad/tag-mount-v3, status migration. |

---

## Known Issues & Technical Debt
- [ ] macOS + CP2102 ignores baud and replays a 63-byte pattern; treat macOS serial as unusable with this adapter. | s2
- [ ] Anchor on Windows `COM5` (CP2102) returns 0 bytes at all bauds; likely needs CH340-based setup first. | s3, ref
- [ ] Makerfabs Assistant v3.66 does not list `COM5`; retest with a CH340. | s3

### Deferred Decisions
- Ask Makerfabs to clarify CP2102 vs CH340 before buying a CH340, or just buy one (cheap). | s4

---

## TODO - Upcoming Tasks

### High Priority
- [ ] Design a 3D-printed tag mount (next session). Inputs: Tag Dimensions below, review `cad/tag-mount-v3/README.md` first. | s4
- [ ] Measure the real tag with calipers: PCB thickness, shield height/footprint, pad size, antenna block. | s4
- [ ] Wire tag power: 3.3V -> pin 16, GND -> pin 1; confirm red LED (pin 6) solid and green LED (pin 5) flashing. | s4
- [ ] Check which box (anchor/tag) is marked on each module label. | ref
- [ ] Buy a CH340 USB-TTL adapter (or get Makerfabs to clarify first). | ref
- [ ] With CH340: install CH341 driver, wire anchor (red -> 5V, black -> GND, green anchor TXD -> adapter RXD, yellow anchor RXD -> adapter TXD). | s3
- [ ] Makerfabs Assistant: `UWB Product -> AOA Series -> Dev Board`, CH340 COM port, 115200; record mode/config used. | s3
- [ ] Then read raw serial and look for 81-byte `0x2A ... 0x23` frames; if silent, power the tag and retest. | ref

### Medium Priority
- [ ] Write frame parser for the 81-byte positioning frame. | ref
- [ ] Heartbeat monitor POC: serial reader + live view (anchor alive, frame rate, checksum state, per-tag distance/angle). | s1
- [ ] Bring up the tag with the anchor and confirm real distance/angle values.

### Parked Ideas
- Full positioning/auto-follow demo (the Gen2 kit is built for follow-me robots).
- Fruit fly connectome as a "roam" behavior (see `ideas\nice to have`, added by omers2008).

---

## Important Context

### Tag Dimensions (Gen2 Tag datasheet v1.0) | s4
- Outline 13 x 23 mm, 2 mm thick, 1.5 g.
- 16 castellated edge pads, 8 per long side, 1.27 mm pitch; pins 1-8 down left, 9-16 up right (top view, dot at pin 1).
- Pin 8/9 centers 2.0 mm from the pad end; rows span 2.0 to ~10.89 mm from that end; pins 1/16 ~12.1 mm from the antenna end.
- Antenna end (~11 mm) has no pads; keep it clear of metal. Shield covers the pad end.
- Not dimensioned: pad size (~1.0 x 1.4 mm by drawing), shield height, antenna footprint, PCB thickness.

### Tag Pins | ref
- 1 GND, 16 VCC 3.3V (200mA rated, ~12mA typical), 11 UART_TX, 12 UART_RX (115200), 13 reset (active low), 5 green LED, 6 red LED, 3/4 CLK/DIO, 2/7/8/9/10/14/15 NC (leave floating).

### Anchor | ref
- 5V supply (not 3.3V), UART-TTL 115200. Front connector `VCC, GND, RXD, TXD` = red, black, yellow, green.
- Positioning frame: 81 bytes, 0x2A ... 0x23, XOR checksum at byte 79, little-endian; full layout in the ref entry.

### Gotchas & Pitfalls
- Identical bytes at every requested baud = adapter not applying baud. Run a throughput test (bytes/sec vs baud) before guessing rates. | s1
- Do not retry baud probing, DTR/RTS reset, or waveform reconstruction on the CP2102/macOS stream. | s1
- Do not power the tag from a CH340 3.3V pin (~25mA limit). | ref

### Things That Look Wrong But Aren't
- Binary protocol looks like garbage in a text terminal (~28% printable); judge health by 0x2A ... 0x23 framing. | s1

### External Dependencies & Constraints
- Makerfabs Assistant is Windows-only; a Windows PC is available. | s3
- Order #000027927: 2x tag, 1x anchor, 1x Makerfabs CP2102. One spare tag. | s4

---

## Where To Find Stuff
- `docs/`: official Gen2 Anchor and Tag datasheets (.docx; tag footprint drawing is an embedded image).
- `tools/MaUWB_AOA_Assistant/`: Makerfabs Windows assistant v3.66.
- `cad/` (untracked): `tag-mount-v3/` pogo bench fixtures (README has dimensions/assumptions, Onshape links), `elevator-base-v1/`, `elevator-base-v2/`.
- `ideas\nice to have`: project ideas list.

---

## How to Resume
1. Read the Tag Dimensions section above and `cad/tag-mount-v3/README.md`.
2. Ask the user about the mount goal: power-only holder with soldered wires, pogo contacts, or wearable enclosure; and whether to reuse tag-mount-v3.
3. If the user has caliper measurements of the real tag, use them over the datasheet nominal values.
4. Design in Onshape (jarvis-onshape-mcp available), export STL for the Ender-3 V3 SE.
