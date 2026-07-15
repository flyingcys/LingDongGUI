/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "layout_parity/layout_parity.h"
#include "tinyui.h"

static tinyui_result_t style_card(tinyui_obj_t *button,
                                  const char *text,
                                  int width,
                                  int height,
                                  unsigned int bg_color)
{
    if (tinyui_button_set_text(button, text) != 0
        || tinyui_obj_set_size(button, width, height) != TINYUI_OK
        || tinyui_obj_set_bg_color(button, bg_color) != TINYUI_OK
        || tinyui_button_set_text_color(button, 0xFFFFFFU) != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    return TINYUI_OK;
}

static tinyui_result_t create_section_shell(tinyui_obj_t *root,
                                            const char *title,
                                            const char *hint,
                                            unsigned int section_bg,
                                            unsigned int sample_bg,
                                            int x,
                                            int y,
                                            tinyui_obj_t **out_sample)
{
    tinyui_obj_t *section;
    tinyui_obj_t *section_title;
    tinyui_obj_t *section_hint;
    tinyui_obj_t *sample;

    section = tinyui_window_create(root);
    if (section == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }
    if (tinyui_obj_set_pos(section, x, y) != TINYUI_OK
        || tinyui_obj_set_size(section, 220, 154) != TINYUI_OK
        || tinyui_window_set_color(section, section_bg) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    section_title = tinyui_text_create(section);
    section_hint = tinyui_text_create(section);
    sample = tinyui_window_create(section);
    if (section_title == NULL || section_hint == NULL || sample == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_text_set_text(section_title, title) != 0
        || tinyui_obj_set_pos(section_title, 12, 10) != TINYUI_OK
        || tinyui_obj_set_size(section_title, 196, 18) != TINYUI_OK
        || tinyui_text_set_text(section_hint, hint) != 0
        || tinyui_obj_set_pos(section_hint, 12, 28) != TINYUI_OK
        || tinyui_obj_set_size(section_hint, 196, 24) != TINYUI_OK
        || tinyui_obj_set_pos(sample, 12, 62) != TINYUI_OK
        || tinyui_obj_set_size(sample, 196, 80) != TINYUI_OK
        || tinyui_window_set_color(sample, sample_bg) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    *out_sample = sample;
    return TINYUI_OK;
}

static tinyui_result_t populate_flex_row_sample(tinyui_obj_t *sample)
{
    tinyui_obj_t *row_a;
    tinyui_obj_t *row_b;
    tinyui_obj_t *row_c;
    tinyui_obj_t *row_d;
    tinyui_obj_t *row_e;
    tinyui_result_t result;

    if (tinyui_window_set_layout_type(sample, TINYUI_WINDOW_LAYOUT_FLEX) != 0
        || tinyui_window_set_padding(sample, 8, 8, 8, 8) != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    result = tinyui_flex_set_flow(sample, TINYUI_FLEX_FLOW_ROW_WRAP);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_align(sample,
                                   TINYUI_ALIGN_START,
                                   TINYUI_ALIGN_CENTER,
                                   TINYUI_ALIGN_CENTER);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_gap(sample, 6, 4);
    if (result != TINYUI_OK) {
        return result;
    }

    row_a = tinyui_button_create(sample);
    row_b = tinyui_button_create(sample);
    row_c = tinyui_button_create(sample);
    row_d = tinyui_button_create(sample);
    row_e = tinyui_button_create(sample);
    if (row_a == NULL || row_b == NULL || row_c == NULL || row_d == NULL || row_e == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (style_card(row_a, "A", 64, 20, 0xE07A5FU) != TINYUI_OK
        || style_card(row_b, "B", 58, 20, 0x457B9DU) != TINYUI_OK
        || style_card(row_c, "C", 52, 20, 0x81B29AU) != TINYUI_OK
        || style_card(row_d, "D*", 60, 20, 0x264653U) != TINYUI_OK
        || style_card(row_e, "E", 48, 20, 0xF4A261U) != TINYUI_OK
        || tinyui_obj_set_flex_new_track(row_d, 1) != TINYUI_OK
        || tinyui_obj_set_flex_min_width(row_e, 48) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }
    return TINYUI_OK;
}

static tinyui_result_t populate_flex_column_sample(tinyui_obj_t *sample)
{
    tinyui_obj_t *col_top;
    tinyui_obj_t *col_1x;
    tinyui_obj_t *col_2x;
    tinyui_obj_t *col_free;
    tinyui_result_t result;

    if (tinyui_window_set_layout_type(sample, TINYUI_WINDOW_LAYOUT_FLEX) != 0
        || tinyui_window_set_padding(sample, 10, 8, 10, 8) != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    result = tinyui_flex_set_flow(sample, TINYUI_FLEX_FLOW_COLUMN);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_align(sample,
                                   TINYUI_ALIGN_START,
                                   TINYUI_ALIGN_CENTER,
                                   TINYUI_ALIGN_START);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_gap(sample, 4, 4);
    if (result != TINYUI_OK) {
        return result;
    }

    col_top = tinyui_button_create(sample);
    col_1x = tinyui_button_create(sample);
    col_2x = tinyui_button_create(sample);
    col_free = tinyui_button_create(sample);
    if (col_top == NULL || col_1x == NULL || col_2x == NULL || col_free == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (style_card(col_top, "top", 68, 18, 0xE76F51U) != TINYUI_OK
        || style_card(col_1x, "1x", 72, 18, 0x2A9D8FU) != TINYUI_OK
        || style_card(col_2x, "2x", 82, 18, 0x264653U) != TINYUI_OK
        || style_card(col_free, "free", 54, 26, 0x577590U) != TINYUI_OK
        || tinyui_obj_set_flex_grow(col_1x, 1) != TINYUI_OK
        || tinyui_obj_set_flex_grow(col_2x, 2) != TINYUI_OK
        || tinyui_obj_set_ignore_layout(col_free, 1) != TINYUI_OK
        || tinyui_obj_set_pos(col_free, 130, 34) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }
    return TINYUI_OK;
}

static tinyui_result_t populate_slot_row_sample(tinyui_obj_t *sample)
{
    tinyui_obj_t *slot_a;
    tinyui_obj_t *slot_b;
    tinyui_obj_t *slot_c;
    tinyui_obj_t *card_a;
    tinyui_obj_t *card_b;
    tinyui_obj_t *card_c;
    tinyui_obj_t *badge;
    tinyui_result_t result;

    if (tinyui_window_set_layout_type(sample, TINYUI_WINDOW_LAYOUT_FLEX) != 0
        || tinyui_window_set_padding(sample, 8, 12, 8, 12) != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    result = tinyui_flex_set_flow(sample, TINYUI_FLEX_FLOW_ROW);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_align(sample,
                                   TINYUI_ALIGN_START,
                                   TINYUI_ALIGN_CENTER,
                                   TINYUI_ALIGN_START);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_gap(sample, 8, 8);
    if (result != TINYUI_OK) {
        return result;
    }

    slot_a = tinyui_window_create(sample);
    slot_b = tinyui_window_create(sample);
    slot_c = tinyui_window_create(sample);
    if (slot_a == NULL || slot_b == NULL || slot_c == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }
    if (tinyui_obj_set_size(slot_a, 52, 30) != TINYUI_OK
        || tinyui_obj_set_size(slot_b, 52, 30) != TINYUI_OK
        || tinyui_obj_set_size(slot_c, 52, 30) != TINYUI_OK
        || tinyui_window_set_color(slot_a, 0xE8ECF2U) != 0
        || tinyui_window_set_color(slot_b, 0xE8ECF2U) != 0
        || tinyui_window_set_color(slot_c, 0xE8ECF2U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    card_a = tinyui_button_create(slot_a);
    card_b = tinyui_button_create(slot_b);
    card_c = tinyui_button_create(slot_c);
    badge = tinyui_text_create(slot_c);
    if (card_a == NULL || card_b == NULL || card_c == NULL || badge == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (style_card(card_a, "A", 48, 30, 0xF4A261U) != TINYUI_OK
        || style_card(card_b, "B", 48, 30, 0x457B9DU) != TINYUI_OK
        || style_card(card_c, "C", 48, 30, 0x2A9D8FU) != TINYUI_OK
        || tinyui_obj_set_visible(card_b, 0) != TINYUI_OK
        || tinyui_text_set_text(badge, "n") != 0
        || tinyui_text_set_bg_color(badge, 0x1D3557U) != 0
        || tinyui_text_set_text_color(badge, 0xFFFFFFU) != 0
        || tinyui_obj_set_size(badge, 14, 12) != TINYUI_OK
        || tinyui_obj_set_ignore_layout(badge, 1) != TINYUI_OK
        || tinyui_obj_set_pos(badge, 30, 4) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }
    return TINYUI_OK;
}

static tinyui_result_t populate_slot_column_sample(tinyui_obj_t *sample)
{
    tinyui_obj_t *top;
    tinyui_obj_t *middle;
    tinyui_obj_t *bottom;
    tinyui_result_t result;

    if (tinyui_window_set_layout_type(sample, TINYUI_WINDOW_LAYOUT_FLEX) != 0
        || tinyui_window_set_padding(sample, 8, 8, 8, 8) != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    result = tinyui_flex_set_flow(sample, TINYUI_FLEX_FLOW_COLUMN);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_align(sample,
                                   TINYUI_ALIGN_START,
                                   TINYUI_ALIGN_START,
                                   TINYUI_ALIGN_START);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_gap(sample, 6, 6);
    if (result != TINYUI_OK) {
        return result;
    }

    top = tinyui_button_create(sample);
    middle = tinyui_button_create(sample);
    bottom = tinyui_button_create(sample);
    if (top == NULL || middle == NULL || bottom == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (style_card(top, "top", 84, 18, 0xE9C46AU) != TINYUI_OK
        || style_card(middle, "middle", 108, 18, 0x457B9DU) != TINYUI_OK
        || style_card(bottom, "bottom", 90, 18, 0x2A9D8FU) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }
    return TINYUI_OK;
}

tinyui_result_t tinyui_demo_layout_parity_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *page_title;
    tinyui_obj_t *page_hint;
    tinyui_obj_t *flex_row_sample;
    tinyui_obj_t *flex_column_sample;
    tinyui_obj_t *slot_row_sample;
    tinyui_obj_t *slot_column_sample;
    tinyui_result_t result;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    if (tinyui_window_set_color(screen, 0xF5F6F8U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    page_title = tinyui_text_create(screen);
    page_hint = tinyui_text_create(screen);
    if (page_title == NULL || page_hint == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_text_set_text(page_title, "Layout Demo") != 0
        || tinyui_text_set_text(
               page_hint,
               "Child-window sections keep flex row/column/grow/new-track/ignore groups structured.") != 0
        || tinyui_obj_set_pos(page_title, 12, 8) != TINYUI_OK
        || tinyui_obj_set_size(page_title, 460, 20) != TINYUI_OK
        || tinyui_obj_set_pos(page_hint, 12, 30) != TINYUI_OK
        || tinyui_obj_set_size(page_hint, 920, 28) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    result = create_section_shell(screen, "Flex wrap",
                                  "D* forces next track; E keeps min-width.",
                                  0xFFFFFFU, 0xF0F0F0U, 12, 68, &flex_row_sample);
    if (result != TINYUI_OK) {
        return result;
    }
    result = create_section_shell(screen, "Flex column",
                                  "1x/2x grow; free ignores layout.",
                                  0xFFFFFFU, 0xECF0E8U, 248, 68, &flex_column_sample);
    if (result != TINYUI_OK) {
        return result;
    }
    result = create_section_shell(screen, "Slot row",
                                  "Hidden middle gap stays visible via slots.",
                                  0xFFFFFFU, 0xE8ECF2U, 12, 238, &slot_row_sample);
    if (result != TINYUI_OK) {
        return result;
    }
    result = create_section_shell(screen, "Slot column",
                                  "Vertical slots stay grouped.",
                                  0xFFFFFFU, 0xEBEFE8U, 248, 238, &slot_column_sample);
    if (result != TINYUI_OK) {
        return result;
    }

    result = populate_flex_row_sample(flex_row_sample);
    if (result != TINYUI_OK) {
        return result;
    }
    result = populate_flex_column_sample(flex_column_sample);
    if (result != TINYUI_OK) {
        return result;
    }
    result = populate_slot_row_sample(slot_row_sample);
    if (result != TINYUI_OK) {
        return result;
    }
    result = populate_slot_column_sample(slot_column_sample);
    if (result != TINYUI_OK) {
        return result;
    }

    return TINYUI_OK;
}
