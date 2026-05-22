#include "uiWidgetPage4.h"

#include "uiWidgetPage1.h"

enum {
    ID_PAGE4_BG = 0,
    ID_PAGE4_HEADER = 1,
    ID_PAGE4_TITLE = 2,
    ID_PAGE4_HINT = 3,
    ID_PAGE4_GAUGE = 10,
    ID_PAGE4_ARC = 11,
    ID_PAGE4_SCROLL = 12,
    ID_PAGE4_COMBO = 13,
    ID_PAGE4_LINE_EDIT = 14,
    ID_PAGE4_GRAPH = 15,
    ID_PAGE4_TABLE = 16,
    ID_PAGE4_LIST = 17,
    ID_PAGE4_LIST_BUTTON = 18,
    ID_PAGE4_MESSAGE_BOX = 19,
    ID_PAGE4_CALENDAR = 20,
};

static ldTimer_t s_page4_switch_timer;
static ldTimer_t s_page4_dynamic_timer;
static float s_page4_angle;

static void uiWidgetPage4Init(ld_scene_t *ptScene);
static void uiWidgetPage4Loop(ld_scene_t *ptScene);
static void uiWidgetPage4Quit(ld_scene_t *ptScene);

const ldPageFuncGroup_t uiWidgetPage4Func = {
    .init = uiWidgetPage4Init,
    .loop = uiWidgetPage4Loop,
    .quit = uiWidgetPage4Quit,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetPage4",
#endif
};

static void uiWidgetPage4Init(ld_scene_t *ptScene)
{
    static const uint16_t c_graph_values_a[8] = {12, 26, 18, 40, 32, 48, 30, 54};
    static const uint16_t c_graph_values_b[8] = {20, 12, 30, 16, 42, 20, 44, 34};
    void *obj;
    ldGraph_t *ptGraph;
    ldTable_t *ptTable;
    ldList_t *ptList;

    s_page4_switch_timer = 0;
    s_page4_dynamic_timer = 0;
    s_page4_angle = 120.0f;

    ldBaseFocusNavigateInit();

    obj = ldWindowInit(ID_PAGE4_BG, 0, 0, 0, LD_CFG_SCREEN_WIDTH, LD_CFG_SCREEN_HEIGHT);
    ldWindowSetColor(obj, __RGB(245, 243, 238));

    obj = ldWindowInit(ID_PAGE4_HEADER, ID_PAGE4_BG, 0, 0, LD_CFG_SCREEN_WIDTH, UI_WIDGET_HEADER_HEIGHT);
    ldWindowSetColor(obj, __RGB(96, 86, 124));

    obj = ldLabelInit(ID_PAGE4_TITLE, ID_PAGE4_BG, UI_WIDGET_MARGIN, 5, 220, 20, FONT_ARIAL_16_A8);
    ldLabelSetText(obj, (uint8_t *)"Page 4  Data");
    ldLabelSetTextColor(obj, GLCD_COLOR_WHITE);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);

    obj = ldLabelInit(ID_PAGE4_HINT, ID_PAGE4_BG, LD_CFG_SCREEN_WIDTH - 64, 7, 52, 18, FONT_ARIAL_12);
    ldLabelSetText(obj, (uint8_t *)"4 / 4");
    ldLabelSetTextColor(obj, GLCD_COLOR_WHITE);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_RIGHT);

    obj = ldGaugeInit(ID_PAGE4_GAUGE, ID_PAGE4_BG, 12, 44, 110, 92, IMAGE_GAUGE_PNG, IMAGE_GAUGE_PNG_Mask, 0, 10);
    ldGaugeSetPointerImage(obj, NULL, IMAGE_GAUGEPOINTER_PNG_Mask, 5, 45);
    ldGaugeSetPointerColor(obj, GLCD_COLOR_BLUE);
    ldGaugeSetAngle(obj, s_page4_angle);
    ldGaugeSetSelectable(obj, true);

    obj = ldArcInit(ID_PAGE4_ARC,
                    ID_PAGE4_BG,
                    132,
                    44,
                    94,
                    94,
                    IMAGE_ARC_QUARTER_PNG_Mask,
                    IMAGE_ARC_QUARTER_MASK_PNG_Mask,
                    __RGB(240, 240, 240));
    ldArcSetBackgroundAngle(obj, 0, 350);
    ldArcSetForegroundAngle(obj, 30);
    ldArcSetColor(obj, __RGB(173, 216, 230), __RGB(144, 238, 144));

    obj = ldScrollSelecterInit(ID_PAGE4_SCROLL, ID_PAGE4_BG, 236, 46, 34, 76, FONT_ARIAL_12);
    ldScrollSelecterSetItems(obj, g_widget_scroll_items, 5);
    ldScrollSelecterSetBackgroundColor(obj, GLCD_COLOR_WHITE);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);

    obj = ldComboBoxInit(ID_PAGE4_COMBO, ID_PAGE4_BG, 280, 50, 82, 28, FONT_ARIAL_12);
    ldComboBoxSetStaticItems(obj, g_widget_combo_box_items, 3);
    ldComboBoxSetSelectable(obj, true);

    obj = ldLineEditInit(ID_PAGE4_LINE_EDIT, ID_PAGE4_BG, 280, 88, 82, 34, FONT_ARIAL_12, 16);
    ldLineEditSetText(obj, (uint8_t *)"123");
    ldLineEditSetKeyboard(obj, UI_WIDGET_KEYBOARD_ID);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);

    ptGraph = ldGraphInit(ID_PAGE4_GRAPH, ID_PAGE4_BG, 370, 44, 98, 88, 2);
    ldGraphSetAxis(ptGraph, 48, 56, 4);
    ldGraphSetGridOffset(ptGraph, 4);
    ldGraphAddSeries(ptGraph, GLCD_COLOR_RED, 2, 8);
    ldGraphAddSeries(ptGraph, GLCD_COLOR_LIGHT_GREY, 2, 8);
    for (uint16_t i = 0; i < 8; i++) {
        ldGraphSetValue(ptGraph, 0, i, c_graph_values_a[i]);
        ldGraphSetValue(ptGraph, 1, i, c_graph_values_b[i]);
    }
    ldGraphSetSelectable(ptGraph, true);
    ldGraphSetCorner(ptGraph, true);

    ptTable = ldTableInit(ID_PAGE4_TABLE, ID_PAGE4_BG, 12, 146, 202, 68, 4, 3, 1);
    ldTableSetExcelType(ptTable, FONT_ARIAL_12);
    ldTableSetKeyboard(ptTable, UI_WIDGET_KEYBOARD_ID);
    ldTableSetItemText(ptTable, 1, 1, (uint8_t *)"id");
    ldTableSetItemText(ptTable, 1, 2, (uint8_t *)"name");
    ldTableSetItemText(ptTable, 1, 3, (uint8_t *)"size");
    ldTableSetItemText(ptTable, 2, 1, (uint8_t *)"1");
    ldTableSetItemText(ptTable, 2, 2, (uint8_t *)"button");
    ldTableSetItemText(ptTable, 2, 3, (uint8_t *)"30x20");
    ldTableSetItemText(ptTable, 3, 1, (uint8_t *)"2");
    ldTableSetItemText(ptTable, 3, 2, (uint8_t *)"image");
    ldTableSetItemText(ptTable, 3, 3, (uint8_t *)"88x118");
    ldTableSetItemText(ptTable, 4, 1, (uint8_t *)"3");
    ldTableSetItemText(ptTable, 4, 2, (uint8_t *)"gauge");
    ldTableSetItemText(ptTable, 4, 3, (uint8_t *)"110x92");
    ldBaseSetSelectable(ptTable, true);

    ptList = ldListInit(ID_PAGE4_LIST, ID_PAGE4_BG, 12, 220, 202, 40);
    ldListSetItemHeight(ptList, 20);
    ldListSetText(ptList, g_widget_scroll_items, 5, FONT_ARIAL_12);
    ldListSetAlign(ptList, ARM_2D_ALIGN_LEFT);
    ldBaseSetSelectable(ptList, true);
    ldBaseSetCorner(ptList, true);

    obj = ldButtonInit(ID_PAGE4_LIST_BUTTON, ID_PAGE4_LIST, 8, 1, 26, 18);
    ldButtonSetFont(obj, FONT_ARIAL_12);
    ldButtonSetText(obj, (uint8_t *)"GO");
    ldListSetItemWidget(ptList, 1, obj);

    obj = ldCalendarInit(ID_PAGE4_CALENDAR, ID_PAGE4_BG, 222, 136, 246, 122, FONT_ARIAL_12, 2026, 1, 1);
    ldCalendarSetDayNames(obj, g_widget_day_names);
    ldCalendarSetHeader(obj, true);
    ldCalendarSetHeaderFormat(obj, g_widget_header_format);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);

    obj = ldMessageBoxInit(ID_PAGE4_MESSAGE_BOX, ID_PAGE4_BG, 176, 80, FONT_ARIAL_12);
    ldMessageBoxSetTitle(obj, g_widget_message_title);
    ldMessageBoxSetMsg(obj, g_widget_message_text);
    ldMessageBoxSetBtn(obj, g_widget_message_buttons, 3);
    ldMessageBoxSetBackgroundColor(obj, __RGB(255, 255, 255));
    ldMessageBoxSetCorner(obj, true);
    ldMessageBoxSetHidden(obj, true);

    ldKeyboardInit(UI_WIDGET_KEYBOARD_ID, ID_PAGE4_BG, FONT_ARIAL_12);
}

static void uiWidgetPage4Loop(ld_scene_t *ptScene)
{
    ldGauge_t *ptGauge = ldBaseGetWidgetById(ID_PAGE4_GAUGE);
    ldArc_t *ptArc = ldBaseGetWidgetById(ID_PAGE4_ARC);

    if (ldTimeOut(UI_WIDGET_DYNAMIC_UPDATE_MS, true, &s_page4_dynamic_timer)) {
        if (NULL != ptArc) {
            ldArcSetRotationAngle(ptArc, s_page4_angle);
        }

        if (NULL != ptGauge) {
            ldGaugeSetAngle(ptGauge, s_page4_angle);
        }

        s_page4_angle += 1.0f;
        if (s_page4_angle >= 360.0f) {
            s_page4_angle = 0.0f;
        }
    }

    if (ldTimeOut(UI_WIDGET_AUTO_SWITCH_MS, false, &s_page4_switch_timer)) {
        ldGuiJumpPage(uiWidgetPage1Func, ARM_2D_SCENE_SWITCH_MODE_SLIDE_DOWN, UI_WIDGET_AUTO_SWITCH_MS);
    }

    uiWidgetHandleFocusNavigation(ptScene);
}

static void uiWidgetPage4Quit(ld_scene_t *ptScene)
{
    (void)ptScene;
    s_page4_switch_timer = 0;
    s_page4_dynamic_timer = 0;
}
