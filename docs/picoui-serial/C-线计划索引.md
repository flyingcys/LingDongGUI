# PicoUI C线计划索引

- `A线` 收口索引：`docs/picoui-serial/A-线计划索引.md`
- `B线` 收口索引：`docs/picoui-serial/B-线计划索引.md`
- `PicoUI` 总设计真相源：`docs/superpowers/specs/2026-05-26-picoui-abstraction-layer-design.md`
- `PicoUI` 测试架构真相源：`docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- `B线` 可见 UI 设计真相源：`docs/superpowers/specs/2026-05-27-picoui-b-line-visible-ui-design.md`
- `B线` 实施口径：`docs/superpowers/plans/2026-05-27-picoui-b-line-visible-ui-implementation.md`
- `C线` 总设计真相源：`docs/superpowers/specs/2026-05-29-picoui-c-line-gate-hardening-design.md`
- `C线` 总实施口径：`docs/superpowers/plans/2026-05-29-picoui-c-line-gate-hardening-implementation.md`
- 当前仓库硬规则：`AGENTS.md`

## 当前状态

- `A线` 已把 `PicoUI -> LingDongGUI` 真实 backend 主线收口。
- `B线` 已建立自动 visible gate，并覆盖 6 个 `picoui` demos：
  - `hello_world`
  - `basic_widgets`
  - `layout_flex`
  - `layout_grid`
  - `theme_showcase`
  - `settings_panel`
- `B线` 当前证据强于 `capture 非空`，但仍主要是 `SDL_VIDEODRIVER=dummy + PPM readback` 的自动化 visible gate。
- 最新 review 明确了三个后续风险：
  - 文档中“真实窗口”表述容易被误读成肉眼 OS 窗口验收已经完成。
  - `ctest --test-dir build -L picoui --output-on-failure` 当前没有包含完整 visible gate 和 backend mapping gate。
  - `check_picoui_backend_mapping.py` 的映射矩阵比 visible gate 覆盖面窄。

## C线目标

- **C线唯一目标**：把 `B线` 已有的 visible correctness 结果，从“可手动运行的自动化脚本证据”推进到“文档口径准确、CTest/CI 可持续执行、mapping matrix 可扩展、人工窗口证据可选补强”的长期工程化门禁。

## C线不是什么

- `C线` 不是新控件开发线。
- `C线` 不是继续美化 demo 外观。
- `C线` 不是把 `backend_app.c` 改回 fake renderer。
- `C线` 不用 demo 侧硬编码来弥补 backend/layout/theme 缺口。
- `C线` 不把 `dummy SDL + PPM readback` 说成等同于人工肉眼窗口验收。

## 当前游标

- 当前活跃游标：`C6` 可选人工窗口 artifact gate
- 当前允许进入：`C6` review
- 当前禁止进入：
  - 先扩 `PicoUI` 新 public API
  - 先扩新控件
  - 先做复杂 theme/style 能力
  - 继续用“真实窗口”宽泛表述覆盖不同证据层级
  - 在 `C6` review 收口前进入 `C7` 长期回归规则与新增 demo 规则

## 阶段导航

- `C0`：计划冻结与证据分层重命名
- `C1`：文档口径收紧，修正“真实窗口”过度声称
- `C2`：把 visible gate 接入 CTest
- `C3`：把 backend mapping gate 接入 CTest
- `C4`：建立 gate 执行矩阵与 CI/本地运行口径
- `C5`：扩展 backend mapping matrix，避免覆盖面被误读
- `C6`：补可选人工窗口 artifact gate
- `C7`：长期回归规则与新增 demo 规则
- `C8`：C线 closeout review 与收口

## 串行规则

- `C1` 未完成前，不进入 `C2`
- `C2` 未完成前，不进入 `C3`
- `C3` review 未收口前，不进入 `C4`
- `C4` 未完成前，不进入 `C5`
- `C5` 未完成前，不进入 `C6`
- `C6` 未完成前，不进入 `C7`
- `C7` 未完成前，不进入 `C8`
- 每个阶段完成后必须记录：
  - 改了哪些文件
  - 跑了哪些命令
  - 哪些证据只能证明 smoke
  - 哪些证据能证明自动 visible correctness
  - 是否存在人工窗口证据

## 逐阶段入口

### C0 计划冻结与证据分层重命名

- 当前目标：
  - 把 `C线` 定义成 `B线` 之后的门禁工程化主线。
  - 冻结证据分层，避免后续继续混用术语。
- 必须完成的事情：
  - 新增本文。
  - 把后续工作拆成只能串行推进的阶段。
  - 明确 `dummy SDL + PPM readback`、`CTest gate`、`manual OS-window artifact` 三类证据的边界。
- 阶段完成判定：
  - 本文存在，并成为 `C线` 唯一入口索引。
  - 后续执行者可以只读本文就知道下一步先做什么、不能做什么。
- 收口证据：
  - `docs/picoui-serial/C-线计划索引.md` 已创建。
  - `git status --short` 只显示预期文档变更。

### C1 文档口径收紧

- 当前目标：
  - 修正 `B线` 文档中容易把自动 visible gate 过度解释成“人工真实窗口验收”的表述。
- 必须修改的文件：
  - `docs/picoui-serial/B-线计划索引.md`
  - `docs/picoui-serial/C-线计划索引.md`
  - `docs/superpowers/specs/2026-05-27-picoui-b-line-visible-ui-design.md`
  - `docs/superpowers/plans/2026-05-27-picoui-b-line-visible-ui-implementation.md`
  - `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
  - `picoui/docs/demo_guide.md`
- 必须完成的事情：
  - 把“真实窗口里 UI 可正常显示、可读、可判定”收紧为“自动 visible gate 已证明 dummy SDL + PPM readback 下可显示、可读、可判定”。
  - 保留用户真实窗口反馈作为 `B线` 启动背景，不再把它写成已经有完整人工验收 artifact。
  - 在文档中明确：若要声称人工窗口验收，需要执行 `C6 / manual window artifact gate`。
  - 在测试架构文档中补齐四类 gate 边界：
    - `smoke gate`
    - `backend mapping gate`
    - `automatic visible gate`
    - `manual window artifact gate`
  - 在 demo guide 中写清直接运行 demo、自动 visible gate、人工窗口观察三者的区别。
- 阶段完成判定：
  - 文档不再暗示 `SDL_VIDEODRIVER=dummy` 等同于人工 OS 窗口验收。
  - 文档仍然承认 `B线` 自动 visible gate 的价值，不退回到“只能证明 capture 非空”。
- 验证命令：

```bash
rg -n "真实窗口|visible gate|SDL_VIDEODRIVER|PICOUI_CAPTURE_FILE|人工|manual" \
  docs/picoui-serial/B-线计划索引.md \
  docs/superpowers/specs/2026-05-27-picoui-b-line-visible-ui-design.md \
  docs/superpowers/plans/2026-05-27-picoui-b-line-visible-ui-implementation.md \
  docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md \
  picoui/docs/demo_guide.md
```

- 收口证据：
  - 上述 `rg` 输出中，每个“真实窗口/人工/manual”相关表述都有明确证据层级。
  - `git diff --check` 通过。

### C2 visible gate 接入 CTest

- 当前目标：
  - 让 `check_picoui_visible_ui.py --all` 成为 `ctest` 可执行的正式门禁，而不是只能靠人工记得单独运行。
- 必须修改的文件：
  - `tests/picoui/CMakeLists.txt`
  - 必要时修改 `tests/picoui/runtime/check_picoui_visible_ui.py`
  - 必要时修改 `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- 必须完成的事情：
  - 新增 CTest 名称：`check_picoui_visible_ui`
  - 脚本参数固定为 `--all`
  - 标签至少包含：
    - `picoui`
    - `runtime`
    - `visible`
  - 失败时必须保留当前脚本的具体失败语义，例如：
    - `color/readback check failed`
    - `near-black/readability check failed`
    - `duplicate-column/structure check failed`
    - `FAKE_FALLBACK`
- 阶段完成判定：
  - `ctest --test-dir build -R check_picoui_visible_ui --output-on-failure` 可直接执行。
  - `ctest --test-dir build -L visible --output-on-failure` 能跑到该测试。
- 验证命令：

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build --target check_picoui_visible_ui
ctest --test-dir build -R check_picoui_visible_ui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
```

- 收口证据：
  - `check_picoui_visible_ui` 在 CTest 中可见并通过。
  - `python3 tests/picoui/runtime/check_picoui_visible_ui.py --all` 仍可独立通过。
  - 本轮已接入 `tests/picoui/CMakeLists.txt`，CTest 名称为 `check_picoui_visible_ui`，固定参数为 `--all`，labels 为 `picoui;runtime;visible`。
  - 本轮验证通过：
    - `rtk cmake -S . -B build -DUSE_DEMO=0`
    - `ctest --test-dir build -N -R check_picoui_visible_ui`
    - `ctest --test-dir build -R check_picoui_visible_ui --output-on-failure`
    - `ctest --test-dir build -L visible --output-on-failure`
    - `python3 tests/picoui/runtime/check_picoui_visible_ui.py --all`

### C3 backend mapping gate 接入 CTest

- 当前目标：
  - 让 `check_picoui_backend_mapping.py` 成为 `ctest` 可执行的正式门禁。
- 必须修改的文件：
  - `tests/picoui/CMakeLists.txt`
  - 必要时修改 `tests/picoui/runtime/check_picoui_backend_mapping.py`
  - 必要时修改 `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- 必须完成的事情：
  - 新增 CTest 名称：`check_picoui_backend_mapping`
  - 标签至少包含：
    - `picoui`
    - `runtime`
    - `backend`
    - `mapping`
  - 保留脚本现有 marker 检查：
    - `PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI`
    - `PICOUI_BACKEND_REAL_WIDGET_IDS=...`
    - 禁止 `PICOUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK`
- 阶段完成判定：
  - `ctest --test-dir build -R check_picoui_backend_mapping --output-on-failure` 可直接执行。
  - `ctest --test-dir build -L mapping --output-on-failure` 能跑到该测试。
- 验证命令：

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build --target check_picoui_backend_mapping
ctest --test-dir build -R check_picoui_backend_mapping --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
```

- 收口证据：
  - `check_picoui_backend_mapping` 在 CTest 中可见并通过。
  - `python3 tests/picoui/runtime/check_picoui_backend_mapping.py` 仍可独立通过。
  - 本轮已接入 `tests/picoui/CMakeLists.txt`，CTest 名称为 `check_picoui_backend_mapping`，labels 为 `picoui;runtime;backend;mapping`。
  - 本轮验证命令：
    - `ctest --test-dir build -N -R check_picoui_backend_mapping`
    - `ctest --test-dir build -R check_picoui_backend_mapping --output-on-failure`
    - `ctest --test-dir build -L mapping --output-on-failure`
    - `python3 tests/picoui/runtime/check_picoui_backend_mapping.py`

### C4 gate 执行矩阵与 CI/本地运行口径

- 当前目标：
  - 固定后续每次 PicoUI 改动必须跑哪些 gate，避免 `ctest -L picoui` 与 standalone 脚本再次分裂。
- 必须修改的文件：
  - `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
  - `docs/picoui-serial/C-线计划索引.md`
  - `picoui/docs/demo_guide.md`
  - 当前存在 `.github/workflows/cmake-single-platform.yml`，但它是 pack/release workflow，不在 `C4` 内改动。
- 必须完成的事情：
  - 明确主项目当前存在 `.github/workflows/cmake-single-platform.yml`。
  - 明确该 workflow 是 `workflow_dispatch` / `release published` 触发的 `build pack` workflow，执行 `gen_pack.sh` 与 `Open-CMSIS-Pack/gen-pack-action`，不是现有测试 workflow。
  - 明确 `C4` 只记录本地 gate 矩阵，不改 workflow、不新造 CI 框架。
  - 若以后给主项目 CI 接入 PicoUI gate，必须复用本阶段固定的同一 gate 矩阵，不另开一套说法。
  - 明确最小本地门禁：

```bash
ctest --test-dir build -L picoui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
```

  - 明确完整本地门禁：

```bash
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
ctest --test-dir build -L picoui --output-on-failure
```

  - 明确汇报规则：
    - `smoke gate` 通过，只能说可启动、可进入 runtime loop。
    - `visible gate` 通过，只能说自动 visible correctness 通过。
    - `backend mapping gate` 通过，只能说 marker 覆盖的 backend 映射通过。
    - `manual artifact gate` 通过后，才允许说人工窗口验收通过。
- 阶段完成判定：
  - 后续执行者不会再只跑 `ctest -L picoui` 就声称完整 B/C 线通过。
  - 文档中每个 gate 都有命令、用途和不能证明的边界。
  - 当前游标只推进到 `C4 / C4 review`，禁止 `C4 review` 收口前进入 `C5`。
- 收口证据：
  - `.github/workflows/cmake-single-platform.yml` 的 pack/release workflow 现状已写清；后续 CI 若接入 PicoUI gate，必须复用同一矩阵。
  - `rg -n "smoke gate|visible gate|backend mapping gate|manual artifact gate|ctest --test-dir build -L" docs picoui/docs` 输出清晰。
  - `git diff --check` 通过。

### C5 扩展 backend mapping matrix

- 当前目标：
  - 让 backend mapping gate 的覆盖面与当前 6 个 demo 的 visible matrix 对齐，避免“部分 mapping 检查”被误读成“所有 demo 的所有 widget id 都已检查”。
- 必须修改的文件：
  - `tests/picoui/runtime/check_picoui_backend_mapping.py`
  - 必要时修改 `picoui/demo/*/main.c`
  - 必要时修改 `picoui/src/backend/ldgui/backend_app.c`
  - `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
- 必须完成的事情：
  - 先把当前脚本内的覆盖矩阵显式结构化，至少区分：
    - `static_mapping_targets`
    - `interactive_mapping_targets`
    - `layout_mapping_targets`
    - `theme_mapping_targets`
  - 对 6 个 demo 逐个明确是否要求 `PICOUI_BACKEND_REAL_WIDGET_IDS`。
  - 对不适合要求具体 id 的 demo，在脚本和文档里写清原因。
  - 禁止为了让 marker 好看而改 demo 语义。
- 阶段完成判定：
  - backend mapping gate 不再只是检查 3 个 target，却让读者误以为覆盖 6 个 demo。
  - 每个 demo 的 mapping 检查范围都有显式说明。
- 验证命令：

```bash
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
ctest --test-dir build -R check_picoui_backend_mapping --output-on-failure
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
```

- 收口证据：
  - backend mapping 脚本输出失败时能指出具体 demo 与具体缺失 marker/id。
  - visible gate 仍覆盖 6 个 demo 并通过。
  - 本轮已把脚本矩阵显式拆成 `static_mapping_targets`、`interactive_mapping_targets`、`layout_mapping_targets`、`theme_mapping_targets`。
  - 6 个 demo 均要求 `PICOUI_BACKEND_REAL_WIDGET_IDS`，没有为了 marker 修改 demo 或 backend：
    - `picoui_hello_world_demo`：`title`, `ok`
    - `picoui_basic_widgets_demo`：`wifi`, `agree`, `volume`, `submit`, `title`, `logo`
    - `picoui_layout_flex_demo`：`first`, `second`, `third`
    - `picoui_layout_grid_demo`：`title`, `left`, `right`
    - `picoui_theme_showcase_demo`：`title`, `body`, `accent`
    - `picoui_settings_panel_demo`：`title`, `wifi`, `brightness`, `apply`
  - layout/theme demo 的 marker 只证明命名对象进入真实 backend tree，不证明 layout solver、theme/style readback 或人工窗口验收；这些仍分别由 unit test、visible gate、C6 artifact gate 证明。
  - 当前游标只推进到 `C5 / C5 review`，禁止 `C5 review` 收口前进入 `C6`。

### C6 可选人工窗口 artifact gate

- 当前目标：
  - 若后续需要继续使用“真实窗口”表述，补一条可复现的人工窗口 artifact gate。
- 必须修改的文件：
  - 新增 `tests/picoui/runtime/check_picoui_manual_window_artifact.py`
  - 修改 `picoui/docs/demo_guide.md`
  - 新增 `docs/picoui-serial/C-线人工窗口验收记录.md`
- 必须完成的事情：
  - 定义非 dummy SDL 运行方式，默认不在无窗口 CI 中强制执行。
  - 产出 artifact 路径，例如：

```text
artifacts/picoui/manual-window/<demo-name>/frame.ppm
```

  - 至少覆盖：
    - `picoui_basic_widgets_demo`
    - `picoui_settings_panel_demo`
  - 记录运行环境：
    - 日期
    - 平台
    - SDL video driver
    - demo target
    - artifact 路径
    - 人工结论
- 阶段完成判定：
  - 没有人工 artifact 时，文档不允许写“人工真实窗口验收通过”。
  - 有人工 artifact 时，可以明确写“人工窗口 artifact gate 通过”，但不能替代 CTest 自动 gate。
- 验证命令：

```bash
rtk cmake -S . -B build -DUSE_DEMO=0
rtk cmake --build build --target picoui_basic_widgets_demo picoui_settings_panel_demo
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo basic_widgets
python3 tests/picoui/runtime/check_picoui_manual_window_artifact.py --demo settings_panel
```

- 收口证据：
  - artifact 文件存在。
  - 验收记录文档包含平台、命令、artifact 路径和结论。
  - CI 默认不因缺少窗口环境失败。
  - 本轮只新增可选人工 artifact gate；未接入 CTest，未修改 CI。
  - 当前游标只推进到 `C6 / C6 review`，禁止 `C6 review` 收口前进入 `C7`。

### C7 长期回归规则与新增 demo 规则

- 当前目标：
  - 固定后续新增 demo、新增 widget、新增 layout/theme 行为时必须同步维护哪些 gate。
- 必须修改的文件：
  - `docs/picoui-serial/C-线计划索引.md`
  - `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
  - `picoui/docs/demo_guide.md`
  - 必要时修改 `AGENTS.md` 或项目局部规则文档；若修改全局规则，必须先单独 review。
- 必须完成的事情：
  - 新增 demo 时，必须同步：
    - `check_picoui_runtime.py`
    - `check_picoui_visible_ui.py`
    - `check_picoui_backend_mapping.py` 的适用矩阵或豁免说明
    - demo guide
  - 新增 widget 时，必须同步：
    - public API contract
    - backend mapping test
    - visible gate 样本
    - theme/style 适用性说明
  - 新增 layout/theme 能力时，必须同步：
    - unit/contract test
    - runtime demo 样本
    - visible gate 或明确的不可见理由
- 阶段完成判定：
  - 后续开发不会只改 demo 或只改 backend，却忘记更新 gate matrix。
  - 每类新增工作都有固定验收路径。
- 收口证据：
  - 文档存在“新增 demo/widget/layout/theme 的 gate 同步清单”。
  - `rg -n "新增 demo|新增 widget|gate 同步|visible matrix|mapping matrix" docs picoui/docs` 能找到唯一口径。

### C8 C线 closeout review 与收口

- 当前目标：
  - 对 `C1-C7` 做只读 review，确认代码、测试、文档口径一致。
- 必须完成的事情：
  - 使用独立 subagent 做 doc/code consistency review。
  - 主线程运行最终验证命令。
  - 只在 review 通过后，把本文当前状态改为 `C8 closeout`。
- 最终验证命令：

```bash
git status --short
python3 tests/picoui/runtime/check_picoui_runtime.py
python3 tests/picoui/runtime/check_picoui_visible_ui.py --all
python3 tests/picoui/runtime/check_picoui_backend_mapping.py
ctest --test-dir build -L picoui --output-on-failure
ctest --test-dir build -L visible --output-on-failure
ctest --test-dir build -L mapping --output-on-failure
git diff --check
```

- 阶段完成判定：
  - review 无阻塞问题。
  - 上述命令全部通过，或失败项被明确记录为非 C线 blocker。
  - 文档不再把不同证据层级混写。
- 收口证据：
  - 本文 `当前状态` 更新为 `C线 已按 C0 -> C8 顺序完成`。
  - 最终汇报区分：
    - 当前阶段完成
    - 自动 gate 完成
    - 是否有人工窗口 artifact
    - 后续新能力线是否可以启动

## 当前明确做什么

1. 先修证据口径，再接入 CTest。
2. 先让 `visible` / `mapping` gate 可被 `ctest` 直接执行，再扩 mapping matrix。
3. 若继续使用“真实窗口”表述，必须补 `C6` 的人工窗口 artifact gate。
4. 后续每次汇报都必须区分：
   - smoke
   - backend mapping
   - automatic visible gate
   - manual window artifact

## 当前明确不做什么

1. 不开始新控件开发。
2. 不开始新 public API 设计。
3. 不把 `dummy SDL + PPM readback` 说成人工窗口验收。
4. 不只跑 `ctest -L picoui` 就声明 `B/C` 线完整通过。
5. 不为了让 mapping marker 通过而改 demo 表达的用户意图。

## 推荐阅读顺序

1. 先读 `docs/picoui-serial/A-线计划索引.md`
2. 再读 `docs/picoui-serial/B-线计划索引.md`
3. 再读本文
4. 再读 `docs/superpowers/specs/2026-05-26-picoui-lingdonggui-test-architecture-design.md`
5. 最后读具体阶段会修改的脚本和 CMake 文件

## 退出口径

下面条件同时成立后，`C线` 才能收口：

- 文档明确区分 `dummy SDL + PPM readback` 和人工 OS 窗口验收。
- `check_picoui_visible_ui.py --all` 已接入 CTest。
- `check_picoui_backend_mapping.py` 已接入 CTest。
- `ctest -L picoui`、`ctest -L visible`、`ctest -L mapping` 的关系已写清。
- backend mapping matrix 不再被误读成 6 个 demo 全量 id 覆盖。
- 新增 demo/widget/layout/theme 的 gate 同步规则已写清。
- 若文档继续声称人工窗口验收，则必须有人工窗口 artifact 记录；否则只能声称自动 visible gate 通过。
