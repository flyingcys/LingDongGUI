# TinyUI v2.3 文档入口

## 版本目标

TinyUI v2.3 是一次破坏式、轻量化的 canonical API 收口：参考 LVGL 的 API 使用模式，继续以 LingDongGUI/Arm-2D 为唯一真实实现，面向 LVGL 无法承载的小型 MCU。

本版本不实现第二套对象树、renderer、layout solver、style selector、事件传播系统或资源中心。

## 阅读顺序

1. `docs/v2.3/2026-07-13-tinyui-v2-3-unified-core-design.md`
2. `docs/v2.3/线计划索引.md`
3. `docs/v2.3/plans/v2.3-orchestration-plan.md`
4. `docs/v2.3/plans/stages/README.md`
5. 按 `M0 -> M1 -> M2 -> M3 -> M4 -> M5` 阅读阶段计划
6. `docs/v2.3/v2.3-capability-evidence-matrix.md`
7. `docs/v2.3/v2.3-migration-guide.md`
8. `docs/v2.3/v2.3-release-gates.md`
9. `docs/v2.3/deferred-port-work.md`

## 固定阶段

| 阶段 | 目标 | 状态 |
| --- | --- | --- |
| M0 | 恢复事实可信度并冻结轻量基线 | 已完成 |
| M1 | 冻结轻量 canonical 公共契约 | 已完成 |
| M2 | 核心纵向闭环与四个样板控件 | 已完成 |
| M3 | 全控件与全部 required 能力闭环 | 已完成（内部里程碑，`DONE_WITH_CONCERNS`） |
| M4 | demo、文档、安装和仓外消费者迁移 | 已完成（内部里程碑，`DONE_WITH_CONCERNS`） |
| M5 | 全量硬化、独立评审和 ABI 冻结 | 内部里程碑 `DONE_WITH_CONCERNS`（Task 1–8；聚合器未全绿） |

状态只能在对应阶段 fresh gate 全部通过后更新，不以提交数量或局部测试通过代替完成证据。

## 轻量硬约束

- 基础 wrapper：当前 `184 B`，上限 `192 B`。
- 每种 wrapper：相对 M0 baseline 的增量不得同时超过 `5%` 和 `8 B`。
- backend widget：当前 `496 B`，上限 `512 B`。
- 32 位 ABI runtime pool：timer、event callback 和 bookkeeping 合计不超过 `1024 B`。
- image source：32 位 ABI 不超过 `80 B`。
- font value：32 位 ABI 不超过 `16 B`。
- steady-state `tinyui_process()` 隐式 heap 分配为 `0`。
- 未启用控件和可选模块不得进入最小裁剪二进制。
- binary size 沿用 `5%/8192 B` 等双阈值策略，详见设计与 release gate。

## 完成口径

v2.3 只在以下条件同时满足时关闭：

- 只剩一套 canonical runtime、object、event、timer、theme 和资源 value API。
- 所有 required 能力达到 L4。
- 可见能力达到 L5-V，可操作能力达到 L5-E。
- 全量构建、CTest、ASan、UBSan、安装消费者和文档示例通过。
- Flash、RAM、对象尺寸和零稳态分配门禁通过。
- 独立评审没有未关闭的 P0/P1 非 port 问题。
- port 延期项仍被明确标记为未完成。

## 执行约束

- 当前项目不创建 worktree。
- 所有 shell 命令使用 `rtk` 前缀。
- 修改任何函数前先运行 GitNexus upstream impact；`HIGH/CRITICAL` 必须先告警并形成调用者迁移清单。
- 每阶段结束运行 GitNexus `detect_changes()`。
- 多个 subagent 写面不得重叠。
- review 不通过时，由原 review subagent 跟踪修复。
- demo 只表达用户意图，不承担 backend/layout 修补。

## 当前声明边界

规划完成不等于实现完成。当前文档只锁定 v2.3 的目标、任务和门禁，不改变 TinyUI 当前 Alpha 状态，也不改变 port 尚未生产闭环的事实。

## 当前执行进度

> **停点（2026-07-15 M5 Task 1–8 closeout）：** 已完成范围 = **M0–M4 内部里程碑 + M5 Task 1–8**。Task7：`gitnexus-impact.json`（MCP 不可用 → git-diff fallback，`fallback=true`）。Task8：ABI manifest 冻结 + check 模式 PASS；release-evidence 补齐 `minimal/abi/install_consumer/docs_examples/capability_matrix`；`abi_frozen=true`、`port_completed=false`、`manual_reviewed_passed=false`。最终聚合器 **诚实 FAIL**（gitnexus fallback；asan/ubsan/manual residual 保持 fail）。**不**声明 v2.3 已完成/可发布；**不**声明 port/L6 生产可用。

### M0 / M1

- M0：**已完成**
- M1：**已完成**（closeout `DONE_WITH_CONCERNS`）
  - 可声明：canonical 公共契约冻结、独立消费者可编译链接、轻量静态门禁与编译期裁剪已建立
  - 复跑证据：`check_tinyui_v23_public_api.py --check`、`check_tinyui_public_api.py`、`check_tinyui_deprecated_api_usage.py` 绿
  - **不得**声明：L3–L5 全能力、port、`fixed_pool`

### M2

- 状态：**已完成（内部里程碑，`DONE_WITH_CONCERNS`）**，M3 Task10 已重基 `v23_core_vertical` L5-V baseline；M2 四样板 L5 再次可绿，但仍以当前机器 capture 为准。
- 仍真实成立：
  - 单实例 runtime / 唯一 LD tree / deferred delete flush
  - 固定 timer·event 池（容量 16）+ process deadline
  - 四样板 label/button/checkbox/slider **L1–L4 unit** 与 **backend mapping** 绿
  - wrapper：base `192 B`；label `200` / button `248` / checkbox `208` / slider `216`
  - 32 位 pool sum：`888 B`；steady-state 隐式 heap `0`
  - baseline `runtime_static_ram.implementation` 仍为 **`legacy`**（不伪造 `fixed_pool`）
- full-tree `tinyui_demo` 签名债已由 M4 Task 1–5 消化；全量 CTest / binary / perf 指纹仍待后续收口。

### M3

- 状态：**已完成（内部里程碑，`DONE_WITH_CONCERNS`）** — Task 11 closeout 2026-07-15
- 任务 1–11 均已落地；Task 10/11 均为 `DONE_WITH_CONCERNS`

| 任务 | 结论 | 复跑/抽查证据 |
| --- | --- | --- |
| 1 ledger | DONE | 634 行、`owner_task` 齐全；exhaustiveness/matrix 绿 |
| 2 仪表族 | DONE | switch/arc/gauge/progress_* unit 绿 |
| 3 选择族 | DONE_WITH_CONCERNS | list/combo/scroll_selector/… unit 绿；CTest 目标历史拼写已收敛到 selector |
| 4 输入数据族 | DONE_WITH_CONCERNS | text/line_edit/keyboard/table/graph unit 绿；部分 dedicated cb |
| 5 媒体复合族 | DONE_WITH_CONCERNS | image/canvas/…/window unit 绿 |
| 6 theme | DONE_WITH_CONCERNS | theme unit/contract 绿；create 不自动 apply |
| 7 layout/resource | DONE | layout/resource unit + layout contract 绿 |
| 8 common adapter | DONE_WITH_CONCERNS | public/deprecated 门禁绿；kind→`ld*` 静态 adapter |
| 9 minimal profile | DONE_WITH_CONCERNS | `check_tinyui_minimal_profile` 1/1 |
| 10 L5 证据 | DONE_WITH_CONCERNS | 六个 `v23_*` L5-V + 家族 partial L5-E |
| 11 closeout | DONE_WITH_CONCERNS | `tinyui_m3_core` 55/55；wrapper/steady-state/minimal/contract 绿；demo/binary/perf 延期 |

- Task 11 复跑：`build/v2.3-m3` 上 `-L tinyui_m3_core` **55/55**；wrapper overhead OK；steady-state 0；minimal 1/1。
- closeout 小修复：`tinyui_deinit` 清 theme 借用；`tinyui_obj_apply_style` 无 runtime 不解引用 sentinel。
- **遗留 concerns：**
  1. full-tree demo 签名债已清；全量 CTest / binary size / runtime perf 指纹仍待后续收口
  2. Task 10 partial L5-E（非每个 setter 独立轨迹）
  3. theme kind 矩阵仍偏样板；legacy `set_on_*` / 历史 misspelled selector 拼写
  4. minimal 仍链接完整 `longdonggui` backend
  5. `runtime_static_ram.implementation=legacy`；port/L6 未做

### 可声明 vs 不得声明

**可声明（截至本停点）：**
- M1 canonical 契约冻结与轻量静态门禁
- M2 核心纵向 + 四样板 L1–L5（以当前机器 baseline 为准）
- M3 全控件族 L4 + 六场景 L5 证据 + common adapter + minimal 裁剪 + closeout 轻量门禁
- `tinyui_m3_core` focused gate 全绿

**不得声明：**
- full-tree demo / 全量 CTest 零失败
- binary/perf 机器指纹已在本机构建并强制通过
- port 生产闭环 / L6
- `runtime_static_ram.implementation=fixed_pool`
- 每个 required setter 均有独立 L5-E
- backend/`ld*` 级 Flash 已随 minimal 同步裁剪
- v2.3 发布完成（仍需 M4/M5）

### M4

- 状态：**已完成（内部里程碑，`DONE_WITH_CONCERNS`）** — Task 8 closeout 2026-07-15
- Task 1–7：demo boundary、29 demo canonical runner、install/export/find_package、仓外 C/C++ consumer、current-facing 用户文档与 docs examples 门禁（详见既有 Task 记录）
- Task 8（2026-07-15）：**DONE_WITH_CONCERNS**
  - `docs/v2.3/v2.3-migration-guide.md`：12 章破坏式迁移映射 + before/after
  - `docs/v2.3/deferred-port-work.md`：明确 M4 SDL consumer/demo **仅测试宿主**，非 L6/生产
  - 扩展 `check_tinyui_deprecated_api_usage.py`；`check_tinyui_removed_api.py` full/minimal 均零 forbidden
  - **删除** 仓内 v2.2 临时迁移桥源文件与全部私有 CMake/宏接线；tests/support 迁 internal runtime / public API
  - MCU 示例模板改为 `tinyui_init` + `screen` + `process`（仍非生产 port）
- **可声明：** M4 消费者/交付边界内部里程碑；29 demo canonical；install consumer；迁移桥已删除
- **不得声明：** port/L6 生产可用；v2.3 发布完成；全量 CTest/binary/perf 零债

### M5

- 状态：**进行中**
- Task 1（2026-07-15）：**完成** — fail-closed release gate manifest + 聚合 checker + CTest 注册；先红确认（缺 ctest 正式名与 release-evidence）
- Task 2（2026-07-15）：**完成** — clean 标准 profile 119/120 + `standard.json`
- Task 3（2026-07-15）：**完成（诚实 FAIL 证据）** — Clang ASan/UBSan 隔离构建/ctest + `asan.json`/`ubsan.json`；修复若干 sanitizer 构建/产品问题，仍有 Arm-2D/heap/consumer 失败未清零
- Task 4（2026-07-15）：**完成（`DONE_WITH_CONCERNS`）** — wrapper/pool/descriptor/binary/time/零分配/minimal 正式 gate 8/8 PASS + `lightweight-gates.json`
  - wrapper base `192`、backend `200≤512`；32-bit pool `888≤1024`；32-bit image/font `72/16`
  - binary 与 `binary.full` baseline 零增量；steady-state 隐式 heap `0`
  - v23 指纹匹配：`screen_object_create` p95 `2.0≤5`、`capture_ready` p95 `5.0≤40`（warmup=5, runs=30）
  - minimal：full 树 wrapper + `build/v2.3-minimal` 直跑均绿
- Task 5（2026-07-15）：**完成（`DONE_WITH_CONCERNS`）** — 能力矩阵关闭 + 人工 review
  - `check_tinyui_release_capability_matrix` PASS；446 required covered 行绑定 `capability_id`/L1–L5 证据路径
  - 修正 scroll_selector ctest 别名；checker 打开 PPM 并拒绝事件/像素交叉替代
  - `build/v2.3/release-evidence/manual-review.json`：七项 `T/F/T/T/T/T/T`，`manual_reviewed_passed=false`
  - `deferred-port-work.md` / `v2.3-release-gates.md` 固定 `port_completed=false`、L6 不进 core
- Task 6（2026-07-15）：**完成（`DONE_WITH_CONCERNS`）** — 独立代码评审与修复跟踪
  - 文档：`docs/v2.3/v2.3-release-review.md`；证据投影：`independent_review.json`；对齐 `manual_review.json`
  - P0=0；P1=2 均 closed（table `create_with_props` 镜像假成功；evidence 文件名）
  - 修复：`tinyui/src/widgets/table.c` + unit/demo；`test_tinyui_table` PASS
- Task 7（2026-07-15）：**完成（`DONE_WITH_CONCERNS`）** — GitNexus 影响复核
  - `build/v2.3/release-evidence/gitnexus-impact.json`：MCP 不可用，git-diff fallback，`fallback=true`，status 诚实非 pass
- Task 8（2026-07-15）：**完成（`DONE_WITH_CONCERNS`）** — ABI 冻结 + 证据补齐 + 最终聚合
  - `tests/tinyui/contract/tinyui_v23_abi_manifest.json` 冻结；`--emit-abi-manifest` / `--check-abi-manifest` 互斥
  - 证据：`minimal.json` `abi.json` `install_consumer.json` `docs_examples.json` `capability_matrix.json`
  - release gate manifest：`abi_frozen=true`，`manual_reviewed_passed=false`，`port_completed=false`
  - 聚合器 FAIL：`artifact_fallback_true`（gitnexus）；未改写 asan/ubsan/manual
- **可声明：** M5 内部里程碑 closeout（`DONE_WITH_CONCERNS`）；canonical ABI manifest 已冻结且可 check；最终聚合 fail-closed；lightweight/install/docs/capability 证据齐
- **不得声明：** v2.3 已完成/可发布；人工矩阵关死；全量 ASan/UBSan 已绿；GitNexus 正式 impact 无 fallback；port/L6 生产可用

### 后续 residual（不阻塞“内部里程碑记录”，阻塞“发布声明”）

1. 消除 gitnexus fallback（恢复 GitNexus MCP/`detect_changes` 正式证据）
2. 清零 ASan/UBSan residual 或给出明确 non-TinyUI 排除清单与人工记录
3. 可选：补齐复杂可操作控件独立 L5-E，使 `manual_reviewed_passed` 可转 true
4. 独立 port 版本线（见 `deferred-port-work.md`）