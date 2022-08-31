# video primitives for Portenta H7 via USBC (to HDMI adapter)

```python
import machine
import _anx7625


i2c = machine.I2C(1)
video_on = machine.Pin.cpu.K2
video_rst = machine.Pin.cpu.J3
otg_on = machine.Pin.cpu.J6
mode = _anx7625.MODE_640x480_60Hz

anx = _anx7625.ANX7625(i2c, video_on, video_rst, otg_on, mode)

```