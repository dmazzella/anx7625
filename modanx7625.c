/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Damiano Mazzella
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY of ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES of MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION of CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT of OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#if defined(MICROPY_PY_ANX7625)

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "py/mphal.h"
#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/objtype.h"
#include "py/objstr.h"
#include "py/objint.h"
#include "pin.h"
#include "extmod/machine_i2c.h"

#include "anx7625.h"
#include "image.h"

STATIC mp_obj_type_t mp_anx7625_type;
mp_anx7625_t anx7625_object = {0};
mp_anx7625_t *anx7625_obj = &anx7625_object;

STATIC bool mp_obj_is_machine_i2c(mp_obj_t i2c)
{
    const mp_obj_type_t *i2c_type = mp_obj_get_type(i2c);
    if (i2c_type->name != MP_QSTR_I2C)
    {
        return false;
    }
    const mp_machine_i2c_p_t *i2c_p = i2c_type->protocol;
    return (i2c_p != NULL && i2c_p->transfer != NULL);
}

#if 0
STATIC bool mp_obj_is_framebuffer(mp_obj_t fb)
{
    const mp_obj_type_t *fb_type = mp_obj_get_type(fb);
    if (fb_type->name != MP_QSTR_framebuf)
    {
        return false;
    }
    return true;
}
#endif

STATIC mp_obj_t mp_anx7625_poll(mp_obj_t self_obj)
{
    mp_anx7625_t *self = MP_OBJ_TO_PTR(self_obj);
    (void)self;

    int offset;
    offset = ((stm32_getXSize() - 300)) + (stm32_getXSize() * (stm32_getYSize() - 300) / 2) * sizeof(uint16_t);
    stm32_LCD_DrawImage((void *)texture_raw, (void *)(getNextFrameBuffer() + offset), 300, 300, DMA2D_INPUT_RGB565);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_anx7625_poll_obj, mp_anx7625_poll);

STATIC const mp_rom_map_elem_t mp_anx7625_locals_dict_table[] = {
    {MP_ROM_QSTR(MP_QSTR_poll), MP_ROM_PTR(&mp_anx7625_poll_obj)},
};

STATIC MP_DEFINE_CONST_DICT(mp_anx7625_locals_dict, mp_anx7625_locals_dict_table);

STATIC mp_obj_t mp_anx7625_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args)
{
    mp_arg_check_num(n_args, n_kw, 0, 8, true);

    enum
    {
        ARG_i2c,
        ARG_video_on,
        ARG_video_rst,
        ARG_otg_on,
        ARG_mode,
        ARG_framebuffer0,
        ARG_framebuffer1,
        ARG_timeout,
    };

    static const mp_arg_t allowed_args[] = {
        {MP_QSTR_i2c, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_video_on, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_video_rst, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_otg_on, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_mode, MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = EDID_MODE_640x480_60Hz}},
        {MP_QSTR_framebuffer0, MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = -1}},
        {MP_QSTR_framebuffer1, MP_ARG_INT | MP_ARG_REQUIRED, {.u_int = -1}},
        {MP_QSTR_timeout, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 500}},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_obj_t i2c_obj = args[ARG_i2c].u_obj;
    if (!mp_obj_is_machine_i2c(i2c_obj))
    {
        mp_raise_TypeError(MP_ERROR_TEXT("expecting a i2c"));
    }

    mp_obj_t pin_video_on_obj = args[ARG_video_on].u_obj;
    mp_hal_get_pin_obj(pin_video_on_obj);

    mp_obj_t pin_video_rst_obj = args[ARG_video_rst].u_obj;
    mp_hal_get_pin_obj(pin_video_rst_obj);

    mp_obj_t pin_otg_on_obj = args[ARG_otg_on].u_obj;
    mp_hal_get_pin_obj(pin_otg_on_obj);

    mp_int_t mode = args[ARG_mode].u_int;

    mp_int_t framebuffer0 = args[ARG_framebuffer0].u_int;
    printf("framebuffer0 : %d\n", framebuffer0);

    mp_int_t framebuffer1 = args[ARG_framebuffer1].u_int;
    printf("framebuffer1 : %d\n", framebuffer1);

    mp_int_t timeout = args[ARG_timeout].u_int;

    anx7625_obj->base.type = &mp_anx7625_type;
    anx7625_obj->i2c_obj = i2c_obj;
    anx7625_obj->pin_video_on_obj = pin_video_on_obj;
    anx7625_obj->pin_video_rst_obj = pin_video_rst_obj;
    anx7625_obj->pin_otg_on_obj = pin_otg_on_obj;
    anx7625_obj->mode = mode;
    anx7625_obj->framebuffer0 = framebuffer0;
    anx7625_obj->framebuffer1 = framebuffer1;
    anx7625_obj->timeout = timeout;

    /* video on */
    mp_hal_pin_config(mp_hal_get_pin_obj(anx7625_obj->pin_video_on_obj), MP_HAL_PIN_MODE_OUTPUT, MP_HAL_PIN_PULL_NONE, 0);
    mp_hal_pin_config_speed(mp_hal_get_pin_obj(anx7625_obj->pin_video_on_obj), MP_HAL_PIN_SPEED_HIGH);
    mp_hal_pin_low(mp_hal_get_pin_obj(anx7625_obj->pin_video_on_obj));

    /* video rst */
    mp_hal_pin_config(mp_hal_get_pin_obj(anx7625_obj->pin_video_rst_obj), MP_HAL_PIN_MODE_OUTPUT, MP_HAL_PIN_PULL_NONE, 0);
    mp_hal_pin_config_speed(mp_hal_get_pin_obj(anx7625_obj->pin_video_rst_obj), MP_HAL_PIN_SPEED_HIGH);
    mp_hal_pin_low(mp_hal_get_pin_obj(anx7625_obj->pin_video_rst_obj));

    /* otg on */
    mp_hal_pin_config(mp_hal_get_pin_obj(anx7625_obj->pin_otg_on_obj), MP_HAL_PIN_MODE_OUTPUT, MP_HAL_PIN_PULL_NONE, 0);
    mp_hal_pin_config_speed(mp_hal_get_pin_obj(anx7625_obj->pin_otg_on_obj), MP_HAL_PIN_SPEED_HIGH);
    mp_hal_pin_low(mp_hal_get_pin_obj(anx7625_obj->pin_otg_on_obj));

    mp_hal_pin_config(mp_hal_get_pin_obj(anx7625_obj->pin_otg_on_obj), MP_HAL_PIN_MODE_INPUT, MP_HAL_PIN_PULL_UP, 0);
    mp_hal_pin_config_speed(mp_hal_get_pin_obj(anx7625_obj->pin_otg_on_obj), MP_HAL_PIN_SPEED_HIGH);

    int ret = -1;
    if ((ret = anx7625_init(0)) < 0)
    {
        mp_raise_TypeError(MP_ERROR_TEXT("anx7625_init failed."));
    }

    if ((ret = anx7625_wait_hpd_event(0)) < 0)
    {
        mp_raise_TypeError(MP_ERROR_TEXT("anx7625_wait_hpd_event failed."));
    }

    struct edid recognized_edid = {0};
    if ((ret = anx7625_dp_get_edid(0, &recognized_edid)) < 0)
    {
        mp_raise_TypeError(MP_ERROR_TEXT("anx7625_dp_get_edid failed."));
    }

    if ((ret = anx7625_dp_start(0, &recognized_edid, anx7625_obj->mode, anx7625_obj->framebuffer0, anx7625_obj->framebuffer1)) < 0)
    {
        mp_raise_TypeError(MP_ERROR_TEXT("anx7625_dp_start failed."));
    }

    return MP_OBJ_FROM_PTR(anx7625_obj);
}

STATIC mp_obj_type_t mp_anx7625_type = {
    {&mp_type_type},
    .name = MP_QSTR_ANX7625,
    .locals_dict = (void *)&mp_anx7625_locals_dict,
    .make_new = mp_anx7625_make_new,
};

STATIC const mp_rom_map_elem_t mp_module_anx7625_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR__anx7625)},
    {MP_ROM_QSTR(MP_QSTR_ANX7625), MP_ROM_PTR(&mp_anx7625_type)},
    {MP_ROM_QSTR(MP_QSTR_MODE_640x480_60Hz), MP_ROM_INT(EDID_MODE_640x480_60Hz)},
    {MP_ROM_QSTR(MP_QSTR_MODE_720x480_60Hz), MP_ROM_INT(EDID_MODE_720x480_60Hz)},
    {MP_ROM_QSTR(MP_QSTR_MODE_800x600_59Hz), MP_ROM_INT(EDID_MODE_800x600_59Hz)},
    {MP_ROM_QSTR(MP_QSTR_MODE_1024x768_60Hz), MP_ROM_INT(EDID_MODE_1024x768_60Hz)},
    {MP_ROM_QSTR(MP_QSTR_MODE_1280x768_60Hz), MP_ROM_INT(EDID_MODE_1280x768_60Hz)},
    {MP_ROM_QSTR(MP_QSTR_MODE_1280x720_60Hz), MP_ROM_INT(EDID_MODE_1280x720_60Hz)}};

STATIC MP_DEFINE_CONST_DICT(mp_module_anx7625_globals, mp_module_anx7625_globals_table);

const mp_obj_module_t mp_module_anx7625 = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&mp_module_anx7625_globals,
};

// Register the module to make it available in Python
MP_REGISTER_MODULE(MP_QSTR__anx7625, mp_module_anx7625);

#endif // MICROPY_PY_ANX7625