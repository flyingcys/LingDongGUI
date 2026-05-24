#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE16_BG = 0,
    ID_PAGE16_HEADER = 1,
    ID_PAGE16_TITLE = 2,
    ID_PAGE16_HINT = 3,
    ID_PAGE16_HELP = 4,
    ID_PAGE16_GAUGE = 10,

};

static uiWidgetSwipeGestureState_t s_page16Gesture;

static bool uiWidgetSwipePage16Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page16Gesture, 15, msg);
}

static void uiWidgetSwipePage16Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE16_BG,
        .headerId = ID_PAGE16_HEADER,
        .titleId = ID_PAGE16_TITLE,
        .hintId = ID_PAGE16_HINT,
        .helpId = ID_PAGE16_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 15, __RGB(240, 246, 244), "uiWidgetSwipePage16")) {
        return;
    }

    obj = ldGaugeInit(ID_PAGE16_GAUGE, ID_PAGE16_BG, 152, 72, 176, 140, IMAGE_GAUGE_PNG, IMAGE_GAUGE_PNG_Mask, 0, 10);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage16", ID_PAGE16_GAUGE);
        return;
    }
    ldGaugeSetPointerImage(obj, NULL, IMAGE_GAUGEPOINTER_PNG_Mask, 5, 45);
    ldGaugeSetPointerColor(obj, GLCD_COLOR_BLUE);
    ldGaugeSetAngle(obj, 120.0f);
    ldGaugeSetSelectable(obj, true);
    connect(ID_PAGE16_BG, SIGNAL_PRESS, uiWidgetSwipePage16Gesture);
    connect(ID_PAGE16_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage16Gesture);
    connect(ID_PAGE16_BG, SIGNAL_RELEASE, uiWidgetSwipePage16Gesture);
}

static void uiWidgetSwipePage16Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage16Func = {
    .init = uiWidgetSwipePage16Init,
    .loop = uiWidgetSwipePage16Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage16",
#endif
};
