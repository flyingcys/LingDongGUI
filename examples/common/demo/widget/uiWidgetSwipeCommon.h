#ifndef __UI_WIDGET_SWIPE_COMMON_H__
#define __UI_WIDGET_SWIPE_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "uiWidgetCommon.h"
#include "uiWidgetSwipeLogic.h"
#include "uiWidgetSwipePages.h"

#define UI_WIDGET_SWIPE_PAGE_COUNT      23
#define UI_WIDGET_SWIPE_GESTURE_PX      96
#define UI_WIDGET_SWIPE_HEADER_BG       __RGB(36, 78, 112)
#define UI_WIDGET_SWIPE_HELP_TEXT       ((const uint8_t *)"Swipe empty space to browse")

typedef struct {
    uint16_t bgId;
    uint16_t headerId;
    uint16_t titleId;
    uint16_t hintId;
    uint16_t helpId;
} uiWidgetSwipeChromeIds_t;

bool uiWidgetSwipeInitChrome(ld_scene_t *ptScene,
                             const uiWidgetSwipeChromeIds_t *ptIds,
                             uint8_t pageIndex,
                             ldColor bgColor,
                             const char *pageName);

bool uiWidgetSwipeHandleGesture(ld_scene_t *ptScene,
                                uiWidgetSwipeGestureState_t *ptGesture,
                                uint8_t pageIndex,
                                ldMsg_t msg);

void uiWidgetSwipeJumpRelative(uint8_t pageIndex, int8_t delta);
void uiWidgetSwipePageLoop(ld_scene_t *ptScene);

#ifdef __cplusplus
}
#endif

#endif
