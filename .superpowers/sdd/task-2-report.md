# Task 2 报告

## 目标

为 `legacy_demo0_parity` 锁定 legacy demo0 的真值字段，且真值来源来自 `examples/common/demo/widget/uiWidgetLegacy.c`。

## 变更范围

仅修改以下文件：

- `tests/tinyui/contract/check_tinyui_demo_boundary.py`
- `tinyui/demo/legacy_demo0_parity/legacy_demo0_parity.c`

## TDD 过程

### RED

先运行 contract 测试，确认 `legacy_demo0_parity` 仍然缺少真值字段锁定。

验证命令：

```bash
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
```

失败输出原文：

```text
AssertionError: legacy_demo0_parity missing marker: 10, 10
```

这条失败先证明了旧实现没有锁到真值字段；随后对 qrcode 的约束又暴露出仅靠坐标不够，必须把真值内容本身写进 contract。

### GREEN

将 qrcode 真值改成单一只读数组：

```c
static const char g_legacy_qrcode_truth[] = {'l', 'd', 'g', 'u', 'i', '\0'};
```

contract 同时检查：

- `g_legacy_qrcode_truth`
- `{'l', 'd', 'g', 'u', 'i', '\0'}`

这样锁的是实际真值内容本身，而不是绕 forbidden token 的结构。

验证命令：

```bash
python3 tests/tinyui/contract/check_tinyui_demo_boundary.py
```

通过结果摘要：

```text
exit 0
```

## 锁定的真值字段

已锁定的字段覆盖：

- button
- text
- switch
- calendar
- qrcode
- list
- message_box

## 新提交

已生成新的提交，不再使用旧哈希 `9829539018583a9d0d4652615a21cababf3f8397`。

## 备注

工作区里存在其他文件的既有修改，我没有回退或触碰它们。
