#include "uiWidgetPage2.h"

#include "uiWidgetPage3.h"

enum {
    ID_PAGE2_BG = 0,
    ID_PAGE2_HEADER = 1,
    ID_PAGE2_TITLE = 2,
    ID_PAGE2_HINT = 3,
    ID_PAGE2_PROGRESS = 10,
    ID_PAGE2_PROGRESS_LABEL = 11,
    ID_PAGE2_TEXT = 12,
    ID_PAGE2_SLIDER_H = 13,
    ID_PAGE2_SLIDER_V = 14,
};

static ldTimer_t s_page2_switch_timer;

static void uiWidgetPage2Init(ld_scene_t *ptScene);
static void uiWidgetPage2Loop(ld_scene_t *ptScene);
static void uiWidgetPage2Quit(ld_scene_t *ptScene);

const ldPageFuncGroup_t uiWidgetPage2Func = {
    .init = uiWidgetPage2Init,
    .loop = uiWidgetPage2Loop,
    .quit = uiWidgetPage2Quit,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetPage2",
#endif
};

static void uiWidgetPage2Init(ld_scene_t *ptScene)
{
    void *obj;

    s_page2_switch_timer = 0;

    ldBaseFocusNavigateInit();

    obj = ldWindowInit(ID_PAGE2_BG, 0, 0, 0, LD_CFG_SCREEN_WIDTH, LD_CFG_SCREEN_HEIGHT);
    if (NULL == obj) {
        uiWidgetInitFailed("uiWidgetPage2", ID_PAGE2_BG);
        return;
    }
    ldWindowSetColor(obj, __RGB(247, 244, 240));

    obj = ldWindowInit(ID_PAGE2_HEADER, ID_PAGE2_BG, 0, 0, LD_CFG_SCREEN_WIDTH, UI_WIDGET_HEADER_HEIGHT);
    if (NULL == obj) {
        uiWidgetInitFailed("uiWidgetPage2", ID_PAGE2_HEADER);
        return;
    }
    ldWindowSetColor(obj, __RGB(150, 88, 41));

    obj = ldLabelInit(ID_PAGE2_TITLE, ID_PAGE2_BG, UI_WIDGET_MARGIN, 5, 220, 20, FONT_ARIAL_16_A8);
    if (NULL == obj) {
        uiWidgetInitFailed("uiWidgetPage2", ID_PAGE2_TITLE);
        return;
    }
    ldLabelSetText(obj, (uint8_t *)"Page 2  Inputs");
    ldLabelSetTextColor(obj, GLCD_COLOR_WHITE);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);

    obj = ldLabelInit(ID_PAGE2_HINT, ID_PAGE2_BG, LD_CFG_SCREEN_WIDTH - 64, 7, 52, 18, FONT_ARIAL_12);
    if (NULL == obj) {
        uiWidgetInitFailed("uiWidgetPage2", ID_PAGE2_HINT);
        return;
    }
    ldLabelSetText(obj, (uint8_t *)"2 / 4");
    ldLabelSetTextColor(obj, GLCD_COLOR_WHITE);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_RIGHT);

    obj = ldLabelInit(ID_PAGE2_PROGRESS_LABEL, ID_PAGE2_BG, 20, 44, 120, 16, FONT_ARIAL_12);
    if (NULL == obj) {
        uiWidgetInitFailed("uiWidgetPage2", ID_PAGE2_PROGRESS_LABEL);
        return;
    }
    ldLabelSetText(obj, (uint8_t *)"Progress 45%");
    ldLabelSetTextColor(obj, GLCD_COLOR_BLACK);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);

    obj = ldProgressBarInit(ID_PAGE2_PROGRESS, ID_PAGE2_BG, 20, 62, 248, 24);
    ldProgressBarSetPercent(obj, 45);
    ldProgressBarSetImage(obj, IMAGE_PROGRESSBARBG_BMP, NULL, IMAGE_PROGRESSBARFG_BMP, NULL);
    ldProgressBarSetSelectable(obj, true);
    ldProgressBarSetCorner(obj, true);

    obj = ldTextInit(ID_PAGE2_TEXT,
                     ID_PAGE2_BG,
                     20,
                     96,
                     248,
                     120,
                     FONT_ARIAL_12,
                     TEXT_BOX_LINE_ALIGN_LEFT,
                     true);
    ldTextSetBackgroundImage(obj, IMAGE_LETTER_PAPER_BMP, NULL);
    ldTextSetText(obj, "123\n12333\nSlider page");
    ldTextSetSelectable(obj, true);
    ldTextSetCorner(obj, true);

    obj = ldSliderInit(ID_PAGE2_SLIDER_H,
                       ID_PAGE2_BG,
                       20,
                       226,
                       248,
                       (IMAGE_INDICATOR_PNG)->tRegion.tSize.iHeight);
    ldSliderSetPercent(obj, 42);
    ldSliderSetImage(obj,
                     IMAGE_SLIDER_PNG,
                     IMAGE_SLIDER_PNG_Mask,
                     IMAGE_INDICATOR_PNG,
                     IMAGE_INDICATOR_PNG_Mask);
    ldSliderSetIndicatorWidth(obj, (IMAGE_INDICATOR_PNG)->tRegion.tSize.iWidth);
    ldSliderSetSelectable(obj, true);

    obj = ldSliderInit(ID_PAGE2_SLIDER_V, ID_PAGE2_BG, 334, 62, 32, 154);
    ldSliderSetHorizontal(obj, false);
    ldSliderSetPercent(obj, 42);
    ldSliderSetSelectable(obj, true);
    ldSliderSetCorner(obj, true);
}

static void uiWidgetPage2Loop(ld_scene_t *ptScene)
{
    if (ldTimeOut(UI_WIDGET_PAGE_DWELL_MS, false, &s_page2_switch_timer)) {
        ldGuiJumpPage(uiWidgetPage3Func, ARM_2D_SCENE_SWITCH_MODE_SLIDE_UP, UI_WIDGET_PAGE_SWITCH_ANIM_MS);
    }

    uiWidgetHandleFocusNavigation(ptScene);
}

static void uiWidgetPage2Quit(ld_scene_t *ptScene)
{
    (void)ptScene;
    s_page2_switch_timer = 0;
}
