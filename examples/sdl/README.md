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

- `1`：Startup
- `2`：Show all widget
- `3`：Printer

传入其他值会在 CMake 配置阶段直接报错。

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
build\\ldgui_sdl_demo.exe
```

## 平台差异

- Windows：使用仓库内自带的 SDL2 头文件、库文件和 `SDL2.dll`。
- Linux：通过 `pkg-config --cflags --libs sdl2` 提供 SDL2 的头文件和链接参数。
