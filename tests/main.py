import _anx7625

import io
import os
import framebuf
import machine
import ctypes


W = 640
H = 480


class DUP(io.IOBase):
    def __init__(self, s):
        self.s = s

    def write(self, data):
        self.s += data
        return len(data)

    def readinto(self, data):
        return 0


if __name__ == "__main__":

    i2c = machine.I2C(1)
    video_on = machine.Pin.cpu.K2
    video_rst = machine.Pin.cpu.J3
    otg_on = machine.Pin.cpu.J6
    mode = _anx7625.MODE_640x480_60Hz
    buf0 = bytearray(W * H * 2)
    buf1 = bytearray(W * H * 2)
    fbuf0 = framebuf.FrameBuffer(buf0, W, H, framebuf.RGB565)
    fbuf1 = framebuf.FrameBuffer(buf1, W, H, framebuf.RGB565)

    s = bytearray()
    dup = DUP(s)
    os.dupterm(dup)

    print(i2c)
    try:
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
        with open("output.txt", "a+b") as f:
            f.write(bytes(dup.s))

        while True:
            anx.poll()
    except:
        with open("error.txt", "a+b") as f:
            f.write(bytes(dup.s))

    os.dupterm(None)
