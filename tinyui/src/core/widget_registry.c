#include <stdlib.h>
#include "internal.h"
#include "../drivers/tinyui_ldgui_port.h"
#include "../../../src/gui/ldBase.h"

void tinyui_app_register_host(struct tinyui_app *app, struct tinyui_widget *w)
{
    if (app == 0 || w == 0) {
        return;
    }
    w->reg_prev = 0;
    w->reg_next = app->host_list_head;
    if (app->host_list_head != 0) {
        app->host_list_head->reg_prev = w;
    }
    app->host_list_head = w;
}

void tinyui_app_unregister_host(struct tinyui_app *app, struct tinyui_widget *w)
{
    if (app == 0 || w == 0) {
        return;
    }
    if (w->reg_prev != 0) {
        w->reg_prev->reg_next = w->reg_next;
    } else if (app->host_list_head == w) {
        app->host_list_head = w->reg_next;
    }
    if (w->reg_next != 0) {
        w->reg_next->reg_prev = w->reg_prev;
    }
    w->reg_prev = 0;
    w->reg_next = 0;
}

uint16_t tinyui_app_alloc_name_id(struct tinyui_app *app)
{
    if (app == 0) {
        return 0;
    }
    if (app->free_name_id_count > 0) {
        return app->free_name_ids[--app->free_name_id_count];
    }
    return ++app->next_ld_name_id;
}

void tinyui_app_free_name_id(struct tinyui_app *app, uint16_t id)
{
    uint16_t *p;
    uint16_t new_cap;

    if (app == 0 || id == 0) {
        return;
    }
    if (app->free_name_id_count >= app->free_name_id_cap) {
        new_cap = (app->free_name_id_cap == 0) ? 8 : (uint16_t)(app->free_name_id_cap * 2);
        p = realloc(app->free_name_ids, (size_t)new_cap * sizeof(uint16_t));
        if (p == 0) {
            return; /* drop on OOM — id is lost; no crash */
        }
        app->free_name_ids = p;
        app->free_name_id_cap = new_cap;
    }
    app->free_name_ids[app->free_name_id_count++] = id;
}

struct tinyui_widget *tinyui_app_lookup_host(const struct tinyui_app *app, uint16_t name_id)
{
    struct tinyui_widget *cur;
    if (app == 0) {
        return 0;
    }
    for (cur = app->host_list_head; cur != 0; cur = cur->reg_next) {
        if (cur->ld_name_id == name_id) {
            return cur;
        }
    }
    return 0;
}

struct tinyui_widget *tinyui_widget_from_ld(const void *ld_node)
{
    struct tinyui_app *app;
    if (ld_node == 0) {
        return 0;
    }
    app = ldgui_port_get_current_app();
    return tinyui_app_lookup_host(app, ((const ldBase_t *)ld_node)->nameId);
}

struct tinyui_widget *tinyui_widget_from_ld_scene(const struct ld_scene_t *scene,
                                                   const void *ld_node)
{
    struct tinyui_app *app;
    if (scene == 0 || ld_node == 0) {
        return 0;
    }
    app = ldgui_port_get_app_for_scene(scene);
    return tinyui_app_lookup_host(app, ((const ldBase_t *)ld_node)->nameId);
}
