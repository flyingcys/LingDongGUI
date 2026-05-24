#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE11_BG = 0,
    ID_PAGE11_HEADER = 1,
    ID_PAGE11_TITLE = 2,
    ID_PAGE11_HINT = 3,
    ID_PAGE11_HELP = 4,
    ID_PAGE11_TEXT = 10,

};

static uiWidgetSwipeGestureState_t s_page11Gesture;

static bool uiWidgetSwipePage11Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page11Gesture, 10, msg);
}

static void uiWidgetSwipePage11Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE11_BG,
        .headerId = ID_PAGE11_HEADER,
        .titleId = ID_PAGE11_TITLE,
        .hintId = ID_PAGE11_HINT,
        .helpId = ID_PAGE11_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 10, __RGB(245, 247, 244), "uiWidgetSwipePage11")) {
        return;
    }

    obj = ldTextInit(ID_PAGE11_TEXT,
                     ID_PAGE11_BG,
                     88,
                     68,
                     304,
                     138,
                     FONT_ARIAL_12,
                     TEXT_BOX_LINE_ALIGN_LEFT,
                     true);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage11", ID_PAGE11_TEXT);
        return;
    }
    ldTextSetBackgroundImage(obj, IMAGE_LETTER_PAPER_BMP, NULL);
    ldTextSetText(obj, "Drag on empty space\nto change widget page.");
    ldTextSetSelectable(obj, true);
    connect(ID_PAGE11_BG, SIGNAL_PRESS, uiWidgetSwipePage11Gesture);
    connect(ID_PAGE11_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage11Gesture);
    connect(ID_PAGE11_BG, SIGNAL_RELEASE, uiWidgetSwipePage11Gesture);
}

static void uiWidgetSwipePage11Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage11Func = {
    .init = uiWidgetSwipePage11Init,
    .loop = uiWidgetSwipePage11Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage11",
#endif
};
