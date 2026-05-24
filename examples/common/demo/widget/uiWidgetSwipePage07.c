#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE07_BG = 0,
    ID_PAGE07_HEADER = 1,
    ID_PAGE07_TITLE = 2,
    ID_PAGE07_HINT = 3,
    ID_PAGE07_HELP = 4,
    ID_PAGE07_SCROLL = 10,

};

static uiWidgetSwipeGestureState_t s_page07Gesture;

static bool uiWidgetSwipePage07Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page07Gesture, 6, msg);
}

static void uiWidgetSwipePage07Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE07_BG,
        .headerId = ID_PAGE07_HEADER,
        .titleId = ID_PAGE07_TITLE,
        .hintId = ID_PAGE07_HINT,
        .helpId = ID_PAGE07_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 6, __RGB(245, 245, 239), "uiWidgetSwipePage07")) {
        return;
    }

    obj = ldScrollSelecterInit(ID_PAGE07_SCROLL, ID_PAGE07_BG, 214, 84, 52, 120, FONT_ARIAL_12);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage07", ID_PAGE07_SCROLL);
        return;
    }
    ldScrollSelecterSetItems(obj, g_widget_scroll_items, 5);
    ldScrollSelecterSetBackgroundColor(obj, GLCD_COLOR_WHITE);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);
    connect(ID_PAGE07_BG, SIGNAL_PRESS, uiWidgetSwipePage07Gesture);
    connect(ID_PAGE07_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage07Gesture);
    connect(ID_PAGE07_BG, SIGNAL_RELEASE, uiWidgetSwipePage07Gesture);
}

static void uiWidgetSwipePage07Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage07Func = {
    .init = uiWidgetSwipePage07Init,
    .loop = uiWidgetSwipePage07Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage07",
#endif
};
