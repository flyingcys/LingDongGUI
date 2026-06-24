#ifndef TINYUI_LDGUI_PORT_H
#define TINYUI_LDGUI_PORT_H

struct tinyui_app;
struct ld_scene_t;

/* 设置/获取当前活跃的 tinyui_app（ARM-2D 回调中使用） */
void ldgui_port_set_current_app(struct tinyui_app *app);
struct tinyui_app *ldgui_port_get_current_app(void);

/* scene→app 注册表：支持多 app 并发（测试场景） */
void ldgui_port_register_scene_app(struct ld_scene_t *scene, struct tinyui_app *app);
void ldgui_port_unregister_scene_app(struct ld_scene_t *scene);
struct tinyui_app *ldgui_port_get_app_for_scene(const struct ld_scene_t *scene);

#endif /* TINYUI_LDGUI_PORT_H */
