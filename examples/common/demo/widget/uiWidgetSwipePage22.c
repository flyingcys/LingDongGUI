#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE22_BG = 0,
    ID_PAGE22_HEADER = 1,
    ID_PAGE22_TITLE = 2,
    ID_PAGE22_HINT = 3,
    ID_PAGE22_HELP = 4,
    ID_PAGE22_RADIAL = 10,

};

static uiWidgetSwipeGestureState_t s_page22Gesture;

static bool uiWidgetSwipePage22Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page22Gesture, 21, msg);
}

static void uiWidgetSwipePage22Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE22_BG,
        .headerId = ID_PAGE22_HEADER,
        .titleId = ID_PAGE22_TITLE,
        .hintId = ID_PAGE22_HINT,
        .helpId = ID_PAGE22_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 21, __RGB(243, 246, 248), "uiWidgetSwipePage22")) {
        return;
    }

    obj = ldRadialMenuInit(ID_PAGE22_RADIAL, ID_PAGE22_BG, 102, 102, 276, 108, 68, 46, 5);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage22", ID_PAGE22_RADIAL);
        return;
    }
    ldRadialMenuAddItem(obj, IMAGE_WEATHER_PNG, IMAGE_WEATHER_PNG_Mask);
    ldRadialMenuAddItem(obj, IMAGE_NOTE_PNG, IMAGE_NOTE_PNG_Mask);
    ldRadialMenuAddItem(obj, IMAGE_BOOK_PNG, IMAGE_BOOK_PNG_Mask);
    ldRadialMenuAddItem(obj, IMAGE_CHART_PNG, IMAGE_CHART_PNG_Mask);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);
    connect(ID_PAGE22_BG, SIGNAL_PRESS, uiWidgetSwipePage22Gesture);
    connect(ID_PAGE22_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage22Gesture);
    connect(ID_PAGE22_BG, SIGNAL_RELEASE, uiWidgetSwipePage22Gesture);
}

static void uiWidgetSwipePage22Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage22Func = {
    .init = uiWidgetSwipePage22Init,
    .loop = uiWidgetSwipePage22Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage22",
#endif
};
