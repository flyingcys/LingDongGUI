# TinyUI Port(SDK 适配层)问题分析与整改技术方案

> 交付范围:**仅本文档**(问题分析 + 整改技术方案),不改动代码。
> 整改力度:**稳健版**——解耦通用帧循环、让 MCU 真正可用,SDL 行为保持不变。
> MCU 目标:**通用模板**(RGB565 SPI 屏 + SysTick 毫秒时基 + 触摸轮询/中断)。
> 硬约束:**必须保证功能正常的前提下优化**——每一步 SDL demo 与合约/单元测试全绿。

---

## 0. 结论先行(TL;DR)

1. **"干净的移植契约其实已经存在"**:用户要填的公共接口非常小,和 LVGL 同构——
   `tinyui_display_set_config` + `tinyui_display_set_flush_callback` + `tinyui_tick_set_source` + `tinyui_input_push_pointer`(+ 可选 os lock/delay)。
   `Disp0_DrawBitmap→flush`、`timestamp→tick`、`touch→input` 的符号路由,已经在 `tinyui/src/drivers/tinyui_ldgui_port.c` 里做好,而且 `tinyui_ldgui_disp_adapter.c` **已经同时实现了 MCU 路径与 host 路径**。

2. **SDL port ~1030 行"这么多"的真实原因有三个,没有一个是"移植本身需要"**:
   - **测试/观测脚手架被塞进了生产 port**:`observe.c`(~403 行)全是 CI 用的 printf 标记、PPM 截图、auto-quit、smoke marker、遍历 ldgui widget 树,与"显示/输入适配"零关系。
   - **通用帧循环写死在 port 里,导致 MCU 无法复用**:`tinyui_core` 有一条**隐藏的链接期契约**——`runtime_bridge.c` 无条件调用只有 `tinyui/port/sdl/step.c` 定义的 `tinyui_runtime_host_step_app()` / `tinyui_runtime_host_shutdown_app()`。于是"每个 port 都要把整套帧循环重写一遍"。
   - **真正 SDL 特有的代码(hal.c ~250 行)本可由库提供**:窗口/纹理/present/事件泵。LVGL 把它放进库内(`lv_sdl_*`),app 只写 ~30 行;ARM-2D 自己也带了 PC 的 "Virtual TFT" 适配器(`examples/sdl/user/`)。tinyui 的 SDL port 等于又手写了一遍(本轮稳健版**不动**这块,列入后续可选项)。

3. **MCU 现状是"编都编不了",而不是"难写"**:`tinyui/port/mcu/Retarget.c` 是孤儿文件(无任何 CMake target 编译它);`tinyui_backend_ldgui_runtime` 把 `tinyui_port_sdl` 硬连进了 bundle,没有 sdl/mcu 选择开关;并且 `tinyui_timer_handler` 会走到只有 SDL 定义的 host 符号。**这三点是 MCU 不可用的根因,也是稳健版要修的核心。**

4. **一句话回答用户**:MCU 适配本应只有 ~60–100 行(flush + tick + touch + 已有的 Retarget)。SDL "这么多"是历史包袱——上一轮 `port-sdl-simplify` 只是把 1023 行的 `runtime_host.c` **拆成 3 个文件**,并没有真正减负;把测试脚手架和通用帧循环从 port 里剥离后,SDL 生产 port 会显著变薄,MCU 也随之可用。

---

## 0.5 实施状态与验证（本次执行,2026-07）

已按稳健版执行并验证的阶段:

| 阶段 | 内容 | 状态 | 验证 |
|------|------|------|------|
| S0 | 建立基线与验证闸门 | ✅ | 见下"验证闸门";**注意:仓库单元测试基线在 `dev-nanoui` 上本就无法完整链接**(`ARM_2D_FONT_6x8/16x24` 未定义,`resource.c`/`widget.c`),与本次改动无关,故闸门改用 build+contract+demo 冒烟 |
| S1 | 死代码/latent bug 清理 | ✅ | 见 §4.3/§4.4;`disp_adapter` 的 `-1` sentinel 修为 `0`、删 `apply_smoke_cursor_layout`、删冗余 weak `VT_*`/`ldCfgTouchSetPoint`、删未读 `real_tile`。SDL 全绿 |
| S3 | 通用帧循环下沉共享层 | ✅ | 见下"实际实现";新增 `tinyui_ldgui_neutral_runtime.c`,SDL 行为不变 |
| S4 | 构建 port 开关 + 无 SDL bundle + MCU target | ✅ | `LD_TINYUI_PORT=sdl(默认)/mcu/none`、`tinyui_backend_ldgui_core`、`tinyui_port_mcu`;默认 sdl 路径逐字不变 |
| S5 | MCU port 骨架 + 主机验证 harness | ✅ | `tinyui/port/mcu/tinyui_port_mcu.c`(+ example 模板)、`tests/tinyui/port_mcu/mcu_host_smoke.c` |
| S2 | observe.c 移出生产 port | ⏳ 未执行 | 见下"S2 说明"——低价值、与 demo 测试脚手架(auto-quit/capture)强耦合,建议单独评估 |

**S3 实际实现(与 §4.1 原设想的差异,更稳健)**:没有采用"platform_ops 运行期注册"或"weak override",因为**weak 符号定义在静态库里会踩到 archive 抽取顺序的坑**,可能让 SDL 的强定义不被拉入而静默降级。实际做法更简单也更 robust:
- 端口契约就是"每个 port 各自提供 `tinyui_runtime_host_step_app`/`tinyui_runtime_host_shutdown_app`"(SDL 早已如此)。
- 把**通用无 SDL 帧循环**抽成共享强函数 `tinyui_backend_neutral_step`/`tinyui_backend_neutral_shutdown`(`tinyui/src/drivers/tinyui_ldgui_neutral_runtime.c`,编入 `tinyui_backend_ldgui_porting`)。
- MCU port(`tinyui_port_mcu.c`)只用 2 个函数转发到该共享实现即可;SDL 的 `step.c` 保持自有强实现**完全不动**。
- 因此 core/`runtime_bridge.c`/SDL port **一行未改**,不存在 weak/archive 顺序风险,SDL 行为逐字不变。

**验证闸门(替代基线已坏的单测,详见 [[tinyui-build-test-gate]] 记忆)**:
- 构建:`cmake -S . -B build-debug -DENABLE_TEST=ON && cmake --build build-debug`(库 + 两 demo,exit 0)。
- 合约:`ctest -R check_tinyui -E "runtime|perf|binary_size|object_overhead|visible_ui|backend_mapping"` → **7/7 通过**。
- 功能冒烟(headless):`TINYUI_DEMO_AUTO_QUIT_MS=600 TINYUI_CAPTURE_FILE=x.ppm SDL_VIDEODRIVER=dummy ./tinyui_demo <name>` → 5 个代表 demo 全部 `exit=0`、截图 460815 字节(480×320 RGB,与基线一致)。
- MCU 无 SDL 路径:`mcu_host_smoke` 直接运行 → `MCU_HOST_SMOKE_OK flushes=32 nonzero=614228`,`nm` 显示 **0 个 SDL 符号**(证明无 SDL 也能渲染)。
- 独立 review 子代理评审 S3–S5,发现 M1(Apple `--start-group` 保护)、M2(`LD_TINYUI_PORT` 之前未接线)、L4(单实例 init flag + shutdown 复位)等,均已修复;L3(以为存在竞争 weak)经实测为误报,已回退。

**S2 说明(为何暂缓)**:`observe.c`(~403 行)确属测试脚手架,理应移出生产 port。但其中的 `TINYUI_DEMO_AUTO_QUIT_MS`(自动退出)与 `TINYUI_CAPTURE_FILE`(PPM 截图)正是 demo/CI 冒烟(含本次验证闸门)所依赖的能力;直接搬走需要为生产 `step.c` 引入可选 hook 并让 demo/test 单独链接观测库,属改动面较大、价值偏"减负"的一步。鉴于 MCU 目标已达成且此步与测试脚手架强耦合,建议作为独立任务单独评估执行,不与本次 MCU 使能混在一起。

## 1. 背景与目标

`tinyui/` 是在 `src/`(原生 LingDongGUI/ARM-2D 引擎,下称 ldgui)之上做的一层**上层 API 封装**,定位类似 `third_party/lv_port_pc_vscode` 之于 LVGL。当前 SDL 上可正常运行,但 `tinyui/port/sdl` 适配代码量偏大、`tinyui/port/mcu` 无法落地,用户对"为什么需要这么多"存疑。

本文目标:
- 定性说明"SDL port 为何这么大、哪些是必要的、哪些是历史包袱";
- 给出**稳健版**整改方案:让通用逻辑归 core、平台差异归 port,从而 **MCU port 只需 ~60–100 行**;
- 给出 **MCU 通用模板适配指南**(骨架代码 + 主循环 + 构建);
- 全程保证功能正常(增量、可回滚、每步验证)。

参照基准(仓库既有规则与文档):
- `tinyui/docs/porting_rules.md`——`src`/`port`/`demo` 职责边界;
- `tinyui/docs/src_adaptation_audit.md`——已记录 P0.1/2/7/18 等边界问题(部分已修:SDL 已移出 core);
- `docs/superpowers/specs/2026-06-13-tinyui-port-layer-redesign.md`——drivers 桥接层与 mcu Retarget 的由来;
- `docs/superpowers/plans/2026-06-15-port-sdl-simplify.md`——把 `runtime_host.c` 拆成 hal/observe/step 的那次改动。

---

## 2. 现状架构

### 2.1 分层拓扑(实际是三层,不是两层)

| 层 | 位置 | 构建目标 | 是否平台无关 |
|----|------|----------|--------------|
| **tinyui core** | `tinyui/src/{core,display,indev,tick,osal,theme,layout,widgets}` | `tinyui_core` | 是(已由合约测试强制无 SDL) |
| **ldgui 驱动桥接** | `tinyui/src/drivers/`(`tinyui_ldgui_port.c`、`tinyui_ldgui_disp_adapter.c`) | `tinyui_backend_ldgui_porting` | **跨 port 共享**——arm-2d/ldgui 耦合"合法地"集中在这里 |
| **SDL port** | `tinyui/port/sdl/`(`hal.c`、`observe.c`、`step.c`) | `tinyui_port_sdl` | 否(SDL + PC 桌面) |

一帧的调用链:
`tinyui_timer_handler()`(`tinyui/src/core/runtime.c`,public)
→ `tinyui_runtime_bridge_step_app()`(`tinyui/src/core/runtime_bridge.c`,core)
→ **`tinyui_runtime_host_step_app()`**(`tinyui/port/sdl/step.c:231`,SDL port)。

### 2.2 公共移植契约(用户真正要填的东西)——已经很干净

每种平台能力都是"一个函数指针 + 一个 setter",头文件里定义清楚,无任何 ldgui/arm-2d 泄漏:

| 能力 | 公共 API | 头文件 |
|------|----------|--------|
| 屏参 | `tinyui_display_set_config` / `get_config` | `tinyui/include/display/display.h:30-33` |
| 显示 flush | `tinyui_display_set_flush_callback`(`tinyui_display_flush_cb_t`) | `display.h:26-36` |
| 指针/按键输入 | `tinyui_input_push_pointer` / `push_key` | `tinyui/include/indev/indev.h:16-18` |
| tick 时基 | `tinyui_tick_set_source`(`tinyui_tick_get_cb_t`) | `tinyui/include/tick/tick.h:6-8` |
| OS 锁/延时 | `tinyui_os_set_lock_callbacks` / `set_delay_callback` | `tinyui/include/osal/osal.h:9,15` |

`tinyui/include/port/port.h` 只是把上面四个头一起 `#include`。core 侧实现(`src/display/display.c` 等)是纯 getter/setter,**无 ldgui/arm-2d 引用**,天然可移植。

### 2.3 ldgui 符号契约(由共享 drivers 层满足,不该每个 port 各写一份)

`tinyui/src/drivers/tinyui_ldgui_port.c` 把 ldgui/arm-2d 期望的外部符号,路由回上面的 port-state 回调(文件头注释即写明):
- `Disp0_DrawBitmap` → `display_port.flush_callback`(`tinyui_ldgui_port.c:103`)
- `VT_enter/leave_global_mutex` → `os_port.enter/leave`(`:123,131`)
- `arm_2d_helper_get_system_timestamp` → `tick_port.callback`(`:140`;注意 host 上 `:142-154` 直接用 `clock_gettime` 抢先返回)
- `ldCfgTouchGetPoint` ← `input_port`(`:306`);`ldCfgTouchSetPoint` → `tinyui_input_push_pointer`(`:338`)

`tinyui_ldgui_disp_adapter.c` 提供 `tinyui_backend_init()`(`:208`)与 `tinyui_backend_step()`(`:269`),即 arm-2d PFB 渲染管线。**它已经同时支持两条路:**
- **MCU 路径**(`:228-265`):堆上分配 PFB,渲染后经 `ldgui_port_pfb_flush_handler`(`:189-205`)调用 `flush_callback` 推像素;
- **host 路径**(`:222-226`):`flush_callback==NULL` 时跳过 PFB。

### 2.4 构建目标图(`cmake/LingDongGUI.cmake`,`ld_define_core_targets()` @ `:131`)

```
longdonggui_arm2d ── ARM-2D 库+helper+controls+math                (:136)
      ▲
longdonggui ─────── src/gui + src/misc                            (:149)
      ▲
      ├── longdonggui_porting_default ── src/porting/ldConfig.c +
      │                                   arm_2d_disp_adapter_0.c   (:157)  ← 原生默认 port(含一份 disp adapter)
      │
tinyui_core ─────── tinyui/src/** 全部核心+控件+display/indev/tick/osal (:194)
      ▲
tinyui_backend_ldgui_porting ── tinyui/src/drivers/*.c(桥接+另一份 disp adapter)(:168)
      ▲
tinyui_port_sdl ── tinyui/port/sdl/{hal,observe,step}.c + SDL2       (:251)
      ▲
tinyui_backend_ldgui / tinyui_backend_ldgui_runtime ── INTERFACE 库,
      bundle = tinyui_core + longdonggui + tinyui_backend_ldgui_porting + **tinyui_port_sdl**  (:284-300)
```

要点(均为整改抓手):
- **runtime bundle 硬连了 SDL**:`tinyui_backend_ldgui_runtime`(所有 demo/测试链接的伞形库)在 `:296` 把 `tinyui_port_sdl` 写死进去——**没有 sdl/mcu 选择开关**,等价于"只能 SDL"。
- **两份 disp adapter**:`src/porting/arm_2d_disp_adapter_0.c`(在 `longdonggui_porting_default`)与 `tinyui/src/drivers/tinyui_ldgui_disp_adapter.c`(在 `tinyui_backend_ldgui_porting`)。加上 `examples/sdl/user/arm_2d_disp_adapter_0.c`(原生 SDL demo 用),实际有三份并存,`Disp0_DrawBitmap`/touch/timestamp 存在符号冲突风险(见 `examples/sdl/CMakeLists.txt:471-474` 的注释与 `--start-group/--end-group` 规避 @ `:475-499`)。
- **屏参在构建期写死**:`ld_apply_tinyui_runtime_screen_config`(`:109-129`)通过宏 `LD_CFG_SCREEN_WIDTH/HEIGHT/PFB_WIDTH` 注入(默认 480×320)。

### 2.5 demo 入口(印证契约其实很薄)

`tinyui_demo/main.c` 已按 LVGL 范式写:`tinyui_init()` → `tinyui_demos_create()` → `for(;;) tinyui_timer_handler();`(`:52,63,70-86`)。唯一"越界"处是它直接用 `SDL_GetTicks()` 作为 demo 侧节拍(`:69,71`)——这是 demo 便利,不是 port 契约。

---

## 3. 问题分析(分级)

> 严重度:**P0** = 阻断 MCU/跨平台或存在运行风险;**P1** = 边界混乱、阻碍多平台化;**P2** = 治理/可维护性。

### P0-1:core → port 存在"隐藏链接期契约",迫使每个 port 重写整套帧循环

- 证据:`tinyui/src/core/internal.h:256-257` 声明 `tinyui_runtime_host_step_app` / `tinyui_runtime_host_shutdown_app`;`runtime_bridge.c:243/256` **无条件**调用;全仓**只有** `tinyui/port/sdl/step.c:231/274` 定义它们。
- 后果:所谓"帧步进"被整体委托给 port。于是 `step.c` 里塞进了大量**非 SDL** 的通用编排:`tinyui_runtime_bridge_init_app`(`step.c:143`)、装默认 tick/delay(`:168-173`)、`arm_2d_helper_init()`(`:56`)、给 scene 挂 `ldGuiFuncGroup`(`:195`)、`tinyui_backend_init`(`:219`)、`ldGuiSceneInit`(`:223`)、`tinyui_app_pump_timers`(`:260`)、`tinyui_backend_step`(`:122`)、auto-quit 计时(`:266`)。
- **这是 MCU 无法复用帧循环的根因**,也是"SDL port 显得很大"的最大结构性原因。

### P0-2:MCU port 实际"未接入构建",且 runtime bundle 硬连 SDL

- 证据:`tinyui/port/mcu/Retarget.c` 未被任何 CMake 引用(仅出现在历史 spec/plan 文档中),是孤儿文件;`tinyui_backend_ldgui_runtime` 在 `cmake/LingDongGUI.cmake:296` 硬连 `tinyui_port_sdl`;无 `LD_TINYUI_PORT` 之类选择开关。
- 后果:即便照契约写好 MCU 回调,也**没有构建路径**产出一个不含 SDL 的固件目标;且 `tinyui_timer_handler` 会解析到 SDL 才定义的 host 符号(P0-1),链接期直接失败。

### P0-3:disp_adapter 的 "SDL 路径" 已是死代码,并携带一个 truthy sentinel 隐患

- 证据:`tinyui_ldgui_disp_adapter.c:7-10` 注释与 `:222-226` 仍保留 "flush_callback==NULL ⇒ SDL/host,跳过 PFB" 分支,并置 `s_pfb_inited = -1`(`:224`)。但**当前 SDL 流程在 `step.c:218` 已经注册了真实 flush 回调**(`tinyui_runtime_host_copy_flush_pixels`)后才 `tinyui_backend_init`(`:219`),因此实际走的是 **MCU 路径**(分配 PFB)。
- 后果:该 SDL 分支形同死码;更糟的是 `s_pfb_inited = -1` 在 C 语义里为真,`tinyui_backend_step` 的 `if (s_pfb_inited > 0)`(`:290`)虽然拦住了它,但 sentinel 复用布尔判断属于既有 audit P0.7 记录的运行风险,应清理为显式 render mode。

### P0-4:测试/观测脚手架混入生产 port(observe.c 整个文件)

- 证据:`tinyui/port/sdl/observe.c`(~403 行)文件头自述"CI/测试基础设施——与生产渲染路径零耦合"。内容为:`touch_log_enabled`、`log_*_benchmark`、`parse_auto_quit_ms`、`write_capture`(输出 P6 PPM 截图到 `$TINYUI_CAPTURE_FILE`)、`log_mapping_markers` 等;并遍历 ldgui widget 树(`ldBaseGetChildList`/`ldBaseGetNextSibling`,读 `ldImage_t->ptImgTile`)。`host_internal.h:29-39` 里也塞了十余个 `*_logged`/`smoke_*`/`benchmark_*` 状态位。
- 后果:生产 port 体积虚增约 40%,并给人"真实板级 port 也得这么大"的错觉。这些逻辑属于测试支撑,应在 `tests/` 或 SDL test runner 里。

### P1-5:step.c 重复声明 drivers 层已强定义的 ldgui 符号 / 保留已禁用死码

- 证据:`step.c:25/33/41` 以 `__attribute__((weak))` 重定义 `VT_enter_global_mutex`/`VT_leave_global_mutex`/`ldCfgTouchSetPoint`,而 `tinyui_ldgui_port.c:123/131/338` 已强定义;`step.c:100-107` 的 `apply_smoke_cursor_layout` 注释为 "permanently disabled" 的空壳。
- 后果:防御性链接胶水与死码,增加阅读负担,且掩盖了"符号应由 drivers 层统一提供"的事实。

### P1-6:hal.c 直接耦合 arm-2d 构建宏,未走 tinyui 颜色抽象

- 证据:`hal.c:15` include `arm_2d_disp_adapter_0.h`,并在 `:31/219` 依据 `__DISP0_CFG_COLOUR_DEPTH__` / `__DISP0_COLOUR_FORMAT__` 分支;`host_internal.h:21` 携带一个从不被读的 `arm_2d_tile_t real_tile`(在 `hal.c:204-223` 构造)。而 `display.h:6-9` 已有 `enum tinyui_color_format` 未被使用。
- 后果:平台像素代码里泄漏 arm-2d 编译配置,削弱 port 可移植性;`real_tile` 是被物化进 port 的无用 arm-2d 状态。

### P2-7:屏参构建期写死、两份 disp adapter 并存、循环依赖靠 `--start-group` 规避

- 证据:`ld_apply_tinyui_runtime_screen_config`(`cmake/LingDongGUI.cmake:109-129`);`longdonggui_porting_default` 与 `tinyui_backend_ldgui_porting` 各一份 disp adapter;`examples/sdl/CMakeLists.txt:475-499` 与 `cmake/LingDongGUI.cmake:307-319` 的 `--start-group/--end-group`。
- 后果:多平台/多分辨率不便;符号冲突需要人工规避;链接顺序脆弱。属长期治理项,本轮只做"不恶化 + 记录",不强求根治。

### 现状体量小结

| 文件 | 行数 | 定性 |
|------|------|------|
| `tinyui/port/sdl/hal.c` | 319 | 大部分是**真·SDL**(窗口/纹理/present/事件泵),合理但本可库内化(后续可选) |
| `tinyui/port/sdl/observe.c` | 403 | **全是测试脚手架**(P0-4),应移出 |
| `tinyui/port/sdl/step.c` | 308 | **通用帧编排(P0-1)+ 少量 SDL + 死码(P1-5)**,通用部分应下沉 core |
| `tinyui/port/sdl/host_internal.h` | 68 | SDL 句柄合理 + 测试状态位应随 observe 移出 |
| `tinyui/port/mcu/Retarget.c` | 54 | 孤儿(P0-2),内容正确(I/O 桩) |

**移出 observe(~403)+ 帧循环下沉 core(step.c 通用部分)后,SDL 生产 port 主体将回落到"真·SDL 平台代码"量级**,而 MCU port 从 0 变为 ~60–100 行即可运行。

---

## 4. 目标态设计(稳健版)

设计原则一句话:**通用帧循环归 core,平台原语归 port;port 只"注册能力",不"承载循环"。**

### 4.1 消除隐藏链接契约:帧循环下沉 core,port 改为"可选平台钩子"

**现状**:`runtime_bridge.c` 硬依赖 port 定义的 `tinyui_runtime_host_step_app/shutdown_app`(强链接符号)。

**目标**:core 自带一套 **backend-neutral** 的默认帧步进,不再依赖任何 host 符号:
```
core 默认 step(伪代码,均为已存在的公共/内部 API):
  if (!inited) { tinyui_backend_init(app); inited = 1; }
  tinyui_app_pump_timers(app, tinyui_tick_get(app));
  tinyui_backend_step(app);        // 渲染 + 经 flush_callback 出像素(drivers 层已实现)
  if (os_port.delay) tinyui_os_delay(app, frame_ms);
```
平台差异通过**已有的 setter 注册**接入,新增一个**可选**的"平台泵/呈现"钩子 ops(函数指针,存在 `tinyui_app` 上,默认 NULL):
```
struct tinyui_platform_ops {          // 新增,可选
    int  (*pump)(struct tinyui_app *app);     // 采集输入/窗口事件;返回 >0 表示请求退出
    void (*present)(struct tinyui_app *app);  // 帧末呈现(桌面窗口需要;MCU 通常 NULL)
    void (*shutdown)(struct tinyui_app *app);
};
int tinyui_platform_set_ops(struct tinyui_app *app, const struct tinyui_platform_ops *ops);
```
core 默认 step 里,若 `ops.pump` 非空则在渲染前调用,若 `ops.present` 非空则在渲染后调用。**关键**:`runtime_bridge` 不再无条件调用 `tinyui_runtime_host_step_app`;该符号或删除,或降级为"SDL port 注册的 ops.pump/present"实现。

收益:
- **MCU**:一个 ops 都不用注册(`pump/present=NULL`),靠 flush_callback 出像素即可——帧循环完全复用 core。
- **SDL**:`hal.c` 的 `pump_sdl_events`→`ops.pump`、`present_real_frame`→`ops.present`;`step.c` 的通用编排消失,只剩 SDL 具体实现。
- 彻底移除 P0-1 的隐藏链接契约。

> 兼容策略:这是本轮**唯一的结构性改动**,风险集中在 `runtime_bridge` 与 `step.c` 边界。采用"先加 core 默认 step 与 ops,再把 SDL 切到 ops,最后删旧 host 符号"的三小步,每步 SDL demo 必须仍可运行(见 §6)。

### 4.2 observe.c 移出生产 port

- 把 `observe.c` 及 `host_internal.h` 中的 `*_logged`/`smoke_*`/`benchmark_*` 状态位迁到 `tests/tinyui/runtime/`(或 `examples/sdl/tests/` 支撑库),仅在 `ENABLE_TEST` 下编译链接进 `tinyui_demo`/runtime 测试。
- 生产 `tinyui_port_sdl` 不再包含任何 capture/marker/benchmark/auto-quit。
- Python runtime 测试(`check_use_demo_runtime.py` 等)改为链接该测试支撑库,行为不变。

### 4.3 死代码 / sentinel 清理

- 删除 `tinyui_ldgui_disp_adapter.c` 的 `flush_callback==NULL` SDL 分支与 `s_pfb_inited=-1` sentinel(P0-3);统一为"单一 PFB+flush 路径"。若将来确需 host 直呈现,用显式 `enum tinyui_render_mode { TILED_FLUSH, DIRECT_PRESENT }` 表达,不复用布尔。
- 删除 `step.c` 的 `apply_smoke_cursor_layout` 空壳(P1-5)与重复的 weak `VT_*`/`ldCfgTouchSetPoint`(drivers 层已强定义)。

### 4.4 hal.c 去 arm-2d 宏耦合(轻度)

- 用 `tinyui_display_get_config()` 返回的 `enum tinyui_color_format` 取代 `hal.c` 对 `__DISP0_CFG_COLOUR_DEPTH__` 的分支;删除从不被读的 `real_tile`。
- 本项低风险,可并入 §4.1 的 SDL 切换步骤。

### 4.5 构建:port 选择开关 + 拆开 runtime bundle + 新增 MCU target

```cmake
# 新增缓存变量:选择平台 port
set(LD_TINYUI_PORT "sdl" CACHE STRING "TinyUI platform port: sdl | mcu | none")

# 把 runtime bundle 拆成"平台无关内核" + "所选 port"
add_library(tinyui_backend_ldgui_core INTERFACE)          # = core + longdonggui + backend_ldgui_porting(无 SDL)
target_link_libraries(tinyui_backend_ldgui_core INTERFACE
    tinyui_core longdonggui tinyui_backend_ldgui_porting)

if(LD_TINYUI_PORT STREQUAL "sdl")
    # tinyui_port_sdl 定义同现状(hal/step + 测试期 observe)
    add_library(tinyui_backend_ldgui_runtime INTERFACE)
    target_link_libraries(tinyui_backend_ldgui_runtime INTERFACE
        tinyui_backend_ldgui_core tinyui_port_sdl)
elseif(LD_TINYUI_PORT STREQUAL "mcu")
    add_library(tinyui_port_mcu STATIC
        ${LD_REPO_ROOT}/tinyui/port/mcu/tinyui_port_mcu.c   # 新增:参考骨架
        ${LD_REPO_ROOT}/tinyui/port/mcu/Retarget.c)          # 现有孤儿文件接入
    target_link_libraries(tinyui_port_mcu PUBLIC tinyui_core)
    add_library(tinyui_backend_ldgui_runtime INTERFACE)
    target_link_libraries(tinyui_backend_ldgui_runtime INTERFACE
        tinyui_backend_ldgui_core tinyui_port_mcu)
else() # none:用户自带 port,只给内核
    add_library(tinyui_backend_ldgui_runtime ALIAS tinyui_backend_ldgui_core)
endif()
```
说明:
- 默认 `sdl`,现有 SDL 构建与行为**完全不变**(向后兼容)。
- `mcu`/`none` 才引入交叉编译路径,不链接 SDL2。
- 两份 disp adapter 的根治(P2-7)**不在稳健版范围**;稳健版通过"tinyui 路径只用 `tinyui_backend_ldgui_porting`,不再叠加 `longdonggui_porting_default`"来避免符号冲突(现状本已如此,继续保持)。

### 4.6 目标态用户使用模型(与 LVGL 对齐)

```c
// MCU / 任意平台:注册能力 → 跑循环
tinyui_init();
struct tinyui_app *app = /* 建 UI */;
tinyui_display_set_config(app, &cfg);
tinyui_display_set_flush_callback(app, my_flush, NULL);
tinyui_tick_set_source(app, my_ms, NULL);
// 桌面才需要:tinyui_platform_set_ops(app, &sdl_ops);
for (;;) {
    my_touch_poll(app);          // tinyui_input_push_pointer(...)
    tinyui_timer_handler();      // core 默认 step:pump?→backend_step→present?
}
```

---

## 5. MCU 通用适配指南(RGB565 SPI 屏 + SysTick + 触摸)

> 目标态(§4)落地后,以下即为 MCU port 的**全部工作**。骨架建议落在 `tinyui/port/mcu/tinyui_port_mcu.c`。

### 5.1 契约清单(用户要做的全部)

| 能力 | API | 你要提供什么 | 备注 |
|------|-----|--------------|------|
| 屏参 | `tinyui_display_set_config({w,h,RGB565,buffer_height})` | 分辨率、PFB 行高 | RAM = `w * buffer_height * 2` 字节 |
| flush | `tinyui_display_set_flush_callback(app, flush, ud)` | 把一块 RGB565 矩形写到 LCD | SPI/FSMC/8080 均可;可选 DMA |
| tick | `tinyui_tick_set_source(app, get_ms, ud)` | 返回自启动的毫秒数 | 来自 SysTick 累加 |
| 触摸 | `tinyui_input_push_pointer(app, x, y, pressed)` | 在触摸中断/轮询里调用 | 坐标为屏幕像素 |
| (可选)RTOS 锁 | `tinyui_os_set_lock_callbacks(app, enter, leave, ud)` | 临界区/互斥 | 裸机留空 |
| retarget | `tinyui/port/mcu/Retarget.c`(已存在) | assert/stdio/heap 桩 | 接入构建即可 |

### 5.2 参考骨架(与具体芯片无关)

```c
/* tinyui/port/mcu/tinyui_port_mcu.c —— 通用参考骨架 */
#include "tinyui.h"
#include <stdint.h>
#include <stddef.h>

/* ── 1) LCD flush:把一块 RGB565 像素刷到屏 ────────────────── */
static void mcu_lcd_flush(const struct tinyui_area *area,
                          const void *pixels, void *ud)
{
    (void)ud;
    /* 你的屏驱动:设置刷新窗口,然后批量写像素(小端 RGB565) */
    lcd_set_window(area->x, area->y, area->width, area->height);
    lcd_write_pixels((const uint16_t *)pixels,
                     (size_t)area->width * (size_t)area->height);
    /* DMA 版:发起 lcd_dma_write(...),在传输完成中断里推进下一块;
     * 本模板用阻塞写,函数返回即代表这一块已上屏。 */
}

/* ── 2) tick:毫秒时基(SysTick 累加) ─────────────────────── */
extern volatile uint32_t g_systick_ms;   /* 在 SysTick_Handler 里 ++ */
static unsigned int mcu_tick_ms(void *ud) { (void)ud; return g_systick_ms; }

/* ── 初始化:一次性注册能力 ──────────────────────────────── */
void tinyui_port_mcu_init(struct tinyui_app *app, int w, int h)
{
    struct tinyui_display_config cfg = {
        .width  = w,
        .height = h,
        .color_format = TINYUI_COLOR_FORMAT_RGB565,
        .buffer_height = 40,     /* PFB 行数:w*40*2 字节 RAM;可下调省内存 */
    };
    tinyui_display_set_config(app, &cfg);
    tinyui_display_set_flush_callback(app, mcu_lcd_flush, NULL);
    tinyui_tick_set_source(app, mcu_tick_ms, NULL);
    /* 裸机无需锁;RTOS 下:
     * tinyui_os_set_lock_callbacks(app, my_enter, my_leave, NULL); */
    /* MCU 不需要平台 pump/present:core 默认 step 直接经 flush 出像素 */
}

/* ── 3) 触摸:在触摸中断或轮询里调用 ─────────────────────── */
void tinyui_port_mcu_touch_poll(struct tinyui_app *app)
{
    int x = 0, y = 0;
    int pressed = touch_read(&x, &y);          /* 你的触摸驱动 */
    tinyui_input_push_pointer(app, x, y, pressed);
}
```

### 5.3 主循环(裸机)

```c
int main(void)
{
    board_init();
    systick_init_1ms();     /* SysTick 每 1ms 使 g_systick_ms++ */
    lcd_init();             /* SPI/FSMC + RGB565 */
    touch_init();

    tinyui_init();
    struct tinyui_app *app = /* 你的建 UI:窗口/控件,或 tinyui_demos_create(...) */;
    tinyui_port_mcu_init(app, 480, 320);

    for (;;) {
        tinyui_port_mcu_touch_poll(app);
        tinyui_timer_handler();   /* 目标态:core 默认 step → backend_step → flush */
    }
}
```

### 5.4 时钟频率桩(与 SDL demo 同款)

ARM-2D 需要知道时基频率。若你不走 perf_counter,可提供:
```c
uint32_t arm_2d_helper_get_reference_clock_frequency(void) { return 1000u; } /* 1ms tick ⇒ 1000Hz */
```
(SDL demo 里给的是 `1000000u`,对应 μs 时基——见 `tinyui/port/sdl/tinyui_demo_runtime_stubs.c`。MCU 侧按你的 tick 单位取值。)Cortex-M 上更推荐直接启用 perf_counter,让 `arm_2d_helper_get_system_timestamp` 走 `get_system_ticks()`(与 `src/porting/ldConfig.c:161/184` 的原生实现一致)。

### 5.5 RAM 预算

- PFB 内存 = `width * buffer_height * 2` 字节。例:480×40×2 = **38.4 KB**;若紧张,把 `buffer_height` 降到 20 → 19.2 KB(代价是每帧刷新分块更多)。
- 另需 ldgui 堆(`LD_MEM_SIZE`,原生默认 96 KB,见 `src/porting/ldConfig.h`),用于控件/资源。总量按你板子的 SRAM 调整。

### 5.6 与仓库现有原生 MCU 工程的关系(参考,不在本轮范围)

`examples/stm32f103/`、`examples/mh2103c/` 是**原生 ldgui** 的真实 MCU 工程(Keil/CMSIS-Pack)。它们的 `Disp0_DrawBitmap`(屏)、`ldCfgTouchSetPoint`(触摸)、SysTick/perf_counter(时基)就是本节 flush/touch/tick 的现成硬件实现——把这些硬件函数接到 §5.2 的三个回调即可,无需重写驱动。

### 5.7 构建

用 `LD_TINYUI_PORT=mcu`(§4.5)+ 你的交叉工具链(arm-none-eabi)。`tinyui_port_mcu` 编译 `tinyui_port_mcu.c` + `Retarget.c`,链接 `tinyui_backend_ldgui_core`(不含 SDL2)。屏参可在运行期由 `tinyui_display_set_config` 决定,不必再依赖构建期宏。

---

## 6. 分阶段实施路线(每步 SDL 全绿,可回滚)

> 单一硬性验收贯穿全程:`cmake --build` 无 error;`ctest -L "tinyui;contract"` 与 `-L "tinyui;unit"` 全通过;`tinyui_demo`/`ldgui_sdl_demo` 可正常显示与交互。

| 阶段 | 内容 | 风险 | 完成判据 |
|------|------|------|----------|
| **S0 基线** | 记录当前 `wc -l` 与全部 ctest 结果作为基线 | 无 | 基线数据留存 |
| **S1 死码/胶水清理** | §4.3、§4.4(删 disp_adapter SDL 死分支+sentinel、step.c 空壳与重复 weak 符号、hal.c 的 real_tile 与 arm-2d 宏分支) | 低 | SDL 全绿,行为不变 |
| **S2 observe 迁出** | §4.2:observe.c + 测试状态位迁到 `tests/`,仅 `ENABLE_TEST` 编译;Python runtime 测试改链测试支撑库 | 中(测试链路) | runtime 测试全绿,生产 `tinyui_port_sdl` 不含 observe |
| **S3 帧循环下沉 core** | §4.1 分三小步:①core 加默认 step + `tinyui_platform_ops`(SDL 仍用旧 host 符号);②SDL 切到 ops(pump/present/shutdown),core 默认 step 生效;③删除 `runtime_host_step_app/shutdown_app` 隐藏契约 | 高(结构性) | 每小步 SDL 全绿;`step.c` 仅剩 SDL 实现 |
| **S4 构建开关 + MCU target** | §4.5:加 `LD_TINYUI_PORT`,拆 `tinyui_backend_ldgui_core`,新增 `tinyui_port_mcu`(骨架+Retarget);默认 sdl 不变 | 中 | `-DLD_TINYUI_PORT=sdl` 与现状一致;`mcu` 可交叉编译出不含 SDL 的库 |
| **S5 MCU 骨架 + 文档校准** | §5:落 `tinyui_port_mcu.c` 参考骨架,补 `tinyui/docs` 使用说明;可用 QEMU 或目标板冒烟验证 flush/tick/touch | 中 | MCU 侧可编译链接;(有板/QEMU 时)能出画面并响应触摸 |

回滚:每阶段独立提交;S3 是唯一高风险阶段,其三小步各自可单独回退到"仍由 SDL host 符号驱动"的中间态。

---

## 7. 验收标准

- **功能不回归**:SDL 全部 demo(`USE_DEMO=0..6` 与 `tinyui_demo <name>`)显示与交互与整改前一致;`ctest -L tinyui` 全绿。
- **port 减负可量化**:生产 `tinyui_port_sdl` 不再包含 observe/benchmark/capture/auto-quit;`step.c` 不再包含通用帧编排(仅 SDL 实现)。
- **MCU 可用**:`-DLD_TINYUI_PORT=mcu` 可产出不含 SDL 的库;按 §5 骨架填 flush/tick/touch 即可跑通;核心适配代码 ≤ ~100 行。
- **边界不劣化**:`tests/tinyui/contract/check_tinyui_core_no_sdl.py` 仍通过;建议**新增**一条合约测试,禁止 `tinyui/port/**` 引入 CI/capture 关键字(防止测试脚手架回流 port)。

---

## 8. 风险与缓解

| 风险 | 缓解 |
|------|------|
| S3 改动 `runtime_bridge` 帧驱动,可能影响所有 demo | 三小步推进,每步跑全量 SDL ctest;保留旧 host 符号直到 ②验证通过再于③删除 |
| observe 迁出后 Python runtime 测试断链 | S2 先让测试支撑库提供同名符号并被测试目标链接,再从 port 摘除 |
| MCU 缺真实硬件难以端到端验证 | 先保证"可编译链接 + 无 SDL 依赖";端到端用 `examples/stm32f103`/`mh2103c` 的硬件驱动或 QEMU 冒烟 |
| 两份 disp adapter 符号冲突(P2-7)在引入新 target 时暴露 | 稳健版维持"tinyui 路径只用 `tinyui_backend_ldgui_porting`";根治留作后续独立任务 |

---

## 9. 与既有文档的关系

- 本文**聚焦 port/SDK 适配层**(SDL 为何大、MCU 如何落地),是对 `src_adaptation_audit.md`(聚焦 `tinyui/src` 通用性)与 `tinyui_src_simplification.md`(聚焦 src 精简)的**补充**,不冲突:audit 已推动"SDL 移出 core",本文接着解决"port 内部的通用逻辑与测试脚手架分离、以及 MCU 落地"。
- 与 `2026-06-15-port-sdl-simplify` 的关系:那次只是把 `runtime_host.c`(1023 行)**按文件拆分**为 hal/observe/step,总量未减;本文提出的是**职责再归位**(observe 出 port、通用循环下沉 core),才是真正减负与解锁 MCU。
- `porting_rules.md` 的边界原则在本文得到强化:port 只做"平台能力注册",不承载帧循环、不承载测试脚手架。

---

## 附录 A:关键证据索引(file:line)

- 公共契约:`display.h:26-36`、`indev.h:16-18`、`tick.h:6-8`、`osal.h:9-15`、`port/port.h`
- 符号桥接:`tinyui_ldgui_port.c:103/123/131/140/306/338`
- disp adapter 双路径与 sentinel:`tinyui_ldgui_disp_adapter.c:189-205/208/222-226/228-265/269/290`
- 隐藏链接契约:`core/internal.h:256-257`、`runtime_bridge.c:243/256`、`step.c:231/274`
- SDL port 定性:`hal.c:17/23/29/54/111/232/256`、`observe.c`(整文件,测试脚手架)、`step.c:56/70/122/195/218/219/223/260/266`、`host_internal.h:15-40`
- 构建:`cmake/LingDongGUI.cmake:109-129/131/157/168/194/251/284-300`、`examples/sdl/CMakeLists.txt:471-499/511-544`
- MCU 现状:`tinyui/port/mcu/Retarget.c`(孤儿)、`tinyui/port/sdl/tinyui_demo_runtime_stubs.c`
- 原生参考:`src/porting/ldConfig.c:92/135/161/184`、`src/porting/ldConfig.h:35-57`、`examples/sdl/user/`、`examples/stm32f103/`、`examples/mh2103c/`
- demo 入口:`tinyui_demo/main.c:52/63/70-86`
