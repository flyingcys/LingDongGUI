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

#include "internal.h"
#include "slider.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldSlider.h"

#include <stdlib.h>
#include <string.h>

static const char *s_fail_test_id = 0;
static ld_scene_t *s_slider_depose_scene = NULL;

static void tinyui_slider_ld_depose_cb(void *ld_widget)
{
    if (s_slider_depose_scene != NULL) {
        ldSlider_depose(s_slider_depose_scene, (ldSlider_t *)ld_widget);
        s_slider_depose_scene = NULL;
    }
}

static ldSlider_t *tinyui_slider_backend(struct tinyui_slider *slider)
{
    if (slider == 0 || slider->widget.ld_widget == 0
        || slider->widget.kind != TINYUI_BACKEND_WIDGET_SLIDER) {
        return 0;
    }

    return (ldSlider_t *)slider->widget.ld_widget;
}

void tinyui_slider_test_fail_indicator_width_for_id(const char *id)
{
    s_fail_test_id = id;
}

static int tinyui_slider_props_are_valid(const struct tinyui_slider_props *props)
{
    return props != 0
        && props->id != 0
        && props->min_value <= props->max_value
        && props->value >= props->min_value
        && props->value <= props->max_value
        && (props->background_source == 0
            || props->background_source->img_tile != 0)
        && (props->indicator_source == 0
            || props->indicator_source->img_tile != 0)
        && (props->indicator_width == -1
            || (props->indicator_width >= 0 && props->indicator_width <= 255))
        && (props->slim_size == -1
            || (props->slim_size >= 0 && props->slim_size <= 255))
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

static void *tinyui_slider_ld_init(void *ctx,
                                   struct ld_scene_t *scene,
                                   uint16_t name_id,
                                   uint16_t parent_name_id)
{
    (void)ctx;
    return ldSlider_init(scene, NULL, name_id, parent_name_id, 0, 0, 220, 30);
}

/**
 * @brief Create slider widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_slider *tinyui_slider_create(struct tinyui_window *parent, const char *id)
{
    struct tinyui_slider *slider;

    if (parent == 0 || id == 0) {
        return 0;
    }
    slider = (struct tinyui_slider *)tinyui_widget_create_leaf(&parent->widget,
                                                               TINYUI_BACKEND_WIDGET_SLIDER,
                                                               tinyui_slider_ld_init,
                                                               0,
                                                               sizeof(*slider));
    if (slider == 0) {
        return 0;
    }
    slider->id = id;
    slider->min_value = 0;
    slider->max_value = 100;

    return slider;
}

/**
 * @brief Create slider widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_slider *tinyui_slider_create_with_props(struct tinyui_window *parent,
                                                      const struct tinyui_slider_props *props)
{
    struct tinyui_slider *slider;

    if (!tinyui_slider_props_are_valid(props)) {
        return 0;
    }

    slider = tinyui_slider_create(parent, props->id);
    if (slider == 0) {
        return 0;
    }

    slider->min_value = props->min_value;
    slider->max_value = props->max_value;
    slider->cb = 0;
    slider->user_data = 0;
    if (tinyui_slider_set_range(slider, props->min_value, props->max_value) != 0
        || tinyui_slider_set_value(slider, props->value) != 0
        || (props->horizontal != -1
            && tinyui_slider_set_horizontal(slider, props->horizontal) != 0)
        || (props->background_source != 0
            && tinyui_slider_set_background_source(slider, props->background_source) != 0)
        || (props->indicator_source != 0
            && tinyui_slider_set_indicator_source(slider, props->indicator_source) != 0)
        || (props->indicator_width != -1
            && tinyui_slider_set_indicator_width(slider, props->indicator_width) != 0)
        || (props->slim_size != -1
            && tinyui_slider_set_slim_size(slider, props->slim_size) != 0)) {
        s_slider_depose_scene = slider->widget.ld_event_bridge_scene;
        tinyui_widget_destroy_common(&slider->widget, tinyui_slider_ld_depose_cb);
        return 0;
    }

    /* Under the validated props contract, the remaining widget metadata/style updates
     * do not expose a real failure path for a valid non-window slider backend. */
    slider->cb = props->on_value_changed;
    slider->user_data = props->user_data;
    (void)tinyui_widget_set_user_data(&slider->widget, props->user_data);
    if (props->style_class != 0) {
        (void)tinyui_widget_set_style_class(&slider->widget, props->style_class);
    }
    if (props->width > 0 || props->height > 0) {
        (void)tinyui_widget_set_size(&slider->widget, props->width, props->height);
    }
    (void)tinyui_widget_set_bg_color(&slider->widget, props->bg_color);
    (void)tinyui_widget_set_text_color(&slider->widget, props->text_color);
    (void)tinyui_widget_set_border_color(&slider->widget, props->border_color);
    (void)tinyui_widget_set_radius(&slider->widget, props->radius);
    (void)tinyui_widget_set_padding(&slider->widget, props->padding);
    return slider;
}

/**
 * @brief Set value of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int tinyui_slider_set_value(struct tinyui_slider *slider, int value)
{
    if (slider == 0 || value < slider->min_value || value > slider->max_value) {
        return -1;
    }

    if (slider->value == value) {
        return 0;
    }

    if (slider->widget.ld_widget == 0) {
        return -1;
    }

    slider->value = value;
    return tinyui_widget_update_value(&slider->widget,
                                      slider->value,
                                      slider->cb,
                                      &slider->widget,
                                      slider->user_data);
}

/**
 * @brief Set range of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] min_value min value
 * @param[in] max_value max value
 * @return 0 on success, -1 on failure
 */

int tinyui_slider_set_range(struct tinyui_slider *slider, int min_value, int max_value)
{
    int old_min_value;
    int old_max_value;
    int old_range;
    int old_percent = 0;
    int new_range;
    int remapped_value;

    if (slider == 0 || min_value > max_value) {
        return -1;
    }

    old_min_value = slider->min_value;
    old_max_value = slider->max_value;
    old_range = old_max_value - old_min_value;
    if (old_range > 0) {
        old_percent = ((slider->value - old_min_value) * 100) / old_range;
        if (old_percent < 0) {
            old_percent = 0;
        }
        if (old_percent > 100) {
            old_percent = 100;
        }
    }

    slider->min_value = min_value;
    slider->max_value = max_value;
    new_range = max_value - min_value;
    if (new_range <= 0) {
        remapped_value = min_value;
    } else {
        remapped_value = min_value + (new_range * old_percent) / 100;
    }

    slider->value = remapped_value;
    if (slider->widget.ld_widget != 0) {
        return tinyui_widget_update_value(&slider->widget,
                                          slider->value,
                                          0,
                                          &slider->widget,
                                          0);
    }

    return 0;
}

/**
 * @brief Set percent of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] percent percent
 * @return -1 on failure
 */

int tinyui_slider_set_percent(struct tinyui_slider *slider, int percent)
{
    int value;

    if (slider == 0 || percent < 0 || percent > 100) {
        return -1;
    }

    value = slider->min_value + ((slider->max_value - slider->min_value) * percent) / 100;
    return tinyui_slider_set_value(slider, value);
}

/**
 * @brief Set horizontal of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] horizontal horizontal
 * @return -1 on failure
 */

int tinyui_slider_set_horizontal(struct tinyui_slider *slider, int horizontal)
{
    ldSlider_t *ld_slider;

    if (slider == 0) {
        return -1;
    }

    ld_slider = tinyui_slider_backend(slider);
    if (ld_slider == 0) {
        return -1;
    }

    ldSliderSetHorizontal(ld_slider, horizontal != 0);
    return 0;
}

/**
 * @brief Get horizontal of slider widget
 *
 * @param[out] slider Slider widget instance
 * @param[in] horizontal horizontal
 * @return -1 on failure
 */

int tinyui_slider_get_horizontal(struct tinyui_slider *slider, int *horizontal)
{
    ldSlider_t *ld_slider;

    if (slider == 0 || horizontal == 0) {
        return -1;
    }

    ld_slider = tinyui_slider_backend(slider);
    if (ld_slider == 0) {
        return -1;
    }

    *horizontal = ld_slider->isHorizontal ? 1 : 0;
    return 0;
}

/**
 * @brief Set background source of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int tinyui_slider_set_background_source(struct tinyui_slider *slider,
                                        struct tinyui_image_source *source)
{
    ldSlider_t *ld_slider;

    if (slider == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    ld_slider = tinyui_slider_backend(slider);
    if (ld_slider == 0) {
        return -1;
    }

    ldSliderSetImage(ld_slider,
                     source != 0 ? source->img_tile : 0,
                     source != 0 ? source->mask_tile : 0,
                     ld_slider->ptIndicImgTile,
                     ld_slider->ptIndicMaskTile);
    return 0;
}

/**
 * @brief Set indicator source of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int tinyui_slider_set_indicator_source(struct tinyui_slider *slider,
                                       struct tinyui_image_source *source)
{
    ldSlider_t *ld_slider;

    if (slider == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    ld_slider = tinyui_slider_backend(slider);
    if (ld_slider == 0) {
        return -1;
    }

    ldSliderSetImage(ld_slider,
                     ld_slider->ptBgImgTile,
                     ld_slider->ptBgMaskTile,
                     source != 0 ? source->img_tile : 0,
                     source != 0 ? source->mask_tile : 0);
    return 0;
}

/**
 * @brief Set image of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] background_source background source
 * @param[in] indicator_source indicator source
 * @return -1 on failure
 */

int tinyui_slider_set_image(struct tinyui_slider *slider,
                            struct tinyui_image_source *background_source,
                            struct tinyui_image_source *indicator_source)
{
    if (tinyui_slider_set_background_source(slider, background_source) != 0) {
        return -1;
    }
    return tinyui_slider_set_indicator_source(slider, indicator_source);
}

/**
 * @brief Set color of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] bg_color Background color
 * @param[in] frame_color frame color
 * @param[in] indicator_color indicator color
 * @return 0 on success, -1 on failure
 */

int tinyui_slider_set_color(struct tinyui_slider *slider,
                            unsigned int bg_color,
                            unsigned int frame_color,
                            unsigned int indicator_color)
{
    ldSlider_t *ld_slider;

    if (slider == 0 || bg_color > 0xFFFFFFU || frame_color > 0xFFFFFFU ||
        indicator_color > 0xFFFFFFU) {
        return -1;
    }

    ld_slider = tinyui_slider_backend(slider);
    if (ld_slider == 0) {
        return -1;
    }

    ldSliderSetColor(ld_slider, (ldColor)bg_color, (ldColor)frame_color, (ldColor)indicator_color);
    return 0;
}

/**
 * @brief Set indicator width of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] indicator_width indicator width
 * @return -1 on failure
 */

int tinyui_slider_set_indicator_width(struct tinyui_slider *slider, int indicator_width)
{
    ldSlider_t *ld_slider;

    if (slider == 0 || indicator_width < 0) {
        return -1;
    }

    ld_slider = tinyui_slider_backend(slider);
    if (ld_slider == 0 || indicator_width > 255) {
        return -1;
    }

    if (s_fail_test_id != 0 && slider->id != 0
        && strcmp(slider->id, s_fail_test_id) == 0) {
        s_fail_test_id = 0;
        return -1;
    }

    ldSliderSetIndicatorWidth(ld_slider, (uint8_t)indicator_width);
    return 0;
}

/**
 * @brief Set slim size of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] slim_size slim size
 * @return -1 on failure
 */

int tinyui_slider_set_slim_size(struct tinyui_slider *slider, int slim_size)
{
    ldSlider_t *ld_slider;

    if (slider == 0 || slim_size < 0) {
        return -1;
    }

    ld_slider = tinyui_slider_backend(slider);
    if (ld_slider == 0 || slim_size > 255) {
        return -1;
    }

    ldSliderSetSlimSize(ld_slider, (uint8_t)slim_size);
    return 0;
}

/**
 * @brief Get percent of slider widget
 *
 * @param[out] slider Slider widget instance
 * @param[in] percent percent
 * @return -1 on failure
 */

int tinyui_slider_get_percent(struct tinyui_slider *slider, int *percent)
{
    ldSlider_t *ld_slider;

    if (slider == 0 || percent == 0) {
        return -1;
    }

    ld_slider = tinyui_slider_backend(slider);
    if (ld_slider == 0) {
        return -1;
    }

    *percent = (int)(ld_slider->permille / 10U);
    return 0;
}

/**
 * @brief Set on value changed of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int tinyui_slider_set_on_value_changed(struct tinyui_slider *slider,
                                       tinyui_value_changed_cb cb,
                                       void *user_data)
{
    if (slider == 0) {
        return -1;
    }

    slider->cb = cb;
    slider->user_data = user_data;
    return 0;
}
