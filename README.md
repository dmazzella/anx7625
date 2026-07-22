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

# Demo [tests/main.py](https://github.com/dmazzella/anx7625/blob/main/tests/main.py?raw=true)

![alt Screen](https://github.com/dmazzella/anx7625/blob/main/tests/main.png?raw=true)
