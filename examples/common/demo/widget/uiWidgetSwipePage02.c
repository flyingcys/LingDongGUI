#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE02_BG = 0,
    ID_PAGE02_HEADER = 1,
    ID_PAGE02_TITLE = 2,
    ID_PAGE02_HINT = 3,
    ID_PAGE02_HELP = 4,
    ID_PAGE02_BUTTON = 10,

};

static uiWidgetSwipeGestureState_t s_page02Gesture;

static bool uiWidgetSwipePage02Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page02Gesture, 1, msg);
}

static void uiWidgetSwipePage02Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE02_BG,
        .headerId = ID_PAGE02_HEADER,
        .titleId = ID_PAGE02_TITLE,
        .hintId = ID_PAGE02_HINT,
        .helpId = ID_PAGE02_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 1, __RGB(247, 244, 240), "uiWidgetSwipePage02")) {
        return;
    }

    obj = ldButtonInit(ID_PAGE02_BUTTON, ID_PAGE02_BG, 160, 104, 160, 64);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage02", ID_PAGE02_BUTTON);
        return;
    }
    ldButtonSetFont(obj, FONT_ARIAL_16_A8);
    ldButtonSetText(obj, (uint8_t *)"Press");
    ldButtonSetTextColor(obj, GLCD_COLOR_WHITE);
    ldButtonSetImage(obj,
                     IMAGE_KEYRELEASE_PNG,
                     IMAGE_KEYRELEASE_PNG_Mask,
                     IMAGE_KEYPRESS_PNG,
                     IMAGE_KEYPRESS_PNG_Mask);
    ldBaseSetSelectable(obj, true);
    connect(ID_PAGE02_BG, SIGNAL_PRESS, uiWidgetSwipePage02Gesture);
    connect(ID_PAGE02_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage02Gesture);
    connect(ID_PAGE02_BG, SIGNAL_RELEASE, uiWidgetSwipePage02Gesture);
}

static void uiWidgetSwipePage02Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage02Func = {
    .init = uiWidgetSwipePage02Init,
    .loop = uiWidgetSwipePage02Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage02",
#endif
};
