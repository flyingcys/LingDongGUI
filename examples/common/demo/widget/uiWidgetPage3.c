#include "uiWidgetPage3.h"

#include "uiWidgetPage4.h"

enum {
    ID_PAGE3_BG = 0,
    ID_PAGE3_HEADER = 1,
    ID_PAGE3_TITLE = 2,
    ID_PAGE3_HINT = 3,
    ID_PAGE3_QRCODE = 10,
    ID_PAGE3_DATETIME = 11,
    ID_PAGE3_RADIAL = 12,
    ID_PAGE3_ICON_SLIDER = 13,
};

static ldTimer_t s_page3_switch_timer;

static void uiWidgetPage3Init(ld_scene_t *ptScene);
static void uiWidgetPage3Loop(ld_scene_t *ptScene);
static void uiWidgetPage3Quit(ld_scene_t *ptScene);

const ldPageFuncGroup_t uiWidgetPage3Func = {
    .init = uiWidgetPage3Init,
    .loop = uiWidgetPage3Loop,
    .quit = uiWidgetPage3Quit,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetPage3",
#endif
};

static void uiWidgetPage3Init(ld_scene_t *ptScene)
{
    void *obj;

    s_page3_switch_timer = 0;

    ldBaseFocusNavigateInit();

    obj = ldWindowInit(ID_PAGE3_BG, 0, 0, 0, LD_CFG_SCREEN_WIDTH, LD_CFG_SCREEN_HEIGHT);
    if (NULL == obj) {
        uiWidgetInitFailed("uiWidgetPage3", ID_PAGE3_BG);
        return;
    }
    ldWindowSetColor(obj, __RGB(240, 246, 244));

    obj = ldWindowInit(ID_PAGE3_HEADER, ID_PAGE3_BG, 0, 0, LD_CFG_SCREEN_WIDTH, UI_WIDGET_HEADER_HEIGHT);
    if (NULL == obj) {
        uiWidgetInitFailed("uiWidgetPage3", ID_PAGE3_HEADER);
        return;
    }
    ldWindowSetColor(obj, __RGB(28, 126, 102));

    obj = ldLabelInit(ID_PAGE3_TITLE, ID_PAGE3_BG, UI_WIDGET_MARGIN, 5, 220, 20, FONT_ARIAL_16_A8);
    if (NULL == obj) {
        uiWidgetInitFailed("uiWidgetPage3", ID_PAGE3_TITLE);
        return;
    }
    ldLabelSetText(obj, (uint8_t *)"Page 3  Showcase");
    ldLabelSetTextColor(obj, GLCD_COLOR_WHITE);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);

    obj = ldLabelInit(ID_PAGE3_HINT, ID_PAGE3_BG, LD_CFG_SCREEN_WIDTH - 64, 7, 52, 18, FONT_ARIAL_12);
    if (NULL == obj) {
        uiWidgetInitFailed("uiWidgetPage3", ID_PAGE3_HINT);
        return;
    }
    ldLabelSetText(obj, (uint8_t *)"3 / 4");
    ldLabelSetTextColor(obj, GLCD_COLOR_WHITE);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_RIGHT);

    obj = ldQRCodeInit(ID_PAGE3_QRCODE,
                       ID_PAGE3_BG,
                       18,
                       52,
                       108,
                       108,
                       "ldgui",
                       GLCD_COLOR_BLUE,
                       GLCD_COLOR_WHITE,
                       QR_ECC_7,
                       2,
                       5);
    ldQRCodeSetOpacity(obj, 100);
    ldQRCodeSetSelectable(obj, true);

    obj = ldDateTimeInit(ID_PAGE3_DATETIME, ID_PAGE3_BG, 154, 54, 188, 40, FONT_ARIAL_12);
    ldBaseSetSelectable(obj, true);

    obj = ldRadialMenuInit(ID_PAGE3_RADIAL, ID_PAGE3_BG, 20, 156, 194, 96, 68, 46, 5);
    ldRadialMenuAddItem(obj, IMAGE_WEATHER_PNG, IMAGE_WEATHER_PNG_Mask);
    ldRadialMenuAddItem(obj, IMAGE_NOTE_PNG, IMAGE_NOTE_PNG_Mask);
    ldRadialMenuAddItem(obj, IMAGE_BOOK_PNG, IMAGE_BOOK_PNG_Mask);
    ldRadialMenuAddItem(obj, IMAGE_CHART_PNG, IMAGE_CHART_PNG_Mask);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);

    obj = ldIconSliderInit(ID_PAGE3_ICON_SLIDER, ID_PAGE3_BG, 232, 142, 224, 86, 46, 2, 4, 1, 2, FONT_ARIAL_12);
    ldIconSliderAddIcon(obj, IMAGE_NOTE_PNG, IMAGE_NOTE_PNG_Mask, g_widget_icon_names[0]);
    ldIconSliderAddIcon(obj, IMAGE_BOOK_PNG, IMAGE_BOOK_PNG_Mask, g_widget_icon_names[1]);
    ldIconSliderAddIcon(obj, IMAGE_WEATHER_PNG, IMAGE_WEATHER_PNG_Mask, g_widget_icon_names[2]);
    ldIconSliderAddIcon(obj, IMAGE_CHART_PNG, IMAGE_CHART_PNG_Mask, g_widget_icon_names[3]);
    ldIconSliderAddIcon(obj, IMAGE_NOTE_PNG, IMAGE_NOTE_PNG_Mask, g_widget_icon_names[4]);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);
}

static void uiWidgetPage3Loop(ld_scene_t *ptScene)
{
    if (ldTimeOut(UI_WIDGET_PAGE_DWELL_MS, false, &s_page3_switch_timer)) {
        ldGuiJumpPage(uiWidgetPage4Func, ARM_2D_SCENE_SWITCH_MODE_SLIDE_RIGHT, UI_WIDGET_PAGE_SWITCH_ANIM_MS);
    }

    uiWidgetHandleFocusNavigation(ptScene);
}

static void uiWidgetPage3Quit(ld_scene_t *ptScene)
{
    (void)ptScene;
    s_page3_switch_timer = 0;
}
