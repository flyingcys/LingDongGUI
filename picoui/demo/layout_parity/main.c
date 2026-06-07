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

#include <time.h>

#include "picoui/picoui.h"
#include "picoui/port/sdl.h"

static struct picoui_window *g_root_window;
static unsigned long long g_layout_last_resize_ms;

struct layout_parity_runtime {
    struct picoui_window *flex_row_sample;
    int compact;
};

static unsigned long long layout_parity_now_ms(void)
{
    struct timespec now;

    (void)timespec_get(&now, TIME_UTC);
    return ((unsigned long long)now.tv_sec * 1000ULL)
           + (unsigned long long)(now.tv_nsec / 1000000L);
}

static void layout_parity_resize_tick(struct layout_parity_runtime *runtime)
{
    if (runtime == 0 || runtime->flex_row_sample == 0) {
        return;
    }

    runtime->compact = !runtime->compact;
    picoui_widget_set_size((struct picoui_widget *)runtime->flex_row_sample,
                           runtime->compact ? 170 : 196,
                           80);
    picoui_flex_set_gap(runtime->flex_row_sample, 6, 4);
}

static void layout_parity_resize_pump(struct layout_parity_runtime *runtime)
{
    unsigned long long now_ms;

    if (runtime == 0 || runtime->flex_row_sample == 0) {
        return;
    }

    now_ms = layout_parity_now_ms();
    while (now_ms - g_layout_last_resize_ms >= 1200ULL) {
        layout_parity_resize_tick(runtime);
        g_layout_last_resize_ms += 1200ULL;
    }
}

static void style_card(struct picoui_button *button,
                       const char *text,
                       int width,
                       int height,
                       unsigned int bg_color)
{
    picoui_button_set_text(button, text);
    picoui_widget_set_size((struct picoui_widget *)button, width, height);
    picoui_widget_set_bg_color((struct picoui_widget *)button, bg_color);
    picoui_button_set_text_color(button, 0xFFFFFFU);
    picoui_widget_set_radius((struct picoui_widget *)button, 6);
}

static struct picoui_window *create_section_shell(struct picoui_window *root,
                                                  const char *section_id,
                                                  const char *title_id,
                                                  const char *hint_id,
                                                  const char *sample_id,
                                                  const char *title,
                                                  const char *hint,
                                                  unsigned int section_bg,
                                                  unsigned int sample_bg)
{
    struct picoui_window *section;
    struct picoui_text *section_title;
    struct picoui_text *section_hint;
    struct picoui_window *sample;

    section = picoui_window_create_child(root, section_id);
    section_title = picoui_text_create(section, title_id);
    section_hint = picoui_text_create(section, hint_id);
    sample = picoui_window_create_child(section, sample_id);

    picoui_widget_set_size((struct picoui_widget *)section, 220, 154);
    picoui_window_set_color(section, section_bg);
    picoui_widget_set_radius((struct picoui_widget *)section, 10);

    picoui_text_set_text(section_title, title);
    picoui_widget_set_pos((struct picoui_widget *)section_title, 12, 10);
    picoui_widget_set_size((struct picoui_widget *)section_title, 196, 18);

    picoui_text_set_text(section_hint, hint);
    picoui_widget_set_pos((struct picoui_widget *)section_hint, 12, 28);
    picoui_widget_set_size((struct picoui_widget *)section_hint, 196, 24);

    picoui_widget_set_pos((struct picoui_widget *)sample, 12, 62);
    picoui_widget_set_size((struct picoui_widget *)sample, 196, 80);
    picoui_window_set_color(sample, sample_bg);
    picoui_widget_set_radius((struct picoui_widget *)sample, 8);

    return sample;
}

static void populate_flex_row_sample(struct picoui_window *sample)
{
    struct picoui_button *row_a;
    struct picoui_button *row_b;
    struct picoui_button *row_c;
    struct picoui_button *row_d;
    struct picoui_button *row_e;

    picoui_window_set_layout_type(sample, PICOUI_WINDOW_LAYOUT_FLEX);
    picoui_window_set_padding_group(sample, 8, 8, 8, 8);
    picoui_flex_set_flow(sample, PICOUI_FLEX_FLOW_ROW_WRAP);
    picoui_flex_set_align(sample,
                          PICOUI_ALIGN_START,
                          PICOUI_ALIGN_CENTER,
                          PICOUI_ALIGN_CENTER);
    picoui_flex_set_gap(sample, 6, 4);

    row_a = picoui_button_create(sample, "row_a");
    row_b = picoui_button_create(sample, "row_b");
    row_c = picoui_button_create(sample, "row_c");
    row_d = picoui_button_create(sample, "row_d");
    row_e = picoui_button_create(sample, "row_e");

    style_card(row_a, "A", 64, 20, 0xE07A5FU);
    style_card(row_b, "B", 58, 20, 0x457B9DU);
    style_card(row_c, "C", 52, 20, 0x81B29AU);
    style_card(row_d, "D*", 60, 20, 0x264653U);
    style_card(row_e, "E", 48, 20, 0xF4A261U);

    picoui_widget_set_flex_new_track((struct picoui_widget *)row_d, 1);
    picoui_widget_set_flex_min_width((struct picoui_widget *)row_e, 48);
}

static void populate_flex_column_sample(struct picoui_window *sample)
{
    struct picoui_button *col_top;
    struct picoui_button *col_1x;
    struct picoui_button *col_2x;
    struct picoui_button *col_free;

    picoui_window_set_layout_type(sample, PICOUI_WINDOW_LAYOUT_FLEX);
    picoui_window_set_padding_group(sample, 10, 8, 10, 8);
    picoui_flex_set_flow(sample, PICOUI_FLEX_FLOW_COLUMN);
    picoui_flex_set_align(sample,
                          PICOUI_ALIGN_START,
                          PICOUI_ALIGN_CENTER,
                          PICOUI_ALIGN_START);
    picoui_flex_set_gap(sample, 4, 4);

    col_top = picoui_button_create(sample, "col_top");
    col_1x = picoui_button_create(sample, "col_1x");
    col_2x = picoui_button_create(sample, "col_2x");
    col_free = picoui_button_create(sample, "col_free");

    style_card(col_top, "top", 68, 18, 0xE76F51U);
    style_card(col_1x, "1x", 72, 18, 0x2A9D8FU);
    style_card(col_2x, "2x", 82, 18, 0x264653U);
    style_card(col_free, "free", 54, 26, 0x577590U);

    picoui_widget_set_flex_grow((struct picoui_widget *)col_1x, 1);
    picoui_widget_set_flex_grow((struct picoui_widget *)col_2x, 2);
    picoui_widget_set_ignore_layout((struct picoui_widget *)col_free, 1);
    picoui_widget_set_pos((struct picoui_widget *)col_free, 130, 34);
}

static void populate_legacy_row_sample(struct picoui_window *sample)
{
    struct picoui_window *slot_a;
    struct picoui_window *slot_b;
    struct picoui_window *slot_c;
    struct picoui_button *card_a;
    struct picoui_button *card_b;
    struct picoui_button *card_c;
    struct picoui_text *badge;

    picoui_window_set_layout_type(sample, PICOUI_WINDOW_LAYOUT_FLEX);
    picoui_window_set_padding_group(sample, 8, 12, 8, 12);
    picoui_flex_set_flow(sample, PICOUI_FLEX_FLOW_ROW);
    picoui_flex_set_align(sample,
                          PICOUI_ALIGN_START,
                          PICOUI_ALIGN_CENTER,
                          PICOUI_ALIGN_START);
    picoui_flex_set_gap(sample, 8, 8);

    slot_a = picoui_window_create_child(sample, "legacy_slot_a");
    slot_b = picoui_window_create_child(sample, "legacy_slot_b");
    slot_c = picoui_window_create_child(sample, "legacy_slot_c");
    picoui_widget_set_size((struct picoui_widget *)slot_a, 52, 30);
    picoui_widget_set_size((struct picoui_widget *)slot_b, 52, 30);
    picoui_widget_set_size((struct picoui_widget *)slot_c, 52, 30);
    picoui_window_set_color(slot_a, 0xE8ECF2U);
    picoui_window_set_color(slot_b, 0xE8ECF2U);
    picoui_window_set_color(slot_c, 0xE8ECF2U);

    card_a = picoui_button_create(slot_a, "legacy_a");
    card_b = picoui_button_create(slot_b, "legacy_b");
    card_c = picoui_button_create(slot_c, "legacy_c");
    badge = picoui_text_create(slot_c, "legacy_c_badge");

    style_card(card_a, "A", 48, 30, 0xF4A261U);
    style_card(card_b, "B", 48, 30, 0x457B9DU);
    style_card(card_c, "C", 48, 30, 0x2A9D8FU);
    picoui_widget_set_visible((struct picoui_widget *)card_b, 0);

    picoui_text_set_text(badge, "n");
    picoui_text_set_bg_color(badge, 0x1D3557U);
    picoui_text_set_text_color(badge, 0xFFFFFFU);
    picoui_widget_set_size((struct picoui_widget *)badge, 14, 12);
    picoui_widget_set_ignore_layout((struct picoui_widget *)badge, 1);
    picoui_widget_set_pos((struct picoui_widget *)badge, 30, 4);
    picoui_widget_set_radius((struct picoui_widget *)badge, 4);
}

static void populate_legacy_column_sample(struct picoui_window *sample)
{
    struct picoui_button *legacy_top;
    struct picoui_button *legacy_middle;
    struct picoui_button *legacy_bottom;

    picoui_window_set_layout_type(sample, PICOUI_WINDOW_LAYOUT_FLEX);
    picoui_window_set_padding_group(sample, 8, 8, 8, 8);
    picoui_flex_set_flow(sample, PICOUI_FLEX_FLOW_COLUMN);
    picoui_flex_set_align(sample,
                          PICOUI_ALIGN_START,
                          PICOUI_ALIGN_START,
                          PICOUI_ALIGN_START);
    picoui_flex_set_gap(sample, 6, 6);

    legacy_top = picoui_button_create(sample, "legacy_top");
    legacy_middle = picoui_button_create(sample, "legacy_middle");
    legacy_bottom = picoui_button_create(sample, "legacy_bottom");

    style_card(legacy_top, "top", 84, 18, 0xE9C46AU);
    style_card(legacy_middle, "middle", 108, 18, 0x457B9DU);
    style_card(legacy_bottom, "bottom", 90, 18, 0x2A9D8FU);
}

static void make_ui(struct picoui_window *win, struct layout_parity_runtime *runtime)
{
    struct picoui_text *page_title;
    struct picoui_text *page_hint;
    struct picoui_window *flex_row_section;
    struct picoui_window *flex_column_section;
    struct picoui_window *legacy_row_section;
    struct picoui_window *legacy_column_section;

    picoui_window_set_color(win, 0xF5F6F8U);
    picoui_window_set_layout_type(win, PICOUI_WINDOW_LAYOUT_FLEX);
    picoui_window_set_padding_group(win, 12, 8, 12, 8);
    picoui_flex_set_flow(win, PICOUI_FLEX_FLOW_ROW_WRAP);
    picoui_flex_set_align(win,
                          PICOUI_ALIGN_START,
                          PICOUI_ALIGN_START,
                          PICOUI_ALIGN_START);
    picoui_flex_set_gap(win, 16, 12);

    page_title = picoui_text_create(win, "page_title");
    page_hint = picoui_text_create(win, "page_hint");
    flex_row_section = create_section_shell(
        win,
        "flex_row_section",
        "flex_row_title",
        "flex_row_hint",
        "flex_row_sample",
        "Flex wrap",
        "D* forces next track; E keeps a public min-width contract.",
        0xFFFFFFU,
        0xF0F0F0U);
    flex_column_section = create_section_shell(
        win,
        "flex_column_section",
        "flex_column_title",
        "flex_column_hint",
        "flex_column_sample",
        "Flex column",
        "center cross-align; 1x/2x grows; 'free' ignores layout.",
        0xFFFFFFU,
        0xECF0E8U);
    legacy_row_section = create_section_shell(
        win,
        "legacy_row_section",
        "legacy_row_title",
        "legacy_row_hint",
        "legacy_row_sample",
        "Legacy row",
        "slot windows keep the hidden middle gap visible without leaking ld* layout APIs.",
        0xFFFFFFU,
        0xE8ECF2U);
    legacy_column_section = create_section_shell(
        win,
        "legacy_column_section",
        "legacy_column_title",
        "legacy_column_hint",
        "legacy_column_sample",
        "Legacy column",
        "classic vertical slots stay grouped inside a dedicated child window section.",
        0xFFFFFFU,
        0xEBEFE8U);

    picoui_text_set_text(page_title, "Layout Demo");
    picoui_text_set_text(page_hint, "Parity baseline now uses child-window sections so row/column/flex groups stay structurally aligned with the legacy SDL layout page.");
    picoui_widget_set_size((struct picoui_widget *)page_title, 460, 20);
    picoui_widget_set_size((struct picoui_widget *)page_hint, 920, 28);
    picoui_widget_set_flex_new_track((struct picoui_widget *)page_title, 1);
    picoui_widget_set_flex_new_track((struct picoui_widget *)page_hint, 1);

    populate_flex_row_sample(flex_row_section);
    populate_flex_column_sample(flex_column_section);
    populate_legacy_row_sample(legacy_row_section);
    populate_legacy_column_sample(legacy_column_section);
    if (runtime != 0) {
        runtime->flex_row_sample = flex_row_section;
    }
}

static int create_demo_ui(struct layout_parity_runtime *runtime)
{
    struct picoui_screen *screen = picoui_screen_active();
    struct picoui_window *win;

    if (runtime == 0 || screen == 0) {
        return -1;
    }

    win = picoui_window_create_root(screen, "root");
    if (win == 0) {
        return -1;
    }

    g_root_window = win;
    make_ui(win, runtime);
    if (runtime->flex_row_sample == 0) {
        g_root_window = 0;
        return -1;
    }
    if (picoui_screen_load(screen) != 0) {
        g_root_window = 0;
        return -1;
    }

    g_layout_last_resize_ms = layout_parity_now_ms();
    return 0;
}

int main(void)
{
    int init_rc;
    int timer_rc;
    struct layout_parity_runtime runtime = {0};

    init_rc = picoui_init();
    if (init_rc != 0) {
        return 1;
    }
    if (picoui_sdl_hal_init(320, 480) != 0) {
        picoui_deinit();
        return 1;
    }
    if (create_demo_ui(&runtime) != 0 || g_root_window == 0) {
        picoui_deinit();
        return 1;
    }

    while (1) {
        layout_parity_resize_pump(&runtime);
        timer_rc = picoui_timer_handler();
        if (timer_rc < 0) {
            picoui_deinit();
            return 1;
        }
        if (timer_rc > 0) {
            picoui_deinit();
            return 0;
        }
    }

    return 0;
}
