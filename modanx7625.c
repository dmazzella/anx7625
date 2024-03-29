/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Damiano Mazzella
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
#include "extmod/modmachine.h"

#include "anx7625.h"

const mp_obj_type_t mp_anx7625_type;
mp_anx7625_t anx7625_object = {0};
mp_anx7625_t *anx7625_obj = &anx7625_object;

static bool mp_obj_is_machine_i2c(mp_obj_t i2c)
{
    const mp_obj_type_t *i2c_type = mp_obj_get_type(i2c);
    if (i2c_type->name != MP_QSTR_I2C)
    {
        return false;
    }
    const mp_machine_i2c_p_t *i2c_p = (mp_machine_i2c_p_t *)MP_OBJ_TYPE_GET_SLOT(i2c_type, protocol);
    return (i2c_p != NULL && i2c_p->transfer != NULL);
}

static mp_obj_t mp_anx7625_image(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    enum
    {
        ARG_buffer,
        ARG_width,
        ARG_height,
        ARG_x,
        ARG_y,
    };
    static const mp_arg_t allowed_args[] = {
        {MP_QSTR_buffer, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_width, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0}},
        {MP_QSTR_height, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0}},
        {MP_QSTR_x, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0}},
        {MP_QSTR_y, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0}},
    };

    mp_anx7625_t *self = MP_OBJ_TO_PTR(args[0]);
    (void)self;

    mp_arg_val_t vals[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, vals);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(vals[ARG_buffer].u_obj, &bufinfo, MP_BUFFER_RW);

    mp_int_t width = vals[ARG_width].u_int;
    mp_int_t height = vals[ARG_height].u_int;
    mp_int_t x = vals[ARG_x].u_int;
    mp_int_t y = vals[ARG_y].u_int;
    mp_int_t buffer_address = (uintptr_t)bufinfo.buf;
    uint32_t offsetPos = (x + (getXSize() * y)) * sizeof(uint16_t);

    DrawImage((void *)buffer_address, (void *)(getActiveFrameBuffer() + offsetPos), width, height, DMA2D_INPUT_RGB565);
    return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_KW(mp_anx7625_image_obj, 1, mp_anx7625_image);

static mp_obj_t mp_anx7625_clear(size_t n_args, const mp_obj_t *args)
{
    mp_anx7625_t *self = MP_OBJ_TO_PTR(args[0]);
    (void)self;

    if (n_args == 2)
    {
        self->background_color = mp_obj_get_int(args[1]);
    }
    Clear(self->background_color);
    return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_anx7625_clear_obj, 1, 2, mp_anx7625_clear);

static mp_obj_t mp_anx7625_flush(mp_obj_t self_obj)
{
    mp_anx7625_t *self = MP_OBJ_TO_PTR(self_obj);
    (void)self;
    drawCurrentFrameBuffer();
    return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1(mp_anx7625_flush_obj, mp_anx7625_flush);

static const mp_rom_map_elem_t mp_anx7625_locals_dict_table[] = {
    {MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&mp_anx7625_clear_obj)},
    {MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&mp_anx7625_flush_obj)},
    {MP_ROM_QSTR(MP_QSTR_image), MP_ROM_PTR(&mp_anx7625_image_obj)},
    {MP_ROM_QSTR(MP_QSTR_buffer), MP_ROM_PTR(mp_const_none)},
    {MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(mp_const_none)},
    {MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(mp_const_none)},
};

static MP_DEFINE_CONST_DICT(mp_anx7625_locals_dict, mp_anx7625_locals_dict_table);

static mp_obj_t mp_anx7625_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args)
{
    mp_arg_check_num(n_args, n_kw, 0, 9, true);

    enum
    {
        ARG_i2c,
        ARG_video_on,
        ARG_video_rst,
        ARG_otg_on,
        ARG_buffer,
        ARG_width,
        ARG_height,
        ARG_timeout,
        ARG_background_color,
    };

    static const mp_arg_t allowed_args[] = {
        {MP_QSTR_i2c, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_video_on, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_video_rst, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_otg_on, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_buffer, MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_obj = MP_OBJ_NULL}},
        {MP_QSTR_width, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 640}},
        {MP_QSTR_height, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 480}},
        {MP_QSTR_timeout, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 500}},
        {MP_QSTR_background_color, MP_ARG_INT | MP_ARG_KW_ONLY, {.u_int = 0}},
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

    mp_obj_t buffer_obj = args[ARG_buffer].u_obj;

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buffer_obj, &bufinfo, MP_BUFFER_RW);

    mp_int_t buffer_address = (uintptr_t)bufinfo.buf;

    mp_int_t width = args[ARG_width].u_int;

    mp_int_t height = args[ARG_height].u_int;

    mp_int_t timeout = args[ARG_timeout].u_int;

    mp_int_t background_color = args[ARG_background_color].u_int;

    anx7625_obj->base.type = &mp_anx7625_type;
    anx7625_obj->i2c_obj = i2c_obj;
    anx7625_obj->pin_video_on_obj = pin_video_on_obj;
    anx7625_obj->pin_video_rst_obj = pin_video_rst_obj;
    anx7625_obj->pin_otg_on_obj = pin_otg_on_obj;
    anx7625_obj->buffer_obj = buffer_obj;
    anx7625_obj->buffer_address = buffer_address;
    anx7625_obj->width = width;
    anx7625_obj->height = height;
    anx7625_obj->timeout = timeout;
    anx7625_obj->background_color = background_color;
    anx7625_obj->mode = video_modes_search_edid(anx7625_obj->width, anx7625_obj->height);

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
    anx7625_dp_get_edid(0, &recognized_edid);
    if ((ret = anx7625_dp_start(0, &recognized_edid, anx7625_obj->mode, anx7625_obj->buffer_address)) < 0)
    {
        mp_raise_TypeError(MP_ERROR_TEXT("anx7625_dp_start failed."));
    }

    return MP_OBJ_FROM_PTR(anx7625_obj);
}

static void mp_anx7625_attr(mp_obj_t obj, qstr attr, mp_obj_t *dest)
{
    mp_anx7625_t *self = MP_OBJ_TO_PTR(obj);
    if (dest[0] == MP_OBJ_NULL)
    {
        const mp_obj_type_t *type = mp_obj_get_type(obj);
        mp_map_t *locals_map = (mp_map_t *)mp_obj_dict_get_map(MP_OBJ_TYPE_GET_SLOT(type, locals_dict));
        mp_map_elem_t *elem = mp_map_lookup(locals_map, MP_OBJ_NEW_QSTR(attr), MP_MAP_LOOKUP);
        if (elem != NULL)
        {
            if (attr == MP_QSTR_buffer)
            {
                dest[0] = self->buffer_obj;
                return;
            }
            if (attr == MP_QSTR_width)
            {
                dest[0] = mp_obj_new_int(self->width);
                return;
            }
            if (attr == MP_QSTR_height)
            {
                dest[0] = mp_obj_new_int(self->height);
                return;
            }
            mp_convert_member_lookup(obj, type, elem->value, dest);
        }
    }
}

MP_DEFINE_CONST_OBJ_TYPE(
    mp_anx7625_type,
    MP_QSTR_ANX7625,
    MP_TYPE_FLAG_NONE,
    make_new, mp_anx7625_make_new,
    attr, mp_anx7625_attr,
    locals_dict, &mp_anx7625_locals_dict);

static const mp_rom_map_elem_t mp_module_anx7625_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR__anx7625)},
    {MP_ROM_QSTR(MP_QSTR_ANX7625), MP_ROM_PTR(&mp_anx7625_type)},
};

static MP_DEFINE_CONST_DICT(mp_module_anx7625_globals, mp_module_anx7625_globals_table);

const mp_obj_module_t mp_module_anx7625 = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&mp_module_anx7625_globals,
};

// Register the module to make it available in Python
MP_REGISTER_MODULE(MP_QSTR__anx7625, mp_module_anx7625);

#endif // MICROPY_PY_ANX7625