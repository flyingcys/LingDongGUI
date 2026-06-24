#ifndef TINYUI_NATIVE_H
#define TINYUI_NATIVE_H

struct tinyui_native_image {
    void *tile;
    void *mask;
    unsigned int mask_color;
};

struct tinyui_native_font {
    void *font;
};

enum tinyui_native_align {
    TINYUI_NATIVE_ALIGN_START = 0,
    TINYUI_NATIVE_ALIGN_CENTER,
    TINYUI_NATIVE_ALIGN_END,
    TINYUI_NATIVE_ALIGN_STRETCH,
    TINYUI_NATIVE_ALIGN_SPACE_EVENLY,
    TINYUI_NATIVE_ALIGN_SPACE_AROUND,
    TINYUI_NATIVE_ALIGN_SPACE_BETWEEN
};

enum tinyui_native_nav_dir {
    TINYUI_NATIVE_NAV_LEFT = 0,
    TINYUI_NATIVE_NAV_RIGHT,
    TINYUI_NATIVE_NAV_UP,
    TINYUI_NATIVE_NAV_DOWN,
    TINYUI_NATIVE_NAV_ENTER,
    TINYUI_NATIVE_NAV_BACK
};

enum tinyui_native_signal {
    TINYUI_NATIVE_SIGNAL_NONE = 0,
    TINYUI_NATIVE_SIGNAL_PRESS,
    TINYUI_NATIVE_SIGNAL_HOLD_DOWN,
    TINYUI_NATIVE_SIGNAL_RELEASE,
    TINYUI_NATIVE_SIGNAL_CLICKED_ITEM,
    TINYUI_NATIVE_SIGNAL_FINISHED,
    TINYUI_NATIVE_SIGNAL_VALUE_CHANGED
};

enum tinyui_native_readback_policy {
    TINYUI_NATIVE_READBACK_NOT_APPLICABLE = 0,
    TINYUI_NATIVE_READBACK_BACKEND_FIELD,
    TINYUI_NATIVE_READBACK_BACKEND_COMMITTED
};

struct tinyui_native_image tinyui_native_image_wrap(void *tile, void *mask, unsigned int mask_color);

struct tinyui_native_font tinyui_native_font_wrap(void *font);

#endif
