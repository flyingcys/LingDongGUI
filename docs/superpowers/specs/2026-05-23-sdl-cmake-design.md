# SDL 示例 CMake 迁移设计

## 背景

`examples/sdl` 当前同时维护 `makefile`、`qt_sdl.pro` 和 README 中的多套使用说明，构建入口分散。用户希望该目录只保留 CMake 一套编译方式，不再维护 Makefile。

## 目标

将 `examples/sdl` 改造成单一 CMake 构建入口，满足以下要求：

1. `examples/sdl` 新增唯一正式入口 `CMakeLists.txt`。
2. 删除旧的 `makefile`，不再提供 Make 构建方式。
3. 不再把 `qt_sdl.pro` 作为正式构建入口，避免多套入口并存造成维护歧义。
4. 保留现有 SDL 示例的源码组织、编译宏、优化参数和 demo 切换能力。
5. 保留跨平台行为：
   - Windows 继续使用仓库自带 `sdl2/32`、`sdl2/64`。
   - Linux 继续使用系统安装的 SDL2（`pkg-config`）。
6. Windows 构建后继续自动复制 `SDL2.dll` 到输出目录。
7. README 改为只描述 CMake 的依赖准备与构建方式。

## 范围

### 范围内

- `examples/sdl/CMakeLists.txt`
- `examples/sdl/README.md`
- 删除 `examples/sdl/makefile`
- 删除 `examples/sdl/qt_sdl.pro`
- 保留并复用现有 `user/`、`virtualNor/`、`sdl2/`、`../common/`、`../../src/` 下源码

### 范围外

- 其他 `examples/*` 目录的构建系统
- SDL 示例功能逻辑、UI 逻辑、demo 内容
- 仓库根级构建系统整合
- VSCode workspace 文件改造

## 约束与兼容性要求

### 构建入口

- 用户进入 `examples/sdl` 后，应只需要通过 CMake 完成配置、编译、运行。
- 不再要求用户依赖 Qt Creator 或 `mingw32-make`。

### 平台策略

#### Windows

- 使用 `CMAKE_SIZEOF_VOID_P` 区分 32/64 位。
- 32 位使用 `sdl2/32`，64 位使用 `sdl2/64`。
- 显式设置 SDL2 头文件目录、库目录和 DLL 路径。
- 构建后自动复制 `SDL2.dll` 到目标输出目录。

#### Linux

- 通过 `find_package(PkgConfig REQUIRED)` + `pkg_check_modules(SDL2 REQUIRED sdl2)` 获取 SDL2。
- 复用 `pkg-config` 提供的头文件和链接参数。

## CMake 设计

### 目标名

- 生成单一可执行文件目标，例如 `ldgui_sdl_demo`。

### demo 切换

- 通过 `set(USE_DEMO 2 CACHE STRING ...)` 暴露 demo 选项。
- 初始支持现有 Makefile 中真正纳入的 demo 集：
  - `1` Startup
  - `2` Show all widget
  - `3` Printer
- 若传入其他值，配置阶段直接报错，避免产物不完整。

### 源文件组织

- 复用现有 Makefile 的 source glob 范围。
- 公共源码、头文件目录、demo 专属源码按模块分组，避免所有内容堆在单个长列表里。
- 与现有构建保持一致：继续编译 `Arm-2D`、`math`、`../../src/gui`、`../../src/misc` 及 demo 对应目录。

### 编译定义与参数

- 保留现有宏定义：
  - `USE_DEMO`
  - `__ARM_2D_USER_APP_CFG_H__`
  - `ARM_SECTION(x)=`
  - `__va_list=va_list`
  - `RTE_Acceleration_Arm_2D_Helper_Disp_Adapter0`
  - `RTE_Acceleration_Arm_2D_Alpha_Blending`
- 统一保留现有优化/兼容参数：
  - `-std=gnu11`
  - `-ffunction-sections`
  - `-fdata-sections`
  - `-fno-ms-extensions`
  - `-Wno-macro-redefined`
  - `-Ofast`
  - `-flto`
- `-lpthread` 继续保留在非 Windows/Windows MinGW 可用场景下的链接行为；CMake 中以平台条件表达，避免无意义硬编码。

## 文档设计

README 调整为中文 CMake 使用说明，结构如下：

1. 构建依赖
   - Windows: CMake + MinGW（或兼容 gcc）
   - Linux: CMake + gcc + pkg-config + SDL2 开发包
2. 获取源码
3. 配置 demo 方式
   - 通过 `-DUSE_DEMO=<n>`
4. 配置与编译命令
5. 运行说明
6. Windows / Linux 差异说明

## 验证方案

至少完成以下验证：

1. `cmake -S examples/sdl -B <build-dir>` 可以成功配置。
2. `cmake --build <build-dir>` 可以成功编译。
3. Windows 分支具备 DLL 复制逻辑；若当前环境非 Windows，至少检查生成规则存在。
4. README 中命令与实际文件名一致，不再引用 `makefile` 或 `qt_sdl.pro` 作为正式入口。

## 风险与处理

### 风险 1：源文件 glob 与旧工程不一致

处理：优先以现有 `makefile` 为真值，必要时参考 `qt_sdl.pro` 补充头文件路径，不额外扩展 demo 范围。

### 风险 2：平台差异导致 SDL2 链接参数不一致

处理：Windows 使用仓库自带 SDL2 路径；Linux 严格通过 `pkg-config` 注入编译/链接参数。

### 风险 3：删除旧入口后 README 仍残留旧说明

处理：README 全文切换为 CMake 方案，并在验证时搜索 `makefile`、`qt_sdl.pro`、`mingw32-make` 等旧入口关键字。
