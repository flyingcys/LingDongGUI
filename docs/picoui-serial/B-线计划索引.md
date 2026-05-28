# PicoUI B线计划索引

- `A线` 收口索引：`docs/picoui-serial/A-线计划索引.md`
- `PicoUI` 总设计真相源：`docs/superpowers/specs/2026-05-26-picoui-abstraction-layer-design.md`
- `PicoUI` 测试架构真相源：`docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- `B线` 总设计真相源：`docs/superpowers/specs/2026-05-27-picoui-b-line-visible-ui-design.md`
- `B线` 总实施口径：`docs/superpowers/plans/2026-05-27-picoui-b-line-visible-ui-implementation.md`
- 当前仓库硬规则：`AGENTS.md`

## 当前状态

- `A线` 已完成的事情：
  - `PicoUI -> LingDongGUI` backend 主线已经打通
  - `window/label/button/checkbox/switch/slider/text/image`、`flex/grid`、`event`、`theme` 已有真实 backend 映射与 smoke 级验证
  - `runtime/capture` 已能证明 demo 可启动、可出首帧、可回归
- 但 `B线` 启动前必须正视一个新事实：
  - `A线` 证明的是“backend 主线和 smoke 闭环成立”，**不是**“真实窗口里的 UI 已经达到可正常显示、可读、可验收”
  - `2026-05-27` 对 `picoui_basic_widgets_demo` 的复核已经证明：当前首帧虽然不是空白，也不是完全无像素，但画面仍明显不合格
  - 当前至少已确认两类可见问题：
    - 首帧存在重复列/重复控件现象，说明最终呈现和预期控件树不一致
    - 画面配色与可读性异常，不能把“有像素”当成“用户看起来正常”
  - 用户在真实运行 `picoui_basic_widgets_demo` 时反馈“UI 一片黑”，应视为 `B线` 的一等输入症状，而不是拿 `A线` smoke 结果直接覆盖

## B线目标

- **B线唯一目标**：把 `PicoUI` 从“backend 主线已通 + smoke 可出图”推进到“真实窗口里 UI 可正常显示、可读、可判定”的状态。

## 当前游标

- 当前活跃游标：`B0`
- 当前允许进入：`B1`
- 当前禁止进入：
  - 在可见 UI 仍不可信之前，继续扩新的 `PicoUI` 能力面
  - 拿 `runtime/capture` 非空首帧继续外推“UI 已正常显示”

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
  - `picoui_basic_widgets_demo` dummy capture 可出图，但可见结果仍出现重复列和异常配色
  - 用户在真实运行时报告“UI 一片黑”

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

1. 先把 `picoui_basic_widgets_demo` 当成 `B线` 一号样本重新验证
2. 把“真实窗口里能否正常显示”提升为与 backend correctness 同等级的门禁
3. 优先修正显示链、颜色链、布局呈现链的 visible correctness 问题

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

只有下面条件同时成立，`B线` 才算收口：

- `picoui_basic_widgets_demo` 在真实窗口里可正常显示、可读、可判定
- `capture` 证据与真实窗口可见结果不再互相打架
- `hello_world/layout_flex/layout_grid/theme_showcase/settings_panel` 也达到同级 visible correctness
- 文档明确区分了 smoke gate 与 visible UI gate
- 后续若继续扩展 `PicoUI`，不再需要先回头解决“窗口看起来像黑屏/近黑屏”的基础问题
