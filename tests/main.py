import _anx7625
import framebuf
import machine
import ctypes

if __name__ == "__main__":
    w = 640
    h = 480

    i2c = machine.I2C(1)
    video_on = machine.Pin.cpu.K2
    video_rst = machine.Pin.cpu.J3
    otg_on = machine.Pin.cpu.J6
    mode = _anx7625.MODE_640x480_60Hz
    buf0 = bytearray(w * h * 2)
    buf1 = bytearray(w * h * 2)
    fbuf0 = framebuf.FrameBuffer(buf0, w, h, framebuf.RGB565)
    fbuf1 = framebuf.FrameBuffer(buf1, w, h, framebuf.RGB565)

    anx = _anx7625.ANX7625(
        i2c,
        video_on,
        video_rst,
        otg_on,
        mode,
        ctypes.addressof(buf0),
        ctypes.addressof(buf1),
    )
    print(anx)

    while True:
        anx.poll()
