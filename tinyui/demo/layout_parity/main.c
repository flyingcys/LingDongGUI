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

#include "tinyui.h"

struct layout_parity_runtime {
    struct tinyui_window *flex_row_sample;
    int compact;
};

static void layout_parity_resize_tick(struct tinyui_app *app,
                                      struct tinyui_app_timer *timer,
                                      void *user_data)
{
    struct layout_parity_runtime *runtime = (struct layout_parity_runtime *)user_data;

    (void)app;
    (void)timer;
    if (runtime == 0 || runtime->flex_row_sample == 0) {
        return;
    }

    runtime->compact = !runtime->compact;
    tinyui_widget_set_size((struct tinyui_widget *)runtime->flex_row_sample,
                           runtime->compact ? 170 : 196,
                           80);
    tinyui_flex_set_gap(runtime->flex_row_sample, 6, 4);
}

static void style_card(struct tinyui_button *button,
                       const char *text,
                       int width,
                       int height,
                       unsigned int bg_color)
{
    tinyui_button_set_text(button, text);
    tinyui_widget_set_size((struct tinyui_widget *)button, width, height);
    tinyui_widget_set_bg_color((struct tinyui_widget *)button, bg_color);
    tinyui_button_set_text_color(button, 0xFFFFFFU);
    tinyui_widget_set_radius((struct tinyui_widget *)button, 6);
}

static struct tinyui_window *create_section_shell(struct tinyui_window *root,
                                                  const char *section_id,
                                                  const char *title_id,
                                                  const char *hint_id,
                                                  const char *sample_id,
                                                  const char *title,
                                                  const char *hint,
                                                  unsigned int section_bg,
                                                  unsigned int sample_bg)
{
    struct tinyui_window *section;
    struct tinyui_text *section_title;
    struct tinyui_text *section_hint;
    struct tinyui_window *sample;

    section = tinyui_window_create_child(root, section_id);
    section_title = tinyui_text_create(section, title_id);
    section_hint = tinyui_text_create(section, hint_id);
    sample = tinyui_window_create_child(section, sample_id);

    tinyui_widget_set_size((struct tinyui_widget *)section, 220, 154);
    tinyui_window_set_color(section, section_bg);
    tinyui_widget_set_radius((struct tinyui_widget *)section, 10);

    tinyui_text_set_text(section_title, title);
    tinyui_widget_set_pos((struct tinyui_widget *)section_title, 12, 10);
    tinyui_widget_set_size((struct tinyui_widget *)section_title, 196, 18);

    tinyui_text_set_text(section_hint, hint);
    tinyui_widget_set_pos((struct tinyui_widget *)section_hint, 12, 28);
    tinyui_widget_set_size((struct tinyui_widget *)section_hint, 196, 24);

    tinyui_widget_set_pos((struct tinyui_widget *)sample, 12, 62);
    tinyui_widget_set_size((struct tinyui_widget *)sample, 196, 80);
    tinyui_window_set_color(sample, sample_bg);
    tinyui_widget_set_radius((struct tinyui_widget *)sample, 8);

    return sample;
}

static void populate_flex_row_sample(struct tinyui_window *sample)
{
    struct tinyui_button *row_a;
    struct tinyui_button *row_b;
    struct tinyui_button *row_c;
    struct tinyui_button *row_d;
    struct tinyui_button *row_e;

    tinyui_window_set_layout_type(sample, TINYUI_WINDOW_LAYOUT_FLEX);
    tinyui_window_set_padding_group(sample, 8, 8, 8, 8);
    tinyui_flex_set_flow(sample, TINYUI_FLEX_FLOW_ROW_WRAP);
    tinyui_flex_set_align(sample,
                          TINYUI_ALIGN_START,
                          TINYUI_ALIGN_CENTER,
                          TINYUI_ALIGN_CENTER);
    tinyui_flex_set_gap(sample, 6, 4);

    row_a = tinyui_button_create(sample, "row_a");
    row_b = tinyui_button_create(sample, "row_b");
    row_c = tinyui_button_create(sample, "row_c");
    row_d = tinyui_button_create(sample, "row_d");
    row_e = tinyui_button_create(sample, "row_e");

    style_card(row_a, "A", 64, 20, 0xE07A5FU);
    style_card(row_b, "B", 58, 20, 0x457B9DU);
    style_card(row_c, "C", 52, 20, 0x81B29AU);
    style_card(row_d, "D*", 60, 20, 0x264653U);
    style_card(row_e, "E", 48, 20, 0xF4A261U);

    tinyui_widget_set_flex_new_track((struct tinyui_widget *)row_d, 1);
    tinyui_widget_set_flex_min_width((struct tinyui_widget *)row_e, 48);
}

static void populate_flex_column_sample(struct tinyui_window *sample)
{
    struct tinyui_button *col_top;
    struct tinyui_button *col_1x;
    struct tinyui_button *col_2x;
    struct tinyui_button *col_free;

    tinyui_window_set_layout_type(sample, TINYUI_WINDOW_LAYOUT_FLEX);
    tinyui_window_set_padding_group(sample, 10, 8, 10, 8);
    tinyui_flex_set_flow(sample, TINYUI_FLEX_FLOW_COLUMN);
    tinyui_flex_set_align(sample,
                          TINYUI_ALIGN_START,
                          TINYUI_ALIGN_CENTER,
                          TINYUI_ALIGN_START);
    tinyui_flex_set_gap(sample, 4, 4);

    col_top = tinyui_button_create(sample, "col_top");
    col_1x = tinyui_button_create(sample, "col_1x");
    col_2x = tinyui_button_create(sample, "col_2x");
    col_free = tinyui_button_create(sample, "col_free");

    style_card(col_top, "top", 68, 18, 0xE76F51U);
    style_card(col_1x, "1x", 72, 18, 0x2A9D8FU);
    style_card(col_2x, "2x", 82, 18, 0x264653U);
    style_card(col_free, "free", 54, 26, 0x577590U);

    tinyui_widget_set_flex_grow((struct tinyui_widget *)col_1x, 1);
    tinyui_widget_set_flex_grow((struct tinyui_widget *)col_2x, 2);
    tinyui_widget_set_ignore_layout((struct tinyui_widget *)col_free, 1);
    tinyui_widget_set_pos((struct tinyui_widget *)col_free, 130, 34);
}

static void populate_legacy_row_sample(struct tinyui_window *sample)
{
    struct tinyui_window *slot_a;
    struct tinyui_window *slot_b;
    struct tinyui_window *slot_c;
    struct tinyui_button *card_a;
    struct tinyui_button *card_b;
    struct tinyui_button *card_c;
    struct tinyui_text *badge;

    tinyui_window_set_layout_type(sample, TINYUI_WINDOW_LAYOUT_FLEX);
    tinyui_window_set_padding_group(sample, 8, 12, 8, 12);
    tinyui_flex_set_flow(sample, TINYUI_FLEX_FLOW_ROW);
    tinyui_flex_set_align(sample,
                          TINYUI_ALIGN_START,
                          TINYUI_ALIGN_CENTER,
                          TINYUI_ALIGN_START);
    tinyui_flex_set_gap(sample, 8, 8);

    slot_a = tinyui_window_create_child(sample, "legacy_slot_a");
    slot_b = tinyui_window_create_child(sample, "legacy_slot_b");
    slot_c = tinyui_window_create_child(sample, "legacy_slot_c");
    tinyui_widget_set_size((struct tinyui_widget *)slot_a, 52, 30);
    tinyui_widget_set_size((struct tinyui_widget *)slot_b, 52, 30);
    tinyui_widget_set_size((struct tinyui_widget *)slot_c, 52, 30);
    tinyui_window_set_color(slot_a, 0xE8ECF2U);
    tinyui_window_set_color(slot_b, 0xE8ECF2U);
    tinyui_window_set_color(slot_c, 0xE8ECF2U);

    card_a = tinyui_button_create(slot_a, "legacy_a");
    card_b = tinyui_button_create(slot_b, "legacy_b");
    card_c = tinyui_button_create(slot_c, "legacy_c");
    badge = tinyui_text_create(slot_c, "legacy_c_badge");

    style_card(card_a, "A", 48, 30, 0xF4A261U);
    style_card(card_b, "B", 48, 30, 0x457B9DU);
    style_card(card_c, "C", 48, 30, 0x2A9D8FU);
    tinyui_widget_set_visible((struct tinyui_widget *)card_b, 0);

    tinyui_text_set_text(badge, "n");
    tinyui_text_set_bg_color(badge, 0x1D3557U);
    tinyui_text_set_text_color(badge, 0xFFFFFFU);
    tinyui_widget_set_size((struct tinyui_widget *)badge, 14, 12);
    tinyui_widget_set_ignore_layout((struct tinyui_widget *)badge, 1);
    tinyui_widget_set_pos((struct tinyui_widget *)badge, 30, 4);
    tinyui_widget_set_radius((struct tinyui_widget *)badge, 4);
}

static void populate_legacy_column_sample(struct tinyui_window *sample)
{
    struct tinyui_button *legacy_top;
    struct tinyui_button *legacy_middle;
    struct tinyui_button *legacy_bottom;

    tinyui_window_set_layout_type(sample, TINYUI_WINDOW_LAYOUT_FLEX);
    tinyui_window_set_padding_group(sample, 8, 8, 8, 8);
    tinyui_flex_set_flow(sample, TINYUI_FLEX_FLOW_COLUMN);
    tinyui_flex_set_align(sample,
                          TINYUI_ALIGN_START,
                          TINYUI_ALIGN_START,
                          TINYUI_ALIGN_START);
    tinyui_flex_set_gap(sample, 6, 6);

    legacy_top = tinyui_button_create(sample, "legacy_top");
    legacy_middle = tinyui_button_create(sample, "legacy_middle");
    legacy_bottom = tinyui_button_create(sample, "legacy_bottom");

    style_card(legacy_top, "top", 84, 18, 0xE9C46AU);
    style_card(legacy_middle, "middle", 108, 18, 0x457B9DU);
    style_card(legacy_bottom, "bottom", 90, 18, 0x2A9D8FU);
}

static void make_ui(struct tinyui_window *win, struct layout_parity_runtime *runtime)
{
    struct tinyui_text *page_title;
    struct tinyui_text *page_hint;
    struct tinyui_window *flex_row_section;
    struct tinyui_window *flex_column_section;
    struct tinyui_window *legacy_row_section;
    struct tinyui_window *legacy_column_section;

    tinyui_window_set_color(win, 0xF5F6F8U);
    tinyui_window_set_layout_type(win, TINYUI_WINDOW_LAYOUT_FLEX);
    tinyui_window_set_padding_group(win, 12, 8, 12, 8);
    tinyui_flex_set_flow(win, TINYUI_FLEX_FLOW_ROW_WRAP);
    tinyui_flex_set_align(win,
                          TINYUI_ALIGN_START,
                          TINYUI_ALIGN_START,
                          TINYUI_ALIGN_START);
    tinyui_flex_set_gap(win, 16, 12);

    page_title = tinyui_text_create(win, "page_title");
    page_hint = tinyui_text_create(win, "page_hint");
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

    tinyui_text_set_text(page_title, "Layout Demo");
    tinyui_text_set_text(page_hint, "Parity baseline now uses child-window sections so row/column/flex groups stay structurally aligned with the legacy SDL layout page.");
    tinyui_widget_set_size((struct tinyui_widget *)page_title, 460, 20);
    tinyui_widget_set_size((struct tinyui_widget *)page_hint, 920, 28);
    tinyui_widget_set_flex_new_track((struct tinyui_widget *)page_title, 1);
    tinyui_widget_set_flex_new_track((struct tinyui_widget *)page_hint, 1);

    populate_flex_row_sample(flex_row_section);
    populate_flex_column_sample(flex_column_section);
    populate_legacy_row_sample(legacy_row_section);
    populate_legacy_column_sample(legacy_column_section);
    if (runtime != 0) {
        runtime->flex_row_sample = flex_row_section;
    }
}

static int run_demo(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_app_timer *timer;
    struct layout_parity_runtime runtime = {0};

    if (app == 0) {
        return 1;
    }

    win = tinyui_window_create(app, "root");
    if (win == 0) {
        tinyui_app_destroy(app);
        return 1;
    }

    make_ui(win, &runtime);
    timer = tinyui_app_timer_create(app);
    if (runtime.flex_row_sample == 0 || timer == 0 ||
        tinyui_app_timer_start(timer, 1200, 1, layout_parity_resize_tick, &runtime) != 0) {
        tinyui_app_destroy(app);
        return 1;
    }
    if (tinyui_app_run(app, win) != 0) {
        tinyui_app_destroy(app);
        return 1;
    }

    tinyui_app_destroy(app);
    return 0;
}

int main(void)
{
    return run_demo();
}
