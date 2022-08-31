/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef EDID_H
#define EDID_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "py/mphal.h"
#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/objtype.h"
#include "py/objstr.h"
#include "py/objint.h"
#include "pin.h"
#include "extmod/machine_i2c.h"

// static inline int printf0(const char *fmt, ...)
// {
//     return 0;
// }

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define printk(x, ...) printf(__VA_ARGS__)
#define console_log_level(x) (1)
#define CONFIG(x) (0)
#define mdelay(x) mp_hal_delay_us(x * 1000)
#define die(...)
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b)) /*!< Return the maximum of the 2 values     */
#endif
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b)) /*!< Return the minimum of the 2 values     */
#endif
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif
#define ALIGN(x, a) __ALIGN_MASK(x, (__typeof__(x))(a)-1UL)
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define ALIGN_UP(x, a) ALIGN((x), (a))
#define ALIGN_DOWN(x, a) ((x) & ~((__typeof__(x))(a)-1UL))
#define IS_ALIGNED(x, a) (((x) & ((__typeof__(x))(a)-1UL)) == 0)
#define DIV_ROUND_UP(x, y) ({                         \
    __typeof__(x) _div_local_x = (x);                 \
    __typeof__(y) _div_local_y = (y);                 \
    (_div_local_x + _div_local_y - 1) / _div_local_y; \
})

static inline int isupper(int ch)
{
    return (((ch >= 'A' && ch <= 'Z')) || (ch >= 0xC0 && ch <= 0xDD));
}

// #include <framebuffer_info.h>
// #include "commonlib/coreboot_tables.h"

enum edid_modes
{
	EDID_MODE_640x480_60Hz,
	EDID_MODE_720x480_60Hz,
	EDID_MODE_800x600_59Hz,
	EDID_MODE_1024x768_60Hz,
	EDID_MODE_1280x768_60Hz,
	EDID_MODE_1280x720_60Hz,
	EDID_MODE_1920x1080_60Hz,
    NUM_KNOWN_MODES,

    EDID_MODE_AUTO
};

struct edid_mode
{
    const char *name;
    unsigned int pixel_clock;
    int lvds_dual_channel;
    unsigned int refresh;
    unsigned int ha;
    unsigned int hbl;
    unsigned int hso;
    unsigned int hspw;
    unsigned int hborder;
    unsigned int va;
    unsigned int vbl;
    unsigned int vso;
    unsigned int vspw;
    unsigned int vborder;
    unsigned char phsync;
    unsigned char pvsync;
    unsigned int x_mm;
    unsigned int y_mm;
};

struct envie_edid_mode
{
    const char *name;
    unsigned int pixel_clock;
    unsigned int refresh;
    unsigned int hactive;
    unsigned int hback_porch;
    unsigned int hfront_porch;
    unsigned int hsync_len;
    unsigned int vactive;
    unsigned int vsync_len;
    unsigned int vback_porch;
    unsigned int vfront_porch;
    unsigned int voffset;
    unsigned int hpol : 1;
    unsigned int vpol : 1;
};

/* structure for communicating EDID information from a raw EDID block to
 * higher level functions.
 * The size of the data types is not critical, so we leave them as
 * unsigned int. We can move more into this struct as needed.
 */

#define EDID_ASCII_STRING_LENGTH 13

struct edid
{
    /* These next three things used to all be called bpp.
     * Merriment ensued. The identifier
     * 'bpp' is herewith banished from our
     * Kingdom.
     */
    /* How many bits in the framebuffer per pixel.
     * Under all reasonable circumstances, it's 32.
     */
    unsigned int framebuffer_bits_per_pixel;
    /* On the panel, how many bits per color?
     * In almost all cases, it's 6 or 8.
     * The standard allows for much more!
     */
    unsigned int panel_bits_per_color;
    /* On the panel, how many bits per pixel.
     * On Planet Earth, there are three colors
     * per pixel, but this is convenient to have here
     * instead of having 3*panel_bits_per_color
     * all over the place.
     */
    unsigned int panel_bits_per_pixel;
    /* used to compute timing for graphics chips. */
    struct edid_mode mode;
    u8 mode_is_supported[NUM_KNOWN_MODES];
    unsigned int link_clock;
    /* 3 variables needed for coreboot framebuffer.
     * In most cases, they are the same as the ha
     * and va variables, but not always, as in the
     * case of a 1366 wide display.
     */
    u32 x_resolution;
    u32 y_resolution;
    u32 bytes_per_line;

    int hdmi_monitor_detected;
    char ascii_string[EDID_ASCII_STRING_LENGTH + 1];
    char manufacturer_name[3 + 1];
};

enum edid_status
{
    EDID_CONFORMANT,
    EDID_NOT_CONFORMANT,
    EDID_ABSENT,
};

/* Defined in src/lib/edid.c */
int decode_edid(unsigned char *edid, int size, struct edid *out);
void edid_set_framebuffer_bits_per_pixel(struct edid *edid, int fb_bpp,
                                         int row_byte_alignment);
int set_display_mode(struct edid *edid, enum edid_modes mode);

#endif /* EDID_H */