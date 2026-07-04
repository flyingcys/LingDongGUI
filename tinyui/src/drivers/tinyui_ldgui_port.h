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

/* Backend-neutral (no-SDL) frame loop — implemented in
 * tinyui_ldgui_neutral_runtime.c.  core 的 runtime_bridge 直接驱动它;平台
 * 通过 display flush/present + indev read_cb + tick 注册能力接入,无需定义
 * 任何 host 帧步进符号(对齐 LVGL:循环归 core,平台只注册能力)。 */
int  tinyui_backend_neutral_step(struct tinyui_app *app);
void tinyui_backend_neutral_shutdown(struct tinyui_app *app);

#endif /* TINYUI_LDGUI_PORT_H */
