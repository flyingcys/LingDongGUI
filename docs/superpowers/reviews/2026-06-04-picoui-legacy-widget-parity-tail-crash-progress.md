# TINYUI `legacy_widget_parity` 尾部崩溃收敛进展

## 目的

记录 `legacy_widget_parity` 在本轮 repair 里的最新真相，避免把“已推进很多”写成“已经闭环”。

## 当前结论

当前可以确认：

1. `check_tinyui_demo_boundary.py` 已恢复为 PASS。
2. `tinyui_legacy_widget_parity_demo` 已能完成编译和资源链接。
3. 启动崩溃不是单一“页面整体坏掉”，而是多个 native 文本/表格写入路径在低内存或分配失败时缺少判空。
4. 本轮已确认并修掉两处真实 root cause：
   - `ldTextSetText()`
   - `ldTableSetItemText()`
5. `legacy_widget_parity` 仍未闭环；崩溃点已从 `text` 段一路后移到 `qrcode` 之后的尾部区块。

## 本轮已完成修复

### 1. demo boundary 红灯已清掉

此前 `legacy_widget_parity/main.c` 直接泄漏了 `arm_2d_tile_t`，违反 demo 边界规则。

本轮已改成：

1. demo 继续只使用 `tinyui_*` API；
2. 不再在 demo 文件中显式声明 `arm_2d_*` 类型；
3. `check_tinyui_demo_boundary.py` 已重新通过。

### 2. 老图片资源已进入 demo 链接图

此前直接接老 `IMAGE_*` 宏时，`tinyui_legacy_widget_parity_demo` 会在链接阶段报大量 `c_tile_*` undefined symbols。

本轮已在：

1. `examples/sdl/CMakeLists.txt`

为 `tinyui_legacy_widget_parity_demo` 补入：

1. `WIDGET_IMAGE_SOURCES`

所以这页现在可以真实链接老 widget 图片对象，而不是停在“demo 源写了资源名但目标没把资源编进去”。

### 3. `text` 崩溃根因已确认并修掉

通过 ASan 和运行日志已确认：

1. 崩溃发生在 `legacy_ui:text_begin`
2. 调用链落到 `tinyui_backend_set_text -> ldTextSetText`
3. 真实原因不是 `text` 入参为空
4. 真实原因是 `ldCalloc(...)` 失败后，`ldTextSetText()` 继续对 `ptWidget->pStr` 做 `strlen(NULL)`

已修文件：

1. `src/gui/ldText.c`

修复方式：

1. 对 `pStr == NULL` 直接返回
2. `ldCalloc` 失败后不再继续 `strlen/strcpy`
3. 失败时把 text reader 重置为空串，避免继续读空指针

### 4. 同类文本 setter 已补最小防御

本轮顺手补齐了同类明显同构风险点：

1. `src/gui/ldLabel.c`
2. `src/gui/ldButton.c`
3. `src/gui/ldCheckBox.c`
4. `src/gui/ldQRCode.c`

这些修复都属于同一类：

1. 输入字符串判空
2. `ldCalloc` 失败后不再继续 `strcpy/strlen`

### 5. `table` 崩溃根因已确认并修掉

在 `text` 修掉后，崩溃点继续后移到：

1. `legacy_ui:table_begin`

随后确认：

1. `tinyui_table_set_cell_text -> tinyui_backend_table_set_cell_text -> ldTableSetItemText`
2. `ldTableSetItemText()` 与 `ldTextSetText()` 存在同类问题
3. 当 `ldCalloc` 失败时，代码仍继续走 `strcpy`

已修文件：

1. `src/gui/ldTable.c`

修复方式：

1. 对 `pText == NULL` 直接返回
2. 重新分配失败时直接返回，不再继续写入
3. 分配失败时把 `textMax` 回退到 `0`

## 当前崩溃推进到哪里

最新直接运行日志显示，这页已经稳定越过以下阶段：

1. `image`
2. `button`
3. `bar`
4. `text`
5. `slider_h`
6. `slider_v`
7. `list`
8. `combo`
9. `calendar`
10. `scroll`
11. `datetime`
12. `message`
13. `graph`
14. `table`
15. `icon_slider`
16. `qrcode`

最新日志停在：

1. `legacy_ui:qrcode_end`

所以当前剩余嫌疑区块主要是：

1. `gauge`
2. `line_edit`
3. `keyboard`
4. `arc`
5. `child_window`

## 当前验证状态

已确认通过：

1. `python3 tests/tinyui/contract/check_tinyui_demo_boundary.py`
2. `ctest --test-dir build/tinyui-runtime -R '^test_tinyui_text$' --output-on-failure`
3. `git diff --check`

已确认仍未通过：

1. `python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --demo legacy_widget_parity`
2. `SDL_VIDEODRIVER=dummy PICOUI_DEMO_AUTO_QUIT_MS=1200 ./build/tinyui-runtime/examples/sdl/tinyui_legacy_widget_parity_demo`

失败方式：

1. demo 仍在尾部区块崩溃退出

## 下一步建议

下一步不要回头重做已通过区块，只继续压缩尾部崩点：

1. 对 `gauge / line_edit / keyboard / arc / child_window` 单独打点或临时跳过
2. 确认第一个必崩 widget
3. 对对应实现先跑 GitNexus `impact`
4. 修复后撤掉当前 `legacy_ui:*` 临时日志

## 不能写成什么

当前仍不能写成：

1. `legacy_widget_parity` 已完成
2. `legacy_widget_parity` visible 已闭环
3. `legacy_widget_parity` 现在只剩资源 fidelity

当前真实说法只能是：

1. 本轮已把启动期崩溃从页面前半段连续推进到尾部少数 widget
2. 已修掉至少两处真实 native OOM 判空缺口
3. 页面仍未完全跑通
