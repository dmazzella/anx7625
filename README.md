# Micropython porting of video primitives for Portenta H7 via USBC (to HDMI adapter)

> [!WARNING] 
> Working in progress, help are welcome!

## How to build

> [!IMPORTANT]
> Currently needs patches to the files

<details><summary><b>ports/stm32/boards/ARDUINO_PORTENTA_H7/mpconfigboard.h</b></summary>
<p>

```diff
diff --git a/ports/stm32/boards/ARDUINO_PORTENTA_H7/mpconfigboard.h b/ports/stm32/boards/ARDUINO_PORTENTA_H7/mpconfigboard.h
index ace63e016..e8216c994 100644
--- a/ports/stm32/boards/ARDUINO_PORTENTA_H7/mpconfigboard.h
+++ b/ports/stm32/boards/ARDUINO_PORTENTA_H7/mpconfigboard.h
@@ -240,6 +240,8 @@ extern struct _spi_bdev_t spi_bdev;
 #define MICROPY_HW_SDRAM_SIZE               (64 / 8 * 1024 * 1024)  // 64 Mbit
 #define MICROPY_HW_SDRAM_STARTUP_TEST       (1)
 #define MICROPY_HW_SDRAM_TEST_FAIL_ON_ERROR (true)
+#define MICROPY_HEAP_START                  ((sdram_valid) ? sdram_start() : &_heap_start)
+#define MICROPY_HEAP_END                    ((sdram_valid) ? sdram_end() : &_heap_end)
 
 // Timing configuration for 200MHz/2=100MHz (10ns)
 #define MICROPY_HW_SDRAM_CLOCK_PERIOD       2
```
</p>
</details>


<details><summary><b>ports/stm32/boards/ARDUINO_PORTENTA_H7/stm32h7xx_hal_conf.h</b></summary>
<p>
 
```diff
diff --git a/ports/stm32/boards/ARDUINO_PORTENTA_H7/stm32h7xx_hal_conf.h b/ports/stm32/boards/ARDUINO_PORTENTA_H7/stm32h7xx_hal_conf.h
index 737a2e5b0..339130ac6 100644
--- a/ports/stm32/boards/ARDUINO_PORTENTA_H7/stm32h7xx_hal_conf.h
+++ b/ports/stm32/boards/ARDUINO_PORTENTA_H7/stm32h7xx_hal_conf.h
@@ -23,6 +23,8 @@
 #define PREFETCH_ENABLE             1
 #define USE_RTOS                    0
 
+#define HAL_DMA2D_MODULE_ENABLED
+#define HAL_DSI_MODULE_ENABLED
 #define HAL_HSEM_MODULE_ENABLED
 #define HAL_JPEG_MODULE_ENABLED
 #define HAL_LPTIM_MODULE_ENABLED
@@ -40,6 +42,14 @@
 #define HAL_SRAM_MODULE_ENABLED
 #define HAL_SWPMI_MODULE_ENABLED
 
+#ifdef HAL_DMA2D_MODULE_ENABLED
+#include "stm32h7xx_hal_dma2d.h"
+#endif
+
+#ifdef HAL_DSI_MODULE_ENABLED
+#include "stm32h7xx_hal_dsi.h"
+#endif
+
 #ifdef HAL_HSEM_MODULE_ENABLED
 #include "stm32h7xx_hal_hsem.h"
 #endif
@@ -48,4 +58,8 @@
 #include "stm32h7xx_hal_mmc.h"
 #endif
 
+#ifdef HAL_LTDC_MODULE_ENABLED
+#include "stm32h7xx_hal_ltdc.h"
+#endif
+
 #endif // MICROPY_INCLUDED_STM32H7XX_HAL_CONF_H
```

</p>
</details>

## Build

```bash
$ git clone https://github.com/micropython/micropython.git
$ cd micropython
micropython$ git submodule update --init --depth 1
micropython$ git clone https://github.com/dmazzella/anx7625.git usercmodule/anx7625
micropython$ git apply usercmodule/anx7625/patches/mpconfigboard.h.patch
micropython$ git apply usercmodule/anx7625/patches/stm32h7xx_hal_conf.h.patch
micropython$ make -j2 -C mpy-cross/
micropython$ make -C ports/stm32 BOARD=ARDUINO_PORTENTA_H7 USER_C_MODULES="$(pwd)/usercmodule"
```

# Example

![alt Screen](https://github.com/dmazzella/anx7625/blob/main/tests/main.png?raw=true)

```python
import _anx7625

import framebuf
import machine
import time

BG = 0x0000
WHITE = 0xFFFF
CYAN = 0x177A


def main():
    i2c = machine.I2C(1, freq=400_000)
    video_on = machine.Pin.cpu.K2
    video_rst = machine.Pin.cpu.J3
    otg_on = machine.Pin.cpu.J6
    width = 800
    height = 600
    # RGB565, double-buffered: the driver uses two framebuffers, so allocate
    # width * height * 2 * 2 bytes for the selected mode.
    buffer = bytearray(width * height * 2 * 2)

    anx = _anx7625.ANX7625(
        i2c, video_on, video_rst, otg_on, buffer, width=width, height=height
    )

    # Simple game loop: redraw the whole frame into the hidden buffer
    # (anx.framebuffer, which alternates on every flush) and present it with
    # anx.flush(). flush() cleans the D-cache for the buffer you drew and flips
    # it in at the next vertical blank, so drawing is coherent and tear-free.
    x = 0
    while True:
        fb = framebuf.FrameBuffer(
            anx.framebuffer, anx.width, anx.height, framebuf.RGB565
        )
        fb.fill(BG)
        fb.text("ANX7625 MicroPython", 20, 20, WHITE)
        fb.fill_rect(x, anx.height // 2 - 40, 80, 80, CYAN)
        anx.flush()

        x = (x + 8) % (anx.width - 80)
        time.sleep_ms(16)


if __name__ == "__main__":
    main()
```

## Video modes

The requested `width` / `height` are matched against a table of known modes
(`video_modes.c`, vendored from the official
[Arduino_Video](https://github.com/arduino-libraries/Arduino_Video) library).
The driver selects the **smallest supported mode that can contain** the
requested resolution, and caps the pixel clock at ~58 MHz (STM32H747 limit:
modes above 1024x768 are remapped to 1024x768).

- If you request an exact known mode (e.g. `720x480`, `800x600`, `1024x768`),
  that mode is used as-is.
- If you request a non-exact resolution (e.g. `1024x600`), the next larger mode
  is selected (`1024x768`) and reported back via `anx.width` / `anx.height`.

The framebuffer is **double-buffered RGB565**, so the `buffer` you pass must be
at least `width * height * 2 * 2` bytes for the *selected* mode. If it is too
small, the constructor raises `ValueError` reporting the exact number of bytes
required.

Each frame, draw into `anx.framebuffer` — the hidden framebuffer, one of the two
halves of the allocation you passed to the constructor, which alternates on
every flush — then call `anx.flush()` to present it at the next vertical blank.
You can redraw the whole frame every iteration (a normal game loop) or update
only what changed; both work. `flush()` is a single-layer page flip: it keeps
the LTDC layer permanently enabled and just moves its start address at the
vertical blank (no copy, no tearing), and it cleans the D-cache for the buffer
you drew so the CPU's drawing is coherent with the LTDC. `anx.image(buf,
width=, height=, x=, y=)` blits an external RGB565 buffer straight into the
visible framebuffer.

The framebuffer lives in the external SDRAM that also holds the MicroPython
heap. The CPU draws through its data cache while the LTDC reads SDRAM directly,
so `flush()` writes the drawn buffer back to SDRAM (D-cache clean) before
presenting it — otherwise freshly drawn pixels would linger in the cache and the
display would show stale data. This is handled for you, and every supported mode
up to `1024x768` renders cleanly (a full-frame redraw at 800x600 is smooth).

Supported modes: 640x480, 720x480, 800x600, 480x800, 1024x768 (higher modes are
remapped to 1024x768).
