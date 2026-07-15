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
#include "widgets/slider.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldSlider.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


static struct tinyui_slider *tinyui_slider_as_slider(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_SLIDER)) {
        return 0;
    }
    return (struct tinyui_slider *)w;
}

static const struct tinyui_slider *tinyui_slider_as_slider_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_SLIDER)) {
        return 0;
    }
    return (const struct tinyui_slider *)w;
}

static const char *s_fail_test_id = 0;


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

static int tinyui_slider_props_are_valid(const tinyui_slider_props_t *props)
{
    return props != 0
        && props->min_value <= props->max_value
        && props->value >= props->min_value
        && props->value <= props->max_value
        && (props->background_source == 0
            || tinyui_image_source_get_image_tile(props->background_source) != 0)
        && (props->indicator_source == 0
            || tinyui_image_source_get_image_tile(props->indicator_source) != 0)
        && (props->indicator_width == -1
            || (props->indicator_width >= 0 && props->indicator_width <= 255))
        && (props->slim_size == -1
            || (props->slim_size >= 0 && props->slim_size <= 255))
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

static void *tinyui_runtime_internal_slider_ld_init(void *ctx,
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

tinyui_obj_t *tinyui_slider_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "slider";
    if (parent_w == 0) { return 0; }

    struct tinyui_slider *slider;

    if (parent_w == 0 || id == 0) {
        return 0;
    }
    slider = (struct tinyui_slider *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                               TINYUI_BACKEND_WIDGET_SLIDER,
                                                               tinyui_runtime_internal_slider_ld_init,
                                                               0,
                                                               sizeof(*slider));
    if (slider == 0) {
        return 0;
    }
    slider->id = id;
    slider->min_value = 0;
    slider->max_value = 100;

    return (tinyui_obj_t *)slider;
}

/**
 * @brief Create slider widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_slider_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_slider_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_slider *slider;

    if (props == 0) {
        return tinyui_slider_create(parent);
    }

    obj = tinyui_slider_create(parent);
    if (obj == 0) {
        return 0;
    }
    slider = (struct tinyui_slider *)(void *)obj;

    if ((props->fields & TINYUI_SLIDER_FIELD_ID) != 0) {
        /* id=0 means runtime auto-alloc; non-zero reserved for host name_id path. */
        (void)props->id;
    }
    if ((props->fields & TINYUI_SLIDER_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&slider->widget, props->user_data) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)slider);
        return 0;
    }
    }
    if ((props->fields & TINYUI_SLIDER_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&slider->widget, props->style_class) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)slider);
        return 0;
    }
    }
        if ((props->fields & TINYUI_SLIDER_FIELD_WIDTH) != 0 || (props->fields & TINYUI_SLIDER_FIELD_HEIGHT) != 0) {
        int w = tinyui_runtime_internal_widget_get_width(&slider->widget);
        int h = tinyui_runtime_internal_widget_get_height(&slider->widget);
        if (w < 0) {
            w = 0;
        }
        if (h < 0) {
            h = 0;
        }
        if ((props->fields & TINYUI_SLIDER_FIELD_WIDTH) != 0) {
            w = props->width;
        }
        if ((props->fields & TINYUI_SLIDER_FIELD_HEIGHT) != 0) {
            h = props->height;
        }
        if (tinyui_runtime_internal_widget_set_size(&slider->widget, w, h) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)slider);
            return 0;
        }
    }
    if ((props->fields & TINYUI_SLIDER_FIELD_BG_COLOR) != 0) {
    if (tinyui_runtime_internal_widget_set_bg_color(&slider->widget, props->bg_color) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)slider);
        return 0;
    }
    }
    if ((props->fields & TINYUI_SLIDER_FIELD_TEXT_COLOR) != 0) {
    if (tinyui_runtime_internal_widget_set_text_color(&slider->widget, props->text_color) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)slider);
        return 0;
    }
    }
    if ((props->fields & TINYUI_SLIDER_FIELD_BORDER_COLOR) != 0) {
    if (tinyui_runtime_internal_widget_set_border_color(&slider->widget, props->border_color) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)slider);
        return 0;
    }
    }
    if ((props->fields & TINYUI_SLIDER_FIELD_RADIUS) != 0) {
    if (tinyui_runtime_internal_widget_set_radius(&slider->widget, props->radius) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)slider);
        return 0;
    }
    }
    if ((props->fields & TINYUI_SLIDER_FIELD_PADDING) != 0) {
    if (tinyui_runtime_internal_widget_set_padding(&slider->widget, props->padding) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)slider);
        return 0;
    }
    }
    if ((props->fields & TINYUI_SLIDER_FIELD_MIN_VALUE) != 0 ||
        (props->fields & TINYUI_SLIDER_FIELD_MAX_VALUE) != 0) {
        int min_v = props->min_value;
        int max_v = props->max_value;
        if ((props->fields & TINYUI_SLIDER_FIELD_MIN_VALUE) == 0) {
            min_v = 0;
        }
        if ((props->fields & TINYUI_SLIDER_FIELD_MAX_VALUE) == 0) {
            max_v = 100;
        }
        if (tinyui_slider_set_range((tinyui_obj_t *)slider, min_v, max_v) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)slider);
            return 0;
        }
    }
    if ((props->fields & TINYUI_SLIDER_FIELD_VALUE) != 0) {
    if (tinyui_slider_set_value((tinyui_obj_t *)slider, props->value) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)slider);
        return 0;
    }
    }
    if ((props->fields & TINYUI_SLIDER_FIELD_HORIZONTAL) != 0) {
    if (tinyui_slider_set_horizontal((tinyui_obj_t *)slider, props->horizontal) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)slider);
        return 0;
    }
    }
    if ((props->fields & TINYUI_SLIDER_FIELD_BACKGROUND_SOURCE) != 0) {
    if (tinyui_slider_set_background_source((tinyui_obj_t *)slider, props->background_source) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)slider);
        return 0;
    }
    }
    if ((props->fields & TINYUI_SLIDER_FIELD_INDICATOR_SOURCE) != 0) {
    if (tinyui_slider_set_indicator_source((tinyui_obj_t *)slider, props->indicator_source) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)slider);
        return 0;
    }
    }
    if ((props->fields & TINYUI_SLIDER_FIELD_INDICATOR_WIDTH) != 0) {
    if (tinyui_slider_set_indicator_width((tinyui_obj_t *)slider, props->indicator_width) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)slider);
        return 0;
    }
    }
    if ((props->fields & TINYUI_SLIDER_FIELD_SLIM_SIZE) != 0) {
    if (tinyui_slider_set_slim_size((tinyui_obj_t *)slider, props->slim_size) != 0) {
        (void)tinyui_obj_delete((tinyui_obj_t *)slider);
        return 0;
    }
    }
    if ((props->fields & TINYUI_SLIDER_FIELD_ON_VALUE_CHANGED) != 0) {
        /* props still carry legacy tinyui_value_changed_cb; Task 7 set_on_* is
         * unified-pool only. Reject rather than fake-success. Prefer
         * tinyui_obj_add_event_cb after create. */
        if (props->on_value_changed != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)slider);
            return 0;
        }
    }

    return obj;
}

static int tinyui_slider_value_to_percent(const struct tinyui_slider *slider, int value)
{
    int64_t range;

    if (slider == 0) {
        return 0;
    }
    range = (int64_t)slider->max_value - (int64_t)slider->min_value;
    if (range <= 0) {
        return 0;
    }
    return (int)((((int64_t)value - (int64_t)slider->min_value) * 100) / range);
}

static int tinyui_slider_commit_value(struct tinyui_slider *slider, int value)
{
    ldSlider_t *ld_slider;
    int percent;

    if (slider == 0) {
        return -1;
    }
    ld_slider = tinyui_slider_backend(slider);
    if (ld_slider == 0) {
        return -1;
    }

    percent = tinyui_slider_value_to_percent(slider, value);
    ldSliderSetPercent(ld_slider, (float)percent);
    slider->value = value;
    slider->widget.value = value;
    return 0;
}


/**
 * @brief Set value of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int tinyui_slider_set_value(tinyui_obj_t *slider_obj, int value)
{
    struct tinyui_slider *slider = tinyui_slider_as_slider(slider_obj);

    if (slider == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return -1;
    }

    if (value < slider->min_value || value > slider->max_value) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_OUT_OF_RANGE);
        return -1;
    }

    if (slider->value == value && slider->widget.value == value) {
        tinyui_runtime_set_last_result(TINYUI_OK);
        return 0;
    }

    if (tinyui_slider_commit_value(slider, value) != 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_BACKEND);
        return -1;
    }
    tinyui_runtime_set_last_result(TINYUI_OK);
    return 0;
}

/**
 * @brief Set range of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] min_value min value
 * @param[in] max_value max value
 * @return 0 on success, -1 on failure
 */

int tinyui_slider_set_range(tinyui_obj_t *slider_obj, int min_value, int max_value)
{
    struct tinyui_slider *slider = tinyui_slider_as_slider(slider_obj);
    int64_t old_range;
    int old_percent = 0;
    int64_t new_range;
    int remapped_value;

    if (slider == 0 || min_value > max_value) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return -1;
    }

    old_range = (int64_t)slider->max_value - (int64_t)slider->min_value;
    if (old_range > 0) {
        old_percent = (int)((((int64_t)slider->value - (int64_t)slider->min_value) * 100)
                            / old_range);
        if (old_percent < 0) {
            old_percent = 0;
        }
        if (old_percent > 100) {
            old_percent = 100;
        }
    }

    /* Commit min/max first so percent mapping uses the new range. */
    slider->min_value = min_value;
    slider->max_value = max_value;
    new_range = (int64_t)max_value - (int64_t)min_value;
    if (new_range <= 0) {
        remapped_value = min_value;
    } else {
        remapped_value = min_value + (int)((new_range * (int64_t)old_percent) / 100);
    }

    if (tinyui_slider_commit_value(slider, remapped_value) != 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_BACKEND);
        return -1;
    }
    tinyui_runtime_set_last_result(TINYUI_OK);
    return 0;
}

/**
 * @brief Set percent of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] percent percent
 * @return -1 on failure
 */

int tinyui_slider_set_percent(tinyui_obj_t *slider_obj, int percent)
{
    struct tinyui_slider *slider = tinyui_slider_as_slider(slider_obj);
    int64_t range;
    int value;

    if (slider == 0) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_INVALID_ARG);
        return -1;
    }
    if (percent < 0 || percent > 100) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_OUT_OF_RANGE);
        return -1;
    }

    range = (int64_t)slider->max_value - (int64_t)slider->min_value;
    if (range <= 0) {
        value = slider->min_value;
    } else {
        value = slider->min_value + (int)((range * (int64_t)percent) / 100);
    }
    return tinyui_slider_set_value((tinyui_obj_t *)slider, value);
}

/**
 * @brief Set horizontal of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] horizontal horizontal
 * @return -1 on failure
 */

int tinyui_slider_set_horizontal(tinyui_obj_t *slider_obj, int horizontal)
{
    struct tinyui_slider *slider = tinyui_slider_as_slider(slider_obj);
    if (slider == 0) { return -1; }

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

int tinyui_slider_get_horizontal(tinyui_obj_t *slider_obj, int *horizontal)
{
    struct tinyui_slider *slider = tinyui_slider_as_slider(slider_obj);
    if (slider == 0) { return -1; }

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

int tinyui_slider_set_background_source(tinyui_obj_t *slider_obj, struct tinyui_image_source *source)
{
    struct tinyui_slider *slider = tinyui_slider_as_slider(slider_obj);
    if (slider == 0) { return -1; }

    ldSlider_t *ld_slider;

    if (slider == 0 || (source != 0 && tinyui_image_source_get_image_tile(source) == 0)) {
        return -1;
    }

    ld_slider = tinyui_slider_backend(slider);
    if (ld_slider == 0) {
        return -1;
    }

    ldSliderSetImage(ld_slider,
                     source != 0 ? tinyui_image_source_get_image_tile(source) : 0,
                     source != 0 ? tinyui_image_source_get_mask_tile(source) : 0,
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

int tinyui_slider_set_indicator_source(tinyui_obj_t *slider_obj, struct tinyui_image_source *source)
{
    struct tinyui_slider *slider = tinyui_slider_as_slider(slider_obj);
    if (slider == 0) { return -1; }

    ldSlider_t *ld_slider;
    arm_2d_tile_t *indicator_tile;

    if (slider == 0 || (source != 0 && tinyui_image_source_get_image_tile(source) == 0)) {
        return -1;
    }
    if (source != 0) {
        indicator_tile = (arm_2d_tile_t *)tinyui_image_source_get_image_tile(source);
        if (indicator_tile->tRegion.tSize.iWidth < 0 ||
            indicator_tile->tRegion.tSize.iWidth > 255) {
            return -1;
        }
    }

    ld_slider = tinyui_slider_backend(slider);
    if (ld_slider == 0) {
        return -1;
    }

    ldSliderSetImage(ld_slider,
                     ld_slider->ptBgImgTile,
                     ld_slider->ptBgMaskTile,
                     source != 0 ? tinyui_image_source_get_image_tile(source) : 0,
                     source != 0 ? tinyui_image_source_get_mask_tile(source) : 0);
    if (source != 0) {
        ldSliderSetIndicatorWidth(ld_slider, (uint8_t)indicator_tile->tRegion.tSize.iWidth);
    }
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

int tinyui_slider_set_image(tinyui_obj_t *slider_obj, struct tinyui_image_source *background_source, struct tinyui_image_source *indicator_source)
{
    struct tinyui_slider *slider = tinyui_slider_as_slider(slider_obj);
    if (slider == 0) { return -1; }

    if (tinyui_slider_set_background_source((tinyui_obj_t *)slider, background_source) != 0) {
        return -1;
    }
    return tinyui_slider_set_indicator_source((tinyui_obj_t *)slider, indicator_source);
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

int tinyui_slider_set_color(tinyui_obj_t *slider_obj, unsigned int bg_color, unsigned int frame_color, unsigned int indicator_color)
{
    struct tinyui_slider *slider = tinyui_slider_as_slider(slider_obj);
    if (slider == 0) { return -1; }

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

int tinyui_slider_set_indicator_width(tinyui_obj_t *slider_obj, int indicator_width)
{
    struct tinyui_slider *slider = tinyui_slider_as_slider(slider_obj);
    if (slider == 0) { return -1; }

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

int tinyui_slider_set_slim_size(tinyui_obj_t *slider_obj, int slim_size)
{
    struct tinyui_slider *slider = tinyui_slider_as_slider(slider_obj);
    if (slider == 0) { return -1; }

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

int tinyui_slider_get_percent(tinyui_obj_t *slider_obj, int *percent)
{
    struct tinyui_slider *slider = tinyui_slider_as_slider(slider_obj);
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

static int tinyui_slider_replace_event_cb(struct tinyui_slider *slider,
                                          tinyui_event_handle_t *slot,
                                          uint32_t event_mask,
                                          tinyui_event_cb_t cb,
                                          void *user_data)
{
    tinyui_event_handle_t handle = 0U;
    tinyui_result_t rc;

    if (slider == 0 || slot == 0) {
        return -1;
    }

    if (*slot != 0U) {
        (void)tinyui_obj_remove_event_cb((tinyui_obj_t *)slider, *slot);
        *slot = 0U;
    }

    if (cb == 0) {
        return 0;
    }

    rc = tinyui_obj_add_event_cb((tinyui_obj_t *)slider,
                                 event_mask,
                                 cb,
                                 user_data,
                                 &handle);
    if (rc != TINYUI_OK) {
        return -1;
    }
    *slot = handle;
    return 0;
}

/**
 * @brief Set on value changed of slider widget (narrow forward to unified event pool)
 *
 * @param[in] slider Slider widget instance
 * @param[in] cb Unified event callback (const tinyui_event_t *)
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int tinyui_slider_set_on_value_changed(tinyui_obj_t *slider_obj, tinyui_event_cb_t cb, void *user_data)
{
    struct tinyui_slider *slider = tinyui_slider_as_slider(slider_obj);
    if (slider == 0) {
        return -1;
    }

    return tinyui_slider_replace_event_cb(slider,
                                          &slider->on_value_changed_handle,
                                          TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED),
                                          cb,
                                          user_data);
}
