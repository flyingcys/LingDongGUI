# TINYUI Text Font 合同线设计

## 目标

- 为 `text | tinyui_text_set_font()` 定义可实现、可验证、且不扭曲现有 public API 的真实合同。
- 明确 `struct tinyui_font { family, size }` 在 TINYUI 中的语义边界，避免继续把 wrapper-only 指针存储误写成完整 backend 能力。
- 为后续 `Text Font 合同线` 的实现 plan 提供唯一 design truth，不在本设计里直接进入实现细节或越界收口其他剩余 gap。

## 背景

- 当前 `tinyui_text_set_font()` 只把 `font` 指针存入 TINYUI `widget` 与 backend wrapper。
- 当前测试最多只能证明 `widget.font == font` 且 `backend_widget->font == font`，不能证明真实 `ldText/text_box` 已消费该字体。
- `ldText` 当前在 `init` 时把 `ptFont` 注入 `text_box` 配置；运行期没有公开的 font rebinding 合同。
- `ldText_depose()` 在 `USE_VIRTUAL_RESOURCE == 1` 下会条件释放 `ptFont`，因此若把外部传入 font 直接视为 runtime 可替换资源，会引入 ownership / cleanup 风险。

## 非目标

- 不在本设计里定义全局字体主题系统。
- 不把 `label font` 既有合同回退或重做。
- 不把 `tinyui_font` 改成 handle 型 API，也不把现有 public API 重写成 token-only 接口。
- 不顺手扩展 `image`、`list metadata` 或其他 G 线剩余项。

## 合同结论

### TINYUI public 语义

- `struct tinyui_font { family, size }` 是描述值，不是 backend font 资源句柄。
- 调用方通过 `family` 与 `size` 表达“希望使用什么字体”，而不是把一个可长期借用、可被原样回传或必须按身份保存的 backend 对象交给 TINYUI。
- `tinyui_text_set_font()` 的成功语义应是：
  - TINYUI 接收这份字体描述。
  - backend 依据该描述解析并绑定真实字体资源。
  - `text` 的真实渲染对象同步完成字体更新与必要的 reflow。
- TINYUI 不承诺 `font` 指针身份保持，不承诺 backend 内部真实字体对象与输入指针一一对应。

### ownership 与生命周期

- 调用方拥有 `struct tinyui_font` 描述值本身；它只是一份输入描述，不是 backend 真实资源所有权的 transfer 载体。
- backend 负责真实字体资源的解析、缓存、fallback 和 cleanup 策略。
- `text` widget 不应直接拥有并释放调用方传入的 `struct tinyui_font`。
- runtime 字体切换后的真实 backend 字体对象生命周期，由 backend 内部合同负责；不能继续复用“直接覆写 `ldText->ptFont` 然后依赖 `ldText_depose()` 条件释放”的不透明路径。

### 默认值与失败语义

- `font == NULL` 表示“回退到 backend 默认字体”，不是未定义行为。
- `family` 无法识别时，backend 必须走明确 fallback，而不是把 text 留在不确定状态。
- `size <= 0` 或超出 backend 支持范围时，backend 必须采用明确归一化策略：
  - 要么钳制到允许范围；
  - 要么回退到默认 size；
  - 但无论选择哪种，都必须形成单一、稳定、可测试的语义。
- `tinyui_text_set_font()` 是否对“描述可接受但需 fallback”的情况返回成功，由后续实现 spec 固定；本设计只要求 fallback 语义显式、稳定、不可悬空。

## 方案对比与选择

### 方案 A：描述型合同

- 保留现有 `family + size` public shape。
- backend 负责描述解析、真实字体绑定与 fallback。
- 优点：
  - 与现有 public API 最一致。
  - 最符合 `TINYUI` 是上层抽象、`LingDongGUI` 是真实 backend 的分层。
  - 不要求调用方理解 `arm_2d_font_t` 或虚拟资源对象生命周期。
- 缺点：
  - backend 需要明确字体解析、rebind 和 cleanup 策略。

### 方案 B：resource-handle 合同

- 把 `tinyui_font` 实际变成稳定资源句柄或 backend-specific 对象入口。
- 优点：
  - 资源身份、缓存和生命周期更容易说清。
- 缺点：
  - 与现有 `struct tinyui_font { family, size }` 明显冲突。
  - 会把 G 线剩余项收口变成 public API 重定义。

### 方案 C：semantic token 合同

- 将字体抽象成 `body/title/caption` 等 token。
- 优点：
  - API 语义更高级，主题系统也更自然。
- 缺点：
  - 当前仓库没有现成 token font 系统。
  - 与现有 `family + size` 公开形状冲突最大。

### 选择结果

- 本线采用方案 A：描述型合同。
- 方案 B、C 不作为当前 `G` 线剩余缺口的直接推进方向。

## 架构边界

### TINYUI 层

- 继续暴露 `struct tinyui_font { family, size }`。
- `tinyui_text_set_font()` 对外只接受描述值语义。
- TINYUI 测试不再把“输入指针被 backend wrapper 原样保存”当成最终合同证据。

### backend 抽象层

- backend 接收 TINYUI font 描述，并负责把它解析为可供 `ldText/text_box` 使用的真实字体。
- backend 必须成为 runtime rebind 的唯一入口，不能要求 demo 或上层 widget 直接操作 `ldText` 内部字段。
- backend 还要隔离真实字体资源生命周期，避免 TINYUI public API 直接卷入 `arm_2d_font_t` ownership。

### LingDongGUI / Arm-2D 层

- `ldText` 与 `text_box` 的字体重绑、reflow 与 cleanup 语义必须在这一层形成稳定合同。
- 若底层需要新增 helper、re-init 路径或更明确的 font 生命周期策略，应由后续实现计划定义。
- 实现必须避免继续依赖“覆写 `ptFont` 指针但不重建 `text_box` 内部状态”的半闭环路径。

## 数据流

1. 调用方构造或传入 `struct tinyui_font { family, size }`。
2. `tinyui_text_set_font()` 接收描述值并更新 TINYUI widget 侧状态。
3. backend 解析 `family/size`，得到真实字体资源或 fallback 结果。
4. backend 将真实字体更新应用到 `ldText/text_box`。
5. `text` 在下一次渲染前完成必要的 layout / line metrics / reflow 同步。
6. widget 销毁时，真实字体资源 cleanup 按 backend 内部合同执行，不把调用方描述值当作可释放资源。

## 错误处理

- `text == NULL` 继续视为 API misuse，返回失败。
- `font == NULL` 不视为失败，而是显式回退默认字体。
- `family` 不支持、`size` 不合法、对应资源不存在，不能让 runtime 留在部分更新状态。
- backend 必须保证：
  - 要么完成“fallback 后的稳定更新”；
  - 要么明确返回失败且保留旧的稳定字体状态；
  - 不能出现 wrapper 状态已变但真实渲染对象未同步的分裂合同。

## 测试要求

后续实现计划至少要覆盖以下测试面：

- unit：
  - `font == NULL` 的默认字体语义。
  - 非法 `size` / 未知 `family` 的 fallback 或失败语义。
  - backend 失败时不留下 TINYUI/backend/render 三层分裂状态。
- contract：
  - 初始化路径和 runtime `set_font()` 路径都能导向一致的真实字体合同。
  - runtime 改 font 后，真实 `ldText/text_box` 已重绑并完成 reflow，不只是 wrapper 指针变化。
  - cleanup 不误释放调用方描述值，也不错误处理 backend 内部字体资源。
- mapping / visible：
  - 需要至少一条能证明真实 `LingDongGUI` 消费该字体合同的证据，而不是只验证 wrapper 存储。

## 验收标准

- `text | tinyui_text_set_font()` 不再依赖 wrapper-only 证据。
- public API 继续保持 `family + size` 形状，不新增 handle-only 或 token-only 前置要求。
- runtime 字体更新成为真实 backend 合同，而不是仅修改缓存字段。
- ownership / cleanup 语义明确，不再允许 `ldText_depose()` 与外部传入描述值之间形成不透明释放风险。

## 对 G 线索引的影响

- `G-线计划索引.md` 应把 `Text Font 合同线` 标记为“已完成设计决策，待进入 plan”。
- `G-线剩余缺口合同决策入口.md` 应为 `Text Font 合同线` 增加 design truth 链接。
- 在后续实现完成并通过验证前，能力矩阵中的 `text font` 状态仍保持 `incomplete_contract`。
