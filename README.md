<h4>
  <a>中文</a> |
  <a href="README.en.md">English</a>
</h4>

<h1 align="center" style="margin: 30px 0 30px; font-weight: bold;">灵动GUI</h1>
<h4 align="center">一个基于ARM-2D的GUI</h4>
<p align="center">
	<a href="https://gitee.com/gzbkey/LingDongGUI/stargazers"><img src="https://gitee.com/gzbkey/LingDongGUI/badge/star.svg"></a>
	<a href="https://gitee.com/gzbkey/LingDongGUI/members"><img src="https://gitee.com/gzbkey/LingDongGUI/badge/fork.svg"></a>
	<a><img src="https://img.shields.io/github/license/gzbkey/LingDongGUI"></a>
	<a><img src="https://img.shields.io/badge/architecture-ARM%20|%20RISC%20V-blue"></a>	
</p>

## 特性

1. **各种芯片通用**
    * 支持ARM单片机、RISC-V单片机
    * 支持在PC上模拟运行(SDL)

2. **简单易用**
    * 对ARM-2D进行二次封装，极大降低ARM-2D的使用难度
    * 事件触发使用信号槽模式，轻松实现多对多触发
    * 界面代码和功能代码分离，逻辑清晰

3. **支持ARM-2D原生API**
    * 可使用ARM-2D原生API进行开发
    * LDGUI和ARM-2D混合编程，灵活性高

4. **动态自动裁剪**
    * 无需宏定义裁剪，自适应最小化体积

5. **自由扩展控件**
    * 使用脚本自动生成新控件框架，专注实现显示效果

6. **内置便捷功能**
    * 软件定时器
    * 多功能按键，支持实体按键和触摸按键
    * 队列
    * 彩色log打印

7. **上位机**
    * 配套免费的通用上位机

## 源码

🏠️Gitee主仓库: https://gitee.com/gzbkey/LingDongGUI

🏠️Github镜像仓库: https://github.com/gzbkey/LingDongGUI

## 上位机

🚀上位机仓库: https://gitee.com/gzbkey/GuiEasyEditor

## 演示效果

widget demo sdl
<p>
	<img src="./docs/images/widget%20demo.gif" width="400" />
</p>

## 控件列表

| 状态 | 名称 | 说明 |
| :----:| ---- | ---- |
| ✅ | window | 窗体，多用于分层 |
| ✅ | image | 图片，用工具生成图片数据，支持png格式 |
| ✅ | button | 按键 |
| ✅ | text | 文本 |
| ✅ | progress bar | 进度条，支持图片移动形成动画效果 |
| ✅ | check box + radio button| 复选框 + 单选功能，支持自定义图片和文字显示 |
| ✅ | radia menu | 旋转菜单 |
| ✅ | label | 简单文本显示 |
| ✅ | slider | 滑动条 |
| ✅ | scroll selecter | 滚动选择器 |
| ✅ | date time | 日期和时间，数字时钟 |
| ✅ | icon slider | 滑动图标 |
| ✅ | gauge | 仪表盘 |
| ✅ | qr code | 二维码 |
| ✅ | table | 表格 |
| ✅ | keyboard | 键盘 |
| ✅ | line edit | 编辑框 |
| ✅ | graph | 波形图 |
| ✅ | combo box | 下拉框 |
| ✅ | arc bar | 圆环进度条 |
| ✅ | animation | gif动画 |
| 🔲 | calendar | 日历 |
| ✅ | message box | 消息框 |
| 🔲 | clock | 模拟时钟 |
| ✅ | list | 列表，可嵌入其他类型控件 |
| 🔲 | time picker | 时间选择器 |
| 🔲 | date picker | 日期选择器 |

## 文件结构
|名称|类型|说明|
|---|---|---|
|docs|文件夹|详细说明文档|
|examples|文件夹|例子项目所在文件夹|
|src|文件夹|源码文件|
|tools|文件夹|配套使用的相关工具|

## 教程

📖 [中文教程](https://ldgui-doc-cn.readthedocs.io)

## 技术交流

🐧 QQ群：187033407

📧 电子邮箱: 59935554@qq.com



