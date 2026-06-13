# TINYUI C线门禁工程化设计文档

> 日期：2026-05-29
> 适用仓库：`/Users/cys/embedded/LingDongGUI`
> 入口索引：`docs/tinyui-serial/C-线计划索引.md`
> 目标：把 `B线` 已有的自动 visible correctness 结果，收口成口径准确、CTest 可执行、长期可维护的 TINYUI 门禁体系。

---

## 1. 背景

`A线` 已完成 `TINYUI -> LingDongGUI` 真实 backend 主线收口。`B线` 已完成自动 visible gate，当前 `tests/tinyui/runtime/check_tinyui_visible_ui.py --all` 覆盖 6 个 `tinyui` demos，并且比早期 `capture 非空` 更严格：

1. 校验 PPM 尺寸。
2. 校验背景色与 theme 背景接近。
3. 校验亮度分位数，拦截近黑/不可读。
4. 校验可见内容 bounds。
5. 对 `basic_widgets` 增加重复列结构检查。
6. 拦截 `PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK`。

但 `B线` 收口后还存在三类工程化风险：

1. 文档里的“真实窗口”表述容易被误读成已经完成人工 OS 窗口验收。
2. `ctest --test-dir build -L tinyui --output-on-failure` 当前没有直接覆盖完整 visible gate 和 backend mapping gate。
3. `tests/tinyui/runtime/check_tinyui_backend_mapping.py` 的覆盖面窄于 visible gate，容易被误读成所有 demo 的全量 id mapping 都已覆盖。

`C线` 用来处理这些风险。它不扩大 TINYUI 功能面，只把已有验证体系变成长期可执行、可解释、可复用的门禁。

---

## 2. C线唯一目标

`C线` 的唯一目标是门禁工程化：

- 文档准确区分不同证据层级。
- `visible gate` 和 `backend mapping gate` 能通过 CTest 直接运行。
- 本地/CI 运行口径清晰。
- backend mapping matrix 显式化。
- 如需继续使用“人工真实窗口验收”表述，必须有可复现 artifact。
- 后续新增 demo/widget/layout/theme 时，有固定 gate 同步规则。

---

## 3. 非目标

`C线` 不处理以下事情：

1. 不新增 TINYUI public API。
2. 不新增控件。
3. 不扩复杂 theme/style 能力。
4. 不美化 demo 外观。
5. 不修改 demo 用户意图来让 marker 更容易通过。
6. 不把 `backend_app.c` 改回 TINYUI 专属 fake renderer。
7. 不把 `dummy SDL + PPM readback` 说成人工 OS 窗口验收。

如果执行过程中发现真实 backend/layout/theme 缺口，必须先记录为后续能力线问题；只有当该缺口直接阻断 C 线门禁工程化时，才允许在本线内做最小修复。

---

## 4. 证据层级

### 4.1 smoke gate

代表命令：

```bash
python3 tests/tinyui/runtime/check_tinyui_runtime.py
ctest --test-dir build -R check_tinyui_runtime --output-on-failure
```

能证明：

- demo target 可构建。
- demo 可进入 runtime loop。
- runtime marker 存在。

不能证明：

- UI 可读。
- 控件树显示正确。
- backend mapping 对所有 widget id 成立。
- 人工窗口验收通过。

### 4.2 backend mapping gate

代表命令：

```bash
python3 tests/tinyui/runtime/check_tinyui_backend_mapping.py
ctest --test-dir build -R check_tinyui_backend_mapping --output-on-failure
```

能证明：

- 脚本 matrix 中列出的 demo target 输出了预期 backend marker。
- `PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI` 存在。
- 指定 widget id 进入 `PICOUI_BACKEND_REAL_WIDGET_IDS`。
- 禁止出现不符合当前阶段的 `PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK`。

不能证明：

- matrix 未列出的 demo 或 widget id 已覆盖。
- UI 可读。
- 人工窗口验收通过。

### 4.3 automatic visible gate

代表命令：

```bash
python3 tests/tinyui/runtime/check_tinyui_visible_ui.py --all
ctest --test-dir build -R check_tinyui_visible_ui --output-on-failure
```

能证明：

- 在 `SDL_VIDEODRIVER=dummy` 下，demo 通过 PPM readback 可得到自动 visible correctness 证据。
- 6 个 demo 不是空白、近黑、明显错色或极小内容区域。
- `basic_widgets` 不再出现当前脚本能捕捉的重复列结构问题。

不能证明：

- 人工 OS 窗口肉眼验收通过。
- 所有布局/交互语义完全正确。
- 所有未来 demo 自动纳入 matrix。

### 4.4 manual window artifact gate

代表命令由 `C6` 定义，默认不进入无窗口 CI 的强制门禁。

能证明：

- 在明确平台、SDL video driver、demo target 和 artifact 路径下，存在人工窗口或非 dummy artifact 记录。
- 可用于支持“人工窗口验收通过”这类表述。

不能证明：

- 自动回归长期稳定。
- 替代 CTest。
- 替代 backend mapping gate。

---

## 5. CTest 标签设计

`C线` 后，TINYUI 相关 CTest 标签必须满足：

| 标签 | 含义 | 典型测试 |
| --- | --- | --- |
| `tinyui` | TINYUI 总入口标签 | unit/contract/runtime/visible/mapping |
| `unit` | C 层单元测试 | `test_tinyui_*` |
| `contract` | public API/demo boundary 检查 | `check_tinyui_public_api` |
| `runtime` | 可构建、可启动、可运行脚本 | `check_tinyui_runtime` |
| `visible` | 自动 visible correctness | `check_tinyui_visible_ui` |
| `backend` | backend marker 或 backend 行为证据 | `check_tinyui_backend_mapping` |
| `mapping` | backend mapping matrix | `check_tinyui_backend_mapping` |

`ctest -L tinyui` 是总入口，但执行汇报必须说明它当前包含了哪些标签。`C2/C3` 完成后，`ctest -L tinyui` 应包含 visible 和 mapping gate；在此之前，文档必须明确 standalone 脚本仍需单独运行。

---

## 6. backend mapping matrix 设计

`check_tinyui_backend_mapping.py` 当前覆盖：

- `tinyui_hello_world_demo`
- `tinyui_theme_showcase_demo`
- `tinyui_settings_panel_demo`

`C5` 后，脚本必须把覆盖面显式结构化。推荐结构：

```python
MAPPING_TARGETS = {
    "tinyui_hello_world_demo": {
        "category": "static",
        "required_markers": ["PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI"],
        "required_real_ids": [],
        "forbidden_markers": ["PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK"],
    },
    "tinyui_settings_panel_demo": {
        "category": "interactive",
        "required_markers": ["PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI"],
        "required_real_ids": ["title", "wifi", "brightness", "apply"],
        "forbidden_markers": ["PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK"],
    },
}
```

是否把所有 6 个 demo 都要求具体 `PICOUI_BACKEND_REAL_WIDGET_IDS`，由代码事实决定。若某个 demo 不适合要求具体 id，必须在脚本和文档中写明原因，而不是沉默跳过。

---

## 7. manual artifact 设计

`C6` 是可选阶段，但它决定能否继续使用“人工真实窗口验收”表述。

默认规则：

- 无 artifact：只能说 automatic visible gate 通过。
- 有 artifact：可以说 manual window artifact gate 通过。
- manual artifact 不进入无窗口 CI 的强制门禁。
- manual artifact 不替代 `visible` / `mapping` CTest。

artifact 记录至少包含：

```markdown
## YYYY-MM-DD <demo-name>

- 平台：
- SDL video driver：
- demo target：
- 构建目录：
- 运行命令：
- artifact 路径：
- 人工结论：
- 已知限制：
```

---

## 8. 长期维护规则

新增 demo 时必须同步：

1. `tests/tinyui/runtime/check_tinyui_runtime.py`
2. `tests/tinyui/runtime/check_tinyui_visible_ui.py`
3. `tests/tinyui/runtime/check_tinyui_backend_mapping.py` 的 matrix 或豁免说明
4. `tinyui/docs/demo_guide.md`
5. 对应 serial 文档的阶段状态

新增 widget 时必须同步：

1. public API contract
2. backend mapping test
3. visible gate 样本或明确不可见理由
4. theme/style 支持或拒绝说明

新增 layout/theme 能力时必须同步：

1. unit/contract test
2. runtime demo 样本
3. visible gate 或明确不可见理由
4. gate matrix 文档

---

## 9. 执行边界

`C线` 必须串行推进：

1. `C1` 先修文档口径。
2. `C2` 再接 visible CTest。
3. `C3` 再接 mapping CTest。
4. `C4` 再写统一运行矩阵。
5. `C5` 再扩 mapping matrix。
6. `C6` 再补可选人工 artifact。
7. `C7` 再固化长期规则。
8. `C8` 最后 review 与收口。

每个阶段完成后都必须保留证据：改动文件、验证命令、是否存在人工窗口 artifact、仍不能证明什么。

---

## 10. 验收口径

`C线` 收口时必须同时满足：

- 文档不再混用 automatic visible gate 和 manual window artifact。
- `check_tinyui_visible_ui.py --all` 已接入 CTest。
- `check_tinyui_backend_mapping.py` 已接入 CTest。
- `ctest -L tinyui`、`ctest -L visible`、`ctest -L mapping` 的关系已写清。
- backend mapping matrix 覆盖面显式，不再被误读成所有 demo 全量 id 覆盖。
- 新增 demo/widget/layout/theme 的 gate 同步规则已写清。
- 若使用人工窗口验收表述，必须存在 artifact 记录。
- 最终 review 无阻塞问题。
