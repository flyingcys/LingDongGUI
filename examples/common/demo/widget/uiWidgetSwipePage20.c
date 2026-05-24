#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE20_BG = 0,
    ID_PAGE20_HEADER = 1,
    ID_PAGE20_TITLE = 2,
    ID_PAGE20_HINT = 3,
    ID_PAGE20_HELP = 4,
    ID_PAGE20_CALENDAR = 10,

};

static uiWidgetSwipeGestureState_t s_page20Gesture;

static bool uiWidgetSwipePage20Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page20Gesture, 19, msg);
}

static void uiWidgetSwipePage20Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE20_BG,
        .headerId = ID_PAGE20_HEADER,
        .titleId = ID_PAGE20_TITLE,
        .hintId = ID_PAGE20_HINT,
        .helpId = ID_PAGE20_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 19, __RGB(241, 246, 243), "uiWidgetSwipePage20")) {
        return;
    }

    obj = ldCalendarInit(ID_PAGE20_CALENDAR, ID_PAGE20_BG, 72, 56, 336, 166, FONT_ARIAL_12, 2026, 1, 1);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage20", ID_PAGE20_CALENDAR);
        return;
    }
    ldCalendarSetDayNames(obj, g_widget_day_names);
    ldCalendarSetHeader(obj, true);
    ldCalendarSetHeaderFormat(obj, g_widget_header_format);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);
    connect(ID_PAGE20_BG, SIGNAL_PRESS, uiWidgetSwipePage20Gesture);
    connect(ID_PAGE20_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage20Gesture);
    connect(ID_PAGE20_BG, SIGNAL_RELEASE, uiWidgetSwipePage20Gesture);
}

static void uiWidgetSwipePage20Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage20Func = {
    .init = uiWidgetSwipePage20Init,
    .loop = uiWidgetSwipePage20Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage20",
#endif
};
