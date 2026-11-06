#!/usr/bin/env python3
"""
NanoSubGHz — Wiring diagram generator
Author: Sassiz.Root
Requires: pip install matplotlib
Usage   : python generate_wiring.py
Output  : wiring.png
"""

import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.lines import Line2D

# Colors
C_LCD    = '#e1f5fe'; C_LCD_E  = '#0277bd'
C_NANO   = '#fff8e1'; C_NANO_E = '#f57f17'
C_BTN    = '#e1f5fe'; C_BTN_E  = '#0277bd'
C_RX     = '#e3f2fd'; C_RX_E   = '#1565c0'
C_TX     = '#fff3e0'; C_TX_E   = '#ef6c00'

W_SDA = '#e91e63'
W_SCL = '#9c27b0'
W_SPI = '#00897b'
W_RX  = '#1976d2'
W_TX  = '#ef6c00'
W_BTN = '#388e3c'
W_AGC = '#5d4037'

fig, ax = plt.subplots(figsize=(16, 14), dpi=150)
ax.set_xlim(0, 1400); ax.set_ylim(0, 1230)
ax.invert_yaxis()  # y grows downward (like SVG)
ax.axis('off')
ax.set_facecolor('#fafafa')
fig.patch.set_facecolor('#fafafa')

def box(x, y, w, h, fc, ec, lw=2):
    ax.add_patch(mpatches.FancyBboxPatch(
        (x, y), w, h, boxstyle="round,pad=0,rounding_size=8",
        fc=fc, ec=ec, lw=lw))

def txt(x, y, s, size=11, color='#263238', ha='left', weight='normal'):
    ax.text(x, y, s, fontsize=size, color=color, ha=ha, va='center',
            family='sans-serif', fontweight=weight)

def wire(pts, color, dashed=False, lw=2.2):
    xs = [p[0] for p in pts]; ys = [p[1] for p in pts]
    ax.plot(xs, ys, color=color, lw=lw,
            dashes=(5,3) if dashed else '-', solid_capstyle='round')

# Titles
txt(700, 34, 'NanoSubGHz — Wiring Diagram', 20, '#1a237e', 'center', 'bold')
txt(700, 60, 'Arduino Nano • LCD1602 I2C • SD • RX/TX 433+315 • 5 Buttons',
    11, '#546e7a', 'center')

# LCD
box(120, 90, 380, 150, C_LCD, C_LCD_E)
txt(140, 114, 'LCD 1602 (I2C)', 13, '#01579b', weight='bold')
txt(140, 132, 'Address: 0x27 (alt 0x3F)', 10, '#4e6b75')
for i, s in enumerate(['● VCC → 5V', '● GND → GND',
                       '● SDA → A4', '● SCL → A5']):
    txt(140, 158 + i*20, s, 10)

# SD
box(900, 90, 380, 150, C_LCD, C_LCD_E)
txt(920, 114, 'SD Card Module (SPI)', 13, '#01579b', weight='bold')
txt(920, 132, 'FAT16/FAT32 • ≤32 GB', 10, '#4e6b75')
for i, s in enumerate(['● CS → D10', '● MOSI → D11',
                       '● MISO → D12', '● SCK → D13']):
    txt(920, 158 + i*20, s, 10)

# Nano
box(550, 380, 300, 500, C_NANO, C_NANO_E, 3)
txt(700, 408, 'ARDUINO NANO', 14, '#e65100', 'center', 'bold')
txt(700, 424, 'ATmega328P • 16 MHz', 10, '#bf360c', 'center')

left_pins  = ['D13','3V3','AREF','A0','A1','A2','A3','A4','A5','A6','A7','5V','RST','GND','VIN']
right_pins = ['D12','D11','D10','D9','D8','D7','D6','D5','D4','D3','D2','GND','RST','RX0','TX1']
used = {'D13','A0','A1','A4','A5','A6','A7','5V','GND',
        'D12','D11','D10','D9','D8','D6','D5','D4','D3','D2'}
for i, p in enumerate(left_pins):
    c = '#b71c1c' if p in used else '#212121'
    txt(560, 454 + i*28, p, 10, c, 'left', 'bold' if p in used else 'normal')
for i, p in enumerate(right_pins):
    c = '#b71c1c' if p in used else '#212121'
    txt(840, 454 + i*28, p, 10, c, 'right', 'bold' if p in used else 'normal')

# Buttons LEFT
for name, y in [('SELECT', 520), ('BACK', 590)]:
    box(80, y, 150, 45, C_BTN, C_BTN_E)
    txt(155, y+20, name, 12, '#01579b', 'center', 'bold')

# Buttons RIGHT
for name, y in [('UP', 520), ('DOWN', 590), ('LEFT', 660)]:
    box(1170, y, 150, 45, C_BTN, C_BTN_E)
    txt(1245, y+20, name, 12, '#01579b', 'center', 'bold')

# RX/TX modules
mods = [
    (100, 'RX 433 MHz', C_RX, C_RX_E,
     ['● DATA → D2', '● AGC  → A6 (RXB6)',
      '● VCC  → 5V', '● GND  → GND', 'Antenna: 17.3 cm']),
    (400, 'RX 315 MHz', C_RX, C_RX_E,
     ['● DATA → D3', '● AGC  → A7 (RXB6)',
      '● VCC  → 5V', '● GND  → GND', 'Antenna: 23.8 cm']),
    (720, 'TX 433 MHz', C_TX, C_TX_E,
     ['● DATA → D4', '● VCC  → 5V',
      '● GND  → GND', 'Antenna: 17.3 cm']),
    (1020, 'TX 315 MHz', C_TX, C_TX_E,
     ['● DATA → D5', '● VCC  → 5V',
      '● GND  → GND', 'Antenna: 23.8 cm']),
]
for x, title, fc, ec, lines in mods:
    box(x, 950, 280, 150, fc, ec)
    txt(x+20, 976, title, 13, ec, weight='bold')
    for i, s in enumerate(lines):
        color = '#c62828' if 'Antenna' in s else '#263238'
        txt(x+20, 998 + i*20, s, 10, color)

# ===== WIRES =====
# LCD
wire([(400,240),(400,340),(520,340),(520,654),(550,654)], W_SDA)
wire([(450,240),(450,365),(490,365),(490,682),(550,682)], W_SCL)
# SD
wire([(1020,240),(1020,514),(850,514)], W_SPI)
wire([(1080,240),(1080,486),(850,486)], W_SPI)
wire([(1140,240),(1140,458),(850,458)], W_SPI)
wire([(1200,240),(1200,270),(340,270),(340,458),(550,458)], W_SPI)
# Buttons
wire([(230,542),(550,542)], W_BTN)
wire([(230,612),(400,612),(400,570),(550,570)], W_BTN)
wire([(1170,542),(1000,542),(1000,570),(850,570)], W_BTN)
wire([(1170,612),(1000,612),(1000,542),(850,542)], W_BTN)
wire([(1170,682),(1000,682),(1000,626),(850,626)], W_BTN)
# RX
wire([(380,950),(380,918),(1110,918),(1110,738),(850,738)], W_RX)
wire([(250,950),(250,932),(520,932),(520,710),(550,710)], W_AGC, dashed=True)
wire([(560,950),(560,908),(1140,908),(1140,710),(850,710)], W_RX)
wire([(480,950),(480,928),(535,928),(535,738),(550,738)], W_AGC, dashed=True)
# TX
wire([(860,950),(860,682),(850,682)], W_TX)
wire([(1170,950),(1170,886),(1120,886),(1120,654),(850,654)], W_TX)

# Footer
txt(700, 1210, 'NanoSubGHz by Sassiz.Root — Educational use only — See DISCLAIMER.md',
    9, '#607d8b', 'center')

plt.savefig('wiring.png', dpi=200, bbox_inches='tight',
            facecolor='#fafafa', pad_inches=0.2)
print('✅ Saved: wiring.png')
