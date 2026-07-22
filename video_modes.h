/*
 * This file is part of the coreboot project.
 *
 * Copyright 2013 Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/*
 * Vendored from the official Arduino_Video library
 * (https://github.com/arduino-libraries/Arduino_Video), src/video_modes.h.
 * Provides the known video-mode table and the best-fit mode selector
 * used by the ANX7625 MicroPython driver.
 */

#ifndef _VIDEO_MODES_H
#define _VIDEO_MODES_H

/* Includes ------------------------------------------------------------------*/
#include "edid.h"

/* Exported struct -----------------------------------------------------------*/
struct envie_edid_mode {
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

/* Exported variables --------------------------------------------------------*/
extern struct envie_edid_mode envie_known_modes[];

/* Exported functions --------------------------------------------------------*/
enum edid_modes video_modes_get_edid(uint32_t h_check, uint32_t v_check);

#endif /* _VIDEO_MODES_H */
