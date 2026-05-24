#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE15_BG = 0,
    ID_PAGE15_HEADER = 1,
    ID_PAGE15_TITLE = 2,
    ID_PAGE15_HINT = 3,
    ID_PAGE15_HELP = 4,
    ID_PAGE15_PROGRESS = 10,

};

static uiWidgetSwipeGestureState_t s_page15Gesture;

static bool uiWidgetSwipePage15Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page15Gesture, 14, msg);
}

static void uiWidgetSwipePage15Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE15_BG,
        .headerId = ID_PAGE15_HEADER,
        .titleId = ID_PAGE15_TITLE,
        .hintId = ID_PAGE15_HINT,
        .helpId = ID_PAGE15_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 14, __RGB(246, 244, 239), "uiWidgetSwipePage15")) {
        return;
    }

    obj = ldProgressBarInit(ID_PAGE15_PROGRESS, ID_PAGE15_BG, 78, 120, 324, 28);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage15", ID_PAGE15_PROGRESS);
        return;
    }
    ldProgressBarSetPercent(obj, 45);
    ldProgressBarSetImage(obj, IMAGE_PROGRESSBARBG_BMP, NULL, IMAGE_PROGRESSBARFG_BMP, NULL);
    ldProgressBarSetSelectable(obj, true);
    ldProgressBarSetCorner(obj, true);
    connect(ID_PAGE15_BG, SIGNAL_PRESS, uiWidgetSwipePage15Gesture);
    connect(ID_PAGE15_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage15Gesture);
    connect(ID_PAGE15_BG, SIGNAL_RELEASE, uiWidgetSwipePage15Gesture);
}

static void uiWidgetSwipePage15Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage15Func = {
    .init = uiWidgetSwipePage15Init,
    .loop = uiWidgetSwipePage15Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage15",
#endif
};
