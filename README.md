[TOC]

# 简易插件框架




## 1. 项目介绍

### 1.1. 基础

该项目基于 `Qt` 实现一个简易的插件框架。

### 1.2. 功能

- 通过编写动态库插件添加功能。




## 2. 项目构建

### 2.1. 项目环境

- `VS2017` （至少支持 C++17）
- `Cmake3.15.0`（及以上）
- `Qt5.14.2`

### 2.2. 项目生成

其中源代码在 `src` 中。

- 使用 `cmake_vs2017.bat` 创建 `VS2017` 工程
- 使用 `cmake-gui `加载 `CMakeLists.txt` 创建工程



## 3. 项目架构

| 标志                 | 说明       |
| -------------------- | ---------- |
| 以 `I` 开头文件      | 接口头文件 |
| 以 `T` 开头文件      | 模板头文件 |
| 以 `Global` 开头文件 | 导入导出宏 |
| 以 `hpp` 结尾文件    | 仅头文件   |
| 以 `Ptr` 结尾属性    | 原生指针   |
| 以 `Sptr` 结尾属性   | 智能指针   |

### 3.1. CMake 文件

`CMakeLists.txt` 生成项目所需的自定义模块。

### 3.2. 头文件

`IPlugin.hpp` 是插件抽象类，插件通过继承该类实例化。

`TSingleton.hpp` 是单例类模板。奇异递归模板模式(Curiously Recurring Template Pattern，CRTP)：把派生类作为基类的模板参数。

`Library.hpp` 动态库加载宏，方便跨平台。

`TSingleton.hpp` `C++11` 线程池[^1]，用于进行多线程数据转发。

### 3.3. 资源文件

`resources` 放置所有资源文件，

- 配置文件
- `qrc` 文件统一加载，防止找不到资源路径。

### 3.4. Core

#### 3.4.1. MainWidget

启动界面，后续可能多个可执行程序，需要改成动态库。

#### 3.4.2. Manager

`ConfigManager` 继承实现单例，配置管理。

`PluginManager` 管理插件的加载。

`DataManager` 继承实现单例，数据管理。

`Framework` 整体框架。

### 3.5. Plugin

插件。



# 参考

[^1]: [C++11线程池](https://github.com/progschj/ThreadPool)

