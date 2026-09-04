# Project Status Document

## Overview
**Project Name:** dum-e
**Purpose:** UWB (ultra-wideband) positioning project using a Makerfabs MaUWB_AOA Gen2 Anchor + Tag pair. First milestone: connect the anchor to the laptop and prove it works with a heartbeat monitor POC.
**Tech Stack:** Python (uv, pyserial) on macOS. Hardware: MaUWB_AOA Gen2 Anchor (STM32F103 + DW3000), external CP2102 USB-UART adapter.

---

## Current Status
**Last Updated:** 2026-09-04
**Current Phase:** Hardware bring-up (anchor serial connection)
**Active Branch:** main (empty repo, no commits yet)

### What We're Working On Now
- Getting clean serial data from the anchor. Blocked in the USB-to-UART transport layer: either the CP2102 adapter is faulty/non-genuine or its interaction with macOS is corrupting/replaying USB receive data.
- Silicon Labs CP210x VCP driver v6.0.3 is installed, approved, active, and enabled on macOS 26.6.2. It did not fix the stream. Next step agreed: test the same adapter and anchor on the user's Windows machine to separate adapter failure from macOS driver incompatibility.
- After clean serial works: build the heartbeat monitor POC (parse 0x2A frames, show anchor alive + distance/angle live).

### Active Side Quests
- Tag setup deferred ("while we figure out what to do with the tag" - user's words). Anchor-only bring-up first.

---

## Evolution & Pivots
None yet - project just started.

---

## Architecture Decisions

### Active Decisions
| Decision | Rationale | Date |
|----------|-----------|------|
| Anchor-first bring-up, tag later | User request; prove the serial link before tackling tag pairing | 2026-09-04 |
| Heartbeat monitor POC as first deliverable | Simple visible proof the anchor functions | 2026-09-04 |
| Python + pyserial via uv for serial tooling | Fast iteration, per user's global Python rules | 2026-09-04 |
| Cross-check the same CP2102 on Windows before replacing it | Distinguishes faulty/counterfeit adapter hardware from macOS/DriverKit corruption using one controlled hardware setup | 2026-09-04 |

---

## Completed Work

### Session History

#### 2026-09-04 - Silicon Labs driver test and online investigation
- **Completed:** Installed and approved the official Silicon Labs CP210x VCP macOS driver v6.0.3. Confirmed `com.silabs.cp210x` is activated and enabled; adapter appeared as both `/dev/cu.SLAB_USBtoUART` and `/dev/cu.usbserial-0001`. Retested both device nodes and researched Makerfabs GitHub, documentation, issues, Silicon Labs documentation, and relevant maker forum reports.
- **Key findings:**
  - Every requested baud (9600, 115200, 921600) delivered roughly 523-526 KB/s on both device nodes, with zero valid 81-byte `0x2A ... 0x23` frames and only seven byte values (`00/0c/70/7c/80/f0/fc`).
  - At 115200, the received data is an exact 63-byte pattern repeated indefinitely. A real 115200 8N1 UART can carry only about 11,520 B/s, so this stream cannot originate from the anchor UART. The 63-byte periodicity strongly suggests USB receive-packet/buffer replay.
  - The original built-in macOS path had already ignored baud settings at about 84,150 B/s. The Silicon Labs DriverKit driver made the symptom worse (~525 KB/s replay), so the third-party driver is not the original cause but may exacerbate it.
  - Makerfabs officially specifies TTL UART at 115200 and a generic USB-TTL adapter; CP2102 is not required. The GitHub Assistant is Windows-only. The only current Gen2 GitHub issue is unrelated parser logic; no Makerfabs/MakerHub community post documents this exact failure.
  - Forum reports only provide anecdotal support that cheap CP2102 adapters can be defective or unreliable; they do not establish the cause for this unit.
- **Context:** The driver installer initially failed Gatekeeper because the DMG inherited a WhatsApp/App Sandbox quarantine marker, then appeared to hang while waiting for Driver Extensions approval. After approval the extension activated normally.
- **Decision:** Test the same CP2102 + anchor wiring on Windows. If Windows also ignores baud or repeats/corrupts data, replace the adapter with a known-good FTDI, CH340, or genuine CP2102N. If Windows works, treat this as a macOS driver compatibility problem.

#### 2026-09-04 - Anchor serial bring-up and root-cause of garbled data
- **Completed:** Detected anchor on /dev/cu.usbserial-0001 (CP2102, Silicon Labs VID 0x10C4 / PID 0xEA60). Diagnosed why all serial reads are garbage. Pulled and read the official Gen2 anchor datasheet.
- **Key findings:**
  - The CP2102 adapter or its macOS driver path ignores baud-rate commands under the built-in macOS driver. It delivered exactly 84,150 bytes/sec regardless of requested baud (9600 through 3M), shredding the anchor's 115200 stream into artifacts (byte alphabet limited to 00/0c/70/7c/80/f0/fc). Later testing showed hardware failure/counterfeit hardware is likely but not yet proven; use the Windows cross-check to distinguish it from macOS incompatibility.
  - Tried and failed: baud probing (9600-3M), DTR/RTS reset pulse, waveform reconstruction + brute-force timing-ratio decode (units 1.5-17, both polarities). All produced artifacts, no real frames. Do not retry these; the data loss happens inside the chip.
- **Context:** User plugged the anchor in via external CP2102 USB-TTL dongle.
- **Next action agreed:** Install Silicon Labs CP210x VCP Mac driver v6.0.3, replug, and retest. This was completed and failed; see the newer session entry.

---

## Known Issues & Technical Debt
- [ ] CP2102/macOS transport ignores baud commands and replays an exact 63-byte pattern at impossible throughput (blocking). Root cause is narrowed to adapter hardware versus macOS driver compatibility; Windows cross-check pending.

---

## TODO - Upcoming Tasks

### High Priority
- [ ] Test the same CP2102 + anchor on Windows at 9600, 115200, and 921600; record throughput and scan the 115200 stream for 81-byte `0x2A ... 0x23` frames
- [ ] If Windows reproduces corruption, replace the adapter with a known-good FTDI, CH340, or genuine CP2102N; do not flash anchor firmware through the suspect adapter
- [ ] Write frame parser for the 81-byte positioning frame (spec below)
- [ ] Build heartbeat monitor POC: Python serial reader + live view (terminal or small web page) showing anchor alive, frame rate, and per-tag distance/angle

### Medium Priority
- [ ] Bring up the tag and confirm real distance/angle values
- [ ] First git commit once POC code exists

### Parked Ideas
- Full positioning/auto-follow demo (the Gen2 kit is built for follow-me robots)

---

## Important Context

### Hardware & Protocol Reference (hard-won, keep)
- **Board:** MaUWB_AOA Gen2 Anchor, STM32F103 + DW3000, UART-TTL at **115200**, 5V supply. Pins: TXD(1), RXD(3), GND(4), VCC(5).
- **Datasheet:** https://github.com/Makerfabs/MaUWB_AOA-Gen2 -> datasheet/Anchor/MaUWB_AOA Gen2 Anchor_Data_Sheet_v1.0.docx (downloaded and extracted this session).
- **Positioning frame (binary, 81 bytes):**
  - head 0x2A (offset 0), cmd_len uint16 (1), cmd_saddr uint64 (3), cmd_daddr uint64 (11), cmd_type uint8 (19; 1=positioning, 0x64=positioning+RSSI), cmd_direct uint8 (20), tag_time uint32 (21), anc_addr16 uint16 (25), tag_addr16 uint16 (27), tag_sn uint8 (29), tag_mask uint8 (30)
  - Per-anchor filtered pairs at offsets 31-46: tag_aN_filter_angle int16 (deg) + tag_aN_filter_range uint16 (cm) for anchors 0-3
  - tag_detail_para uint32 (47, battery/charging bits), accel x/y/z uint16 (51/53/55), gyro x/y/z uint16 (57/59/61)
  - self_raw_angle int16 (63), self_raw_range uint16 (65), self_raw_degree int32 (67), angle_360 uint16 (71), angle_dir uint8 (73), reserve 5B (74), checksum uint8 (79, over cmd_saddr..reserve), foot 0x23 (80)
- The anchor streams these frames continuously on power-up; no command needed to see output.

### Gotchas & Pitfalls
- The serial "garbage" was NOT a baud mismatch to solve by guessing rates: identical bytes at every requested baud means the adapter chip is not applying baud changes. Diagnose with a throughput test (bytes/sec vs requested baud) before anything else.
- The anchor protocol is binary, so even a correct link will look like "garbage" in a text terminal. Judge health by finding 0x2A ... 0x23 framing, not printability.
- Anchor needs 5V on VCC (not 3.3V).

### Things That Look Wrong But Aren't
- 28% "printable" serial data is expected once the link works - it's a binary protocol.

### External Dependencies & Constraints
- Windows machine is available for the controlled CP2102 cross-check.
- Silicon Labs CP210x VCP Mac driver v6.0.3 is installed and active, but did not produce a usable stream on macOS 26.6.2.
- Makerfabs' MaUWB Assistant is Windows-only; no native macOS build is provided in the Gen2 repository.

---

## Key Files & Structure
```
Repo is empty (only .git). No code committed yet.
Scratchpad (session-temporary, not preserved): serial probing scripts + anchor_ds.docx datasheet copy.
```

---

## How to Resume
1. Move the same CP2102 adapter and anchor wiring to the user's Windows machine. Keep anchor TXD -> adapter RXD, anchor RXD -> adapter TXD, common GND, and anchor VCC at 5V.
2. Identify the COM port in Device Manager. Install Silicon Labs' official Windows CP210x driver only if Windows does not create a COM port automatically.
3. Test 9600, 115200, and 921600. Throughput must scale with the selected rate; at 115200 the theoretical 8N1 ceiling is about 11,520 B/s. Capture binary data and scan for 81-byte frames beginning `0x2A` and ending `0x23`.
4. Interpret the result: corruption/63-byte replay on Windows means the adapter is faulty or non-genuine; clean frames on Windows indicate macOS/DriverKit incompatibility.
5. If the adapter fails on Windows, replace it with a known-good FTDI, CH340, or genuine CP2102N. Do not flash anchor firmware through the suspect adapter.
6. Once clean frames are available, build the parser and heartbeat monitor showing frame rate, checksum state, and tag distance/angle.
