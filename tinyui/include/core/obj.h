#ifndef TINYUI_OBJ_H
#define TINYUI_OBJ_H

#include "core/result.h"

#include <stddef.h>
#include <stdint.h>

/* Legacy implementation headers may arrive first inside the private build.
 * Standalone canonical includes remain opaque; the compatibility branch only
 * preserves the old internal typedef while M4 removes those headers. */
#ifndef TINYUI_OBJ_T_DEFINED
typedef struct tinyui_obj tinyui_obj_t;
#define TINYUI_OBJ_T_DEFINED
#endif

tinyui_result_t tinyui_obj_delete(tinyui_obj_t *obj);
tinyui_result_t tinyui_obj_get_id(const tinyui_obj_t *obj, uint16_t *id);
tinyui_obj_t *tinyui_obj_find_by_id(tinyui_obj_t *root, uint16_t id);
tinyui_obj_t *tinyui_obj_get_parent(const tinyui_obj_t *obj);
tinyui_obj_t *tinyui_obj_get_first_child(const tinyui_obj_t *obj);
tinyui_obj_t *tinyui_obj_get_next_sibling(const tinyui_obj_t *obj);
tinyui_obj_t *tinyui_obj_get_root(const tinyui_obj_t *obj);
tinyui_result_t tinyui_obj_get_child_count(const tinyui_obj_t *obj, uint16_t *count);

/* Common property setters: static kind→adapter, real LD or NOT_SUPPORTED. */
tinyui_result_t tinyui_obj_set_pos(tinyui_obj_t *obj, int x, int y);
tinyui_result_t tinyui_obj_set_size(tinyui_obj_t *obj, int width, int height);
tinyui_result_t tinyui_obj_set_text(tinyui_obj_t *obj, const char *text);
tinyui_result_t tinyui_obj_set_bg_color(tinyui_obj_t *obj, unsigned int rgb);
tinyui_result_t tinyui_obj_set_text_color(tinyui_obj_t *obj, unsigned int rgb);
tinyui_result_t tinyui_obj_set_border_color(tinyui_obj_t *obj, unsigned int rgb);
tinyui_result_t tinyui_obj_set_border_width(tinyui_obj_t *obj, int width);
tinyui_result_t tinyui_obj_set_radius(tinyui_obj_t *obj, int radius);
tinyui_result_t tinyui_obj_set_padding(tinyui_obj_t *obj, int padding);
tinyui_result_t tinyui_obj_set_visible(tinyui_obj_t *obj, int visible);
tinyui_result_t tinyui_obj_set_enabled(tinyui_obj_t *obj, int enabled);
tinyui_result_t tinyui_obj_set_opacity(tinyui_obj_t *obj, int opacity);
tinyui_result_t tinyui_obj_set_selectable(tinyui_obj_t *obj, int selectable);
tinyui_result_t tinyui_obj_set_selected(tinyui_obj_t *obj, int selected);

#endif
