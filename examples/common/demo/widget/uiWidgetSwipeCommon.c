#include "uiWidgetSwipeCommon.h"

static const ldPageFuncGroup_t *c_pages[UI_WIDGET_SWIPE_PAGE_COUNT] = {
    &uiWidgetSwipePage01Func, &uiWidgetSwipePage02Func, &uiWidgetSwipePage03Func,
    &uiWidgetSwipePage04Func, &uiWidgetSwipePage05Func, &uiWidgetSwipePage06Func,
    &uiWidgetSwipePage07Func, &uiWidgetSwipePage08Func, &uiWidgetSwipePage09Func,
    &uiWidgetSwipePage10Func, &uiWidgetSwipePage11Func, &uiWidgetSwipePage12Func,
    &uiWidgetSwipePage13Func, &uiWidgetSwipePage14Func, &uiWidgetSwipePage15Func,
    &uiWidgetSwipePage16Func, &uiWidgetSwipePage17Func, &uiWidgetSwipePage18Func,
    &uiWidgetSwipePage19Func, &uiWidgetSwipePage20Func, &uiWidgetSwipePage21Func,
    &uiWidgetSwipePage22Func, &uiWidgetSwipePage23Func,
};

static const uint8_t *c_titles[UI_WIDGET_SWIPE_PAGE_COUNT] = {
    (const uint8_t *)"Switch",
    (const uint8_t *)"Button",
    (const uint8_t *)"CheckBox",
    (const uint8_t *)"LineEdit",
    (const uint8_t *)"ComboBox",
    (const uint8_t *)"Slider",
    (const uint8_t *)"ScrollSelecter",
    (const uint8_t *)"List",
    (const uint8_t *)"Table",
    (const uint8_t *)"Label",
    (const uint8_t *)"Text",
    (const uint8_t *)"Image",
    (const uint8_t *)"Window",
    (const uint8_t *)"MessageBox",
    (const uint8_t *)"ProgressBar",
    (const uint8_t *)"Gauge",
    (const uint8_t *)"Arc",
    (const uint8_t *)"Graph",
    (const uint8_t *)"DateTime",
    (const uint8_t *)"Calendar",
    (const uint8_t *)"QRCode",
    (const uint8_t *)"RadialMenu",
    (const uint8_t *)"IconSlider",
};

static const uint8_t *c_hints[UI_WIDGET_SWIPE_PAGE_COUNT] = {
    (const uint8_t *)"1 / 23", (const uint8_t *)"2 / 23", (const uint8_t *)"3 / 23",
    (const uint8_t *)"4 / 23", (const uint8_t *)"5 / 23", (const uint8_t *)"6 / 23",
    (const uint8_t *)"7 / 23", (const uint8_t *)"8 / 23", (const uint8_t *)"9 / 23",
    (const uint8_t *)"10 / 23", (const uint8_t *)"11 / 23", (const uint8_t *)"12 / 23",
    (const uint8_t *)"13 / 23", (const uint8_t *)"14 / 23", (const uint8_t *)"15 / 23",
    (const uint8_t *)"16 / 23", (const uint8_t *)"17 / 23", (const uint8_t *)"18 / 23",
    (const uint8_t *)"19 / 23", (const uint8_t *)"20 / 23", (const uint8_t *)"21 / 23",
    (const uint8_t *)"22 / 23", (const uint8_t *)"23 / 23",
};

static bool uiWidgetSwipeCreateLabel(ld_scene_t *ptScene,
                                     uint16_t widgetId,
                                     uint16_t parentId,
                                     int16_t x,
                                     int16_t y,
                                     int16_t width,
                                     int16_t height,
                                     const uint8_t *text,
                                     ldColor textColor,
                                     uint8_t align,
                                     const char *pageName)
{
    ldLabel_t *ptLabel = ldLabelInit(widgetId, parentId, x, y, width, height, FONT_ARIAL_12);

    (void)ptScene;
    if (ptLabel == NULL) {
        uiWidgetInitFailed(pageName, widgetId);
        return false;
    }

    ldLabelSetText(ptLabel, (uint8_t *)text);
    ldLabelSetTextColor(ptLabel, textColor);
    ldLabelSetAlign(ptLabel, align);
    return true;
}

bool uiWidgetSwipeInitChrome(ld_scene_t *ptScene,
                             const uiWidgetSwipeChromeIds_t *ptIds,
                             uint8_t pageIndex,
                             ldColor bgColor,
                             const char *pageName)
{
    ldWindow_t *ptBg;
    ldWindow_t *ptHeader;
    ldLabel_t *ptTitle;
    ldLabel_t *ptHint;
    ldLabel_t *ptHelp;

    if ((ptScene == NULL) || (ptIds == NULL) || (pageIndex >= UI_WIDGET_SWIPE_PAGE_COUNT)) {
        return false;
    }

    ldBaseFocusNavigateInit();

    ptBg = ldWindowInit(ptIds->bgId, 0, 0, 0, LD_CFG_SCREEN_WIDTH, LD_CFG_SCREEN_HEIGHT);
    if (ptBg == NULL) {
        uiWidgetInitFailed(pageName, ptIds->bgId);
        return false;
    }
    ldWindowSetColor(ptBg, bgColor);

    ptHeader = ldWindowInit(ptIds->headerId,
                            ptIds->bgId,
                            0,
                            0,
                            LD_CFG_SCREEN_WIDTH,
                            UI_WIDGET_HEADER_HEIGHT);
    if (ptHeader == NULL) {
        uiWidgetInitFailed(pageName, ptIds->headerId);
        return false;
    }
    ldWindowSetColor(ptHeader, UI_WIDGET_SWIPE_HEADER_BG);

    ptTitle = ldLabelInit(ptIds->titleId, ptIds->bgId, UI_WIDGET_MARGIN, 6, 230, 18, FONT_ARIAL_16_A8);
    if (ptTitle == NULL) {
        uiWidgetInitFailed(pageName, ptIds->titleId);
        return false;
    }
    ldLabelSetText(ptTitle, (uint8_t *)c_titles[pageIndex]);
    ldLabelSetTextColor(ptTitle, GLCD_COLOR_WHITE);
    ldLabelSetAlign(ptTitle, ARM_2D_ALIGN_LEFT);

    ptHint = ldLabelInit(ptIds->hintId,
                         ptIds->bgId,
                         LD_CFG_SCREEN_WIDTH - 72,
                         7,
                         60,
                         16,
                         FONT_ARIAL_12);
    if (ptHint == NULL) {
        uiWidgetInitFailed(pageName, ptIds->hintId);
        return false;
    }
    ldLabelSetText(ptHint, (uint8_t *)c_hints[pageIndex]);
    ldLabelSetTextColor(ptHint, GLCD_COLOR_WHITE);
    ldLabelSetAlign(ptHint, ARM_2D_ALIGN_RIGHT);

    ptHelp = ldLabelInit(ptIds->helpId,
                         ptIds->bgId,
                         UI_WIDGET_MARGIN,
                         LD_CFG_SCREEN_HEIGHT - 20,
                         LD_CFG_SCREEN_WIDTH - (UI_WIDGET_MARGIN * 2),
                         12,
                         FONT_ARIAL_12);
    if (ptHelp == NULL) {
        uiWidgetInitFailed(pageName, ptIds->helpId);
        return false;
    }
    ldLabelSetText(ptHelp, (uint8_t *)UI_WIDGET_SWIPE_HELP_TEXT);
    ldLabelSetTextColor(ptHelp, __RGB(72, 72, 72));
    ldLabelSetAlign(ptHelp, ARM_2D_ALIGN_LEFT);

    return true;
}

bool uiWidgetSwipeHandleGesture(ld_scene_t *ptScene,
                                uiWidgetSwipeGestureState_t *ptGesture,
                                uint8_t pageIndex,
                                ldMsg_t msg)
{
    int16_t x;
    int16_t y;
    uiWidgetSwipeDecision_t decision = UI_WIDGET_SWIPE_NONE;

    (void)ptScene;

    switch (msg.signal) {
    case SIGNAL_PRESS:
        x = (int16_t)GET_SIGNAL_VALUE_X(msg.value);
        y = (int16_t)GET_SIGNAL_VALUE_Y(msg.value);
        uiWidgetSwipeGestureBegin(ptGesture, x, y);
        break;
    case SIGNAL_HOLD_DOWN:
        x = (int16_t)GET_SIGNAL_VALUE_X(msg.value);
        y = (int16_t)GET_SIGNAL_VALUE_Y(msg.value);
        uiWidgetSwipeGestureTrack(ptGesture, x, y);
        break;
    case SIGNAL_RELEASE:
        x = (int16_t)GET_SIGNAL_VALUE_X(msg.value);
        y = (int16_t)GET_SIGNAL_VALUE_Y(msg.value);
        decision = uiWidgetSwipeGestureEnd(ptGesture, x, y, UI_WIDGET_SWIPE_GESTURE_PX);
        if (decision == UI_WIDGET_SWIPE_NEXT) {
            uiWidgetSwipeJumpRelative(pageIndex, 1);
        } else if (decision == UI_WIDGET_SWIPE_PREV) {
            uiWidgetSwipeJumpRelative(pageIndex, -1);
        }
        break;
    default:
        break;
    }

    return false;
}

void uiWidgetSwipeJumpRelative(uint8_t pageIndex, int8_t delta)
{
    uint8_t nextIndex;
    const ldPageFuncGroup_t *ptNext;

    if (pageIndex >= UI_WIDGET_SWIPE_PAGE_COUNT) {
        return;
    }

    nextIndex = uiWidgetSwipeWrapIndex(pageIndex, delta, UI_WIDGET_SWIPE_PAGE_COUNT);
    ptNext = c_pages[nextIndex];
    if (ptNext == NULL) {
        return;
    }

    __ldGuiJumpPage((ldPageFuncGroup_t *)ptNext,
                    (delta > 0) ? &ARM_2D_SCENE_SWITCH_MODE_SLIDE_LEFT
                                : &ARM_2D_SCENE_SWITCH_MODE_SLIDE_RIGHT,
                    UI_WIDGET_PAGE_SWITCH_ANIM_MS);
}

void uiWidgetSwipePageLoop(ld_scene_t *ptScene)
{
    uiWidgetHandleFocusNavigation(ptScene);
}
