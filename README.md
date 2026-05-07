# 🎮本地游戏启动器

![C++](https://img.shields.io/badge/C++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Qt](https://img.shields.io/badge/Qt-%23217346.svg?style=for-the-badge&logo=Qt&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-blue.svg?style=for-the-badge)

一款基于 **Qt C++** 开发的现代化、轻量级本地游戏管理与启动工具。告别杂乱的桌面快捷方式，为你电脑里的本地游戏、独立游戏、视觉小说（Galgame）等提供一个优雅、沉浸式的统一展示与启动平台。

<img width="1024" height="632" alt="11c13873-b174-4ce1-8b5d-4eb6949c47b2" src="https://github.com/user-attachments/assets/63881c4e-fd1d-4621-b26b-ae1d815b5625" />
---
## ✨ 核心特性 (Features)

* 🚀 **全自动扫描与识别**
    * 只需将游戏文件夹放入指定的“游戏库”目录，软件即可自动读取并按文件夹命名生成游戏卡片。
    * **首次启动引导**：首次打开软件时会强制引导用户设置游戏存放路径，逻辑清晰，开箱即用。
* 🎨 **高度自定义的美化系统**
    * **全局背景**：支持自定义启动器的主背景图片，随心打造专属外观。
    * **独立游戏元数据**：点击单个游戏后，可自由替换游戏海报/封面、设置独立的游戏背景图。
    * **信息编辑**：支持自定义修改游戏名称，并可以为每个游戏编写专属的详细简介。
* 📈 **游玩数据统计**
    * 自动记录并展示每款游戏的 **“启动次数”**。
    * 精确记录 **“上次游玩时间”**，随时掌握你的游戏历程。
* 💎 **现代化 UI 体验**
    * 采用无边框窗口设计与现代化的卡片式网格布局。
    * 深度整合毛玻璃（亚克力）模糊特效，辅以平滑的侧边栏菜单和详情弹出面板，视觉体验极佳。

---

## 📸 界面展示 (Screenshots)

| 游戏库主界面 | 游戏详情与启动面板 |
| :---: | :---: |
| <img width="2277" height="1405" alt="" src="https://github.com/user-attachments/assets/23b41ca1-43d8-4792-b7c0-39e86ce36bed" />|<img width="1024" height="632" alt="11c13873-b174-4ce1-8b5d-4eb6949c47b2" src="https://github.com/user-attachments/assets/63881c4e-fd1d-4621-b26b-ae1d815b5625" />|
| *自动扫描并以网格排版展示所有游戏* | *右侧滑出的详情面板，展示统计数据并提供自定义选项* |

---

## 🛠️ 技术栈 (Tech Stack)

* **编程语言**: C++
* **GUI 框架**: Qt (基于 QWidget / QML)
* **构建工具**: CMake / qmake

---

## 🚀 快速开始 (Getting Started)
### 直接下载
* 直接下载release中的zip文件，解压并打开GameLauncher.exe并完成初始化配置。或
### 手动编译
#### 环境依赖
* Qt 5.15+ 或 Qt 6.x
* C++17 编译器 (MSVC / MinGW / GCC 等)
* CMake 3.16+
#### 编译
* 使用 Qt Creator 打开 CMakeLists.txt 文件进行编译
 ---
## 💡使用指南 (Usage)
* 添加游戏：在设定的库目录中新建文件夹并命名，将游戏核心文件放入其中即可。
* 设置路径：点击左侧边栏的 文件夹图标 可以随时重新配置游戏库路径、点击 重试图标 可刷新列表、点击 图片图标 可以设置全局背景。
* 个性化游戏：在主界面点击任意游戏卡片，右侧会呼出详情面板。点击 更换背景 可以更改该游戏的封面和内部背景，点击简介区域右上角编辑可编辑文字说明，点击游戏名右侧编辑可修改游戏名。
* 启动游戏：在详情面板点击 开始游戏 按钮，首次启动会扫描文件夹中的可执行文件，选择后即可启动，也可在右侧详细信息面板下方修改路径出修改，以后直接点击启动游戏即可直接启动游戏！
 ---
## 📄 许可证 (License)
使用Qt开发，遵循LGPL v3许可。
并遵循本项目采用 MIT License 协议。
