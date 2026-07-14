#include "style/style.h"
#include "theme/theme.h"

#include <stdint.h>

static const tinyui_theme_t *g_tinyui_theme;

extern void tinyui_runtime_set_last_result(tinyui_result_t result);

static tinyui_result_t tinyui_theme_return(tinyui_result_t result)
{
    tinyui_runtime_set_last_result(result);
    return result;
}

void tinyui_internal_theme_reset(void)
{
    g_tinyui_theme = 0;
}

static int tinyui_style_fields_are_valid(const tinyui_style_t *style)
{
    const uint32_t all_fields = UINT32_C(0xff);

    if ((style->fields & ~all_fields) != 0) {
        return 0;
    }
    if ((style->fields & TINYUI_STYLE_BORDER_WIDTH) != 0 && style->border_width < 0) {
        return 0;
    }
    if ((style->fields & TINYUI_STYLE_RADIUS) != 0 && style->radius < 0) {
        return 0;
    }
    if ((style->fields & TINYUI_STYLE_PADDING) != 0 && style->padding < 0) {
        return 0;
    }
    if ((style->fields & TINYUI_STYLE_FONT) != 0 && style->font == 0) {
        return 0;
    }
    return 1;
}

tinyui_result_t tinyui_obj_apply_style(tinyui_obj_t *obj,
                                       tinyui_part_t part,
                                       tinyui_state_t state,
                                       const tinyui_style_t *style)
{
    if (obj == 0) {
        return tinyui_theme_return(TINYUI_ERROR_INVALID_OBJECT);
    }
    if (style == 0) {
        return tinyui_theme_return(TINYUI_ERROR_INVALID_ARG);
    }
    if (part < TINYUI_PART_MAIN || part > TINYUI_PART_TRACK
        || state < TINYUI_STATE_DEFAULT || state > TINYUI_STATE_FOCUSED) {
        return tinyui_theme_return(TINYUI_ERROR_OUT_OF_RANGE);
    }
    if (!tinyui_style_fields_are_valid(style)) {
        return tinyui_theme_return(TINYUI_ERROR_INVALID_ARG);
    }

    /* M1 freezes validation only; no backend style state or descriptor pointer is retained. */
    return tinyui_theme_return(TINYUI_ERROR_NOT_SUPPORTED);
}

tinyui_result_t tinyui_theme_set(const tinyui_theme_t *theme)
{
    if (theme == 0) {
        return tinyui_theme_return(TINYUI_ERROR_INVALID_ARG);
    }

    g_tinyui_theme = theme;
    return tinyui_theme_return(TINYUI_OK);
}

const tinyui_theme_t *tinyui_theme_get(void)
{
    return g_tinyui_theme;
}

tinyui_result_t tinyui_theme_apply(tinyui_obj_t *obj)
{
    if (obj == 0) {
        return tinyui_theme_return(TINYUI_ERROR_INVALID_OBJECT);
    }
    if (g_tinyui_theme == 0) {
        return tinyui_theme_return(TINYUI_ERROR_INVALID_STATE);
    }

    /* M1 deliberately does not walk the object tree or emulate backend styling. */
    return tinyui_theme_return(TINYUI_ERROR_NOT_SUPPORTED);
}
