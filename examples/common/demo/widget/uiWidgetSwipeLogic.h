#ifndef __UI_WIDGET_SWIPE_LOGIC_H__
#define __UI_WIDGET_SWIPE_LOGIC_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int16_t startX;
    int16_t startY;
    int16_t lastX;
    int16_t lastY;
    bool active;
} uiWidgetSwipeGestureState_t;

typedef enum {
    UI_WIDGET_SWIPE_NONE = 0,
    UI_WIDGET_SWIPE_PREV = -1,
    UI_WIDGET_SWIPE_NEXT = 1,
} uiWidgetSwipeDecision_t;

void uiWidgetSwipeGestureBegin(uiWidgetSwipeGestureState_t *ptState,
                               int16_t x,
                               int16_t y);
void uiWidgetSwipeGestureTrack(uiWidgetSwipeGestureState_t *ptState,
                               int16_t x,
                               int16_t y);
uiWidgetSwipeDecision_t uiWidgetSwipeGestureEnd(uiWidgetSwipeGestureState_t *ptState,
                                                int16_t x,
                                                int16_t y,
                                                int16_t threshold);
uint8_t uiWidgetSwipeWrapIndex(uint8_t currentIndex,
                               int8_t delta,
                               uint8_t pageCount);
bool uiWidgetSwipeIsHorizontalIntent(int16_t dx,
                                     int16_t dy,
                                     int16_t threshold);

#endif
