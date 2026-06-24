#ifndef TINYUI_RUNTIME_H
#define TINYUI_RUNTIME_H

#include "core/widget.h"

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

#endif
