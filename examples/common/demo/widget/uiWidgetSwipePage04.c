#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE04_BG = 0,
    ID_PAGE04_HEADER = 1,
    ID_PAGE04_TITLE = 2,
    ID_PAGE04_HINT = 3,
    ID_PAGE04_HELP = 4,
    ID_PAGE04_EDIT = 10,
};

static uiWidgetSwipeGestureState_t s_page04Gesture;

static bool uiWidgetSwipePage04Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page04Gesture, 3, msg);
}

static void uiWidgetSwipePage04Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE04_BG,
        .headerId = ID_PAGE04_HEADER,
        .titleId = ID_PAGE04_TITLE,
        .hintId = ID_PAGE04_HINT,
        .helpId = ID_PAGE04_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 3, __RGB(246, 243, 238), "uiWidgetSwipePage04")) {
        return;
    }

    obj = ldLineEditInit(ID_PAGE04_EDIT, ID_PAGE04_BG, 110, 96, 260, 40, FONT_ARIAL_12, 16);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage04", ID_PAGE04_EDIT);
        return;
    }
    ldLineEditSetText(obj, (uint8_t *)"switch");
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);
    connect(ID_PAGE04_BG, SIGNAL_PRESS, uiWidgetSwipePage04Gesture);
    connect(ID_PAGE04_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage04Gesture);
    connect(ID_PAGE04_BG, SIGNAL_RELEASE, uiWidgetSwipePage04Gesture);
}

static void uiWidgetSwipePage04Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage04Func = {
    .init = uiWidgetSwipePage04Init,
    .loop = uiWidgetSwipePage04Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage04",
#endif
};
