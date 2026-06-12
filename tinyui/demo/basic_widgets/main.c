/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "button.h"
#include "checkbox.h"
#include "image.h"
#include "layout.h"
#include "runtime.h"
#include "slider.h"
#include "switch.h"
#include "text.h"
#include "widget.h"
#include "window.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

struct picoui_demo_benchmark_state {
    int enabled;
    int first_frame_logged;
    double screen_object_create_start_ms;
    double screen_object_create_end_ms;
    double capture_ready_start_ms;
    const char *capture_path;
};

static int picoui_demo_benchmark_enabled(void)
{
    const char *value = getenv("PICOUI_BENCHMARK_LOG");

    return value != 0 && value[0] != '\0' && value[0] != '0';
}

static double picoui_demo_benchmark_now_ms(void)
{
    struct timespec timestamp;

    if (clock_gettime(CLOCK_MONOTONIC, &timestamp) != 0) {
        return 0.0;
    }

    return (double)timestamp.tv_sec * 1000.0 + (double)timestamp.tv_nsec / 1000000.0;
}

static int picoui_demo_benchmark_capture_ready(const char *path)
{
    struct stat st;

    if (path == 0 || path[0] == '\0') {
        return 0;
    }

    if (stat(path, &st) != 0) {
        return 0;
    }

    return st.st_size > 32 ? 1 : 0;
}

static void picoui_demo_benchmark_log_screen_object_create(const struct picoui_demo_benchmark_state *benchmark)
{
    const double elapsed_ms =
        benchmark->screen_object_create_end_ms - benchmark->screen_object_create_start_ms;

    printf("PICOUI_BENCHMARK_SCREEN_OBJECT_CREATE_MS=%.3f\n", elapsed_ms);
    printf("PICOUI_BENCHMARK_SCREEN_CREATE_MS=%.3f\n", elapsed_ms);
    fflush(stdout);
}

static void picoui_demo_benchmark_log_capture_ready(const struct picoui_demo_benchmark_state *benchmark,
                                                    double capture_ready_end_ms)
{
    const double elapsed_ms = capture_ready_end_ms - benchmark->capture_ready_start_ms;

    printf("PICOUI_BENCHMARK_CAPTURE_READY_MS=%.3f\n", elapsed_ms);
    printf("PICOUI_BENCHMARK_FIRST_FRAME_MS=%.3f\n", elapsed_ms);
    fflush(stdout);
}

static void on_wifi_changed(struct picoui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    (void)value;
}

static void on_button_clicked(struct picoui_widget *widget, void *user_data)
{
    (void)widget;
    (void)user_data;
}

static void make_ui(struct picoui_window *win)
{
    struct picoui_switch *sw = picoui_switch_create(win, "wifi");
    struct picoui_checkbox *cb = picoui_checkbox_create(win, "agree");
    struct picoui_slider *slider = picoui_slider_create(win, "volume");
    struct picoui_button *button = picoui_button_create(win, "submit");
    struct picoui_text *text = picoui_text_create(win, "title");
    struct picoui_image *image = picoui_image_create(win, "logo");
    struct picoui_image_source *image_source = 0;
    const int cols[] = {220, 0};
    const int rows[] = {24, 30, 30, 36, 28, 64, 0};

    picoui_grid_set_columns(win, cols, 2);
    picoui_grid_set_rows(win, rows, 7);
    picoui_grid_set_gap(win, 12, 12);
    picoui_grid_set_align(win, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_window_set_padding_group(win, 16, 24, 16, 16);

    picoui_widget_set_size((struct picoui_widget *)sw, 48, 24);
    picoui_widget_set_size((struct picoui_widget *)cb, 220, 30);
    picoui_widget_set_size((struct picoui_widget *)slider, 220, 30);
    picoui_widget_set_size((struct picoui_widget *)button, 160, 36);
    picoui_widget_set_size((struct picoui_widget *)text, 220, 28);
    picoui_widget_set_size((struct picoui_widget *)image, 220, 60);

    picoui_widget_set_grid_cell((struct picoui_widget *)sw, 0, 0, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_widget_set_grid_cell((struct picoui_widget *)cb, 0, 1, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_widget_set_grid_cell((struct picoui_widget *)slider, 0, 2, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_widget_set_grid_cell((struct picoui_widget *)button, 0, 3, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_widget_set_grid_cell((struct picoui_widget *)text, 0, 4, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);
    picoui_widget_set_grid_cell((struct picoui_widget *)image, 0, 5, 1, 1, PICOUI_ALIGN_START, PICOUI_ALIGN_START);

    picoui_switch_set_checked(sw, 1);
    picoui_checkbox_set_checked(cb, 1);
    picoui_slider_set_value(slider, 28);
    picoui_switch_set_on_toggled(sw, on_wifi_changed, 0);
    picoui_checkbox_set_on_toggled(cb, on_wifi_changed, 0);
    picoui_slider_set_on_value_changed(slider, on_wifi_changed, 0);
    picoui_checkbox_set_text(cb, "Wi-Fi Enabled");
    picoui_button_set_text(button, "Submit");
    picoui_button_set_on_clicked(button, on_button_clicked, 0);
    picoui_text_set_text(text, "Basic Widgets");
    picoui_image_set_source(image, image_source);
}

static int run_demo(void)
{
    struct picoui_window *screen;
    struct picoui_demo_benchmark_state benchmark = {0};

    if (picoui_init() != 0) {
        return 1;
    }

    benchmark.enabled = picoui_demo_benchmark_enabled();
    benchmark.capture_path = getenv("PICOUI_CAPTURE_FILE");
    benchmark.screen_object_create_start_ms = picoui_demo_benchmark_now_ms();
    screen = picoui_screen_create();
    benchmark.screen_object_create_end_ms = picoui_demo_benchmark_now_ms();
    if (screen == 0) {
        picoui_deinit();
        return 1;
    }

    make_ui(screen);
    if (picoui_screen_load(screen) != 0) {
        picoui_deinit();
        return 1;
    }

    benchmark.capture_ready_start_ms = picoui_demo_benchmark_now_ms();
    printf("PICOUI_RUNTIME_LOOP\n");
    fflush(stdout);
    if (benchmark.enabled) {
        picoui_demo_benchmark_log_screen_object_create(&benchmark);
    }

    while (1) {
        picoui_timer_handler();
        if (benchmark.enabled &&
            !benchmark.first_frame_logged &&
            picoui_demo_benchmark_capture_ready(benchmark.capture_path)) {
            picoui_demo_benchmark_log_capture_ready(&benchmark, picoui_demo_benchmark_now_ms());
            benchmark.first_frame_logged = 1;
        }
    }

    picoui_deinit();
    return 0;
}

/**
 * @brief Application entry point
 *
 * @return 0 on success, -1 on failure
 */

int main(void)
{
    return run_demo();
}
