#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE23_BG = 0,
    ID_PAGE23_HEADER = 1,
    ID_PAGE23_TITLE = 2,
    ID_PAGE23_HINT = 3,
    ID_PAGE23_HELP = 4,
    ID_PAGE23_ICON_SLIDER = 10,

};

static uiWidgetSwipeGestureState_t s_page23Gesture;

static bool uiWidgetSwipePage23Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page23Gesture, 22, msg);
}

static void uiWidgetSwipePage23Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE23_BG,
        .headerId = ID_PAGE23_HEADER,
        .titleId = ID_PAGE23_TITLE,
        .hintId = ID_PAGE23_HINT,
        .helpId = ID_PAGE23_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 22, __RGB(244, 244, 239), "uiWidgetSwipePage23")) {
        return;
    }

    obj = ldIconSliderInit(ID_PAGE23_ICON_SLIDER, ID_PAGE23_BG, 78, 120, 324, 86, 46, 2, 4, 1, 2, FONT_ARIAL_12);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage23", ID_PAGE23_ICON_SLIDER);
        return;
    }
    ldIconSliderAddIcon(obj, IMAGE_NOTE_PNG, IMAGE_NOTE_PNG_Mask, g_widget_icon_names[0]);
    ldIconSliderAddIcon(obj, IMAGE_BOOK_PNG, IMAGE_BOOK_PNG_Mask, g_widget_icon_names[1]);
    ldIconSliderAddIcon(obj, IMAGE_WEATHER_PNG, IMAGE_WEATHER_PNG_Mask, g_widget_icon_names[2]);
    ldIconSliderAddIcon(obj, IMAGE_CHART_PNG, IMAGE_CHART_PNG_Mask, g_widget_icon_names[3]);
    ldIconSliderAddIcon(obj, IMAGE_NOTE_PNG, IMAGE_NOTE_PNG_Mask, g_widget_icon_names[4]);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);
    connect(ID_PAGE23_BG, SIGNAL_PRESS, uiWidgetSwipePage23Gesture);
    connect(ID_PAGE23_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage23Gesture);
    connect(ID_PAGE23_BG, SIGNAL_RELEASE, uiWidgetSwipePage23Gesture);
}

static void uiWidgetSwipePage23Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage23Func = {
    .init = uiWidgetSwipePage23Init,
    .loop = uiWidgetSwipePage23Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage23",
#endif
};
