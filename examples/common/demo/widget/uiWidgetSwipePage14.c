#include "uiWidgetSwipeCommon.h"

enum {
    ID_PAGE14_BG = 0,
    ID_PAGE14_HEADER = 1,
    ID_PAGE14_TITLE = 2,
    ID_PAGE14_HINT = 3,
    ID_PAGE14_HELP = 4,
    ID_PAGE14_MSG = 10,

};

static uiWidgetSwipeGestureState_t s_page14Gesture;
static bool s_page14DialogRebuildPending;

static bool uiWidgetSwipePage14Gesture(ld_scene_t *ptScene, ldMsg_t msg)
{
    return uiWidgetSwipeHandleGesture(ptScene, &s_page14Gesture, 13, msg);
}

static void uiWidgetSwipePage14DialogClosed(ld_scene_t *ptScene, ldMessageBox_t *ptWidget)
{
    (void)ptScene;
    (void)ptWidget;
    s_page14DialogRebuildPending = true;
}

static bool uiWidgetSwipePage14CreateMessageBox(ld_scene_t *ptScene)
{
    ldMessageBox_t *ptMessageBox;

    ptMessageBox = ldMessageBoxInit(ID_PAGE14_MSG, ID_PAGE14_BG, 140, 86, FONT_ARIAL_12);
    if (ptMessageBox == NULL) {
        uiWidgetInitFailed("uiWidgetSwipePage14", ID_PAGE14_MSG);
        return false;
    }

    ldMessageBoxSetTitle(ptMessageBox, g_widget_message_title);
    ldMessageBoxSetMsg(ptMessageBox, g_widget_message_text);
    ldMessageBoxSetBtn(ptMessageBox, g_widget_message_buttons, 3);
    ldMessageBoxSetBackgroundColor(ptMessageBox, __RGB(255, 255, 255));
    ldMessageBoxSetCorner(ptMessageBox, true);
    ldMessageBoxSetCallback(ptMessageBox, uiWidgetSwipePage14DialogClosed);
    ldMessageBoxSetHidden(ptMessageBox, false);
    return true;
}

static void uiWidgetSwipePage14Init(ld_scene_t *ptScene)
{
    const uiWidgetSwipeChromeIds_t ids = {
        .bgId = ID_PAGE14_BG,
        .headerId = ID_PAGE14_HEADER,
        .titleId = ID_PAGE14_TITLE,
        .hintId = ID_PAGE14_HINT,
        .helpId = ID_PAGE14_HELP,
    };
    if (!uiWidgetSwipeInitChrome(ptScene, &ids, 13, __RGB(244, 248, 243), "uiWidgetSwipePage14")) {
        return;
    }
    s_page14DialogRebuildPending = false;
    if (!uiWidgetSwipePage14CreateMessageBox(ptScene)) {
        return;
    }
    connect(ID_PAGE14_BG, SIGNAL_PRESS, uiWidgetSwipePage14Gesture);
    connect(ID_PAGE14_BG, SIGNAL_HOLD_DOWN, uiWidgetSwipePage14Gesture);
    connect(ID_PAGE14_BG, SIGNAL_RELEASE, uiWidgetSwipePage14Gesture);
}

static void uiWidgetSwipePage14Loop(ld_scene_t *ptScene)
{
    if (s_page14DialogRebuildPending && (ldBaseGetWidget(ptScene->ptNodeRoot, ID_PAGE14_MSG) == NULL)) {
        s_page14DialogRebuildPending = false;
        if (!uiWidgetSwipePage14CreateMessageBox(ptScene)) {
            return;
        }
    }

    uiWidgetSwipePageLoop(ptScene);
}

const ldPageFuncGroup_t uiWidgetSwipePage14Func = {
    .init = uiWidgetSwipePage14Init,
    .loop = uiWidgetSwipePage14Loop,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetSwipePage14",
#endif
};
