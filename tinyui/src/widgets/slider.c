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

int tinyui_runtime_bridge_unbind_host(void *backend_widget);
int tinyui_runtime_bridge_detach_from_parent(void *backend_widget);

static const char *tinyui_slider_fail_indicator_width_id = 0;
static struct picoui_backend_widget tinyui_slider_last_disposed_backend_snapshot;
static int tinyui_slider_last_disposed_backend_valid = 0;

static struct picoui_backend_widget *tinyui_slider_backend(struct picoui_slider *slider)
{
    struct picoui_backend_widget *backend;

    if (slider == 0 || slider->widget.backend_widget == 0) {
        return 0;
    }

    backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_SLIDER || backend->ld_widget == 0) {
        return 0;
    }

    return backend;
}

static ldSlider_t *tinyui_slider_get_ld(struct picoui_slider *slider)
{
    struct picoui_backend_widget *backend = tinyui_slider_backend(slider);

    if (backend == 0) {
        return 0;
    }

    return (ldSlider_t *)backend->ld_widget;
}

static int tinyui_slider_should_fail_indicator_width(const struct picoui_slider *slider)
{
    if (tinyui_slider_fail_indicator_width_id == 0 || slider == 0 || slider->id == 0) {
        return 0;
    }

    if (strcmp(slider->id, tinyui_slider_fail_indicator_width_id) != 0) {
        return 0;
    }

    tinyui_slider_fail_indicator_width_id = 0;
    return 1;
}

static void tinyui_slider_dispose_partial(struct picoui_slider *slider)
{
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;

    if (slider == 0) {
        return;
    }

    backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    if (backend != 0) {
        app_state = tinyui_runtime_bridge_backend_state(backend->owner);
        if (backend->parent != 0) {
            (void)tinyui_runtime_bridge_detach_from_parent(backend);
        }
        (void)tinyui_runtime_bridge_unbind_host(backend);
        tinyui_slider_last_disposed_backend_snapshot = *backend;
        tinyui_slider_last_disposed_backend_valid = 1;
        if (app_state != 0 && app_state->ld_scene != 0 && backend->ld_widget != 0) {
            ldSlider_depose(app_state->ld_scene, (ldSlider_t *)backend->ld_widget);
        }
        free(backend);
    }

    free(slider);
}

void tinyui_slider_test_fail_indicator_width_for_id(const char *id)
{
    tinyui_slider_fail_indicator_width_id = id;
}

const struct picoui_backend_widget *tinyui_slider_test_last_disposed_backend(void)
{
    if (tinyui_slider_last_disposed_backend_valid == 0) {
        return 0;
    }

    return &tinyui_slider_last_disposed_backend_snapshot;
}

static int tinyui_slider_props_are_valid(const struct picoui_slider_props *props)
{
    return props != 0
        && props->id != 0
        && props->min_value <= props->max_value
        && props->value >= props->min_value
        && props->value <= props->max_value
        && (props->has_background_source == 0
            || props->background_source == 0
            || props->background_source->img_tile != 0)
        && (props->has_indicator_source == 0
            || props->indicator_source == 0
            || props->indicator_source->img_tile != 0)
        && (props->has_indicator_width == 0
            || (props->indicator_width >= 0 && props->indicator_width <= 255))
        && (props->has_slim_size == 0
            || (props->slim_size >= 0 && props->slim_size <= 255))
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

/**
 * @brief Create slider widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_slider *picoui_slider_create(struct picoui_window *parent, const char *id)
{
    struct picoui_slider *slider;
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    struct picoui_backend_app_state *app_state;
    ldSlider_t *ld_slider;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    parent_backend = (struct picoui_backend_widget *)parent->widget.backend_widget;
    app_state = tinyui_runtime_bridge_backend_state_from_parent(parent_backend);
    if (parent_backend == 0 || parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    slider = calloc(1, sizeof(*slider));
    if (slider == 0) {
        return 0;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        free(slider);
        return 0;
    }

    name_id = tinyui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(backend);
        free(slider);
        return 0;
    }

    ld_slider = ldSlider_init(app_state->ld_scene,
                              NULL,
                              name_id,
                              parent_backend->ld_name_id,
                              0,
                              0,
                              220,
                              30);
    if (ld_slider == 0) {
        free(backend);
        free(slider);
        return 0;
    }

    if (tinyui_widget_init_child(backend,
                                         parent_backend,
                                         PICOUI_BACKEND_WIDGET_SLIDER,
                                         id,
                                         parent_backend->theme) != 0) {
        ldSlider_depose(app_state->ld_scene, ld_slider);
        free(backend);
        free(slider);
        return 0;
    }
    backend->ld_widget = ld_slider;
    backend->ld_name_id = name_id;
    backend->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (tinyui_widget_attach_child(parent_backend, backend) != 0) {
        ldSlider_depose(app_state->ld_scene, ld_slider);
        free(backend);
        free(slider);
        return 0;
    }

    slider->id = id;
    slider->widget.backend_widget = backend;
    slider->min_value = 0;
    slider->max_value = 100;
    slider->widget.visible = 1;
    slider->widget.enabled = 1;
    if (tinyui_runtime_bridge_bind_host(slider->widget.backend_widget, &slider->widget) != 0) {
        tinyui_slider_dispose_partial(slider);
        return 0;
    }
    return slider;
}

/**
 * @brief slider init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct picoui_slider *picoui_slider_init(struct picoui_window *parent, const char *id)
{
    return picoui_slider_create(parent, id);
}

/**
 * @brief Create slider widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_slider *picoui_slider_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_slider_props *props)
{
    struct picoui_slider *slider;

    if (!tinyui_slider_props_are_valid(props)) {
        return 0;
    }

    slider = picoui_slider_create(parent, props->id);
    if (slider == 0) {
        return 0;
    }

    slider->min_value = props->min_value;
    slider->max_value = props->max_value;
    slider->cb = 0;
    slider->user_data = 0;
    if (picoui_slider_set_range(slider, props->min_value, props->max_value) != 0
        || picoui_slider_set_value(slider, props->value) != 0
        || (props->has_horizontal != 0
            && picoui_slider_set_horizontal(slider, props->horizontal) != 0)
        || (props->has_background_source != 0
            && picoui_slider_set_background_source(slider, props->background_source) != 0)
        || (props->has_indicator_source != 0
            && picoui_slider_set_indicator_source(slider, props->indicator_source) != 0)
        || (props->has_indicator_width != 0
            && picoui_slider_set_indicator_width(slider, props->indicator_width) != 0)
        || (props->has_slim_size != 0
            && picoui_slider_set_slim_size(slider, props->slim_size) != 0)) {
        tinyui_slider_dispose_partial(slider);
        return 0;
    }

    /* Under the validated props contract, the remaining widget metadata/style updates
     * do not expose a real failure path for a valid non-window slider backend. */
    slider->cb = props->on_value_changed;
    slider->user_data = props->user_data;
    (void)picoui_widget_set_user_data(&slider->widget, props->user_data);
    if (props->style_class != 0) {
        (void)picoui_widget_set_style_class(&slider->widget, props->style_class);
    }
    if (props->width > 0 || props->height > 0) {
        (void)picoui_widget_set_size(&slider->widget, props->width, props->height);
    }
    (void)picoui_widget_set_bg_color(&slider->widget, props->bg_color);
    (void)picoui_widget_set_text_color(&slider->widget, props->text_color);
    (void)picoui_widget_set_border_color(&slider->widget, props->border_color);
    (void)picoui_widget_set_radius(&slider->widget, props->radius);
    (void)picoui_widget_set_padding(&slider->widget, props->padding);
    return slider;
}

/**
 * @brief Set value of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int picoui_slider_set_value(struct picoui_slider *slider, int value)
{
    if (slider == 0 || value < slider->min_value || value > slider->max_value) {
        return -1;
    }

    if (slider->value == value) {
        return 0;
    }

    if (slider->widget.backend_widget == 0) {
        return -1;
    }

    slider->value = value;
    return tinyui_widget_update_value(slider->widget.backend_widget,
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

int picoui_slider_set_range(struct picoui_slider *slider, int min_value, int max_value)
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
    if (slider->widget.backend_widget != 0) {
        return tinyui_widget_update_value(slider->widget.backend_widget,
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

int picoui_slider_set_percent(struct picoui_slider *slider, int percent)
{
    int value;

    if (slider == 0 || percent < 0 || percent > 100) {
        return -1;
    }

    value = slider->min_value + ((slider->max_value - slider->min_value) * percent) / 100;
    return picoui_slider_set_value(slider, value);
}

/**
 * @brief Set horizontal of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] horizontal horizontal
 * @return -1 on failure
 */

int picoui_slider_set_horizontal(struct picoui_slider *slider, int horizontal)
{
    ldSlider_t *ld_slider;

    if (slider == 0) {
        return -1;
    }

    ld_slider = tinyui_slider_get_ld(slider);
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

int picoui_slider_get_horizontal(struct picoui_slider *slider, int *horizontal)
{
    ldSlider_t *ld_slider;

    if (slider == 0 || horizontal == 0) {
        return -1;
    }

    ld_slider = tinyui_slider_get_ld(slider);
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

int picoui_slider_set_background_source(struct picoui_slider *slider,
                                        struct picoui_image_source *source)
{
    ldSlider_t *ld_slider;

    if (slider == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    ld_slider = tinyui_slider_get_ld(slider);
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

int picoui_slider_set_indicator_source(struct picoui_slider *slider,
                                       struct picoui_image_source *source)
{
    ldSlider_t *ld_slider;

    if (slider == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    ld_slider = tinyui_slider_get_ld(slider);
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

int picoui_slider_set_image(struct picoui_slider *slider,
                            struct picoui_image_source *background_source,
                            struct picoui_image_source *indicator_source)
{
    if (picoui_slider_set_background_source(slider, background_source) != 0) {
        return -1;
    }
    return picoui_slider_set_indicator_source(slider, indicator_source);
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

int picoui_slider_set_color(struct picoui_slider *slider,
                            unsigned int bg_color,
                            unsigned int frame_color,
                            unsigned int indicator_color)
{
    struct picoui_backend_widget *backend;
    ldSlider_t *ld_slider;

    if (slider == 0 || bg_color > 0xFFFFFFU || frame_color > 0xFFFFFFU ||
        indicator_color > 0xFFFFFFU || slider->widget.backend_widget == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    ld_slider = (ldSlider_t *)backend->ld_widget;
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

int picoui_slider_set_indicator_width(struct picoui_slider *slider, int indicator_width)
{
    ldSlider_t *ld_slider;

    if (slider == 0 || indicator_width < 0) {
        return -1;
    }

    ld_slider = tinyui_slider_get_ld(slider);
    if (ld_slider == 0 || indicator_width > 255) {
        return -1;
    }
    if (tinyui_slider_should_fail_indicator_width(slider)) {
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

int picoui_slider_set_slim_size(struct picoui_slider *slider, int slim_size)
{
    ldSlider_t *ld_slider;

    if (slider == 0 || slim_size < 0) {
        return -1;
    }

    ld_slider = tinyui_slider_get_ld(slider);
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

int picoui_slider_get_percent(struct picoui_slider *slider, int *percent)
{
    ldSlider_t *ld_slider;

    if (slider == 0 || percent == 0) {
        return -1;
    }

    ld_slider = tinyui_slider_get_ld(slider);
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

int picoui_slider_set_on_value_changed(struct picoui_slider *slider,
                                       picoui_value_changed_cb cb,
                                       void *user_data)
{
    if (slider == 0) {
        return -1;
    }

    slider->cb = cb;
    slider->user_data = user_data;
    return 0;
}
