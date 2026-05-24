#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE19_BG = 0,
    ID_PAGE19_HEADER = 1,
    ID_PAGE19_TITLE = 2,
    ID_PAGE19_HINT = 3,
    ID_PAGE19_HELP = 4,
    ID_PAGE19_DATETIME = 10,

};

static uiWidgetSwipeGestureState_t s_page19Gesture;

static bool uiWidgetSwipePage19Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page19Gesture, 18, msg);
}

static void uiWidgetSwipePage19Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE19_BG,
        .headerId = ID_PAGE19_HEADER,
        .titleId = ID_PAGE19_TITLE,
        .hintId = ID_PAGE19_HINT,
        .helpId = ID_PAGE19_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 18, __RGB(246, 244, 239), "uiWidgetSwipePage19")) {
        return;
    }

    obj = ldDateTimeInit(ID_PAGE19_DATETIME, ID_PAGE19_BG, 110, 112, 260, 44, FONT_ARIAL_12);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage19", ID_PAGE19_DATETIME);
        return;
    }
    ldBaseSetSelectable(obj, true);
    connect(ID_PAGE19_BG, SIGNAL_PRESS, uiWidgetSwipePage19Gesture);
    connect(ID_PAGE19_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage19Gesture);
    connect(ID_PAGE19_BG, SIGNAL_RELEASE, uiWidgetSwipePage19Gesture);
}

static void uiWidgetSwipePage19Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage19Func = {
    .init = uiWidgetSwipePage19Init,
    .loop = uiWidgetSwipePage19Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage19",
#endif
};
