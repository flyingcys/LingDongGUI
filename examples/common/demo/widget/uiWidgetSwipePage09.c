#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE09_BG = 0,
    ID_PAGE09_HEADER = 1,
    ID_PAGE09_TITLE = 2,
    ID_PAGE09_HINT = 3,
    ID_PAGE09_HELP = 4,
    ID_PAGE09_TABLE = 10,
};

static uiWidgetSwipeGestureState_t s_page09Gesture;

static bool uiWidgetSwipePage09Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page09Gesture, 8, msg);
}

static void uiWidgetSwipePage09Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE09_BG,
        .headerId = ID_PAGE09_HEADER,
        .titleId = ID_PAGE09_TITLE,
        .hintId = ID_PAGE09_HINT,
        .helpId = ID_PAGE09_HELP,
    };
    ldTable_t *ptTable;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 8, __RGB(246, 244, 250), "uiWidgetSwipePage09")) {
        return;
    }

    ptTable = ldTableInit(ID_PAGE09_TABLE, ID_PAGE09_BG, 56, 78, 368, 120, 4, 3, 1);
    if (ptTable == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage09", ID_PAGE09_TABLE);
        return;
    }
    ldTableSetExcelType(ptTable, FONT_ARIAL_12);
    ldTableSetItemText(ptTable, 1, 1, (uint8_t *)"id");
    ldTableSetItemText(ptTable, 1, 2, (uint8_t *)"name");
    ldTableSetItemText(ptTable, 1, 3, (uint8_t *)"size");
    ldTableSetItemText(ptTable, 2, 1, (uint8_t *)"1");
    ldTableSetItemText(ptTable, 2, 2, (uint8_t *)"switch");
    ldTableSetItemText(ptTable, 2, 3, (uint8_t *)"58x28");
    ldTableSetItemText(ptTable, 3, 1, (uint8_t *)"2");
    ldTableSetItemText(ptTable, 3, 2, (uint8_t *)"button");
    ldTableSetItemText(ptTable, 3, 3, (uint8_t *)"96x52");
    ldTableSetItemText(ptTable, 4, 1, (uint8_t *)"3");
    ldTableSetItemText(ptTable, 4, 2, (uint8_t *)"list");
    ldTableSetItemText(ptTable, 4, 3, (uint8_t *)"288x122");
    ldBaseSetSelectable(ptTable, true);
    connect(ID_PAGE09_BG, SIGNAL_PRESS, uiWidgetSwipePage09Gesture);
    connect(ID_PAGE09_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage09Gesture);
    connect(ID_PAGE09_BG, SIGNAL_RELEASE, uiWidgetSwipePage09Gesture);
}

static void uiWidgetSwipePage09Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage09Func = {
    .init = uiWidgetSwipePage09Init,
    .loop = uiWidgetSwipePage09Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage09",
#endif
};
