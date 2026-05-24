#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE08_BG = 0,
    ID_PAGE08_HEADER = 1,
    ID_PAGE08_TITLE = 2,
    ID_PAGE08_HINT = 3,
    ID_PAGE08_HELP = 4,
    ID_PAGE08_LIST = 10,

};

static uiWidgetSwipeGestureState_t s_page08Gesture;

static bool uiWidgetSwipePage08Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page08Gesture, 7, msg);
}

static void uiWidgetSwipePage08Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE08_BG,
        .headerId = ID_PAGE08_HEADER,
        .titleId = ID_PAGE08_TITLE,
        .hintId = ID_PAGE08_HINT,
        .helpId = ID_PAGE08_HELP,
    };
    ldList_t *ptList;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 7, __RGB(244, 248, 245), "uiWidgetSwipePage08")) {
        return;
    }

    ptList = ldListInit(ID_PAGE08_LIST, ID_PAGE08_BG, 96, 78, 288, 122);
    if (ptList == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage08", ID_PAGE08_LIST);
        return;
    }
    ldListSetItemHeight(ptList, 24);
    ldListSetText(ptList, g_widget_scroll_items, 5, FONT_ARIAL_12);
    ldListSetAlign(ptList, ARM_2D_ALIGN_LEFT);
    ldBaseSetSelectable(ptList, true);
    ldBaseSetCorner(ptList, true);
    connect(ID_PAGE08_BG, SIGNAL_PRESS, uiWidgetSwipePage08Gesture);
    connect(ID_PAGE08_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage08Gesture);
    connect(ID_PAGE08_BG, SIGNAL_RELEASE, uiWidgetSwipePage08Gesture);
}

static void uiWidgetSwipePage08Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage08Func = {
    .init = uiWidgetSwipePage08Init,
    .loop = uiWidgetSwipePage08Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage08",
#endif
};
