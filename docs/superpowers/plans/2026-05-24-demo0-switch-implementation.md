# Demo0 Switch 演示接入 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在 `demo0` legacy widget 页面中新增一个可切换的 `switch` 和一行 `ON/OFF` 状态文字，展示 `ldSwitch` 的最小接入方式。

**Architecture:** 变更集中在 `uiWidgetLegacy.c`，通过在 `uiWidgetLegacyInit()` 中创建 switch 与 label，并使用 `SIGNAL_VALUE_CHANGED` 回调更新文案，不把这类一次性 UI 演示逻辑塞进页面循环。测试侧扩展现有 Python 检查脚本，用源码断言锁定 demo0 已接入 `ldSwitchInit` 与状态文字。

**Tech Stack:** C, Python, LingDongGUI, SDL2, CMake, GitNexus

---

### Task 1: 先补 failing 测试锁定 demo0 必须接入 switch 演示

**Files:**
- Modify: `examples/sdl/tests/check_use_demo_0_legacy_widget.py`
- Test: `examples/sdl/tests/check_use_demo_0_legacy_widget.py`

- [ ] **Step 1: 在测试里增加对 legacy demo 源码的静态断言**

```python
LEGACY_WIDGET_FILE = SDL_ROOT.parent / "common" / "demo" / "widget" / "uiWidgetLegacy.c"


def test_demo0_legacy_widget_contains_switch_demo() -> None:
    source = LEGACY_WIDGET_FILE.read_text(encoding="utf-8")
    assert "ldSwitchInit(" in source
    assert '"ON"' in source
    assert '"OFF"' in source
```

- [ ] **Step 2: 先运行单测，确认当前基线失败**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI/examples/sdl
rtk pytest tests/check_use_demo_0_legacy_widget.py -q
```

Expected: FAIL，因为当前 `uiWidgetLegacy.c` 里还没有 `ldSwitchInit` / `ON` / `OFF`。

- [ ] **Step 3: 如失败原因不是断言失败，先修测试路径再重跑**

```python
assert LEGACY_WIDGET_FILE.exists(), LEGACY_WIDGET_FILE
```

- [ ] **Step 4: 再跑一次，确认现在失败点就是缺少 switch 接入**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI/examples/sdl
rtk pytest tests/check_use_demo_0_legacy_widget.py::test_demo0_legacy_widget_contains_switch_demo -q
```

Expected: FAIL，断言 `ldSwitchInit(` 不存在。

### Task 2: 在 demo0 页面中接入 switch 与状态文字

**Files:**
- Modify: `examples/common/demo/widget/uiWidgetLegacy.c`
- Test: `examples/sdl/tests/check_use_demo_0_legacy_widget.py`

- [ ] **Step 1: 先补本地常量和回调声明，给 switch/status label 预留固定 id**

```c
static const uint8_t g_legacy_switch_on_str[] = "ON";
static const uint8_t g_legacy_switch_off_str[] = "OFF";

static bool uiWidgetLegacySwitchValueChanged(ld_scene_t *ptScene, ldMsg_t msg)
{
    (void)msg;
    ldLabel_t *ptLabel = ldBaseGetWidget(ptScene->ptNodeRoot, 31);
    ldSwitch_t *ptSwitch = ldBaseGetWidget(ptScene->ptNodeRoot, 30);

    if (ptLabel != NULL && ptSwitch != NULL) {
        ldLabelSetText(ptLabel, (uint8_t *)(ldSwitchIsChecked(ptSwitch) ? g_legacy_switch_on_str : g_legacy_switch_off_str));
    }
    return false;
}
```

- [ ] **Step 2: 在 `uiWidgetLegacyInit()` 的常用控件区附近新增 switch 和状态 label**

```c
obj = ldSwitchInit(30, 0, 220, 110, 72, 36);
ldSwitchSetSelectable(obj, true);
ldSwitchSetChecked(obj, false);

obj = ldLabelInit(31, 0, 300, 104, 72, 24, FONT_ARIAL_12);
ldLabelSetText(obj, (uint8_t *)g_legacy_switch_off_str);
ldLabelSetBackgroundColor(obj, GLCD_COLOR_LIGHT_GREY);
ldLabelSetAlign(obj, ARM_2D_ALIGN_MIDDLE_LEFT);
ldBaseSetCorner(obj, true);
```

- [ ] **Step 3: 把 switch 的值变化信号接到新回调**

```c
connect(30, SIGNAL_VALUE_CHANGED, uiWidgetLegacySwitchValueChanged);
```

- [ ] **Step 4: 保持最小实现，不把状态同步逻辑塞进 `uiWidgetLegacyLoop()`**

```c
/* no loop polling needed; switch state sync stays in SIGNAL_VALUE_CHANGED callback */
```

- [ ] **Step 5: 运行单测，确认从红变绿**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI/examples/sdl
rtk pytest tests/check_use_demo_0_legacy_widget.py -q
```

Expected: PASS。

### Task 3: 跑构建验证，确认 legacy demo 仍可编译

**Files:**
- Verify only: `examples/sdl/CMakeLists.txt`

- [ ] **Step 1: 配置一个临时 build 目录**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI/examples/sdl
rtk cmake -S . -B build-demo0-switch -DUSE_DEMO=0
```

Expected: 配置成功，生成 build 文件。

- [ ] **Step 2: 执行构建**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI/examples/sdl
rtk cmake --build build-demo0-switch
```

Expected: 构建成功，`demo0` 相关目标无编译错误。

- [ ] **Step 3: 复跑最小测试，避免“构建绿、测试红”**

Run:

```bash
cd /Users/cys/embedded/LingDongGUI/examples/sdl
rtk pytest tests/check_use_demo_0_legacy_widget.py -q
```

Expected: PASS。
