# SDL Demo 移动/拖动卡顿分析与修复技术方案

## 1. 背景与目标

本方案面向 `examples/sdl` 下的 SDL 桌面 Demo。当前现象不是单纯的“拖动时掉帧”，而是 SDL 前端刷新链路、Arm-2D 渲染节奏、输入事件处理和线程同步方式叠加后，导致窗口移动、鼠标拖动、窗口暴露重绘时都明显发卡，且空闲时 CPU 占用异常偏高。

本文目标有两个：

1. 把这次排查过程沉淀为长期可复用的诊断文档。
2. 给出分阶段修复方案，后续实现时可以按优先级落地，而不是一次性大改。

## 2. 复现环境与现象

### 2.1 复现入口

- 可执行文件：`build/demo`
- 构建命令：`rtk make build/demo`
- 运行方式：

```bash
rtk proxy sh -lc 'env SDL_VIDEODRIVER=dummy ./build/demo'
```

### 2.2 直观现象

- Demo 正常启动，但窗口移动、拖动、暴露重绘时不流畅。
- 即使没有主动交互，进程空闲 CPU 依然明显偏高。
- 交互卡顿不是单点问题，而是“持续高刷新 + 全屏搬运 + 双线程忙等 + EXPOSED 重绘”共同放大的结果。

### 2.3 运行态验证证据

对 `build/demo` 做 5 秒空闲采样，使用如下命令：

```bash
rtk proxy sh -lc 'log=$(mktemp); env SDL_VIDEODRIVER=dummy ./build/demo >"$log" 2>&1 & pid=$!; sleep 5; ps -p "$pid" -o pid,ppid,pcpu,pmem,etime,comm; echo "--threads--"; ps -L -p "$pid" -o pid,tid,pcpu,stat,comm; kill "$pid"; wait "$pid" 2>/dev/null || true; sed -n "1,40p" "$log"; rm -f "$log"'
```

本次复核到的关键结果如下：

```text
PID    PPID %CPU %MEM ELAPSED COMMAND
35977  35971  161  0.0 00:05   demo

--threads--
PID    TID   %CPU STAT COMMAND
35977  35977 100  Rl   demo
35977  35979 61.6 Sl   arm-2d thread
```

结论：

- 5 秒空闲样本下，总 CPU 已达到 `150%+`，本次实测约 `161%`，与“空闲约 151%”的异常判断一致。
- CPU 并非只集中在单线程，而是主线程和 `arm-2d thread` 同时持续消耗。
- 这说明问题不是一次性初始化尖峰，而是常驻运行机制本身存在忙轮询/高频刷新。

## 3. 已阅读的代码路径

本次分析直接阅读并交叉定位了以下代码路径：

- `user/main.c`
- `user/Virtual_TFT_Port.c`
- `user/Virtual_TFT_Port.h`
- `user/arm_2d_disp_adapter_0.c`
- `user/arm_2d_disp_adapter_0.h`
- `user/arm_2d_cfg.h`
- `user/ldConfig.h`

重点关注点如下：

### 3.1 `user/main.c`

- `SDL_CreateThread(app_2d_main_thread, "arm-2d thread", NULL);`
- 主线程持续执行 `VT_sdl_refresh_task()`
- 子线程持续执行 `ldGuiLoop()`

这说明整个 Demo 当前是典型的“双线程常驻轮询”模型，而不是事件驱动或条件唤醒模型。

### 3.2 `user/Virtual_TFT_Port.c`

该文件是 SDL 侧问题最集中的位置：

- `VT_sdl_refresh_task()` 内部用 `arm_2d_helper_is_time_out(1000/60)` 固定做约 60Hz 刷新判断。
- 每轮刷新都执行：
  - `VT_Fill_Multiple_Colors(...)`
  - `SDL_UpdateTexture(texture, NULL, tft_fb, ...)`
  - `SDL_RenderClear(renderer)`
  - `SDL_RenderCopy(renderer, texture, NULL, NULL)`
  - `SDL_RenderPresent(renderer)`
- 事件泵采用 `while(SDL_PollEvent(&event))`
- `SDL_WINDOWEVENT_EXPOSED` 分支再次执行一次整纹理上传和整屏呈现。
- `VT_sdl_vsync()` 中如果未拿到刷新完成标记，只做 `SDL_Delay(1)` 后继续轮询。

这意味着：即使没有有效输入、没有脏区变化，也会周期性做整屏搬运和整纹理上传；同时 `EXPOSED` 还会放大重绘成本。

### 3.3 `user/arm_2d_disp_adapter_0.h`

存在显式帧率锁定逻辑：

```c
if (arm_2d_helper_is_time_out(1000 / (1000,##__VA_ARGS__))) {
    ...
}
```

说明上层渲染框架本身就带有按帧推进的节奏控制。

### 3.4 `user/arm_2d_disp_adapter_0.c`

- `__on_each_frame_complete()` 里有统计 CPU/FPS 的逻辑。
- `disp_adapter0_nano_prepare()` 的 `__DISP0_CFG_NANO_ONLY__` 分支里会调用 `arm_2d_helper_pfb_full_frame_refresh_mode(..., true)`。
- 这说明框架内部本身具备“整帧刷新路径”；即使当前 SDL demo 不一定走到该分支，也说明代码体系里存在优先整帧刷新的实现倾向，后续修复时需要明确区分“当前运行链路”和“可选分支行为”。

### 3.5 `user/ldConfig.h`

当前配置显示：

- `LD_CFG_SCREEN_WIDTH = 480`
- `LD_CFG_SCREEN_HEIGHT = 272`
- `LD_CFG_PFB_WIDTH = LD_CFG_SCREEN_WIDTH`
- `LD_CFG_PFB_HEIGHT = LD_CFG_SCREEN_HEIGHT / 10`
- `__DISP0_CFG_DEBUG_DIRTY_REGIONS__ = 0`
- `__DISP0_CFG_ENABLE_3FB_HELPER_SERVICE__ = 1`

这里的关键不是分块高度本身，而是 SDL 端没有真正按脏区/局部刷新的思路使用这些分块结果，最终仍退化成整屏路径。

## 4. 逐步排查过程

### 4.1 先确认问题是否真实存在于当前二进制

- 先确认 `build/demo` 可执行文件存在。
- 再用 `rtk make build/demo` 确认当前目录可重新产出二进制。
- 运行 5 秒做空闲采样，确认 CPU 异常是现状而不是历史结论残留。

结果：问题可稳定复现。

### 4.2 判断是单线程热点还是双线程共同拉高

通过 `ps -L -p <pid>` 查看线程级 CPU：

- 主线程约 `100%`
- `arm-2d thread` 约 `61.6%`

结论：主线程 SDL 刷新/事件泵和后台 GUI 渲染线程都在持续跑，属于“双线程同时忙”的问题。

### 4.3 排查主线程为何空闲仍高占用

阅读 `VT_sdl_refresh_task()` 后可以确定：

1. 主线程不是阻塞在 `SDL_WaitEvent`。
2. 刷新逻辑按固定 60Hz 周期持续触发。
3. 每次触发都走整纹理上传和整屏 `RenderCopy + Present`。
4. 事件泵处理完后立即返回，外层 `while(1)` 又立刻继续下一轮。

结论：主线程即便空闲，也会持续做高频刷新和事件轮询。

### 4.4 排查子线程为何持续消耗 CPU

`app_2d_main_thread()` 中：

```c
while(1) {
    if (VT_is_request_quit()) {
        break;
    }
    ldGuiLoop();
}
```

这里没有等待条件、没有阻塞事件、没有按需唤醒，说明子线程也处于“不断推进 GUI 循环”的模式。`VT_sdl_vsync()` 虽然带了 `SDL_Delay(1)`，但本质仍然是 1ms 粒度的忙轮询同步，不是有效的睡眠/信号唤醒。

结论：子线程的高 CPU 不来自一次性渲染，而来自长期忙等。

### 4.5 排查是否存在窗口系统事件放大

`SDL_WINDOWEVENT_EXPOSED` 分支中，又做了一次：

- `SDL_UpdateTexture(...)`
- `SDL_RenderClear(...)`
- `SDL_RenderCopy(...)`
- `SDL_RenderPresent(...)`

窗口拖动、遮挡恢复、桌面合成器触发暴露事件时，这段路径会与正常 60Hz 刷新叠加，放大卡顿感。

结论：窗口交互期间更卡，和 EXPOSED 分支的整屏重绘直接相关。

### 4.6 排查是否只是输入事件过多

单看输入事件并不足以解释空闲 150%+ CPU，因为在无输入情况下 CPU 依然偏高。输入只是在高频刷新链上继续加压：

- 鼠标移动事件本身频率高。
- 现有实现没有输入降采样或坐标变化合并。
- 高速拖动时，输入事件与整屏刷新互相叠加。

结论：输入不是唯一根因，但会明显放大问题。

## 5. 根因结论

本次问题应明确拆成四类根因，缺一不可：

### 5.1 刷新 60Hz + 输入未降采样

- `VT_sdl_refresh_task()` 以固定约 60Hz 运行。
- 鼠标移动/窗口交互期间，输入事件没有做节流、合并或只保留最新位置。
- 结果是渲染链和输入链同时高频工作，拖动时更容易出现卡顿。

### 5.2 SDL 整屏搬运 / 整纹理上传

- 每次刷新都 `SDL_UpdateTexture(texture, NULL, tft_fb, ...)`。
- `NULL` 区域意味着整块纹理上传。
- 后续 `SDL_RenderCopy(renderer, texture, NULL, NULL)` 也是整屏提交。
- 即使 Arm-2D/PFB 在内部有分块或脏区思路，到了 SDL 端仍被整屏路径抵消。

这是当前 CPU 持续偏高和拖动卡顿的核心放大器。

### 5.3 双线程轮询 / 忙等同步

- 主线程：无限循环调用 `VT_sdl_refresh_task()`
- 子线程：无限循环调用 `ldGuiLoop()`
- 同步：`VT_sdl_vsync()` 失败后只 `SDL_Delay(1)`

这不是事件驱动，而是低粒度睡眠包装下的忙轮询。空闲 CPU 高、线程同时活跃、系统调度抖动都由此而来。

### 5.4 窗口 `EXPOSED` 重绘放大

- `SDL_WINDOWEVENT_EXPOSED` 中直接再次整纹理上传和整屏 present。
- 拖动窗口、遮挡恢复、桌面合成器触发暴露事件时，这条路径会频繁进入。

因此用户感受到的“移动/拖动更卡”，不是心理感受，而是运行链路确实在窗口事件下进一步放大了重绘成本。

## 6. 修复技术方案

修复建议按阶段推进，先拿最小收益比最高的项，避免一开始就重构过深。

### 阶段 A：先降空闲 CPU 与拖动卡顿

推荐优先级：`P0`

目标：

- 先把空闲 CPU 从 `150%+` 拉下来。
- 先让窗口移动/拖动时不再明显卡顿。

涉及文件：

- `user/Virtual_TFT_Port.c`
- `user/main.c`

建议动作：

1. 把主线程刷新从“固定 60Hz + 忙轮询”改成“事件等待 + 定时唤醒”模型。
2. 优先评估 `SDL_WaitEventTimeout()` 取代当前 `SDL_PollEvent()` 外层空转循环。
3. 在没有新帧、没有输入、没有窗口事件时，不做整屏 `SDL_UpdateTexture + RenderPresent`。
4. `VT_sdl_vsync()` 不再使用 `SDL_Delay(1)` 忙等，改成条件变量、信号量或 SDL 事件唤醒。

风险点：

- 需要确认 `ldGuiLoop()` 当前是否依赖持续 polling 才能推进动画或 timeout。
- 如果直接改成纯阻塞，可能出现动画不更新或页面轮播停住。

验证口径：

- 空闲 5 秒 CPU 明显下降，目标先压到单核以下，再逐步优化。
- 窗口静止时 CPU 不应维持 `150%+`。
- 页面自动切换、控件动画、键鼠交互不能回归失效。

### 阶段 B：把 SDL 输出从整屏改成脏区/局部刷新

推荐优先级：`P0`

目标：

- 把最大的搬运成本从 SDL 端切掉。

涉及文件：

- `user/Virtual_TFT_Port.c`
- `user/arm_2d_disp_adapter_0.c`
- `user/arm_2d_disp_adapter_0.h`

建议动作：

1. 评估是否能直接拿到当前 PFB/dirty region 的有效区域。
2. 把 `SDL_UpdateTexture(texture, NULL, ...)` 改为局部矩形更新。
3. 把 `SDL_RenderCopy(renderer, texture, NULL, NULL)` 的整屏拷贝，按实际 dirty rect 提交。
4. 如果当前 SDL renderer 路径不适合局部提交，可评估切换为更适合 streaming update 的纹理策略。

风险点：

- SDL 逻辑尺寸映射、缩放、旋转情况下，dirty rect 坐标需要校准。
- 多个脏区合并策略如果处理不好，可能引入撕裂、漏刷或区域残影。

验证口径：

- 连续拖动时 CPU 明显下降。
- 静态页面无交互时不应再持续做全屏上传。
- 拖动滚动条、进度条等局部变化控件时，性能提升应明显。

### 阶段 C：输入事件降采样与合并

推荐优先级：`P1`

目标：

- 把高速鼠标移动、拖动时的输入风暴削平。

涉及文件：

- `user/Virtual_TFT_Port.c`

建议动作：

1. 对 `SDL_MOUSEMOTION` 只保留最新坐标，合并同一刷新周期内的中间事件。
2. 对拖动类交互引入最小时间片或最小位移阈值。
3. 对窗口移动期间的 EXPOSED/输入叠加场景，优先保留最新状态，而不是逐条消费。

风险点：

- 阈值过大可能让拖动精度变差。
- 某些控件若依赖原始高频输入，可能需要单独例外处理。

验证口径：

- 鼠标快速拖动滑块、列表、窗口时，视觉上不再抖动或明显掉速。
- 输入轨迹不能出现明显跳点。

### 阶段 D：收敛窗口 `EXPOSED` 重绘策略

推荐优先级：`P1`

目标：

- 避免窗口事件把正常刷新链放大成双倍甚至多倍开销。

涉及文件：

- `user/Virtual_TFT_Port.c`

建议动作：

1. `SDL_WINDOWEVENT_EXPOSED` 不直接无条件整屏上传。
2. 将 EXPOSED 仅作为“需要重绘”标记，并合并到下一次正常刷新周期。
3. 如果必须立即重绘，也只重绘上一次可见有效帧，而不是重复走完整搬运链。

风险点：

- 处理不当可能在窗口恢复显示后短暂出现旧帧。

验证口径：

- 拖动窗口、最小化恢复、遮挡后恢复时，卡顿明显改善。
- 不出现长时间黑屏、花屏、旧帧残留。

### 阶段 E：再考虑 60Hz 是否需要保留为硬目标

推荐优先级：`P2`

目标：

- 在 P0/P1 完成后，再评估刷新率目标是否需要从固定 60Hz 调整为自适应。

涉及文件：

- `user/Virtual_TFT_Port.c`
- `user/arm_2d_disp_adapter_0.h`

建议动作：

1. 对静态页面采用更低刷新频率或按需刷新。
2. 对动画/轮播期间再拉高刷新率。
3. 把“固定 60Hz”改成“活动场景 60Hz，静态场景低频或事件驱动”。

风险点：

- 需要区分静态页和动画页，否则容易引入肉眼可见卡顿。

验证口径：

- 静态场景 CPU 继续下降。
- 动态动画场景保持可接受流畅度。

## 7. 推荐实施顺序

建议严格按下面顺序落地：

1. 先做阶段 A，解决双线程忙轮询。
2. 再做阶段 B，切掉 SDL 整屏上传/整屏呈现。
3. 再做阶段 C 和 D，处理输入风暴与 EXPOSED 放大。
4. 最后再评估阶段 E，决定是否保留固定 60Hz。

原因很直接：

- 如果不先去掉忙轮询，后续所有优化都会被高频空转吞掉收益。
- 如果不把整屏上传改掉，仅靠输入节流，改善会有限。
- `EXPOSED` 和输入合并属于放大项，适合在主链优化后继续收口。

## 8. 验证方案

后续每一阶段都建议保持相同口径验证，避免“感觉变快了”这种不可复用结论。

### 8.1 空闲 CPU

验证命令：

```bash
rtk proxy sh -lc 'env SDL_VIDEODRIVER=dummy ./build/demo >/tmp/demo.log 2>&1 & pid=$!; sleep 5; ps -p "$pid" -o pid,pcpu,etime,comm; ps -L -p "$pid" -o pid,tid,pcpu,stat,comm; kill "$pid"; wait "$pid" 2>/dev/null || true'
```

关注项：

- 总 CPU 是否从 `150%+` 明显下降。
- 主线程和 `arm-2d thread` 是否都下降，而不是只转移热点。

### 8.2 拖动与窗口移动

验证动作：

- 拖动 SDL 窗口
- 高频移动鼠标
- 拖动控件
- 遮挡后恢复窗口

关注项：

- 主观流畅度是否改善。
- EXPOSED 期间是否仍出现明显卡顿尖峰。

### 8.3 功能回归

关注项：

- 页面自动切换仍正常。
- 控件输入、键盘输入、鼠标点击拖动不失效。
- 不引入漏刷、残影、撕裂。

## 9. 最终结论

本问题的本质不是某一处算法慢，而是当前 SDL Demo 运行模型对桌面端不友好：

- 固定 60Hz 持续刷新
- 输入无降采样
- SDL 端整屏搬运/整纹理上传
- 双线程轮询和 1ms 忙等同步
- `EXPOSED` 事件直接整屏重绘

因此修复也不应只盯着单点，而应按“先去忙轮询，再去整屏刷新，再收口输入与窗口事件”的顺序推进。只要阶段 A 和阶段 B 落地，空闲 CPU 与拖动卡顿通常就会先出现量级改善；阶段 C 和阶段 D 负责把交互体验进一步收稳。
