## TINYUI 开发规则

- `TINYUI` 是上层 API，不是第二套 GUI 渲染器。
- `TINYUI` 的 100% 目标是控件与用户可用能力 100% 覆盖，不是把 `LingDongGUI` 的 `ld*` API 逐字翻译成 `tinyui_*` API；过去能用 `LingDongGUI` 实现的控件功能和对应能力，现在必须能只用 `TINYUI` public API 实现。
- 判断缺口时以“用户能否通过 TINYUI 完成同等控件能力”为准；允许 API 形态、命名、参数模型与 `LingDongGUI` 不同，但不得因为不是一一 API wrapper 就漏掉原本可实现的功能。
- `policy_never_public` 只能用于生命周期、渲染管线、内存、宿主内部、调试或 backend-private helper 等不应成为用户控件能力的内部项；不能把真实用户可见/可操作的控件能力放进 policy 来规避实现。
- 禁止在 `tinyui/src/backend/ldgui/backend_app.c` 里继续堆固定坐标、固定尺寸、假控件画法；SDL 只做宿主显示，不做 `TINYUI` 专属 fake renderer。
- `TINYUI -> LingDongGUI` 必须走真实 backend 映射：`window/label/button/checkbox/switch/slider/text/image`、`flex/grid`、`theme/event` 都要落到真实 `LingDongGUI` 对象和行为。
- `tinyui/demo/*` 只允许使用 `tinyui_*` API；禁止泄漏 `ld*`、`arm_2d_*`、`SIGNAL_*`。
- 禁止用修改 demo 页面、硬编码 `set_size()/set_pos()`、补假视觉，来掩盖 backend/layout 缺口；demo 代码只能表达用户意图，不能承担适配补丁职责。
- 任何“UI 已完成”结论，必须基于真实 `LingDongGUI` 输出证据；“能弹窗”不等于“适配完成”。
- `tests/tinyui/runtime/*` 只能证明 smoke / 启动 / capture；不能把 fake renderer 通过当成 backend 已闭环。
- 做 `TINYUI` 改动时，优先补真实 backend tree、真实 widget mapping、真实 layout/event/theme；不要优先修截图外观。
- 若当前实现只能靠临时 fake 路径工作，文档必须明确标注为 `temporary smoke path`，不得写成正式方案。

- 采用 cmake 编译
- 当前项目不创建 worktree。


<!-- gitnexus:start -->
# GitNexus — Code Intelligence

This project is indexed by GitNexus as **LingDongGUI** (122549 symbols, 181185 relationships, 300 execution flows). Use the GitNexus MCP tools to understand code, assess impact, and navigate safely.

> If any GitNexus tool warns the index is stale, run `npx gitnexus analyze` in terminal first.

## Always Do

- **MUST run impact analysis before editing any symbol.** Before modifying a function, class, or method, run `gitnexus_impact({target: "symbolName", direction: "upstream"})` and report the blast radius (direct callers, affected processes, risk level) to the user.
- **MUST run `gitnexus_detect_changes()` before committing** to verify your changes only affect expected symbols and execution flows.
- **MUST warn the user** if impact analysis returns HIGH or CRITICAL risk before proceeding with edits.
- When exploring unfamiliar code, use `gitnexus_query({query: "concept"})` to find execution flows instead of grepping. It returns process-grouped results ranked by relevance.
- When you need full context on a specific symbol — callers, callees, which execution flows it participates in — use `gitnexus_context({name: "symbolName"})`.

## Never Do

- NEVER edit a function, class, or method without first running `gitnexus_impact` on it.
- NEVER ignore HIGH or CRITICAL risk warnings from impact analysis.
- NEVER rename symbols with find-and-replace — use `gitnexus_rename` which understands the call graph.
- NEVER commit changes without running `gitnexus_detect_changes()` to check affected scope.

## Resources

| Resource | Use for |
|----------|---------|
| `gitnexus://repo/LingDongGUI/context` | Codebase overview, check index freshness |
| `gitnexus://repo/LingDongGUI/clusters` | All functional areas |
| `gitnexus://repo/LingDongGUI/processes` | All execution flows |
| `gitnexus://repo/LingDongGUI/process/{name}` | Step-by-step execution trace |

## CLI

| Task | Read this skill file |
|------|---------------------|
| Understand architecture / "How does X work?" | `.claude/skills/gitnexus/gitnexus-exploring/SKILL.md` |
| Blast radius / "What breaks if I change X?" | `.claude/skills/gitnexus/gitnexus-impact-analysis/SKILL.md` |
| Trace bugs / "Why is X failing?" | `.claude/skills/gitnexus/gitnexus-debugging/SKILL.md` |
| Rename / extract / split / refactor | `.claude/skills/gitnexus/gitnexus-refactoring/SKILL.md` |
| Tools, resources, schema reference | `.claude/skills/gitnexus/gitnexus-guide/SKILL.md` |
| Index, status, clean, wiki CLI commands | `.claude/skills/gitnexus/gitnexus-cli/SKILL.md` |

<!-- gitnexus:end -->
