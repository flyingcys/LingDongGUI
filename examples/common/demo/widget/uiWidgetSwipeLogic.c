#include <stddef.h>

#include "uiWidgetSwipeLogic.h"

#include <stddef.h>

static int16_t uiWidgetSwipeAbs16(int16_t value)
{
    return (value < 0) ? (int16_t)-value : value;
}

void uiWidgetSwipeGestureBegin(uiWidgetSwipeGestureState_t *ptState, int16_t x, int16_t y)
{
    if (ptState == NULL) {
        return;
    }

    ptState->startX = x;
    ptState->startY = y;
    ptState->lastX = x;
    ptState->lastY = y;
    ptState->active = true;
}

void uiWidgetSwipeGestureTrack(uiWidgetSwipeGestureState_t *ptState, int16_t x, int16_t y)
{
    if ((ptState == NULL) || !ptState->active) {
        return;
    }

    ptState->lastX = x;
    ptState->lastY = y;
}

bool uiWidgetSwipeIsHorizontalIntent(int16_t dx, int16_t dy, int16_t threshold)
{
    int16_t absDx = uiWidgetSwipeAbs16(dx);
    int16_t absDy = uiWidgetSwipeAbs16(dy);

    return (absDx >= threshold) && (absDx > absDy);
}

uiWidgetSwipeDecision_t uiWidgetSwipeGestureEnd(uiWidgetSwipeGestureState_t *ptState,
                                                int16_t x,
                                                int16_t y,
                                                int16_t threshold)
{
    int16_t dx;
    int16_t dy;

    if ((ptState == NULL) || !ptState->active) {
        return UI_WIDGET_SWIPE_NONE;
    }

    dx = (int16_t)(x - ptState->startX);
    dy = (int16_t)(y - ptState->startY);
    ptState->lastX = x;
    ptState->lastY = y;
    ptState->active = false;

    if (!uiWidgetSwipeIsHorizontalIntent(dx, dy, threshold)) {
        return UI_WIDGET_SWIPE_NONE;
    }

    return (dx < 0) ? UI_WIDGET_SWIPE_NEXT : UI_WIDGET_SWIPE_PREV;
}

uint8_t uiWidgetSwipeWrapIndex(uint8_t currentIndex, int8_t delta, uint8_t pageCount)
{
    int16_t nextIndex = (int16_t)currentIndex + (int16_t)delta;

    if (pageCount == 0U) {
        return 0U;
    }

    if (nextIndex < 0) {
        nextIndex = (int16_t)pageCount - 1;
    } else if (nextIndex >= pageCount) {
        nextIndex = 0;
    }

    return (uint8_t)nextIndex;
}
