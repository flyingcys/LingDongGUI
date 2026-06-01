#include "picoui/native.h"
#include "../backend/ldgui/backend.h"

enum {
    PICOUI_LD_SIGNAL_NO_OPERATION = 0,
    PICOUI_LD_SIGNAL_PRESS = 1,
    PICOUI_LD_SIGNAL_HOLD_DOWN = 2,
    PICOUI_LD_SIGNAL_RELEASE = 3,
    PICOUI_LD_SIGNAL_CLICKED_ITEM = 12,
    PICOUI_LD_SIGNAL_FINISHED = 13,
    PICOUI_LD_SIGNAL_VALUE_CHANGED = 14,
};

enum {
    PICOUI_LD_NAV_UP = 0,
    PICOUI_LD_NAV_DOWN = 1,
    PICOUI_LD_NAV_LEFT = 2,
    PICOUI_LD_NAV_RIGHT = 3,
};

struct picoui_native_image picoui_native_image_wrap(void *tile, void *mask, unsigned int mask_color)
{
    struct picoui_native_image image;

    image.tile = tile;
    image.mask = mask;
    image.mask_color = mask_color;
    return image;
}

struct picoui_native_font picoui_native_font_wrap(void *font)
{
    struct picoui_native_font native_font;

    native_font.font = font;
    return native_font;
}

int picoui_native_signal_to_ld(enum picoui_native_signal signal)
{
    switch (signal) {
    case PICOUI_NATIVE_SIGNAL_PRESS:
        return PICOUI_LD_SIGNAL_PRESS;
    case PICOUI_NATIVE_SIGNAL_HOLD_DOWN:
        return PICOUI_LD_SIGNAL_HOLD_DOWN;
    case PICOUI_NATIVE_SIGNAL_RELEASE:
        return PICOUI_LD_SIGNAL_RELEASE;
    case PICOUI_NATIVE_SIGNAL_CLICKED_ITEM:
        return PICOUI_LD_SIGNAL_CLICKED_ITEM;
    case PICOUI_NATIVE_SIGNAL_FINISHED:
        return PICOUI_LD_SIGNAL_FINISHED;
    case PICOUI_NATIVE_SIGNAL_VALUE_CHANGED:
        return PICOUI_LD_SIGNAL_VALUE_CHANGED;
    case PICOUI_NATIVE_SIGNAL_NONE:
    default:
        return PICOUI_LD_SIGNAL_NO_OPERATION;
    }
}

int picoui_native_readback_policy_to_backend(enum picoui_native_readback_policy policy)
{
    switch (policy) {
    case PICOUI_NATIVE_READBACK_BACKEND_FIELD:
    case PICOUI_NATIVE_READBACK_BACKEND_COMMITTED:
        return PICOUI_BACKEND_DATA_TRUTH_BACKEND_VALUE;
    case PICOUI_NATIVE_READBACK_NOT_APPLICABLE:
    default:
        return PICOUI_BACKEND_DATA_TRUTH_NOT_APPLICABLE;
    }
}
