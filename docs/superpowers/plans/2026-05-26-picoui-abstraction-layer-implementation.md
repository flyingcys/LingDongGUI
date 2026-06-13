# TINYUI Abstraction Layer Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 `TINYUI` 从“能编译、能弹窗、但主要靠假渲染器拼 UI”的状态，纠偏为“真实映射到 LingDongGUI 控件/布局/事件链”的抽象层。

**Architecture:** `TINYUI` 只负责 public API、状态归一和用户入口；`tinyui/src/backend/ldgui/` 必须把 `window/label/button/checkbox/switch/slider/text/image`、`flex/grid`、`theme/event` 真实映射到 `LingDongGUI` 现有对象树与渲染/事件系统。SDL 只作为 `LingDongGUI` 的宿主显示层，不再承担 `TINYUI` 专属假控件绘制职责。

**Tech Stack:** C11、CMake、LingDongGUI、ARM-2D、SDL2 host runtime、Python3 验证脚本、CTest、GitNexus

---

## 0. 当前状态与纠偏结论

### 0.1 当前主线真相

当前主线已经具备以下能力：

- `tinyui_*` 基础 API 已落地。
- `TINYUI` demo target 已可构建、启动。
- `tests/tinyui/runtime/check_tinyui_runtime.py` 已能验证启动/capture。
- `examples/sdl` 的 `USE_DEMO=0..6` 已有 runtime/capture 级证据。

但这不等于 `TINYUI -> LingDongGUI` 适配已经成立。

### 0.2 当前错误方向

以下内容必须视为**临时 smoke 方案**，不能继续扩展为主实现：

- `tinyui/src/backend/ldgui/backend_app.c`
  - 当前承担了 `TINYUI` 专属 SDL 开窗、假控件绘制、固定行高/间距/按钮宽度/slider 轨道长度等职责。
  - 这条线的本质是“fake preview renderer”，不是“真实 backend 适配”。
- `tinyui/demo/*/main.c`
  - 当前为配合假渲染器，已经出现 `set_size(...)` 这类强人工摆位/定尺寸补丁。
  - 这些补丁不能继续蔓延，否则会把 layout 问题伪装成 demo 代码问题。
- `tests/tinyui/runtime/check_tinyui_runtime.py`
  - 当前更适合做“窗口是否起来 + 是否有非空画面”的 smoke。
  - 不应继续用它证明“真实 backend 语义已成立”。

### 0.3 纠偏要求

从这一版计划开始，后续实现必须遵守：

1. **停止继续强化 fake renderer。**
2. **优先做真实 backend 对象映射。**
3. **layout/theme/event 的闭环要落在 `LingDongGUI` 真实对象树上。**
4. **SDL 只负责显示 `LingDongGUI` 的输出，不再单独为 `TINYUI` 造一套视觉系统。**

---

## 1. 文件结构与责任重划

### 1.1 保留并继续演进的文件

- `tinyui/include/tinyui/*.h`
- `tinyui/src/core/*`
- `tinyui/src/widgets/*`
- `tinyui/src/backend/ldgui/backend.h`
- `tinyui/src/backend/ldgui/backend_window.c`
- `tinyui/src/backend/ldgui/backend_label.c`
- `tinyui/src/backend/ldgui/backend_button.c`
- `tinyui/src/backend/ldgui/backend_checkbox.c`
- `tinyui/src/backend/ldgui/backend_switch.c`
- `tinyui/src/backend/ldgui/backend_slider.c`
- `tinyui/src/backend/ldgui/backend_text.c`
- `tinyui/src/backend/ldgui/backend_image.c`
- `tinyui/src/backend/ldgui/backend_layout.c`
- `tinyui/src/backend/ldgui/backend_event.c`
- `tinyui/src/backend/ldgui/backend_theme.c`
- `tests/tinyui/unit/*`
- `tests/tinyui/contract/*`
- `tests/tinyui/runtime/*`

### 1.2 降级为临时过渡层的文件

- `tinyui/src/backend/ldgui/backend_app.c`

责任调整：

- 只保留“最小 host smoke/runtime harness”
- 不再扩展固定控件画法
- 后续只负责：
  - demo 进程生命周期
  - 最小窗口打开/退出 smoke
  - 必要时的 capture 输出

### 1.3 下一阶段应该新增/拆分的文件

- Create: `tinyui/src/backend/ldgui/backend_runtime_bridge.c`
  - 负责把 TINYUI backend 根对象接到 `LingDongGUI` scene/page lifecycle
- Create: `tinyui/src/backend/ldgui/backend_style_apply.c`
  - 负责把 `theme/state/part/style` 映射到 `LingDongGUI` 控件属性
- Create: `tinyui/src/backend/ldgui/backend_widget_tree.c`
  - 负责 parent/child、id、根节点、生命周期绑定
- Create: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
  - 验证“不是 fake renderer”，而是真实 `LingDongGUI` 对象树

### 1.4 禁止继续做的事情

- 不再在 `backend_app.c` 里新增新的固定坐标、固定尺寸、固定 line/rect/circle 画法。
- 不再通过修改 `tinyui/demo/*/main.c` 去掩盖 backend/layout 缺口。
- 不再把“窗口能打开”当成“抽象层完成”的证据。

---

## 2. 下一阶段串行工作总览

这是一条**单线程、可连续执行 8 小时以上**的串行工作流。

工作原则：

- 每一步都必须有明确失败测试。
- 每一步都必须尽量只解决一种能力。
- 每一步完成后都要回归上一层能力，防止假闭环。

建议分为 7 个串行阶段：

1. **Stage A: 去 fake renderer 依赖，补真实 backend tree**
2. **Stage B: 真实 window/label/button/text/image 映射**
3. **Stage C: 真实 checkbox/switch/slider 映射**
4. **Stage D: 真实 flex/grid layout 映射**
5. **Stage E: 真实 event/message pipeline**
6. **Stage F: 真实 theme/state/part/style 映射**
7. **Stage G: demo/UI 验收、文档能力矩阵、删除临时方案**

---

## 3. 串行阶段详细计划

### Stage A: 去 fake renderer 依赖，补真实 backend tree

**目标：** 先让 TINYUI backend 真正拥有“backend widget -> LingDongGUI widget/root/tree”的结构，而不是单靠 runtime 自己遍历假节点。

**Files:**
- Modify: `tinyui/src/backend/ldgui/backend.h`
- Modify: `tinyui/src/backend/ldgui/backend_widget.c`
- Modify: `tinyui/src/backend/ldgui/backend_window.c`
- Create: `tinyui/src/backend/ldgui/backend_widget_tree.c`
- Test: `tests/tinyui/unit/test_tinyui_widgets.c`
- Test: `tests/tinyui/runtime/check_tinyui_runtime.py`

- [ ] **Step 1: 写失败测试，要求 backend root/tree 有真实 owner/parent/child 关系**

验证点：

- `window` 是唯一根。
- `label/button/...` 子控件挂在 `window` 下。
- 子控件顺序稳定。
- backend tree 不依赖 fake runtime 的线性表。

- [ ] **Step 2: 跑 unit test，确认当前只靠 `first_child/next_sibling` 的结构不足以表达真实 backend owner 语义**

Run:

```bash
ctest --test-dir build -R test_tinyui_widgets --output-on-failure
```

Expected:

- FAIL，原因是 backend tree 只能支撑 fake traversal，不能证明真实 LingDongGUI 绑定

- [ ] **Step 3: 实装 `backend_widget_tree.c`，统一树管理**

要求：

- 把 parent/child 插入逻辑从 `backend_widget.c` 抽出来。
- 增加 root/owner/backend-kind 校验 helper。
- 后续 widget create 都通过统一 helper 挂树。

- [ ] **Step 4: 跑 unit test，确认树结构通过**

Run:

```bash
ctest --test-dir build -R test_tinyui_widgets --output-on-failure
```

Expected:

- PASS

- [ ] **Step 5: Commit**

```bash
git add tinyui/src/backend/ldgui/backend.h \
        tinyui/src/backend/ldgui/backend_widget.c \
        tinyui/src/backend/ldgui/backend_window.c \
        tinyui/src/backend/ldgui/backend_widget_tree.c \
        tests/tinyui/unit/test_tinyui_widgets.c
git commit -m "refactor(tinyui): add backend widget tree helpers"
```

### Stage B: 真实 window/label/button/text/image 映射

**目标：** 先打通最容易落地的静态控件，证明 `TINYUI` 可以生成真实 `LingDongGUI` 对象，而不是画假 panel。

**Files:**
- Modify: `tinyui/src/backend/ldgui/backend_window.c`
- Modify: `tinyui/src/backend/ldgui/backend_label.c`
- Modify: `tinyui/src/backend/ldgui/backend_button.c`
- Modify: `tinyui/src/backend/ldgui/backend_text.c`
- Modify: `tinyui/src/backend/ldgui/backend_image.c`
- Modify: `tinyui/src/backend/ldgui/backend_app.c`
- Create: `tests/tinyui/runtime/check_tinyui_backend_mapping.py`
- Test: `tests/tinyui/runtime/check_tinyui_runtime.py`

- [ ] **Step 1: 写失败检查，证明 `hello_world/theme_showcase/settings_panel` 仍主要依赖 fake renderer**

建议方法：

- runtime 输出中增加一个 debug marker，区分：
  - `fake renderer path`
  - `real ldgui widget path`
- 当前应当仍走 fake path

- [ ] **Step 2: 运行检查，确认失败**

Run:

```bash
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
```

Expected:

- FAIL，提示 demo 仍由 fake renderer 输出

- [ ] **Step 3: 在 backend create 阶段真实创建 `ldWindow/ldLabel/ldButton/ldText/ldImage`**

要求：

- `backend_window.c` 创建真实 root/container
- `backend_label.c` 创建真实 label
- `backend_button.c` 创建真实 button
- `backend_text.c` 创建真实 text
- `backend_image.c` 创建真实 image

限制：

- 不要为了过图去在 runtime 里补坐标
- 坐标/尺寸仍通过 widget/layout 语义传递

- [ ] **Step 4: 让 `backend_app.c` 优先走真实 backend 输出，只保留 fake path 作为临时 fallback**

- [ ] **Step 5: 跑 runtime/capture 验证**

Run:

```bash
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
python3 tests/tinyui/runtime/check_tinyui_runtime.py
```

Expected:

- `backend_mapping` PASS
- `check_tinyui_runtime` PASS

- [ ] **Step 6: Commit**

```bash
git add tinyui/src/backend/ldgui/backend_window.c \
        tinyui/src/backend/ldgui/backend_label.c \
        tinyui/src/backend/ldgui/backend_button.c \
        tinyui/src/backend/ldgui/backend_text.c \
        tinyui/src/backend/ldgui/backend_image.c \
        tinyui/src/backend/ldgui/backend_app.c \
        tests/tinyui/runtime/check_tinyui_backend_mapping.py \
        tests/tinyui/runtime/check_tinyui_runtime.py
git commit -m "feat(tinyui): map static widgets to ldgui"
```

### Stage C: 真实 checkbox/switch/slider 映射

**目标：** 消灭 `basic_widgets` 当前“几条色块”问题，把交互控件映射到真实 `LingDongGUI` 控件。

**Files:**
- Modify: `tinyui/src/backend/ldgui/backend_checkbox.c`
- Modify: `tinyui/src/backend/ldgui/backend_switch.c`
- Modify: `tinyui/src/backend/ldgui/backend_slider.c`
- Modify: `tinyui/src/backend/ldgui/backend_event.c`
- Modify: `tinyui/demo/basic_widgets/main.c`
- Modify: `tests/tinyui/runtime/check_tinyui_runtime.py`
- Test: `tests/tinyui/unit/test_tinyui_widgets.c`

- [ ] **Step 1: 写失败测试，要求 `basic_widgets` 不再只是同质 panel 条块**

要求：

- checkbox/switch/slider 截图必须有可辨认内部结构
- 不能只验证“行颜色不同”

- [ ] **Step 2: 运行失败测试**

Run:

```bash
python3 tests/tinyui/runtime/check_tinyui_runtime.py
```

Expected:

- FAIL，指出 `basic_widgets` 仍是占位外观

- [ ] **Step 3: 真实创建 `ldCheckBox/ldSwitch/ldSlider` 并接值同步**

要求：

- `set_checked/set_value` 更新真实底层控件
- runtime 截图来自真实 `LingDongGUI` 形态
- 不再靠 `backend_app.c` 手动画 knob/track/checkmark

- [ ] **Step 4: 跑 `basic_widgets` 回归**

Run:

```bash
python3 tests/tinyui/runtime/check_tinyui_runtime.py
ctest --test-dir build -R test_tinyui_widgets --output-on-failure
```

Expected:

- PASS

- [ ] **Step 5: Commit**

```bash
git add tinyui/src/backend/ldgui/backend_checkbox.c \
        tinyui/src/backend/ldgui/backend_switch.c \
        tinyui/src/backend/ldgui/backend_slider.c \
        tinyui/src/backend/ldgui/backend_event.c \
        tinyui/demo/basic_widgets/main.c \
        tests/tinyui/runtime/check_tinyui_runtime.py \
        tests/tinyui/unit/test_tinyui_widgets.c
git commit -m "feat(tinyui): map interactive widgets to ldgui"
```

### Stage D: 真实 flex/grid layout 映射

**目标：** 去掉 demo 里用 `set_size(...)` 硬撑画面的补丁，让 `layout_flex/layout_grid/settings_panel` 真实依赖 `LingDongGUI` layout。

**Files:**
- Modify: `tinyui/src/backend/ldgui/backend_layout.c`
- Modify: `tinyui/src/widgets/window.c`
- Modify: `tinyui/src/core/widget.c`
- Modify: `tinyui/demo/layout_flex/main.c`
- Modify: `tinyui/demo/layout_grid/main.c`
- Modify: `tinyui/demo/settings_panel/main.c`
- Modify: `tests/tinyui/unit/test_tinyui_layout.c`
- Modify: `examples/sdl/tests/check_use_demo_capture.py`

- [ ] **Step 1: 写失败测试，要求 layout demo 不依赖 demo 侧硬编码尺寸补丁**
- [ ] **Step 2: 运行，确认当前失败**
- [ ] **Step 3: backend_layout 真实映射到 `ldWindowSetFlex*` / `ldWindowSetGrid*` / grid cell 语义**
- [ ] **Step 4: 删除不必要的 demo 强人工尺寸补丁**
- [ ] **Step 5: 回归 layout unit/runtime/capture**

Run:

```bash
ctest --test-dir build -R test_tinyui_layout --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_runtime.py
python3 examples/sdl/tests/check_use_demo_capture.py --demo 4 --build-dir build/capture-demo-4
python3 examples/sdl/tests/check_use_demo_capture.py --demo 5 --build-dir build/capture-demo-5
```

- [ ] **Step 6: Commit**

```bash
git add tinyui/src/backend/ldgui/backend_layout.c \
        tinyui/src/widgets/window.c \
        tinyui/src/core/widget.c \
        tinyui/demo/layout_flex/main.c \
        tinyui/demo/layout_grid/main.c \
        tinyui/demo/settings_panel/main.c \
        tests/tinyui/unit/test_tinyui_layout.c \
        examples/sdl/tests/check_use_demo_capture.py
git commit -m "feat(tinyui): map flex and grid to ldgui layout"
```

### Stage E: 真实 event/message pipeline

**目标：** 把当前大量“setter 触发 callback”的伪事件语义，收敛成真实底层事件上送。

**Files:**
- Modify: `tinyui/src/backend/ldgui/backend_event.c`
- Modify: `tinyui/src/widgets/button.c`
- Modify: `tinyui/src/widgets/checkbox.c`
- Modify: `tinyui/src/widgets/switch.c`
- Modify: `tinyui/src/widgets/slider.c`
- Modify: `tests/tinyui/unit/test_tinyui_button_events.c`
- Modify: `tests/tinyui/unit/test_tinyui_widgets.c`

- [ ] **Step 1: 写失败测试，区分 setter-path 与 native-event-path**
- [ ] **Step 2: 运行，确认当前事件主要还是 setter-path**
- [ ] **Step 3: backend_event 接真实消息队列/控件事件**
- [ ] **Step 4: 保留 setter 同步状态，但不再把它当唯一事件来源**
- [ ] **Step 5: 跑事件测试**

Run:

```bash
ctest --test-dir build -R 'test_tinyui_widgets|test_tinyui_button_events' --output-on-failure
```

- [ ] **Step 6: Commit**

```bash
git add tinyui/src/backend/ldgui/backend_event.c \
        tinyui/src/widgets/button.c \
        tinyui/src/widgets/checkbox.c \
        tinyui/src/widgets/switch.c \
        tinyui/src/widgets/slider.c \
        tests/tinyui/unit/test_tinyui_button_events.c \
        tests/tinyui/unit/test_tinyui_widgets.c
git commit -m "feat(tinyui): route native widget events through backend"
```

### Stage F: 真实 theme/state/part/style 映射

**目标：** 把现在 mostly contract-level 的 `theme/state/part` 变成真实样式应用。

> 现态回写（A6 完成后）：当前真实 backend style apply 已覆盖 `window/button/checkbox/switch/slider/label/text`；`image` 当前合同不是“默认支持”，而是**明确拒绝** `PICOUI_PART_MAIN`。后续若要支持 `image` style apply，必须单独新增底层可观察语义与测试，不能靠空实现返回成功。

**Files:**
- Modify: `tinyui/src/theme/theme.c`
- Create: `tinyui/src/backend/ldgui/backend_style_apply.c`
- Modify: `tinyui/src/backend/ldgui/backend_theme.c`
- Modify: `tests/tinyui/unit/test_tinyui_theme.c`

- [ ] **Step 1: 写失败测试，要求 style 改变能反映到真实 backend 控件**
- [ ] **Step 2: 运行，确认当前只改本地字段**
- [ ] **Step 3: 实装 style apply 路径**
- [ ] **Step 4: 跑 theme 测试与 demo smoke**

Run:

```bash
ctest --test-dir build -R test_tinyui_theme --output-on-failure
python3 tests/tinyui/runtime/check_tinyui_runtime.py
```

- [ ] **Step 5: Commit**

```bash
git add tinyui/src/theme/theme.c \
        tinyui/src/backend/ldgui/backend_style_apply.c \
        tinyui/src/backend/ldgui/backend_theme.c \
        tests/tinyui/unit/test_tinyui_theme.c
git commit -m "feat(tinyui): apply theme state and part to ldgui widgets"
```

### Stage G: 文档/能力矩阵/删除临时方案

**目标：** 在真实 backend 映射完成后，把文档与代码重新对齐，并把 fake renderer 退回纯 smoke。

> 现态回写（A7 已完成）：文档/能力矩阵、`backend_app.c` 收缩线与 TINYUI 主线门禁现已收口。`backend_app.c` 当前仍保留 host runtime、fallback marker、capture 等过渡职责，但它们都已统一标注为 `temporary smoke path`，不再被文档写成正式 backend 内核。

**Files:**
- Modify: `docs/superpowers/specs/2026-05-26-tinyui-abstraction-layer-design.md`
- Modify: `docs/superpowers/specs/2026-05-26-tinyui-lingdonggui-test-architecture-design.md`
- Modify: `docs/superpowers/plans/2026-05-26-tinyui-abstraction-layer-implementation.md`
- Modify: `README.md`
- Modify: `README.en.md`
- Modify: `tinyui/docs/demo_guide.md`
- Modify: `tinyui/src/backend/ldgui/backend_app.c`

- [ ] **Step 1: 写完成门禁检查清单**
- [ ] **Step 2: 明确文档中的“临时方案”与“真实 backend 完成态”**
- [ ] **Step 3: 把 `backend_app.c` 收缩成纯 host smoke/capture**
- [ ] **Step 4: 跑最终全量回归**

当前 Stage G 关闭口径：

- 已完成：`A1-A6` 文档回写、能力矩阵纠偏、`image` style apply 语义收紧、runtime/capture 证据层级标注。
- 已完成：`backend_app.c` 收缩为纯 host smoke/capture 主责，`Stage G` 全量门禁已按最新口径复跑通过。

Run:

```bash
python3 tests/tinyui/runtime/check_tinyui_runtime.py
ctest --test-dir build -L tinyui --output-on-failure
python3 examples/sdl/tests/check_use_demo_runtime.py --demo 0 --build-dir build/verify-demo-0
python3 examples/sdl/tests/check_use_demo_runtime.py --demo 6 --build-dir build/verify-demo-6
python3 examples/sdl/tests/check_use_demo_capture.py --demo 1 --build-dir build/capture-demo-1
python3 examples/sdl/tests/check_use_demo_capture.py --demo 2 --build-dir build/capture-demo-2
python3 examples/sdl/tests/check_use_demo_capture.py --demo 3 --build-dir build/capture-demo-3
python3 examples/sdl/tests/check_use_demo_capture.py --demo 4 --build-dir build/capture-demo-4
python3 examples/sdl/tests/check_use_demo_capture.py --demo 5 --build-dir build/capture-demo-5
```

- [ ] **Step 5: Commit**

```bash
git add docs/superpowers/specs/2026-05-26-tinyui-abstraction-layer-design.md \
        docs/superpowers/specs/2026-05-26-tinyui-lingdonggui-test-architecture-design.md \
        docs/superpowers/plans/2026-05-26-tinyui-abstraction-layer-implementation.md \
        README.md \
        README.en.md \
        tinyui/docs/demo_guide.md \
        tinyui/src/backend/ldgui/backend_app.c
git commit -m "docs(tinyui): align plan with real backend direction"
```

---

## 4. 这条串行工作为什么可以连续做 8 小时以上

因为它不是一个点状 patch，而是一条明确的能力链：

1. backend tree
2. static widget mapping
3. interactive widget mapping
4. layout mapping
5. native event pipeline
6. theme/style application
7. docs cleanup and fake-renderer rollback

每个阶段都能单独失败、单独验证、单独提交。  
即使中途暂停，也不会丢失上下文边界。

---

## 5. 当前执行建议

当前建议：若继续扩面，应在 `A线` 最终收口之后，按新目标重开阶段或新计划，而不是回到 `backend_app.c` 继续堆积新的过渡逻辑。

---

## 6. 自检结论

### 6.1 Spec coverage

这版计划已经覆盖：

- 为什么当前方向错
- 哪些文件应降级为临时方案
- 下一阶段真实 backend 映射路径
- 串行工作拆分
- 最终验收门禁

### 6.2 Placeholder scan

已避免：

- `TODO/TBD`
- “类似前一步”这类省略
- 不带文件路径的抽象步骤

### 6.3 类型/术语一致性

本计划统一使用：

- fake renderer / 临时 smoke
- 真实 backend 映射
- `TINYUI -> backend/ldgui -> LingDongGUI -> SDL host`

---

Plan updated in place: `docs/superpowers/plans/2026-05-26-tinyui-abstraction-layer-implementation.md`
