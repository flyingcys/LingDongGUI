#include "uiLayout.h"

#include "ldGui.h"
#include "uiFonts.h"

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

    ID_LAYOUT_FLEX_COLUMN_A = 260,
    ID_LAYOUT_FLEX_COLUMN_A_LABEL = 261,
    ID_LAYOUT_FLEX_COLUMN_B = 262,
    ID_LAYOUT_FLEX_COLUMN_B_LABEL = 263,
    ID_LAYOUT_FLEX_COLUMN_C = 264,
    ID_LAYOUT_FLEX_COLUMN_C_LABEL = 265,
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

const ldPageFuncGroup_t uiLayoutFunc = {
    .init = uiLayoutInit,
    .loop = uiLayoutLoop,
    .quit = uiLayoutQuit,
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

    obj = ldWindowInit(cardId, parentNameId, 0, 0, width, height);
    ldWindowSetColor(obj, color);

    obj = ldLabelInit(labelId, cardId, 0, 0, width, height, FONT_ARIAL_12);
    ldLabelSetText(obj, (uint8_t *)text);
    ldLabelSetTextColor(obj, GLCD_COLOR_WHITE);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_CENTRE);
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
    ldLabelSetText(obj, (uint8_t *)"Task 5 compares legacy slots and flex reflow. Flex row resizes every 1200ms.");
    ldLabelSetTextColor(obj, __RGB(82, 86, 92));
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);

    obj = ldWindowInit(ID_LAYOUT_FLEX_ROW, ID_LAYOUT_BG, 12, 40, 220, 80);
    ldWindowSetColor(obj, __RGB(240, 240, 240));
    ldWindowSetFlexFlow(obj, ldFlexFlowRow);
    ldWindowSetPadding(obj, (ldPadding_t){ .left = 8, .top = 8, .right = 8, .bottom = 8 });
    ldWindowSetGap(obj, 10);
    ldWindowSetFlexAlign(obj, ldFlexMainAlignCenter, ldFlexCrossAlignCenter);

    uiLayoutCreateCard(ptScene, ID_LAYOUT_FLEX_ROW_A, ID_LAYOUT_FLEX_ROW_A_LABEL, ID_LAYOUT_FLEX_ROW, 52, 26, __RGB(224, 122, 95), "A");
    uiLayoutCreateCard(ptScene, ID_LAYOUT_FLEX_ROW_B, ID_LAYOUT_FLEX_ROW_B_LABEL, ID_LAYOUT_FLEX_ROW, 52, 26, __RGB(69, 123, 157), "B");
    uiLayoutCreateCard(ptScene, ID_LAYOUT_FLEX_ROW_C, ID_LAYOUT_FLEX_ROW_C_LABEL, ID_LAYOUT_FLEX_ROW, 52, 26, __RGB(129, 178, 154), "C");
    ldBaseSetHidden((ldBase_t *)ldBaseGetWidget(ptScene->ptNodeRoot, ID_LAYOUT_FLEX_ROW_B), true);

    obj = ldWindowInit(ID_LAYOUT_FLEX_COLUMN, ID_LAYOUT_BG, 248, 40, 220, 80);
    ldWindowSetColor(obj, __RGB(236, 240, 232));
    ldWindowSetFlexFlow(obj, ldFlexFlowColumn);
    ldWindowSetPadding(obj, (ldPadding_t){ .left = 10, .top = 8, .right = 10, .bottom = 8 });
    ldWindowSetGap(obj, 8);
    ldWindowSetFlexAlign(obj, ldFlexMainAlignEnd, ldFlexCrossAlignCenter);

    uiLayoutCreateCard(ptScene, ID_LAYOUT_FLEX_COLUMN_A, ID_LAYOUT_FLEX_COLUMN_A_LABEL, ID_LAYOUT_FLEX_COLUMN, 76, 16, __RGB(231, 111, 81), "wide");
    uiLayoutCreateCard(ptScene, ID_LAYOUT_FLEX_COLUMN_B, ID_LAYOUT_FLEX_COLUMN_B_LABEL, ID_LAYOUT_FLEX_COLUMN, 56, 16, __RGB(42, 157, 143), "mid");
    uiLayoutCreateCard(ptScene, ID_LAYOUT_FLEX_COLUMN_C, ID_LAYOUT_FLEX_COLUMN_C_LABEL, ID_LAYOUT_FLEX_COLUMN, 40, 16, __RGB(38, 70, 83), "thin");

    uiLayoutCreateInfoLabel(ptScene, ID_LAYOUT_FLEX_ROW_TITLE, 12, 122, 220, GLCD_COLOR_BLACK, "Flex row");
    uiLayoutCreateInfoLabel(ptScene, ID_LAYOUT_FLEX_ROW_HINT, 12, 136, 220, __RGB(82, 86, 92), "B hidden: A and C close up");
    uiLayoutCreateInfoLabel(ptScene, ID_LAYOUT_FLEX_COLUMN_TITLE, 248, 122, 220, GLCD_COLOR_BLACK, "Flex column");
    uiLayoutCreateInfoLabel(ptScene, ID_LAYOUT_FLEX_COLUMN_HINT, 248, 136, 220, __RGB(82, 86, 92), "column + end-center align");

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
    ldBaseSetWidth((ldBase_t *)ptFlexRow, s_layout_compact ? 180 : 220);
    ldWindowSetGap(ptFlexRow, 10);
}

void uiLayoutQuit(ld_scene_t *ptScene)
{
    (void)ptScene;
    s_layout_resize_timer = 0;
    s_layout_compact = false;
}
