[TOC]

# 简易插件框架




## 1. 项目介绍

### 1.1. 基础

该项目基于 `Qt` 实现一个简易的插件框架。

### 1.2. 功能

- 通过编写动态库插件添加功能。



## 2. 项目构建

```
git clone --recursive git@github.com:august295/EasyPluginFramework.git
```

### 2.1. 项目环境

- `C++11`（及以上）
- `Cmake3.15.0`（及以上）
- `Qt5`

### 2.2. 项目生成

使用 `CMake` 创建工程。如果使用非 `Qt Creator` 打开工程，需要设置 `cmake/module_qt.cmake` 中 `qt` 的路径。

```
# 默认 64 位
cmake -B"build" -G"Visual Studio 17 2022"
# 指定 32 位
cmake -B"build" -G"Visual Studio 17 2022" -A"Win32" -D"QT_PATH=C:\\Qt\\5.15.2\\msvc2019"
```



## 3. 项目架构

### 3.1. CMake 文件

`CMakeLists.txt` 生成项目所需的自定义模块。

### 3.2. 头文件

`IPlugin.hpp` 是插件抽象类，插件通过继承该类实例化。

`TSingleton.hpp` 是单例类模板。奇异递归模板模式(Curiously Recurring Template Pattern，CRTP)：把派生类作为基类的模板参数。

`Library.hpp` 动态库加载宏，方便跨平台。

`TSingleton.hpp` `C++11` 线程池[^1]，用于进行多线程数据转发。

### 3.3. 资源文件

`resources` 放置所有资源文件。

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

