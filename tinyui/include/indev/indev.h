#ifndef TINYUI_INDEV_H
#define TINYUI_INDEV_H

struct tinyui_app;

enum tinyui_input_key {
    TINYUI_INPUT_KEY_NONE = 0,
    TINYUI_INPUT_KEY_LEFT,
    TINYUI_INPUT_KEY_RIGHT,
    TINYUI_INPUT_KEY_UP,
    TINYUI_INPUT_KEY_DOWN,
    TINYUI_INPUT_KEY_ENTER,
    TINYUI_INPUT_KEY_BACK,
};

/* core 循环每帧在处理定时器前调用一次;平台在此采集输入
 * (SDL: 泵事件后 tinyui_input_push_pointer)。返回值:>0 请求退出,
 * 0 继续,<0 错误。未注册则 core 跳过采集(app 自行 push)。 */
typedef int (*tinyui_input_read_cb_t)(struct tinyui_app *app, void *user_data);
int tinyui_input_set_read_callback(struct tinyui_app *app,
                                   tinyui_input_read_cb_t callback,
                                   void *user_data);

int tinyui_input_push_pointer(struct tinyui_app *app, int x, int y, int pressed);
int tinyui_input_get_pointer(const struct tinyui_app *app, int *x, int *y, int *pressed);
int tinyui_input_push_key(struct tinyui_app *app, enum tinyui_input_key key, int pressed);
int tinyui_input_get_key(const struct tinyui_app *app,
                         enum tinyui_input_key *key,
                         int *pressed);

#endif
