#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE06_BG = 0,
    ID_PAGE06_HEADER = 1,
    ID_PAGE06_TITLE = 2,
    ID_PAGE06_HINT = 3,
    ID_PAGE06_HELP = 4,
    ID_PAGE06_SLIDER = 10,

};

static uiWidgetSwipeGestureState_t s_page06Gesture;

static bool uiWidgetSwipePage06Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page06Gesture, 5, msg);
}

static void uiWidgetSwipePage06Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE06_BG,
        .headerId = ID_PAGE06_HEADER,
        .titleId = ID_PAGE06_TITLE,
        .hintId = ID_PAGE06_HINT,
        .helpId = ID_PAGE06_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 5, __RGB(241, 245, 250), "uiWidgetSwipePage06")) {
        return;
    }

    obj = ldSliderInit(ID_PAGE06_SLIDER,
                       ID_PAGE06_BG,
                       76,
                       120,
                       328,
                       (IMAGE_INDICATOR_PNG)->tRegion.tSize.iHeight);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage06", ID_PAGE06_SLIDER);
        return;
    }
    ldSliderSetPercent(obj, 42);
    ldSliderSetImage(obj,
                     IMAGE_SLIDER_PNG,
                     IMAGE_SLIDER_PNG_Mask,
                     IMAGE_INDICATOR_PNG,
                     IMAGE_INDICATOR_PNG_Mask);
    ldSliderSetIndicatorWidth(obj, (IMAGE_INDICATOR_PNG)->tRegion.tSize.iWidth);
    ldSliderSetSelectable(obj, true);
    connect(ID_PAGE06_BG, SIGNAL_PRESS, uiWidgetSwipePage06Gesture);
    connect(ID_PAGE06_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage06Gesture);
    connect(ID_PAGE06_BG, SIGNAL_RELEASE, uiWidgetSwipePage06Gesture);
}

static void uiWidgetSwipePage06Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage06Func = {
    .init = uiWidgetSwipePage06Init,
    .loop = uiWidgetSwipePage06Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage06",
#endif
};
