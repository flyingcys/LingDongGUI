# Task 1 报告：编译 ldgui_sdl_demo（USE_DEMO=0）

## 目标

为 LingDongGUI SDL demo 构建一个使用 USE_DEMO=0（legacy widget demo）的可执行文件，作为后续视觉对比的基准。

## 执行步骤

### Step 1：编译 ldgui_sdl_demo（USE_DEMO=0）

**命令：**
```bash
cd /home/share/samba/flyingcys/LingDongGUI
cmake -S examples/sdl -B build-legacy0 -DUSE_DEMO=0
cmake --build build-legacy0 -j8 --target ldgui_sdl_demo
```

**输出（关键部分）：**
```
-- The C compiler identification is GNU 11.4.0
-- Found PkgConfig: /usr/bin/pkg-config (found version "0.29.2") 
-- Checking for module 'sdl2'
--   Found sdl2, version 2.0.20
-- Found Threads: TRUE  
-- Configuring done
-- Generating done
-- Build files have been written to: /home/share/samba/flyingcys/LingDongGUI/build-legacy0
[100%] Linking C executable ldgui_sdl_demo
[100%] Built target ldgui_sdl_demo
```

**结果：** SUCCESS - 编译完成，无 error

### Step 2：验证可执行文件存在

**命令：**
```bash
ls -la build-legacy0/ldgui_sdl_demo
```

**输出：**
```
-rwxrwxr-x 1 cys cys 3.0M  6月 30 11:11 /home/share/samba/flyingcys/LingDongGUI/build-legacy0/ldgui_sdl_demo
```

**验证：**
```
ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked
with debug_info, not stripped
```

**结果：** SUCCESS - 文件存在，大小 3.0M > 0

### Step 3：确认 tinyui_demo 可用

**状态：** tinyui_demo 存在链接错误

tinyui_demo 目标有来自以下符号的未定义引用：
- `tinyui_backend_init`（位于 `tinyui/port/sdl/step.c:223`）
- `tinyui_backend_step`（位于 `tinyui/port/sdl/step.c:126`）

这些错误不在 Task 1 的范围内（Task 1 只关注 ldgui_sdl_demo），但在后续任务中可能需要修复。

## 构建配置

- **CMake 版本：** 3.16+
- **C 编译器：** GNU 11.4.0
- **编译标志：** `-Ofast -flto -std=gnu11 -g`
- **链接选项：** `-Ofast -flto`
- **依赖库：** SDL2 2.0.20, Threads, Arm-2D

## 预期输出验证

- [x] CMake 配置成功
- [x] 编译输出最后包含 `[100%]` 和 `Built target ldgui_sdl_demo`
- [x] 无 error（仅有编译器 warning，不影响链接）
- [x] 可执行文件存在：`/home/share/samba/flyingcys/LingDongGUI/build-legacy0/ldgui_sdl_demo`
- [x] 可执行文件大小 > 0（实际 3.0M）

## 总结

**Task 1 状态：DONE**

ldgui_sdl_demo（USE_DEMO=0）已成功编译，生成的可执行文件位于：
```
/home/share/samba/flyingcys/LingDongGUI/build-legacy0/ldgui_sdl_demo (3.0M)
```

该可执行文件已准备好作为后续任务的视觉对比基准。
