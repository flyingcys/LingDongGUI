# TinyUI v2.3 M5 硬化与 ABI 冻结实施计划

> **面向执行代理：** 必须使用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans`，逐任务执行本计划，并使用复选框（`- [ ]`）跟踪步骤。

**目标：** 用 fresh 的全量构建、sanitizer、轻量预算、能力证据、独立评审和 GitNexus 结果关闭所有非 port P0/P1，并只在所有门禁通过后冻结一次 v2.3 canonical ABI。

**架构：** M5 不新增产品能力，先把设计中的门禁变成 fail-closed 机器清单，再在互相隔离的 clean build 中运行标准、ASan、UBSan、minimal 和安装消费者 profile。能力矩阵、人工 review、独立代码评审、GitNexus diff 与 ABI manifest 必须引用同一批 fresh 证据；port 延期项保持明确未完成。

**技术栈：** C11、C++17、CMake/CTest、Clang ASan/UBSan、Python 3、JSON、`size`/`nm`/link map、GitNexus、Markdown。

## 全局约束

- M0-M4 必须完成，且所有输入 baseline、schema、证据和安装 consumer 可重跑；缺失、fallback 或陈旧证据一律失败。
- M5 不修 port-production，不修改具体 SDL/MCU port 契约，不把测试宿主结果描述为 L6。
- wrapper：基础上限 `192 B`；其他 wrapper 相对 M0 baseline 增量不得同时超过 `5%` 和 `8 B`，且不得超过逐项绝对安全上限；backend widget 上限 `512 B`。
- 32 位 ABI 下 timer pool、event callback pool 和 runtime bookkeeping 静态 RAM 合计不得超过 `1024 B`。
- 32 位 ABI 下 `sizeof(tinyui_image_source_t) <= 80 B`、`sizeof(tinyui_font_t) <= 16 B`；两块 image private storage 均须容纳 `arm_2d_tile_t`。
- binary 双阈值：`__text` 为 `5%/8192 B`、`__data` 为 `25%/256 B`、`__bss` 为 `10%/8192 B`、`total` 为 `5%/12288 B`；相对与绝对同时越线才失败。
- `tinyui_process()` 每轮隐式 heap 分配必须为 `0`；layout、event dispatch、theme apply 和普通 getter 同样必须为 `0`。
- 时间采样先 warmup `5` 次，再独立运行 `30` 次，同时记录 median/p95；`screen_object_create_ms` p95 `<= 5 ms`，`capture_ready_ms` p95 `<= 40 ms`。
- 时间基线仅在 OS/版本、CPU/架构、compiler/版本/target triple、build type/优化/完整选项、baseline commit 全部一致时比较；不一致时 fail-closed。
- required 能力至少 L4；可见能力必须 L5-V，可操作能力必须 L5-E；二者兼具时两种证据都必须存在。
- v2.3 关闭时不得存在未解决 P0/P1 非 port 问题；P2 必须有明确 disposition。
- 本项目不创建 worktree。本文不包含提交步骤；所有 shell 命令必须经 `rtk` 执行。

---

## 文件结构

新增：

- `tests/tinyui/contract/tinyui_v23_release_gate_manifest.json`：最终 gate 名称、label、artifact 和阈值真相源。
- `tests/tinyui/contract/check_tinyui_v23_release_gates.py`：fail-closed 聚合检查器。
- `tests/tinyui/contract/tinyui_v23_abi_manifest.json`：冻结后的 canonical 头、函数签名、枚举值和导出符号清单。
- `docs/v2.3/v2.3-release-review.md`：独立 reviewer 的问题、修复和复核记录。

修改：

- `tests/tinyui/CMakeLists.txt`：注册最终 release gate。
- `tests/tinyui/contract/check_tinyui_public_api.py`：生成/校验 ABI manifest，禁止静默刷新。
- `tests/tinyui/contract/tinyui_release_capability_matrix.json`：每项绑定 L1-L5 具体证据。
- `tests/tinyui/contract/check_tinyui_release_capability_matrix.py`：拒绝缺证据、错误 policy 和 port 冒充完成。
- `tests/tinyui/perf/tinyui_perf_baseline.json`：只在指纹一致且有审查记录时冻结 v2.3 baseline。
- `tests/tinyui/perf/check_tinyui_object_overhead.py`：全部 wrapper/backend/pool/descriptor ABI gate。
- `tests/tinyui/perf/check_tinyui_binary_size.py`：双阈值和 baseline schema gate。
- `tests/tinyui/perf/check_tinyui_perf.py`：5+30、median/p95 和指纹 gate。
- `docs/v2.3/v2.3-capability-evidence-matrix.md`：机器矩阵的人类可读投影。
- `docs/v2.3/v2.3-performance-baseline.md`：最终基线、指纹和统计结果。
- `docs/v2.3/v2.3-release-gates.md`：门禁结果与人工检查表。
- `docs/v2.3/deferred-port-work.md`：保持 port 明确延期。

## 最终 gate 标识

release manifest 必须引用下表精确 CTest 名。M0-M4 若使用过不同名称，进入 M5 前先统一注册为下表名称；不保留同义 gate，也不允许通过删除 gate 缩小覆盖面：

| 类别 | CTest/target |
| --- | --- |
| 公共头 C/C++ | `check_tinyui_public_headers_c`、`check_tinyui_public_headers_cpp` |
| 公共符号链接 | `check_tinyui_public_symbols_link` |
| inventory | `check_ldgui_public_api_inventory` |
| demo/文档/安装 | `check_tinyui_demo_boundary`、`check_tinyui_docs_examples`、`check_tinyui_install_consumer` |
| runtime/backend/视觉/事件 | `check_tinyui_runtime`、`check_tinyui_backend_mapping`、`check_tinyui_visible_ui`、`check_tinyui_event_evidence` |
| wrapper/pool/descriptor | `test_tinyui_wrapper_struct_overhead`、`test_tinyui_pool_abi`、`test_tinyui_descriptor_abi` |
| 分配 | `test_tinyui_steady_state_allocation` |
| binary/time/minimal | `check_tinyui_binary_size`、`check_tinyui_perf`、`check_tinyui_minimal_profile` |
| 能力/ABI/release | `check_tinyui_release_capability_matrix`、`check_tinyui_public_api`、`check_tinyui_v23_release_gates` |

### 任务 1：建立 fail-closed release gate manifest

**文件：**

- 新增：`tests/tinyui/contract/tinyui_v23_release_gate_manifest.json`
- 新增：`tests/tinyui/contract/check_tinyui_v23_release_gates.py`
- 修改：`tests/tinyui/CMakeLists.txt`

**接口：**

- 消费：CTest JSON inventory、性能/能力/ABI JSON、安装和 runtime artifact。
- 产出：唯一最终聚合 CTest `check_tinyui_v23_release_gates`，labels 为 `tinyui;contract;release;v2.3`。

- [ ] **步骤 1：先写缺项测试**

checker 接受 `--build-dir`、`--manifest` 和 `--artifact-root`；读取 `ctest --show-only=json-v1` 输出，逐项验证 required test 存在且未 disabled，artifact 存在且 schema/version 完整，阈值与设计一致。对缺 test、缺 artifact、空 evidence、`fallback=true`、`status=pending`、未知字段均返回非零。

- [ ] **步骤 2：写完整 manifest**

manifest 必须包含：上表全部 gate 名；M0 performance baseline JSON；M3 capability matrix JSON；M4 install consumer 和 docs compile 结果；M5 standard/ASan/UBSan/minimal 结果；wrapper/pool/descriptor/binary/time/allocation 阈值；`manual_reviewed_passed`；`port_completed=false`；`abi_frozen=true`。所有路径必须相对 repo 或 artifact root，不保存开发机绝对路径。

- [ ] **步骤 3：运行并确认先失败**

运行：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_v23_release_gates.py --build-dir build/v2.3 --manifest tests/tinyui/contract/tinyui_v23_release_gate_manifest.json --artifact-root build/v2.3/release-evidence
```

预期：FAIL，列出尚未生成的 fresh standard/ASan/UBSan/minimal/review/ABI artifact；不得因 traceback 失败。

- [ ] **步骤 4：注册 CTest**

CTest 只在前置 artifact 生成后运行，不能用宽泛正则跳过；聚合器不得自己把失败 gate 标成通过。

运行：

```bash
rtk git diff --check
```

预期：PASS。

### 任务 2：运行 clean 标准 CMake/CTest 和消费者全量 gate

**文件：**

- 生成：`build/v2.3/release-evidence/standard.json`
- 不修改产品代码；若失败，回到拥有该问题的原任务修复并重新从 clean build 开始。

**接口：**

- 消费：M0-M4 全部 target/test。
- 产出：标准 profile 零失败证据。

- [ ] **步骤 1：清理并配置标准 profile**

运行：

```bash
rtk cmake -E remove_directory build/v2.3
rtk cmake -S . -B build/v2.3 -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_VISUAL_TESTS=OFF -DLD_TINYUI_PORT=sdl
```

预期：配置 PASS；不得报告缺 source、target 重名、inventory 漂移或公共头泄漏。

- [ ] **步骤 2：构建全部 target**

运行：

```bash
rtk cmake --build build/v2.3 -j
```

预期：PASS，零 warning-as-error 失败，全部 demo/test/consumer prerequisite 构建完成。

- [ ] **步骤 3：运行全部注册 CTest**

运行：

```bash
rtk ctest --test-dir build/v2.3 --output-on-failure
```

预期：100% tests passed，0 tests failed；不得只跑 TinyUI label 子集代替全量。

- [ ] **步骤 4：单独复核交付边界**

运行：

```bash
rtk ctest --test-dir build/v2.3 -R 'check_tinyui_(demo_boundary|docs_examples|install_consumer)' --output-on-failure
```

预期：3/3 PASS，仓外 C/C++ consumer 使用安装包完成配置、链接和运行。

- [ ] **步骤 5：写 standard 机器证据**

`standard.json` 必须记录 git commit、CMake cache 中全部 TinyUI 选项、compiler ID/version/target、测试总数/通过数、开始/结束时间和日志 SHA-256；任一字段不得为 null。

### 任务 3：分别运行 Clang ASan 与 UBSan 全量 TinyUI core/contract gate

**文件：**

- 生成：`build/v2.3/release-evidence/asan.json`
- 生成：`build/v2.3/release-evidence/ubsan.json`
- 必要时修改：`tests/tinyui/CMakeLists.txt`，仅给确属第三方且有精确 label 的 test 标注 sanitizer 排除。

**接口：**

- 消费：object/widget/event/timer/theme/layout/resource/demo boundary tests。
- 产出：两个互相隔离、无 sanitizer 报告的 profile。

- [ ] **步骤 1：配置并构建 ASan**

运行：

```bash
rtk cmake -E remove_directory build/v2.3-asan
rtk cmake -S . -B build/v2.3-asan -DENABLE_TEST=ON -DLD_BUILD_RUNTIME_TESTS=ON -DCMAKE_C_COMPILER=clang -DCMAKE_C_FLAGS='-fsanitize=address -fno-omit-frame-pointer'
rtk cmake --build build/v2.3-asan -j
```

预期：配置和构建 PASS；TinyUI core/unit/contract target 均带 ASan flags。

- [ ] **步骤 2：运行 ASan CTest**

运行：

```bash
rtk ctest --test-dir build/v2.3-asan --output-on-failure
```

预期：全部注册测试 PASS，日志无 AddressSanitizer error、leak、use-after-free、double-free 或 OOB。

- [ ] **步骤 3：配置并构建 UBSan**

运行：

```bash
rtk cmake -E remove_directory build/v2.3-ubsan
rtk cmake -S . -B build/v2.3-ubsan -DENABLE_TEST=ON -DLD_BUILD_RUNTIME_TESTS=ON -DCMAKE_C_COMPILER=clang -DCMAKE_C_FLAGS='-fsanitize=undefined -fno-omit-frame-pointer'
rtk cmake --build build/v2.3-ubsan -j
```

预期：配置和构建 PASS；TinyUI core/unit/contract target 均带 UBSan flags。

- [ ] **步骤 4：运行 UBSan CTest**

运行：

```bash
rtk ctest --test-dir build/v2.3-ubsan --output-on-failure
```

预期：全部注册测试 PASS，日志无 undefined behavior、signed overflow、invalid shift、misaligned access 或 invalid enum。

- [ ] **步骤 5：审计 sanitizer 覆盖面**

运行：

```bash
rtk ctest --test-dir build/v2.3-asan --show-only=json-v1
rtk ctest --test-dir build/v2.3-ubsan --show-only=json-v1
```

预期：object/widget/event/timer/theme/layout/resource/demo boundary 全部存在；任何排除只允许精确第三方 label，并在 ASan/UBSan JSON 中记录 test 名和原因，不允许 `-E tinyui` 或宽泛正则。

### 任务 4：关闭 wrapper、pool、descriptor、binary、time、零分配和 minimal profile

**文件：**

- 修改：`tests/tinyui/perf/check_tinyui_object_overhead.py`
- 修改：`tests/tinyui/perf/check_tinyui_binary_size.py`
- 修改：`tests/tinyui/perf/check_tinyui_perf.py`
- 修改：`tests/tinyui/perf/tinyui_perf_baseline.json`
- 修改：`docs/v2.3/v2.3-performance-baseline.md`
- 生成：`build/v2.3/release-evidence/lightweight-gates.json`

**接口：**

- 消费：M0 baseline、32 位 ABI probe、link map/nm、allocator counter、host fingerprint。
- 产出：设计第 6/22.3 节全部硬预算的单项证据。

- [ ] **步骤 1：运行 wrapper/pool/descriptor ABI probe**

运行：

```bash
rtk ctest --test-dir build/v2.3 -R '^(test_tinyui_wrapper_struct_overhead|test_tinyui_pool_abi|test_tinyui_descriptor_abi|check_tinyui_object_overhead)$' --output-on-failure
```

预期：4/4 PASS；输出逐项列出全部 wrapper 当前值/baseline/绝对上限，backend widget `<=512 B`，pool+bookkeeping `<=1024 B`，image/font `<=80/16 B`，两块 tile storage ABI probe PASS。

- [ ] **步骤 2：运行 binary 双阈值**

运行：

```bash
rtk ctest --test-dir build/v2.3 -R '^check_tinyui_binary_size$' --output-on-failure
```

预期：PASS；分别输出 `__text/__data/__bss/total` 当前值、M0 值、相对差、绝对差和判定，缺 baseline/schema/checker fallback 必须失败。

- [ ] **步骤 3：运行 steady-state 零分配**

运行：

```bash
rtk ctest --test-dir build/v2.3 -R '^test_tinyui_steady_state_allocation$' --output-on-failure
```

预期：PASS；`tinyui_process`、layout、event dispatch、theme apply、getter 的隐式 allocation count 全部为 `0`；显式对象/字符串分配不混入该计数。

- [ ] **步骤 4：运行时间统计 gate**

运行：

```bash
rtk ctest --test-dir build/v2.3 -R '^check_tinyui_perf$' --output-on-failure
```

预期：PASS；指纹完全匹配；每场景显示 warmup=5、runs=30、median、p95；`screen_object_create_ms` p95 `<=5 ms`，`capture_ready_ms` p95 `<=40 ms`。

- [ ] **步骤 5：运行 minimal profile**

运行：

```bash
rtk cmake -E remove_directory build/v2.3-minimal
rtk cmake -S . -B build/v2.3-minimal -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=OFF -DLD_BUILD_RUNTIME_TESTS=OFF -DLD_TINYUI_PORT=none -DTINYUI_PROFILE=minimal
rtk cmake --build build/v2.3-minimal -j
rtk ctest --test-dir build/v2.3-minimal -R '^check_tinyui_minimal_profile$' --output-on-failure
```

预期：PASS；runtime、screen/window、label、button 存在；其他控件、theme、diagnostics、native interop public implementation symbols 在 map/nm 中零命中；不得保留空注册表或通用分发表。

- [ ] **步骤 6：更新轻量机器证据和文档投影**

`lightweight-gates.json` 和 `v2.3-performance-baseline.md` 必须记录所有实测值、阈值、指纹、baseline commit、日志 SHA-256 和 PASS；禁止只写“未回归”。若机器指纹不匹配，停止并建立经审查的新机器 baseline，不得覆盖旧值继续比较。

### 任务 5：关闭能力矩阵并完成人工 review

**文件：**

- 修改：`tests/tinyui/contract/tinyui_release_capability_matrix.json`
- 修改：`tests/tinyui/contract/check_tinyui_release_capability_matrix.py`
- 修改：`docs/v2.3/v2.3-capability-evidence-matrix.md`
- 修改：`docs/v2.3/v2.3-release-gates.md`
- 修改：`docs/v2.3/deferred-port-work.md`
- 生成：`build/v2.3/release-evidence/manual-review.json`

**接口：**

- 消费：M3 L1-L5 证据、M4 demo/install/docs 证据、任务 2-4 fresh logs。
- 产出：零 pending 的 required matrix 和 `manual_reviewed_passed=true`。

- [ ] **步骤 1：让 matrix checker 拒绝虚假完成**

每个 required 条目必须包含 `capability_id`、public symbol/header、L2 consumer test、L3 contract test、L4 backend state test；可见项须有 L5-V test/artifact，可操作项须有 L5-E test/artifact。checker 必须打开证据路径并确认存在，不接受同一事件证据替代像素或同一截图替代事件。

`policy_never_public` 只允许生命周期、渲染管线、内存、宿主、调试和 backend-private helper；任何 widget 用户能力命中该 policy 直接失败。

- [ ] **步骤 2：逐项填满能力矩阵**

按 create/props/common set-get/special set-get/event/focus/theme-style/layout/image-font/destroy-error 审查每个控件。canvas、table、keyboard、graph、animation 等复杂控件必须绑定真实 L4/L5，不能只绑定 smoke/runtime 启动。

- [ ] **步骤 3：运行矩阵 gate**

运行：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_release_capability_matrix.py
rtk ctest --test-dir build/v2.3 -R '^check_tinyui_release_capability_matrix$' --output-on-failure
```

预期：PASS；required 无 missing/pending/policy 规避；所有证据文件可定位。

- [ ] **步骤 4：执行人工检查表**

人工逐项检查并在 `manual-review.json` 记录 reviewer、时间、git commit、每项 evidence 和布尔结果：

1. canonical API 无 legacy/native 泄漏。
2. 每个 required 能力的 L4/L5 可定位。
3. demo 未使用新增固定坐标/尺寸掩盖 layout/backend 缺口。
4. 无第二套 renderer、layout、style、event propagation 或 resource runtime。
5. 未用 `policy_never_public` 规避用户能力。
6. wrapper/pool/descriptor/binary/time/零分配/minimal 全部通过。
7. port 延期项未写成完成。

只有七项全 true 时才写 `manual_reviewed_passed=true`。

- [ ] **步骤 5：明确 port 非完成**

`deferred-port-work.md` 与 `v2.3-release-gates.md` 必须明确：RGB565/ARGB8888 framebuffer/stride/PFB、同步/异步 flush/DMA/fence、平台时钟、SDL event pump/resize/context、MCU toolchain/RTOS/ISR/cache、真机板级验证均未完成；`port_completed=false`、L6 不纳入 v2.3 core 完成判断。

### 任务 6：执行独立代码评审，并由同一 reviewer 跟踪修复

**文件：**

- 新增：`docs/v2.3/v2.3-release-review.md`
- review 发现涉及的文件；写面必须由 review 子代理独占。

**接口：**

- 消费：`master...HEAD` diff、设计、M0-M5 计划、全部 fresh gate 证据。
- 产出：按 P0/P1/P2 排序的独立评审记录，最终 P0/P1 为 0。

- [ ] **步骤 1：启动独立 review 子代理**

主线程必须派发未参与对应实现写面的独立 review subagent，要求先读设计与 AGENTS.md，再审查：真实 backend 映射、对象/池生命周期、descriptor 借用、错误传播、裁剪链接、安装边界、demo canonical、测试真实性和 port 声明。

- [ ] **步骤 2：按严重度记录 finding**

每条 finding 必须有 `P0/P1/P2`、精确文件/行、可复现命令、实际/预期、影响和修复要求。无 finding 时也必须记录残余风险和未覆盖的 port/L6，而不是只写“通过”。

- [ ] **步骤 3：不通过时由同一 review 子代理修复**

若存在 finding，同一 review subagent 保持写面并完成修复，不新开修复代理，主线程不直接代修。修改任何 C 函数前，reviewer 先把精确符号发给主线程；主线程运行 upstream `impact`，对 HIGH/CRITICAL 告警后再授权该 reviewer 继续。修复后 reviewer 重跑最小复现、focused CTest 和受影响的 broad gate。

- [ ] **步骤 4：复核到 P0/P1 清零**

运行：

```bash
rtk git diff --check
rtk ctest --test-dir build/v2.3 --output-on-failure
```

预期：PASS；review 文档中 P0/P1 状态全部 `closed`；P2 必须为 `closed` 或有不影响发布的明确 disposition。

### 任务 7：用 GitNexus 复核实际影响范围

**文件：**

- 修改：`docs/v2.3/v2.3-release-gates.md`
- 生成：`build/v2.3/release-evidence/gitnexus-impact.json`

**接口：**

- 消费：最终工作树相对 `master` 的改动。
- 产出：符号、执行流程和风险与计划一致的影响证明。

- [ ] **步骤 1：主线程运行最终 detect_changes**

调用 GitNexus MCP：

```text
detect_changes({scope: "compare", base_ref: "master"})
```

预期：报告所有 changed symbols、affected processes 和风险；不得使用 working tree 的局部猜测替代 compare master。

- [ ] **步骤 2：对 HIGH/CRITICAL 符号补 upstream impact**

对结果中的每个 HIGH/CRITICAL 公共符号调用：

```text
impact({target: "<精确符号名>", direction: "upstream"})
```

预期：调用者迁移清单与 M1-M4 的 demo、测试、文档和消费者集合一致；出现未迁移调用者时返回对应阶段修复并重跑全 gate。

- [ ] **步骤 3：记录执行流程差异**

`gitnexus-impact.json` 记录 base ref、git commit、changed symbols、affected processes、risk、disposition。禁止忽略 HIGH/CRITICAL；若出现设计外第二套 runtime/layout/resource 流程，M5 失败。

### 任务 8：冻结 canonical ABI 并运行最终聚合门禁

**文件：**

- 修改：`tests/tinyui/contract/check_tinyui_public_api.py`
- 新增：`tests/tinyui/contract/tinyui_v23_abi_manifest.json`
- 修改：`tests/tinyui/contract/tinyui_v23_release_gate_manifest.json`
- 修改：`docs/v2.3/v2.3-release-gates.md`

**接口：**

- 消费：最终安装 public headers、导出符号、C/C++ header/link tests、任务 1-7 证据。
- 产出：只冻结一次的 v2.3 ABI manifest 和最终 release gate PASS。

- [ ] **步骤 1：先实现 ABI manifest 校验模式**

`check_tinyui_public_api.py` 增加两个互斥参数：

```text
--emit-abi-manifest <path>
--check-abi-manifest <path>
```

manifest 必须规范化记录：安装 public header 相对路径；公开 typedef/opaque type；函数名、返回类型和参数类型；enum 名和值；公开宏常量值；导出 target/symbol；禁止头 token。排序和 JSON 格式确定，不记录时间戳或绝对路径。check 模式只比较并失败，绝不自动重写。

- [ ] **步骤 2：先确认未冻结 manifest 会失败**

运行：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_public_api.py --check-abi-manifest tests/tinyui/contract/tinyui_v23_abi_manifest.json
```

预期：FAIL，精确报告 manifest 不存在；不得 fallback 到旧版本清单。

- [ ] **步骤 3：在全部评审完成后生成唯一 ABI manifest**

运行：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_public_api.py --emit-abi-manifest tests/tinyui/contract/tinyui_v23_abi_manifest.json
rtk python3 tests/tinyui/contract/check_tinyui_public_api.py --check-abi-manifest tests/tinyui/contract/tinyui_v23_abi_manifest.json
rtk ctest --test-dir build/v2.3 -R 'check_tinyui_public_(headers_c|headers_cpp|symbols_link|api)' --output-on-failure
```

预期：manifest 生成一次；随后 check PASS；C headers、C++ `extern "C"`、每个函数独立链接和 API manifest 全部 PASS。

- [ ] **步骤 4：补齐 release manifest 的 fresh artifact**

将 standard、ASan、UBSan、lightweight、minimal、capability、manual review、independent review、GitNexus、ABI、install consumer 的路径和 SHA-256 写入 release manifest；`manual_reviewed_passed=true`、`abi_frozen=true`、`port_completed=false`。

- [ ] **步骤 5：运行最终聚合 gate**

运行：

```bash
rtk python3 tests/tinyui/contract/check_tinyui_v23_release_gates.py --build-dir build/v2.3 --manifest tests/tinyui/contract/tinyui_v23_release_gate_manifest.json --artifact-root build/v2.3/release-evidence
rtk ctest --test-dir build/v2.3 -R '^check_tinyui_v23_release_gates$' --output-on-failure
rtk ctest --test-dir build/v2.3 --output-on-failure
rtk git diff --check
```

预期：全部 PASS；全量 CTest 仍为 100%，聚合器无 missing/pending/fallback，格式检查通过。

- [ ] **步骤 6：执行最后一次 placeholder 和声明边界扫描**

运行：

```bash
rtk rg -n '[T]BD|[T]ODO|implement[[:space:]]+later|fill[[:space:]]+in[[:space:]]+details|待[[:space:]]*补|后续[[:space:]]*补|暂时[[:space:]]*通过' docs/v2.3 tests/tinyui/contract/tinyui_v23_release_gate_manifest.json tests/tinyui/contract/tinyui_v23_abi_manifest.json
rtk rg -n 'port.*(完成|生产可用)|L6.*(完成|通过)|LVGL.*(兼容|功能集)' docs/v2.3
```

预期：第一条零命中；第二条只允许在“不得声明/明确未完成”的否定语境中命中，人工逐条确认。

## M5 完成定义

- clean 标准构建、全部 CTest、Clang ASan、Clang UBSan、安装消费者均 fresh PASS。
- 全部 wrapper、backend widget、pool/bookkeeping、image/font/tile storage ABI 在硬上限内。
- binary 双阈值、5+30 时间统计、steady-state 零分配、minimal profile 均 PASS，且没有 fallback。
- required 能力全部达到 L4；可见/可操作能力分别达到 L5-V/L5-E，复杂控件不是 smoke 证据。
- `manual_reviewed_passed=true` 有逐项证据；独立 review 无未关闭 P0/P1 非 port 问题。
- GitNexus `detect_changes(compare master)` 与实际影响一致，HIGH/CRITICAL 调用者均已迁移。
- canonical ABI 只在上述条件满足后冻结一次，C/C++ header/link/installed consumer 同时通过。
- `port_completed=false`；SDL/MCU production、PFB、DMA、flush、时钟、RTOS/ISR/cache 和真机 L6 仍明确延期。
- 只有 M0-M5 均满足时，才可声明 v2.3 非 port core 完成；不得声明 LVGL 兼容、LVGL 功能集或 port 生产可用。
