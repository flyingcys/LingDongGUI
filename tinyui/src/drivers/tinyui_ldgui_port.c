/*
 * tinyui_ldgui_port.c
 *
 * 将 ARM-2D / LingDongGUI 期望的外部符号路由到 tinyui_app 的 port state。
 * 用户代码不直接接触 ldgui / arm_2d 头文件。
 *
 * 符号对应关系：
 *   Disp0_DrawBitmap        → display_port.flush_callback
 *   VT_enter/leave_global_mutex → os_port.enter / os_port.leave
 *   arm_2d_helper_get_system_timestamp → tick_port.callback (ms→us)
 *   ldCfgTouchGetPoint      → input_port (read)
 *   ldCfgTouchSetPoint      → tinyui_input_push_pointer (write)
 */

#include "tinyui_ldgui_port.h"
#include "tinyui_ldgui_port_config.h"
#include "internal.h"

#include "display/display.h"   /* tinyui_display_get_config, tinyui_area, tinyui_display_config */
#include "indev/indev.h"     /* tinyui_input_get_pointer, tinyui_input_push_pointer */
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldKeyboard.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#if defined(_POSIX_VERSION) || defined(CLOCK_REALTIME) || defined(__APPLE__)
#include <time.h>
#endif

/* ─── 全局 app 指针 ─── */
static struct tinyui_app *s_ldgui_current_app;

void ldgui_port_set_current_app(struct tinyui_app *app)
{
    s_ldgui_current_app = app;
}

struct tinyui_app *ldgui_port_get_current_app(void)
{
    return s_ldgui_current_app;
}

/* ─── scene→app 注册表（固定大小，足够测试场景使用）─── */
#define LDGUI_PORT_SCENE_REGISTRY_SIZE 16

static struct {
    struct ld_scene_t *scene;
    struct tinyui_app *app;
} s_scene_app_registry[LDGUI_PORT_SCENE_REGISTRY_SIZE];

void ldgui_port_register_scene_app(struct ld_scene_t *scene, struct tinyui_app *app)
{
    int i;
    if (scene == NULL || app == NULL) {
        return;
    }
    for (i = 0; i < LDGUI_PORT_SCENE_REGISTRY_SIZE; i++) {
        if (s_scene_app_registry[i].scene == scene) {
            s_scene_app_registry[i].app = app;
            return;
        }
    }
    for (i = 0; i < LDGUI_PORT_SCENE_REGISTRY_SIZE; i++) {
        if (s_scene_app_registry[i].scene == NULL) {
            s_scene_app_registry[i].scene = scene;
            s_scene_app_registry[i].app = app;
            return;
        }
    }
}

void ldgui_port_unregister_scene_app(struct ld_scene_t *scene)
{
    int i;
    if (scene == NULL) {
        return;
    }
    for (i = 0; i < LDGUI_PORT_SCENE_REGISTRY_SIZE; i++) {
        if (s_scene_app_registry[i].scene == scene) {
            s_scene_app_registry[i].scene = NULL;
            s_scene_app_registry[i].app = NULL;
            return;
        }
    }
}

struct tinyui_app *ldgui_port_get_app_for_scene(const struct ld_scene_t *scene)
{
    int i;
    if (scene == NULL) {
        return NULL;
    }
    for (i = 0; i < LDGUI_PORT_SCENE_REGISTRY_SIZE; i++) {
        if (s_scene_app_registry[i].scene == scene) {
            return s_scene_app_registry[i].app;
        }
    }
    return NULL;
}

/* ─── ① LCD flush → tinyui display_port.flush_callback ─── */
int32_t Disp0_DrawBitmap(int16_t x, int16_t y, int16_t width, int16_t height,
                         const uint8_t *bitmap)
{
    struct tinyui_app *app = s_ldgui_current_app;
    if (app == NULL || app->display_port.flush_callback == NULL) {
        return 0;
    }

    struct tinyui_area area;
    area.x      = (int)x;
    area.y      = (int)y;
    area.width  = (int)width;
    area.height = (int)height;

    app->display_port.flush_callback(&area, (const void *)bitmap,
                                     app->display_port.flush_user_data);
    return 0;
}

/* ─── ② mutex → tinyui os_port.enter / os_port.leave ─── */
void VT_enter_global_mutex(void)
{
    struct tinyui_app *app = s_ldgui_current_app;
    if (app != NULL && app->os_port.enter != NULL) {
        app->os_port.enter(app->os_port.lock_user_data);
    }
}

void VT_leave_global_mutex(void)
{
    struct tinyui_app *app = s_ldgui_current_app;
    if (app != NULL && app->os_port.leave != NULL) {
        app->os_port.leave(app->os_port.lock_user_data);
    }
}

/* ─── ③ tick / host wall clock ─── */
int64_t arm_2d_helper_get_system_timestamp(void)
{
#if defined(_POSIX_VERSION) || defined(CLOCK_REALTIME) || defined(__APPLE__)
    struct timespec timestamp;

    if (clock_gettime(CLOCK_REALTIME, &timestamp) == 0) {
        static int s_tzset_done = 0;
        if (!s_tzset_done) {
            s_tzset_done = 1;
            tzset();
        }
        return (int64_t)(timestamp.tv_sec - timezone) * 1000000LL
             + (int64_t)timestamp.tv_nsec / 1000LL;
    }
#endif

    struct tinyui_app *app = s_ldgui_current_app;
    if (app == NULL || app->tick_port.callback == NULL) {
        return 0;
    }
    return (int64_t)app->tick_port.callback(app->tick_port.user_data) * 1000;
}

static void ldgui_port_get_screen_size_for_app(struct tinyui_app *app, int16_t *width, int16_t *height)
{
    int screen_width = LD_CFG_SCREEN_WIDTH;
    int screen_height = LD_CFG_SCREEN_HEIGHT;

    if (app != NULL) {
        struct tinyui_display_config cfg;
        if (tinyui_display_get_config(app, &cfg) == 0 && cfg.width > 0 && cfg.height > 0) {
            screen_width = cfg.width;
            screen_height = cfg.height;
        }
    }

    if (width != NULL) {
        *width = (int16_t)screen_width;
    }
    if (height != NULL) {
        *height = (int16_t)screen_height;
    }
}

/* ─── ④ touch read → tinyui input_port ─── */
void ldCfgScreenGetSize(int16_t *width, int16_t *height)
{
    ldgui_port_get_screen_size_for_app(s_ldgui_current_app, width, height);
}

void ldCfgScreenGetSizeForScene(struct ld_scene_t *scene, int16_t *width, int16_t *height)
{
    struct tinyui_app *app = ldgui_port_get_app_for_scene(scene);

    if (app == NULL) {
        app = s_ldgui_current_app;
    }
    ldgui_port_get_screen_size_for_app(app, width, height);
}

static ldBase_t *ldgui_port_top_window_for_widget(struct tinyui_widget *widget, ldBase_t *root)
{
    ldBase_t *node;
    ldBase_t *parent;

    if (widget == NULL || widget->ld_widget == NULL || root == NULL) {
        return NULL;
    }
    node = (ldBase_t *)widget->ld_widget;
    while (node != NULL) {
        parent = ldBaseGetParent(node);
        if (parent == root && node->widgetType == widgetTypeWindow) {
            return node;
        }
        node = parent;
    }
    return NULL;
}

static ldBase_t *ldgui_port_top_window_for_keyboard_editor(struct tinyui_app *app, ldBase_t *root)
{
    struct tinyui_widget *host;
    struct tinyui_widget *editor;
    ldKeyboard_t *keyboard;

    if (app == NULL || root == NULL) {
        return NULL;
    }
    for (host = app->host_list_head; host != NULL; host = host->reg_next) {
        if (host->kind != TINYUI_BACKEND_WIDGET_KEYBOARD || host->ld_widget == NULL) {
            continue;
        }
        keyboard = (ldKeyboard_t *)host->ld_widget;
        if (((ldBase_t *)keyboard)->isHidden || keyboard->editorId == 0) {
            continue;
        }
        editor = tinyui_app_lookup_host(app, keyboard->editorId);
        if (editor == NULL) {
            continue;
        }
        return ldgui_port_top_window_for_widget(editor, root);
    }
    return NULL;
}

void ldCfgBgMoveDidUpdate(struct ld_scene_t *scene,
                          int16_t bgWidth,
                          int16_t bgHeight,
                          int16_t offsetX,
                          int16_t offsetY)
{
    struct tinyui_app *app = ldgui_port_get_app_for_scene(scene);
    ldBase_t *root;
    ldBase_t *child;
    ldBase_t *target = NULL;
    int window_count = 0;
    int16_t ext_width;
    int16_t ext_height;

    (void)bgWidth;
    (void)bgHeight;

    if (app == NULL || scene == NULL || scene->ptNodeRoot == NULL) {
        return;
    }

    root = (ldBase_t *)scene->ptNodeRoot;
    ext_width = root->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth;
    ext_height = root->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight;
    if (ext_width <= 0 || ext_height <= 0) {
        return;
    }

    if (app->root_window != NULL && app->root_window->widget.ld_widget != NULL) {
        target = (ldBase_t *)app->root_window->widget.ld_widget;
        if (ldBaseGetParent(target) != root || target->widgetType != widgetTypeWindow) {
            target = NULL;
        }
    }
    if (target == NULL) {
        target = ldgui_port_top_window_for_widget(app->editing_owner, root);
    }
    if (target == NULL) {
        target = ldgui_port_top_window_for_widget(app->focus_owner, root);
    }
    if (target == NULL) {
        target = ldgui_port_top_window_for_keyboard_editor(app, root);
    }
    if (target == NULL) {
        for (child = ldBaseGetChildList(root); child != NULL; child = ldBaseGetNextSibling(child)) {
            if (child->widgetType != widgetTypeWindow) {
                continue;
            }
            target = child;
            window_count++;
        }
        if (window_count != 1) {
            target = NULL;
        }
    }
    if (target != NULL) {
        ldBaseMove(target, offsetX, offsetY);
        ldBaseSetWidth(target, ext_width);
        ldBaseSetHeight(target, ext_height);
    }
}

bool ldCfgTouchGetPoint(int16_t *x, int16_t *y)
{
    struct tinyui_app *app = s_ldgui_current_app;
    if (app == NULL) {
        if (x) *x = -1;
        if (y) *y = -1;
        return false;
    }

    int px = 0, py = 0, pressed = 0;
    tinyui_input_get_pointer(app, &px, &py, &pressed);

    if (pressed) {
        struct tinyui_display_config cfg;
        tinyui_display_get_config(app, &cfg);

        if (px < 0)           px = 0;
        if (py < 0)           py = 0;
        if (px >= cfg.width)  px = cfg.width - 1;
        if (py >= cfg.height) py = cfg.height - 1;

        *x = (int16_t)px;
        *y = (int16_t)py;
        return true;
    }

    if (x) *x = -1;
    if (y) *y = -1;
    return false;
}

/* ─── ④ touch write → tinyui_input_push_pointer ─── */
void ldCfgTouchSetPoint(int16_t x, int16_t y, bool pressed)
{
    struct tinyui_app *app = s_ldgui_current_app;
    if (app != NULL) {
        tinyui_input_push_pointer(app, (int)x, (int)y, pressed ? 1 : 0);
    }
}

/* ─── ARM MCU 工具链桩（仅非 host 平台编译）─── */
#if !defined(__APPLE__) && !defined(__linux__) && !defined(_WIN32)
__attribute__((weak))
void __aeabi_assert(const char *chCond, const char *chLine, int wErrCode)
{
    (void)chCond;
    (void)chLine;
    (void)wErrCode;
    while (1) {}
}
#endif
