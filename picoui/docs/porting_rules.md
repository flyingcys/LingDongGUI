# PicoUI Port 分层与适配规则

## 目的

本文定义 PicoUI 后续开发时 `src`、`port`、`demo` 三类目录的职责边界，避免把固定核心代码、平台适配代码、示例代码混在一起。

## 总原则

1. `picoui/src/` 放 PicoUI 固定核心实现。
2. `picoui/port/` 放开发者按芯片、OS、屏幕、输入设备自行适配的 port。
3. `picoui/demo/` 放示例应用，不承担平台适配职责。
4. `picoui/src/backend/ldgui/` 继续是 PicoUI 到 LingDongGUI/ARM-2D 的唯一私有桥接层。

## 目录职责

### `picoui/src/`

适合放这里的内容：

- PicoUI core 生命周期
- 固定 public contract 的默认状态实现
- widget/layout/theme/event/resource 等通用逻辑
- backend private bridge

约束：

- 这层代码默认应直接加入 PicoUI 编译目标。
- 这层不要求开发者按板卡或芯片修改。
- 不把板级差异、OS 差异、显示驱动差异、输入驱动差异塞进这里。

当前 port 相关 core 状态建议保留在：

- `picoui/src/display/`
- `picoui/src/indev/`
- `picoui/src/tick/`
- `picoui/src/osal/`

这些目录表达的是 PicoUI 内部 contract/state，不是开发者要改的 board port。

### `picoui/port/`

适合放这里的内容：

- SDL host port
- 板卡 port
- RTOS/裸机 port
- 屏幕 flush、触摸采样、tick source、delay/lock、文件系统/资源入口等平台适配代码

约束：

- 这层是“开发者可能需要改”的代码。
- 不同项目可以只编译自己需要的 port。
- 不应默认把所有 `picoui/port/*` 全量编进所有目标。
- port 的职责是把外部平台能力接到 PicoUI public contract，不直接承载业务 demo。

推荐形态：

```text
picoui/port/sdl/
picoui/port/<board_name>/
picoui/port/<rtos_name>/
```

例如：

- `picoui/port/sdl/`
- `picoui/port/mh2103c/`

### `picoui/demo/`

适合放这里的内容：

- API 用法示例
- 视觉/交互演示
- smoke/runtime 展示页面

约束：

- demo 只能表达用户意图，不承担适配补丁职责。
- demo 不应包含板级驱动、SDL 初始化细节、触摸驱动细节。
- demo 不应替代 port。

## “固定编译” 与 “按需编译”

### 必须固定编译

默认进入 PicoUI 主库或 backend 库：

- `picoui/src/core/*`
- `picoui/src/widgets/*`
- `picoui/src/layout/*`
- `picoui/src/theme/*`
- `picoui/src/display/*`
- `picoui/src/indev/*`
- `picoui/src/tick/*`
- `picoui/src/osal/*`
- `picoui/src/backend/ldgui/*`

原因：

- 这些是 PicoUI 自身实现。
- 使用 PicoUI public API 时默认就应存在。
- 不应要求开发者为“让 PicoUI 自己能工作”去手改这些源码。

### 应按需选择编译

按目标平台显式选择：

- `picoui/port/sdl/*`
- `picoui/port/<board>/*`
- 未来其他 host/board port

原因：

- 同一个项目不会同时需要所有平台 port。
- 不同平台的依赖不同，例如 SDL、裸机 BSP、RTOS。
- 这层天然是变体点。

### 可以不编译

- `picoui/demo/*`
- 专门给 demo 服务的临时 port 或 demo runner

原因：

- demo 不是 PicoUI 核心合同的一部分。
- 有些产品只要库，不要 demo。

## backend 与 port 的边界

### backend 负责

- 把 PicoUI widget/app 状态映射到底层 LingDongGUI 对象
- 从 PicoUI display/indev/tick/osal 状态读取数据
- 维持 PicoUI 与 LingDongGUI 的私有桥接

### port 负责

- 把 SDL/板卡/OS 的真实能力写入 PicoUI public contract
- 提供 display config、pointer/key 输入、tick source、delay/lock 等适配

### 明确禁止

- 不把 LingDongGUI backend 私有实现挪进 `picoui/port/ldgui`
- 不把 demo 页面逻辑写进 port
- 不把平台驱动细节散落回 `picoui/src/core` 或 widget 实现

## 推荐开发流程

1. 先在 `picoui/src/` 定义稳定的 PicoUI contract。
2. 再在 `picoui/port/<target>/` 实现具体平台适配。
3. 最后用 `picoui/demo/` 或 runtime test 验证效果。

若需求来自新芯片或新宿主环境，优先问自己：

- 这是 PicoUI 固有能力吗？
  - 是：进 `picoui/src/`
- 这是平台差异吗？
  - 是：进 `picoui/port/`
- 这只是示例页面吗？
  - 是：进 `picoui/demo/`

## 当前建议

结合当前仓库，推荐保持：

- `picoui/src/display/` `picoui/src/indev/` `picoui/src/tick/` `picoui/src/osal/`
  - 表达 PicoUI 内部固定 contract/state
- `picoui/port/sdl/`
  - 表达 SDL host port

这样可以同时满足两点：

1. `src` 下面的代码默认固定编译，开发者通常不改。
2. `port` 下面的代码明确是平台适配点，开发者可按目标平台维护。
