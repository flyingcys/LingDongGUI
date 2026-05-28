# PicoUI B线计划索引

- `A线` 收口索引：`docs/picoui-serial/A-线计划索引.md`
- `PicoUI` 总设计真相源：`docs/superpowers/specs/2026-05-26-picoui-abstraction-layer-design.md`
- `PicoUI` 测试架构真相源：`docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- `B线` 总设计真相源：`docs/superpowers/specs/2026-05-27-picoui-b-line-visible-ui-design.md`
- `B线` 总实施口径：`docs/superpowers/plans/2026-05-27-picoui-b-line-visible-ui-implementation.md`
- 当前仓库硬规则：`AGENTS.md`

## 当前状态

- `B线` 已按 `B0 -> B1 -> B2 -> B3 -> B4 -> B5` 顺序完成：
  - `B1` 建立了独立 visible gate：`tests/picoui/runtime/check_picoui_visible_ui.py`
  - `B2` 修正了 `LingDongGUI RGB565 framebuffer -> SDL ARGB8888 / PPM RGB888` 的显示与 capture 颜色链
  - `B3` 收口了 `picoui_basic_widgets_demo` 的可见基线
  - `B4` 把 visible gate 推广到 6 个 `picoui` demos，并让 `settings_panel` 的 `wifi/brightness` 不再走 fallback marker
  - `B5` 将 smoke gate / visible gate / closeout 口径写回文档
- `A线` 已完成的事情：
  - `PicoUI -> LingDongGUI` backend 主线已经打通
  - `window/label/button/checkbox/switch/slider/text/image`、`flex/grid`、`event`、`theme` 已有真实 backend 映射与 smoke 级验证
  - `runtime/capture` 已能证明 demo 可启动、可出首帧、可回归
- `B线` 启动前正视的新事实已被关闭：
  - `A线` 证明的是“backend 主线和 smoke 闭环成立”，**不是**“真实窗口里的 UI 已经达到可正常显示、可读、可验收”
  - `2026-05-27` 对 `picoui_basic_widgets_demo` 的复核已经证明：当前首帧虽然不是空白，也不是完全无像素，但画面仍明显不合格
  - 当前至少已确认两类可见问题：
    - 首帧存在重复列/重复控件现象，说明最终呈现和预期控件树不一致
    - 画面配色与可读性异常，不能把“有像素”当成“用户看起来正常”
  - 用户在真实运行 `picoui_basic_widgets_demo` 时反馈“UI 一片黑”，已通过 B2 的颜色链修正和 B3/B4 visible gate 收口处理

## B线目标

- **B线唯一目标已完成**：`PicoUI` 已从“backend 主线已通 + smoke 可出图”推进到“真实窗口里 UI 可正常显示、可读、可判定”的状态。

## 当前游标

- 当前活跃游标：`B5 closeout`
- 当前允许进入：后续新能力线必须另开计划，不在本文继续扩面
- 当前禁止进入：
  - 继续把 `runtime/capture` 非空首帧外推成“UI 已正常显示”
  - 回到 `backend_app.c` 堆 fake renderer 或 demo 侧硬编码

## 阶段导航

- `B0`：现状复核与验收边界冻结
- `B1`：真实可见证据链建立
- `B2`：窗口显示链与颜色链纠偏
- `B3`：`basic_widgets` 可见正确性收口
- `B4`：其他 `picoui` demos 可见正确性铺开
- `B5`：可见 UI gate、文档与 closeout

## 逐阶段入口

### B0 现状复核与验收边界冻结

- 当前目标：
  - 把 `A线` 与 `B线` 的证据边界彻底分开
  - 固定“什么叫 UI 正常显示”，不再让 smoke 与 visible UI 混用
- 当前明确结论：
  - `A线` 成功不等于 `B线` 成功
  - `B线` 的首要问题不是“再加控件/再补 API”，而是“当前真实窗口里的 UI 仍不可信”
  - `basic_widgets` 是 `B线` 的最小、最重要样本，后续可见性复核必须先从它开始
- 当前证据：
  - `python3 tests/picoui/runtime/check_picoui_runtime.py` 通过
  - `python3 tests/picoui/runtime/check_picoui_visible_ui.py --all` 通过
  - `python3 tests/picoui/runtime/check_picoui_backend_mapping.py` 通过
  - `ctest --test-dir build -L picoui --output-on-failure` 通过

### B1 真实可见证据链建立

- 当前目标：
  - 建立专门针对“人眼可见结果”的验证链，而不只是 build / launch / capture
  - 区分：
    - `capture 非空`
    - `窗口真的显示出可读 UI`
    - `显示内容与预期控件树一致`
- 必须完成的事情：
  - 为 `picoui_basic_widgets_demo` 建立可见验收基线
  - 固定真实窗口下的截图/观察/必要 marker 输出方式
  - 明确哪些检查只能证明 smoke，哪些检查才可证明 visible correctness
- 阶段完成判定：
  - 后续每个 `picoui` demo 都有统一的 visible evidence 入口
  - 不再出现“测试全绿，但人眼看到像黑屏/近黑屏/不可读”的证据错位
- 收口证据：
  - `check_picoui_visible_ui.py --demo basic_widgets` 从 B1 RED 变成 B3 PASS
  - `check_picoui_visible_ui.py --all` 覆盖 6 个 demo

### B2 窗口显示链与颜色链纠偏

- 当前目标：
  - 查清 `PicoUI` 真实窗口显示链为什么会和 dummy capture / smoke 证据出现偏差
  - 查清当前异常配色、近黑/误黑、颜色通道、渲染呈现路径的问题是否存在系统性偏差
- 优先排查方向：
  - `backend_app.c` 的 host present 路径是否与 capture/readback 路径存在颜色/像素解释差异
  - `LingDongGUI -> SDL` 的窗口显示链与 `PICOUI_CAPTURE_FILE` 的读回路径是否使用了不同的颜色语义
  - 当前默认 theme token、底层颜色映射与最终视觉呈现是否存在“技术上有像素，但用户视觉上不可读”的问题
- 阶段完成判定：
  - `PicoUI` 真实窗口与 capture 之间不再出现“一个看起来正常、一个看起来近黑/错色”的系统性偏差
  - `B线` 后续 visible gate 可以基于稳定的显示链继续推进
- 收口证据：
  - `backend_app.c` 将 `COLOUR_INT` framebuffer 按 `LD_CFG_COLOR_DEPTH=16` 转成 SDL `ARGB8888` 与 PPM `RGB888`
  - visible gate 不再出现 `color/readback` 或 `near-black/readability` 失败

### B3 `basic_widgets` 可见正确性收口

- 当前目标：
  - 把 `picoui_basic_widgets_demo` 变成 `B线` 的第一块真正可见样板
- 必须完成的事情：
  - 消灭重复列/重复控件/错误布局这类“画出来了但不对”的问题
  - 让 `switch/checkbox/slider/button/text/image` 在真实窗口里达到最小可读状态
  - 明确 `image` 在当前阶段是正常显示真实内容、占位显示、还是显式未实现；不能让它继续造成误判
- 阶段完成判定：
  - `basic_widgets` 在真实窗口里不再被用户描述为黑屏、近黑屏或不可读
  - `basic_widgets` 的视觉结果与预期控件树一致，不再靠“首帧非空”过关
- 收口证据：
  - `python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo basic_widgets` 通过
  - `image` 当前以真实 `ldImage` 对象和显式占位 mask 进入 visible evidence，不再造成“未说明状态”的误判

### B4 其他 `picoui` demos 可见正确性铺开

- 当前目标：
  - 把 `basic_widgets` 的 visible correctness 经验推广到其他 `picoui` demos
- 覆盖范围：
  - `hello_world`
  - `layout_flex`
  - `layout_grid`
  - `theme_showcase`
  - `settings_panel`
- 当前原则：
  - 顺序仍然从最小样本开始，不并行扩散到新的大功能面
  - `settings_panel` 若继续暴露 mixed-tree / bridge / placeholder 问题，应作为 `B线` 后段样本处理，不反向污染 `basic_widgets`
- 阶段完成判定：
  - 6 个 `picoui` demos 都有“真实窗口里可读、可判定”的 visible evidence
  - 不再只有 `runtime/capture` 级绿灯
- 收口证据：
  - `python3 tests/picoui/runtime/check_picoui_visible_ui.py --all` 通过
  - `settings_panel` 的 `title/wifi/brightness/apply` 均进入 `PICOUI_BACKEND_REAL_WIDGET_IDS`
  - `settings_panel` 不再输出 `PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK`

### B5 可见 UI gate、文档与 closeout

- 当前目标：
  - 把 `B线` 的 visible gate 写进正式文档和验证链
  - 明确 `A线` 与 `B线` 的最终边界
- 必须完成的事情：
  - 文档回写：哪些门禁只能证明 smoke，哪些门禁可证明真实可见 UI
  - 为 `picoui` demos 建立 visible matrix 或等价的可见验收清单
  - 关闭“黑屏 / 近黑屏 / 错色 / 重复列 / 不可读但测试为绿”这类证据错位
- 退出口径：
  - `B线` 完成时，`PicoUI` 不只是“backend 映射已闭环”，还必须达到“真实窗口里可正常显示”
- 收口证据：
  - smoke gate：`python3 tests/picoui/runtime/check_picoui_runtime.py`
  - visible gate：`python3 tests/picoui/runtime/check_picoui_visible_ui.py --all`
  - backend mapping gate：`python3 tests/picoui/runtime/check_picoui_backend_mapping.py`
  - repo gate：`ctest --test-dir build -L picoui --output-on-failure`

## 当前规则

- `B线` 的第一优先级是**真实可见 UI**，不是继续补新的 backend 能力面
- `B1` 未完成前，不进入 `B2`
- `B2` 未完成前，不进入 `B3`
- `B3` 未完成前，不进入 `B4`
- `B4` 未完成前，不进入 `B5`
- 所有阶段都禁止把 `capture` 非空首帧直接等价成“显示正常”
- 所有阶段都禁止为了“看起来有内容”重新把 fake renderer 扩回正式主线
- 所有阶段都禁止继续用 demo 侧硬编码来掩盖真实 visible correctness 缺口

## 当前明确做什么

1. 后续新能力线继续保留 `check_picoui_visible_ui.py --all` 作为 visible gate
2. 后续汇报继续区分 smoke gate、backend correctness gate、visible gate
3. 若新增 demo，必须同步加入 visible matrix 或等价检查

## 当前明确不做什么

1. 不先扩新的 `PicoUI` 控件或 public API
2. 不先继续追加 theme/style 花样能力
3. 不把“测试绿了”当成“用户眼里已经显示正常”
4. 不把 `A线` 的 closeout 直接包装成“UI 已经完成”

## 推荐阅读顺序

1. 先读 `docs/picoui-serial/A-线计划索引.md`
2. 再读 `docs/superpowers/specs/2026-05-26-picoui-abstraction-layer-design.md` 第 `24` 节
3. 再读 `docs/superpowers/specs/2026-05-27-picoui-b-line-visible-ui-design.md`
4. 再读 `docs/superpowers/plans/2026-05-27-picoui-b-line-visible-ui-implementation.md`
5. 最后读本文，确认当前游标和阶段顺序

## 退出口径

下面条件已同时成立，`B线` 已收口：

- `picoui_basic_widgets_demo` 在真实窗口里可正常显示、可读、可判定
- `capture` 证据与真实窗口可见结果不再互相打架
- `hello_world/layout_flex/layout_grid/theme_showcase/settings_panel` 也达到同级 visible correctness
- 文档明确区分了 smoke gate 与 visible UI gate
- 后续若继续扩展 `PicoUI`，不再需要先回头解决“窗口看起来像黑屏/近黑屏”的基础问题
