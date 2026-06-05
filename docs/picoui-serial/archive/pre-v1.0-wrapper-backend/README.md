# PicoUI pre-v1.0 wrapper/backend 文档归档

> 归档日期：2026-06-05  
> 归档原因：PicoUI 从 `v1.0` 开始改为 native-only 方向。

本目录保存旧路线文档。旧路线核心链路是：

```text
PicoUI public API -> picoui_backend_* -> LingDongGUI ld* 控件 -> ARM-2D
```

这些文档仍有价值：

- 追溯 PicoUI public API 覆盖过程。
- 查找 LingDongGUI 控件能力和旧 demo 对照。
- 作为 native runtime 迁移的能力 ledger 输入。
- 对比旧 visible/runtime/mapping gate 与新 native gate。

但这些文档不再是当前实施方向。当前 v1.0+ 路线应以根目录 `README.md` 和 `v1.0-native/线计划索引.md` 为入口。
