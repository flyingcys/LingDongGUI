#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE10_BG = 0,
    ID_PAGE10_HEADER = 1,
    ID_PAGE10_TITLE = 2,
    ID_PAGE10_HINT = 3,
    ID_PAGE10_HELP = 4,
    ID_PAGE10_LABEL = 10,

};

static uiWidgetSwipeGestureState_t s_page10Gesture;

static bool uiWidgetSwipePage10Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page10Gesture, 9, msg);
}

static void uiWidgetSwipePage10Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE10_BG,
        .headerId = ID_PAGE10_HEADER,
        .titleId = ID_PAGE10_TITLE,
        .hintId = ID_PAGE10_HINT,
        .helpId = ID_PAGE10_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 9, __RGB(247, 245, 242), "uiWidgetSwipePage10")) {
        return;
    }

    obj = ldLabelInit(ID_PAGE10_LABEL, ID_PAGE10_BG, 94, 92, 292, 72, FONT_ARIAL_16_A8);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage10", ID_PAGE10_LABEL);
        return;
    }
    ldLabelSetText(obj, (uint8_t *)"Single widget\nper page");
    ldLabelSetBackgroundColor(obj, GLCD_COLOR_LIGHT_GREY);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_CENTRE);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);
    connect(ID_PAGE10_BG, SIGNAL_PRESS, uiWidgetSwipePage10Gesture);
    connect(ID_PAGE10_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage10Gesture);
    connect(ID_PAGE10_BG, SIGNAL_RELEASE, uiWidgetSwipePage10Gesture);
}

static void uiWidgetSwipePage10Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage10Func = {
    .init = uiWidgetSwipePage10Init,
    .loop = uiWidgetSwipePage10Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage10",
#endif
};
