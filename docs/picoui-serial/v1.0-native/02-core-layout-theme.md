# PicoUI v1.0 Native P2 Core Layout Theme

## P2-A Native widget tree contract

状态：已完成 `P2-A` 的 contract 文件与最小 native tree foundation。

本阶段目标：

- 建立最小 native widget tree contract
- 覆盖 `parent / first_child / next_sibling / root / screen`
- 不在 `native_widget.c` 里调用 `picoui_backend_widget_create()` 或任何 `ld*`

本阶段新增：

- `tests/picoui/native/test_picoui_native_widget_tree.c`
- `picoui/src/native/native_widget.c`
- `cmake/LingDongGUI.cmake` 已把 `native_widget.c` 接到 `picoui_core`

repo reality 说明：

- plan 示例若写成 `picoui_window_create_root(screen)`，当前仓库真实签名仍是
  `picoui_window_create_root(screen, "root")`
- `P2-A` test 按当前 header reality 使用后者，没有改 public header

当前实现边界：

- 允许继续复用现有 backend create 路径创建 `window / label / button`
- `native_widget.c` 只建立最小 private native tree registry
- 绑定入口是 `picoui_native_widget_bind_root(screen, root_window)` +
  `picoui_native_widget_bind_child(parent, child)`
- `parent / first_child / next_sibling / root` 的公开查询在 `P2-A` 改为优先读 native registry，查不到时再 fallback 旧 backend 路径
- `screen` 通过 `picoui_widget_get_screen(widget)` 从 native registry 返回

已证明：

- `init -> active screen -> root window -> label A -> button B` 链路下，可以显式绑定 root tree
- `label` 当前真实创建路径还没有稳定的 backend host bind，因此 `P2-A` 用显式 child bind 补齐 native tree contract
- 绑定后，`picoui_widget_get_screen(root)` 返回当前 active screen
- 绑定后，`picoui_widget_get_parent/first_child/next_sibling/root` 对 root/label/button 的结果成立

未证明：

- 还没有把所有 widget 创建路径自动接到 native registry
- 还没有覆盖 remove/destroy/reparent 后的 native tree 更新
- 还没有进入 `P2-D`

## P2-B Native geometry / dirty contract

状态：已完成 `P2-B` 的最小 geometry + internal dirty foundation。

本阶段目标：

- 覆盖 `set_pos / set_size / get_x / get_y / get_width / get_height`
- 建立 native-only dirty helper
- 验证 dirty 能从 widget 冒泡到 screen / default display
- 不凭空新增 public `picoui_widget_is_dirty()` header contract

本阶段新增：

- `tests/picoui/native/test_picoui_native_geometry_dirty.c`
- `picoui/src/native/native_dirty.c`
- `picoui/src/native/native_widget.c` 增加 native dirty mark bridge
- `picoui/src/core/widget.c` 的 `set_pos / set_size` 现会触发 native dirty mark

repo reality 说明：

- `geometry` 本来就已经是 public API：`set_pos / set_size / get_x / get_y / get_width / get_height`
- `dirty` 在 spec 中是 native runtime/render 内部职责，不是当前 public header 已存在合同
- 因此 `P2-B` 没有把 `picoui_widget_is_dirty()` 加进 public header；测试改走 native-only helper

当前实现边界：

- `native_dirty.c` 只记录最近一次被置脏的 `widget / screen / default display`
- 当前 dirty 证明的是最小 invalidation behavior，不是完整 dirty-region 系统
- `screen` 通过 `native_widget` registry 关联，`display` 通过 `picoui_display_get_default()` 关联
- 还没有接入 render scheduling、dirty rectangle merge、clear-after-flush

已证明：

- `picoui_widget_set_pos()` 与 `picoui_widget_set_size()` 后，geometry getter 返回最新值
- 上述写操作会把目标 widget 标记为 dirty
- 同一次写操作会把 owning screen 和 default display 标记为 dirty
- `P1` 相关 native runtime/tree/input/render smoke 没被 `P2-B` 打回

未证明：

- 还没有做多 widget/multi-dirty merge
- 还没有做 clear/ack dirty 生命周期
- 还没有把 dirty 挂到真实 render/flush 调度
- 还没有进入 `P2-D`

## P2-C Native flex layout contract

状态：已完成 `P2-C` 的最小 native flex layout foundation。

本阶段目标：

- 建立最小 native flex layout contract
- 覆盖 `ROW / COLUMN` 单轨布局
- 让 child geometry 由 native layout 计算，不依赖 ld window layout
- 不提前扩到 `wrap / justify / align / grow / new_track`

本阶段新增：

- `tests/picoui/native/test_picoui_native_flex_layout.c`
- `picoui/src/native/native_layout.c`
- `cmake/LingDongGUI.cmake` 已把 `native_layout.c` 接到 `picoui_core`

repo reality 说明：

- `picoui/src/layout/flex.c` 当前 public setter 仍会写 window state，并继续调用现有 backend setter
- `P2-C` 先新增 private native entry `picoui_native_layout_apply_root(root_window)` 作为 contract 入口
- 这一步没有去重写旧的 `tests/picoui/unit/test_picoui_layout.c`，因为那批测试仍然主要在证明老 ld parity

当前实现边界：

- `native_layout.c` 目前只支持：
  - `PICOUI_FLEX_FLOW_ROW`
  - `PICOUI_FLEX_FLOW_COLUMN`
  - `main/cross/track align = START`
- 当前明确不支持：
  - `ROW_WRAP / COLUMN_WRAP`
  - `*_REVERSE`
  - `justify / align` 其他模式
  - `flex_grow`
  - `flex_new_track`
- child 若 `ignore_layout != 0` 会被跳过，不参与本轮 geometry 排布

已证明：

- 300x100 root + row flex + item gap 10 + 三个 `50x20` child 的 native layout 结果稳定为 `x=0/60/120`
- `P2-C` 的 native layout 计算通过 public geometry getter 可读出
- `P1`、`P2-A`、`P2-B` 相关 native tests 没被 `P2-C` 打回

未证明：

- 还没有 `wrap`
- 还没有 `reverse`
- 还没有 `justify/cross/track align`
- 还没有 `grow/new_track`
- 还没有进入 `P2-E`

## P2-D Native grid layout contract

状态：已完成 `P2-D` 的最小 native grid layout foundation。

本阶段目标：

- 建立最小 native grid layout contract
- 覆盖固定 track、gap、cell assignment、row/column span
- 让 child geometry 由 native layout 计算，不依赖 ld window grid layout
- 不提前扩到 percent track、复杂对齐或更广 demo 子集

本阶段新增：

- `tests/picoui/native/test_picoui_native_grid_layout.c`
- `picoui/src/native/native_layout.c` 增加 grid 分支

repo reality 说明：

- `picoui/src/layout/grid.c` 的 public setter 仍继续写 `window` / `widget` 上的公开状态，并调用现有 backend setter
- `P2-D` 继续复用 private native entry `picoui_native_layout_apply_root(root_window)`，没有再扩新的 public API
- 当前 grid 分支直接读取：
  - `window->grid_cols / grid_rows / grid_col_count / grid_row_count / grid gaps`
  - `widget->grid_col / grid_row / grid_col_span / grid_row_span`

当前实现边界：

- 目前只支持 fixed tracks
- 目前只支持最小 `START` 几何落点
- 目前不处理 percent tracks
- 目前不处理复杂 align
- 目前不处理 auto sizing / intrinsic sizing

已证明：

- 2x2 fixed grid 下：
  - child `A` 在 `(0,0)`
  - child `B` 在右侧单元
  - child `C` 从第二行起跨两列
- gap 会参与跨列宽度计算
- `P1`、`P2-A`、`P2-B`、`P2-C` 的 native tests 没被 `P2-D` 打回

未证明：

- 还没有 percent tracks
- 还没有复杂 align
- 还没有更广 grid parity/demo 子集
- `P2-D` 本身没有覆盖 style/theme，已交给 `P2-E`

## P2-E Native theme/style contract

状态：已完成 `P2-E` 的最小 native style/theme foundation。

本阶段目标：

- 建立 native style cache/readback contract
- 覆盖 background color、text color、border color、radius、padding
- 覆盖 direct public setter 与 `picoui_theme_apply_to_widget()` 两条写入路径
- 不新增 public style getter，不把 native helper 暴露成 public API

本阶段新增：

- `tests/picoui/native/test_picoui_native_style_theme.c`
- `picoui/src/native/native_style.c`
- `picoui/src/core/widget.c` 的 style setter 写入 native cache
- `picoui/src/theme/theme.c` 的 theme apply 写入 native cache

repo reality 说明：

- 当前 `label` 公开 getter 仍走旧 backend readback；`P2-E` 不把这条旧语义扩成完成标准
- `picoui_window_create_root()` 仍是 `P1` compatibility shim，会创建未挂 theme 的 compat app
- 因此 `picoui_theme_apply_to_widget()` 在 `P2-E` 中先更新 widget/native cache；只有 backend theme 可用时才同步旧 ld backend

当前实现边界：

- `native_style.c` 使用固定容量 side table，按 widget + part + state 记录颜色
- radius/padding 当前按 widget 记录标量值，不区分 part/state
- 当前覆盖到 `PICOUI_PART_MAIN` 和 `DEFAULT/PRESSED` 合同样例
- 未实现 inheritance rules
- 未实现 border width
- 未实现 padding left/right/top/bottom 分量

已证明：

- direct setter 写入后，native readback 能读回 root bg、label bg/text、button border/radius/padding
- theme apply 到 button `MAIN/PRESSED` 后，native readback 能读回 active bg/border 与 radius/padding
- backend theme 已挂载时，旧 `test_picoui_theme` 仍保持通过
- `P1`、`P2-A`、`P2-B`、`P2-C`、`P2-D` 的 native tests 没被 `P2-E` 打回

未证明：

- 还没有 style inheritance
- 还没有 border width
- 还没有 per-side padding
- 还没有 disabled/focused 的更广 native state matrix
- 还没有把 style cache 接入真实 ARM-2D render

## P2-F Closeout

状态：已完成 `P2` closeout。

本阶段验证：

- `ctest --test-dir build -R 'widget_tree|geometry_dirty|flex_layout|grid_layout|style_theme' --output-on-failure`
- `ctest --test-dir build -R 'test_picoui_native_runtime|test_picoui_native_display_indev|test_picoui_native_screen|test_picoui_native_screen_window|test_picoui_native_render_window_label_button|test_picoui_native_input_button|test_picoui_native_widget_tree|test_picoui_native_geometry_dirty|test_picoui_native_flex_layout|test_picoui_native_grid_layout|test_picoui_native_style_theme' --output-on-failure`
- `ctest --test-dir build -R '^test_picoui_theme$' --output-on-failure`
- `git diff --check`

P2 总结：

- native widget tree 最小 contract 已建立
- geometry dirty 最小 contract 已建立
- flex/grid 最小 native layout contract 已建立
- style/theme native cache/readback 最小 contract 已建立

P2 未完成边界：

- 未完成完整 create/remove/destroy/reparent tree 生命周期
- 未完成 dirty-region merge/clear/flush 调度
- 未完成 flex wrap/reverse/align/grow/new_track
- 未完成 grid percent track/复杂 align/auto sizing
- 未完成 style inheritance、border width、per-side padding、完整 state matrix
- 未完成真实 ARM-2D render output artifact
