#ifndef TINYUI_OBJ_H
#define TINYUI_OBJ_H

#include "core/result.h"

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

#endif
