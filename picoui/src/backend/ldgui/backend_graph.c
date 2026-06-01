#include "backend.h"
#include "internal.h"
#include "ldGraph.h"

#include <stdlib.h>

extern const arm_2d_tile_t c_tileWhiteDotMask;

static struct picoui_backend_app_state *picoui_backend_graph_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldGraph_t *picoui_backend_graph_get_ld(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldGraph_t *)widget->ld_widget;
}

void *picoui_backend_create_graph(void *parent, const char *id, int series_max)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldGraph_t *ld_graph;
    uint16_t name_id;

    if (parent == 0 || id == 0 || series_max <= 0 || series_max > PICOUI_GRAPH_MAX_SERIES) {
        return 0;
    }

    app_state = picoui_backend_graph_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_graph = ldGraph_init(app_state->ld_scene,
                            NULL,
                            name_id,
                            parent_widget->ld_name_id,
                            0,
                            0,
                            240,
                            120,
                            (uint8_t)series_max);
    if (ld_graph == NULL) {
        free(widget);
        return 0;
    }

    ldGraphSetFrameSpace(ld_graph, 8, false);
    ldGraphSetGridOffset(ld_graph, 20);
    ldGraphSetAxis(ld_graph, 100, 100, 5);
    ldGraphSetPointImageMask(ld_graph, (arm_2d_tile_t *)&c_tileWhiteDotMask);

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_GRAPH;
    widget->theme = parent_widget->theme;
    widget->ld_widget = ld_graph;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_graph_set_axis(void *backend_widget, int x_axis, int y_axis)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || x_axis <= 0 || y_axis <= 0) {
        return -1;
    }

    ldGraphSetAxis(ld_graph, (uint16_t)x_axis, (uint16_t)y_axis, ld_graph->xAxisOffset);
    return 0;
}

int picoui_backend_graph_set_axis_offset(void *backend_widget, int axis_offset)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || axis_offset < 0) {
        return -1;
    }

    ldGraphSetAxisOffset(ld_graph, (uint16_t)axis_offset);
    return 0;
}

int picoui_backend_graph_set_frame_space(void *backend_widget, int frame_space)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || frame_space < 0) {
        return -1;
    }

    ldGraphSetFrameSpace(ld_graph, (uint8_t)frame_space, false);
    return 0;
}

int picoui_backend_graph_set_grid_offset(void *backend_widget, int grid_offset)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || grid_offset <= 0) {
        return -1;
    }

    ldGraphSetGridOffset(ld_graph, (uint8_t)grid_offset);
    return 0;
}

int picoui_backend_graph_set_point_mask_source(void *backend_widget, struct picoui_image_source *source)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || source == NULL || source->mask_tile == NULL) {
        return -1;
    }

    ldGraphSetPointImageMask(ld_graph, source->mask_tile);
    return 0;
}

int picoui_backend_graph_add_series(void *backend_widget,
                                    unsigned int series_color,
                                    int line_size,
                                    int point_max)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || line_size < 0 || point_max <= 0 || point_max > PICOUI_GRAPH_MAX_POINTS) {
        return -1;
    }

    return (int)ldGraphAddSeries(ld_graph, (ldColor)series_color, (uint8_t)line_size, (uint16_t)point_max);
}

int picoui_backend_graph_set_value(void *backend_widget, int series_index, int value_index, int value)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || series_index < 0 || series_index >= ld_graph->seriesCount ||
        value_index < 0 || value_index >= ld_graph->pSeries[series_index].valueCountMax ||
        value < 0) {
        return -1;
    }

    ldGraphSetValue(ld_graph, (uint8_t)series_index, (uint16_t)value_index, (uint16_t)value);
    return 0;
}

int picoui_backend_graph_move_add(void *backend_widget, int series_index, int value)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || series_index < 0 || series_index >= ld_graph->seriesCount || value < 0) {
        return -1;
    }

    ldGraphMoveAdd(ld_graph, (uint8_t)series_index, (uint16_t)value);
    return 0;
}

int picoui_backend_graph_get_series_count(void *backend_widget)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL) {
        return -1;
    }

    return (int)ld_graph->seriesCount;
}

int picoui_backend_graph_get_value(void *backend_widget, int series_index, int value_index)
{
    ldGraph_t *ld_graph = picoui_backend_graph_get_ld(backend_widget);

    if (ld_graph == NULL || series_index < 0 || series_index >= ld_graph->seriesCount ||
        value_index < 0 || value_index >= ld_graph->pSeries[series_index].valueCountMax) {
        return -1;
    }

    return (int)ld_graph->pSeries[series_index].pValueList[value_index];
}
