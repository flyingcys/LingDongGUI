#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE03_BG = 0,
    ID_PAGE03_HEADER = 1,
    ID_PAGE03_TITLE = 2,
    ID_PAGE03_HINT = 3,
    ID_PAGE03_HELP = 4,
    ID_PAGE03_CHECK_A = 10,
    ID_PAGE03_CHECK_B = 11,

};

static uiWidgetSwipeGestureState_t s_page03Gesture;

static bool uiWidgetSwipePage03Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page03Gesture, 2, msg);
}

static void uiWidgetSwipePage03Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE03_BG,
        .headerId = ID_PAGE03_HEADER,
        .titleId = ID_PAGE03_TITLE,
        .hintId = ID_PAGE03_HINT,
        .helpId = ID_PAGE03_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 2, __RGB(240, 246, 244), "uiWidgetSwipePage03")) {
        return;
    }

    obj = ldCheckBoxInit(ID_PAGE03_CHECK_A, ID_PAGE03_BG, 164, 94, 152, 24);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage03", ID_PAGE03_CHECK_A);
        return;
    }
    ldCheckBoxSetRadioButtonGroup(obj, 0);
    ldCheckBoxSetText(obj, FONT_ARIAL_12, (uint8_t *)"Option A");
    ldCheckBoxSetSelectable(obj, true);

    obj = ldCheckBoxInit(ID_PAGE03_CHECK_B, ID_PAGE03_BG, 164, 130, 152, 24);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage03", ID_PAGE03_CHECK_B);
        return;
    }
    ldCheckBoxSetRadioButtonGroup(obj, 0);
    ldCheckBoxSetText(obj, FONT_ARIAL_12, (uint8_t *)"Option B");
    ldCheckBoxSetSelectable(obj, true);
    connect(ID_PAGE03_BG, SIGNAL_PRESS, uiWidgetSwipePage03Gesture);
    connect(ID_PAGE03_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage03Gesture);
    connect(ID_PAGE03_BG, SIGNAL_RELEASE, uiWidgetSwipePage03Gesture);
}

static void uiWidgetSwipePage03Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage03Func = {
    .init = uiWidgetSwipePage03Init,
    .loop = uiWidgetSwipePage03Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage03",
#endif
};
