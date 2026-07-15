# TinyUI v2.3 延期 Port 工作

## 状态

本文件记录 v2.3 明确不处理的 port 工作。它们不是 `policy_never_public`，也不是“已完成”；只是从 v2.3 核心/API 收口线延期。

**机器/发布口径（M5 Task 5 固定）：**

- `port_completed=false`（见 `tests/tinyui/contract/tinyui_v23_release_gate_manifest.json` 与 `build/v2.3/release-evidence/manual-review.json`）。
- **L6 不纳入 v2.3 core 完成判断**；L1–L5 证据关闭也不能推导 L6。
- 下列项在 v2.3 一律视为**未完成**：RGB565/ARGB8888 framebuffer/stride/PFB、同步/异步 flush/DMA/fence、平台时钟、SDL event pump/resize/context、MCU toolchain/RTOS/ISR/cache、真机板级验证。

**v2.3 / M4 / M5 关闭后仍不能宣称：**

- SDL / MCU port 生产可用；
- 整个 TinyUI 达到 L6；
- “M4 的 SDL consumer 或 demo 已证明 port 完成”；
- “M5 release evidence 或 manual review 证明 port 完成”。

### M4 SDL consumer / demo 的定位（必须强调）

M4 的 SDL consumer、demo runner 与仓外 `find_package` 消费，**只是测试宿主与交付边界证据**：

- 证明 demo / 文档 / install 包可在当前 SDL 测试宿主上链接与运行；
- **不是** L6 证据；
- **不是** 生产可用证明；
- **不**关闭本文件中的任一延期项。

**M4 关闭后 port 仍延期。** 后续 port 工作必须走独立版本线（见文末启动条件），不得借 M4 完成声明顺带宣称 flush/DMA/MCU/真机已闭环。

---

## 显示与 PFB

逐项保留、仍延期：

- RGB565 / ARGB8888 framebuffer 真实格式支持。
- stride、bytes-per-pixel 和 PFB buffer 尺寸一致性。
- 同步 / 异步 flush 契约。
- DMA ready、fence 和 buffer 复用时机。
- PFB helper、buffer 和 app target 的销毁与重绑。
- 未注册 flush 时的初始化失败和后续重试。
- 用户提供静态 PFB、allocator、对齐和容量。
- cache clean / invalidate 与 DMA 一致性。

---

## 平台时间

逐项保留、仍延期：

- MCU tick 单位与 Arm-2D reference clock 换算。
- POSIX monotonic clock provider。
- 平台 wall clock 与 UI monotonic time 隔离。
- RTOS tick、低功耗 sleep 和唤醒。

v2.3 只修 core timer 回绕、固定池和 next deadline，不处理 provider。

---

## SDL

逐项保留、仍延期：

- `LD_TINYUI_PORT=mcu/none` 时彻底不查找 SDL。
- SDL target 自包含 runtime reference clock，不依赖 demo stub。
- SDL event pump 的 pointer down/up 保序。
- SDL keyboard 采集和 key integration 调用。
- resize 时 texture / buffer 原子重建。
- SDL update / present 错误传播。
- `SDL_Init` / `SDL_Quit` 所有权。
- 绑定外部 SDL window / renderer / context。
- 重复建窗和不同尺寸安全性。

说明：当前仓库内 SDL demo / M4 consumer 仅作为 **L5 级测试宿主**使用，不覆盖上表生产契约。

---

## MCU

逐项保留、仍延期：

- 无 SDL 环境的真实交叉编译（MCU toolchain）。
- LCD 同步 / 异步 flush 模板。
- DMA、cache、ISR 和 RTOS 调用边界。
- MCU retarget syscall strong symbol 冲突。
- heap / static memory 选择与预算。
- 真机触摸、按键、动画和长时间运行。
- 至少一块目标开发板的 L6 证据。

---

## Port 公共契约

逐项保留、仍延期：

- display callback 的 `user_data`。
- display / indev / tick / OSAL 独立集成头的最终稳定 ABI。
- port 错误到 TinyUI runtime 的完整传播。
- 多 display、多 input 和未来 context 模型。
- port install / export package。

v2.3 允许为 canonical core 编译而做最小签名迁移，但不得借机宣称这些契约完成。

---

## v2.3 可使用的 Port 相邻能力

以下属于 core 或测试宿主，**不在**本文件延期：

- 单实例 canonical runtime。
- app 用户能力的 runtime / screen / timer / theme / focus 替代。
- core timer 回绕和固定池。
- 同步 event callback 与 key 抽象消费。
- TinyUI 统一结果码。
- image / font value 对 Arm-2D 类型的隔离。
- 使用当前 SDL 作为 **测试宿主**（非生产、非 L6）。

---

## 禁止的发布声明

v2.3 / M4 后仍禁止：

- “ARGB8888 display 已支持”。
- “DMA / 异步 flush 已安全”。
- “MCU port 可直接量产”。
- “无 SDL 的 MCU 工程已验证”。
- “重复 init / deinit 的真实 PFB 生命周期已闭环”。
- “所有目标平台已经达到 L6”。
- “M4 SDL consumer / demo 证明 port 生产可用或 L6 完成”。

---

## 后续启动条件

只有 v2.3 canonical API 和 core 冻结后，才启动独立 port 版本线。该版本线必须：

1. 重新运行 display、indev、tick、PFB 和 CMake 符号的 GitNexus impact。
2. 独立设计同步 / 异步 flush 和 memory ownership。
3. 分别建立 SDL host 与 MCU board 证据（真机验证不可被 M4 测试宿主替代）。
4. 不修改已冻结的普通用户控件 API。
