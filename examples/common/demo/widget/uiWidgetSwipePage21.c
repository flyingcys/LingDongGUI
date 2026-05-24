#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE21_BG = 0,
    ID_PAGE21_HEADER = 1,
    ID_PAGE21_TITLE = 2,
    ID_PAGE21_HINT = 3,
    ID_PAGE21_HELP = 4,
    ID_PAGE21_QRCODE = 10,

};

static uiWidgetSwipeGestureState_t s_page21Gesture;

static bool uiWidgetSwipePage21Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page21Gesture, 20, msg);
}

static void uiWidgetSwipePage21Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE21_BG,
        .headerId = ID_PAGE21_HEADER,
        .titleId = ID_PAGE21_TITLE,
        .hintId = ID_PAGE21_HINT,
        .helpId = ID_PAGE21_HELP,
    };
    void *obj;

    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 20, __RGB(245, 247, 244), "uiWidgetSwipePage21")) {
        return;
    }

    obj = ldQRCodeInit(ID_PAGE21_QRCODE,
                       ID_PAGE21_BG,
                       170,
                       72,
                       140,
                       140,
                       "ldgui",
                       GLCD_COLOR_BLUE,
                       GLCD_COLOR_WHITE,
                       QR_ECC_7,
                       2,
                       5);
    if (obj == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage21", ID_PAGE21_QRCODE);
        return;
    }
    ldQRCodeSetOpacity(obj, 100);
    ldQRCodeSetSelectable(obj, true);
    connect(ID_PAGE21_BG, SIGNAL_PRESS, uiWidgetSwipePage21Gesture);
    connect(ID_PAGE21_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage21Gesture);
    connect(ID_PAGE21_BG, SIGNAL_RELEASE, uiWidgetSwipePage21Gesture);
}

static void uiWidgetSwipePage21Loop(ld_scene_t *ptScene)
{
    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage21Func = {
    .init = uiWidgetSwipePage21Init,
    .loop = uiWidgetSwipePage21Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage21",
#endif
};
