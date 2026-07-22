import _anx7625

import array
import framebuf
import machine
import math
import time


# ----------------------------------------------------------------------------
# Richer animated demo. Every frame the whole scene is redrawn into the hidden
# ping-pong buffer and presented with anx.flush() (single-layer page flip at the
# vertical blanking). This is a normal game loop: bouncing balls, two rotating
# polygons, a radar sweep, a dotted spinner and a progress bar.
# ----------------------------------------------------------------------------


def rgb(r, g, b):
    # 8-8-8 -> RGB565
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


BG = rgb(8, 10, 28)
PANEL = rgb(18, 22, 52)
WHITE = 0xFFFF
CYAN = rgb(33, 178, 242)
ORANGE = rgb(242, 140, 40)
GREEN = rgb(61, 214, 140)
PINK = rgb(240, 90, 155)
YELLOW = rgb(242, 211, 60)


def poly_regular(cx, cy, radius, sides, ang):
    # Absolute int16 coords for a regular polygon, ready for framebuf.poly().
    pts = []
    for i in range(sides):
        a = ang + i * 2.0 * math.pi / sides
        pts.append(int(cx + radius * math.cos(a)))
        pts.append(int(cy + radius * math.sin(a)))
    return array.array("h", pts)


def draw_spinner(fb, cx, cy, radius, head, n, base):
    # Classic dotted loader: n dots on a circle, brightness trailing the head.
    for i in range(n):
        a = 2.0 * math.pi * i / n
        x = int(cx + radius * math.cos(a))
        y = int(cy + radius * math.sin(a))
        f = 1.0 - ((head - i) % n) / n
        col = rgb(int(base[0] * f) + 18, int(base[1] * f) + 18, int(base[2] * f) + 22)
        r = 3 + int(3 * f)
        fb.ellipse(x, y, r, r, col, True)


def main():
    i2c = machine.I2C(1, freq=400_000)
    video_on = machine.Pin.cpu.K2
    video_rst = machine.Pin.cpu.J3
    otg_on = machine.Pin.cpu.J6

    width = 800
    height = 600
    buffer = bytearray(width * height * 2 * 2)

    anx = _anx7625.ANX7625(i2c, video_on, video_rst, otg_on, buffer, width=width, height=height)
    W = anx.width
    H = anx.height

    # Bouncing balls: [x, y, vx, vy, r, color].
    balls = [
        [140.0, 180.0, 3.4, 2.3, 26, ORANGE],
        [520.0, 300.0, -2.8, 3.1, 20, GREEN],
        [360.0, 420.0, 3.9, -2.2, 32, PINK],
        [650.0, 210.0, -3.1, -3.6, 18, YELLOW],
    ]
    top = 60  # keep objects below the header
    bottom = H - 50  # keep objects above the footer

    ang = 0.0
    prog = 0.0
    frame = 0
    head = 0

    while True:
        fb = framebuf.FrameBuffer(anx.framebuffer, W, H, framebuf.RGB565)

        # background + header/footer panels
        fb.fill(BG)
        fb.fill_rect(0, 0, W, 44, PANEL)
        fb.fill_rect(0, H - 44, W, 44, PANEL)
        fb.text("ANX7625 MicroPython - HDMI demo", 20, 16, WHITE)
        fb.text("frame %d" % frame, W - 110, 16, CYAN)

        # bouncing balls
        for b in balls:
            b[0] += b[2]
            b[1] += b[3]
            if b[0] - b[4] < 0:
                b[0] = b[4]
                b[2] = -b[2]
            elif b[0] + b[4] > W:
                b[0] = W - b[4]
                b[2] = -b[2]
            if b[1] - b[4] < top:
                b[1] = top + b[4]
                b[3] = -b[3]
            elif b[1] + b[4] > bottom:
                b[1] = bottom - b[4]
                b[3] = -b[3]
            fb.ellipse(int(b[0]), int(b[1]), b[4], b[4], b[5], True)

        # rotating filled square with outline
        sq = poly_regular(170, 300, 70, 4, ang)
        fb.poly(0, 0, sq, CYAN, True)
        fb.poly(0, 0, sq, WHITE, False)

        # rotating triangle (spins the other way)
        tri = poly_regular(W - 170, 300, 72, 3, -ang * 1.3)
        fb.poly(0, 0, tri, ORANGE, True)

        # radar sweep in the centre
        cx = W // 2
        cy = H // 2
        fb.ellipse(cx, cy, 78, 78, rgb(40, 48, 90), False)
        fb.line(cx, cy, int(cx + 78 * math.cos(ang * 2)), int(cy + 78 * math.sin(ang * 2)), GREEN)

        # dotted spinner loader
        draw_spinner(fb, cx, 150, 32, head, 12, (90, 200, 250))

        # progress bar in the footer
        bx = 20
        by = H - 30
        bw = W - 230
        bh = 16
        fb.rect(bx, by, bw, bh, WHITE)
        fb.fill_rect(bx + 2, by + 2, int((bw - 4) * prog), bh - 4, GREEN)
        fb.text("loading %3d%%" % int(prog * 100), bx + bw + 15, by + 4, WHITE)

        anx.flush()

        # advance animation state
        ang += 0.06
        prog += 0.008
        if prog > 1.0:
            prog = 0.0
        frame += 1
        if frame % 3 == 0:
            head = (head + 1) % 12
        time.sleep_ms(16)


if __name__ == "__main__":
    main()
