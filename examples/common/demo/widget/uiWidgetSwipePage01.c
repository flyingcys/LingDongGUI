#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE01_BG = 0,
    ID_PAGE01_HEADER = 1,
    ID_PAGE01_TITLE = 2,
    ID_PAGE01_HINT = 3,
    ID_PAGE01_HELP = 4,
    ID_PAGE01_SWITCH = 10,

};

static uiWidgetSwipeGestureState_t s_page01Gesture;

static bool uiWidgetSwipePage01Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page01Gesture, 0, msg);
}

static void uiWidgetSwipePage01Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE01_BG,
        .headerId = ID_PAGE01_HEADER,
        .titleId = ID_PAGE01_TITLE,
        .hintId = ID_PAGE01_HINT,
        .helpId = ID_PAGE01_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 0, __RGB(243, 246, 248), "uiWidgetSwipePage01")) {
        return;
    }

    obj = ldSwitchInit(ID_PAGE01_SWITCH, ID_PAGE01_BG, 182, 112, 116, 56);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage01", ID_PAGE01_SWITCH);
        return;
    }
    ldSwitchSetSelectable(obj, true);
    ldSwitchSetChecked(obj, true);
    connect(ID_PAGE01_BG, SIGNAL_PRESS, uiWidgetSwipePage01Gesture);
    connect(ID_PAGE01_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage01Gesture);
    connect(ID_PAGE01_BG, SIGNAL_RELEASE, uiWidgetSwipePage01Gesture);
}

static void uiWidgetSwipePage01Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage01Func = {
    .init = uiWidgetSwipePage01Init,
    .loop = uiWidgetSwipePage01Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage01",
#endif
};
