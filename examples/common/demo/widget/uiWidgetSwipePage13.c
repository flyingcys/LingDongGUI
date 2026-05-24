#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE13_BG = 0,
    ID_PAGE13_HEADER = 1,
    ID_PAGE13_TITLE = 2,
    ID_PAGE13_HINT = 3,
    ID_PAGE13_HELP = 4,
    ID_PAGE13_WINDOW = 10,
    ID_PAGE13_LABEL = 11,

};

static uiWidgetSwipeGestureState_t s_page13Gesture;

static bool uiWidgetSwipePage13Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page13Gesture, 12, msg);
}

static void uiWidgetSwipePage13Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE13_BG,
        .headerId = ID_PAGE13_HEADER,
        .titleId = ID_PAGE13_TITLE,
        .hintId = ID_PAGE13_HINT,
        .helpId = ID_PAGE13_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 12, __RGB(245, 241, 236), "uiWidgetSwipePage13")) {
        return;
    }

    obj = ldWindowInit(ID_PAGE13_WINDOW, ID_PAGE13_BG, 118, 82, 244, 108);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage13", ID_PAGE13_WINDOW);
        return;
    }
    ldWindowSetColor(obj, __RGB(148, 214, 178));
    ldBaseSetCorner(obj, true);
    ldBaseSetSelectable(obj, true);

    obj = ldLabelInit(ID_PAGE13_LABEL, ID_PAGE13_WINDOW, 20, 34, 180, 24, FONT_ARIAL_12);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage13", ID_PAGE13_LABEL);
        return;
    }
    ldLabelSetText(obj, (uint8_t *)"Nested window");
    ldLabelSetTextColor(obj, GLCD_COLOR_BLACK);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);
    connect(ID_PAGE13_BG, SIGNAL_PRESS, uiWidgetSwipePage13Gesture);
    connect(ID_PAGE13_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage13Gesture);
    connect(ID_PAGE13_BG, SIGNAL_RELEASE, uiWidgetSwipePage13Gesture);
}

static void uiWidgetSwipePage13Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage13Func = {
    .init = uiWidgetSwipePage13Init,
    .loop = uiWidgetSwipePage13Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage13",
#endif
};
