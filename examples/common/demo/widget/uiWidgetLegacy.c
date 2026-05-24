#include "uiWidgetLegacy.h"

#include <stdlib.h>

#include "fonts/uiFonts.h"
#include "images/uiImages.h"
#include "ldGui.h"

static void uiWidgetLegacyInit(ld_scene_t *ptScene);
static void uiWidgetLegacyLoop(ld_scene_t *ptScene);
static void uiWidgetLegacyQuit(ld_scene_t *ptScene);
static bool uiWidgetLegacySwitchValueChanged(ld_scene_t *ptScene, ldMsg_t msg);

enum {
    UI_WIDGET_LEGACY_SWITCH_ID = 30,
    UI_WIDGET_LEGACY_SWITCH_LABEL_ID = 31,
    UI_WIDGET_LEGACY_SWITCH_X = 300,
    UI_WIDGET_LEGACY_SWITCH_Y = 220,
    UI_WIDGET_LEGACY_SWITCH_WIDTH = 72,
    UI_WIDGET_LEGACY_SWITCH_HEIGHT = 36,
    UI_WIDGET_LEGACY_SWITCH_LABEL_X = 380,
    UI_WIDGET_LEGACY_SWITCH_LABEL_Y = 218,
    UI_WIDGET_LEGACY_SWITCH_LABEL_WIDTH = 60,
    UI_WIDGET_LEGACY_SWITCH_LABEL_HEIGHT = 40,
};

enum {
    UI_WIDGET_LEGACY_CAPTURE_H_OFF_ID = 100,
    UI_WIDGET_LEGACY_CAPTURE_H_ON_ID = 101,
    UI_WIDGET_LEGACY_CAPTURE_V_OFF_ID = 102,
    UI_WIDGET_LEGACY_CAPTURE_V_ON_ID = 103,
    UI_WIDGET_LEGACY_CAPTURE_DISABLED_OFF_ID = 104,
    UI_WIDGET_LEGACY_CAPTURE_DISABLED_ON_ID = 105,
};

const ldPageFuncGroup_t uiWidgetLegacyFunc = {
    .init = uiWidgetLegacyInit,
    .loop = uiWidgetLegacyLoop,
    .quit = uiWidgetLegacyQuit,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetLegacy",
#endif
};

static bool uiWidgetLegacySlotTest(ld_scene_t *ptScene, ldMsg_t msg)
{
    (void)msg;
    ldImage_t *img = ldBaseGetWidget(ptScene->ptNodeRoot, 1);
    ldBaseSetOpacity((ldBase_t *)img, 128);
    return false;
}

static bool uiWidgetLegacySwitchValueChanged(ld_scene_t *ptScene, ldMsg_t msg)
{
    ldSwitch_t *sw = ldBaseGetWidget(ptScene->ptNodeRoot, UI_WIDGET_LEGACY_SWITCH_ID);
    ldLabel_t *label = ldBaseGetWidget(ptScene->ptNodeRoot, UI_WIDGET_LEGACY_SWITCH_LABEL_ID);

    (void)msg;
    if ((sw == NULL) || (label == NULL)) {
        return false;
    }

    ldLabelSetText(label, (uint8_t *)(ldSwitchIsChecked(sw) ? "ON" : "OFF"));
    return false;
}

static const uint8_t *g_legacy_scroll_items[] = {"1", "10", "123", "99", "7"};
static const uint8_t *g_legacy_icon_names[] = {"11", "22", "33", "44", "55"};
static const uint8_t *g_legacy_combo_box_items[] = {"11", "22", "00"};
static const uint8_t g_legacy_title_str[] = "title";
static const uint8_t g_legacy_msg_str[] = "12345678abcdefg\n99556";
static const uint8_t *g_legacy_btn_str[] = {"11", "22", "33"};
static uint8_t *g_legacy_day_names[] = {
    (uint8_t *)"Sun",
    (uint8_t *)"Mon",
    (uint8_t *)"Tue",
    (uint8_t *)"Wed",
    (uint8_t *)"Thu",
    (uint8_t *)"Fir",
    (uint8_t *)"Sat",
};
static uint8_t g_legacy_header_format[] = "yyyy - mm - dd";

static bool uiWidgetLegacyCaptureMatrixEnabled(void)
{
    const char *pchEnv = getenv("LD_SWITCH_CAPTURE_MATRIX");

    return (pchEnv != NULL) && (pchEnv[0] != '\0') && (pchEnv[0] != '0');
}

static ldSwitch_t *uiWidgetLegacyInitCaptureSwitch(ld_scene_t *ptScene,
                                                   uint16_t nameId,
                                                   int16_t x,
                                                   int16_t y,
                                                   int16_t width,
                                                   int16_t height)
{
    return ldSwitch_init(ptScene, NULL, nameId, 0, x, y, width, height);
}

static void uiWidgetLegacyInitCaptureMatrix(ld_scene_t *ptScene)
{
    void *obj;

    ldBaseFocusNavigateInit();

    obj = ldWindowInit(0, 0, 0, 0, LD_CFG_SCREEN_WIDTH, LD_CFG_SCREEN_HEIGHT);
    ldWindowSetColor(obj, GLCD_COLOR_BLACK);

    obj = uiWidgetLegacyInitCaptureSwitch(ptScene, UI_WIDGET_LEGACY_CAPTURE_H_OFF_ID, 80, 80, 90, 40);
    ldSwitchSetChecked(obj, false);

    obj = uiWidgetLegacyInitCaptureSwitch(ptScene, UI_WIDGET_LEGACY_CAPTURE_H_ON_ID, 80, 150, 90, 40);
    ldSwitchSetChecked(obj, true);

    obj = uiWidgetLegacyInitCaptureSwitch(ptScene, UI_WIDGET_LEGACY_CAPTURE_V_OFF_ID, 250, 60, 40, 90);
    ldSwitchSetDirection(obj, LD_SWITCH_DIRECTION_VERTICAL);
    ldSwitchSetChecked(obj, false);

    obj = uiWidgetLegacyInitCaptureSwitch(ptScene, UI_WIDGET_LEGACY_CAPTURE_V_ON_ID, 250, 210, 40, 90);
    ldSwitchSetDirection(obj, LD_SWITCH_DIRECTION_VERTICAL);
    ldSwitchSetChecked(obj, true);

    obj = uiWidgetLegacyInitCaptureSwitch(ptScene, UI_WIDGET_LEGACY_CAPTURE_DISABLED_OFF_ID, 80, 290, 90, 40);
    ldSwitchSetChecked(obj, false);
    ldSwitchSetDisabled(obj, true);

    obj = uiWidgetLegacyInitCaptureSwitch(ptScene, UI_WIDGET_LEGACY_CAPTURE_DISABLED_ON_ID, 80, 360, 90, 40);
    ldSwitchSetChecked(obj, true);
    ldSwitchSetDisabled(obj, true);
}

static void uiWidgetLegacyInit(ld_scene_t *ptScene)
{
    void *obj, *list;

    if (uiWidgetLegacyCaptureMatrixEnabled()) {
        uiWidgetLegacyInitCaptureMatrix(ptScene);
        return;
    }

    ldBaseFocusNavigateInit();

    ldWindowInit(0, 0, 0, 0, LD_CFG_SCREEN_WIDTH, LD_CFG_SCREEN_HEIGHT);

    obj = ldImageInit(1, 0, 100, 120, 50, 80, NULL, NULL);
    ldImageSetImage(obj, IMAGE_LETTER_PAPER_BMP, NULL);
    ldBaseSetCorner(obj, true);
    ldBaseSetSelectable(obj, true);

    obj = ldButtonInit(2, 0, 10, 10, 79, 53);
    ldButtonSetFont(obj, FONT_ARIAL_16_A8);
    ldButtonSetText(obj, (uint8_t *)"123");
    ldButtonSetTextColor(obj, GLCD_COLOR_WHITE);
    ldButtonSetImage(obj, IMAGE_KEYRELEASE_PNG, IMAGE_KEYRELEASE_PNG_Mask, IMAGE_KEYPRESS_PNG, IMAGE_KEYPRESS_PNG_Mask);
    ldBaseSetSelectable(obj, true);

    obj = ldWindowInit(3, 0, 200, 95, 20, 20);
    ldWindowSetColor(obj, GLCD_COLOR_GREEN);
    ldBaseSetCorner(obj, true);
    ldBaseSetSelectable(obj, true);

    connect(2, SIGNAL_RELEASE, uiWidgetLegacySlotTest);

    obj = ldLabelInit(4, 0, 100, 50, 100, 50, FONT_ARIAL_12);
    ldLabelSetText(obj, (uint8_t *)"123");
    ldLabelSetBackgroundColor(obj, GLCD_COLOR_LIGHT_GREY);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_BOTTOM_LEFT);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);

    obj = ldCheckBoxInit(5, 0, 220, 10, 50, 20);
    ldCheckBoxSetRadioButtonGroup(obj, 0);
    ldCheckBoxSetText(obj, FONT_ARIAL_12, (uint8_t *)"999");
    ldCheckBoxSetSelectable(obj, true);

    obj = ldCheckBoxInit(6, 0, 220, 40, 50, 20);
    ldCheckBoxSetRadioButtonGroup(obj, 0);
    ldCheckBoxSetSelectable(obj, true);

    obj = ldCheckBoxInit(7, 0, 220, 70, 50, 20);
    ldBaseSetCorner(obj, true);
    ldCheckBoxSetSelectable(obj, true);

    obj = ldSwitchInit(
        UI_WIDGET_LEGACY_SWITCH_ID,
        0,
        UI_WIDGET_LEGACY_SWITCH_X,
        UI_WIDGET_LEGACY_SWITCH_Y,
        UI_WIDGET_LEGACY_SWITCH_WIDTH,
        UI_WIDGET_LEGACY_SWITCH_HEIGHT
    );
    ldSwitchSetChecked(obj, false);
    ldSwitchSetSelectable(obj, true);

    obj = ldLabelInit(
        UI_WIDGET_LEGACY_SWITCH_LABEL_ID,
        0,
        UI_WIDGET_LEGACY_SWITCH_LABEL_X,
        UI_WIDGET_LEGACY_SWITCH_LABEL_Y,
        UI_WIDGET_LEGACY_SWITCH_LABEL_WIDTH,
        UI_WIDGET_LEGACY_SWITCH_LABEL_HEIGHT,
        FONT_ARIAL_16_A8
    );
    ldLabelSetText(obj, (uint8_t *)"OFF");
    ldLabelSetAlign(obj, ARM_2D_ALIGN_MIDDLE_LEFT);
    ldBaseSetSelectable(obj, true);

    connect(UI_WIDGET_LEGACY_SWITCH_ID, SIGNAL_VALUE_CHANGED, uiWidgetLegacySwitchValueChanged);

    obj = ldProgressBarInit(8, 0, 10, 500, 300, 30);
    ldProgressBarSetPercent(obj, 45);
    ldProgressBarSetImage(obj, IMAGE_PROGRESSBARBG_BMP, NULL, IMAGE_PROGRESSBARFG_BMP, NULL);
    ldBaseSetSelectable(obj, true);

    obj = ldTextInit(9, 0, 300, 10, 150, 200, FONT_ARIAL_12, TEXT_BOX_LINE_ALIGN_LEFT, true);
    ldTextSetBackgroundImage(obj, IMAGE_LETTER_PAPER_BMP, NULL);
    ldTextSetText(obj, "123\n12333");
    ldTextSetSelectable(obj, true);
    ldTextSetCorner(obj, true);

    obj = ldSliderInit(10, 0, 50, 300, 317, (IMAGE_INDICATOR_PNG)->tRegion.tSize.iHeight);
    ldSliderSetPercent(obj, 42);
    ldSliderSetImage(obj, IMAGE_SLIDER_PNG, IMAGE_SLIDER_PNG_Mask, IMAGE_INDICATOR_PNG, IMAGE_INDICATOR_PNG_Mask);
    ldSliderSetIndicatorWidth(obj, (IMAGE_INDICATOR_PNG)->tRegion.tSize.iWidth);

    obj = ldSliderInit(11, 0, 400, 300, 30, 100);
    ldSliderSetHorizontal(obj, false);
    ldSliderSetPercent(obj, 42);
    ldSliderSetSelectable(obj, true);
    ldSliderSetCorner(obj, true);

    obj = ldRadialMenuInit(12, 0, 500, 200, 150, 100, 100, 80, 5);
    ldRadialMenuAddItem(obj, IMAGE_WEATHER_PNG, IMAGE_WEATHER_PNG_Mask);
    ldRadialMenuAddItem(obj, IMAGE_NOTE_PNG, IMAGE_NOTE_PNG_Mask);
    ldRadialMenuAddItem(obj, IMAGE_WEATHER_PNG, IMAGE_WEATHER_PNG_Mask);
    ldRadialMenuAddItem(obj, IMAGE_NOTE_PNG, IMAGE_NOTE_PNG_Mask);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);

    obj = ldDateTimeInit(13, 0, 600, 100, 200, 50, FONT_ARIAL_12);
    ldBaseSetSelectable(obj, true);

    obj = ldIconSliderInit(14, 0, 500, 350, 150, 65, 48, 2, 5, 1, 1, FONT_ARIAL_12);
    ldIconSliderAddIcon(obj, IMAGE_NOTE_PNG, IMAGE_NOTE_PNG_Mask, g_legacy_icon_names[0]);
    ldIconSliderAddIcon(obj, IMAGE_BOOK_PNG, IMAGE_BOOK_PNG_Mask, g_legacy_icon_names[1]);
    ldIconSliderAddIcon(obj, IMAGE_WEATHER_PNG, IMAGE_WEATHER_PNG_Mask, g_legacy_icon_names[2]);
    ldIconSliderAddIcon(obj, IMAGE_CHART_PNG, IMAGE_CHART_PNG_Mask, g_legacy_icon_names[3]);
    ldIconSliderAddIcon(obj, IMAGE_NOTE_PNG, IMAGE_NOTE_PNG_Mask, g_legacy_icon_names[4]);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);

    obj = ldQRCodeInit(15, 0, 500, 10, 200, 200, "ldgui", GLCD_COLOR_BLUE, GLCD_COLOR_WHITE, QR_ECC_7, 2, 5);
    ldQRCodeSetOpacity(obj, 100);
    ldQRCodeSetSelectable(obj, true);

    obj = ldScrollSelecterInit(16, 0, 700, 200, 30, 50, FONT_ARIAL_12);
    ldScrollSelecterSetItems(obj, g_legacy_scroll_items, 5);
    ldScrollSelecterSetBackgroundColor(obj, GLCD_COLOR_WHITE);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);

    obj = ldGaugeInit(17, 0, 700, 300, 120, 98, IMAGE_GAUGE_PNG, IMAGE_GAUGE_PNG_Mask, 0, 10);
    ldGaugeSetPointerImage(obj, NULL, IMAGE_GAUGEPOINTER_PNG_Mask, 5, 45);
    ldGaugeSetPointerColor(obj, GLCD_COLOR_BLUE);
    ldGaugeSetAngle(obj, 120);
    ldGaugeSetSelectable(obj, true);

    obj = ldComboBoxInit(18, 0, 700, 420, 100, 30, FONT_ARIAL_12);
    ldComboBoxSetStaticItems(obj, g_legacy_combo_box_items, 3);
    ldComboBoxSetSelectable(obj, true);

    obj = ldGraphInit(19, 0, 830, 10, 100, 100, 2);
    ldGraphSetAxis(obj, 80, 80, 5);
    ldGraphSetGridOffset(obj, 4);
    ldGraphAddSeries(obj, GLCD_COLOR_RED, 2, 16);
    srand(10);
    for (int i = 0; i < 16; i++) {
        ldGraphSetValue(obj, 0, i, rand() % 81);
    }
    ldGraphAddSeries(obj, GLCD_COLOR_LIGHT_GREY, 2, 16);
    for (int i = 0; i < 16; i++) {
        ldGraphSetValue(obj, 1, i, rand() % 81);
    }
    ldGraphSetSelectable(obj, true);

    obj = ldTableInit(20, 0, 780, 150, 200, 100, 10, 6, 1);
    ldTableSetExcelType(obj, FONT_ARIAL_12);
    ldTableSetKeyboard(obj, 22);
    ldTableSetItemText(obj, 1, 1, (uint8_t *)"id");
    ldTableSetItemFont(obj, 1, 1, FONT_ARIAL_12);
    ldTableSetItemText(obj, 1, 2, (uint8_t *)"name");
    ldTableSetItemFont(obj, 1, 2, FONT_ARIAL_12);
    ldTableSetItemText(obj, 1, 3, (uint8_t *)"size");
    ldTableSetItemFont(obj, 1, 3, FONT_ARIAL_12);
    ldTableSetItemText(obj, 2, 1, (uint8_t *)"1");
    ldTableSetItemFont(obj, 2, 1, FONT_ARIAL_12);
    ldTableSetItemText(obj, 2, 2, (uint8_t *)"button");
    ldTableSetItemFont(obj, 2, 2, FONT_ARIAL_12);
    ldTableSetItemText(obj, 2, 3, (uint8_t *)"30*20");
    ldTableSetItemFont(obj, 2, 3, FONT_ARIAL_12);
    ldTableSetItemText(obj, 3, 1, (uint8_t *)"2");
    ldTableSetItemFont(obj, 3, 1, FONT_ARIAL_12);
    ldTableSetItemText(obj, 3, 2, (uint8_t *)"image");
    ldTableSetItemFont(obj, 3, 2, FONT_ARIAL_12);
    ldTableSetItemText(obj, 3, 3, (uint8_t *)"100*100");
    ldTableSetItemFont(obj, 3, 3, FONT_ARIAL_12);
    ldBaseSetSelectable(obj, true);

    obj = ldLineEditInit(21, 0, 850, 400, 100, 50, FONT_ARIAL_12, 16);
    ldLineEditSetText(obj, "123");
    ldLineEditSetKeyboard(obj, 22);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);

    ldWindowInit(23, 0, 850, 450, 100, 100);
    obj = ldButtonInit(24, 23, 8, 3, 30, 30);
    ldButtonSetFont(obj, FONT_ARIAL_16_A8);
    ldButtonSetText(obj, (uint8_t *)"123");

    ldKeyboardInit(22, 0, FONT_ARIAL_12);

    obj = ldArcInit(25, 0, 450, 450, 103, 103, IMAGE_ARC_QUARTER_PNG_Mask, IMAGE_ARC_QUARTER_MASK_PNG_Mask, __RGB(240, 240, 240));
    ldArcSetBackgroundAngle(obj, 0, 350);
    ldArcSetForegroundAngle(obj, 30);
    ldArcSetColor(obj, __RGB(173, 216, 230), __RGB(144, 238, 144));

    list = ldListInit(26, 0, 850, 280, 100, 100);
    ldListSetItemHeight(list, 30);
    ldListSetText(list, g_legacy_scroll_items, 5, FONT_ARIAL_12);
    ldListSetAlign(list, ARM_2D_ALIGN_LEFT);
    ldBaseSetSelectable(list, true);

    obj = ldButtonInit(27, 26, 10, 3, 20, 20);
    ldListSetItemWidget(list, 1, obj);

    obj = ldMessageBoxInit(28, 0, 200, 150, FONT_ARIAL_12);
    ldMessageBoxSetTitle(obj, g_legacy_title_str);
    ldMessageBoxSetMsg(obj, g_legacy_msg_str);
    ldMessageBoxSetBtn(obj, g_legacy_btn_str, 3);

    obj = ldCalendarInit(29, 0, 50, 340, 300, 150, FONT_ARIAL_12, 2026, 1, 1);
    ldCalendarSetDayNames(obj, g_legacy_day_names);
    ldBaseSetCorner(obj, true);
    ldBaseSetSelectable(obj, true);
    ldBaseSetSelect(obj, true);
    ldCalendarSetHeader(obj, true);
    ldCalendarSetHeaderFormat(obj, g_legacy_header_format);
}

static void uiWidgetLegacyLoop(ld_scene_t *ptScene)
{
    static float angle = 120;
    ldGauge_t *ptGauge = ldBaseGetWidgetById(17);

    if (ldTimeOut(100, true)) {
        ldArcSetRotationAngle(ldBaseGetWidgetById(25), angle);
        ldGaugeSetAngle(ptGauge, angle);
        angle += 1;
        if (angle >= 360) {
            angle = 0;
        }
    }

    if (xBtnGetState(KEY_NUM_UP, BTN_RELEASE)) {
        ldBaseFocusNavigate(ptScene, NAV_UP);
    }
    if (xBtnGetState(KEY_NUM_DOWN, BTN_RELEASE)) {
        ldBaseFocusNavigate(ptScene, NAV_DOWN);
    }
    if (xBtnGetState(KEY_NUM_LEFT, BTN_RELEASE)) {
        ldBaseFocusNavigate(ptScene, NAV_LEFT);
    }
    if (xBtnGetState(KEY_NUM_RIGHT, BTN_RELEASE)) {
        ldBaseFocusNavigate(ptScene, NAV_RIGHT);
    }
}

static void uiWidgetLegacyQuit(ld_scene_t *ptScene)
{
    (void)ptScene;
}
