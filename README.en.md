<h4>
  <a href="README.md">中文</a> |
  <a>English</a>
</h4>

<h1 align="center" style="margin: 30px 0 30px; font-weight: bold;">LingDongGUI</h1>
<h4 align="center">A GUI based on ARM-2D</h4>
<p align="center">
	<a href="https://gitee.com/gzbkey/LingDongGUI/stargazers"><img src="https://gitee.com/gzbkey/LingDongGUI/badge/star.svg"></a>
	<a href="https://gitee.com/gzbkey/LingDongGUI/members"><img src="https://gitee.com/gzbkey/LingDongGUI/badge/fork.svg"></a>
	<a><img src="https://img.shields.io/github/license/gzbkey/LingDongGUI"></a>
	<a><img src="https://img.shields.io/badge/architecture-ARM%20|%20RISC%20V-blue"></a>	
</p>

## Features

1. **Universal chip support**
    * Supports ARM microcontrollers and RISC-V microcontrollers
    * Support simulated running on PC (SDL)

2. **Easy to use**
    * Secondary encapsulation greatly reduces the difficulty of using ARM-2D
    * Signal-slot mechanism makes many-to-many event handling effortless
    * A clean separation of UI and business logic yields crystal-clear program flow

3. **Support for Native ARM-2D APIs**
    * ARM-2D native APIs are available as needed
    * Mix LDGUI with ARM-2D for flexible hybrid development

4. **Dynamic Auto-Trimming**
    * Unused widgets auto-drop

5. **Extensible Widgets**
    * Script-generated skeletons let you focus on look and feel

6. **Built-in Functions**
    * Software timer
    * Multi-function keys: physical & touch
    * Queue
    * Color log print
7. **GUI Editor**
    * Comes with a free GUI editor

## Source Code

🏠️Gitee repo: https://gitee.com/gzbkey/LingDongGUI

🏠️Github repo: https://github.com/gzbkey/LingDongGUI

## GUI Editor

🚀GUI Editor repo: https://gitee.com/gzbkey/GuiEasyEditor

## Demo Preview

widget demo sdl
<p>
	<img src="./docs/images/widget%20demo.gif" width="400" />
</p>

## Widget List

| Status | Name | Description |
| :----:| ---- | ---- |
| ✅ | window | Layered container ，Support automatic horizontal or vertical layout|
| ✅ | image | image display |
| ✅ | button | Clickable button |
| ✅ | text | text |
| ✅ | progress bar | Progress with movable-image animation |
| ✅ | progress wheel | progress wheel (ARM 2D) |
| ✅ | check box + radio button| Check-box & radio styles with custom icons/text |
| ✅ | radia menu | Rotary selection menu |
| ✅ | label | Simple text label |
| ✅ | slider | Drag slider |
| ✅ | switch | Toggle switch with horizontal/vertical layouts, disabled state, and animated transitions |
| ✅ | scroll selecter | Scroll-wheel picker |
| ✅ | date time | Digital clock & date |
| ✅ | icon slider | Sliding icon bar |
| ✅ | gauge | Circular gauge |
| ✅ | qr code | QR-code display |
| ✅ | table | Data table |
| ✅ | keyboard | On-screen keyboard |
| ✅ | line edit | Text input field |
| ✅ | graph | Waveform display |
| ✅ | combo box | Drop-down list |
| ✅ | arc bar | Circular progress ring |
| ✅ | animation | GIF-style animation |
| ✅ | calendar | calendar |
| ✅ | message box | Pop-up message |
| ✅ | clock | Analog clock |
| ✅ | list | Scrollable list hosting any widget |
| 🔲 | spectrum viewer | Spectrum viewer |

## File Structure

|Name|Type|Description|
|---|---|---|
|docs|folder|Detailed documentation|
|examples|folder|Example projects|
|src|folder|Source code|
|tools|folder|Companion utilities and scripts|

## Tutorial

📖 [中文教程](https://ldgui-doc-cn.readthedocs.io)

## PicoUI

PicoUI is the application-level abstraction built on top of LingDongGUI and exposes a unified Linux-style `picoui_*` API.

- Start from the repository root CMake entrypoint: `rtk cmake -S . -B build`
- The default test entrypoint also lives at the repository root via `CTest`
- The current default build still enables `LD_BUILD_SDL_DEMO=ON`, so SDL dependencies remain part of the default build path
- `tests/picoui/runtime/check_picoui_runtime.py` uses a separate build tree at `build/picoui-runtime`
- Direct `examples/sdl` configure is still useful for focused SDL demo debugging, but it is no longer the primary recommended path

### PicoUI 当前能力矩阵

| 能力 | 当前状态 | 说明 |
| --- | --- | --- |
| `window/label/button/text` 真实 backend 映射 | 已完成 | 已映射到真实 `LingDongGUI` 控件对象 |
| `checkbox/switch/slider` 真实 backend 映射 | 已完成 | 已接入真实值同步与 native event |
| `image` 对象映射与 source 绑定 | 已完成 | 已可把 `picoui_image_source` 绑定到底层 `ldImage` |
| `image` 的 theme/style apply | 当前拒绝 | 当前明确拒绝 `PICOUI_PART_MAIN`，不能写成默认支持 |
| `flex/grid` 布局映射 | 已完成 | 已映射到底层 layout 语义 |
| `theme/state/part/style` | 部分完成 | `window/button/checkbox/switch/slider/label/text` 已有真实 apply，`image` 仍拒绝 |
| runtime/capture | smoke 级证据 | 只能证明 build、启动、capture、回归，不等于 backend 最终闭环 |
| `backend_app.c` | temporary smoke path | 当前仍是 host runtime/fallback/capture 过渡层，不是正式渲染内核 |

### 当前口径提醒

- `PicoUI` 当前主线已收口为“真实 backend 映射”，不是继续扩写 SDL 假渲染器
- runtime/capture 测试只提供 smoke / 回归证据
- `backend_app.c` 当前仍是 `temporary smoke path` / host harness，但已不再承担正式 fake renderer 主输出职责

## Contact Information

🐧 QQ Group：187033407

📧 E-Mail: 59935554@qq.com
