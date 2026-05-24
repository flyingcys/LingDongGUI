#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE17_BG = 0,
    ID_PAGE17_HEADER = 1,
    ID_PAGE17_TITLE = 2,
    ID_PAGE17_HINT = 3,
    ID_PAGE17_HELP = 4,
    ID_PAGE17_ARC = 10,

};

static uiWidgetSwipeGestureState_t s_page17Gesture;

static bool uiWidgetSwipePage17Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page17Gesture, 16, msg);
}

static void uiWidgetSwipePage17Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE17_BG,
        .headerId = ID_PAGE17_HEADER,
        .titleId = ID_PAGE17_TITLE,
        .hintId = ID_PAGE17_HINT,
        .helpId = ID_PAGE17_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 16, __RGB(245, 247, 242), "uiWidgetSwipePage17")) {
        return;
    }

    obj = ldArcInit(ID_PAGE17_ARC,
                    ID_PAGE17_BG,
                    156,
                    72,
                    168,
                    168,
                    IMAGE_ARC_QUARTER_PNG_Mask,
                    IMAGE_ARC_QUARTER_MASK_PNG_Mask,
                    __RGB(240, 240, 240));
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage17", ID_PAGE17_ARC);
        return;
    }
    ldArcSetBackgroundAngle(obj, 0, 350);
    ldArcSetForegroundAngle(obj, 30);
    ldArcSetColor(obj, __RGB(173, 216, 230), __RGB(144, 238, 144));
    ldBaseSetSelectable(obj, true);
    connect(ID_PAGE17_BG, SIGNAL_PRESS, uiWidgetSwipePage17Gesture);
    connect(ID_PAGE17_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage17Gesture);
    connect(ID_PAGE17_BG, SIGNAL_RELEASE, uiWidgetSwipePage17Gesture);
}

static void uiWidgetSwipePage17Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage17Func = {
    .init = uiWidgetSwipePage17Init,
    .loop = uiWidgetSwipePage17Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage17",
#endif
};
