import _anx7625

import framebuf
import machine
import asyncio


async def anx_task(anx):
    while True:
        # anx.begin(0xfa97)

        buffer = bytearray(500 * 400 * 2)

        fbuf = framebuf.FrameBuffer(buffer, 500, 400, framebuf.RGB565)
        fbuf.fill(0xFA97)

        fbuf.text("ANX7625 Micropython porting", 180, 20, 0x177A)

        for i in range(5):
            fbuf.rect(180 + i * 30, 40 + i * 20, 60, 60, 0x177A, True)

        fbuf.fill_rect(1, 1, 15, 15, 0xFFFF)
        fbuf.vline(4, 4, 12, 0)
        fbuf.vline(8, 1, 12, 0)
        fbuf.vline(12, 4, 12, 0)
        fbuf.vline(14, 13, 2, 0)

        anx.image(buffer, width=500, height=400)

        # anx.end()

        await asyncio.sleep(0)


async def main():
    i2c = machine.I2C(1, freq=400_000)
    video_on = machine.Pin.cpu.K2
    video_rst = machine.Pin.cpu.J3
    otg_on = machine.Pin.cpu.J6
    mode = _anx7625.MODE_720x480_60Hz
    width = 720
    height = 480
    buffer = bytearray(width * height * 2)

    anx = _anx7625.ANX7625(
        i2c,
        video_on,
        video_rst,
        otg_on,
        mode,
        buffer,
        width=width,
        height=height,
        background_color=0x3433,
    )

    anx.begin()

    fbuf = framebuf.FrameBuffer(anx.buffer, anx.width, anx.height, framebuf.RGB565)
    fbuf.fill(0x3433)

    fbuf.text("ANX7625 Micropython porting", 80, 20, 0xFFFF)

    for i in range(5):
        fbuf.rect(80 + i * 30, 40 + i * 20, 60, 60, 0xECAE, True)

    fbuf.fill_rect(1, 1, 15, 15, 0xFFFF)
    fbuf.vline(4, 4, 12, 0)
    fbuf.vline(8, 1, 12, 0)
    fbuf.vline(12, 4, 12, 0)
    fbuf.vline(14, 13, 2, 0)

    anx.end()

    await asyncio.gather(
        asyncio.create_task(anx_task(anx)),
    )


if __name__ == "__main__":
    try:
        asyncio.run(main())
    finally:
        asyncio.new_event_loop()
