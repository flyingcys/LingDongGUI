#ifndef TINYUI_SDL_OBSERVE_H
#define TINYUI_SDL_OBSERVE_H

/*
 * SDL 驱动的测试观测钩子。实现在 tests/tinyui/runtime/tinyui_sdl_observe.c,
 * 仅在 ENABLE_TEST 下编入 tinyui_port_sdl。生产构建(ENABLE_TEST=OFF)下所有
 * 钩子展开为 no-op,hal.c 不引用任何 observe 符号 —— 生产 port 零测试脚手架。
 */

struct tinyui_app;
struct tinyui_runtime_host_state;

#if defined(ENABLE_TEST)

void tinyui_sdl_observe_on_setup(struct tinyui_app *app,
                                 struct tinyui_runtime_host_state *state);
void tinyui_sdl_observe_on_present(struct tinyui_app *app,
                                   struct tinyui_runtime_host_state *state);
int  tinyui_sdl_observe_should_quit(struct tinyui_app *app,
                                    struct tinyui_runtime_host_state *state);

#define TINYUI_SDL_OBSERVE_ON_SETUP(app, state)    tinyui_sdl_observe_on_setup((app), (state))
#define TINYUI_SDL_OBSERVE_ON_PRESENT(app, state)  tinyui_sdl_observe_on_present((app), (state))
#define TINYUI_SDL_OBSERVE_SHOULD_QUIT(app, state) tinyui_sdl_observe_should_quit((app), (state))

#else

#define TINYUI_SDL_OBSERVE_ON_SETUP(app, state)    ((void)0)
#define TINYUI_SDL_OBSERVE_ON_PRESENT(app, state)  ((void)0)
#define TINYUI_SDL_OBSERVE_SHOULD_QUIT(app, state) (0)

#endif /* ENABLE_TEST */

#endif /* TINYUI_SDL_OBSERVE_H */
