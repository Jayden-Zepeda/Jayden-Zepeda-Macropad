# Zepeda Macropad

> A custom 14-key macropad with dual rotary encoders, per-key RGB lighting, and an OLED display — built on the Seeed XIAO RP2040 with QMK firmware.

<p align="center">
  <img src="Images\Final-pic 1.png" alt="Zepeda Macropad Top View" width="700"/>
</p>

---

## 📖 Overview

The **Zepeda** is a compact, handcrafted macropad designed for productivity and personalization. It features 14 MX-style switches arranged in a 3×4 grid (plus a dedicated column), two EC11 rotary encoders for scrolling or volume control, individually addressable SK6812 MINI-E RGB LEDs under every key, and a 0.91" OLED display for live feedback — all driven by a Seeed XIAO RP2040 and QMK firmware.

The key matrix is read through a **74HC165 shift register**, keeping GPIO usage minimal while supporting full anti-ghosting via 1N4148 diodes. The entire build fits inside a custom 3-part 3D-printed case secured with heatset inserts and M3 screws.

PCB is designed through KICAD and the case is designed in Autodesk Fusion-FREE.

---

## 🖼️ Screenshots

### 3D Render

<p align="center">
  <img src="Images\Final-pic 2.png" alt="Top-down render" width="600"/>
  <br/><em>Top-down view</em>
</p>

<p align="center">
  <img src="Images\Final-pic 1.png" alt="Isometric render" width="600"/>
  <br/><em>Isometric view showing case depth and USB cutout</em>
</p>

---

### Schematic

> Full schematic showing the XIAO RP2040, 74HC165 shift register, LED chain, diode matrix, rotary encoders, and OLED connector.

<p align="center">
  <img src="Images\macropad-sch.pdf" alt="Schematic" width="700"/>
  <br/><em>KiCad schematic — macropad.kicad_sch</em>
</p>

---

### PCB Layout

<p align="center">
  <img src="Images\pcb.png" alt="PCB Layout" width="700"/>
  <br/><em>KiCad PCB layout — red/blue layers, switch footprints, and LED positions visible</em>
</p>

---

### Case

<p align="center">
  <img src="Images\Final-pic 3.png" alt="Case isometric" width="600"/>
  <br/><em>3-part 3D-printed enclosure with heatset inserts. The PCB sits inside the bottom shell; the top plate exposes keycaps, encoders, and the OLED. A middle layer sandwiches everything together. Screws are enter from bottom, they go in 2mm freely and then left side screws all the way into the heatset insterts that are in the top shell while, the right side screws does a very similar thing except the heatset insterts are 1mm in the top shell and 3mm in the middle shell.</em>
</p>

The case consists of three printed parts(created in Autodesk Fusion-FREE) :
- **Bottom shell** — holds the PCB
- **Middle frame / plate** — key switch mounting plate
- **Top accent** — surrounds the OLED and encoder knobs

All three parts are secured with **4× M3×16mm screws** threading into **4× M3×5mm×4mm heatset inserts** pressed into the top/middle shell.

---

## 🧰 Bill of Materials (BOM)

| Qty | Part | Notes |
|-----|------|-------|
| 1 | Seeed XIAO RP2040 | Microcontroller (unsoldered) |
| 1 | 74HC165 Shift Register | Through-hole DIP-16 |
| 16 | SK6812 MINI-E LEDs | Per-key addressable RGB |
| 16 | 1N4148 Diodes | Through-hole, for key matrix |
| 14 | MX-Style Switches | Any MX-compatible switch |
| 14 | White Blank DSA Keycaps | 1u, blank |
| 2 | EC11 Rotary Encoders | With push-click |
| 1 | 0.91" OLED Display | I2C, SSD1306 |
| 1 | 3D-Printed Case | 3 printed parts |
| 4 | M3×16mm Screws | Case assembly |
| 4 | M3×5mm×4mm Heatset Inserts | Press-fit into top/middle shell |

---

## ⚡ Firmware

This macropad runs **QMK Firmware**. The 74HC165 shift register is used to read the key matrix, and the SK6812 MINI-E LEDs are driven via QMK's RGB Matrix feature. Created a full keyboard and keymap layout from scratch. Included in firmware folder(macropad).                                    

Features:
-Copy/Paste Key: First click is copy-second is paste, with an led and OLED display to show your status.
-Mute/Unmute Key: Led and OLED display to show your status
-4 Different Layers: Base, Media, Gaming, and CFG -- w/ led status

To build and flash:

```bash
qmk compile -kb macropad -km default
qmk flash -kb macropad -km default
```

Put the XIAO RP2040 into bootloader mode by holding **BOOT** while plugging in USB (or double-tapping RST).

---

## 🔧 Build Notes

- Solder **heatset inserts first** before assembling the case
- Flash QMK firmware onto the XIAO RP2040 **before** soldering it to the board
- The LED chain is daisy-chained: DOUT of each SK6812 connects to DIN of the next
- Diodes are oriented with the **cathode (K) toward the row line** — check markings before soldering
- The 74HC165's `~PL`, `CP`, and `Q7` lines connect to GPIO26, GPIO27, and GPIO28 on the XIAO respectively

---

## 📁 Repository Structure

```
Jayden-Zepeda-Macropad/
├── pcb/               # KiCad project files (.kicad_sch, .kicad_pcb)
├── case/              # 3D print files (.step / .stl)
├── firmware/          # QMK keymap and config
├── images/            # Images
├── production/            # Files for production
└── README.md
```

---

*Made with ❤️ as part of [Hack Club's PCB program](https://hackclub.com/)*
