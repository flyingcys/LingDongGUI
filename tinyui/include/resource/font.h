#ifndef TINYUI_FONT_H
#define TINYUI_FONT_H

#include "style/style.h"

#include <stdint.h>

typedef enum tinyui_builtin_font {
    TINYUI_FONT_6X8,
    TINYUI_FONT_16X24,
    TINYUI_FONT_ARIAL_12,
    TINYUI_FONT_ARIAL_16_A8,
} tinyui_builtin_font_t;

#ifdef TINYUI_WIDGET_H
/* Private build compatibility for the pre-M4 widget implementation. */
typedef enum tinyui_font_kind tinyui_font_kind_t;
#else
typedef enum tinyui_font_kind {
    TINYUI_FONT_KIND_BUILTIN,
    TINYUI_FONT_KIND_VRES,
} tinyui_font_kind_t;

struct tinyui_font {
    tinyui_font_kind_t kind;
    union {
        tinyui_builtin_font_t builtin;
        uint32_t vres_address;
    } value;
    uintptr_t _private[2];
};
#endif

tinyui_result_t tinyui_font_from_builtin(tinyui_builtin_font_t builtin,
                                         tinyui_font_t *out);
tinyui_result_t tinyui_font_from_vres(uint32_t address,
                                      tinyui_font_t *out);
void tinyui_font_deinit(tinyui_font_t *font);

#endif
