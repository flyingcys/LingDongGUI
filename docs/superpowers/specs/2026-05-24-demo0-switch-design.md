# demo0 页面 switch 演示设计说明

**日期**：2026-05-24  
**状态**：已确认，可进入实现  
**范围类型**：现有 demo 页面小范围增强

---

## 1. 目标

在 `USE_DEMO=0` 对应的 legacy widget demo（`demo0`）页面中新增一个可点击切换的 `switch` 控件，用于展示 LingDongGUI 现有 `ldSwitch` 控件在 legacy 页面中的最小接入方式。

本次只做 UI 演示，不绑定真实业务逻辑。

---

## 2. 用户确认后的范围

### 2.1 本轮要做

- 在 `demo0` 页面新增 `1` 个 `switch`
- `switch` 放在按钮 / checkbox / slider 那组常用控件附近
- 用户点击后可在开 / 关之间切换
- 在 `switch` 附近新增 `1` 行状态文字
- 状态文字跟随开关实时显示 `ON` / `OFF`

### 2.2 本轮不做

- 不绑定真实业务状态或功能开关
- 不改 demo 路由
- 不改 `CMakeLists.txt`
- 不改 `demo0` 现有其它控件的业务行为
- 不扩展 `ldSwitch` 控件能力本身

---

## 3. 代码落点

主要改动文件：

- `examples/common/demo/widget/uiWidgetLegacy.c`
- `examples/sdl/tests/check_use_demo_0_legacy_widget.py`

说明：

- `demo0` 的页面搭建位于 `uiWidgetLegacyInit()`
- 本轮直接在该初始化函数中接入 `ldSwitch`
- 状态文字也在同一初始化函数中创建
- 开关状态变化通过 `SIGNAL_VALUE_CHANGED` 回调更新文字

---

## 4. 交互设计

### 4.1 初始状态

- switch 默认关闭
- 状态文字初始显示 `OFF`

### 4.2 点击行为

- 点击 switch 后切换 checked 状态
- 触发 `SIGNAL_VALUE_CHANGED`
- 回调根据当前状态把文字改成 `ON` 或 `OFF`

### 4.3 布局原则

- 尽量贴近 button / checkbox / slider 区域
- 不遮挡现有控件
- 不大幅调整现有 legacy demo 布局

---

## 5. 实现策略

采用最小方案：

1. 在 `uiWidgetLegacyInit()` 中新增一个未占用 id 的 `ldSwitch`
2. 在 switch 附近新增一个 label 作为状态文字
3. 新增一个本地回调函数处理 `SIGNAL_VALUE_CHANGED`
4. 回调里读取 switch 当前状态并更新 label 文本

这样可以：

- 复用 demo0 现有页面搭建方式
- 复用 `ldSwitch` 现有消息机制
- 避免把简单演示逻辑放进 `uiWidgetLegacyLoop()` 做轮询

---

## 6. 测试策略

遵循最小 TDD：

1. 先补一个 failing 测试，校验 `USE_DEMO=0` 的 legacy demo 源码中已经接入 `ldSwitchInit` 和状态文字 `ON/OFF`
2. 运行测试，确认当前基线失败
3. 实现最小代码
4. 再跑测试确认通过
5. 跑一次 cmake 构建，确保示例仍可编译

---

## 7. 风险与影响

GitNexus impact 结果：

- `uiWidgetLegacyInit`：`LOW`，0 直接调用，0 受影响流程
- `uiWidgetLegacySlotTest`：`LOW`，0 直接调用，0 受影响流程

结论：

- 本轮变更面很小
- 适合直接在 legacy demo 页面内完成
- 不需要改动公共路由与构建链
