# PicoUI 串行路线入口

> 当前路线从 `v1.0` 开始改变方向。旧的 `PicoUI -> LingDongGUI backend wrapper` 文档已归档，只作为历史参考，不再作为当前实施计划。
> 当前 authoritative 状态：`v1.0.1 ready`

## 当前路线

PicoUI `v1.0+` 的目标是 native-only：

```text
PicoUI public API -> PicoUI native runtime/widgets -> ARM-2D -> PicoUI port/display
```

当前真相源：

- `docs/picoui-serial/v1.0-native/线计划索引.md`
- `docs/superpowers/specs/2026-06-05-picoui-v1-0-1-native-arm2d-runtime-design.md`

状态判定规则：

- 当前是否仍有剩余开发工作，以 `docs/picoui-serial/v1.0-native/线计划索引.md` 和各 phase closeout 文档为准。
- `docs/superpowers/plans/2026-06-05-picoui-v1-0-1-*.md` 里的 checkbox 主要保留执行模板语义，不作为当前收口状态真相源。
- phase 文档内出现的 `RED`、`待迁移`、`未完成边界` 等描述，默认按“当时阶段快照”理解；是否仍是当前 blocker，必须回到线索引和 release gate 文档确认。

## 归档路线

旧路线已移动到：

- `docs/picoui-serial/archive/pre-v1.0-wrapper-backend/`

归档内容包括：

- A/B/C/D/F/G/H/J 线
- `a-0.3` 到 `a-0.14` 线
- 旧 wrapper/backend mapping、visible gate、capability parity、port-layer 等历史计划

这些文档可用于迁移参考、对照 oracle、历史追溯。不得把其中的 `ldgui backend` 目标继续当成 v1.0 当前方向。
