# TinyUI v2.3 M0 事实基线实施计划

> **供代理执行者：** 必须使用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans`，按任务逐项实施本计划。所有步骤使用复选框（`- [ ]`）跟踪。

**目标：** 修复干净目录下的 TinyUI 全量构建和测试，建立可由源码重复生成的 LingDongGUI API inventory、递归公共头 C/C++ 编译与逐符号链接门禁，并冻结所有 wrapper、静态池预算、最小裁剪产物、二进制尺寸、稳态分配和宿主性能基线。

**架构：** M0 不改变 TinyUI 公共模型，只恢复事实可信度。所有基线都由可执行 probe 生成机器 JSON，再由 fail-closed checker 校验；Markdown 仅解释 JSON，不作为机器真相源。公共契约检查从 `tinyui/include/**/*.h` 递归生成独立消费者，LingDongGUI inventory 从 `src/gui/ld*.h` 生成。现有 Picoui 扫描入口迁移到 TinyUI contract 目录，扫描、分类和 JSON 写入只保留一份实现。

**技术栈：** C11、C++17 消费者探针、CMake 3.16、CTest、Python 3、LingDongGUI/Arm-2D、SDL 测试宿主、`size`/`llvm-size`、`nm`、GitNexus。

## 全局约束

- TinyUI 是 LingDongGUI 的薄上层 API，不得新增 renderer、对象树、布局器、样式引擎或资源中心。
- 本阶段不修改 SDL、MCU 或其他 port 的生产契约；SDL 仅用于取得真实宿主基线。
- 本阶段不创建 worktree；从当前工作区执行，并保留所有与本计划无关的已有改动。
- 修改任何函数前先运行 GitNexus `impact({target: "符号名", direction: "upstream"})`；`HIGH` 或 `CRITICAL` 必须先记录调用者迁移清单并告警。
- 每个 shell 命令都以 `rtk` 开头；构建系统统一使用 CMake。
- 所有 checker 在输入缺失、schema 缺项、工具不可用、输出无法解析或环境指纹不一致时返回非零，不得采用 fallback baseline。
- M0 只允许修复“当前声明、当前实现、当前构建”之间的事实冲突；legacy API、canonical ABI 和控件命名统一留给 M1。
- 每个任务完成后运行对应窄测试和 `rtk git diff --check`；M0 关闭前运行全量门禁和 GitNexus `detect_changes()`。实际提交不属于本实施计划。

---

## 文件结构

- `cmake/LingDongGUI.cmake`：增加 contract probe、最小消费者和性能 probe 目标；只有复现证明源码清单确实漂移时才修改现有清单。
- `tests/tinyui/CMakeLists.txt`：注册 M0 新增的 inventory、公共头、逐符号链接、尺寸、分配和性能 CTest。
- `tests/tinyui/contract/check_ldgui_public_api_inventory.py`：由现有 Picoui 入口迁移而来，负责从 `src/gui/ld*.h` 扫描、分类、生成和检查规范化 inventory；不修改 ledger。
- `tests/picoui/contract/check_ldgui_public_api_inventory.py`：迁移后删除，Picoui 若仍需消费 inventory，只读取 TinyUI 生成的同一 JSON，不保留第二份扫描实现。
- `tests/tinyui/contract/generate_tinyui_public_contract_probes.py`：递归扫描公共头并生成 C、C++ 和逐函数链接消费者。
- `tests/tinyui/contract/check_tinyui_public_contract_manifest.py`：验证探针 manifest 与当前公共头、公开函数严格一致。
- `tests/tinyui/contract/ldgui_public_api_inventory.json`：LingDongGUI 公共 API 机器真相快照。
- `tests/tinyui/contract/ldgui_public_api_expected_symbols.json`：inventory 期望符号集合。
- `tests/tinyui/unit/test_tinyui_wrapper_struct_overhead.c`：输出每一种 concrete TinyUI wrapper、backend wrapper 和 runtime 静态状态尺寸。
- `tests/tinyui/unit/test_tinyui_steady_state_allocation.c`：在稳定 UI 上统计 runtime 单轮处理的隐式分配。
- `tests/tinyui/minimal/minimal_consumer.c`：只创建 runtime、screen/window、label、button 的最小消费者。
- `tests/tinyui/perf/collect_tinyui_v23_baseline.py`：统一采集结构、静态 RAM、二进制、裁剪、分配、性能和环境指纹。
- `tests/tinyui/perf/check_tinyui_v23_baseline.py`：按 schema 和硬阈值校验机器基线。
- `tests/tinyui/perf/v2.3-baseline.schema.json`：完整、禁止额外字段的基线 schema。
- `tests/tinyui/perf/v2.3-baselines/<fingerprint>.json`：当前机器的 M0 冻结值；文件名是指纹 SHA-256 前 16 位。
- `docs/v2.3/v2.3-performance-baseline.md`：解释测量口径、命令、指纹和冻结结果。

## 任务 1：锁定并修复默认全量构建的真实首个失败

**文件：**

- 修改：`tests/tinyui/unit/test_tinyui_button_events.c`
- 必要时修改：提供 `ldGuiClickedAction()` 声明的既有 LingDongGUI internal/test-support 头
- 检查：`cmake/LingDongGUI.cmake`（`tinyui/src/widgets/canvas.c` 已在 target 中，不得重复添加）

**接口：**

- 消费：顶层选项 `ENABLE_TEST`、`LD_BUILD_SDL_DEMO`、`LD_BUILD_RUNTIME_TESTS`、`LD_BUILD_VISUAL_TESTS`、`LD_TINYUI_PORT`。
- 产出：干净构建目录 `build/v2.3-m0-full` 中的 `tinyui_core`、`tinyui_demo`、全部 TinyUI unit/contract/runtime target；测试不再依赖隐式函数声明。

- [x] **步骤 1：在干净目录复现并保存唯一首个失败**

运行：

```bash
rtk cmake -S . -B build/v2.3-m0-full -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_VISUAL_TESTS=OFF -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m0-full -j
```

预期：配置成功；当前首个失败固定为 `tests/tinyui/unit/test_tinyui_button_events.c:432` 调用未声明的 `ldGuiClickedAction()`。实现已存在于 `src/gui/ldGui.c`，本步骤必须记录编译器诊断，不能误写成 `canvas.c` 漏入 target。

- [x] **步骤 2：锁定 internal action 的声明可见性边界**

在 `test_tinyui_button_events.c` 增加编译期签名检查；声明必须来自既有 LingDongGUI internal/test-support 边界，或使用与其他同类单测一致的测试局部前置声明。不得把 `ldGuiClickedAction()` 暴露为 TinyUI public API：

```c
void ldGuiClickedAction(ld_scene_t *scene,
                        uint8_t touch_signal,
                        arm_2d_location_t location);
```

运行：

```bash
rtk cmake --build build/v2.3-m0-full --target test_tinyui_button_events -j
```

预期：修改前因隐式声明失败；补齐可见声明后编译和链接通过，且 `tinyui.h`、TinyUI public headers 与安装边界均不出现该 internal action。

- [x] **步骤 3：对真实失败符号执行影响分析并做最小修复**

修改前调用：

```text
impact({target: "ldGuiClickedAction", direction: "upstream"})
```

若 fresh 结果为 `HIGH/CRITICAL`，先告警并记录全部测试调用者。最小修复仅解决测试编译边界；不得改 action 行为、SDL host、port 或渲染路径。

- [x] **步骤 4：重新配置并逐个修复剩余事实性构建错误**

重新从干净目录构建。每遇到后续公开函数声明但无定义时，先用以下只读检查确认其用户能力和调用者：

```bash
rtk rg -n "函数名" tinyui/include tinyui/src tinyui/demo tests/tinyui
rtk nm -u build/v2.3-m0-full/tests/tinyui/失败目标
```

然后调用 GitNexus `impact({target: "函数名", direction: "upstream"})`。若函数有真实用户能力且已有实现文件，把实现文件纳入 target；若它是无消费者、无实现、无能力映射的孤儿声明，只删除该声明及专门验证该孤儿的旧测试。不得用返回成功的空函数补链接。

运行：

```bash
rtk cmake --build build/v2.3-m0-full -j
rtk ctest --test-dir build/v2.3-m0-full --output-on-failure -L '^unit$'
```

预期：全量构建完成；TinyUI unit 测试零失败。任何后续首失败都按“复现一个、修复一个、重跑一个”的顺序处理，不预设为源码清单问题。

- [x] **步骤 5：运行格式与范围检查**

```bash
rtk git diff --check
rtk git diff -- tests/tinyui/unit/test_tinyui_button_events.c
```

## 任务 2：建立 LingDongGUI public header inventory 单一真相源

**文件：**

- 移动并重构：`tests/picoui/contract/check_ldgui_public_api_inventory.py` → `tests/tinyui/contract/check_ldgui_public_api_inventory.py`
- 修改：`tests/tinyui/contract/ldgui_public_api_inventory.json`
- 修改：`tests/tinyui/contract/ldgui_public_api_expected_symbols.json`
- 修改：Picoui 对 inventory 的 CMake/脚本引用，统一读取 TinyUI contract 产物
- 修改：`tests/tinyui/CMakeLists.txt`
- 测试：`tests/tinyui/contract/test_ldgui_public_api_inventory.py`

**接口：**

- 消费：`src/gui/ld*.h` 中去注释后的外部 `ld[A-Z]*` 函数声明和函数式宏。
- 产出：唯一实现中的 `scan_headers(root: Path) -> list[dict]`、`build_inventory(rows: list[dict]) -> dict`、CLI `--check` 与 `--write`。

- [x] **步骤 1：写 generator 的解析单元测试**

测试 fixture 必须包含多行函数、`extern "C"`、函数式宏、注释内伪声明和重复声明，并断言只得到两个稳定排序项：

```python
def test_scan_text_ignores_comments_and_normalizes_declarations():
    text = """\
    /* int ldFake(void); */
    extern "C" { int ldLabelSetText(ldLabel_t *label, const char *text); }
    #define ldLabelGetText(obj) _ldLabelGetText(obj)
    """
    assert scan_header_text("src/gui/ldLabel.h", text) == [
        {"header": "src/gui/ldLabel.h", "kind": "macro", "symbol": "ldLabelGetText", "target": "_ldLabelGetText"},
        {"header": "src/gui/ldLabel.h", "kind": "function", "symbol": "ldLabelSetText", "signature": "int ldLabelSetText(ldLabel_t *label, const char *text);"},
    ]
```

运行：

```bash
rtk python3 tests/tinyui/contract/test_ldgui_public_api_inventory.py -v
```

预期：重构前因 TinyUI contract 目录没有可导入扫描模块或 CLI 参数不完整而失败。

- [x] **步骤 2：实现纯扫描器和稳定 JSON 输出**

把现有 Picoui checker 的扫描、分类和 JSON 逻辑迁移到 TinyUI contract 目录，不复制函数。CLI 固定为：

```text
check_ldgui_public_api_inventory.py --root <repo> --output <inventory.json> --expected-output <expected.json> --check
check_ldgui_public_api_inventory.py --root <repo> --output <inventory.json> --expected-output <expected.json> --write
```

`--check` 对内存生成结果与磁盘 JSON 做结构化比较并输出 `missing`、`extra`、`changed_signature`；`--write` 只写 inventory 和 expected symbols，不写 `native_api_gap_ledger.json` 或 release matrix。schema 固定为 `tinyui-v2.3-ldgui-public-api-inventory-v1`，每行必须有 `header`、`symbol`、`kind`、`signature` 或 `target`。

删除旧 Picoui 扫描入口；若 Picoui contract 仍需 inventory，只能 import 新模块或读取生成 JSON。仓库中搜索 `_strip_comments`、`scan_headers` 和 `HEADER_GLOB` 后，inventory 扫描实现必须各只有一份。

运行：

```bash
rtk python3 tests/tinyui/contract/test_ldgui_public_api_inventory.py -v
```

预期：全部通过。

- [x] **步骤 3：先证明当前 inventory 漂移门禁为红**

运行：

```bash
rtk python3 tests/tinyui/contract/check_ldgui_public_api_inventory.py --root . --output tests/tinyui/contract/ldgui_public_api_inventory.json --expected-output tests/tinyui/contract/ldgui_public_api_expected_symbols.json --check
```

预期：非零退出，并至少逐项报告四个当前已知 extra symbol：`ldBaseGetScreenSize`、`ldBaseGetScreenSizeForScene`、`ldGuiDisposeNodeTree`、`ldTextSetScrollEnabled`；缺文件、额外漂移或 schema 不符也必须非零。

- [x] **步骤 4：由生成器刷新两个真相文件并立即复查**

```bash
rtk python3 tests/tinyui/contract/check_ldgui_public_api_inventory.py --root . --output tests/tinyui/contract/ldgui_public_api_inventory.json --expected-output tests/tinyui/contract/ldgui_public_api_expected_symbols.json --write
rtk python3 tests/tinyui/contract/check_ldgui_public_api_inventory.py --root . --output tests/tinyui/contract/ldgui_public_api_inventory.json --expected-output tests/tinyui/contract/ldgui_public_api_expected_symbols.json --check
```

预期：第二条命令输出 `LDGUI_PUBLIC_API_INVENTORY_OK`；ledger 文件的哈希在执行前后不变。

- [x] **步骤 5：注册唯一 TinyUI inventory CTest**

在 `tests/tinyui/CMakeLists.txt` 注册 `check_ldgui_public_api_inventory`，命令只使用 `--check`，label 为 `tinyui;contract;inventory`。删除或停用从未注册到 TinyUI 阶段、且会维护第二份生成逻辑的重复入口。

```bash
rtk cmake -S . -B build/v2.3-m0-contract -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=OFF -DLD_BUILD_RUNTIME_TESTS=OFF -DLD_TINYUI_PORT=none
rtk ctest --test-dir build/v2.3-m0-contract --output-on-failure -R '^check_ldgui_public_api_inventory$'
```

预期：一个且仅一个同名 CTest 被执行并通过。

- [x] **步骤 6：运行单一实现与格式检查**

```bash
rtk rg -n 'def (_strip_comments|scan_headers)|HEADER_GLOB' tests/tinyui/contract tests/picoui/contract
rtk git diff --check
```

预期：扫描实现只位于 `tests/tinyui/contract/check_ldgui_public_api_inventory.py`，Picoui 没有第二套 parser。

## 任务 3：递归建立每个公共头的独立 C/C++ 编译门禁

**文件：**

- 创建：`tests/tinyui/contract/generate_tinyui_public_contract_probes.py`
- 创建：`tests/tinyui/contract/check_tinyui_public_contract_manifest.py`
- 修改：`tests/tinyui/CMakeLists.txt`
- 测试：`tests/tinyui/contract/test_generate_tinyui_public_contract_probes.py`

**接口：**

- 消费：递归 glob `tinyui/include/**/*.h`，排除 `tinyui/include/internal/**`。
- 产出：`build/.../generated/tinyui-public-contract/probes.cmake`、一个 C probe、一个 C++ probe 和每个 public 函数一个链接 probe；manifest 字段为 `headers`、`functions`、`generated_files`。

- [x] **步骤 1：写递归和 C++ 包装失败测试**

测试临时目录包含 `tinyui.h`、`core/runtime.h`、`widgets/label.h` 和 `internal/private.h`，断言输出只包含前三个头；C probe 必须直接 `#include` 目标头，C++ probe 必须为：

```cpp
extern "C" {
#include "widgets/label.h"
}
int main() { return 0; }
```

运行：

```bash
rtk python3 tests/tinyui/contract/test_generate_tinyui_public_contract_probes.py -v
```

预期：失败，提示 generator 不存在。

- [x] **步骤 2：实现 probe 生成器并拒绝 basename 冲突**

生成文件名使用相对路径 SHA-256 前 12 位，不能只用 basename；每个头产生独立 C 与 C++ executable，编译选项固定为 `-Wall -Wextra -Werror`，C++ 标准固定为 17。manifest 中保存相对路径和哈希，checker 重新扫描后要求集合完全相等。

- [x] **步骤 3：把生成结果接入配置期 CMake target**

`tests/tinyui/CMakeLists.txt` 在 configure 时执行 generator 并 `include(probes.cmake)`，同时仅为 contract probe 调用 `enable_language(CXX)`；每个 header target 只获得 `${CMAKE_SOURCE_DIR}/tinyui/include`，不得获得 `tinyui/src/core`、`tinyui/src/drivers` 或 `${LD_COMMON_INCLUDE_DIRS}`。分别建立聚合 CTest `check_tinyui_public_headers_c` 和 `check_tinyui_public_headers_cpp`，统一 label 为 `tinyui;contract;public-header`，供 M5 release manifest 直接引用。

运行：

```bash
rtk cmake -S . -B build/v2.3-m0-headers -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=OFF -DLD_BUILD_RUNTIME_TESTS=OFF -DLD_TINYUI_PORT=none
rtk cmake --build build/v2.3-m0-headers --target tinyui_public_header_probes -j
```

预期：当前自包含性或 native include 泄漏导致至少一个 probe 失败，并明确打印相对头路径。

- [x] **步骤 4：只修复头自包含问题**

对失败头补充它直接使用的标准头或 canonical public 头；不得给 probe 增加 private include 路径。每次修改公共头前，对其中受影响函数逐一调用 GitNexus upstream impact；本任务不改函数签名。

```bash
rtk cmake --build build/v2.3-m0-headers --target tinyui_public_header_probes -j
rtk ctest --test-dir build/v2.3-m0-headers --output-on-failure -L '^public-header$'
```

预期：所有递归 public header 都能独立通过 C11 和 C++17 `extern "C"` 编译；internal 头不在 manifest。

- [x] **步骤 5：运行聚合测试与格式检查**

```bash
rtk ctest --test-dir build/v2.3-m0-headers -R '^check_tinyui_public_headers_(c|cpp)$' --output-on-failure
rtk git diff --check
```

## 任务 4：建立每个公开函数的独立链接门禁并清理孤儿声明

**文件：**

- 修改：`tests/tinyui/contract/generate_tinyui_public_contract_probes.py`
- 修改：`tests/tinyui/contract/check_tinyui_public_contract_manifest.py`
- 修改：`tests/tinyui/CMakeLists.txt`
- 修改：`tinyui/include/**/*.h`（只删除已证明无实现且无用户能力的孤儿声明）
- 测试：`tests/tinyui/contract/test_generate_tinyui_public_contract_probes.py`

**接口：**

- 消费：public header 中所有外部 `tinyui_*` 函数声明。
- 产出：每个函数一个取地址消费者 `static void *volatile symbol_ref = (void *)&symbol;`，分别链接 `tinyui_backend_ldgui`；聚合 CTest 固定名为 `check_tinyui_public_symbols_link`，label 为 `tinyui;contract;public-link`。

- [x] **步骤 1：写函数声明解析与 overload 拒绝测试**

fixture 覆盖函数指针 typedef、多行声明、`static inline`、宏和普通函数；只为普通外部函数生成 probe。若同名函数出现不同签名，generator 必须非零退出并打印两处文件与签名。

- [x] **步骤 2：生成逐符号链接消费者**

每个消费者包含声明所在头并取该函数地址，不调用函数，从而不伪造参数；每个 executable 只引用一个 public function。`probes.cmake` 必须创建聚合 target `tinyui_public_symbol_link_probes`。

运行：

```bash
rtk cmake -S . -B build/v2.3-m0-links -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=OFF -DLD_BUILD_RUNTIME_TESTS=OFF -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m0-links --target tinyui_public_symbol_link_probes -j
```

预期：当前只有声明没有定义的函数逐项失败；输出必须能定位 public header 和符号名。

- [x] **步骤 3：逐项处理孤儿声明**

对每个失败符号执行：

```bash
rtk rg -n "失败符号" tinyui/include tinyui/src tinyui/demo tests/tinyui
rtk nm -g build/v2.3-m0-links/libtinyui_core.a
```

调用 GitNexus upstream impact 后分类：有用户能力则把现有真实实现接入构建；只有声明、没有实现、没有调用者且 capability ledger 不要求的项才可删除声明；禁止添加空实现，禁止把 required 用户能力放入 `policy_never_public`。

当前已知 orphan 声明 `tinyui_window_get_layout_type()` 和 `tinyui_window_get_gap()` 都是用户可见 layout getter，不允许按孤儿删除。M0 必须让它们读取真实 LingDongGUI layout 状态；若 backend 没有 getter，则读取现有 window 创建/成功 setter 已维护的最小单项状态，并用 setter 后 getter 与真实 layout 配置联合测试证明。二者在本阶段必须通过逐符号链接和行为单测，不得返回常量、假成功或转入 policy。

- [x] **步骤 4：验证每个函数独立链接**

```bash
rtk cmake --build build/v2.3-m0-links --target tinyui_public_symbol_link_probes -j
rtk ctest --test-dir build/v2.3-m0-links --output-on-failure -L '^public-link$'
rtk ctest --test-dir build/v2.3-m0-links -R '^check_tinyui_public_symbols_link$' --output-on-failure
```

预期：manifest 中 `functions` 数量等于 link probe 数量，每个 probe 通过。

- [x] **步骤 5：运行格式检查**

```bash
rtk git diff --check
```

## 任务 5：冻结全部 wrapper 和 runtime 静态 RAM 尺寸

**文件：**

- 修改：`tests/tinyui/unit/test_tinyui_wrapper_struct_overhead.c`
- 修改：`tests/tinyui/perf/check_tinyui_object_overhead.py`
- 创建：`tests/tinyui/perf/v2.3-baseline.schema.json`
- 修改：`tests/tinyui/perf/tinyui_perf_baseline.json`
- 修改：`tests/tinyui/CMakeLists.txt`
- 测试：`tests/tinyui/perf/test_check_tinyui_object_overhead.py`

**接口：**

- 消费：`tinyui/src/core/internal.h` 中所有 concrete wrapper 以及当前 timer/event/runtime bookkeeping 类型。
- 产出：标记 `TINYUI_SIZEOF_<UPPER_SNAKE_NAME>=<bytes>`；checker API `parse_probe(stdout) -> dict[str, int]`。

- [x] **步骤 1：写“缺任一 wrapper 即失败”的 checker 测试**

固定 required wrapper 集合为：`widget`、`window`、`background`、`canvas`、`label`、`button`、`keyboard`、`checkbox`、`switch`、`slider`、`progress_bar`、`animation`、`arc`、`gauge`、`icon_slider`、`radial_menu`、`qrcode`、`progress_wheel`、`date_time`、`calendar`、`clock`、`list`、`message_box`、`text`、`line_edit`、`combo_box`、`scroll_selecter`、`image`、`graph`、`table`。fixture 删除 `table` 时必须报 `missing wrapper metric: table_wrapper_struct_bytes`。

- [x] **步骤 2：扩展 C probe 输出全部 concrete wrapper**

用单一宏避免名称和值分离：

```c
#define PRINT_SIZE(name, type) \
    printf("TINYUI_SIZEOF_%s=%zu\n", name, sizeof(type))

PRINT_SIZE("WIDGET_WRAPPER", struct tinyui_widget);
PRINT_SIZE("WINDOW_WRAPPER", struct tinyui_window);
PRINT_SIZE("BACKGROUND_WRAPPER", struct tinyui_background);
PRINT_SIZE("CANVAS_WRAPPER", struct tinyui_canvas);
PRINT_SIZE("TABLE_WRAPPER", struct tinyui_table);
```

实现时为上一步列出的每个 wrapper 各写一行，不允许只用最大值代替逐项值。另输出 `backend_widget_struct_bytes`、`legacy_timer_node_bytes`、`legacy_event_callback_storage_bytes` 和 `runtime_bookkeeping_bytes`；没有固定池时对应 legacy 字段必须真实记录当前值，不能写零冒充已实现。当前 backend 指标测量真实 concrete `ldWindow_t`，不再把已删除的旧 backend mirror 或 `ldBase_t` 公共前缀当作 widget。

- [x] **步骤 3：定义完整尺寸 schema 和阈值计算**

每个 wrapper 规则包含：

```json
{
  "baseline_bytes": 184,
  "max_increase_percent": 5.0,
  "max_increase_bytes": 8,
  "absolute_max_bytes": 192
}
```

`widget_wrapper_struct_bytes` 固定 `absolute_max_bytes=192`；`switch_wrapper_struct_delta_bytes` 固定 `absolute_max_bytes=32`；`backend_widget_struct_bytes` 固定设计参考 `baseline_bytes=496`、`absolute_max_bytes=512`，probe 必须报告当前真实 concrete backend 尺寸。其他 wrapper 的 `absolute_max_bytes` 等于 M0 实测值加 `8`；失败条件为超过 absolute 上限，或相对增长大于 `5%` 且绝对增长大于 `8 B`。

32 位 canonical `timer_pool_bytes + event_callback_pool_bytes + runtime_bookkeeping_bytes` 的硬上限字段固定为 `1024`。M0 尚未实现 canonical 固定池时，JSON 必须以 `implementation="legacy"`、真实 legacy node/storage 数值记录，不得把 M0 结果宣称为 canonical 池已通过；M1/M2 将把该字段切换为 `fixed_pool`。若当前基础 wrapper 实测不是设计冻结的 `184 B`，不得把漂移值改写成新 baseline；门禁保持失败，先对 `struct tinyui_widget` 做 upstream impact，并删除已能从 LingDongGUI 查询的重复缓存，直到基础 wrapper 不超过 `192 B`。

- [x] **步骤 4：运行 probe 并冻结当前值**

```bash
rtk cmake --build build/v2.3-m0-full --target test_tinyui_wrapper_struct_overhead -j
rtk ./build/v2.3-m0-full/tests/tinyui/test_tinyui_wrapper_struct_overhead
rtk python3 tests/tinyui/perf/check_tinyui_object_overhead.py --probe build/v2.3-m0-full/tests/tinyui/test_tinyui_wrapper_struct_overhead --baseline tests/tinyui/perf/tinyui_perf_baseline.json
```

预期：输出全部 required metric；缺项、重复项、负数或 schema 额外字段均失败。

- [x] **步骤 5：运行格式检查**

```bash
rtk git diff --check
```

## 任务 6：冻结稳态 runtime 零隐式分配基线

**文件：**

- 修改：`tests/support/tinyui_test_support.h`
- 修改：`tests/support/tinyui_test_support.c`
- 创建：`tests/tinyui/unit/test_tinyui_steady_state_allocation.c`
- 修改：`tests/tinyui/CMakeLists.txt`
- 修改：`tests/tinyui/perf/tinyui_perf_baseline.json`

**接口：**

- 产出：`tinyui_test_allocator_reset()`、`tinyui_test_allocator_snapshot()`，快照字段为 `alloc_calls`、`calloc_calls`、`realloc_calls`、`free_calls`、`bytes_requested`。

- [x] **步骤 1：先写稳定帧分配失败测试**

测试先完成 init、screen、label、button 和首次渲染，再清零计数，连续执行 100 次当前 canonical handler；断言每轮增量和总增量均为零：

```c
tinyui_test_allocator_reset();
for (unsigned int i = 0; i < 100; ++i) {
    assert(tinyui_timer_handler() >= 0);
    struct tinyui_test_allocator_snapshot now = tinyui_test_allocator_snapshot();
    assert(now.alloc_calls == 0);
    assert(now.calloc_calls == 0);
    assert(now.realloc_calls == 0);
}
```

运行：

```bash
rtk cmake --build build/v2.3-m0-full --target test_tinyui_steady_state_allocation -j
rtk ctest --test-dir build/v2.3-m0-full --output-on-failure -R '^test_tinyui_steady_state_allocation$'
```

预期：测试尚未注册或分配计数非零，门禁为红。

- [x] **步骤 2：在 test support 截获项目统一 allocator**

只在测试 target 通过链接替换或项目 allocator hook 计数；生产库不保存统计字段。计数 hook 必须调用真实 allocator，并防止计数器自身分配。若稳定循环发生分配，定位并把一次性初始化移到 reset 前；不得在测试里忽略某类 heap API。

- [x] **步骤 3：注册并通过零分配 CTest**

```bash
rtk cmake --build build/v2.3-m0-full --target test_tinyui_steady_state_allocation -j
rtk ctest --test-dir build/v2.3-m0-full --output-on-failure -R '^test_tinyui_steady_state_allocation$'
```

预期：输出 `TINYUI_STEADY_STATE_ALLOCATIONS=0`。M1 把调用点从 `tinyui_timer_handler()` 原样迁移为 `tinyui_process(&next_ms)`。

- [x] **步骤 4：运行格式检查**

```bash
rtk git diff --check
```

## 任务 7：建立最小消费者、裁剪符号和二进制尺寸基线

**文件：**

- 创建：`tests/tinyui/minimal/minimal_consumer.c`
- 创建：`tests/tinyui/perf/check_tinyui_minimal_symbols.py`
- 修改：`tests/tinyui/perf/check_tinyui_binary_size.py`
- 修改：`tests/tinyui/perf/tinyui_perf_baseline.json`
- 修改：`tests/tinyui/CMakeLists.txt`
- 修改：`cmake/LingDongGUI.cmake`
- 测试：`tests/tinyui/perf/test_check_tinyui_minimal_symbols.py`

**接口：**

- 消费：当前 runtime、screen/window、label、button public API。
- 产出：`tinyui_minimal_consumer`、link map `tinyui_minimal_consumer.map`、`collect_defined_tinyui_symbols(binary) -> set[str]`。

- [x] **步骤 1：写最小消费者**

程序只能 include `tinyui.h`，初始化 runtime，创建 screen、label、button，设置两段文本，加载 screen，处理一轮并 deinit；禁止引用其他控件、theme、diagnostics 或 native API。M0 使用当时仍存在的等价公开调用，M1 必须迁移成最终 canonical 调用。

- [x] **步骤 2：写 nm/map checker 的失败测试**

fixture 符号集包含 `tinyui_slider_create`、`tinyui_theme_set`、`tinyui_native_font_wrap` 时，checker 必须分别报告 `disabled_widget`、`theme`、`native_interop` 三类泄漏。M0 的 checker 以 `mode=baseline` 记录现状；`mode=enforce` 留给 M1，禁止在 M0 伪造“已裁剪”。

- [x] **步骤 3：增加带 section GC 和 link map 的最小 target**

只对该 target 增加函数/数据 section 与链接器 GC；GNU/Clang 使用 `-ffunction-sections -fdata-sections` 和 `-Wl,--gc-sections,-Map,<path>`，Apple Clang 使用 `-Wl,-dead_strip,-map,<path>`。CMake 必须对不支持的工具链明确失败或选择已测试分支，不静默省略 map。

运行：

```bash
rtk cmake --build build/v2.3-m0-full --target tinyui_minimal_consumer -j
rtk python3 tests/tinyui/perf/check_tinyui_minimal_symbols.py --mode baseline --binary build/v2.3-m0-full/tests/tinyui/tinyui_minimal_consumer --map build/v2.3-m0-full/tests/tinyui/tinyui_minimal_consumer.map --output build/v2.3-m0-full/tinyui-minimal-symbols.json
```

预期：输出实际进入产物的 TinyUI 定义符号集合和 SHA-256，不宣称未启用模块已被裁剪。

- [x] **步骤 4：修正二进制双阈值和 fail-closed 语义**

统一 runner 的阈值必须为：`__text 5%/8192 B`、`__data 25%/256 B`、`__bss 10%/8192 B`、`total 5%/12288 B`；只有百分比和绝对增长同时超限才失败。删除脚本中的 `UNIFIED_RUNNER_BASELINE` 内置 fallback，baseline artifact 不匹配时直接失败。

- [x] **步骤 5：记录 full 与 minimal 两个 artifact**

```bash
rtk python3 tests/tinyui/perf/check_tinyui_binary_size.py --binary build/v2.3-m0-full/examples/sdl/tinyui_demo --baseline tests/tinyui/perf/tinyui_perf_baseline.json
rtk python3 tests/tinyui/perf/check_tinyui_binary_size.py --binary build/v2.3-m0-full/tests/tinyui/tinyui_minimal_consumer --baseline tests/tinyui/perf/tinyui_perf_baseline.json
```

预期：两个 artifact 均有独立记录，不共享猜测值；工具输出无法解析时非零。

- [x] **步骤 6：运行格式检查**

```bash
rtk git diff --check
```

## 任务 8：建立可重复的 5 次预热、30 次采样性能基线

**文件：**

- 创建：`tests/tinyui/perf/collect_tinyui_v23_baseline.py`
- 创建：`tests/tinyui/perf/check_tinyui_v23_baseline.py`
- 创建：`tests/tinyui/perf/test_tinyui_v23_baseline.py`
- 创建：`tests/tinyui/perf/v2.3-baselines/<fingerprint>.json`
- 创建：`docs/v2.3/v2.3-performance-baseline.md`
- 修改：`tests/tinyui/perf/check_tinyui_perf.py`
- 修改：`tests/tinyui/CMakeLists.txt`

**接口：**

- 产出：`collect_environment() -> dict`、`percentile_nearest_rank(samples, 0.95) -> float`、CLI `collect --build-dir --output` 与 `check --baseline --build-dir`。

- [x] **步骤 1：写统计和指纹单元测试**

30 个升序样本的 p95 使用 nearest-rank 第 29 个值；指纹必须覆盖 OS/版本、CPU/架构、compiler 名称/版本/target triple、CMake build type、优化 flags、完整 `TINYUI_*` 与 `LD_*` 选项、git commit。任一字段变化必须产生不同 SHA-256。

- [x] **步骤 2：实现统一 collector**

每个 `screen_object_create_ms` 与 `capture_ready_ms` 场景先独立运行 5 次丢弃结果，再独立运行 30 次；保存原始 30 个样本、median、p95。collector 同时嵌入任务 5、6、7 的 probe 输出和 artifact SHA-256，任一子项失败则不写结果文件。

- [x] **步骤 3：实现严格 checker**

checker 先比较完整环境指纹；不一致输出 `BASELINE_FINGERPRINT_MISMATCH` 并非零退出。相同时要求 `screen_object_create_ms.p95 <= 5.0`、`capture_ready_ms.p95 <= 40.0`，并验证样本数严格为 30、warmup 严格为 5。

- [x] **步骤 4：在当前机器采集并复查**

```bash
rtk python3 tests/tinyui/perf/collect_tinyui_v23_baseline.py collect --build-dir build/v2.3-m0-full --output-dir tests/tinyui/perf/v2.3-baselines
rtk python3 tests/tinyui/perf/check_tinyui_v23_baseline.py check --build-dir build/v2.3-m0-full --baseline-dir tests/tinyui/perf/v2.3-baselines
```

预期：collector 输出指纹文件路径；checker 输出两个 metric 的 median/p95、尺寸、分配和裁剪摘要。若当前 `capture_ready_ms.p95` 超过 40 ms，M0 不得关闭，先定位真实回归或在完全相同环境重新测量，不能提高阈值。

- [x] **步骤 5：写人类可读基线说明**

`docs/v2.3/v2.3-performance-baseline.md` 必须列出机器 JSON 相对路径、git commit、完整配置命令、fingerprint、full/minimal artifact、全部 wrapper、legacy/fixed pool 状态、零分配结论、两项 median/p95，并明确 SDL 时间只属于测试宿主，不代表 port L6。

- [x] **步骤 6：注册性能 CTest 并运行格式检查**

```bash
rtk cmake -S . -B build/v2.3-m0-full -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_VISUAL_TESTS=OFF -DLD_TINYUI_PORT=sdl
rtk ctest --test-dir build/v2.3-m0-full --output-on-failure -L '^perf$'
rtk git diff --check
```

## 任务 9：执行 M0 closeout 门禁

**文件：**

- 修改：`docs/v2.3/v2.3-performance-baseline.md`（仅补 fresh run 证据）
- 检查：本计划所有文件

**接口：**

- 消费：任务 1-8 的全部 target、CTest 和机器基线。
- 产出：M0 关闭证据；不产出“TinyUI 已完整适配”的发布声明。

- [x] **步骤 1：从干净构建目录执行标准配置与全量构建**

```bash
rtk cmake -S . -B build/v2.3-m0-closeout -DENABLE_TEST=ON -DLD_BUILD_SDL_DEMO=ON -DLD_BUILD_RUNTIME_TESTS=ON -DLD_BUILD_VISUAL_TESTS=OFF -DLD_TINYUI_PORT=sdl
rtk cmake --build build/v2.3-m0-closeout -j
rtk ctest --test-dir build/v2.3-m0-closeout --output-on-failure
```

预期：配置、全量构建和全部注册 CTest 零失败。

实际证据：`build/v2.3-m0-closeout` 标准配置完成，低并发构建完成，CTest `89/89` 通过。

- [x] **步骤 2：重复检查 inventory、公共头、逐符号链接与 baseline**

```bash
rtk ctest --test-dir build/v2.3-m0-closeout --output-on-failure -L '^contract$'
rtk ctest --test-dir build/v2.3-m0-closeout --output-on-failure -L '^perf$'
rtk python3 tests/tinyui/perf/check_tinyui_v23_baseline.py check --build-dir build/v2.3-m0-closeout --baseline-dir tests/tinyui/perf/v2.3-baselines
```

预期：inventory 无漂移；每个 public header 的 C/C++ probe 和每个公开函数链接 probe 通过；所有 baseline schema 完整且可重复。

实际证据：contract、perf 标签门禁通过；full 与 closeout 两个构建目录的 v2.3 baseline checker 均输出 `TINYUI_V23_BASELINE_OK`，性能 p95 为 `screen_object_create_ms=2 ms`、`capture_ready_ms=15 ms`。

- [x] **步骤 3：运行 GitNexus 影响收敛**

调用 GitNexus `detect_changes()`，确认受影响符号只属于构建接线、contract probe、孤儿声明和测试基线；若出现 runtime/layout/widget 行为流变化，回到相应任务补充 impact 和行为测试。

实际证据：已执行 unstaged 变更检测；新增的 runtime、layout、widget 行为修复均补充了 upstream impact、TDD 回归和窄门禁，不能归类为未审计的构建接线。

- [x] **步骤 4：运行格式检查**

```bash
rtk git diff --check
```

实际证据：`rtk git diff --check` 通过。

M0 完成时只允许声明“事实基线可重复、构建与 contract 门禁恢复”；不得声明 canonical API、真实 backend 全能力或 port 已完成。
