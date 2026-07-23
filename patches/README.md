# Patches

These patches adapt the upstream MicroPython `ARDUINO_PORTENTA_H7` board so the
`_anx7625` C module (HDMI output over USB-C through the ANX7625 bridge) can be
built and run. They are applied to the MicroPython tree, not to the module
itself.

## Required

The following three patches are **mandatory** — without them the module does not
build or the display cannot work:

- **`mpconfigboard.h.patch`** — moves the MicroPython heap into the external
  8 MB SDRAM (`MICROPY_HEAP_START` / `MICROPY_HEAP_END`). The RGB565
  double-buffered framebuffer is allocated on the heap, so the heap must live in
  SDRAM (the internal RAM is far too small for it).
- **`stm32h7xx_hal_conf.h.patch`** — enables the STM32 HAL modules the driver
  needs (`HAL_DMA2D_MODULE_ENABLED`, `HAL_DSI_MODULE_ENABLED`) and includes the
  DMA2D / DSI / LTDC HAL headers. The driver programs the LTDC + DSI and uses
  DMA2D for blits, so these must be enabled or the module will not compile/link.
- **`mpconfigboard.mk.patch`** — sets `USER_C_MODULES = $(BOARD_DIR)/modules`,
  which tells the build to compile the C modules found under the board's
  `modules/` directory (where `anx7625` lives).

## Optional (only for LVGL)

- **`stm32h747.ld.patch`** — extends the `FLASH_TEXT` (CM7 firmware) region from
  1280 KB to 1792 KB by reclaiming the unused 512 KB `FLASH_CM4` region (the
  Cortex-M4 core is not used by this port). This is **only** required to build
  [`lv_binding_micropython`](https://github.com/lvgl/lv_binding_micropython) as
  an additional user C module: LVGL v9 does not fit in the default 1280 KB
  region. The plain `_anx7625` driver builds and runs fine **without** this
  patch, so apply it only when you also build the LVGL binding.

## Applying

Run from the root of your MicroPython checkout:

```bash
P=ports/stm32/boards/ARDUINO_PORTENTA_H7/modules/anx7625/patches

# Mandatory
git apply $P/mpconfigboard.h.patch
git apply $P/stm32h7xx_hal_conf.h.patch
git apply $P/mpconfigboard.mk.patch

# Optional — only when also building lv_binding_micropython as a usermod
git apply $P/stm32h747.ld.patch
```

