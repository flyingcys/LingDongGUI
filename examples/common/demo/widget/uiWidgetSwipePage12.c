#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE12_BG = 0,
    ID_PAGE12_HEADER = 1,
    ID_PAGE12_TITLE = 2,
    ID_PAGE12_HINT = 3,
    ID_PAGE12_HELP = 4,
    ID_PAGE12_IMAGE = 10,

};

static uiWidgetSwipeGestureState_t s_page12Gesture;

static bool uiWidgetSwipePage12Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page12Gesture, 11, msg);
}

static void uiWidgetSwipePage12Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE12_BG,
        .headerId = ID_PAGE12_HEADER,
        .titleId = ID_PAGE12_TITLE,
        .hintId = ID_PAGE12_HINT,
        .helpId = ID_PAGE12_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 11, __RGB(242, 246, 248), "uiWidgetSwipePage12")) {
        return;
    }

    obj = ldImageInit(ID_PAGE12_IMAGE, ID_PAGE12_BG, 164, 60, 152, 152, NULL, NULL);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage12", ID_PAGE12_IMAGE);
        return;
    }
    ldImageSetImage(obj, IMAGE_LETTER_PAPER_BMP, NULL);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);
    connect(ID_PAGE12_BG, SIGNAL_PRESS, uiWidgetSwipePage12Gesture);
    connect(ID_PAGE12_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage12Gesture);
    connect(ID_PAGE12_BG, SIGNAL_RELEASE, uiWidgetSwipePage12Gesture);
}

static void uiWidgetSwipePage12Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage12Func = {
    .init = uiWidgetSwipePage12Init,
    .loop = uiWidgetSwipePage12Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage12",
#endif
};
