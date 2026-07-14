#ifndef TINYUI_TIMER_H
#define TINYUI_TIMER_H

#include "core/result.h"

#include <stdbool.h>
#include <stdint.h>

#ifndef TINYUI_TIMER_CAPACITY
#define TINYUI_TIMER_CAPACITY 16
#endif

typedef struct tinyui_timer tinyui_timer_t;
typedef void (*tinyui_timer_cb_t)(tinyui_timer_t *timer, void *user_data);

tinyui_timer_t *tinyui_timer_create(uint32_t interval_ms,
                                    bool repeat,
                                    tinyui_timer_cb_t cb,
                                    void *user_data);
tinyui_result_t tinyui_timer_start(tinyui_timer_t *timer);
tinyui_result_t tinyui_timer_stop(tinyui_timer_t *timer);
tinyui_result_t tinyui_timer_set_interval(tinyui_timer_t *timer,
                                          uint32_t interval_ms);
void tinyui_timer_delete(tinyui_timer_t *timer);

#endif
