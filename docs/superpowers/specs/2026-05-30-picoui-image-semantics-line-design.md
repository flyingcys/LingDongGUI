# TINYUI Image 语义线设计

## 目标

- 为 `image` 剩余未闭环项建立稳定、诚实、可长期维护的 public 合同边界。
- 把 `image` 当前已经真实闭环的 layout / source / visible 能力，与仍然只是 metadata、明确 reject、或暂时 deferred 的语义彻底拆开。
- 为后续是否需要实现某个更小的 `image` 子线提供唯一 design truth，而不是继续把 `image` broad gap 混写成“一整组通用 setter 都未闭环”。

## 背景

- 当前 `image` 的真实 backend 映射已经存在于 `ldImage`：
  - `tinyui_image_create()` / `tinyui_image_create_with_props()` -> `ldImage_init`
  - `tinyui_image_set_source()` -> `ldImageSetImage`
- 当前 `image` 的位置、尺寸、可见性和布局相关能力已通过 `ldBase` 通用入口闭环。
- 但 `image` 的 style/state 剩余项并不处于同一语义层：
  - `style_class / user_data` 当前只是 TINYUI / backend wrapper metadata 存储
  - `bg_color / text_color / border_color / radius` 当前没有真实 image-style backend 承接点
  - `padding` 的 public 含义并未被定义清楚
  - `enabled` 没有 image-specific disabled 语义

## 非目标

- 不在本设计里为 `image` 新增真实视觉 style backend。
- 不把 `image` 强行纳入当前 text / button / list 已有的 theme/style 体系。
- 不把 `enabled` 偷换成 `visible`、`opacity`、`selectable` 或其他近似语义。
- 不在本线里顺手收口 `list style_class / user_data`。

## 当前事实冻结

### 已真实闭环的 image 能力

- `tinyui_image_create()`、`tinyui_image_create_with_props()`：真实落到 `ldImage_init`
- `tinyui_image_set_source()`：真实落到 `ldImageSetImage`
- `tinyui_widget_set_pos()` / `set_size()` / `set_visible()`：真实落到 `ldBase` 通用入口
- `tinyui_widget_set_flex_grow()` / `set_flex_new_track()` / `set_ignore_layout()` / `set_grid_cell()`：真实落到当前 layout backend

### 当前未闭环项的真实现状

- `tinyui_widget_set_style_class()` / `tinyui_widget_set_user_data()`
  - 当前只更新 TINYUI `widget` 与 backend wrapper 字段
  - 没有真实 `ldImage` 消费链
- `tinyui_widget_set_bg_color()` / `set_text_color()` / `set_border_color()` / `set_radius()`
  - 当前 `image` style backend 不存在
  - `backend_style_apply.c` 对 `PICOUI_BACKEND_WIDGET_IMAGE` 直接拒绝
- `tinyui_widget_set_padding()`
  - 当前 API 形状存在
  - 但 `image padding` 还没有被定义为 content inset、layout gap 或其他稳定语义
- `tinyui_widget_set_enabled()`
  - 当前没有 image-specific backend bridge
  - `tinyui_widget_set_enabled()` 只对 `list` 和 `switch` 有真实特化语义

## 合同结论

### image style_class / user_data

- 当前只承认 metadata 存储语义。
- 它们可以继续作为 TINYUI / backend wrapper 侧的附加信息存在，但在没有真实 `ldImage` 消费链之前，不承诺任何视觉或交互行为。
- 因此这两项在当前矩阵中继续保持 `incomplete_contract`，而不是 `support`。

### image bg_color / text_color / border_color / radius

- 当前明确维持 `reject`。
- 原因不是“差一点实现”，而是当前没有真实 image-style backend 承接点，也没有既有 `ldImage` public 合同能自然映射这些语义。
- 后续若要支持，必须重开更窄的 image-style 子线，而不是在当前 broad line 下把 reject 偷改成 incomplete。

### image padding

- 当前维持 `deferred`。
- `padding` 的问题不在于“有没有 setter”，而在于 public 语义没有冻结：
  - 如果它表示 image content inset，就需要真实 image content box 模型
  - 如果它表示 layout spacing，就不应挂在 `image` 自身 style 上
- 在语义未定义前，不适合把它写成 `support` 或 `reject`。

### image enabled

- 当前维持 `reject`。
- `image` 没有天然的 disabled 交互模型；也不存在 `ldImage disabled` 专属入口。
- 因此不能把 `enabled` 偷换成：
  - hidden
  - alpha/opacity
  - selectable
  - “不响应点击”一类未定义近似语义

## 设计选择

### 方案 A：metadata-only + reject/deferred 冻结（推荐）

- `style_class / user_data` 继续承认 metadata-only
- `bg_color / text_color / border_color / radius / enabled` 继续明确为 `reject`
- `padding` 继续 `deferred`
- 优点：
  - 完全贴合当前真实实现
  - 不会把不存在的 image-style/backend 语义误写成“差一点 support”
  - 便于后续单独挑更小的子线重开
- 缺点：
  - 当前不会新增任何 user-facing image style 能力

### 方案 B：把所有剩余项都收成 incomplete_contract

- 把 `image` 剩余 style/state 统一写成“已有 API、还没闭环”
- 缺点：
  - 会抹平语义差异
  - 把明确不存在真实承接点的项误表述成“已有方向、待实现”
- 不采用

### 方案 C：直接挑一个 image 实现子线开工

- 例如先做 `image enabled` 或 `image style_class`
- 缺点：
  - 目前还没把 broad 语义先冻结清楚
  - 很容易再次把 metadata、reject、deferred 混回一条 broad gap
- 不采用

### 选择结果

- 本线采用方案 A：先做 metadata-only + reject/deferred 的语义冻结。

## 架构边界

### TINYUI 层

- 继续保留现有 `image` public API 形状。
- 但文档与测试口径必须清楚区分：
  - 哪些 API 已有真实 backend 消费
  - 哪些只是 metadata
  - 哪些当前就是 reject / deferred

### backend 层

- 当前 backend 只负责：
  - `ldImage` 创建
  - source/tile 绑定
  - `ldBase` 通用 layout/visible 入口
- 当前不负责 image-specific style backend，也不负责 disabled image 语义。

### LingDongGUI 层

- 当前 `ldImage` 不是 text/button/list 那种带丰富 style/state 的控件。
- 因此在没有新 backend 合同前，不应把它外推成“天然能吃 style_class / bg_color / enabled”的控件。

## 测试与证据要求

- 当前 `image` 已有真实闭环的项，继续由既有 unit / mapping / visible 证据支撑。
- `style_class / user_data` 若后续仍只保持 metadata-only，则测试只能证明“值被存储”，不能把存储证据外推成行为 support。
- `reject` 项不应再通过微调测试或 wording 被弱化成 `incomplete_contract`。
- `deferred` 项应明确是“语义未定义，暂不承诺”，而不是“实现中断”。

## 验收标准

- `image` broad gap 不再被笼统写成“一整组通用 setter 都未闭环”。
- `style_class / user_data`、`reject`、`deferred` 三类边界被清楚拆开。
- 真相源能直接指导下一步：
  - 若继续推进 `image`，必须先挑一个更窄的子线
  - 而不是继续把 `image` broad gap 当实现任务硬做

## 对 G 线索引的影响

- `G-线剩余缺口合同决策入口.md` 应为 `Image 语义线` 增加 design truth 链接。
- `G-线计划索引.md` 应把当前下一条串行线记为 `Image 语义线`，直到该线完成实现或被更精细的子线取代。
- 当前阶段不直接改变矩阵里 `image` 剩余项的状态值；本线先冻结语义，后续若进入实现，再更新矩阵。
