#include "uiWidgetPage1.h"

#include "uiWidgetPage2.h"

enum {
    ID_PAGE1_BG = 0,
    ID_PAGE1_HEADER = 1,
    ID_PAGE1_TITLE = 2,
    ID_PAGE1_HINT = 3,
    ID_PAGE1_IMAGE = 10,
    ID_PAGE1_BUTTON = 11,
    ID_PAGE1_LABEL = 12,
    ID_PAGE1_CHECK_A = 13,
    ID_PAGE1_CHECK_B = 14,
    ID_PAGE1_CHECK_C = 15,
    ID_PAGE1_WINDOW = 16,
    ID_PAGE1_WINDOW_LABEL = 17,
};

static ldTimer_t s_page1_switch_timer;
static bool s_page1_image_dimmed;

static bool uiWidgetPage1ButtonRelease(ld_scene_t *ptScene, ldMsg_t msg);
static void uiWidgetPage1Init(ld_scene_t *ptScene);
static void uiWidgetPage1Loop(ld_scene_t *ptScene);
static void uiWidgetPage1Quit(ld_scene_t *ptScene);

const ldPageFuncGroup_t uiWidgetPage1Func = {
    .init = uiWidgetPage1Init,
    .loop = uiWidgetPage1Loop,
    .quit = uiWidgetPage1Quit,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "uiWidgetPage1",
#endif
};

static bool uiWidgetPage1ButtonRelease(ld_scene_t *ptScene, ldMsg_t msg)
{
    (void)msg;

    ldImage_t *ptImage = ldBaseGetWidget(ptScene->ptNodeRoot, ID_PAGE1_IMAGE);
    if (NULL == ptImage) {
        return false;
    }

    s_page1_image_dimmed = !s_page1_image_dimmed;
    ldBaseSetOpacity((ldBase_t *)ptImage, s_page1_image_dimmed ? 128 : 255);
    return false;
}

static void uiWidgetPage1Init(ld_scene_t *ptScene)
{
    void *obj;

    s_page1_switch_timer = 0;
    s_page1_image_dimmed = false;

    ldBaseFocusNavigateInit();

    obj = ldWindowInit(ID_PAGE1_BG, 0, 0, 0, LD_CFG_SCREEN_WIDTH, LD_CFG_SCREEN_HEIGHT);
    if (NULL == obj) {
        uiWidgetInitFailed("uiWidgetPage1", ID_PAGE1_BG);
        return;
    }
    ldWindowSetColor(obj, __RGB(244, 246, 248));

    obj = ldWindowInit(ID_PAGE1_HEADER, ID_PAGE1_BG, 0, 0, LD_CFG_SCREEN_WIDTH, UI_WIDGET_HEADER_HEIGHT);
    if (NULL == obj) {
        uiWidgetInitFailed("uiWidgetPage1", ID_PAGE1_HEADER);
        return;
    }
    ldWindowSetColor(obj, __RGB(45, 102, 132));

    obj = ldLabelInit(ID_PAGE1_TITLE, ID_PAGE1_BG, UI_WIDGET_MARGIN, 5, 200, 20, FONT_ARIAL_16_A8);
    if (NULL == obj) {
        uiWidgetInitFailed("uiWidgetPage1", ID_PAGE1_TITLE);
        return;
    }
    ldLabelSetText(obj, (uint8_t *)"Page 1  Basics");
    ldLabelSetTextColor(obj, GLCD_COLOR_WHITE);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);

    obj = ldLabelInit(ID_PAGE1_HINT, ID_PAGE1_BG, LD_CFG_SCREEN_WIDTH - 64, 7, 52, 18, FONT_ARIAL_12);
    if (NULL == obj) {
        uiWidgetInitFailed("uiWidgetPage1", ID_PAGE1_HINT);
        return;
    }
    ldLabelSetText(obj, (uint8_t *)"1 / 4");
    ldLabelSetTextColor(obj, GLCD_COLOR_WHITE);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_RIGHT);

    obj = ldImageInit(ID_PAGE1_IMAGE, ID_PAGE1_BG, 16, 48, 88, 118, NULL, NULL);
    ldImageSetImage(obj, IMAGE_LETTER_PAPER_BMP, NULL);
    ldBaseSetCorner(obj, true);
    ldBaseSetSelectable(obj, true);

    obj = ldButtonInit(ID_PAGE1_BUTTON, ID_PAGE1_BG, 124, 54, 96, 52);
    ldButtonSetFont(obj, FONT_ARIAL_16_A8);
    ldButtonSetText(obj, (uint8_t *)"123");
    ldButtonSetTextColor(obj, GLCD_COLOR_WHITE);
    ldButtonSetImage(obj,
                     IMAGE_KEYRELEASE_PNG,
                     IMAGE_KEYRELEASE_PNG_Mask,
                     IMAGE_KEYPRESS_PNG,
                     IMAGE_KEYPRESS_PNG_Mask);
    ldBaseSetSelectable(obj, true);

    obj = ldLabelInit(ID_PAGE1_LABEL, ID_PAGE1_BG, 124, 118, 150, 50, FONT_ARIAL_12);
    ldLabelSetText(obj, (uint8_t *)"button toggles\nimage opacity");
    ldLabelSetBackgroundColor(obj, GLCD_COLOR_LIGHT_GREY);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_BOTTOM_LEFT);
    ldBaseSetSelectable(obj, true);
    ldBaseSetCorner(obj, true);

    obj = ldCheckBoxInit(ID_PAGE1_CHECK_A, ID_PAGE1_BG, 302, 58, 132, 22);
    ldCheckBoxSetRadioButtonGroup(obj, 0);
    ldCheckBoxSetText(obj, FONT_ARIAL_12, (uint8_t *)"Option A");
    ldCheckBoxSetSelectable(obj, true);

    obj = ldCheckBoxInit(ID_PAGE1_CHECK_B, ID_PAGE1_BG, 302, 92, 132, 22);
    ldCheckBoxSetRadioButtonGroup(obj, 0);
    ldCheckBoxSetText(obj, FONT_ARIAL_12, (uint8_t *)"Option B");
    ldCheckBoxSetSelectable(obj, true);

    obj = ldCheckBoxInit(ID_PAGE1_CHECK_C, ID_PAGE1_BG, 302, 126, 132, 22);
    ldCheckBoxSetRadioButtonGroup(obj, 0);
    ldCheckBoxSetText(obj, FONT_ARIAL_12, (uint8_t *)"Corner");
    ldBaseSetCorner(obj, true);
    ldCheckBoxSetSelectable(obj, true);

    obj = ldWindowInit(ID_PAGE1_WINDOW, ID_PAGE1_BG, 292, 164, 160, 78);
    ldWindowSetColor(obj, __RGB(148, 214, 178));
    ldBaseSetCorner(obj, true);
    ldBaseSetSelectable(obj, true);

    obj = ldLabelInit(ID_PAGE1_WINDOW_LABEL, ID_PAGE1_WINDOW, 14, 18, 120, 24, FONT_ARIAL_12);
    ldLabelSetText(obj, (uint8_t *)"Nested window");
    ldLabelSetTextColor(obj, GLCD_COLOR_BLACK);
    ldLabelSetAlign(obj, ARM_2D_ALIGN_LEFT);

    connect(ID_PAGE1_BUTTON, SIGNAL_RELEASE, uiWidgetPage1ButtonRelease);
}

static void uiWidgetPage1Loop(ld_scene_t *ptScene)
{
    if (ldTimeOut(UI_WIDGET_PAGE_DWELL_MS, false, &s_page1_switch_timer)) {
        ldGuiJumpPage(uiWidgetPage2Func, ARM_2D_SCENE_SWITCH_MODE_SLIDE_LEFT, UI_WIDGET_PAGE_SWITCH_ANIM_MS);
    }

    uiWidgetHandleFocusNavigation(ptScene);
}

static void uiWidgetPage1Quit(ld_scene_t *ptScene)
{
    (void)ptScene;
    s_page1_switch_timer = 0;
}
