#include "uiLayout.h"

#include "ldGui.h"
#include "uiFonts.h"

#define UI_GRID_PANEL_WIDTH        124
#define UI_GRID_PANEL_HEIGHT_SM    54
#define UI_GRID_PANEL_HEIGHT_MD    64
#define UI_GRID_PANEL_HEIGHT_LG    74

enum {
    ID_LAYOUT_BG = 0,
    ID_LAYOUT_LEGACY = 10,
    ID_LAYOUT_FLEX_ROW = 20,
    ID_LAYOUT_FLEX_COLUMN = 30,
    ID_LAYOUT_LEGACY_VERTICAL = 40,

    ID_LAYOUT_PAGE_TITLE = 100,
    ID_LAYOUT_PAGE_HINT = 101,

    ID_LAYOUT_FLEX_ROW_TITLE = 110,
    ID_LAYOUT_FLEX_ROW_HINT = 111,
    ID_LAYOUT_FLEX_COLUMN_TITLE = 112,
    ID_LAYOUT_FLEX_COLUMN_HINT = 113,
    ID_LAYOUT_LEGACY_TITLE = 114,
    ID_LAYOUT_LEGACY_HINT = 115,
    ID_LAYOUT_LEGACY_VERTICAL_TITLE = 116,
    ID_LAYOUT_LEGACY_VERTICAL_HINT = 117,

    ID_LAYOUT_LEGACY_ROW_A = 200,
    ID_LAYOUT_LEGACY_ROW_A_LABEL = 201,
    ID_LAYOUT_LEGACY_ROW_B = 202,
    ID_LAYOUT_LEGACY_ROW_B_LABEL = 203,
    ID_LAYOUT_LEGACY_ROW_C = 204,
    ID_LAYOUT_LEGACY_ROW_C_LABEL = 205,
    ID_LAYOUT_LEGACY_ROW_C_BADGE = 206,
    ID_LAYOUT_LEGACY_ROW_C_BADGE_LABEL = 207,

    ID_LAYOUT_LEGACY_COLUMN_A = 220,
    ID_LAYOUT_LEGACY_COLUMN_A_LABEL = 221,
    ID_LAYOUT_LEGACY_COLUMN_B = 222,
    ID_LAYOUT_LEGACY_COLUMN_B_LABEL = 223,
    ID_LAYOUT_LEGACY_COLUMN_C = 224,
    ID_LAYOUT_LEGACY_COLUMN_C_LABEL = 225,

    ID_LAYOUT_FLEX_ROW_A = 240,
    ID_LAYOUT_FLEX_ROW_A_LABEL = 241,
    ID_LAYOUT_FLEX_ROW_B = 242,
    ID_LAYOUT_FLEX_ROW_B_LABEL = 243,
    ID_LAYOUT_FLEX_ROW_C = 244,
    ID_LAYOUT_FLEX_ROW_C_LABEL = 245,
    ID_LAYOUT_FLEX_ROW_D = 246,
    ID_LAYOUT_FLEX_ROW_D_LABEL = 247,
    ID_LAYOUT_FLEX_ROW_E = 248,
    ID_LAYOUT_FLEX_ROW_E_LABEL = 249,

    ID_LAYOUT_FLEX_COLUMN_A = 260,
    ID_LAYOUT_FLEX_COLUMN_A_LABEL = 261,
    ID_LAYOUT_FLEX_COLUMN_B = 262,
    ID_LAYOUT_FLEX_COLUMN_B_LABEL = 263,
    ID_LAYOUT_FLEX_COLUMN_C = 264,
    ID_LAYOUT_FLEX_COLUMN_C_LABEL = 265,
    ID_LAYOUT_FLEX_COLUMN_D = 266,
    ID_LAYOUT_FLEX_COLUMN_D_LABEL = 267,

    ID_GRID_BG = 1000,
    ID_GRID_PAGE_TITLE = 1001,
    ID_GRID_PAGE_HINT = 1002,
    ID_GRID_CANVAS = 1010,
    ID_GRID_GUIDE_TEXT = 1011,

    ID_GRID_CELL_A = 1100,
    ID_GRID_CELL_A_TITLE = 1101,
    ID_GRID_CELL_A_HINT = 1102,
    ID_GRID_CELL_B = 1110,
    ID_GRID_CELL_B_TITLE = 1111,
    ID_GRID_CELL_B_HINT = 1112,
    ID_GRID_CELL_C = 1120,
    ID_GRID_CELL_C_TITLE = 1121,
    ID_GRID_CELL_C_HINT = 1122,
    ID_GRID_CELL_D = 1130,
    ID_GRID_CELL_D_TITLE = 1131,
    ID_GRID_CELL_D_HINT = 1132,
    ID_GRID_CELL_E = 1140,
    ID_GRID_CELL_E_TITLE = 1141,
    ID_GRID_CELL_E_HINT = 1142,
    ID_GRID_CELL_F = 1150,
    ID_GRID_CELL_F_TITLE = 1151,
    ID_GRID_CELL_F_HINT = 1152,
};

static ldTimer_t s_layout_resize_timer;
static bool s_layout_compact;

static void uiLayoutCreateInfoLabel(ld_scene_t *ptScene,
                                    uint16_t nameId,
                                    int16_t x,
                                    int16_t y,
                                    int16_t width,
                                    ldColor color,
                                    const char *text);
static void uiLayoutCreateCard(ld_scene_t *ptScene,
                               uint16_t cardId,
                               uint16_t labelId,
                               uint16_t parentNameId,
                               int16_t width,
                               int16_t height,
                               ldColor color,
                               const char *text);
static void uiGridCreatePanel(ld_scene_t *ptScene,
                              uint16_t panelId,
                              uint16_t titleId,
                              uint16_t hintId,
                              int16_t width,
                              int16_t height,
                              ldColor color,
                              const char *title,
                              const char *hint);

const ldPageFuncGroup_t uiLayoutFunc = {
    .init = uiLayoutInit,
    .loop = uiLayoutLoop,
    .quit = uiLayoutQuit,
};

const ldPageFuncGroup_t uiGridFunc = {
    .init = uiGridInit,
    .loop = uiGridLoop,
    .quit = uiGridQuit,
};

static void uiLayoutCreateInfoLabel(ld_scene_t *ptScene,
                                    uint16_t nameId,
                                    int16_t x,
                                    int16_t y,
                                    int16_t width,
                                    ldColor color,
                                    const char *text)
{
    void *obj;

    (void)ptScene;
    obj = ldLabelInit(nameId, ID_LAYOUT_BG, x, y, width, 12, FONT_ARIAL_12);
    ldLabelSetText(obj, (uint8_t *)text);
    ldLabelSetTextColor(obj, color);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);
}

static void uiLayoutCreateCard(ld_scene_t *ptScene,
                               uint16_t cardId,
                               uint16_t labelId,
                               uint16_t parentNameId,
                               int16_t width,
                               int16_t height,
                               ldColor color,
                               const char *text)
{
    void *obj;

    (void)ptScene;
    obj = ldWindowInit(cardId, parentNameId, 0, 0, width, height);
    ldWindowSetColor(obj, color);

    obj = ldLabelInit(labelId, cardId, 0, 0, width, height, FONT_ARIAL_12);
    ldLabelSetText(obj, (uint8_t *)text);
    ldLabelSetTextColor(obj, GLCD_COLOR_WHITE);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_CENTRE);
    ldLabelSetTransparent(obj, true);
}

static void uiGridCreatePanel(ld_scene_t *ptScene,
                              uint16_t panelId,
                              uint16_t titleId,
                              uint16_t hintId,
                              int16_t width,
                              int16_t height,
                              ldColor color,
                              const char *title,
                              const char *hint)
{
    void *obj;

    (void)ptScene;
    obj = ldWindowInit(panelId, ID_GRID_CANVAS, 0, 0, width, height);
    ldWindowSetColor(obj, color);

    obj = ldLabelInit(titleId, panelId, 10, 8, width - 20, 16, FONT_ARIAL_16_A8);
    ldLabelSetText(obj, (uint8_t *)title);
    ldLabelSetTextColor(obj, GLCD_COLOR_WHITE);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);
    ldLabelSetTransparent(obj, true);

    obj = ldLabelInit(hintId, panelId, 10, 30, width - 20, height - 38, FONT_ARIAL_12);
    ldLabelSetText(obj, (uint8_t *)hint);
    ldLabelSetTextColor(obj, __RGB(242, 244, 247));
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);
    ldLabelSetTransparent(obj, true);
}

void uiLayoutInit(ld_scene_t *ptScene)
{
    void *obj;

    s_layout_resize_timer = 0;
    s_layout_compact = false;

    obj = ldWindowInit(ID_LAYOUT_BG, ID_LAYOUT_BG, 0, 0, LD_CFG_SCREEN_WIDTH, LD_CFG_SCREEN_HEIGHT);
    ldWindowSetColor(obj, __RGB(245, 246, 248));

    obj = ldLabelInit(ID_LAYOUT_PAGE_TITLE, ID_LAYOUT_BG, 12, 8, 220, 18, FONT_ARIAL_16_A8);
    ldLabelSetText(obj, (uint8_t *)"Layout Demo");
    ldLabelSetTextColor(obj, GLCD_COLOR_BLACK);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);

    obj = ldLabelInit(ID_LAYOUT_PAGE_HINT, ID_LAYOUT_BG, 12, 24, 456, 12, FONT_ARIAL_12);
    ldLabelSetText(obj, (uint8_t *)"Top row now shows row-wrap/new-track and column-grow/ignore-layout; left panel narrows every 1200ms.");
    ldLabelSetTextColor(obj, __RGB(82, 86, 92));
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);

    obj = ldWindowInit(ID_LAYOUT_FLEX_ROW, ID_LAYOUT_BG, 12, 40, 220, 80);
    ldWindowSetColor(obj, __RGB(240, 240, 240));
    ldWindowSetFlexFlow(obj, ldFlexFlowRowWrap);
    ldWindowSetPadding(obj, (ldPadding_t){ .left = 8, .top = 8, .right = 8, .bottom = 8 });
    ldWindowSetFlexGap(obj, 6, 4);
    ldWindowSetFlexAlign(obj, ldFlexMainAlignStart, ldFlexCrossAlignCenter);
    ldWindowSetFlexTrackAlign(obj, ldFlexTrackAlignCenter);

    uiLayoutCreateCard(ptScene, ID_LAYOUT_FLEX_ROW_A, ID_LAYOUT_FLEX_ROW_A_LABEL, ID_LAYOUT_FLEX_ROW, 64, 14, __RGB(224, 122, 95), "A");
    uiLayoutCreateCard(ptScene, ID_LAYOUT_FLEX_ROW_B, ID_LAYOUT_FLEX_ROW_B_LABEL, ID_LAYOUT_FLEX_ROW, 58, 14, __RGB(69, 123, 157), "B");
    uiLayoutCreateCard(ptScene, ID_LAYOUT_FLEX_ROW_C, ID_LAYOUT_FLEX_ROW_C_LABEL, ID_LAYOUT_FLEX_ROW, 52, 14, __RGB(129, 178, 154), "C");
    uiLayoutCreateCard(ptScene, ID_LAYOUT_FLEX_ROW_D, ID_LAYOUT_FLEX_ROW_D_LABEL, ID_LAYOUT_FLEX_ROW, 60, 14, __RGB(38, 70, 83), "D*");
    uiLayoutCreateCard(ptScene, ID_LAYOUT_FLEX_ROW_E, ID_LAYOUT_FLEX_ROW_E_LABEL, ID_LAYOUT_FLEX_ROW, 48, 14, __RGB(244, 162, 97), "E");
    ldBaseSetFlexNewTrack((ldBase_t *)ldBaseGetWidget(ptScene->ptNodeRoot, ID_LAYOUT_FLEX_ROW_D), true);

    obj = ldWindowInit(ID_LAYOUT_FLEX_COLUMN, ID_LAYOUT_BG, 248, 40, 220, 80);
    ldWindowSetColor(obj, __RGB(236, 240, 232));
    ldWindowSetFlexFlow(obj, ldFlexFlowColumn);
    ldWindowSetPadding(obj, (ldPadding_t){ .left = 10, .top = 8, .right = 10, .bottom = 8 });
    ldWindowSetGap(obj, 4);
    ldWindowSetFlexAlign(obj, ldFlexMainAlignStart, ldFlexCrossAlignCenter);

    uiLayoutCreateCard(ptScene, ID_LAYOUT_FLEX_COLUMN_A, ID_LAYOUT_FLEX_COLUMN_A_LABEL, ID_LAYOUT_FLEX_COLUMN, 68, 10, __RGB(231, 111, 81), "top");
    uiLayoutCreateCard(ptScene, ID_LAYOUT_FLEX_COLUMN_B, ID_LAYOUT_FLEX_COLUMN_B_LABEL, ID_LAYOUT_FLEX_COLUMN, 72, 10, __RGB(42, 157, 143), "1x");
    uiLayoutCreateCard(ptScene, ID_LAYOUT_FLEX_COLUMN_C, ID_LAYOUT_FLEX_COLUMN_C_LABEL, ID_LAYOUT_FLEX_COLUMN, 82, 10, __RGB(38, 70, 83), "2x");
    uiLayoutCreateCard(ptScene, ID_LAYOUT_FLEX_COLUMN_D, ID_LAYOUT_FLEX_COLUMN_D_LABEL, ID_LAYOUT_FLEX_COLUMN, 54, 16, __RGB(87, 117, 144), "free");
    ldBaseSetFlexGrow((ldBase_t *)ldBaseGetWidget(ptScene->ptNodeRoot, ID_LAYOUT_FLEX_COLUMN_B), 1);
    ldBaseSetFlexGrow((ldBase_t *)ldBaseGetWidget(ptScene->ptNodeRoot, ID_LAYOUT_FLEX_COLUMN_C), 2);
    ldBaseSetIgnoreLayout((ldBase_t *)ldBaseGetWidget(ptScene->ptNodeRoot, ID_LAYOUT_FLEX_COLUMN_D), true);
    ldBaseMove((ldBase_t *)ldBaseGetWidget(ptScene->ptNodeRoot, ID_LAYOUT_FLEX_COLUMN_D), 142, 50);

    uiLayoutCreateInfoLabel(ptScene, ID_LAYOUT_FLEX_ROW_TITLE, 12, 122, 220, GLCD_COLOR_BLACK, "Flex wrap");
    uiLayoutCreateInfoLabel(ptScene, ID_LAYOUT_FLEX_ROW_HINT, 12, 136, 220, __RGB(82, 86, 92), "D* forces next track; compact width adds row 3");
    uiLayoutCreateInfoLabel(ptScene, ID_LAYOUT_FLEX_COLUMN_TITLE, 248, 122, 220, GLCD_COLOR_BLACK, "Flex column");
    uiLayoutCreateInfoLabel(ptScene, ID_LAYOUT_FLEX_COLUMN_HINT, 248, 136, 220, __RGB(82, 86, 92), "center cross-align; 1x/2x grows; 'free' ignores layout");

    obj = ldWindowInit(ID_LAYOUT_LEGACY, ID_LAYOUT_BG, 12, 150, 220, 88);
    ldWindowSetColor(obj, __RGB(232, 236, 242));
    ldWindowSetLayout(obj, layoutHorizontal);

    uiLayoutCreateCard(ptScene, ID_LAYOUT_LEGACY_ROW_A, ID_LAYOUT_LEGACY_ROW_A_LABEL, ID_LAYOUT_LEGACY, 48, 30, __RGB(244, 162, 97), "A");
    uiLayoutCreateCard(ptScene, ID_LAYOUT_LEGACY_ROW_B, ID_LAYOUT_LEGACY_ROW_B_LABEL, ID_LAYOUT_LEGACY, 48, 30, __RGB(69, 123, 157), "B");
    uiLayoutCreateCard(ptScene, ID_LAYOUT_LEGACY_ROW_C, ID_LAYOUT_LEGACY_ROW_C_LABEL, ID_LAYOUT_LEGACY, 48, 30, __RGB(42, 157, 143), "C");
    obj = ldWindowInit(ID_LAYOUT_LEGACY_ROW_C_BADGE, ID_LAYOUT_LEGACY_ROW_C, 28, 4, 14, 12);
    ldWindowSetColor(obj, __RGB(29, 53, 87));
    obj = ldLabelInit(ID_LAYOUT_LEGACY_ROW_C_BADGE_LABEL, ID_LAYOUT_LEGACY_ROW_C_BADGE, 0, 0, 14, 12, FONT_ARIAL_12);
    ldLabelSetText(obj, (uint8_t *)"n");
    ldLabelSetTextColor(obj, GLCD_COLOR_WHITE);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_CENTRE);
    ldLabelSetTransparent(obj, true);
    ldBaseSetHidden((ldBase_t *)ldBaseGetWidget(ptScene->ptNodeRoot, ID_LAYOUT_LEGACY_ROW_B), true);

    obj = ldWindowInit(ID_LAYOUT_LEGACY_VERTICAL, ID_LAYOUT_BG, 248, 150, 220, 88);
    ldWindowSetColor(obj, __RGB(235, 239, 232));
    ldWindowSetLayout(obj, layoutVertical);

    uiLayoutCreateCard(ptScene, ID_LAYOUT_LEGACY_COLUMN_A, ID_LAYOUT_LEGACY_COLUMN_A_LABEL, ID_LAYOUT_LEGACY_VERTICAL, 84, 18, __RGB(233, 196, 106), "top");
    uiLayoutCreateCard(ptScene, ID_LAYOUT_LEGACY_COLUMN_B, ID_LAYOUT_LEGACY_COLUMN_B_LABEL, ID_LAYOUT_LEGACY_VERTICAL, 108, 18, __RGB(69, 123, 157), "middle");
    uiLayoutCreateCard(ptScene, ID_LAYOUT_LEGACY_COLUMN_C, ID_LAYOUT_LEGACY_COLUMN_C_LABEL, ID_LAYOUT_LEGACY_VERTICAL, 90, 18, __RGB(42, 157, 143), "bottom");

    uiLayoutCreateInfoLabel(ptScene, ID_LAYOUT_LEGACY_TITLE, 12, 240, 220, GLCD_COLOR_BLACK, "Legacy row");
    uiLayoutCreateInfoLabel(ptScene, ID_LAYOUT_LEGACY_HINT, 12, 254, 220, __RGB(82, 86, 92), "B hidden: middle slot stays");
    uiLayoutCreateInfoLabel(ptScene, ID_LAYOUT_LEGACY_VERTICAL_TITLE, 248, 240, 220, GLCD_COLOR_BLACK, "Legacy column");
    uiLayoutCreateInfoLabel(ptScene, ID_LAYOUT_LEGACY_VERTICAL_HINT, 248, 254, 220, __RGB(82, 86, 92), "classic vertical slots");
}

void uiLayoutLoop(ld_scene_t *ptScene)
{
    ldWindow_t *ptFlexRow;

    if (!ldTimeOut(1200, true, &s_layout_resize_timer))
    {
        return;
    }

    ptFlexRow = ldBaseGetWidget(ptScene->ptNodeRoot, ID_LAYOUT_FLEX_ROW);
    if (ptFlexRow == NULL)
    {
        return;
    }

    s_layout_compact = !s_layout_compact;
    ldBaseSetWidth((ldBase_t *)ptFlexRow, s_layout_compact ? 170 : 220);
    ldWindowSetFlexGap(ptFlexRow, 6, 4);
}

void uiLayoutQuit(ld_scene_t *ptScene)
{
    (void)ptScene;
    s_layout_resize_timer = 0;
    s_layout_compact = false;
}

void uiGridInit(ld_scene_t *ptScene)
{
    void *obj;

    obj = ldWindowInit(ID_GRID_BG, ID_GRID_BG, 0, 0, LD_CFG_SCREEN_WIDTH, LD_CFG_SCREEN_HEIGHT);
    ldWindowSetColor(obj, __RGB(246, 247, 250));

    obj = ldLabelInit(ID_GRID_PAGE_TITLE, ID_GRID_BG, 12, 8, 220, 18, FONT_ARIAL_16_A8);
    ldLabelSetText(obj, (uint8_t *)"Grid Demo");
    ldLabelSetTextColor(obj, GLCD_COLOR_BLACK);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);

    obj = ldLabelInit(ID_GRID_PAGE_HINT, ID_GRID_BG, 12, 24, 456, 12, FONT_ARIAL_12);
    ldLabelSetText(obj, (uint8_t *)"Uses ldWindowSetGridColumns + ldWindowSetGridGap + ldWindowSetGridPadding on a 3-column dashboard.");
    ldLabelSetTextColor(obj, __RGB(82, 86, 92));
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);

    obj = ldWindowInit(ID_GRID_CANVAS, ID_GRID_BG, 12, 40, 456, 204);
    ldWindowSetColor(obj, __RGB(232, 236, 242));
    ldWindowSetGridColumns(obj, 3);
    ldWindowSetGridGap(obj, 12, 12);
    ldWindowSetGridPadding(obj, (ldPadding_t){ .left = 10, .top = 10, .right = 10, .bottom = 10 });

    uiGridCreatePanel(ptScene, ID_GRID_CELL_A, ID_GRID_CELL_A_TITLE, ID_GRID_CELL_A_HINT, UI_GRID_PANEL_WIDTH, UI_GRID_PANEL_HEIGHT_SM, __RGB(33, 64, 95), "Overview", "col 0\n54px high\nfirst visible child");
    uiGridCreatePanel(ptScene, ID_GRID_CELL_B, ID_GRID_CELL_B_TITLE, ID_GRID_CELL_B_HINT, UI_GRID_PANEL_WIDTH, UI_GRID_PANEL_HEIGHT_LG, __RGB(42, 157, 143), "Status", "col 1\n74px high\nsets row 0 height");
    uiGridCreatePanel(ptScene, ID_GRID_CELL_C, ID_GRID_CELL_C_TITLE, ID_GRID_CELL_C_HINT, UI_GRID_PANEL_WIDTH, UI_GRID_PANEL_HEIGHT_MD, __RGB(231, 111, 81), "Queue", "col 2\n64px high\nrow-first placement");
    uiGridCreatePanel(ptScene, ID_GRID_CELL_D, ID_GRID_CELL_D_TITLE, ID_GRID_CELL_D_HINT, UI_GRID_PANEL_WIDTH, UI_GRID_PANEL_HEIGHT_MD, __RGB(69, 123, 157), "Events", "next row, col 0\nstarts after row gap");
    uiGridCreatePanel(ptScene, ID_GRID_CELL_E, ID_GRID_CELL_E_TITLE, ID_GRID_CELL_E_HINT, UI_GRID_PANEL_WIDTH, UI_GRID_PANEL_HEIGHT_SM, __RGB(244, 162, 97), "Logs", "next row, col 1\npadding keeps left inset");
    uiGridCreatePanel(ptScene, ID_GRID_CELL_F, ID_GRID_CELL_F_TITLE, ID_GRID_CELL_F_HINT, UI_GRID_PANEL_WIDTH, UI_GRID_PANEL_HEIGHT_LG, __RGB(87, 117, 144), "Alerts", "next row, col 2\nrow 1 height follows tallest tile");

    obj = ldLabelInit(ID_GRID_GUIDE_TEXT, ID_GRID_BG, 12, 248, 456, 12, FONT_ARIAL_12);
    ldLabelSetText(obj, (uint8_t *)"Grid container: 3 columns, 12px row/column gap, 10px padding.");
    ldLabelSetTextColor(obj, __RGB(90, 94, 102));
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);
}

void uiGridLoop(ld_scene_t *ptScene)
{
    (void)ptScene;
}

void uiGridQuit(ld_scene_t *ptScene)
{
    (void)ptScene;
}
