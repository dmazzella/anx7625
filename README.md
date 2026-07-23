# Micropython porting of video primitives for Portenta H7 via USBC (to HDMI adapter)

## How to build

> [!IMPORTANT]
> Currently needs patches. See [patches/README.md](patches/README.md) for the full list of required and optional patches and how to apply them.

## Build

```bash
$ git clone https://github.com/micropython/micropython.git
$ cd micropython
micropython$ git submodule update --init --depth 1
micropython$ git clone https://github.com/dmazzella/anx7625.git ports/stm32/boards/ARDUINO_PORTENTA_H7/modules/anx7625
micropython$ P=ports/stm32/boards/ARDUINO_PORTENTA_H7/modules/anx7625/patches
micropython$ git apply $P/mpconfigboard.h.patch
micropython$ git apply $P/stm32h7xx_hal_conf.h.patch
micropython$ git apply $P/mpconfigboard.mk.patch
micropython$ # optional, only to build lv_binding_micropython as a usermod:
micropython$ # git apply $P/stm32h747.ld.patch
micropython$ make -j2 -C mpy-cross/
micropython$ make -C ports/stm32 BOARD=ARDUINO_PORTENTA_H7
```

# Demo [tests/main.py](https://github.com/dmazzella/anx7625/blob/main/tests/main.py?raw=true)

![alt Screen](https://github.com/dmazzella/anx7625/blob/main/tests/main.png?raw=true)


# Demo LVGL [tests/main_lvgl.py](https://github.com/dmazzella/anx7625/blob/main/tests/main_lvgl.py?raw=true)

![alt Screen](https://github.com/dmazzella/anx7625/blob/main/tests/main_lvgl.png?raw=true)