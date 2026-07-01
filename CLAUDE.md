## TINYUI 开发规则

- `TINYUI` 是上层 API，不是第二套 GUI 渲染器。
- `TINYUI` 的 100% 目标是控件与用户可用能力 100% 覆盖，不是把 `LingDongGUI` 的 `ld*` API 逐字翻译成 `tinyui_*` API；过去能用 `LingDongGUI` 实现的控件功能和对应能力，现在必须能只用 `TINYUI` public API 实现。
- 判断缺口时以“用户能否通过 TINYUI 完成同等控件能力”为准；允许 API 形态、命名、参数模型与 `LingDongGUI` 不同，但不得因为不是一一 API wrapper 就漏掉原本可实现的功能。
- `policy_never_public` 只能用于生命周期、渲染管线、内存、宿主内部、调试或 backend-private helper 等不应成为用户控件能力的内部项；不能把真实用户可见/可操作的控件能力放进 policy 来规避实现。
- 没有 backend
- 采用 cmake 编译
- worktree 合并，可以不管主仓库文档变更，主仓库的文档可能一直在自动更新。

<!-- gitnexus:start -->
# GitNexus — Code Intelligence

This project is indexed by GitNexus as **LingDongGUI** (106375 symbols, 136709 relationships, 300 execution flows). Use the GitNexus MCP tools to understand code, assess impact, and navigate safely.

> Index stale? Run `node .gitnexus/run.cjs analyze` from the project root — it auto-selects an available runner. No `.gitnexus/run.cjs` yet? `npx gitnexus analyze` (npm 11 crash → `npm i -g gitnexus`; #1939).

## Always Do

- **MUST run impact analysis before editing any symbol.** Before modifying a function, class, or method, run `impact({target: "symbolName", direction: "upstream"})` and report the blast radius (direct callers, affected processes, risk level) to the user.
- **MUST run `detect_changes()` before committing** to verify your changes only affect expected symbols and execution flows. For regression review, compare against the default branch: `detect_changes({scope: "compare", base_ref: "master"})`.
- **MUST warn the user** if impact analysis returns HIGH or CRITICAL risk before proceeding with edits.
- When exploring unfamiliar code, use `query({query: "concept"})` to find execution flows instead of grepping. It returns process-grouped results ranked by relevance.
- When you need full context on a specific symbol — callers, callees, which execution flows it participates in — use `context({name: "symbolName"})`.

## Never Do

- NEVER edit a function, class, or method without first running `impact` on it.
- NEVER ignore HIGH or CRITICAL risk warnings from impact analysis.
- NEVER rename symbols with find-and-replace — use `rename` which understands the call graph.
- NEVER commit changes without running `detect_changes()` to check affected scope.

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
