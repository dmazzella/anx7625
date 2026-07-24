#  LVGL on HDMI (ANX7625) demo / bring-up -- asyncio version.
#
#  Same UI and pipeline as main_lvgl.py, but the single blocking event loop is
#  split into cooperative asyncio tasks that yield with asyncio.sleep_ms:
#    * lvgl_pump()  drives LVGL's time base + task handler and reports FPS,
#    * animate()    self-animates the value-driven widgets,
#    * blink()      toggles the LED ~1 Hz.
#
#  Pipeline: LVGL renders dirty areas into its own partial draw buffer, then our
#  flush_cb blits each area into the ANX7625 visible framebuffer with a DMA2D
#  copy (anx.image), which the LTDC scans out to the HDMI display. No input
#  device is wired (HDMI is output only), so the UI self-animates to prove the
#  render -> flush -> scan-out path works end to end.
#
#  Copy this file to the board as main.py (the firmware must be the LVGL build).
#
#  If colours look swapped (red<->blue) enable LV_COLOR_16_SWAP in lv_conf.h and
#  rebuild. Some method/enum names are LVGL v9 specific; if one is missing,
#  introspect on the board with e.g. print(dir(lv)) / print(dir(lv.bar)).

import _anx7625

import asyncio
import framebuf
import io
import lvgl as lv
import machine
import sys
import time


def show_error(anx, text):
    # No REPL is attached, so render the traceback onto the HDMI screen itself
    # via the known-good framebuf path, then keep it displayed.
    fb = framebuf.FrameBuffer(anx.framebuffer, anx.width, anx.height, framebuf.RGB565)
    fb.fill(0x0000)
    y = 8
    for raw in text.split("\n"):
        while True:
            fb.text(raw[:98], 6, y, 0xFFFF)
            y += 10
            raw = raw[98:]
            if not raw:
                break
    anx.flush()


async def run_lvgl(anx):
    # LVGL is auto-initialized by the binding on import (its mp_lv_init_gc calls
    # lv_init), so there is no lv.init() to call at module level.

    # Partial draw buffer: LVGL renders the screen in horizontal chunks of ~100
    # lines; the flush callback blits each dirty area into the ANX7625 visible
    # framebuffer with a DMA2D copy (anx.image), which the LTDC scans out.
    draw_buf = bytearray(anx.width * 100 * 2)  # RGB565, 2 bytes/pixel

    disp = lv.display_create(anx.width, anx.height)
    try:
        disp.set_color_format(lv.COLOR_FORMAT.RGB565)
    except AttributeError:
        pass  # RGB565 is already native for LV_COLOR_DEPTH 16
    disp.set_buffers(draw_buf, None, len(draw_buf), lv.DISPLAY_RENDER_MODE.PARTIAL)

    def flush_cb(disp, area, px_map):
        w = (area.x2 - area.x1) + 1
        h = (area.y2 - area.y1) + 1
        # px_map points at w*h RGB565 pixels; anx.image reads it read-only and
        # DMA2D-blits it into the visible framebuffer (no copy).
        anx.image(px_map.__dereference__(w * h * 2), width=w, height=h, x=area.x1, y=area.y1)
        disp.flush_ready()

    disp.set_flush_cb(flush_cb)

    # ---- UI (white theme) ----
    scr = lv.screen_active()
    scr.set_style_bg_color(lv.color_hex(0xFFFFFF), 0)

    TEXT = lv.color_hex(0x1A2233)

    title = lv.label(scr)
    title.set_text("ANX7625 + LVGL on MicroPython (asyncio)")
    title.set_style_text_color(TEXT, 0)
    title.align(lv.ALIGN.TOP_MID, 0, 16)

    fps_label = lv.label(scr)
    fps_label.set_text("-- fps")
    fps_label.set_style_text_color(TEXT, 0)
    fps_label.align(lv.ALIGN.TOP_RIGHT, -16, 12)

    # --- left column: button, switch, checkbox, LED, dropdown ---
    btn = lv.button(scr)
    btn.set_size(200, 56)
    btn.align(lv.ALIGN.TOP_LEFT, 40, 70)
    btn_label = lv.label(btn)
    btn_label.set_text("Button")
    btn_label.center()

    sw = lv.switch(scr)
    sw.align(lv.ALIGN.TOP_LEFT, 40, 150)
    sw.add_state(lv.STATE.CHECKED)
    sw_label = lv.label(scr)
    sw_label.set_text("Switch")
    sw_label.set_style_text_color(TEXT, 0)
    sw_label.align(lv.ALIGN.TOP_LEFT, 140, 158)

    cb = lv.checkbox(scr)
    cb.set_text("Checkbox")
    cb.add_state(lv.STATE.CHECKED)
    cb.set_style_text_color(TEXT, 0)
    cb.align(lv.ALIGN.TOP_LEFT, 40, 210)

    led = lv.led(scr)
    led.set_size(26, 26)
    led.align(lv.ALIGN.TOP_LEFT, 40, 262)
    led.set_color(lv.color_hex(0x2ECC71))
    led.set_brightness(255)
    led_label = lv.label(scr)
    led_label.set_text("LED")
    led_label.set_style_text_color(TEXT, 0)
    led_label.align(lv.ALIGN.TOP_LEFT, 80, 268)

    dd = lv.dropdown(scr)
    dd.set_options("Option 1\nOption 2\nOption 3")
    dd.align(lv.ALIGN.TOP_LEFT, 40, 310)

    # --- center: arc gauge (animated) ---
    arc = lv.arc(scr)
    arc.set_size(200, 200)
    arc.align(lv.ALIGN.CENTER, 0, 30)
    arc.set_range(0, 100)
    arc.set_value(0)
    arc_label = lv.label(scr)
    arc_label.set_style_text_color(TEXT, 0)
    arc_label.align(lv.ALIGN.CENTER, 0, 30)

    # --- right column: slider + bar (animated) ---
    slider = lv.slider(scr)
    slider.set_size(280, 14)
    slider.align(lv.ALIGN.TOP_RIGHT, -40, 100)
    slider.set_range(0, 100)

    bar = lv.bar(scr)
    bar.set_size(280, 22)
    bar.align(lv.ALIGN.TOP_RIGHT, -40, 160)
    bar.set_range(0, 100)
    bar.set_value(0, 0)  # 0 = LV_ANIM_OFF

    val_label = lv.label(scr)
    val_label.set_style_text_color(TEXT, 0)
    val_label.align(lv.ALIGN.TOP_RIGHT, -40, 210)

    # --- bottom-left: calendar (July 2026, today highlighted) ---
    cal_label = lv.label(scr)
    cal_label.set_text("Calendar")
    cal_label.set_style_text_color(TEXT, 0)
    cal_label.align(lv.ALIGN.TOP_LEFT, 40, 368)

    cal = lv.calendar(scr)
    cal.set_size(200, 200)
    cal.align(lv.ALIGN.TOP_LEFT, 40, 388)
    cal.set_today_date(2026, 7, 23)
    cal.set_month_shown(2026, 7)

    # --- bottom-right: table (last row shows the live animated value) ---
    tbl_label = lv.label(scr)
    tbl_label.set_text("Table")
    tbl_label.set_style_text_color(TEXT, 0)
    tbl_label.align(lv.ALIGN.TOP_RIGHT, -40, 290)

    tbl = lv.table(scr)
    tbl.set_column_count(2)
    tbl.set_row_count(4)
    tbl.set_column_width(0, 120)
    tbl.set_column_width(1, 90)
    tbl.set_cell_value(0, 0, "Signal")
    tbl.set_cell_value(0, 1, "Value")
    tbl.set_cell_value(1, 0, "Voltage")
    tbl.set_cell_value(1, 1, "3.30 V")
    tbl.set_cell_value(2, 0, "Current")
    tbl.set_cell_value(2, 1, "120 mA")
    tbl.set_cell_value(3, 0, "Load")
    tbl.set_cell_value(3, 1, "0%")
    tbl.align(lv.ALIGN.TOP_RIGHT, -40, 312)

    # ---- Cooperative asyncio tasks. Each runs its own loop and yields with
    #      asyncio.sleep_ms so the scheduler can interleave the others.
    async def lvgl_pump():
        # Drive LVGL's time base + task handler and report the render rate.
        frames = 0
        fps_t0 = time.ticks_ms()
        prev = fps_t0
        while True:
            now = time.ticks_ms()
            lv.tick_inc(time.ticks_diff(now, prev))
            prev = now
            lv.timer_handler()
            frames += 1
            if time.ticks_diff(time.ticks_ms(), fps_t0) >= 1000:
                fps_label.set_text("%d fps" % frames)
                frames = 0
                fps_t0 = time.ticks_ms()
            await asyncio.sleep_ms(16)

    async def animate():
        # Self-animate the value-driven widgets (no input device on HDMI-out).
        value = 0
        step = 1
        while True:
            value += step
            if value >= 100 or value <= 0:
                step = -step
                value = max(0, min(100, value))
            bar.set_value(value, 0)  # 0 = LV_ANIM_OFF
            slider.set_value(value, 0)
            arc.set_value(value)
            val_label.set_text("value %d%%" % value)
            arc_label.set_text("%d%%" % value)
            tbl.set_cell_value(3, 1, "%d%%" % value)
            await asyncio.sleep_ms(16)

    async def blink():
        # Blink the LED ~1 Hz (toggle on/off every 500 ms).
        while True:
            led.toggle()
            await asyncio.sleep_ms(500)

    await asyncio.gather(lvgl_pump(), animate(), blink())


def main():
    # The ANX7625 / HDMI hardware is set up first, outside the asyncio try/except,
    # so that if LVGL raises we still have a working display to print the error.
    i2c = machine.I2C(1, freq=400_000)
    video_on = machine.Pin.cpu.K2
    video_rst = machine.Pin.cpu.J3
    otg_on = machine.Pin.cpu.J6

    width = 800
    height = 600
    # RGB565 double-buffered allocation required by the driver (2*2 bytes/pixel).
    anx_buffer = bytearray(width * height * 2 * 2)
    anx = _anx7625.ANX7625(
        i2c, video_on, video_rst, otg_on, anx_buffer, width=width, height=height
    )

    try:
        asyncio.run(run_lvgl(anx))
    except Exception as exc:
        buf = io.StringIO()
        sys.print_exception(exc, buf)
        show_error(anx, buf.getvalue())
        while True:
            time.sleep_ms(500)


if __name__ == "__main__":
    main()
