# Task 3 执行报告

## 1. Impact 分析结果

**方式**：rg fallback（gitnexus 未启用）

```
/home/share/samba/flyingcys/LingDongGUI/tinyui/src/widgets/arc.c:50:  定义
/home/share/samba/flyingcys/LingDongGUI/tinyui/src/widgets/arc.c:166: 唯一调用点（tinyui_arc_create 内部）
```

**风险评估**：LOW — `tinyui_arc_ld_init` 是 static 函数，只有 `tinyui_arc_create` 调用，无跨文件暴露。

---

## 2. Category A 修复：根窗口背景色

**文件**：`tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c`

**修改位置**：`if (win == 0)` 的 return 之后

**添加内容**：
```c
tinyui_window_set_color(win, 0xF0F0F0U);
```

**修改后上下文**：
```c
    if (win == 0) {
        return;
    }

    tinyui_window_set_color(win, 0xF0F0F0U);   // 新增

    g_runtime.image = 0;
```

**原理**：`ldWindowInit(nameId=0)` 默认设置背景为 `RGB(240,240,240)`，`tinyui_screen_create()` nameId 不为 0 不触发该逻辑，导致背景黑色 `0x000000`，arc 的 `parentColor=0xF0F0F0` 与实际背景不匹配，镂空显示灰色。

---

## 3. Category B 修复：arc 初始化尺寸

**文件**：`tinyui/src/widgets/arc.c`

**修改位置**：第 68-69 行（`ldArc_init` 调用内）

| 参数 | 修改前 | 修改后 |
|------|--------|--------|
| width | 160 | 1 |
| height | 160 | 1 |

**原理**：`ldArc_init` 用传入尺寸初始化 `tTempRegion`（dirty region cache）。后续 `tinyui_widget_set_size(arc, 103, 103)` 更新 `tRegion` 但 `tTempRegion` 不同步，导致 dirty region 管理使用 160x160 错误缓存。改为 1x1 最小安全值，避免缓存超大尺寸。

---

## 4. 构建结果

**结果**：成功

**最后输出**：
```
[83%] Building C object CMakeFiles/tinyui_demo.dir/.../legacy_demo0_parity.c.o
[85%] Linking C executable tinyui_demo
[100%] Built target tinyui_demo
```

**警告**：仅存在已有的 `-Wfree-nonheap-object` LTO 警告（非新增，来自 Arm-2D 内部），无 error。

**二进制**：`build-tinyui/tinyui_demo`（4.2M）

---

## 5. detect_changes 结果

**方式**：`git diff --stat`

本任务仅 stage 并提交了以下两个文件：
- `tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c`（+2 行，Category A）
- `tinyui/src/widgets/arc.c`（4 行变更，Category B）

无意外文件变更。

---

## 6. Commit Hash

```
5bf3248 fix(tinyui): fix arc shape error — correct background color and arc init size
```
