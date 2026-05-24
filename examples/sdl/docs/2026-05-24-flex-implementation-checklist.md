# Flex 补齐任务清单

> 日期：2026-05-24  
> 目标：把 LingDongGUI 的 flex 从“单轨一维排队器”补成“多轨、带 child 语义的 flex 容器”  
> 范围：只补 flex 能力；**不**做统一 layout/style/property 抽象

## 1. 本轮完成标准

完成后，flex 至少需要满足：

1. flow 不再只限于 row / column
2. 内部从单轨算法升级成多 track 算法
3. child 具备最小 flex 元数据
4. `grow / new track / ignore layout` 至少进入第一轮主线
5. 对齐语义扩展到更接近 LVGL
6. 补齐 flex 行为测试矩阵与 demo

---

## 2. 涉及文件

### 核心实现

- `src/gui/ldWindow.h`
- `src/gui/ldWindow.c`
- `src/gui/ldWindowLayoutInternal.h`
- `src/gui/ldWindowLayoutInternal.c`
- `src/gui/ldBase.h`
- `src/gui/ldBase.c`

### 当前测试与 demo

- `examples/sdl/tests/layout/test_layout_window.c`
- `examples/common/demo/layout/uiLayout.c`
- `examples/sdl/CMakeLists.txt`

### 对照基线

- `third_party/lvgl/include/lvgl/layouts/lv_flex.h`
- `third_party/lvgl/src/layouts/flex/lv_flex.c`
- `third_party/lvgl/docs/src/common-widget-features/layouts/flex.mdx`
- `third_party/lvgl/examples/layouts/flex/flex_flow/lv_example_flex_flow.c`
- `third_party/lvgl/examples/layouts/flex/flex_align/lv_example_flex_align.c`
- `third_party/lvgl/examples/layouts/flex/flex_grow/lv_example_flex_grow.c`
- `third_party/lvgl/examples/layouts/flex/flex_new_track/lv_example_flex_new_track.c`
- `third_party/lvgl/examples/layouts/flex/flex_ignore_layout/lv_example_flex_ignore_layout.c`
- `third_party/lvgl/examples/layouts/flex/flex_rtl/lv_example_flex_rtl.c`
- `third_party/lvgl/tests/src/test_cases/test_align_flex.c`
- `third_party/lvgl/tests/src/test_cases/test_flex_grow.c`
- `third_party/lvgl/tests/src/test_cases/test_margin_flex.c`

---

## 3. A 组：容器级 flex 骨架补足

## A1. 扩容器配置语义

### 目标

先把容器能表达的 flex 语义补全。

### 步骤

- [x] 扩 `ldFlexFlow_t`
  - 至少补 `row_wrap`
  - 至少补 `column_wrap`
  - 至少补 `row_reverse`
  - 至少补 `column_reverse`
  - 若一次到位，可直接按 8 种 flow 设计
- [x] 扩主轴对齐枚举
  - 增加 `space-evenly`
  - 增加 `space-around`
- [x] 增加 track 对齐枚举
  - 相当于多轨情形下的 `align-content`
- [x] 拆分 gap 语义
  - `item gap`
  - `track gap`
- [x] 保持旧 `ldWindowSetGap()` 作为兼容入口
  - 若内部拆分 gap，旧接口至少要有可解释映射

### 验证

- [x] 编译通过
- [x] 旧 row / column demo 不回归
- [x] setter host-side 测试补齐

## A2. 重写为多 track 算法骨架

### 目标

让 wrap / reverse / track align 有落点，不再堆在单 for-loop 上。

### 步骤

- [x] 审视当前 `ldWindowApplyFlexLayout()` 单轨算法
- [x] 在内部 helper 中拆出 track 计算
  - `find_track_end`
  - `repos_track_items`
  - `place_track_content`
- [x] 让容器先分 track
- [x] 再让每个 track 内单独排 child
- [x] reverse / wrap / track align 都建立在这套骨架上
- [x] 为 grow 预留挂点
  - 即便 A2 阶段先不做 grow，也不要写死成“永不改尺寸”

### 验证

- [x] host-side 测试：row_wrap
- [x] host-side 测试：column_wrap
- [x] host-side 测试：row_reverse
- [x] host-side 测试：column_reverse
- [x] host-side 测试：track align

## A3. 容器级语义先形成最小 demo / test 基线

### 目标

先把容器的 flow / align 行为钉死，再进入 child 级 grow/new-track。

### 步骤

- [x] 在测试中形成 flow 子矩阵
- [x] 在 demo 中至少增加
  - flow 示例
  - align 示例
  - wrap 示例
- [ ] 若本轮纳入 RTL，单独做一个示例，不要混进其它场景

---

## 4. B 组：child 级 flex 语义补足

## B1. 给 child 增加最小 flex 元数据

### 目标

不做上层抽象，但补齐 flex 必需的 child 语义。

### 步骤

- [x] 在 `src/gui/ldBase.h` 增加最小字段
  - `flexGrow`
  - `flexInNewTrack`
  - `ignoreLayout`
- [x] 新增 `ldBaseSetFlexGrow()`
- [x] 新增 `ldBaseSetFlexNewTrack()`
- [x] 新增 `ldBaseSetIgnoreLayout()`
- [x] 所有 setter 触发父容器 layout dirty

### 验证

- [x] host-side 测试：`SetFlexGrow()` 回标父布局
- [x] host-side 测试：`SetFlexNewTrack()` 回标父布局
- [x] host-side 测试：`SetIgnoreLayout()` 回标父布局

## B2. 实现 grow / new track / ignore layout

### 目标

让 flex 开始具备真实的空间分配与 child 控制能力。

### 步骤

- [x] `grow`
  - 按剩余主轴空间按权重分配
  - 真正修改 child 主轴尺寸
- [x] `new track`
  - 仅在 wrap 模式生效
  - 被标记 child 强制成为新轨起点
- [x] `ignore layout`
  - child 保持可见
  - 但不参与 flex 槽位分配
  - 保持手工定位
- [x] 明确 hidden 与 ignore-layout 的区别
  - hidden：不显示
  - ignore-layout：显示，但不参与布局

### 验证

- [x] host-side 测试：两个 grow child 按权重 1:2 分配
- [x] host-side 测试：new track 确实强制换轨
- [x] host-side 测试：ignore-layout child 不占 flex 槽位
- [x] host-side 测试：hidden 与 ignore-layout 行为不同

## B3. 最小尺寸联动闭环

### 目标

避免 grow 一进主线就出现溢出、抖动、假对齐。

### 步骤

- [x] grow 以前，明确 child base size 取值
- [x] wrap 计算时把 gap 算进剩余空间
- [x] 为 min/max 预留挂点
  - 即便第一轮只支持绝对值，也要把扩展点留好
- [x] 若本轮不做 margin / pct / content-size，必须在文档中明确记账

### 验证

- [x] host-side 测试：grow 后不超过容器主轴空间
- [x] host-side 测试：wrap 与 grow 组合时不出错

---

## 5. C 组：验证与 demo 补足

## C1. 补 flex 测试矩阵

### 最少补齐的用例

- [x] flow 子集或全量 flow
- [x] main align 全枚举
- [x] track align
- [x] hidden child
- [x] wrap
- [x] reverse
- [x] grow
- [x] new track
- [x] ignore layout
- [x] child 属性变化导致父布局重排

### 建议文件

- 继续扩 `examples/sdl/tests/layout/test_layout_window.c`
- 或把 flex 专属测试拆出去，避免单文件过大

## C2. 升级 flex demo

### 建议场景

- [x] row / column 基础流
- [x] wrap
- [x] align
- [x] grow
- [x] new track
- [x] ignore-layout overlay

### 修改点

- [x] 继续扩 `examples/common/demo/layout/uiLayout.c`
- [ ] 若 demo 文件已过大，拆出 flex demo helper

## C3. 文档同步

### 目标

让 flex 的说明文档和真实能力边界一致。

### 步骤

- [x] 更新 `examples/sdl/docs/2026-05-24-flex-vs-lvgl-gap-analysis.md`
  - 回写已完成项和延期项
- [x] README 已补 `USE_DEMO=4` 的 flex wrap/grow/new-track/ignore-layout 说明
- [x] tutorial 已补 `ldWindowSetFlexTrackAlign()`、`ldBaseSetFlexGrow()`、`ldBaseSetFlexNewTrack()` 等新增 API 说明
- [x] 若 RTL / margin / pct 不做，明确记账，不要模糊写成“已支持 flex”

## 8. 当前未完成项

- 已补 absolute `min/max` hook：child 可声明内部宽高 clamp 位，flex 会在 base size、wrap 判定与 grow 落地路径上统一经过该挂点
- README 与 tutorial 已同步新增 API / 示例说明；文档侧剩余工作主要是后续若继续补 RTL、margin、percent、content-size 时再继续记账

---

## 6. 推荐顺序

### 第一阶段

- A1 扩容器语义
- A2 多 track 算法骨架

### 第二阶段

- B1 child 最小元数据
- B2 grow / new track / ignore layout

### 第三阶段

- B3 最小约束联动
- C1 测试矩阵
- C2 demo 升级
- C3 文档同步

---

## 7. 完成判定

满足下面条件才算 flex 第一轮补齐完成：

- flow 不再只限 row / column
- wrap / reverse 已进入主线
- 内部已有多 track 算法骨架
- child 有最小 flex 元数据
- grow / new track / ignore layout 至少完成第一轮主线
- 测试与 demo 都不再只覆盖最小 row/column 场景
