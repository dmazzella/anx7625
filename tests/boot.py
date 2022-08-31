# boot.py -- run on boot to configure USB and filesystem
# Put app code in main.py

import os

import machine
import pyb

pyb.country("US")  # ISO 3166-1 Alpha-2 code, eg US, GB, DE, AU
# pyb.main('main.py') # main script to run after this one
# pyb.usb_mode('VCP+MSC') # act as a serial and a storage device
# pyb.usb_mode('VCP+HID') # act as a serial device and a mouse

# filename = "/flash/cnt.txt"
# counter = 0
# try:
#     os.stat(filename)
#     with open(filename, "b+") as fd:
#         counter = int.from_bytes(fd.read(), "little")
#         if counter < 5:
#             fd.write(int.to_bytes(counter + 1, 1, "little"))
# except OSError:
#     with open(filename, "wb") as fd:
#         fd.write(int.to_bytes(counter, 1, "little"))

# if counter < 5:
#     pyb.usb_mode(None)
# else:
#     pyb.usb_mode("VCP+MSC")