#ifndef TINYUI_RUNTIME_H
#define TINYUI_RUNTIME_H

#include "widget.h"

/**
 * @brief Initialize TinyUI runtime
 *
 * @return 0 on success, -1 on failure
 */
int tinyui_init(void);

/**
 * @brief Deinitialize TinyUI runtime
 */
void tinyui_deinit(void);

/**
 * @brief Create a new screen (window)
 *
 * @return Pointer to the screen object on success, NULL on failure
 */
tinyui_obj_t *tinyui_screen_create(void);

/**
 * @brief Load and activate a screen
 *
 * @param[in] screen Screen object to load
 * @return 0 on success, -1 on failure
 */
int tinyui_screen_load(tinyui_obj_t *screen);

/**
 * @brief Handle timer events (call in main loop)
 *
 * Canonical LVGL-like handler for the TinyUI event loop.
 *
 * @return <0 on error, 0 while running, >0 when finished.
 *         The caller decides whether to deinit/exit.
 */
int tinyui_timer_handler(void);

/*
 * Deprecated backward-compatibility aliases.
 * These will be removed in a future version.
 * New code should use the tinyui_* API directly.
 */
struct picoui_window;

static inline int picoui_init(void)
{
    return tinyui_init();
}

static inline void picoui_deinit(void)
{
    tinyui_deinit();
}

static inline struct picoui_window *picoui_screen_create(void)
{
    return (struct picoui_window *)tinyui_screen_create();
}

static inline int picoui_screen_load(struct picoui_window *screen)
{
    return tinyui_screen_load((tinyui_obj_t *)screen);
}

static inline int picoui_timer_handler(void)
{
    return tinyui_timer_handler();
}

#endif
