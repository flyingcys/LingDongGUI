# TinyUI Port 分层与适配规则

## 目的

本文定义 TinyUI 当前 `src`、`port`、`demo` 三类目录的职责边界，避免把固定核心代码、平台适配代码、示例代码混在一起。

## 总原则

1. `tinyui/src/` 放 TinyUI 固定核心实现。
2. `tinyui/port/` 放开发者按芯片、OS、屏幕、输入设备自行适配的 port。
3. `tinyui/demo/` 放示例应用，不承担平台适配职责。
4. `tinyui/src/internal bridge/` 是 历史 LingDongGUI/ARM-2D 桥接规划，当前无 backend 硬指标下不作为 TinyUI 质量依据。

## 目录职责

### `tinyui/src/`

适合放这里的内容：

- TinyUI core 生命周期
- 固定 public contract 的默认状态实现
- widget/layout/theme/event/resource 等通用逻辑
- runtime/resource/internal bridge

约束：

- 这层代码默认直接进入 TinyUI 编译目标。
- 这层不要求开发者按板卡或芯片修改。
- 不把板级差异、OS 差异、显示驱动差异、输入驱动差异塞进这里。

当前 port 相关 core 状态保留在：

- `tinyui/src/display/`
- `tinyui/src/indev/`
- `tinyui/src/tick/`
- `tinyui/src/osal/`

这些目录表达的是 TinyUI 内部 contract/state，不是开发者要改的 board port。

### `tinyui/port/`

适合放这里的内容：

- SDL host port
- 板卡 port
- RTOS/裸机 port
- 屏幕 flush、触摸采样、tick source、delay/lock、文件系统/资源入口等平台适配代码

约束：

- 这层是开发者可能需要改的代码。
- 不同项目可以只编译自己需要的 port。
- 不应默认把所有 `tinyui/port/*` 全量编进所有目标。
- port 的职责是把外部平台能力接到 TinyUI public contract，不直接承载业务 demo。

推荐形态：

```text
tinyui/port/sdl/
tinyui/port/<board_name>/
tinyui/port/<rtos_name>/
```

### `tinyui/demo/`

适合放这里的内容：

- API 用法示例
- 视觉/交互演示
- smoke/runtime 展示页面

约束：

- demo 只能表达用户意图，不承担适配补丁职责。
- demo 不应包含板级驱动、SDL 初始化细节、触摸驱动细节。
- demo 不应替代 port。

## 固定编译与按需编译

### 必须固定编译

默认进入 TinyUI 主库或 TinyUI 支撑库：

- `tinyui/src/core/*`
- `tinyui/src/widgets/*`
- `tinyui/src/layout/*`
- `tinyui/src/theme/*`
- `tinyui/src/display/*`
- `tinyui/src/indev/*`
- `tinyui/src/tick/*`
- `tinyui/src/osal/*`
- 无 backend 目录，本轮不编译 backend 私有层

### 应按需选择编译

按目标平台显式选择：

- `tinyui/port/sdl/*`
- `tinyui/port/<board>/*`

### 可以不编译

- `tinyui/demo/*`
- 专门给 demo 服务的临时 runner

## internal runtime 与 port 的边界

### TinyUI internal runtime 负责

- 把 TinyUI widget/app 状态映射到底层 LingDongGUI 对象
- 从 TinyUI display/indev/tick/osal 状态读取数据
- 维持 TinyUI 与 LingDongGUI 的私有桥接

### port 负责

- 把 SDL/板卡/OS 的真实能力写入 TinyUI public contract
- 提供 display config、pointer/key 输入、tick source、delay/lock 等适配

### 明确禁止

- 不把 底层私有实现挪进 `tinyui/port/ldgui`
- 不把 demo 页面逻辑写进 port
- 不把平台驱动细节散落回 `tinyui/src/core` 或 widget 实现

## 推荐开发流程

1. 先在 `tinyui/src/` 定义稳定的 TinyUI contract。
2. 再在 `tinyui/port/<target>/` 实现具体平台适配。
3. 最后用 `tinyui/demo/` 或 runtime test 验证效果。

若需求来自新芯片或新宿主环境，优先问自己：

- 这是 TinyUI 固有能力吗？
  - 是：进 `tinyui/src/`
- 这是平台差异吗？
  - 是：进 `tinyui/port/`
- 这只是示例页面吗？
  - 是：进 `tinyui/demo/`
