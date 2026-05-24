#include "uiWidgetSwipeCommon.h"

static const uint16_t c_page18_values_a[8] = {12, 26, 18, 40, 32, 48, 30, 54};
static const uint16_t c_page18_values_b[8] = {20, 12, 30, 16, 42, 20, 44, 34};

enum {
    ID_PAGE18_BG = 0,
    ID_PAGE18_HEADER = 1,
    ID_PAGE18_TITLE = 2,
    ID_PAGE18_HINT = 3,
    ID_PAGE18_HELP = 4,
    ID_PAGE18_GRAPH = 10,

};

static uiWidgetSwipeGestureState_t s_page18Gesture;

static bool uiWidgetSwipePage18Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page18Gesture, 17, msg);
}

static void uiWidgetSwipePage18Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE18_BG,
        .headerId = ID_PAGE18_HEADER,
        .titleId = ID_PAGE18_TITLE,
        .hintId = ID_PAGE18_HINT,
        .helpId = ID_PAGE18_HELP,
    };
    ldGraph_t *ptGraph;
    uint16_t i;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 17, __RGB(244, 243, 248), "uiWidgetSwipePage18")) {
        return;
    }

    ptGraph = ldGraphInit(ID_PAGE18_GRAPH, ID_PAGE18_BG, 86, 68, 308, 148, 2);
    if (ptGraph == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage18", ID_PAGE18_GRAPH);
        return;
    }
    ldGraphSetAxis(ptGraph, 48, 56, 4);
    ldGraphSetGridOffset(ptGraph, 4);
    ldGraphAddSeries(ptGraph, GLCD_COLOR_RED, 2, 8);
    ldGraphAddSeries(ptGraph, GLCD_COLOR_LIGHT_GREY, 2, 8);
    for (i = 0; i < 8; i++) {
        ldGraphSetValue(ptGraph, 0, i, c_page18_values_a[i]);
        ldGraphSetValue(ptGraph, 1, i, c_page18_values_b[i]);
    }
    ldGraphSetSelectable(ptGraph, true);
    ldGraphSetCorner(ptGraph, true);
    connect(ID_PAGE18_BG, SIGNAL_PRESS, uiWidgetSwipePage18Gesture);
    connect(ID_PAGE18_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage18Gesture);
    connect(ID_PAGE18_BG, SIGNAL_RELEASE, uiWidgetSwipePage18Gesture);
}

static void uiWidgetSwipePage18Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage18Func = {
    .init = uiWidgetSwipePage18Init,
    .loop = uiWidgetSwipePage18Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage18",
#endif
};
