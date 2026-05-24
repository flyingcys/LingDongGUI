#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE05_BG = 0,
    ID_PAGE05_HEADER = 1,
    ID_PAGE05_TITLE = 2,
    ID_PAGE05_HINT = 3,
    ID_PAGE05_HELP = 4,
    ID_PAGE05_COMBO = 10,

};

static uiWidgetSwipeGestureState_t s_page05Gesture;

static bool uiWidgetSwipePage05Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page05Gesture, 4, msg);
}

static void uiWidgetSwipePage05Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE05_BG,
        .headerId = ID_PAGE05_HEADER,
        .titleId = ID_PAGE05_TITLE,
        .hintId = ID_PAGE05_HINT,
        .helpId = ID_PAGE05_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 4, __RGB(244, 246, 251), "uiWidgetSwipePage05")) {
        return;
    }

    obj = ldComboBoxInit(ID_PAGE05_COMBO, ID_PAGE05_BG, 146, 106, 188, 32, FONT_ARIAL_12);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage05", ID_PAGE05_COMBO);
        return;
    }
    ldComboBoxSetStaticItems(obj, g_widget_combo_box_items, 3);
    ldComboBoxSetSelectable(obj, true);
    connect(ID_PAGE05_BG, SIGNAL_PRESS, uiWidgetSwipePage05Gesture);
    connect(ID_PAGE05_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage05Gesture);
    connect(ID_PAGE05_BG, SIGNAL_RELEASE, uiWidgetSwipePage05Gesture);
}

static void uiWidgetSwipePage05Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage05Func = {
    .init = uiWidgetSwipePage05Init,
    .loop = uiWidgetSwipePage05Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage05",
#endif
};
