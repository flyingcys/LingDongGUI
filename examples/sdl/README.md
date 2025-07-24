# Qt SDL Demo

## 软件准备

### Qt

当前项目使用Qt 5.14.2(32bit 64bit)编译通过

https://download.qt.io/archive/qt/5.14/5.14.2/qt-opensource-windows-x86-5.14.2.exe

Qt 5.14.2为官方离线安装版的最后一个版本。安装该软件时，***电脑断网***则无需登录即可完成安装。

如果出现Download from your IP address is not allowed，可以到QQ群文件中下载。

### git

安装git，用于下载源码。(也可以手动下载ldgui源码，但不推荐)

https://git-scm.com/download/win

## 下载源码

### 方法1

##### 步骤1
//gitee

git clone https://gitee.com/gzbkey/LingDongGUI.git

//github 

git clone https://github.com/gzbkey/LingDongGUI.git

##### 步骤2

git submodule init

git submodule update

### 方法2

//gitee

git clone --recursive https://gitee.com/gzbkey/LingDongGUI.git

//github

git clone --recursive https://github.com/gzbkey/LingDongGUI.git

## Qt SDL Demo的使用

1. 双击项目qt_sdl.pro，启动Qt。

2. 项目打开后，会自动复制SDL2的dll文件到对应的文件内，无需手动操作。

3. 打开qt_sdl.pro文件，修改宏定义USE_DEMO为对应demo序号。

4. 直接点击左下角三角形，或CTRL+R，进行编译、运行。

# VSCode SDL Demo

## 软件准备

### VSCode

https://code.visualstudio.com/Download

安装插件c/c++、Code Runner

### git

安装git，用于下载源码。(也可以手动下载ldgui源码，但不推荐)

https://git-scm.com/download/win

### 安装 gcc

我使用的是MinGW-w64

https://www.mingw-w64.org/

然后在这网站，找到这里

https://github.com/niXman/mingw-builds-binaries/releases/

i686是32位

x86_64是64位

选择参考

i686-15.1.0-release-posix-dwarf-ucrt-rt_v12-rev0.7z

x86_64-15.1.0-release-posix-seh-ucrt-rt_v12-rev0.7z

demo默认gcc安装位置为：

D:\mingw32

D:\mingw64

根据自己需求选择即可

## 下载源码

### 方法1

##### 步骤1
//gitee

git clone https://gitee.com/gzbkey/LingDongGUI.git

//github 

git clone https://github.com/gzbkey/LingDongGUI.git

##### 步骤2

git submodule init

git submodule update

### 方法2

//gitee

git clone --recursive https://gitee.com/gzbkey/LingDongGUI.git

//github

git clone --recursive https://github.com/gzbkey/LingDongGUI.git

## VSCode SDL Demo 的使用

### 修改配置

gcc安装位置，如果使用以上的默认位置，则跳过本步骤

1. 通过vscode打开项目文件夹

2. 修改对应版本的compilerPath

    .vscode\c_cpp_properties.json

3. 修改对应版本的miDebuggerPath

    .vscode\launch.json

4. 修改对应版本的command

    .vscode\tasks.json

### 编译

1. 打开ldConfig.h文件，修改宏定义USE_DEMO为对应demo序号。

2. 点击左侧按键"运行和调试"，选择对应编译器版本，按F5编译即可。

3. 项目编译前，会自动复制SDL2的dll文件到对应的文件内，无需手动操作。

# Makefile SDL Demo

## 软件准备

### 安装 gcc

我使用的是MinGW-w64

https://www.mingw-w64.org/

然后在这网站，找到这里

https://github.com/niXman/mingw-builds-binaries/releases/

i686是32位

x86_64是64位

选择参考

i686-15.1.0-release-posix-dwarf-ucrt-rt_v12-rev0.7z

x86_64-15.1.0-release-posix-seh-ucrt-rt_v12-rev0.7z

demo默认gcc安装位置为：

D:\mingw32

D:\mingw64

根据自己需求选择即可

将gcc安装位置添加到环境变量中

### 编译

进入makefile目录下，执行mingw32-make即可进行编译
