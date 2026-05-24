# SDL Demo（CMake）

`examples/sdl` 现在只保留 CMake 作为正式构建入口。

## 依赖

### Windows

- CMake 3.16 或更高版本
- MinGW-w64 或其他兼容 GCC 的 C 编译器

Windows 构建会按 `CMAKE_SIZEOF_VOID_P` 自动选择 `sdl2/32` 或 `sdl2/64`，并在构建后自动复制对应的 `SDL2.dll` 到目标目录。

### Linux

- CMake 3.16 或更高版本
- GCC
- `pkg-config`
- SDL2 开发包

例如在 Debian/Ubuntu 上可安装：

```bash
sudo apt-get install build-essential cmake pkg-config libsdl2-dev
```

## 获取源码

推荐直接递归拉取子模块：

```bash
git clone --recursive https://github.com/gzbkey/LingDongGUI.git
```

如需使用 Gitee：

```bash
git clone --recursive https://gitee.com/gzbkey/LingDongGUI.git
```

## 选择 Demo

通过 `-DUSE_DEMO=<n>` 选择运行的界面：

- `0`：Legacy show all widget（单页全控件）
- `1`：Startup
- `2`：Show all widget
- `3`：Printer
- `4`：Layout（flex wrap/grow/new-track/ignore-layout demo）
- `5`：Grid demo

传入其他值会在 CMake 配置阶段直接报错。

### `USE_DEMO=4` 布局演示说明

- 左上窗口使用 `ldFlexFlowRowWrap`，并通过 `ldWindowSetFlexGap(6, 4)` 区分同一行卡片间距与不同行之间的 track 间距。
- 同一窗口还会调用 `ldWindowSetFlexTrackAlign(..., ldFlexTrackAlignCenter)`，让多行内容在交叉轴方向保持居中。
- 标记为 `D*` 的卡片通过 `ldBaseSetFlexNewTrack(..., true)` 强制从新的一行开始，便于观察 new-track 的效果。
- 右上窗口演示纵向 flex：`1x` / `2x` 分别通过 `ldBaseSetFlexGrow(..., 1)` 和 `ldBaseSetFlexGrow(..., 2)` 按剩余空间的 1:2 比例扩展。
- 名为 `free` 的卡片调用 `ldBaseSetIgnoreLayout(..., true)` 后脱离布局槽位，再用 `ldBaseMove` 手动摆放，所以会悬浮在列布局旁边。
- Demo 运行后左侧 flex 窗口会每 1200ms 在宽版与窄版之间切换，便于直接观察 wrap 后新增 track 的排布变化。

## 配置与编译

进入 `examples/sdl` 后直接执行：

```bash
cd examples/sdl
cmake -S . -B build -DUSE_DEMO=2
cmake --build build
```

如果希望使用其他构建目录，可以替换 `-B` 后面的路径，例如 `-B ../../build/sdl-test`。

## 运行

单配置生成器下，可执行文件默认位于构建目录根部：

```bash
./build/ldgui_sdl_demo
```

Windows 下一般为：

```bash
build\ldgui_sdl_demo.exe
```

## 平台差异

- Windows：使用仓库内自带的 SDL2 头文件、库文件和 `SDL2.dll`。
- Linux：通过 `pkg-config --cflags --libs sdl2` 提供 SDL2 的头文件和链接参数。
