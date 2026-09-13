# Project Status Document

## Overview
**Project Name:** dum-e
**Purpose:** UWB (ultra-wideband) positioning project using a Makerfabs MaUWB_AOA Gen2 Anchor + Tag pair. First milestone: connect the anchor to the laptop and prove it works with a heartbeat monitor POC.
**Tech Stack:** Python (uv, pyserial) on macOS/Windows. Hardware: MaUWB_AOA Gen2 Anchor (STM32F103 + DW3000), external CP2102 USB-UART adapter, Makerfabs MaUWB Assistant on Windows.

---

## Current Status
**Last Updated:** 2026-09-13
**Current Phase:** Waiting on hardware: Makerfabs support confirmed the anchor must be configured via a CH340 USB-UART adapter; need to buy a CH340 before continuing
**Active Branch:** main tracking `origin/main` (`https://github.com/omers2008/dum-e.git`)

### What We're Working On Now
- **Makerfabs support email (received 2026-09-13) resolved the diagnosis:** the AOA2AN anchor "need to be set with CH340 before usage". A CH340 USB-UART adapter is required for the anchor's initial configuration; our CP2102 does not qualify. This likely explains both the zero-byte serial reads (unconfigured anchor may not stream telemetry) and the Makerfabs Assistant's empty port dropdown (it probably enumerates only CH340 COM ports).
- Also from the email: the AOA2TA tag needs no setup, just power it on. The small-battery "AOA Tag PCBA" is for button/feedback only, not part of the positioning path. AOA2TA is a chipset; Makerfabs does not sell a separate board for it.
- **Our tag is a bare castellated SMD module (photo 2026-09-13): label `JL-U2-CA`, Sn 100543, component marked 8060 (onboard ceramic antenna), u.FL/IPEX connector (optional external antenna), anchor/tag (Chinese: 基站/标签) checkboxes on the label.** The Gen2 Tag datasheet (read 2026-09-13) confirms: 3.3V/200mA supply, 12mA operating current, onboard full-directional antenna ("simply plug it in to use"), so **no external antenna is required** - just 3.3V on pin 16 and GND on pin 1. Do NOT feed it from a CH340 3.3V pin (200mA budget exceeds its ~25mA limit). Use the ESP32 dev board 3.3V pin or an AMS1117-3.3 regulator board.
- The Gen2 repo ships `Driver/CH341SER.EXE` (the CH340 Windows driver), further confirming the toolchain expects a CH340 adapter.
- Next hardware step: buy a CH340 USB-TTL adapter, wire it the same way (5V, GND, TX/RX crossed), and retry Makerfabs Assistant on Windows.
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
| Do not replace the CP2102 yet | Windows driver install and loopback test proved the adapter can enumerate as `COM5` and echo serial data correctly | 2026-09-04 |
| Validate with Makerfabs MaUWB Assistant before deeper custom parsing | The board is powered but silent over our raw serial reads; the vendor Windows tool may reveal required product mode/configuration | 2026-09-04 |

---

## Completed Work

### Session History

#### 2026-09-04 - Windows CP2102 driver, loopback, wiring validation, and Makerfabs Assistant
- **Completed:** Connected the CP2102 adapter to the user's Windows PC. Installed the official Silicon Labs CP210x Universal Windows Driver from Silicon Labs after Windows initially showed the adapter as `CP2102 USB to UART Bridge Controller` with Code 28 / driver not installed. After elevated `pnputil` install, Windows recognized it as `Silicon Labs CP210x USB to UART Bridge (COM5)`.
- **Completed:** Ran raw serial tests on `COM5`. With the anchor attached, reads returned 0 bytes at 9600, 38400, 57600, 115200, 230400, 460800, and 921600.
- **Completed:** Ran a CP2102 loopback test with TXD connected directly to RXD. Sent `DUME_LOOPBACK_12345` at 115200 and received the exact same 19 bytes back, proving the adapter and Windows driver path can transmit/receive normally.
- **Completed:** Validated wiring from user photos:
  - Anchor front-side UART connector labels are `VCC, GND, RXD, TXD`; observed colors were red, black, yellow, green respectively.
  - Correct CP2102 mapping is red -> `5V`, black -> `GND`, green(anchor TXD) -> adapter `RXD`, yellow(anchor RXD) -> adapter `TXD`.
  - The back-side `VCC / CLK / DIO / GND` label appears to describe a different debug/programming-style interface, not the UART mapping to use for serial.
- **Completed:** Downloaded and extracted `MaUWB_AOA_Assistant.zip` from the official Makerfabs `MaUWB_AOA-Gen2` GitHub repository into `tools/MaUWB_AOA_Assistant/`. Launched `MaUWB_Assistant(v3.66).exe`.
- **Completed:** Used Computer Use to navigate Makerfabs Assistant. Selected `UWB Product -> AOA Series -> Dev Board`, which opened the main AOA form (`Makerfabs[AOA系列v3.6.6]`). The top-right serial dropdown and the separate serial/network helper both showed an empty port field. The helper's scan/magnifying-glass button closed the helper but did not populate the main port dropdown. Restarting the app and reselecting the dev board did not change this.
- **Key finding:** Windows still reports `Silicon Labs CP210x USB to UART Bridge (COM5)` as present and OK, but Makerfabs Assistant does not enumerate it. Next best test is to run Makerfabs Assistant as administrator and check whether `COM5` appears. If not, investigate whether the assistant has an old Qt serial-port enumeration issue with the current CP210x Universal Windows Driver.
- **Current interpretation:** The CP2102 is not faulty. The anchor is powered/flashing, but raw serial is silent. Next diagnosis should focus on vendor tool configuration, whether the selected connector/mode is the telemetry UART, whether a tag must be powered/paired, or whether firmware/config differs from the datasheet expectation that frames stream continuously.

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
- [ ] CP2102/macOS transport ignores baud commands and replays an exact 63-byte pattern at impossible throughput. Windows loopback proves the adapter can work correctly under Windows, so this is likely a macOS driver/transport incompatibility or macOS-specific failure mode rather than total adapter failure.
- [ ] Anchor is powered/flashing but raw serial on Windows `COM5` receives 0 bytes at all tested baud rates. Per Makerfabs support (2026-09-13), the anchor requires initial setup via a CH340 adapter; the CP2102 path is the likely cause of both this and the Assistant issue below.
- [ ] Makerfabs Assistant v3.66 does not enumerate `COM5` (CP2102). Likely the Assistant only enumerates CH340 ports; retest once a CH340 adapter arrives.

---

## TODO - Upcoming Tasks

### High Priority
- [x] Test the same CP2102 + anchor on Windows at 9600, 115200, and 921600; record throughput and scan the 115200 stream for 81-byte `0x2A ... 0x23` frames
- [x] Run CP2102 loopback on Windows to decide whether the adapter is faulty
- [x] Contact Makerfabs support; answer received 2026-09-13: anchor requires CH340 for setup, tag needs power only
- [ ] Buy a CH340 USB-TTL adapter
- [x] Pull the Gen2 Tag datasheet and map the module pins (see Hardware & Protocol Reference); onboard antenna confirmed, no external antenna needed
- [ ] Tag shopping list (bare JL-U2-CA module): 3.3V source (ESP32 3.3V pin or AMS1117-3.3 board) and a breakout/perfboard or two soldered wires on the VCC/GND castellations
- [ ] Check which box (anchor/tag, 基站/标签) is marked on each module label
- [ ] Once the CH340 arrives: install the CH340/CH341 Windows driver, wire anchor to CH340 (red -> 5V, black -> GND, green anchor TXD -> adapter RXD, yellow anchor RXD -> adapter TXD)
- [ ] Retry Makerfabs Assistant (`UWB Product -> AOA Series -> Dev Board`), select the CH340 COM port at `115200`, and configure the anchor
- [ ] After anchor setup, check whether raw serial now streams 81-byte `0x2A ... 0x23` frames
- [ ] If Makerfabs Assistant sees the board, capture its mode/config and use that to update the custom serial parser plan
- [ ] If still silent after CH340 setup, power on the tag (needs no setup per Makerfabs) and retest anchor output
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
- **Tag module (Gen2 Tag datasheet v1.0, 16 castellated pins, 13x23x2mm):** pin 1 GND; pin 16 VCC **3.3V** (supply rated 3.3V/200mA, ~12mA operating); pin 11 UART_TX and pin 12 UART_RX at 115200; pin 13 reset (active low); pin 5 green status LED (flashes when powered); pin 6 red power LED (solid when on); pins 3/4 download CLK/DIO; pins 2,7,8,9,10,14,15 NC (leave floating). Onboard full-directional antenna; CH9 band 8987.2 MHz; tag continuously emits UWB, anchor computes distance+angle. No configuration needed - power only.
- **Tag datasheet:** `datasheet/Tag/MaUWB_AOA Gen2 Tag_Data_Sheet_v1.0.docx` in the Gen2 repo; tag firmware bins in `datasheet/Tag/bin/`.
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
- Silicon Labs CP210x Universal Windows Driver is installed and active; CP2102 appears as `COM5`.
- Makerfabs' MaUWB Assistant is Windows-only; no native macOS build is provided in the Gen2 repository. Local extracted path: `tools/MaUWB_AOA_Assistant/MaUWB_AOA_Assistant/MaUWB_Assistant(v3.66).exe`.

---

## Key Files & Structure
```
Remote repo contains .ai-playground/project-status.md and local tools now include Makerfabs Assistant.
docs/ - official Makerfabs Gen2 datasheets: Anchor and Tag Data Sheet v1.0 (.docx, saved 2026-09-13)
tools/MaUWB_AOA_Assistant/ - extracted official Makerfabs Windows assistant (portable app)
DUME_Clicker_Phrase_Selector/ - local untracked ESP32 clicker sketch
```

---

## How to Resume
1. If the CH340 adapter has arrived: install the CH340/CH341 Windows driver, wire the anchor to it (red -> 5V, black -> GND, green anchor TXD -> adapter RXD, yellow anchor RXD -> adapter TXD).
2. Launch Makerfabs Assistant, select `UWB Product -> AOA Series -> Dev Board`, pick the CH340 COM port at `115200`, and open the serial connection to set up the anchor (Makerfabs: "The AOA2AN need to be set with CH340 before usage").
3. Record exactly which mode/product page and config values the Assistant uses.
4. After setup, read raw serial and look for 81-byte `0x2A ... 0x23` frames.
5. If still silent, power on the tag (no setup needed per Makerfabs) and retest anchor output.
6. Once clean frames are available, build the parser and heartbeat monitor showing frame rate, checksum state, and tag distance/angle.
