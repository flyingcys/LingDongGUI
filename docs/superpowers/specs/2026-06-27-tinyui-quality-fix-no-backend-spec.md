# TinyUI 无 backend 质量修复详细 Spec

## 0. 一句话结论

本轮目标不是补 backend，也不是证明 backend。目标是把 TinyUI 按“public API 用户能力覆盖”口径从 8.4/10 打磨到 9.2/10 以上：API 命名更干净、资源生命周期更清楚、demo 完全不泄漏底层符号、contract 和文档不再把 backend 当质量依据。

## 1. 背景和当前事实

### 1.1 当前能力结论

当前 TinyUI 的能力覆盖合同已经达到可验收状态：

1. LingDongGUI public API inventory：`629` 条。
2. release matrix：`629` 行。
3. required 行全部 `covered`。
4. missing gap：`0`。
5. 按“用户能否通过 TinyUI public API 完成等价控件能力”口径，可判定能力覆盖达标。

本 spec 不再讨论 backend 是否存在，也不把 backend 作为扣分项。

### 1.2 当前质量短板

当前短板集中在 5 类：

1. `tinyui/demo/**` 仍有底层资源泄漏。
2. demo boundary checker 允许 `arm_2d_tile_t` 透明声明，门禁过松。
3. public API 存在历史命名和拼写错误 alias。
4. `struct tinyui_image_source` / `struct tinyui_font` 的生命周期、所有权、销毁规则文档不够硬。
5. 部分 current-facing 文档仍用 backend proof / backend mapping 表述质量依据。

### 1.3 已确认的具体问题

当前仓库中已确认：

1. `tinyui/demo/animation_basic/animation_basic.c` 出现：
   - `typedef struct arm_2d_tile_t arm_2d_tile_t;`
   - `extern const arm_2d_tile_t c_tileQuaterArcGRAY8;`
   - `.img_tile = (void *)&c_tileQuaterArcGRAY8`
2. `tests/tinyui/contract/check_tinyui_demo_boundary.py` 存在 `OPAQUE_ASSET_DECL_RE`，会过滤上述声明。
3. `tinyui/demo/legacy_widget_parity/legacy_widget_parity.c` include `../../../examples/common/demo/widget/images/uiImages.h`，并通过 `IMAGE_*` 宏手写 `tinyui_image_source`。
4. `tinyui/include/widgets/table.h` 只有 `tinyui_tabel_show_keyboard`，缺少正确拼写的 `tinyui_table_show_keyboard`。
5. `tinyui/include/widgets/qrcode.h` 仍暴露 `tinyui_q_r_code_init`、`tinyui_q_r_code_set_text`。
6. `tests/tinyui/contract/*json` 里仍有 `tinyui_q_r_code_*`、`tinyui_tabel_show_keyboard` 作为主 tinyui_api 字段。

## 2. 硬指标

### 2.1 不做 backend

本轮严格禁止：

1. 不创建 `tinyui/src/backend/**`。
2. 不恢复任何已删除 backend。
3. 不新增 backend adapter、backend mapping、backend proof、backend gate。
4. 不把 backend 缺失作为 TinyUI 质量问题。
5. 不把 demo 修复写成 backend 适配。
6. 不运行 `check_tinyui_backend_mapping.py` 作为本轮完成门禁。

如果历史测试或历史 JSON 字段仍含 `backend` 字样，本轮只做口径隔离：不让它成为 current-facing 质量结论。

### 2.2 tinyui_demo 零泄漏

扫描范围：

1. `tinyui/demo/**/*.{c,h}`
2. `tinyui_demo/**/*.{c,h}`
3. `tinyui/docs/demo_guide.md`
4. demo 相关新增文档和示例代码块

上述范围中禁止出现：

1. `LingDongGUI`
2. `lingdonggui`
3. `Arm-2D`
4. `arm2d`
5. `arm_2d`
6. `arm_2d_*`
7. `SIGNAL_*`
8. `ld*` 底层符号，例如 `ldWindow`、`ldImage`、`ldBase`
9. `c_tile*`
10. `IMAGE_*`
11. `.img_tile`
12. `.mask_tile`
13. `uiImages.h`
14. `src/gui/`
15. `src/misc/`

说明：

1. `tinyui_demo` 的用户可见 demo 层必须零泄漏。
2. CMake 内部链接库名属于构建实现，本 spec 不把全工程 CMake 文本零命中作为目标；若要求 CMake 也零命中，需要单独做全工程命名治理。
3. demo 只允许展示 `tinyui_*` public API、标准 C API、demo 自己的函数。

### 2.3 不破坏兼容

1. 历史 public API 不直接删除。
2. 错拼或历史 API 改成 deprecated alias。
3. demo、quick start、API overview、能力文档只使用 canonical API。
4. 旧 API 的行为必须继续有单测覆盖。

## 3. 目标状态

### 3.1 质量目标

完成后目标评分：

1. 能力覆盖：`9.5/10`，required 100% covered 不变。
2. API 封装质量：`9.0/10`，历史 alias 不污染 demo 和主文档。
3. API 一致性：`8.7/10`，资源命名、所有权、销毁规则清楚。
4. 文档完整度：`9.2/10`，无 backend 口径冲突。
5. 验证质量：`9.0/10`，demo boundary 和语义测试变硬。
6. 综合目标：`9.2/10` 以上。

### 3.2 代码目标

1. `tinyui/demo/**` 和 `tinyui_demo/**` 不出现任何禁用 token。
2. demo 不手写 `struct tinyui_image_source` 的底层字段。
3. demo 需要图片资源时，只调用 TinyUI public resource API。
4. `tinyui_table_show_keyboard` 成为 canonical API。
5. `tinyui_tabel_show_keyboard` 仅作为 deprecated alias。
6. `tinyui_qrcode_create`、`tinyui_qrcode_set_text` 是 qrcode 文档和 demo 唯一推荐 API。
7. `tinyui_q_r_code_*` 仅作为 deprecated alias。

### 3.3 文档目标

1. 所有新增/修改 markdown 使用中文。
2. current-facing 文档不再写“backend 证明 TinyUI 质量”。
3. 能力覆盖文档说明：TinyUI 是 public API 封装层，目标是用户能力覆盖，不是逐字 wrapper，也不是 backend 完整性。
4. 资源生命周期单独成文。

## 4. 详细设计

### 4.1 demo boundary checker

修改文件：

1. `tests/tinyui/contract/check_tinyui_demo_boundary.py`

设计要求：

1. 删除 `OPAQUE_ASSET_DECL_RE`。
2. 扫描 `.c` 和 `.h`。
3. 扫描 `tinyui/demo` 和 `tinyui_demo` 两个目录。
4. 不跳过注释。
5. 不跳过字符串。
6. 不保留任何透明底层类型豁免。
7. 错误信息必须包含相对路径、命中 token、规则名称。

建议规则表：

```python
FORBIDDEN_PATTERNS = {
    "LingDongGUI": re.compile(r"LingDongGUI"),
    "lingdonggui": re.compile(r"lingdonggui"),
    "Arm-2D": re.compile(r"Arm-2D"),
    "arm2d": re.compile(r"arm2d", re.I),
    "arm_2d": re.compile(r"arm_2d[A-Za-z0-9_]*"),
    "SIGNAL_*": re.compile(r"\bSIGNAL_[A-Za-z0-9_]+\b"),
    "ld*": re.compile(r"\bld(?:[A-Za-z0-9_]*|\*)\b|\bld\*"),
    "c_tile*": re.compile(r"\bc_tile[A-Za-z0-9_]*\b"),
    "IMAGE_*": re.compile(r"\bIMAGE_[A-Za-z0-9_]*\b"),
    ".img_tile": re.compile(r"\.img_tile\b"),
    ".mask_tile": re.compile(r"\.mask_tile\b"),
    "uiImages.h": re.compile(r"uiImages\.h"),
    "src/gui": re.compile(r"src/gui/"),
    "src/misc": re.compile(r"src/misc/"),
}
```

兼容点：

1. `TINYUI_NATIVE_SIGNAL_*` 不在 `tinyui/demo/**` 中使用；若未来 demo 需要事件，只用控件级 callback。
2. `tinyui/demo/tinyui_demos.c` 的注释不能写底层词。
3. `demo_guide.md` 不能再推荐 backend mapping。

预期失败信号：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
```

修复前应失败，至少报出 `animation_basic.c` 的 `arm_2d_tile_t`。

预期通过信号：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
rtk rg -n 'LingDongGUI|lingdonggui|Arm-2D|arm2d|arm_2d|SIGNAL_|\\bld[A-Z_][A-Za-z0-9_]*\\b|\\bc_tile[A-Za-z0-9_]*\\b|\\bIMAGE_[A-Za-z0-9_]*\\b|\\.img_tile|\\.mask_tile|uiImages\\.h|src/gui/|src/misc/' tinyui/demo tinyui_demo tinyui/docs/demo_guide.md
```

第二条命令预期无输出。

### 4.2 TinyUI 中性资源 API

修改文件：

1. `tinyui/include/widgets/image.h`
2. `tinyui/src/core/resource.c`
3. 必要时新增 `tinyui/src/core/builtin_resource.c`
4. 必要时修改构建文件，把新增 `.c` 加入 TinyUI core 源列表

不允许：

1. 不新增 backend 目录。
2. 不把资源 API 放进 demo。
3. 不让 demo include 旧图片头。

#### 4.2.1 新增 image source kind

建议在 `tinyui/include/widgets/image.h` 增加：

```c
enum tinyui_image_source_kind {
    TINYUI_IMAGE_SOURCE_KIND_EMPTY = 0,
    TINYUI_IMAGE_SOURCE_KIND_EXTERNAL = 1,
    TINYUI_IMAGE_SOURCE_KIND_VRES = 2,
    TINYUI_IMAGE_SOURCE_KIND_BUILTIN = 3,
};
```

字段语义：

1. `EMPTY`：未初始化资源。
2. `EXTERNAL`：用户外部静态资源或历史兼容资源，TinyUI 不拥有内存。
3. `VRES`：由 `tinyui_image_source_from_vres` 创建，TinyUI destroy 负责释放其 factory 资源。
4. `BUILTIN`：由 `tinyui_image_source_from_builtin` 返回，静态内置资源，destroy 不释放底层资源。

`struct tinyui_image_source.kind` 继续保持 `unsigned int` 可兼容现有代码，但文档中按上述 enum 解释。

#### 4.2.2 新增 builtin image 枚举

建议在 `tinyui/include/widgets/image.h` 增加：

```c
enum tinyui_builtin_image {
    TINYUI_BUILTIN_IMAGE_LETTER_PAPER = 1,
    TINYUI_BUILTIN_IMAGE_KEY_RELEASE,
    TINYUI_BUILTIN_IMAGE_KEY_PRESS,
    TINYUI_BUILTIN_IMAGE_PROGRESS_BG,
    TINYUI_BUILTIN_IMAGE_PROGRESS_FG,
    TINYUI_BUILTIN_IMAGE_SLIDER_BG,
    TINYUI_BUILTIN_IMAGE_SLIDER_INDICATOR,
    TINYUI_BUILTIN_IMAGE_WEATHER,
    TINYUI_BUILTIN_IMAGE_NOTE,
    TINYUI_BUILTIN_IMAGE_BOOK,
    TINYUI_BUILTIN_IMAGE_CHART,
    TINYUI_BUILTIN_IMAGE_GAUGE_BG,
    TINYUI_BUILTIN_IMAGE_GAUGE_POINTER,
    TINYUI_BUILTIN_IMAGE_ARC_QUARTER,
};
```

设计说明：

1. 每个 enum 返回一个完整 `struct tinyui_image_source`。
2. 若该资源有 mask，API 内部填好 `mask_tile`。
3. demo 不知道 mask 是否存在。
4. `ARC_QUARTER` 同时包含 arc quarter image 和对应 mask。
5. 不暴露 `IMAGE_*`、`c_tile*`、`arm_2d_tile_t`。

#### 4.2.3 新增 factory 函数

建议 public API：

```c
int tinyui_image_source_from_builtin(enum tinyui_builtin_image image,
                                     struct tinyui_image_source *out);
```

返回值：

1. `0`：成功，`out` 已被完整初始化。
2. `-1`：参数非法或资源不存在，`out` 不应被用户使用。

成功后的 `out`：

1. `out->img_tile != 0`
2. `out->kind == TINYUI_IMAGE_SOURCE_KIND_BUILTIN`
3. `out->vres_addr == 0`
4. `out->mask_tile` 由资源决定，可为 `0`

#### 4.2.4 destroy 规则

修改 `tinyui_image_source_destroy`：

1. `source == 0`：直接返回。
2. `kind == TINYUI_IMAGE_SOURCE_KIND_VRES`：释放 factory 创建的底层对象。
3. `kind == TINYUI_IMAGE_SOURCE_KIND_BUILTIN`：不释放底层对象，只清空结构。
4. `kind == TINYUI_IMAGE_SOURCE_KIND_EXTERNAL` 或 `0`：不释放底层对象，只清空结构。
5. destroy 必须幂等。

修改 `tinyui_image_source_from_vres`：

1. 成功时设置 `kind = TINYUI_IMAGE_SOURCE_KIND_VRES`。
2. 失败时返回 `-1`。

#### 4.2.5 builtin 资源映射表

建议映射：

| TinyUI enum | 内部资源语义 | mask 语义 |
| --- | --- | --- |
| `TINYUI_BUILTIN_IMAGE_LETTER_PAPER` | paper 背景 | 无 |
| `TINYUI_BUILTIN_IMAGE_KEY_RELEASE` | button release | 有 |
| `TINYUI_BUILTIN_IMAGE_KEY_PRESS` | button press | 有 |
| `TINYUI_BUILTIN_IMAGE_PROGRESS_BG` | progress bg | 无 |
| `TINYUI_BUILTIN_IMAGE_PROGRESS_FG` | progress fg | 无 |
| `TINYUI_BUILTIN_IMAGE_SLIDER_BG` | slider bg | 有 |
| `TINYUI_BUILTIN_IMAGE_SLIDER_INDICATOR` | slider indicator | 有 |
| `TINYUI_BUILTIN_IMAGE_WEATHER` | weather icon | 有 |
| `TINYUI_BUILTIN_IMAGE_NOTE` | note icon | 有 |
| `TINYUI_BUILTIN_IMAGE_BOOK` | book icon | 有 |
| `TINYUI_BUILTIN_IMAGE_CHART` | chart icon | 有 |
| `TINYUI_BUILTIN_IMAGE_GAUGE_BG` | gauge background | 有 |
| `TINYUI_BUILTIN_IMAGE_GAUGE_POINTER` | gauge pointer | 有 |
| `TINYUI_BUILTIN_IMAGE_ARC_QUARTER` | arc quarter | 有 |

内部实现可以 include 旧图片头，但只能在 TinyUI core 源文件里出现，不能出现在 demo 源文件、demo 文档、public header。

### 4.3 demo 改造

#### 4.3.1 `animation_basic`

修改文件：

1. `tinyui/demo/animation_basic/animation_basic.c`

删除：

1. `typedef struct arm_2d_tile_t arm_2d_tile_t;`
2. `extern const arm_2d_tile_t c_tileQuaterArcGRAY8;`
3. 静态 `.img_tile` 初始化。

新增：

1. `static struct tinyui_image_source s_animation_source;`
2. `static int s_animation_source_ready;`
3. `static int ensure_animation_source(void);`

推荐逻辑：

```c
static struct tinyui_image_source s_animation_source;
static int s_animation_source_ready;

static int ensure_animation_source(void)
{
    if (s_animation_source_ready != 0) {
        return 0;
    }
    if (tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_ARC_QUARTER,
                                         &s_animation_source) != 0) {
        return -1;
    }
    s_animation_source_ready = 1;
    return 0;
}
```

`make_ui` 中：

1. 先调用 `ensure_animation_source()`。
2. 失败则 return。
3. `props.source = &s_animation_source`。
4. 不做底层 fallback。

生命周期要求：

1. source 必须静态或长于 widget 生命周期。
2. 不使用局部 `struct tinyui_image_source` 传给 widget。

#### 4.3.2 `legacy_widget_parity`

修改文件：

1. `tinyui/demo/legacy_widget_parity/legacy_widget_parity.c`

删除：

1. `#include "../../../examples/common/demo/widget/images/uiImages.h"`
2. 所有 `IMAGE_*`
3. 所有 `.img_tile =`
4. 所有 `.mask_tile =`

新增：

1. 一个 demo 私有资源结构。
2. 一个初始化函数。

推荐结构：

```c
struct legacy_widget_sources {
    struct tinyui_image_source paper;
    struct tinyui_image_source button_release;
    struct tinyui_image_source button_press;
    struct tinyui_image_source progress_bg;
    struct tinyui_image_source progress_fg;
    struct tinyui_image_source slider_bg;
    struct tinyui_image_source slider_indicator;
    struct tinyui_image_source weather;
    struct tinyui_image_source note;
    struct tinyui_image_source book;
    struct tinyui_image_source chart;
    struct tinyui_image_source gauge_bg;
    struct tinyui_image_source gauge_pointer;
    struct tinyui_image_source arc_quarter;
};
```

推荐初始化：

```c
static struct legacy_widget_sources s_sources;
static int s_sources_ready;

static int load_source(enum tinyui_builtin_image image,
                       struct tinyui_image_source *source)
{
    return tinyui_image_source_from_builtin(image, source);
}

static int ensure_legacy_sources(void)
{
    if (s_sources_ready != 0) {
        return 0;
    }
    if (load_source(TINYUI_BUILTIN_IMAGE_LETTER_PAPER, &s_sources.paper) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_KEY_RELEASE, &s_sources.button_release) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_KEY_PRESS, &s_sources.button_press) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_PROGRESS_BG, &s_sources.progress_bg) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_PROGRESS_FG, &s_sources.progress_fg) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_SLIDER_BG, &s_sources.slider_bg) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_SLIDER_INDICATOR, &s_sources.slider_indicator) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_WEATHER, &s_sources.weather) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_NOTE, &s_sources.note) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_BOOK, &s_sources.book) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_CHART, &s_sources.chart) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_GAUGE_BG, &s_sources.gauge_bg) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_GAUGE_POINTER, &s_sources.gauge_pointer) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_ARC_QUARTER, &s_sources.arc_quarter) != 0) {
        return -1;
    }
    s_sources_ready = 1;
    return 0;
}
```

替换规则：

| 当前资源 | 新资源 |
| --- | --- |
| `s_legacy_image_source` | `&s_sources.paper` |
| `s_legacy_button_release_source` | `&s_sources.button_release` |
| `s_legacy_button_press_source` | `&s_sources.button_press` |
| `s_legacy_progress_bg_source` | `&s_sources.progress_bg` |
| `s_legacy_progress_fg_source` | `&s_sources.progress_fg` |
| `s_legacy_text_bg_source` | `&s_sources.paper` |
| `s_legacy_slider_bg_source` | `&s_sources.slider_bg` |
| `s_legacy_slider_indicator_source` | `&s_sources.slider_indicator` |
| `s_legacy_radial_weather_source` | `&s_sources.weather` |
| `s_legacy_radial_note_source` | `&s_sources.note` |
| `s_legacy_icon_note_source` | `&s_sources.note` |
| `s_legacy_icon_book_source` | `&s_sources.book` |
| `s_legacy_icon_weather_source` | `&s_sources.weather` |
| `s_legacy_icon_chart_source` | `&s_sources.chart` |
| `s_legacy_gauge_bg_source` | `&s_sources.gauge_bg` |
| `s_legacy_gauge_pointer_source` | `&s_sources.gauge_pointer` |
| `s_legacy_arc_quarter_source` | `&s_sources.arc_quarter` |

入口要求：

1. `make_ui` 开头调用 `ensure_legacy_sources()`。
2. 失败则 return。
3. 不打印底层错误。
4. 不 fallback 到底层资源。

#### 4.3.3 其他 demo

所有其他 demo 按 checker 结果修：

1. 若出现底层 token，必须删除。
2. 若是注释或字符串，也要改写。
3. 若是 include 泄漏，改为 TinyUI public API。
4. demo marker 检查只允许 `tinyui_*` marker。

### 4.4 API 命名整理

#### 4.4.1 table keyboard API

修改文件：

1. `tinyui/include/widgets/table.h`
2. `tinyui/src/widgets/table.c`
3. `tests/tinyui/unit/test_tinyui_table.c`
4. `tests/tinyui/contract/native_api_gap_ledger.json`
5. `tests/tinyui/contract/ldgui_public_api_inventory.json`
6. `tests/tinyui/contract/tinyui_release_capability_matrix.json`

新增 canonical API：

```c
int tinyui_table_show_keyboard(struct tinyui_table *table);
```

保留 deprecated alias：

```c
int tinyui_tabel_show_keyboard(struct tinyui_table *table);
```

实现要求：

1. `tinyui_table_show_keyboard` 承载真实实现。
2. `tinyui_tabel_show_keyboard` 只调用 `tinyui_table_show_keyboard`。
3. 单测同时覆盖 canonical 和 alias。
4. demo 和主文档只允许出现 `tinyui_table_show_keyboard`。
5. matrix 主 `tinyui_api` 字段改成 `tinyui_table_show_keyboard`。
6. alias 只在 deprecated 说明和 alias 单测中出现。

#### 4.4.2 qrcode API

当前 canonical API：

1. `tinyui_qrcode_create`
2. `tinyui_qrcode_create_with_props`
3. `tinyui_qrcode_set_text`
4. `tinyui_qrcode_get_text`
5. `tinyui_qrcode_set_qr_color`
6. `tinyui_qrcode_set_bg_color`
7. `tinyui_qrcode_set_ecc`
8. `tinyui_qrcode_set_max_version`
9. `tinyui_qrcode_set_zoom`

deprecated alias：

1. `tinyui_q_r_code_init`
2. `tinyui_q_r_code_set_text`

修复要求：

1. header 中可保留 alias。
2. source 中 alias 只转调 canonical。
3. demo 不出现 alias。
4. current-facing 文档不推荐 alias。
5. matrix 主 `tinyui_api` 字段从 `tinyui_q_r_code_*` 改成 canonical API。
6. alias 单测保留，用于证明兼容。

#### 4.4.3 `*_image` 与 `*_source`

规则：

1. canonical 命名优先使用 `source`，因为 public 参数类型是 `struct tinyui_image_source *`。
2. `*_image` 可作为历史 alias 保留。
3. demo 和新文档优先使用 `*_source`。
4. 如果某个控件只有 `*_image` 没有 `*_source`，不强制本轮补齐，除非 demo 必须使用该能力。

当前建议：

1. button 保留 `tinyui_button_set_image`，demo 若需要成对设置可继续使用，因为它不是底层 image 命名，而是 TinyUI 语义 API。
2. slider 保留 `tinyui_slider_set_image`，但文档优先展示 `set_background_source` / `set_indicator_source`。
3. progress_bar 保留 `tinyui_progress_bar_set_image`，但文档优先展示 `set_bg_source` / `set_fg_source`。
4. gauge/arc/clock 中已有 `*_source` 的，demo 优先使用 `*_source`。

### 4.5 deprecated API 使用门禁

新增文件：

1. `tests/tinyui/contract/check_tinyui_deprecated_api_usage.py`

加入 CMake：

1. `tests/tinyui/CMakeLists.txt`

扫描范围：

1. `tinyui/demo/**/*.c`
2. `tinyui/demo/**/*.h`
3. `tinyui_demo/**/*.c`
4. `tinyui_demo/**/*.h`
5. `tinyui/docs/*.md`

禁止在 demo 中出现：

1. `tinyui_tabel_show_keyboard`
2. `tinyui_q_r_code_init`
3. `tinyui_q_r_code_set_text`

文档规则：

1. `tinyui/docs/api_overview.md` 和 `tinyui/docs/quick_start.md` 禁止出现 deprecated alias。
2. 若新增 `tinyui/docs/deprecated_api.md`，允许列出 deprecated alias，但必须明确“不要在新代码使用”。

验收：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_deprecated_api_usage.py
```

### 4.6 contract JSON 更新规则

修改文件：

1. `tests/tinyui/contract/ldgui_public_api_inventory.json`
2. `tests/tinyui/contract/native_api_gap_ledger.json`
3. `tests/tinyui/contract/tinyui_release_capability_matrix.json`

更新规则：

1. 不改变行数。
2. 不改变 required 判定。
3. 不把用户能力改成 policy。
4. 将主 `tinyui_api` 从 deprecated alias 改成 canonical API。
5. qrcode init 行使用 `tinyui_qrcode_create` 或 `tinyui_qrcode_create_with_props`。
6. qrcode set text 行使用 `tinyui_qrcode_set_text`。
7. table show keyboard 行使用 `tinyui_table_show_keyboard`。
8. `unit_test` 字段补 alias 兼容测试名称。
9. `audit_note` 或 `rationale` 中说明 alias 仍兼容，但不作为主 API。

操作要求：

1. 使用 JSON parser 更新，不用全局字符串替换。
2. 更新后运行所有 contract checker。
3. 保持 `629` 行不变。

关于历史证据字段：

1. 将旧 `backend_proof` 字段迁移为 `evidence_tag`，避免 current-facing contract 继续使用 backend 口径。
2. `evidence_tag` 仅表示历史 ledger 证据标签，不参与 no-backend 质量评分。
3. checker 只校验字段存在、ledger/matrix 一致和 public API 存在，不把底层适配当完成门禁。
4. 不新增 backend proof gate。

### 4.7 资源生命周期文档

新增文件：

1. `tinyui/docs/resource_lifetime.md`

修改文件：

1. `tinyui/docs/api_overview.md`
2. `tinyui/docs/quick_start.md`
3. `tinyui/docs/demo_guide.md`

必须写清楚：

1. `tinyui_image_source` 是 TinyUI 资源句柄。
2. 用户不应手写 `.img_tile` / `.mask_tile`。
3. demo 不展示底层资源字段。
4. 推荐资源来源：
   - `tinyui_image_source_from_builtin`
   - `tinyui_image_source_from_vres`
5. `tinyui_image_source_from_builtin` 返回静态内置资源，通常不需要 destroy。
6. `tinyui_image_source_from_vres` 返回需按规则 destroy 的资源。
7. `tinyui_image_source_destroy` 对 builtin/external 安全 no-op，对 vres 释放并清空。
8. `tinyui_font_from_vres` 和 `tinyui_font_destroy` 的规则。
9. source 指针必须长于使用它的 widget。
10. 不要把栈上临时 `tinyui_image_source` 传给长期存在的 widget。

demo guide 修改：

1. 删除 backend mapping 作为 demo 证据层。
2. demo 自动 gate 改成：
   - demo boundary
   - deprecated API usage
   - runtime smoke
   - visible UI
   - contract
   - unit

### 4.8 能力覆盖文档口径

修改文件：

1. `docs/ability/tinyui_lingdonggui_api_coverage.md`

要求：

1. 保留 100% covered 结论。
2. 明确“不按 backend 评分”。
3. 将旧证据字段相关描述改成 `evidence_tag` / public API 证据说明。
4. 每个控件仍保持二级标题。
5. 表格可以保留原始字段，但解释必须改成：
   - public API 证据
   - contract 证据
   - unit 语义测试
   - demo boundary
6. 不新增“backend 缺失”扣分。

### 4.9 测试补强

#### 4.9.1 resource unit

建议新增：

1. `tests/tinyui/unit/test_tinyui_resource.c`

测试用例：

1. `test_builtin_image_source_success`
2. `test_builtin_image_source_rejects_invalid_id`
3. `test_builtin_image_source_rejects_null_out`
4. `test_builtin_image_source_destroy_is_noop_and_clears`
5. `test_vres_image_source_sets_kind`
6. `test_vres_image_source_destroy_is_idempotent`
7. `test_font_from_vres_lifetime_contract`

CMake：

1. 将 `unit/test_tinyui_resource.c` 加入 `TINYUI_UNIT_TESTS`。

#### 4.9.2 deprecated alias unit

可新增：

1. `tests/tinyui/unit/test_tinyui_deprecated_alias.c`

也可合入现有：

1. `tests/tinyui/unit/test_tinyui_table.c`
2. `tests/tinyui/unit/test_tinyui_qrcode.c`

最低要求：

1. `tinyui_table_show_keyboard(table) == tinyui_tabel_show_keyboard(table)` 行为一致。
2. `tinyui_q_r_code_init` 创建对象成功。
3. `tinyui_q_r_code_set_text` 设置后 `tinyui_qrcode_get_text` 可读到相同值。

#### 4.9.3 demo boundary regression

`check_tinyui_demo_boundary.py` 本身要能防止回归：

1. 禁止透明 typedef。
2. 禁止 extern 底层资源。
3. 禁止底层 include。
4. 禁止 `.img_tile` / `.mask_tile`。

## 5. 文件级任务拆分

### Task 1：收紧 demo boundary checker

文件：

1. 修改 `tests/tinyui/contract/check_tinyui_demo_boundary.py`

步骤：

1. 删除 `OPAQUE_ASSET_DECL_RE`。
2. 将扫描范围改为 `tinyui/demo` + `tinyui_demo`。
3. 添加 forbidden patterns。
4. 先运行 checker，确认失败。
5. 保留失败输出作为修复依据。

完成信号：

1. 修复 demo 前 checker 能抓到当前泄漏。
2. checker 错误信息可定位文件和 token。

### Task 2：实现 TinyUI builtin resource API

文件：

1. 修改 `tinyui/include/widgets/image.h`
2. 修改 `tinyui/src/core/resource.c`
3. 必要时新增 `tinyui/src/core/builtin_resource.c`
4. 必要时修改 TinyUI core 源列表

步骤：

1. 加 enum。
2. 加 `tinyui_image_source_from_builtin` 声明。
3. 实现 builtin 映射。
4. 修 `tinyui_image_source_from_vres` 设置 kind。
5. 修 `tinyui_image_source_destroy` 按 kind 释放。

完成信号：

1. public header 不出现 `arm_2d` / `ld*`。
2. API 可被 demo 调用。
3. 单测可覆盖。

### Task 3：修 demo 资源泄漏

文件：

1. 修改 `tinyui/demo/animation_basic/animation_basic.c`
2. 修改 `tinyui/demo/legacy_widget_parity/legacy_widget_parity.c`
3. 修改 checker 抓出的其他 demo 文件

步骤：

1. 删除底层 typedef / extern / include。
2. 删除手写 source 字段。
3. 用 `tinyui_image_source_from_builtin` 初始化静态资源。
4. demo 初始化失败时直接 return。
5. 运行 demo boundary checker。

完成信号：

1. 运行“6.1 demo 洁净度”中的完整 `rtk rg` 命令无输出。
2. `tinyui_demo` 仍能 build。

### Task 4：整理 canonical / deprecated API

文件：

1. 修改 `tinyui/include/widgets/table.h`
2. 修改 `tinyui/src/widgets/table.c`
3. 修改 `tests/tinyui/unit/test_tinyui_table.c`
4. 修改 `tests/tinyui/unit/test_tinyui_qrcode.c`

步骤：

1. 新增 `tinyui_table_show_keyboard`。
2. 旧 `tinyui_tabel_show_keyboard` 转调新函数。
3. 确认 qrcode alias 只转调 canonical。
4. 单测覆盖 canonical 和 alias。

完成信号：

1. 旧 API 不断。
2. 新 API 有测试。
3. demo 不使用 deprecated alias。

### Task 5：更新 contract 和 deprecated usage checker

文件：

1. 新增 `tests/tinyui/contract/check_tinyui_deprecated_api_usage.py`
2. 修改 `tests/tinyui/CMakeLists.txt`
3. 修改三个 contract JSON

步骤：

1. 写 checker。
2. 接入 CMake。
3. 用 JSON parser 更新主 `tinyui_api` 字段。
4. 运行 contract checker。

完成信号：

1. matrix 行数仍为 629。
2. missing gap 仍为 0。
3. deprecated alias 不出现在 demo。

### Task 6：补文档

文件：

1. 新增 `tinyui/docs/resource_lifetime.md`
2. 修改 `tinyui/docs/api_overview.md`
3. 修改 `tinyui/docs/quick_start.md`
4. 修改 `tinyui/docs/demo_guide.md`
5. 修改 `docs/ability/tinyui_lingdonggui_api_coverage.md`

步骤：

1. 写资源生命周期。
2. 改 demo guide gate。
3. 改 API overview 示例。
4. 改能力覆盖文档 no-backend 口径。
5. 扫描 forbidden tokens。

完成信号：

1. markdown 中文。
2. demo 文档不出现底层词。
3. current-facing 文档不把 backend 当质量依据。

### Task 7：完整验证

必须运行：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
rtk python3 tests/tinyui/contract/check_tinyui_deprecated_api_usage.py
rtk python3 tests/tinyui/contract/check_tinyui_public_api.py
rtk python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
rtk python3 tests/tinyui/contract/check_tinyui_native_api_exhaustiveness.py
rtk cmake -S . -B build
rtk cmake --build build -j8 --target tinyui_demo
rtk ctest --test-dir build -L 'tinyui' -E 'check_tinyui_backend_mapping' --output-on-failure
rtk git diff --check
```

可选运行：

```bash
rtk python3 tests/tinyui/runtime/check_tinyui_runtime.py
rtk python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
```

禁止作为本轮完成门禁：

```bash
rtk python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
```

## 6. 验收清单

### 6.1 demo 洁净度

必须无输出：

```bash
rtk rg -n 'LingDongGUI|lingdonggui|Arm-2D|arm2d|arm_2d|SIGNAL_|\\bld[A-Z_][A-Za-z0-9_]*\\b|\\bc_tile[A-Za-z0-9_]*\\b|\\bIMAGE_[A-Za-z0-9_]*\\b|\\.img_tile|\\.mask_tile|uiImages\\.h|src/gui/|src/misc/' tinyui/demo tinyui_demo tinyui/docs/demo_guide.md
```

### 6.2 API 洁净度

demo 中必须无输出：

```bash
rtk rg -n 'tinyui_tabel_show_keyboard|tinyui_q_r_code_init|tinyui_q_r_code_set_text' tinyui/demo tinyui_demo
```

### 6.3 contract 洁净度

必须满足：

1. `check_tinyui_public_api.py` 通过。
2. `check_tinyui_release_capability_matrix.py` 通过。
3. `check_tinyui_native_api_exhaustiveness.py` 通过。
4. matrix 行数仍为 `629`。
5. required covered 仍为 100%。

### 6.4 文档口径

必须满足：

1. no-backend 口径明确。
2. demo guide 不再把 backend mapping 写成必跑 gate。
3. 能力覆盖文档不因 backend 缺失扣分。
4. deprecated API 只在兼容说明中出现。

## 7. 风险和处理

### 7.1 `tinyui_image_source_destroy` 行为变化

风险：

1. 旧代码可能把手写 `.img_tile` 的 external source 传给 destroy。
2. 当前 destroy 会释放非空 `img_tile`，这对静态资源危险。

处理：

1. 新规则按 `kind` 判断。
2. `kind == 0` 视为 external，不释放底层指针。
3. 这更安全，且符合 public API 语义。

### 7.2 builtin resource 依赖旧图片头

风险：

1. TinyUI core 实现会 include 旧图片头。

处理：

1. 这是内部资源实现，不是 backend。
2. 不暴露到 public header。
3. 不暴露到 demo。
4. 不作为用户示例。

### 7.3 contract JSON 手改风险

风险：

1. 手改 JSON 容易破坏行数或字段。

处理：

1. 使用 JSON parser 更新。
2. 更新前后统计行数。
3. 运行三类 contract checker。

### 7.4 历史证据字段误解

风险：

1. 历史证据标签若继续使用 backend 口径会造成误解。

处理：

1. 本轮迁移为 `evidence_tag`。
2. current-facing 文档写清楚该字段是历史证据标签。
3. 评分和验收不引用底层适配字段。
4. contract checker 负责防止 ledger/matrix 字段漂移。

## 8. 非目标

本轮不做：

1. backend。
2. renderer。
3. fake visual。
4. 全工程 CMake/link target 低层命名清零。
5. 删除兼容 API。
6. 大规模重构 TinyUI widget 实现。
7. 把 `ld*` 逐字翻译成 `tinyui_*`。
8. 用改 demo 固定坐标掩盖控件能力问题。

## 9. 最终交付物

最终应交付：

1. 一个更严格的 demo boundary checker。
2. 一个 TinyUI public builtin resource API。
3. 干净的 `tinyui/demo/**` 和 `tinyui_demo/**`。
4. canonical table keyboard API。
5. deprecated alias 兼容测试。
6. 资源生命周期中文文档。
7. no-backend 口径修正后的 demo guide 和能力覆盖文档。
8. 全部指定门禁通过的验证记录。

完成后可以给出的质量结论：

> 按无 backend 的硬指标，TinyUI 已达到 public API 能力覆盖和 demo 洁净度双验收状态；能力覆盖维持 100%，API polish、资源生命周期和验证质量达到发布候选高置信水平。
