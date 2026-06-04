# PicoUI App Timer / Tick Capability Design

**日期**：2026-06-04  
**范围**：`picoui/app` public 定时器/帧驱动能力  
**目标用户**：只使用 `picoui_*` public API 的 demo / 应用作者

## 1. 背景

当前 `picoui/demo/layout_parity` 与 `picoui/demo/legacy_widget_parity` 都已经建立了结构 baseline，但还缺老 SDL truth-source 里的周期行为：

1. `layout_parity`
   - 老 `uiLayoutLoop()` 每 `1200ms` 在 `170 / 220` 之间切换 `flex row` 宽度。
2. `legacy_widget_parity`
   - 老 `uiWidgetLegacyLoop()` 每 `100ms` 驱动 `arc/gauge` 动画。

现在的 PicoUI public app 面只有：

- `picoui_app_create()`
- `picoui_app_run()`
- `picoui_app_set_window()`
- `picoui_app_switch_window()`
- `picoui_app_destroy()`

它没有任何 public `timer / tick / frame callback` 注册接口。  
backend SDL loop 虽然每帧在跑，但 `.loop / .frameStart / .frameComplete` 没有暴露成 public hook，demo 无法合法表达老页面行为。

因此，这条线的真实目标不是“补一个 demo 特判”，而是补一个通用的 public app 定时器能力。

## 2. 真相源

### 2.1 行为真相源

- `examples/common/demo/layout/uiLayout.c`
- `examples/common/demo/widget/uiWidgetLegacy.c`

### 2.2 PicoUI public app / backend 真相源

- `picoui/include/picoui/app.h`
- `picoui/src/core/app.c`
- `picoui/src/backend/ldgui/backend_app.c`

### 2.3 当前需求承接页

- `picoui/demo/layout_parity/main.c`
- `picoui/demo/legacy_widget_parity/main.c`
- `docs/superpowers/reviews/2026-06-04-picoui-three-parity-demo-gap-audit.md`

## 3. 目标

为 PicoUI 增加一组“public app 级定时器能力”，满足以下要求：

1. 只能通过 `picoui_*` public API 使用。
2. 允许 demo / 应用在 app 事件循环里注册周期任务。
3. 支持“到期后回调”这种一步到位的语义，而不是只给一个每帧裸 hook。
4. 可以同时解锁：
   - `layout_parity` 的 `1200ms` 宽度切换
   - `legacy_widget_parity` 的 `100ms` arc/gauge 动画
5. 不把某个 parity demo 的业务行为偷偷塞进 backend。

## 4. 非目标

这条线不做以下事情：

1. 不引入 PicoUI 专属 fake renderer。
2. 不做复杂调度系统：
   - 不做优先级
   - 不做线程
   - 不做跨 app 全局 scheduler
3. 不把 LingDongGUI 的 `ldTimeOut()` 直接暴露成 PicoUI public API。
4. 不顺手补：
   - `legacy_widget_parity` 真实图片资源 exact-match
   - `grid_parity` 内容 fidelity
   - dedicated visible 对照
5. 不新增与定时器无关的窗口、布局或主题 API。

## 5. 设计原则

### 5.1 Public capability，不是 demo patch

能力必须挂在 `picoui/app` 公共层，而不是只给某个 demo 写一段 backend special case。

### 5.2 一步到位采用 timer 语义

这轮用户已明确要求“按照一步到位进一步拆分 spec 和 plan”。  
因此这里不选“只有每帧 tick callback”的最小路线，而是直接设计“public timer register / cancel / repeat”语义。

### 5.3 语义尽量小而完整

虽然选的是 timer 方案，但仍然要保持小：

1. 单线程
2. 绑定到单个 `picoui_app`
3. 回调在 app 主循环线程执行
4. 不承诺实时精度，只承诺“在 backend loop 驱动下尽快触发”

### 5.4 允许 demo 自己维护业务状态

timer 只负责调度，不负责替 demo 保存动画角度、宽度切换状态等业务变量。

## 6. 方案对比

### 方案 A：只暴露 frame/tick callback

形式：

- `picoui_app_set_frame_callback(app, cb, user_data)`

优点：

1. 实现最小
2. backend 很容易接线

缺点：

1. 应用每次都要自己算节拍和重复逻辑
2. public 语义更低级
3. 与“这轮一步到位”目标不匹配

### 方案 B：public timer register / cancel / repeat

形式：

- 创建 timer handle
- 注册 interval
- 允许 one-shot / repeat
- 回调到期触发

优点：

1. 直接匹配老 truth-source 的 `1200ms` / `100ms` 需求
2. 更像稳定可复用的 public capability
3. demo 代码更干净，不需要重复自己写节拍器

缺点：

1. 比纯 tick callback 多一层 API 和状态管理
2. 需要定义 timer 生命周期

### 方案 C：把 `ldTimeOut()` 包装成 PicoUI 公共函数

优点：

1. 看起来最接近老代码

缺点：

1. 泄漏底层实现语义
2. 不符合 PicoUI 作为上层 API 的边界
3. 容易把 backend/private 行为直接搬进 public 面

### 选择结果

采用 **方案 B**。  
理由：这是当前最小但完整的 public capability，既满足“进一步一步到位”，又不至于把 backend/private 细节泄漏给用户。

## 7. Public API 设计

## 7.1 新类型

新增一个 public timer 句柄类型：

- `struct picoui_app_timer`

它是 app 级资源，由 `picoui_app` 管理生命周期。

## 7.2 新回调类型

新增回调签名：

```c
typedef void (*picoui_app_timer_cb_t)(struct picoui_app *app,
                                      struct picoui_app_timer *timer,
                                      void *user_data);
```

说明：

1. 回调拿到 `app`，便于切窗或读写页面状态。
2. 回调拿到 `timer`，便于 one-shot 自停或后续扩展。
3. `user_data` 由调用方提供。

## 7.3 新 public API

建议新增以下接口：

1. `struct picoui_app_timer *picoui_app_timer_create(struct picoui_app *app);`
2. `int picoui_app_timer_start(struct picoui_app_timer *timer,
                               unsigned int interval_ms,
                               int repeat,
                               picoui_app_timer_cb_t callback,
                               void *user_data);`
3. `int picoui_app_timer_stop(struct picoui_app_timer *timer);`
4. `int picoui_app_timer_is_running(const struct picoui_app_timer *timer);`
5. `void picoui_app_timer_destroy(struct picoui_app_timer *timer);`

语义约束：

1. `interval_ms == 0` 返回失败。
2. `callback == NULL` 返回失败。
3. `repeat == 0` 表示 one-shot。
4. `repeat != 0` 表示 repeating timer。
5. 已启动 timer 再次 `start()`：
   - 允许重置 interval / repeat / callback / user_data
   - 行为视为“restart”

## 7.4 生命周期规则

1. timer 必须绑定到某个 `picoui_app`。
2. `picoui_app_destroy(app)` 时，app 持有的 timer 全部失效并清理。
3. timer 回调只会在 `picoui_app_run()` 驱动的主循环中执行。
4. 不承诺在 `picoui_app_run()` 之外触发。

## 8. 实现架构

## 8.1 Public 层

文件：

- `picoui/include/picoui/app.h`
- `picoui/src/core/app.c`

职责：

1. 暴露新 public 类型和函数声明。
2. 做最小参数校验。
3. 管理 `picoui_app` 持有的 timer 列表或数组。

## 8.2 Backend 层

文件：

- `picoui/src/backend/ldgui/backend_app.c`

职责：

1. 在当前 SDL loop 中检查 timer 是否到期。
2. 到期时在主循环线程内触发回调。
3. one-shot 触发后自动停止。
4. repeating timer 更新下一次到期时间。

这里不新增 OS 线程，不依赖 SDL 自带 timer thread。

## 8.3 内部状态建议

每个 timer 至少需要：

1. `owner_app`
2. `interval_ms`
3. `repeat`
4. `running`
5. `next_fire_ticks`
6. `callback`
7. `user_data`

`next_fire_ticks` 基于 `SDL_GetTicks()` 或同层现有时钟源计算。

## 8.4 回调执行策略

每一帧主循环：

1. 先处理 SDL 事件
2. 再扫描 timer
3. 触发所有到期 timer
4. 再 render / delay

这样做的原因：

1. 避免回调长期落后于输入事件
2. 保持逻辑简单
3. 不需要把 timer 挂进 LingDongGUI scene loop

## 9. 错误处理

所有 public API 都应对以下情况返回失败而不是崩溃：

1. `app == NULL`
2. `timer == NULL`
3. `callback == NULL`
4. `interval_ms == 0`
5. timer 不属于当前 app
6. 重复 destroy / stop 未启动 timer

建议口径：

1. 参数非法返回 `-1`
2. `is_running()` 返回 `0/1`
3. `destroy(NULL)` 安静返回

## 10. 对 parity demo 的影响

### 10.1 `layout_parity`

补完后允许 demo 自己注册 repeating timer：

- interval: `1200ms`
- callback 内切换紧凑/宽松状态
- 再通过现有 `picoui_widget_set_width()` 或等价 public API 改容器宽度

### 10.2 `legacy_widget_parity`

补完后允许 demo 自己注册 repeating timer：

- interval: `100ms`
- callback 内更新 arc rotation 和 gauge angle

### 10.3 `grid_parity`

这条线对 `grid_parity` 没有直接 capability 收益。  
`grid_parity` 当前剩余问题仍然是内容 fidelity 和 visible 对照。

## 11. 测试与验证要求

至少补三层证据：

### 11.1 unit / contract

验证：

1. create/start/stop/destroy 的参数边界
2. repeat / one-shot 状态切换
3. app destroy 时 timer 生命周期收口

### 11.2 backend runtime

验证：

1. 在 `picoui_app_run()` 驱动下 timer 会触发
2. repeating timer 能多次触发
3. one-shot 只触发一次

### 11.3 parity demo 消费

验证：

1. `layout_parity` 真用该 public API 补上 `1200ms` 行为
2. `legacy_widget_parity` 真用该 public API 补上 `100ms` 动画

## 12. 验收标准

完成后才能诚实地说：

1. PicoUI public app 层已具备通用 timer/tick capability。
2. `layout_parity` 缺的 runtime behavior capability 已补通。
3. `legacy_widget_parity` 的老页面动画不再被 capability 缺口阻塞。

完成后仍然不能说：

1. 三页 parity demo 已与老 SDL 页面完全一致。
2. `legacy_widget_parity` 已完成真实图片资源 exact-match。
3. 三页都已有 dedicated visible 对照。

## 13. 后续顺序

这条线完成后，推荐顺序是：

1. 用新 timer capability 收口 `layout_parity`
2. 用新 timer capability 收口 `legacy_widget_parity` 动画
3. 再回到 `legacy_widget_parity` 真实资源 exact-match
4. 最后补 dedicated visible 对照
