#ifndef TINYUI_LDGUI_PORT_H
#define TINYUI_LDGUI_PORT_H

struct tinyui_app;

/* 设置/获取当前活跃的 tinyui_app（ARM-2D 回调中使用） */
void ldgui_port_set_current_app(struct tinyui_app *app);
struct tinyui_app *ldgui_port_get_current_app(void);

#endif /* TINYUI_LDGUI_PORT_H */
