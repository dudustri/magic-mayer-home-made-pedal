#!/usr/bin/env python3
"""Draws every schematic in this repository as SVG, into each project folder's schematics/ directory.

Run from anywhere: python3 make_schematics.py
Each SVG has a <title> and <desc> for screen readers, and a white background so it reads in dark mode too.
"""
import math
from html import escape
from pathlib import Path

ROOT = Path(__file__).parent
INK, MUTED, PAPER, FILL = "#1f2937", "#6b7280", "#ffffff", "#f6f8fa"
KIND = {"pwr": "#c43d2f", "gnd": "#4b5563", "audio": "#8a3fb0", "i2s": "#2563a8",
        "i2c": "#2f8a4a", "spi": "#b0731a", "ink": INK}
LEGEND = [("pwr", "Power"), ("gnd", "Ground"), ("audio", "Analog audio"),
          ("i2s", "I2S audio data and clocks"), ("i2c", "I2C setup"), ("spi", "SPI radio")]
PITCH = 30


class Sheet:
    def __init__(self, w, h, title, desc, legend=True):
        self.w, self.h, self.title, self.desc, self.legend = w, h, title, desc, legend
        self.parts = []

    def add(self, s):
        self.parts.append(s)

    def text(self, x, y, t, size=13, anchor="start", weight="normal", color=INK):
        self.add(f'<text x="{x}" y="{y}" font-size="{size}" text-anchor="{anchor}" '
                 f'font-weight="{weight}" fill="{color}">{escape(t)}</text>')

    def wire(self, *pts, kind="ink", dash=False):
        p = " ".join(f"{x},{y}" for x, y in pts)
        d = ' stroke-dasharray="5 4"' if dash else ""
        self.add(f'<polyline points="{p}" fill="none" stroke="{KIND[kind]}" stroke-width="2" '
                 f'stroke-linejoin="round"{d}/>')

    def node(self, x, y, kind="ink"):
        self.add(f'<circle cx="{x}" cy="{y}" r="3.5" fill="{KIND[kind]}"/>')

    def nc(self, x, y):
        self.add(f'<path d="M{x-5} {y-5} L{x+5} {y+5} M{x-5} {y+5} L{x+5} {y-5}" stroke="{MUTED}" stroke-width="2"/>')

    def gnd(self, x, y):
        self.add(f'<path d="M{x} {y} V{y+10} M{x-12} {y+10} H{x+12} M{x-8} {y+15} H{x+8} '
                 f'M{x-4} {y+20} H{x+4}" stroke="{KIND["gnd"]}" stroke-width="2" fill="none"/>')

    def tag(self, x, y, name, kind="pwr", side="left"):  # side: left, right, up, down
        """Net label: wires with the same label are connected. side = where the tag sits from (x, y)."""
        w = 8 * len(name) + 14
        x0, y0 = {"left": (x - w, y - 10), "right": (x, y - 10), "up": (x - w / 2, y - 20), "down": (x - w / 2, y)}[side]
        self.add(f'<rect x="{x0}" y="{y0}" width="{w}" height="20" rx="4" fill="{PAPER}" '
                 f'stroke="{KIND[kind]}" stroke-width="1.5"/>')
        self.text(x0 + w / 2, y0 + 14, name, 12, "middle", "bold", KIND[kind])

    def res(self, x, y, ref, val, vertical=True):
        """Resistor centred on (x, y), 40 long. Returns both ends."""
        if vertical:
            self.add(f'<rect x="{x-7}" y="{y-20}" width="14" height="40" fill="{PAPER}" stroke="{INK}" stroke-width="2"/>')
            self.text(x + 14, y - 3, ref, 12, weight="bold")
            self.text(x + 14, y + 12, val, 12, color=MUTED)
            return (x, y - 20), (x, y + 20)
        self.add(f'<rect x="{x-20}" y="{y-7}" width="40" height="14" fill="{PAPER}" stroke="{INK}" stroke-width="2"/>')
        self.text(x, y - 13, f"{ref} {val}", 12, "middle")
        return (x - 20, y), (x + 20, y)

    def cap(self, x, y, ref, val, vertical=True, polar=False):
        """Capacitor centred on (x, y), 30 long. Returns both ends (first end is + when polar)."""
        if vertical:
            self.add(f'<path d="M{x} {y-15} V{y-4} M{x-12} {y-4} H{x+12} M{x-12} {y+4} H{x+12} '
                     f'M{x} {y+4} V{y+15}" stroke="{INK}" stroke-width="2" fill="none"/>')
            self.text(x + 18, y - 3, ref, 12, weight="bold")
            self.text(x + 18, y + 12, val, 12, color=MUTED)
            if polar:
                self.text(x - 22, y - 5, "+", 14, weight="bold")
            return (x, y - 15), (x, y + 15)
        self.add(f'<path d="M{x-15} {y} H{x-4} M{x-4} {y-12} V{y+12} M{x+4} {y-12} V{y+12} '
                 f'M{x+4} {y} H{x+15}" stroke="{INK}" stroke-width="2" fill="none"/>')
        self.text(x, y - 18, f"{ref} {val}", 12, "middle")
        return (x - 15, y), (x + 15, y)

    def opamp(self, x, top, h, ref, pins):
        """Triangle with inputs on the left (- upper, + lower) and output at the tip.
        pins = (minus, plus, out) pin numbers. Returns (minus, plus, out) coordinates."""
        tip = (x + h, top + h / 2)
        self.add(f'<path d="M{x} {top} L{x} {top+h} L{tip[0]} {tip[1]} Z" fill="{PAPER}" stroke="{INK}" stroke-width="2"/>')
        m, p = (x, top + h * 0.27), (x, top + h * 0.71)
        self.text(x + 6, m[1] + 5, "−", 15, weight="bold")
        self.text(x + 6, p[1] + 5, "+", 15, weight="bold")
        self.text(x - 6, m[1] - 5, pins[0], 11, "end", color=MUTED)
        self.text(x - 6, p[1] - 5, pins[1], 11, "end", color=MUTED)
        self.text(tip[0] + 4, tip[1] - 7, pins[2], 11, color=MUTED)
        self.text(x, top - 8, ref, 12, weight="bold")
        return m, p, tip

    def switch(self, x, y, ref, note):
        """Vertical switch from (x, y) to (x, y + 40)."""
        self.add(f'<circle cx="{x}" cy="{y+4}" r="3" fill="{PAPER}" stroke="{INK}" stroke-width="2"/>'
                 f'<circle cx="{x}" cy="{y+36}" r="3" fill="{PAPER}" stroke="{INK}" stroke-width="2"/>'
                 f'<path d="M{x} {y+33} L{x+16} {y+8}" stroke="{INK}" stroke-width="2"/>')
        self.text(x - 12, y + 20, ref, 12, "end", "bold")
        self.text(x - 12, y + 34, note, 11, "end", color=MUTED)

    def battery(self, x, y, ref, note):
        """Vertical cell, + at (x, y), - at (x, y + 36)."""
        self.add(f'<path d="M{x} {y} V{y+14} M{x-16} {y+14} H{x+16} M{x-9} {y+22} H{x+9} M{x} {y+22} V{y+36}" '
                 f'stroke="{INK}" stroke-width="2" fill="none"/>')
        self.text(x + 22, y + 13, "+", 14, weight="bold")
        self.text(x - 24, y + 16, ref, 12, "end", "bold")
        self.text(x - 24, y + 30, note, 11, "end", color=MUTED)

    def box(self, x, y, w, ref, name, left=(), right=(), bottom=(), pitch=PITCH):
        """Module with pins. Returns {pin name: (x, y)} at the outer end of each pin line."""
        rows = max(len(left), len(right), 1)
        h = 50 + rows * pitch + (26 if bottom else 0)
        self.add(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="6" fill="{FILL}" stroke="{INK}" stroke-width="2"/>')
        self.text(x + w / 2, y + 22, ref, 15, "middle", "bold")
        self.text(x + w / 2, y + 40, name, 12, "middle", color=MUTED)
        pins = {}
        for side, names in (("l", left), ("r", right)):
            for i, p in enumerate(names):
                if p is None:
                    continue
                py = y + 50 + i * pitch + pitch // 2
                x0, x1 = (x - 20, x) if side == "l" else (x + w, x + w + 20)
                self.add(f'<line x1="{x0}" y1="{py}" x2="{x1}" y2="{py}" stroke="{INK}" stroke-width="2"/>')
                if side == "l":
                    self.text(x + 8, py + 4, p, 12)
                    pins[p] = (x0, py)
                else:
                    self.text(x + w - 8, py + 4, p, 12, "end")
                    pins[p] = (x1, py)
        for i, p in enumerate(bottom):
            px = round(x + (i + 1) * w / (len(bottom) + 1))
            self.add(f'<line x1="{px}" y1="{y+h}" x2="{px}" y2="{y+h+20}" stroke="{INK}" stroke-width="2"/>')
            self.text(px, y + h - 8, p, 12, "middle")
            pins[p] = (px, y + h + 20)
        return pins

    def diode(self, x, y, ref, val, led=False, flip=False):
        """Horizontal diode centred on (x, y), 30 long. Anode left (right when flip). Returns (anode, cathode)."""
        d = -1 if flip else 1
        self.add(f'<path d="M{x-15} {y} H{x+15} M{x-7*d} {y-8} L{x-7*d} {y+8} L{x+7*d} {y} Z M{x+7*d} {y-8} V{y+8}" '
                 f'stroke="{INK}" stroke-width="2" fill="{PAPER}"/>')
        if led:
            for o in (0, 7):
                self.add(f'<path d="M{x-2+o} {y-11} l6 -8 M{x+1+o} {y-19} h3 v3" stroke="{INK}" stroke-width="1.5" fill="none"/>')
        self.text(x, y + 24, f"{ref} {val}".strip(), 11, "middle", color=MUTED)
        a, k = (x - 15, y), (x + 15, y)
        return (k, a) if flip else (a, k)

    def pot(self, x, y, ref, val):
        """Vertical potentiometer centred on (x, y). Returns (top, bottom, wiper); the wiper points right."""
        self.add(f'<rect x="{x-7}" y="{y-20}" width="14" height="40" fill="{PAPER}" stroke="{INK}" stroke-width="2"/>'
                 f'<path d="M{x+24} {y} H{x+10} M{x+10} {y} l7 -5 M{x+10} {y} l7 5" stroke="{INK}" stroke-width="2" fill="none"/>')
        self.text(x - 12, y - 3, ref, 12, "end", "bold")
        self.text(x - 12, y + 12, val, 11, "end", color=MUTED)
        return (x, y - 20), (x, y + 20), (x + 24, y)

    def save(self, name):
        if self.legend:
            x = 30
            for k, label in LEGEND:
                self.add(f'<line x1="{x}" y1="{self.h-24}" x2="{x+24}" y2="{self.h-24}" stroke="{KIND[k]}" stroke-width="3"/>')
                self.text(x + 30, self.h - 20, label, 12, color=MUTED)
                x += 44 + 7.5 * len(label)
        svg = (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {self.w} {self.h}" width="{self.w}" '
               f'height="{self.h}" role="img" aria-labelledby="title desc" '
               f'font-family="DejaVu Sans, Helvetica, Arial, sans-serif">\n'
               f'<title id="title">{escape(self.title)}</title>\n<desc id="desc">{escape(self.desc)}</desc>\n'
               f'<rect width="100%" height="100%" fill="{PAPER}"/>\n' + "\n".join(self.parts) + "\n</svg>\n")
        path = ROOT / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(svg)


def heading(s, title, sub):
    s.text(30, 40, title, 22, weight="bold")
    s.text(30, 64, sub, 13, color=MUTED)


def buffer(s, ic="U3", part="MCP6002", supply="3V3", vref="1.65 V", c2="10 µF", c3="1 µF", jack=("P1", "6.35 mm plug")):
    """Op-amp input buffer (voltage follower on a single supply). Returns the C3 output end."""
    pins = s.box(30, 180, 110, jack[0], jack[1], right=["Tip", "Sleeve"])
    tip, sleeve = pins["Tip"], pins["Sleeve"]
    s.gnd(*sleeve)
    s.wire(tip, (200, 245), kind="audio")
    s.node(200, 245, "audio")
    a, b = s.res(200, 295, "R1", "1 MΩ")
    s.wire((200, 245), a)
    s.wire(b, (200, 325))
    s.gnd(200, 325)
    c1a, c1b = s.cap(245, 245, "C1", "100 nF", vertical=False)
    s.wire((200, 245), c1a, kind="audio")
    minus, plus, out = s.opamp(360, 178, 95, f"{ic}A  {part}", ("2", "3", "1"))
    s.wire(c1b, (plus[0], 245), kind="audio")
    s.node(300, 245, "audio")
    a, b = s.res(300, 305, "R2", "1 MΩ")
    s.wire((300, 245), a)
    s.wire(b, (300, 370))
    # feedback: output straight back to the minus input = gain of exactly 1
    s.wire(out, (490, out[1]), kind="audio")
    s.node(490, out[1], "audio")
    s.wire((490, out[1]), (490, 140), (340, 140), (340, minus[1]), minus, kind="audio")
    c3a, c3b = s.cap(530, out[1], "C3", c3, vertical=False)
    s.wire((490, out[1]), c3a, kind="audio")
    # Vref = supply / 2
    s.wire((300, 370), (490, 370))
    s.node(300, 370)
    s.node(400, 370)
    s.node(460, 370)
    s.text(306, 362, f"Vref {vref}", 12, color=MUTED)
    a, b = s.res(400, 320, "R3", "100 kΩ")
    s.tag(*a, supply, side="up")
    s.wire(b, (400, 370))
    a, b = s.res(400, 410, "R4", "100 kΩ")
    s.wire((400, 370), a)
    s.wire(b, (400, 440))
    s.gnd(400, 440)
    a, b = s.cap(460, 400, "C2", c2, polar=True)
    s.wire((460, 370), a)
    s.wire(b, (460, 425))
    s.gnd(460, 425)
    s.tag(490, 370, "Vref", "ink", side="right")
    # unused half, parked as a follower on Vref so it cannot oscillate
    m2, p2, o2 = s.opamp(520, 560, 70, "", ("6", "5", "7"))
    s.text(520, 652, f"{ic}B  unused half", 12, weight="bold")
    s.wire((495, p2[1]), p2)
    s.tag(495, p2[1], "Vref", "ink")
    s.wire(o2, (605, o2[1]), (605, 545), (505, 545), (505, m2[1]), m2)
    # supply pins
    sup = s.box(180, 470, 110, ic, f"{part} supply", right=["8  V+", "4  V−"])
    vplus, vminus = sup["8  V+"], sup["4  V−"]
    s.wire(vplus, (350, vplus[1]), kind="pwr")
    s.node(350, vplus[1], "pwr")
    s.tag(350, vplus[1] - 4, supply, side="up")
    a, b = s.cap(390, vplus[1] + 20, "C4", "100 nF")
    s.wire((350, vplus[1]), (390, vplus[1]), a, kind="pwr")
    s.gnd(*b)
    s.wire(vminus, (330, vminus[1]))
    s.gnd(330, vminus[1])
    return c3b


def wm8960(s, left, right):
    """WM8960 module at a fixed spot between the analog side and the XIAO."""
    pins = s.box(620, 160, 170, "U2", "WM8960 module", left=left, right=right)
    v, g = pins["VCC"], pins["GND"]
    s.wire(v, (v[0] - 20, v[1]), kind="pwr")
    s.tag(v[0] - 20, v[1], "3V3")
    s.wire(g, (g[0] - 10, g[1]), kind="gnd")
    s.gnd(g[0] - 10, g[1])
    return pins


I2S_ROWS = [("D0", "MCLK", "i2s"), ("D1", "BCLK", "i2s"), ("D2", "LRCK", "i2s"),
            ("D3", "DATA", "i2s"), ("D4", "SDA", "i2c"), ("D5", "SCL", "i2c")]


def xiao(s, name, wm, data_pin):
    """XIAO in its real pin order, wired to the WM8960 and the battery. Returns its pins."""
    pins = s.box(900, 160, 170, "U1", name,
                 left=["D0", "D1", "D2", "D3", "D4", "D5", "D6"],
                 right=["5V", "GND", "3V3", "D10", "D9", "D8", "D7"],
                 bottom=["BAT+", "BAT−"])
    for d, w, kind in I2S_ROWS:
        s.wire(wm[data_pin if w == "DATA" else w], pins[d], kind=kind)
    s.nc(*pins["5V"])
    g = pins["GND"]
    s.wire(g, (g[0] + 60, g[1]), kind="gnd")
    s.gnd(g[0] + 60, g[1])
    s.tag(*pins["3V3"], "3V3", side="right")
    bp, bm = pins["BAT+"], pins["BAT−"]
    s.wire(bp, (bp[0], bp[1] + 10), kind="pwr")
    s.switch(bp[0], bp[1] + 10, "SW1", "power")
    s.wire((bp[0], bp[1] + 50), (bp[0], bp[1] + 66), kind="pwr")
    s.battery(bp[0], bp[1] + 66, "BT1", "LiPo 1S 300-500 mAh")
    s.wire((bp[0], bp[1] + 102), (bp[0], bp[1] + 120), (bm[0], bp[1] + 120), bm, kind="gnd")
    return pins


def plug_out(s):
    """Receiver output: WM8960 headphone outputs to the 3.5 mm plug through 47 ohm resistors."""
    p = s.box(220, 160, 130, "P1", "3.5 mm plug (P3)", right=["Tip", "Ring", "Sleeve"])
    s.text(220, 330, "For P10: push on a 3.5 mm female", 12, color=MUTED)
    s.text(220, 346, "to 6.35 mm male adapter.", 12, color=MUTED)
    return p


def rx_wiring(s, wm, p):
    a, b = s.res(450, 225, "R1", "47 Ω", vertical=False)
    s.wire(p["Tip"], a, kind="audio")
    s.wire(b, wm["HP_L"], kind="audio")
    a, b = s.res(530, 255, "R2", "47 Ω", vertical=False)
    s.wire(p["Ring"], a, kind="audio")
    s.wire(b, wm["HP_R"], kind="audio")
    s.wire(p["Sleeve"], wm["HP GND"], kind="gnd")


def nrf24(s, x):
    """nRF24L01+ next to the XIAO ESP32-C3 right side (D10..D7)."""
    n = s.box(x, 160, 150, "U4", "nRF24L01+",
              left=[None, None, None, "MOSI", "MISO", "SCK", "CSN"],
              right=["CE", "IRQ", None, "VCC", "GND"])
    return n


def option_b(s, xp, csn_x=1125):
    n = nrf24(s, 1180)
    for d, f in (("D10", "MOSI"), ("D9", "MISO"), ("D8", "SCK"), ("D7", "CSN")):
        s.wire(xp[d], n[f], kind="spi")
    # CSN pull-up keeps the radio quiet while the ESP32-C3 boots (GPIO9 is a boot pin)
    y = n["CSN"][1]
    s.node(csn_x, y, "spi")
    a, b = s.res(csn_x, y + 35, "R6", "10 kΩ")
    s.wire((csn_x, y), a)
    s.wire(b, (csn_x, y + 70), (csn_x + 15, y + 70))
    s.tag(csn_x + 15, y + 70, "3V3", side="right")
    s.tag(*n["CE"], "CE", "spi", side="right")
    s.tag(*xp["D6"], "CE", "spi")
    s.nc(*n["IRQ"])
    v, g = n["VCC"], n["GND"]
    s.wire(v, (1470, v[1]), kind="pwr")
    s.node(1390, v[1], "pwr")
    s.node(1470, v[1], "pwr")
    s.tag(1470, v[1] - 4, "3V3", side="up")
    for cx, ref, val, polar in ((1390, "C6", "10 µF", True), (1470, "C7", "100 nF", False)):
        a, b = s.cap(cx, v[1] + 35, ref, val, polar=polar)
        s.wire((cx, v[1]), a)
        s.wire(b, (cx, v[1] + 60))
        s.gnd(cx, v[1] + 60)
    s.gnd(*g)
    s.text(1180, 470, "C6 and C7: solder directly on the", 12, color=MUTED)
    s.text(1180, 486, "nRF24 VCC and GND pins.", 12, color=MUTED)


TX_LEFT = ["LINPUT1", "VCC", "GND"]
RX_LEFT = ["HP_L", "HP_R", "HP GND", "VCC", "GND"]


def tx_right(data):
    return ["MCLK", "BCLK", "LRCK", data, "SDA", "SCL"]


def build_wireless():
    common = ("Pins drawn in the XIAO's real order: D0 to D6 on one side; 5V, GND, 3V3, D10 to D7 on the other; "
              "battery pads underneath. Labels with the same name (3V3, Vref, CE) are connected.")

    # buffer explained
    s = Sheet(760, 720, "How the transmitter input buffer works",
              "MCP6002 voltage follower. Point 1, the guitar tip: a small wave of about plus and minus 0.5 volt around 0 volt. "
              "C1 blocks DC and R2 lifts the wave to Vref, 1.65 volt, half of the 3.3 volt supply, so the op-amp on a single "
              "supply can follow both halves; point 2. The op-amp output is wired back to its minus input, so it copies "
              "its plus input with a gain of 1 but can drive the WM8960. C3 removes the 1.65 volt again; point 3, the same "
              "wave around 0 volt. R1 keeps the input at 0 volt when nothing is plugged in. R3 and R4 divide 3.3 volt into "
              "Vref, C2 keeps it quiet. The unused second half of the chip is parked as a follower on Vref.")
    c3 = buffer(s)
    s.wire(c3, (575, c3[1]), kind="audio")
    s.tag(575, c3[1], "to WM8960 LINPUT1", "audio", side="right")

    def probe(n, px, py, bx, center, l1, l2):
        bw, bh, by = 150, 80, 22
        s.add(f'<rect x="{bx}" y="{by}" width="{bw}" height="{bh}" rx="4" fill="{FILL}" stroke="{MUTED}"/>')
        yv = lambda v: by + bh - (v + 1) * bh / 4.3  # axis from -1 V to 3.3 V
        for v, lab in ((0, "0 V"), (1.65, "1.65 V")):
            s.wire((bx, yv(v)), (bx + bw, yv(v)), kind="gnd", dash=v != 0)
            s.text(bx + bw - 4, yv(v) - 3, lab, 10, "end", color=MUTED)
        pts = " ".join(f"{bx + i * bw / 60:.1f},{yv(center + 0.5 * math.sin(i / 60 * 4 * math.pi)):.1f}" for i in range(61))
        s.add(f'<polyline points="{pts}" fill="none" stroke="{KIND["audio"]}" stroke-width="2"/>')
        s.text(bx, by + bh + 16, l1, 11)
        s.text(bx, by + bh + 30, l2, 11, color=MUTED)
        s.wire((bx + bw / 2, by + bh + 36), (px, py), kind="gnd", dash=True)
        for cx, cy in ((bx + 12, by + 12), (px, py)):
            s.add(f'<circle cx="{cx}" cy="{cy}" r="9" fill="{INK}"/>')
            s.text(cx, cy + 4, str(n), 11, "middle", "bold", PAPER)

    probe(1, 200, 245, 40, 0, "Guitar: about ±0.5 V", "around 0 V")
    probe(2, 300, 245, 230, 1.65, "Same wave, lifted", "to 1.65 V by R2")
    probe(3, 560, c3[1], 570, 0, "Same wave, back to 0 V,", "now able to drive the WM8960")
    s.save("wireless-guitar-system/schematics/buffer-explained.svg")

    # option A transmitter
    s = Sheet(1240, 700, "Transmitter, option A: XIAO nRF52840",
              "Guitar plug P1 into the MCP6002 buffer, then C3 into WM8960 LINPUT1. WM8960 MCLK, BCLK, LRCK, ADCDAT, SDA, "
              "SCL connect to XIAO D0, D1, D2, D3, D4, D5. WM8960 and buffer run on the XIAO 3V3. LiPo battery through "
              "switch SW1 to the XIAO BAT+ and BAT- pads. " + common)
    heading(s, "Transmitter, option A: XIAO nRF52840", "WM8960 is the I2S clock master. XIAO D0 gives it a 16 MHz MCK. Status: RGB LED on the XIAO.")
    c3 = buffer(s)
    wm = wm8960(s, TX_LEFT, tx_right("ADCDAT"))
    s.wire(c3, wm["LINPUT1"], kind="audio")
    xp = xiao(s, "XIAO nRF52840", wm, "ADCDAT")
    for d in ("D6", "D10", "D9", "D8", "D7"):
        s.nc(*xp[d])
    s.save("wireless-guitar-system/schematics/tx-option-a.svg")

    # option A receiver
    s = Sheet(1240, 640, "Receiver, option A: XIAO nRF52840",
              "WM8960 headphone outputs HP_L and HP_R through 47 ohm resistors R1 and R2 to the tip and ring of the 3.5 mm "
              "plug; headphone ground to the sleeve. WM8960 MCLK, BCLK, LRCK, DACDAT, SDA, SCL connect to XIAO D0 to D5. "
              "LiPo battery through SW1 to the XIAO battery pads. " + common)
    heading(s, "Receiver, option A: XIAO nRF52840", "Same as the transmitter, but D3 sends audio to the WM8960 and its headphone amp drives the plug.")
    wm = wm8960(s, RX_LEFT, tx_right("DACDAT"))
    rx_wiring(s, wm, plug_out(s))
    xp = xiao(s, "XIAO nRF52840", wm, "DACDAT")
    for d in ("D6", "D10", "D9", "D8", "D7"):
        s.nc(*xp[d])
    s.save("wireless-guitar-system/schematics/rx-option-a.svg")

    # option B transmitter and receiver
    for side, data, title in (("tx", "ADCDAT", "Transmitter"), ("rx", "DACDAT", "Receiver")):
        s = Sheet(1560, 720 if side == "tx" else 640, f"{title}, option B: XIAO ESP32-C3 + nRF24L01+",
                  f"Same as option A {title.lower()}, with the XIAO ESP32-C3 as I2S master giving 12.288 MHz MCLK, and an "
                  "nRF24L01+ radio on SPI: XIAO D10 to MOSI, D9 to MISO, D8 to SCK, D7 to CSN with a 10 kilohm pull-up to "
                  "3V3, D6 to CE. 10 microfarad and 100 nanofarad capacitors on the nRF24 power pins. " + common)
        heading(s, f"{title}, option B: XIAO ESP32-C3 + nRF24L01+",
                "XIAO ESP32-C3 is the I2S clock master (12.288 MHz MCLK on D0). No status LED: status over USB serial.")
        if side == "tx":
            c3 = buffer(s)
            wm = wm8960(s, TX_LEFT, tx_right(data))
            s.wire(c3, wm["LINPUT1"], kind="audio")
        else:
            wm = wm8960(s, RX_LEFT, tx_right(data))
            rx_wiring(s, wm, plug_out(s))
        xp = xiao(s, "XIAO ESP32-C3", wm, data)
        option_b(s, xp)
        s.save(f"wireless-guitar-system/schematics/{side}-option-b.svg")

    # system overview
    s = Sheet(1180, 250, "Wireless guitar system overview",
              "Guitar, then the transmitter: buffer, WM8960 ADC, XIAO with radio. Radio link at 2.4 GHz. Receiver: XIAO "
              "with radio, WM8960 DAC and headphone amp, then a 3.5 mm plug to headphones or a line input, or through an "
              "adapter to a 6.35 mm amp input.", legend=False)
    blocks = [(20, "Guitar", "pickups"), (200, "Buffer", "MCP6002"), (340, "ADC", "WM8960"), (480, "Radio", "XIAO"),
              (700, "Radio", "XIAO"), (840, "DAC + amp", "WM8960"), (980, "Output", "3.5 mm plug")]
    for x, t, sub in blocks:
        s.add(f'<rect x="{x}" y="90" width="120" height="64" rx="6" fill="{FILL}" stroke="{INK}" stroke-width="2"/>')
        s.text(x + 60, 118, t, 14, "middle", "bold")
        s.text(x + 60, 138, sub, 12, "middle", color=MUTED)
    for (x1, *_), (x2, *_) in zip(blocks, blocks[1:]):
        if x1 == 480:
            continue  # the radio link is drawn as waves
        s.wire((x1 + 120, 122), (x2 - 8, 122), kind="audio")
        s.add(f'<path d="M{x2-8} 116 L{x2-8} 128 L{x2} 122 Z" fill="{KIND["audio"]}"/>')
    for r in (12, 24, 36):
        s.add(f'<path d="M{604 + r} {122 - r} A{r} {r} 0 0 1 {604 + r} {122 + r}" stroke="{KIND["i2s"]}" stroke-width="2" fill="none"/>')
    s.text(650, 200, "2.4 GHz, about 4-5 ms", 12, "middle", color=MUTED)
    s.add(f'<rect x="185" y="70" width="430" height="104" rx="10" fill="none" stroke="{MUTED}" stroke-dasharray="6 5"/>')
    s.add(f'<rect x="685" y="70" width="430" height="104" rx="10" fill="none" stroke="{MUTED}" stroke-dasharray="6 5"/>')
    s.text(195, 62, "TRANSMITTER (plugs into the guitar, LiPo)", 12, weight="bold", color=MUTED)
    s.text(695, 62, "RECEIVER (LiPo)", 12, weight="bold", color=MUTED)
    s.text(1040, 200, "P3: headphones, line in", 12, "middle", color=MUTED)
    s.text(1040, 216, "P10: with adapter, to the amp", 12, "middle", color=MUTED)
    s.save("wireless-guitar-system/schematics/system-overview.svg")


def chain(s, blocks, y=90, w=120, gap=40, x0=20):
    """Row of labelled blocks joined by arrows. blocks = [(title, sub)]. Returns the x of each block."""
    xs = []
    for i, (t, sub) in enumerate(blocks):
        x = x0 + i * (w + gap)
        xs.append(x)
        s.add(f'<rect x="{x}" y="{y}" width="{w}" height="64" rx="6" fill="{FILL}" stroke="{INK}" stroke-width="2"/>')
        s.text(x + w / 2, y + 28, t, 14, "middle", "bold")
        s.text(x + w / 2, y + 48, sub, 12, "middle", color=MUTED)
        if i:
            s.wire((x - gap, y + 32), (x - 8, y + 32), kind="audio")
            s.add(f'<path d="M{x-8} {y+26} L{x-8} {y+38} L{x} {y+32} Z" fill="{KIND["audio"]}"/>')
    return xs


def build_digital():
    # overview
    s = Sheet(1180, 300, "Digital pedal overview",
              "Guitar into a TL072 buffer, PCM1808 ADC, ESP32 running pedal.h (Tube Screamer, Klon, Katana), PCM5102A DAC, "
              "master volume, amp. Seven pots, a pull switch and three footswitches tell the ESP32 the knob settings.", legend=False)
    xs = chain(s, [("Guitar", "J1 in"), ("Buffer", "TL072"), ("ADC", "PCM1808"), ("ESP32", "pedal.h"),
                   ("DAC", "PCM5102A"), ("Volume", "RV8 master"), ("Amp", "J2 out")], y=60, gap=45)
    x = xs[3] + 60
    s.wire((x, 210), (x, 132), kind="ink")
    s.add(f'<path d="M{x-6} 132 L{x+6} 132 L{x} 124 Z" fill="{INK}"/>')
    s.add(f'<rect x="{x-170}" y="210" width="340" height="56" rx="6" fill="{FILL}" stroke="{INK}" stroke-width="2"/>')
    s.text(x, 234, "Front panel", 14, "middle", "bold")
    s.text(x, 254, "7 pots, pull switch, 3 footswitches, 3 LEDs", 12, "middle", color=MUTED)
    s.save("digital-pedal/schematics/overview.svg")

    # audio path and power
    s = Sheet(1800, 760, "Digital pedal: audio path and power",
              "J1 input jack into a TL072 voltage follower biased at 4.5 volt, then C3 into the PCM1808 VINL. PCM1808 SCK, BCK, LRC, "
              "OUT to ESP32 GPIO0, 26, 25, 19. ESP32 GPIO22 to PCM5102A DIN; BCK and LRCK shared with the PCM5102A. PCM5102A "
              "SCK to ground. PCM5102A LOUT through R5 1 kilohm to the RV8 master volume pot, wiper to the J2 output jack. "
              "Power: 9 volt DC jack J3, D1 1N5817, C5 100 microfarad, AMS1117-5.0 regulator to 5 volt. The TL072 runs on 9 volt.")
    heading(s, "Digital pedal: audio path and power", "ESP32 is the I2S clock master (12.288 MHz MCLK on GPIO0). ESP32 pins drawn in logical order.")
    c3 = buffer(s, ic="U4", part="TL072", supply="9V", vref="4.5 V", c2="47 µF", c3="10 µF", jack=("J1", "6.35 mm in"))
    adc = s.box(620, 160, 170, "U2", "PCM1808 (ADC)", left=["VINL", "VCC", "VDD", "GND", "FMT", "MD0", "MD1"],
                right=["SCK", None, "BCK", None, "LRC", None, "OUT"])
    s.wire(c3, adc["VINL"], kind="audio")
    for pin, net in (("VCC", "5V"), ("VDD", "3V3")):
        s.wire(adc[pin], (adc[pin][0] - 20, adc[pin][1]), kind="pwr")
        s.tag(adc[pin][0] - 20, adc[pin][1], net)
    for pin in ("GND", "FMT", "MD0", "MD1"):
        s.wire(adc[pin], (adc[pin][0] - 10, adc[pin][1]), kind="gnd")
        s.gnd(adc[pin][0] - 10, adc[pin][1])
    s.text(620, 440, "VINR: not connected", 12, color=MUTED)
    esp = s.box(900, 160, 190, "U1", "ESP32 DevKit, 38 pin", left=["GPIO0", None, "GPIO26", None, "GPIO25", None, "GPIO19"],
                right=["GPIO22", None, "VIN (5V)", "3V3", "GND"])
    for a, b, lab in (("SCK", "GPIO0", "MCLK"), ("BCK", "GPIO26", "BCK"), ("LRC", "GPIO25", "LRCK"), ("OUT", "GPIO19", "DATA")):
        s.wire(adc[a], esp[b], kind="i2s")
        s.text(845, adc[a][1] - 6, lab, 11, "middle", color=MUTED)
    for pin, net in (("BCK", "BCK"), ("LRC", "LRCK")):
        y = adc[pin][1]
        s.node(865, y, "i2s")
        s.wire((865, y), (865, y + 10), kind="i2s")
        s.tag(865, y + 10, net, "i2s", side="down")
    s.wire(esp["VIN (5V)"], (esp["VIN (5V)"][0] + 10, esp["VIN (5V)"][1]), kind="pwr")
    s.tag(esp["VIN (5V)"][0] + 10, esp["VIN (5V)"][1], "5V", side="right")
    s.wire(esp["3V3"], (esp["3V3"][0] + 10, esp["3V3"][1]), kind="pwr")
    s.tag(esp["3V3"][0] + 10, esp["3V3"][1], "3V3", side="right")
    g = esp["GND"]
    s.wire(g, (g[0] + 30, g[1]), kind="gnd")
    s.gnd(g[0] + 30, g[1])
    dac = s.box(1290, 160, 170, "U3", "PCM5102A (DAC)", left=["DIN", "BCK", "LCK", "VIN", "GND", "SCK"], right=["LOUT"])
    s.wire(esp["GPIO22"], dac["DIN"], kind="i2s")
    s.tag(*dac["BCK"], "BCK", "i2s")
    s.tag(*dac["LCK"], "LRCK", "i2s")
    s.tag(*dac["VIN"], "5V")
    for pin in ("GND", "SCK"):
        s.wire(dac[pin], (dac[pin][0] - 10, dac[pin][1]), kind="gnd")
        s.gnd(dac[pin][0] - 10, dac[pin][1])
    s.text(1290, 412, "Jumpers on the back: FLT L, DEMP L,", 12, color=MUTED)
    s.text(1290, 428, "XSMT H, FMT L. SCK to GND = internal PLL.", 12, color=MUTED)
    a, b = s.res(1520, dac["LOUT"][1], "R5", "1 kΩ", vertical=False)
    s.wire(dac["LOUT"], a, kind="audio")
    top, bot, wip = s.pot(1590, 245, "RV8", "A10K master")
    s.wire(b, (1590, 225), top, kind="audio")
    s.wire(bot, (1590, 280))
    s.gnd(1590, 280)
    out = s.box(1660, 180, 120, "J2", "6.35 mm out", left=["Tip", "Sleeve"])
    s.wire(wip, out["Tip"], kind="audio")
    s.gnd(*out["Sleeve"])
    # power
    pj = s.box(900, 480, 130, "J3", "DC jack 9 V", right=["+ (sleeve)", "− (center)"])
    s.gnd(*pj["− (center)"])
    an, ca = s.diode(1085, 545, "D1", "1N5817")
    s.wire(pj["+ (sleeve)"], an, kind="pwr")
    reg = s.box(1250, 480, 140, "U5", "AMS1117-5.0", left=["IN", "GND"], right=["OUT"])
    s.wire(ca, reg["IN"], kind="pwr")
    s.node(1125, 545, "pwr")
    s.tag(1125, 545, "9V", side="up")
    s.node(1160, 545, "pwr")
    a, b = s.cap(1160, 580, "C5", "100 µF", polar=True)
    s.wire((1160, 545), a, kind="pwr")
    s.gnd(*b)
    s.wire(reg["GND"], (reg["GND"][0], 600), kind="gnd")
    s.gnd(reg["GND"][0], 600)
    s.wire(reg["OUT"], (1500, 545), kind="pwr")
    s.node(1450, 545, "pwr")
    a, b = s.cap(1450, 580, "C7", "10 µF", polar=True)
    s.wire((1450, 545), a, kind="pwr")
    s.gnd(*b)
    s.tag(1500, 545, "5V", side="right")
    s.text(900, 665, "U4 TL072 runs on 9V. ESP32, PCM1808 and PCM5102A run on 5V; the ESP32 makes the 3V3.", 12, color=MUTED)
    s.text(900, 681, "C6 10 µF on the regulator input only if the module has none. 100 nF (C8-C10) at the U1, U2, U3 power pins.", 12, color=MUTED)
    s.save("digital-pedal/schematics/audio-and-power.svg")

    # front panel
    pots = [("RV1", "GPIO32", "TS DRIVE"), ("RV2", "GPIO33", "TS TONE"), ("RV3", "GPIO34", "TS LEVEL"),
            ("RV4", "GPIO35", "KLON GAIN"), ("RV5", "GPIO36", "KLON TREBLE"), ("RV6", "GPIO39", "KLON OUTPUT"),
            ("RV7", "GPIO4", "KATANA VOLUME")]
    right = [("GPIO23", "sw", "RV7 pull switch", "Katana dirty mode"), ("GPIO13", "sw", "SW1", "TS on/off"),
             ("GPIO14", "sw", "SW2", "Klon on/off"), ("GPIO27", "sw", "SW3", "Katana on/off"),
             ("GPIO16", "led", "R6, LED1", "TS"), ("GPIO17", "led", "R7, LED2", "Klon"), ("GPIO18", "led", "R8, LED3", "Katana")]
    s = Sheet(1180, 820, "Digital pedal: front panel",
              "Seven 10 kilohm linear pots: one outer pin to 3V3, the other to ground, the wiper to ESP32 GPIO 32, 33, 34, 35, 36, 39 "
              "and 4, each wiper with a 100 nanofarad capacitor to ground. RV7 pull switch to GPIO23. Footswitches SW1, SW2, SW3 "
              "to GPIO13, 14, 27, switching to ground with internal pull-ups. LEDs on GPIO16, 17, 18 through 1 kilohm resistors.")
    heading(s, "Digital pedal: front panel", "Pots: 10k linear (B). Switches close to GND (internal pull-ups). GPIO4 is ADC2: works only with Wi-Fi off.")
    esp = s.box(560, 110, 200, "U1", "ESP32 DevKit, 38 pin", left=[p[1] for p in pots], right=[r[0] for r in right], pitch=80)
    for i, (ref, pin, name) in enumerate(pots):
        y = esp[pin][1]
        top, bot, wip = s.pot(260, y, ref, "10k B")
        s.wire(top, (260, y - 28), (230, y - 28), kind="pwr")
        s.tag(230, y - 28, "3V3")
        s.wire(bot, (260, y + 28))
        s.gnd(260, y + 28)
        s.wire(wip, esp[pin])
        s.text(300, y - 6, name, 11, color=MUTED)
        s.node(420, y)
        a, b = s.cap(420, y + 25, f"C{11 + i}", "100 nF")
        s.wire((420, y), a)
        s.gnd(*b)
    for pin, kind, ref, note in right:
        x, y = esp[pin]
        if kind == "sw":
            s.wire((x, y), (840, y))
            s.switch(840, y, "", "")
            s.wire((840, y + 40), (840, y + 44))
            s.gnd(840, y + 44)
            s.text(865, y + 18, ref, 12, weight="bold")
            s.text(865, y + 33, note, 11, color=MUTED)
        else:
            a, b = s.res(840, y, "", "", vertical=False)
            s.text(840, y + 24, "1 kΩ", 11, "middle", color=MUTED)
            s.wire((x, y), a)
            an, ca = s.diode(910, y, "", "", led=True)
            s.wire(b, an)
            s.wire(ca, (950, y))
            s.gnd(950, y)
            s.text(980, y + 4, ref, 12, weight="bold")
            s.text(980, y + 19, note, 11, color=MUTED)
    s.save("digital-pedal/schematics/front-panel.svg")


def build_analog():
    s = Sheet(1240, 300, "Tube Screamer TS808 block diagram",
              "Input jack, 3PDT footswitch, transistor input buffer, clipping stage with the first half of the JRC4558 and the "
              "DRIVE pot, tone stage with the second half and the TONE pot, LEVEL pot, transistor output buffer, footswitch, "
              "output jack. With the footswitch off, the input jack connects straight to the output jack (true bypass).", legend=False)
    xs = chain(s, [("In", "jack"), ("Input buffer", "2SC1815"), ("Clipping", "4558 A, DRIVE"), ("Tone", "4558 B, TONE"),
                   ("Level", "LEVEL pot"), ("Output buffer", "2SC1815"), ("Out", "jack")], y=110, w=140, gap=34)
    s.add(f'<path d="M{xs[0]+70} 110 V70 H{xs[-1]+70} V110" fill="none" stroke="{MUTED}" stroke-width="2" stroke-dasharray="6 5"/>')
    s.text((xs[0] + xs[-1]) / 2 + 70, 62, "Footswitch off: true bypass (3PDT connects In straight to Out)", 12, "middle", color=MUTED)
    s.text(20, 230, "Power: 9 V, 1N4001 polarity protection, 2 x 10 kΩ + 47 µF make Vref = 4.5 V for the op-amp.", 12, color=MUTED)
    s.text(20, 248, "Values from the TS808 schematic as described in the ElectroSmash analysis. Use the parts list of the PCB you build.", 12, color=MUTED)
    s.save("analog-pedals/schematics/ts808-overview.svg")

    s = Sheet(1040, 540, "TS808 clipping stage",
              "Non-inverting op-amp stage, first half of the JRC4558. Plus input from the input buffer. Minus input to Vref through "
              "4.7 kilohm and 47 nanofarad, a high-pass at about 720 hertz that makes the mid hump. Feedback from output to minus "
              "input: 51 kilohm in series with the 500 kilohm DRIVE pot, in parallel with 51 picofarad and two 1N914 diodes in "
              "opposite directions. Gain 12 to 118.")
    heading(s, "TS808 clipping stage", "The heart of the Tube Screamer. Gain = 1 + (51k + DRIVE) / 4.7k = about 12x to 118x.")
    m, p, o = s.opamp(420, 300, 100, "U1A  JRC4558", ("2", "3", "1"))
    s.wire((400, 450), (400, p[1]), p, kind="audio")
    s.tag(400, 450, "from input buffer", "audio", side="down")
    s.node(380, m[1])
    s.wire((380, m[1]), m)
    a, b = s.res(330, m[1], "R", "4.7 kΩ", vertical=False)
    s.wire((380, m[1]), b)
    s.wire(a, (280, m[1]))
    a, b = s.cap(280, m[1] + 30, "C", "47 nF")
    s.wire((280, m[1]), a)
    s.wire(b, (280, m[1] + 55))
    s.tag(280, m[1] + 55, "Vref 4.5 V", "ink", side="down")
    s.node(580, o[1], "audio")
    s.wire(o, (640, o[1]), kind="audio")
    s.tag(640, o[1], "to tone stage", "audio", side="right")
    s.wire((380, m[1]), (380, 95))
    s.wire((580, o[1]), (580, 95))
    for y in (250, 195, 140, 95):
        s.node(380, y)
        s.node(580, y)
    a, b = s.res(430, 250, "R", "51 kΩ", vertical=False)
    s.wire((380, 250), a)
    a2, b2 = s.res(520, 250, "DRIVE", "500 kΩ A", vertical=False)
    s.wire(b, a2)
    s.wire(b2, (580, 250))
    a, b = s.cap(480, 195, "C", "51 pF", vertical=False)
    s.wire((380, 195), a)
    s.wire(b, (580, 195))
    an, ca = s.diode(480, 140, "", "")
    s.wire((380, 140), an)
    s.wire(ca, (580, 140))
    an, ca = s.diode(480, 95, "", "", flip=True)
    s.wire((380, 95), ca)
    s.wire(an, (580, 95))
    s.text(600, 122, "D1, D2 1N914, opposite directions", 12, color=MUTED)
    for i, t in enumerate(["4.7 kΩ + 47 nF: high-pass at about 720 Hz = the mid hump",
                           "51 kΩ + DRIVE: sets the gain",
                           "51 pF: cuts high-frequency fizz",
                           "Two diodes, opposite directions: soft, symmetric clipping"]):
        s.text(620, 400 + i * 18, t, 12, color=MUTED)
    s.save("analog-pedals/schematics/ts808-clipping-stage.svg")


if __name__ == "__main__":
    build_wireless()
    build_digital()
    build_analog()
