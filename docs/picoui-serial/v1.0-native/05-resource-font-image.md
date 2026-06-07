# P5 Resource / Font / Image / Text

## P5-A `resource handle design contract`

### RED

- 新增 focused native test：
  - [tests/picoui/native/test_picoui_native_resource.c](/Users/cys/embedded/LingDongGUI/tests/picoui/native/test_picoui_native_resource.c)
  - [tests/picoui/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/tests/picoui/CMakeLists.txt)
- 首轮真实 `RED`：
  - `make -C build test_picoui_native_resource`
  - 链接失败，缺少：
    - `picoui_resource_create_from_memory`
    - `picoui_resource_create_from_file`
    - `picoui_resource_ref`
    - `picoui_resource_unref`
    - `picoui_resource_get_type`
    - `picoui_resource_get_size`
    - `picoui_resource_get_refcount`
- 该 `RED` 证明：
  - 当前代码还不存在 `P5-A` 计划要求的 public resource handle
  - 现有 `picoui_image_source_from_vres()` / `picoui_font_from_vres()` 旧 helper 不能替代 `resource handle` 合同

### GREEN

- 新增最小 public contract：
  - [picoui/include/picoui/resource.h](/Users/cys/embedded/LingDongGUI/picoui/include/picoui/resource.h)
  - [picoui/include/picoui/picoui.h](/Users/cys/embedded/LingDongGUI/picoui/include/picoui/picoui.h)
- 新增最小 native 实现：
  - [picoui/src/native/native_resource.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_resource.c)
  - [cmake/LingDongGUI.cmake](/Users/cys/embedded/LingDongGUI/cmake/LingDongGUI.cmake)
- 当前建立的最小 resource handle contract：
  - `picoui_resource_create_from_memory()`：
    - 拒绝 `null` 指针
    - 拒绝 `size == 0`
    - 资源自持拷贝输入 bytes
  - `picoui_resource_create_from_file()`：
    - 拒绝 `null/empty path`
    - 资源自持文件内容与路径副本
  - `picoui_resource_ref()/unref()`：
    - 建立最小引用计数语义
    - `refcount` 归零时释放资源自身持有的数据
  - `picoui_resource_get_type/get_size/get_refcount()`：
    - `null` 输入走安全边界值
- focused test 当前证明：
  - `const memory` 资源可创建，`type/size/refcount` 正确
  - `file` 资源可创建，`type/size/refcount` 正确
  - `ref/unref` 可稳定增减引用计数
  - `null source` / 空输入边界拒绝成立

### 验证

- `cmake -S . -B build`
- `make -C build test_picoui_native_resource`
- `./build/tests/picoui/test_picoui_native_resource`
- `ctest --test-dir build -R '^test_picoui_native_resource$' --output-on-failure`

### 当前结论

- 当前 `P5-A resource handle design contract` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - PicoUI 已有独立的 public `resource handle` 入口
  - `const memory/file path/refcount/type/size` 的最小合同已成立
- 当前实现只建立 `resource handle` 本身，不代表 `P5-B ARM-2D font bridge`、`P5-C image tile/mask bridge`、`P5-D canvas primitive renderer` 已完成。
- 当前阶段下一任务推进到 `P5-B ARM-2D font bridge`。

## P5-B `ARM-2D font bridge`

### RED

- 新增 focused native test：
  - [tests/picoui/native/test_picoui_native_font_renderer.c](/Users/cys/embedded/LingDongGUI/tests/picoui/native/test_picoui_native_font_renderer.c)
  - [tests/picoui/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/tests/picoui/CMakeLists.txt)
- 首轮真实 `RED`：
  - `make -C build test_picoui_native_font_renderer`
  - 链接失败，缺少：
    - `picoui_native_font_default`
    - `picoui_native_font_render_text`
- 该 `RED` 证明：
  - 当前代码还不存在 `P5-B` 计划要求的独立 native font renderer 入口
  - 现有 `picoui_native_text_render()` 只记录 render state，还没有可验证的 software buffer dirty area

### GREEN

- 新增最小 public/native 入口：
  - [picoui/include/picoui/font.h](/Users/cys/embedded/LingDongGUI/picoui/include/picoui/font.h)
  - [picoui/include/picoui/native.h](/Users/cys/embedded/LingDongGUI/picoui/include/picoui/native.h)
  - [picoui/src/native/native_text_renderer.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_text_renderer.c)
  - [cmake/LingDongGUI.cmake](/Users/cys/embedded/LingDongGUI/cmake/LingDongGUI.cmake)
- 同步把 font resolve/render 接回现有 native text 路径：
  - [picoui/src/core/resource.c](/Users/cys/embedded/LingDongGUI/picoui/src/core/resource.c)
  - [picoui/src/native/native_text.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_text.c)
  - [tests/picoui/native/test_picoui_native_text.c](/Users/cys/embedded/LingDongGUI/tests/picoui/native/test_picoui_native_text.c)
- 当前建立的最小 `P5-B` 合同：
  - `picoui_native_font_default()`：
    - 返回 ARM-2D 默认 `6x8` 字体句柄
  - `picoui_font_resolve_native()`：
    - `VRES font` 继续解析到 `ldBaseGetVresFont()`
    - `Sans >= 20` 解析到 `ARM_2D_FONT_16x24`
    - 其他 / `null` 输入回退到 `ARM_2D_FONT_6x8`
  - `picoui_native_font_render_text()`：
    - 对 `ASCII` 文本做最小 software buffer 渲染
    - 对不支持字符走 `?` fallback glyph
    - 返回非空 dirty rect
    - 非法输入安全失败
  - `picoui_native_text_render()`：
    - 不再只记文本和宽度
    - 现在会执行 font resolve + software render
    - 可读回 dirty rect 和 changed pixel count
- focused test 当前证明：
  - 默认字体能把 ASCII 文本写进 software buffer
  - dirty area 非空
  - 无效输入不会假阳性写 dirty area
- native text focused test 当前证明：
  - 文本 render 后有非空 dirty rect
  - changed pixel count 大于 0
  - 现有 text/wrap render state 行为仍然成立

### 验证

- `cmake -S . -B build`
- `make -C build test_picoui_native_font_renderer test_picoui_native_text`
- `ctest --test-dir build -R '^test_picoui_native_font_renderer$' --output-on-failure`
- `ctest --test-dir build -R '^test_picoui_native_text$' --output-on-failure`
- `./build/tests/picoui/test_picoui_native_font_renderer`

### 当前结论

- 当前 `P5-B ARM-2D font bridge` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - PicoUI 已有独立的 native font renderer 入口
  - default font / ASCII render / fallback glyph / dirty area 合同已成立
  - 现有 native text render 已接入这条 font renderer 路径
- 当前实现只建立最小 font bridge，不代表 `P5-C image tile/mask bridge`、`P5-D canvas primitive renderer`、`P5-E qrcode bitmap renderer` 已完成。
- 当前阶段下一任务推进到 `P5-C image tile/mask bridge`。

## P5-C `image tile/mask/source bridge`

### RED

- 新增 focused native test：
  - [tests/picoui/native/test_picoui_native_image_renderer.c](/Users/cys/embedded/LingDongGUI/tests/picoui/native/test_picoui_native_image_renderer.c)
  - [tests/picoui/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/tests/picoui/CMakeLists.txt)
- 首轮真实 `RED`：
  - `make -C build test_picoui_native_image_renderer`
  - 链接失败，缺少：
    - `picoui_native_image_render_buffer`
- 该 `RED` 证明：
  - 当前代码还不存在 `P5-C` 计划要求的独立 native image renderer 入口
  - 现有 `picoui_native_image_render()` 只记录 source，还没有 tile copy / mask recolor 的 software buffer 合同

### GREEN

- 新增最小 native image renderer：
  - [picoui/src/native/native_image_renderer.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_image_renderer.c)
  - [picoui/include/picoui/native.h](/Users/cys/embedded/LingDongGUI/picoui/include/picoui/native.h)
  - [cmake/LingDongGUI.cmake](/Users/cys/embedded/LingDongGUI/cmake/LingDongGUI.cmake)
- 同步把 image render state 接回现有 native image 路径：
  - [picoui/src/native/native_image.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_image.c)
  - [tests/picoui/native/test_picoui_native_image.c](/Users/cys/embedded/LingDongGUI/tests/picoui/native/test_picoui_native_image.c)
- 当前建立的最小 `P5-C` 合同：
  - `picoui_native_image_render_buffer()`：
    - 接受 `img_tile` 作为 ARGB software pixel source
    - 无 mask 时直接 copy 像素
    - 有 `mask_tile` 时，对非零 mask pixel 应用 `mask_color`
    - 返回完整 dirty rect
    - 非法输入安全失败且不写 buffer
  - `picoui_native_image_render()`：
    - 不再只记 source
    - 现在会执行 software render
    - 可读回 dirty rect 和 changed pixel count
- focused test 当前证明：
  - `2x2` ARGB tile copy 到目标 buffer 像素正确
  - `2x2` masked tile 会对命中 mask 的像素应用 `mask_color`
  - invalid source 会失败且不污染 buffer
- native image focused test 当前证明：
  - image render 后可读回非空 dirty rect
  - changed pixel count 大于 0
  - 现有 source render state 行为仍然成立

### 验证

- `cmake -S . -B build`
- `make -C build test_picoui_native_image_renderer test_picoui_native_image`
- `ctest --test-dir build -R '^(test_picoui_native_image_renderer|test_picoui_native_image)$' --output-on-failure`

### 当前结论

- 当前 `P5-C image tile/mask/source bridge` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - PicoUI 已有独立的 native image renderer 入口
  - ARGB tile copy / mask recolor / invalid-source rejection 合同已成立
  - 现有 native image render 已接入这条 image renderer 路径
- 当前实现只建立最小 image bridge，不代表 `P5-D canvas primitive renderer`、`P5-E qrcode bitmap renderer`、`P5-F animation frame source` 已完成。
- 当前阶段下一任务推进到 `P5-D canvas primitive renderer`。

## P5-D `canvas primitive renderer`

### RED

- 新增 focused native test：
  - [tests/picoui/native/test_picoui_native_canvas_renderer.c](/Users/cys/embedded/LingDongGUI/tests/picoui/native/test_picoui_native_canvas_renderer.c)
  - [tests/picoui/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/tests/picoui/CMakeLists.txt)
- 首轮真实 `RED`：
  - `make -C build test_picoui_native_canvas_renderer`
  - 链接失败，缺少：
    - `picoui_native_canvas_render_buffer`
- 首轮测试合同修正：
  - `P5-D` 计划要求验证 `line/fill_rect/circle` primitive render
  - 当前 public header 不存在 `picoui_canvas_draw_rect()`
  - focused test 改为用四条 `picoui_canvas_draw_line()` 组成矩形边框，不发明新 public API
- 该 `RED` 证明：
  - 当前代码还不存在 `P5-D` 计划要求的独立 native canvas renderer 入口
  - 现有 `picoui_native_canvas_render()` 只同步 command kind/count，还没有 dirty area / pixel changed 的 software buffer 合同

### GREEN

- 新增最小 native canvas renderer：
  - [picoui/src/native/native_canvas_renderer.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_canvas_renderer.c)
  - [picoui/include/picoui/native.h](/Users/cys/embedded/LingDongGUI/picoui/include/picoui/native.h)
  - [cmake/LingDongGUI.cmake](/Users/cys/embedded/LingDongGUI/cmake/LingDongGUI.cmake)
- 同步把 canvas render state 接回现有 native canvas 路径：
  - [picoui/src/native/native_canvas.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_canvas.c)
  - [tests/picoui/native/test_picoui_native_animation_canvas.c](/Users/cys/embedded/LingDongGUI/tests/picoui/native/test_picoui_native_animation_canvas.c)
- 当前建立的最小 `P5-D` 合同：
  - `picoui_native_canvas_render_buffer()`：
    - 接受 `fill_rect` 命令并把像素写入 software buffer
    - 接受 `draw_line` 命令并用最小 Bresenham 路径写入 software buffer
    - 继续复用现有 circle 编码语义：
      - `draw_circle` 仍由 `picoui_canvas_draw_circle()` 编码成特殊 `draw_line`
      - renderer 识别 `x==x1 && y==y1 && line_size>0` 为 circle primitive
    - 返回非空 dirty rect
    - 非法输入安全失败且不改 buffer
  - `picoui_native_canvas_render()`：
    - 不再只同步 rendered command kind/count
    - 现在会执行 software render
    - 可读回 dirty rect 和 changed pixel count
- focused test 当前证明：
  - `fill_rect + line + line-rectangle + circle` 可把像素写进 software buffer
  - dirty area 非空
  - invalid input 会失败且不污染 buffer
- `animation_canvas` 回归 test 当前证明：
  - 原有 command count / command kind 合同仍成立
  - render 后可读回非空 dirty rect
  - changed pixel count 大于 0

### 调试记录

- 中途出现过 `test_picoui_native_canvas_renderer` 挂住
- root cause 已确认在 `native_canvas_renderer.c` 的 line raster 循环
  - 首版实现对 `err` 做了两次条件判定，但第二次判定复用了已更新后的 `err`
  - 某些斜率下会导致 `current_x/current_y` 不按标准 Bresenham 收敛
- 已改成单次 `e2 = 2 * err` 的标准判定路径后恢复稳定
- 调试期临时 `lldb/sample` 与 stderr instrumentation 已清理，不保留到最终代码

### 验证

- `cmake -S . -B build`
- `make -C build test_picoui_native_canvas_renderer test_picoui_native_animation_canvas`
- `./build/tests/picoui/test_picoui_native_canvas_renderer`
- `./build/tests/picoui/test_picoui_native_animation_canvas`
- `ctest --test-dir build -R '^(test_picoui_native_canvas_renderer|test_picoui_native_animation_canvas)$' --output-on-failure`
- `git diff --check -- picoui/include/picoui/native.h picoui/src/native/native_canvas_renderer.c picoui/src/native/native_canvas.c cmake/LingDongGUI.cmake tests/picoui/native/test_picoui_native_canvas_renderer.c tests/picoui/native/test_picoui_native_animation_canvas.c`

### 当前结论

- 当前 `P5-D canvas primitive renderer` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - PicoUI 已有独立的 native canvas renderer 入口
  - `fill_rect/line/circle` 的 software buffer render 合同已成立
  - 现有 native canvas render 已接入 dirty rect / changed pixel readback
- 当前实现只建立最小 canvas primitive renderer，不代表 `P5-E qrcode bitmap renderer`、`P5-F animation frame source`、`P5-G P5 closeout` 已完成。
- 当前阶段下一任务推进到 `P5-E qrcode bitmap renderer`。

## P5-E `qrcode bitmap renderer`

### RED

- 新增 focused native test：
  - [tests/picoui/native/test_picoui_native_qrcode_renderer.c](/Users/cys/embedded/LingDongGUI/tests/picoui/native/test_picoui_native_qrcode_renderer.c)
  - [tests/picoui/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/tests/picoui/CMakeLists.txt)
- 首轮真实 `RED`：
  - `make -C build test_picoui_native_qrcode_renderer`
  - 链接失败，缺少：
    - `picoui_native_qrcode_render_buffer`
- 该 `RED` 证明：
  - 当前代码还不存在 `P5-E` 计划要求的独立 qrcode bitmap renderer 入口
  - 现有 [native_qrcode.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_qrcode.c) 只证明 `qrcodegen encode + module_count` 成功，还没有 software buffer bitmap render 合同

### GREEN

- 新增最小 native qrcode renderer：
  - [picoui/src/native/native_qrcode_renderer.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_qrcode_renderer.c)
  - [picoui/include/picoui/native.h](/Users/cys/embedded/LingDongGUI/picoui/include/picoui/native.h)
  - [cmake/LingDongGUI.cmake](/Users/cys/embedded/LingDongGUI/cmake/LingDongGUI.cmake)
- 同步把 qrcode render state 接回现有 native qrcode 路径：
  - [picoui/src/native/native_qrcode.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_qrcode.c)
  - [tests/picoui/native/test_picoui_native_qrcode.c](/Users/cys/embedded/LingDongGUI/tests/picoui/native/test_picoui_native_qrcode.c)
- 同步修正 render-state invalidation：
  - [picoui/src/widgets/qrcode.c](/Users/cys/embedded/LingDongGUI/picoui/src/widgets/qrcode.c)
  - `picoui_qrcode_set_qr_color()` / `picoui_qrcode_set_bg_color()` 现在也会 reset native render state
- 当前建立的最小 `P5-E` 合同：
  - `picoui_native_qrcode_render_buffer()`：
    - 使用 `qrcodegen_encodeText()` 编码当前 payload
    - 按 `max_version/ecc/zoom` 生成 module bitmap
    - 用 `qr_color/bg_color` 写入 software buffer
    - 居中落到目标 buffer
    - 返回非空 dirty rect
    - 非法输入安全失败且不改 buffer
  - `picoui_native_qrcode_render()`：
    - 不再只写 `runtime_evidence_flags`
    - 现在会执行 software render
    - 可读回 dirty rect 和 changed pixel count
- focused test 当前证明：
  - payload `PICOUI` 可 render 到 software buffer
  - finder-pattern 三个角的代表像素为 dark module
  - dirty area 非空
  - invalid input 会失败且不污染 buffer
- `native_qrcode` 回归 test 当前证明：
  - 原有 text/ecc/module_count render state 合同仍成立
  - render 后可读回非空 dirty rect
  - changed pixel count 大于 0

### 验证

- `cmake -S . -B build`
- `make -C build test_picoui_native_qrcode_renderer test_picoui_native_qrcode`
- `./build/tests/picoui/test_picoui_native_qrcode_renderer`
- `./build/tests/picoui/test_picoui_native_qrcode`
- `ctest --test-dir build -R '^(test_picoui_native_qrcode_renderer|test_picoui_native_qrcode)$' --output-on-failure`
- `git diff --check -- picoui/include/picoui/native.h picoui/src/native/native_qrcode_renderer.c picoui/src/native/native_qrcode.c picoui/src/widgets/qrcode.c cmake/LingDongGUI.cmake tests/picoui/native/test_picoui_native_qrcode_renderer.c tests/picoui/native/test_picoui_native_qrcode.c tests/picoui/CMakeLists.txt`

### 当前结论

- 当前 `P5-E qrcode bitmap renderer` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - PicoUI 已有独立的 native qrcode bitmap renderer 入口
  - payload encode / finder-pattern pixel / dirty rect / changed pixel 合同已成立
  - 现有 native qrcode render 已接入这条 bitmap renderer 路径
- 当前实现只建立最小 qrcode bitmap renderer，不代表 `P5-F animation frame source`、`P5-G P5 closeout`、`P6` 之后阶段已完成。
- 当前阶段下一任务推进到 `P5-F animation frame source`。

## P5-F `animation frame source`

### RED

- 新增 focused native test：
  - [tests/picoui/native/test_picoui_native_animation_frames.c](/Users/cys/embedded/LingDongGUI/tests/picoui/native/test_picoui_native_animation_frames.c)
  - [tests/picoui/CMakeLists.txt](/Users/cys/embedded/LingDongGUI/tests/picoui/CMakeLists.txt)
- 首轮真实 `RED`：
  - `make -C build test_picoui_native_animation_frames`
  - 链接失败，缺少：
    - `picoui_native_animation_bind_frame_sources`
    - `picoui_native_animation_get_rendered_dirty_rect`
    - `picoui_native_animation_get_rendered_changed_pixels`
- 该 `RED` 证明：
  - 当前代码还不存在 `P5-F` 计划要求的 native animation frame source 绑定入口
  - 现有 [native_animation.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_animation.c) 只能处理旧的单 `source` sprite-strip 合同，还没有独立 frame source 队列和 software render 读回合同

### GREEN

- 在 [picoui/src/native/native_animation.c](/Users/cys/embedded/LingDongGUI/picoui/src/native/native_animation.c) 和 [picoui/include/picoui/native.h](/Users/cys/embedded/LingDongGUI/picoui/include/picoui/native.h) 新增最小 native-only frame source bridge：
  - `picoui_native_animation_bind_frame_sources()`
  - `picoui_native_animation_get_rendered_dirty_rect()`
  - `picoui_native_animation_get_rendered_changed_pixels()`
- 当前建立的最小 `P5-F` 合同：
  - `picoui_native_animation_bind_frame_sources()`：
    - 允许为单个 animation 绑定多张 frame image source
    - 校验每个 frame source 都具备真实 `img_tile`
    - 同步重置 `frame_count/frame_index/source/render_ready`
  - `picoui_native_animation_render()`：
    - 继续复用既有 timer/frame_index 同步逻辑
    - 在绑定了 `frame_sources` 时，按当前 frame 选择真实 source
    - 用 software buffer render 当前 frame，并读回 dirty rect / changed pixel count
    - 旧的单 `source` sprite-strip animation 路径保持原行为，不强行套用新的 software render 读回分支
- focused test 当前证明：
  - 三张独立 frame source 可被 animation native render 正确轮转
  - `80ms / period=40ms` 可稳定推进到第 3 帧
  - dirty area 非空
  - changed pixel count 大于 0
  - invalid input 会失败
- `animation_canvas` 回归 test 当前证明：
  - 既有 `animation + canvas` native timer/render 合同仍成立
  - 旧 sprite-strip animation 路径没有被 `P5-F` frame-source 绑定改坏

### 调试记录

- 中途出现过一次 `test_picoui_native_animation_canvas` 在 `picoui_timer_handler()` 断言失败。
- root cause 已确认不是旧动画合同本身回归，而是首版 `P5-F` 实现把 software render 读回逻辑误套到了未绑定 `frame_sources` 的旧 sprite-strip 路径。
- 已收窄为：
  - 只有绑定了 `frame_sources` 的 animation 才执行 software render dirty/pixel 读回
  - 旧单 `source` 路径继续只走原有 timer/frame show 合同
- 调试过程中的临时重跑与 `lldb` 验证未留在最终代码中。

### 验证

- `make -C build test_picoui_native_animation_frames test_picoui_native_animation_canvas`
- `./build/tests/picoui/test_picoui_native_animation_frames`
- `./build/tests/picoui/test_picoui_native_animation_canvas`
- `ctest --test-dir build -R '^(test_picoui_native_animation_frames|test_picoui_native_animation_canvas)$' --output-on-failure`
- `python3 tests/picoui/runtime/check_picoui_runtime.py --demo animation_basic --build-dir build`
- `python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo animation_basic --build-dir build`
- `git diff --check -- picoui/include/picoui/native.h picoui/src/native/native_animation.c tests/picoui/native/test_picoui_native_animation_frames.c tests/picoui/CMakeLists.txt`

### 当前结论

- 当前 `P5-F animation frame source` 已按任务范围完成 closeout。
- 当前 fresh 证据已经证明：
  - PicoUI 已有最小 native-only animation frame source 绑定入口
  - frame source timer advance / dirty rect / changed pixel 合同已成立
  - 既有 `animation_basic` smoke 和 `animation_canvas` 回归合同仍成立
- 当前实现只建立最小 animation frame source bridge，不代表 `P5-G P5 closeout`、`P6` 或之后阶段已完成。
- 当前阶段下一任务推进到 `P5-G P5 closeout`。

## P5-G `P5 closeout`

### 收口范围

- 本任务只收口 `P5 resource/font/image renderer bridges` 的阶段证据。
- 当前 closeout 覆盖：
  - `P5-A resource handle`
  - `P5-B font renderer bridge`
  - `P5-C image tile/mask bridge`
  - `P5-D canvas primitive renderer`
  - `P5-E qrcode bitmap renderer`
  - `P5-F animation frame source`
- 当前不扩写到 `P6 demo main migration / SDL port / artifact`，也不把旧 `picoui_app_run()` demo 自动算成已完成 native-main 迁移。

### Fresh 验证

- focused renderer aggregation：
  - `ctest --test-dir build -R 'test_picoui_native_(resource|font_renderer|image_renderer|canvas_renderer|qrcode_renderer|animation_frames)$' --output-on-failure`
- existing native widget regression anchors：
  - `ctest --test-dir build -R '^(test_picoui_native_animation_canvas|test_picoui_native_qrcode|test_picoui_native_text|test_picoui_native_image)$' --output-on-failure`
- demo/runtime smoke：
  - `python3 tests/picoui/runtime/check_picoui_runtime.py --demo basic_widgets --build-dir build`
  - `python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo basic_widgets --build-dir build`
  - `python3 tests/picoui/runtime/check_picoui_runtime.py --demo qrcode_basic --build-dir build`
  - `python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo qrcode_basic --build-dir build`
  - `python3 tests/picoui/runtime/check_picoui_runtime.py --demo animation_basic --build-dir build`
  - `python3 tests/picoui/runtime/check_picoui_visible_ui.py --demo animation_basic --build-dir build`
- contract/format：
  - `git diff --check -- picoui/include/picoui/resource.h picoui/include/picoui/font.h picoui/include/picoui/native.h picoui/src/core/resource.c picoui/src/native/native_resource.c picoui/src/native/native_text_renderer.c picoui/src/native/native_image_renderer.c picoui/src/native/native_canvas_renderer.c picoui/src/native/native_qrcode_renderer.c picoui/src/native/native_animation.c tests/picoui/native/test_picoui_native_resource.c tests/picoui/native/test_picoui_native_font_renderer.c tests/picoui/native/test_picoui_native_image_renderer.c tests/picoui/native/test_picoui_native_canvas_renderer.c tests/picoui/native/test_picoui_native_qrcode_renderer.c tests/picoui/native/test_picoui_native_animation_frames.c tests/picoui/contract/picoui_native_migration_ledger.json`

### 证据边界

- `basic_widgets` 负责给 `text/image` 提供真实 runtime + visible 证据。
- `qrcode_basic` 负责给 `qrcode bitmap renderer` 提供真实 runtime + visible 证据。
- `animation_basic` 负责给 `animation frame source` 提供真实 runtime + visible 证据。
- 当前仓库没有单独的 `canvas_basic` demo；`canvas` 能力在 `P5` 范围内继续由 [test_picoui_native_canvas_renderer.c](/Users/cys/embedded/LingDongGUI/tests/picoui/native/test_picoui_native_canvas_renderer.c) 与 [test_picoui_native_animation_canvas.c](/Users/cys/embedded/LingDongGUI/tests/picoui/native/test_picoui_native_animation_canvas.c) 提供 focused/native 回归证据，不发明不存在的 demo gate。

### GitNexus / Worktree 状态

- `gitnexus detect_changes(scope=all)` 当前返回 `risk_level=critical`。
- 该结果来自整棵 worktree 已累计的 57 个 changed files / 428 个 changed symbols，不是本次 `P5` renderer closeout 独立 blast radius。
- 就本次 `P5` 路线本身，前面每个具体实现任务都已在编辑前做过目标 impact，涉及 public/native renderer 入口均为 `LOW`，没有出现需要中止编辑的 `HIGH/CRITICAL` 单点 blast radius。

### 当前结论

- 当前 `P5 resource/font/image` 阶段已按串行计划完成 closeout。
- 当前 fresh 证据已经证明：
  - PicoUI 已具备最小 native resource handle、font bridge、image tile/mask bridge、canvas primitive renderer、qrcode bitmap renderer、animation frame source bridge
  - 相关 focused renderer 合同和关键 native widget 回归合同都已成立
  - `basic_widgets`、`qrcode_basic`、`animation_basic` 的 runtime / visible smoke 当前为绿
- 当前 closeout 不宣称：
  - `P6` demo main 全量 native migration 已完成
  - `P7` 去掉默认 ldgui runtime 依赖已完成
  - `P8` release gate 已完成
- 当前阶段下一任务推进到 `P6-A demo main migration and SDL port`.
