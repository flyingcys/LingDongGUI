/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * M1 capacity placeholders for 32-bit static RAM budget probes.
 * M2 wires real fixed-pool semantics on top of these layouts; do not treat this
 * header as evidence that dispatch/timer pools are behaviorally complete.
 */

#ifndef TINYUI_INTERNAL_RUNTIME_POOLS_H
#define TINYUI_INTERNAL_RUNTIME_POOLS_H

#include "core/event.h"
#include "core/result.h"
#include "core/timer.h"

#include <stdbool.h>
#include <stdint.h>

#ifndef TINYUI_RUNTIME_POOL_CAPACITY
#define TINYUI_RUNTIME_POOL_CAPACITY 16
#endif

/* Fixed timer slot: public tinyui_timer_t is this storage (no freelist heap). */
struct tinyui_timer {
    uint32_t interval_ms;
    uint32_t deadline_ms;
    tinyui_timer_cb_t cb;
    void *user_data;
    uint16_t generation;
    uint16_t born_epoch;
    uint8_t allocated;
    uint8_t running;
    uint8_t repeat;
    uint8_t deleting;
};

struct tinyui_timer_pool {
    struct tinyui_timer slots[TINYUI_TIMER_CAPACITY];
    uint16_t active_count;
    uint16_t next_generation;
};

struct tinyui_event_callback_slot {
    tinyui_obj_t *object;
    uint32_t event_mask;
    tinyui_event_cb_t cb;
    void *user_data;
    uint16_t generation;
    uint16_t registration_order;
    uint16_t born_epoch;
    uint8_t allocated;
    uint8_t _pad;
};

struct tinyui_event_callback_pool {
    struct tinyui_event_callback_slot slots[TINYUI_EVENT_CB_CAPACITY];
    uint16_t active_count;
    uint16_t next_generation;
    uint16_t next_registration_order;
    uint16_t dispatch_epoch;
};

/* Fixed diagnostic / runtime bookkeeping that counts toward the 1024 B budget. */
struct tinyui_runtime_bookkeeping {
    tinyui_result_t last_result;
    char last_error_message[64];
    uint32_t process_epoch;
    uint16_t focus_serial;
    uint16_t reserved0;
    uintptr_t active_screen;
    uintptr_t focus_object;
    uintptr_t editing_object;
    uint32_t flags;
    uint32_t reserved1[4];
};

#endif /* TINYUI_INTERNAL_RUNTIME_POOLS_H */
