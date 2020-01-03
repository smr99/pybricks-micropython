// SPDX-License-Identifier
// Copyright (c) 2019 David Lechner
// Copyright (c) 2013, 2014 Damien P. George

// class Screen

#include <string.h>

#include <grx-3.0.h>

#include <pbio/light.h>

#include "py/mpconfig.h"
#include "py/misc.h"
#include "py/mpprint.h"
#include "py/obj.h"
#include "py/runtime.h"

#include "pb_ev3dev_types.h"
#include "modparameters.h"
#include "pbkwarg.h"

typedef struct _ev3dev_Screen_obj_t {
    mp_obj_base_t base;
    mp_obj_t width;
    mp_obj_t height;
    gboolean cleared;
    gboolean initialized;
    GrxTextOptions *text_options;
    gint print_x;
    gint print_y;
} ev3dev_Screen_obj_t;

STATIC ev3dev_Screen_obj_t ev3dev_Screen_singleton;

// map Pybricks color enum to GRX color value using standard web CSS values
STATIC GrxColor map_color(mp_obj_t *obj) {
    pbio_light_color_t color = pb_type_enum_get_value(obj, &pb_enum_type_Color);

    switch (color) {
    case PBIO_LIGHT_COLOR_NONE:
        return GRX_COLOR_NONE;
    case PBIO_LIGHT_COLOR_BLACK:
        return GRX_COLOR_BLACK;
    case PBIO_LIGHT_COLOR_BLUE:
        return grx_color_get(0, 0, 255);
    case PBIO_LIGHT_COLOR_GREEN:
        return grx_color_get(0, 128, 0);
    case PBIO_LIGHT_COLOR_YELLOW:
        return grx_color_get(255, 255, 0);
    case PBIO_LIGHT_COLOR_RED:
        return grx_color_get(255, 0, 0);
    case PBIO_LIGHT_COLOR_WHITE:
        return GRX_COLOR_WHITE;
    case PBIO_LIGHT_COLOR_BROWN:
        return grx_color_get(165, 42, 42);
    case PBIO_LIGHT_COLOR_ORANGE:
        return grx_color_get(255, 165, 0);
    case PBIO_LIGHT_COLOR_PURPLE:
        return grx_color_get(128, 0, 128);
    }
    return grx_color_get_black();
}

STATIC mp_obj_t ev3dev_Screen_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    if (!ev3dev_Screen_singleton.initialized) {
        ev3dev_Screen_singleton.base.type = &pb_type_ev3dev_Screen;
        ev3dev_Screen_singleton.width = MP_OBJ_NEW_SMALL_INT(grx_get_screen_width());
        ev3dev_Screen_singleton.height = MP_OBJ_NEW_SMALL_INT(grx_get_screen_height());
        ev3dev_Screen_singleton.initialized = TRUE;
        GError *error = NULL;
        GrxFont *font = grx_font_load("Lucida", 12, &error);
        if (font) {
            ev3dev_Screen_singleton.text_options = grx_text_options_new(font, GRX_COLOR_BLACK);
            grx_font_unref(font);
        }
        else {
            fprintf(stderr, "Could not load default font: %s\n", error->message);
            g_error_free(error);
        }
    }
    return &ev3dev_Screen_singleton;
}

STATIC mp_obj_t ev3dev_Screen_clear(mp_obj_t self_in) {
    grx_clear_screen(GRX_COLOR_WHITE);
    ev3dev_Screen_singleton.cleared = TRUE;
    ev3dev_Screen_singleton.print_x = 0;
    ev3dev_Screen_singleton.print_y = 0;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ev3dev_Screen_clear_obj, ev3dev_Screen_clear);

// Ensure that screen has been cleared before we start drawing anything else
STATIC void clear_once(void) {
    if (!ev3dev_Screen_singleton.cleared) {
        grx_clear_screen(GRX_COLOR_WHITE);
        ev3dev_Screen_singleton.cleared = TRUE;
    }
}

STATIC mp_obj_t ev3dev_Screen_draw_pixel(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        ev3dev_Screen_obj_t, self,
        PB_ARG_REQUIRED(x),
        PB_ARG_REQUIRED(y),
        PB_ARG_DEFAULT_ENUM(color, pb_const_black)
    );

    (void)self; // unused
    mp_int_t _x = mp_obj_get_int(x);
    mp_int_t _y = mp_obj_get_int(y);
    GrxColor _color = map_color(color);

    clear_once();
    grx_draw_pixel(_x, _y, _color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(ev3dev_Screen_draw_pixel_obj, 0, ev3dev_Screen_draw_pixel);

STATIC mp_obj_t ev3dev_Screen_draw_line(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        ev3dev_Screen_obj_t, self,
        PB_ARG_REQUIRED(x1),
        PB_ARG_REQUIRED(y1),
        PB_ARG_REQUIRED(x2),
        PB_ARG_REQUIRED(y2),
        PB_ARG_DEFAULT_ENUM(color, pb_const_black)
    );

    (void)self; // unused
    mp_int_t _x1 = mp_obj_get_int(x1);
    mp_int_t _y1 = mp_obj_get_int(y1);
    mp_int_t _x2 = mp_obj_get_int(x2);
    mp_int_t _y2 = mp_obj_get_int(y2);
    GrxColor _color = map_color(color);

    clear_once();
    grx_draw_line(_x1, _y1, _x2, _y2, _color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(ev3dev_Screen_draw_line_obj, 0, ev3dev_Screen_draw_line);

STATIC mp_obj_t ev3dev_Screen_draw_box(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        ev3dev_Screen_obj_t, self,
        PB_ARG_REQUIRED(x1),
        PB_ARG_REQUIRED(y1),
        PB_ARG_REQUIRED(x2),
        PB_ARG_REQUIRED(y2),
        PB_ARG_DEFAULT_INT(r, 0),
        PB_ARG_DEFAULT_FALSE(fill),
        PB_ARG_DEFAULT_ENUM(color, pb_const_black)
    );

    (void)self; // unused
    mp_int_t _x1 = mp_obj_get_int(x1);
    mp_int_t _y1 = mp_obj_get_int(y1);
    mp_int_t _x2 = mp_obj_get_int(x2);
    mp_int_t _y2 = mp_obj_get_int(y2);
    mp_int_t _r = mp_obj_get_int(r);
    GrxColor _color = map_color(color);

    clear_once();
    if (mp_obj_is_true(fill)) {
        if (_r > 0) {
            grx_draw_filled_rounded_box(_x1, _y1, _x2, _y2, _r, _color);
        }
        else {
            grx_draw_filled_box(_x1, _y1, _x2, _y2, _color);
        }
    }
    else {
        if (_r > 0) {
            grx_draw_rounded_box(_x1, _y1, _x2, _y2, _r, _color);
        }
        else {
            grx_draw_box(_x1, _y1, _x2, _y2, _color);
        }
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(ev3dev_Screen_draw_box_obj, 0, ev3dev_Screen_draw_box);

STATIC mp_obj_t ev3dev_Screen_draw_circle(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        ev3dev_Screen_obj_t, self,
        PB_ARG_REQUIRED(x),
        PB_ARG_REQUIRED(y),
        PB_ARG_DEFAULT_INT(r, 0),
        PB_ARG_DEFAULT_FALSE(fill),
        PB_ARG_DEFAULT_ENUM(color, pb_const_black)
    );

    (void)self; // unused
    mp_int_t _x = mp_obj_get_int(x);
    mp_int_t _y = mp_obj_get_int(y);
    mp_int_t _r = mp_obj_get_int(r);
    GrxColor _color = map_color(color);

    clear_once();
    if (mp_obj_is_true(fill)) {
        grx_draw_filled_circle(_x, _y, _r, _color);
    }
    else {
        grx_draw_circle(_x, _y, _r, _color);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(ev3dev_Screen_draw_circle_obj, 0, ev3dev_Screen_draw_circle);

STATIC mp_obj_t ev3dev_Screen_draw_text(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        ev3dev_Screen_obj_t, self,
        PB_ARG_REQUIRED(x),
        PB_ARG_REQUIRED(y),
        PB_ARG_REQUIRED(text),
        PB_ARG_DEFAULT_ENUM(color, pb_const_black)
    );

    mp_int_t _x = mp_obj_get_int(x);
    mp_int_t _y = mp_obj_get_int(y);
    const char *_text = mp_obj_str_get_str(text);
    GrxColor _color = map_color(color);

    if (!self->text_options) {
        mp_raise_msg(&mp_type_RuntimeError, "Font is not loaded");
    }

    clear_once();
    grx_text_options_set_fg_color(self->text_options, _color);
    grx_draw_text(_text, _x, _y, self->text_options);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(ev3dev_Screen_draw_text_obj, 0, ev3dev_Screen_draw_text);

STATIC mp_obj_t ev3dev_Screen_set_font(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    PB_PARSE_ARGS_METHOD(n_args, pos_args, kw_args,
        ev3dev_Screen_obj_t, self,
        PB_ARG_DEFAULT_NONE(family),
        PB_ARG_DEFAULT_INT(size, 12),
        PB_ARG_DEFAULT_FALSE(bold),
        PB_ARG_DEFAULT_FALSE(italic),
        PB_ARG_DEFAULT_FALSE(monospace),
        PB_ARG_DEFAULT_NONE(lang),
        PB_ARG_DEFAULT_NONE(script)
    );

    const char *_family = NULL;
    if (family != mp_const_none) {
        _family = mp_obj_str_get_str(family);
    }
    mp_int_t _size = mp_obj_get_int(size);
    mp_int_t _dpi = -1; // use screen dpi
    GrxFontWeight _weight = mp_obj_is_true(bold) ? GRX_FONT_WEIGHT_BOLD : GRX_FONT_WEIGHT_REGULAR;
    GrxFontSlant _slant = mp_obj_is_true(italic) ? GRX_FONT_SLANT_ITALIC : GRX_FONT_SLANT_REGULAR;
    GrxFontWidth _width = GRX_FONT_WIDTH_REGULAR;
    bool _monospace = mp_obj_is_true(monospace);
    const char *_lang = NULL;
    if (lang != mp_const_none) {
        _lang = mp_obj_str_get_str(lang);
    }
    const char *_script = NULL;
    if (script != mp_const_none) {
        _script = mp_obj_str_get_str(script);
        if (strlen(_script) != 4) {
            mp_raise_ValueError("script code must have 4 characters");
        }
    }

    GError *error = NULL;
    GrxFont *font = grx_font_load_full(_family, _size, _dpi, _weight, _slant,
                                       _width, _monospace, _lang, _script, &error);
    if (font) {
        if (self->text_options) {
            grx_text_options_set_font(self->text_options, font);
        }
        else {
            self->text_options = grx_text_options_new(font, GRX_COLOR_BLACK);
        }
        grx_font_unref(font);
    }
    else {
        g_error_free(error);
        mp_raise_msg(&mp_type_RuntimeError, "Failed to load font");
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(ev3dev_Screen_set_font_obj, 0, ev3dev_Screen_set_font);

// copy of mp_builtin_print modified to print to vstr
STATIC mp_obj_t ev3dev_Screen_print(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_sep, ARG_end };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_sep, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_QSTR(MP_QSTR__space_)} },
        { MP_QSTR_end, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_QSTR(MP_QSTR__0x0a_)} },
    };

    // parse args (a union is used to reduce the amount of C stack that is needed)
    union {
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        size_t len[2];
    } u;
    mp_arg_parse_all(0, NULL, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, u.args);

    ev3dev_Screen_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    // extract the objects first because we are going to use the other part of the union
    mp_obj_t sep = u.args[ARG_sep].u_obj;
    mp_obj_t end = u.args[ARG_end].u_obj;
    const char *sep_data = mp_obj_str_get_data(sep, &u.len[0]);
    const char *end_data = mp_obj_str_get_data(end, &u.len[1]);

    vstr_t vstr;
    mp_print_t print;
    vstr_init_print(&vstr, 16, &print);

    for (size_t i = 1; i < n_args; i++) {
        if (i > 1) {
            mp_print_strn(&print, sep_data, u.len[0], 0, 0, 0);
        }
        mp_obj_print_helper(&print, pos_args[i], PRINT_STR);
    }
    mp_print_strn(&print, end_data, u.len[1], 0, 0, 0);

    if (!self->text_options) {
        mp_raise_msg(&mp_type_RuntimeError, "Font is not loaded");
    }

    GrxFont *font = grx_text_options_get_font(self->text_options);
    gint font_height = grx_font_get_height(font);
    gchar **lines = g_strsplit(vstr_null_terminated_str(&vstr), "\n", -1);
    for (gchar **l = lines; *l; l++) {
        // if this is not the first line, then we had a newline and need to advance
        if (l != lines) {
            self->print_x = 0;
            self->print_y += font_height;
        }
        // if printing would run off of the bottom of the screen, scroll
        // everything on the screen up enough to fit one more line
        gint screen_height = grx_get_height();
        gint over = self->print_y + font_height - screen_height;
        if (over > 0) {
            gint max_x = grx_get_max_x();
            for (int y = 0; y < screen_height; y++) {
                const GrxColor *scan_line = grx_get_scanline(0, max_x, y + over, NULL);
                if (scan_line) {
                    grx_put_scanline(0, max_x, y, scan_line, GRX_COLOR_MODE_WRITE);
                }
                else {
                    grx_draw_hline(0, max_x, y, GRX_COLOR_WHITE);
                }
            }
            self->print_y -= over;
        }
        grx_draw_text(*l, self->print_x, self->print_y, self->text_options);
        self->print_x += grx_font_get_text_width(font, *l);
    }
    g_strfreev(lines);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(ev3dev_Screen_print_obj, 0, ev3dev_Screen_print);

STATIC const mp_rom_map_elem_t ev3dev_Screen_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_clear),       MP_ROM_PTR(&ev3dev_Screen_clear_obj)                    },
    { MP_ROM_QSTR(MP_QSTR_draw_pixel),  MP_ROM_PTR(&ev3dev_Screen_draw_pixel_obj)               },
    { MP_ROM_QSTR(MP_QSTR_draw_line),   MP_ROM_PTR(&ev3dev_Screen_draw_line_obj)                },
    { MP_ROM_QSTR(MP_QSTR_draw_box),    MP_ROM_PTR(&ev3dev_Screen_draw_box_obj)                 },
    { MP_ROM_QSTR(MP_QSTR_draw_circle), MP_ROM_PTR(&ev3dev_Screen_draw_circle_obj)              },
    { MP_ROM_QSTR(MP_QSTR_draw_text),   MP_ROM_PTR(&ev3dev_Screen_draw_text_obj)                },
    { MP_ROM_QSTR(MP_QSTR_set_font),    MP_ROM_PTR(&ev3dev_Screen_set_font_obj)                 },
    { MP_ROM_QSTR(MP_QSTR_print),       MP_ROM_PTR(&ev3dev_Screen_print_obj)                    },
    { MP_ROM_QSTR(MP_QSTR_WIDTH),       MP_ROM_ATTRIBUTE_OFFSET(ev3dev_Screen_obj_t, width)     },
    { MP_ROM_QSTR(MP_QSTR_HEIGHT),      MP_ROM_ATTRIBUTE_OFFSET(ev3dev_Screen_obj_t, height)    },
};
STATIC MP_DEFINE_CONST_DICT(ev3dev_Screen_locals_dict, ev3dev_Screen_locals_dict_table);

const mp_obj_type_t pb_type_ev3dev_Screen = {
    { &mp_type_type },
    .name = MP_QSTR_Screen,
    .make_new = ev3dev_Screen_make_new,
    .locals_dict = (mp_obj_dict_t*)&ev3dev_Screen_locals_dict,
};