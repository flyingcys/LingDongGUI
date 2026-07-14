#ifndef TINYUI_NAV_H
#define TINYUI_NAV_H

typedef enum tinyui_nav_dir {
    TINYUI_NAV_LEFT = 0,
    TINYUI_NAV_RIGHT,
    TINYUI_NAV_UP,
    TINYUI_NAV_DOWN,
    TINYUI_NAV_ENTER,
    TINYUI_NAV_BACK
} tinyui_nav_dir_t;

typedef enum tinyui_signal {
    TINYUI_SIGNAL_NONE = 0,
    TINYUI_SIGNAL_PRESS,
    TINYUI_SIGNAL_HOLD_DOWN,
    TINYUI_SIGNAL_RELEASE,
    TINYUI_SIGNAL_CLICKED_ITEM,
    TINYUI_SIGNAL_FINISHED,
    TINYUI_SIGNAL_VALUE_CHANGED
} tinyui_signal_t;

#endif
