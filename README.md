# video primitives for Portenta H7 via USBC (to HDMI adapter)

> [!WARNING] 
> Working in progress, help are welcome!

# Example

![alt Screen](https://github.com/dmazzella/anx7625/blob/main/tests/main.png?raw=true)

```python
import _anx7625

import framebuf
import machine
import time


def main():
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
        height=height
    )

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

    while True:
        for i in range(5):
            fbuf.rect(80 + i * 30, 140 + i * 20, 60, 60, 0x177A, True)

        fbuf.vline(4, 4, 12, 0)
        fbuf.vline(8, 1, 12, 0)
        fbuf.vline(12, 4, 12, 0)
        fbuf.vline(14, 13, 2, 0)



if __name__ == "__main__":
    main()

```
