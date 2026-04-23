[TOC]

# EasyPluginFramework

一个基于 `Qt Widgets` 的轻量级插件框架示例项目。  
项目通过动态库加载插件，在主程序中自动发现、初始化并挂载插件界面，同时提供一个独立的事件总线组件用于插件间消息分发。

## 项目特点

- 使用 `Qt` 构建桌面主程序界面
- 使用动态库形式扩展插件能力
- 通过 `IPlugin` 统一插件生命周期和展示入口
- 通过 `EventBus` 实现插件与主程序之间的事件通信
- 使用 `CMake` 管理整个工程
- 内置多个示例插件，便于继续扩展

## 开发环境

当前工程已使用 `C++17`。

- `C++17`
- `CMake >= 3.15`
- `Qt5`
- Visual Studio 2022 或其他支持 CMake 的编译器
- `vcpkg`，用于安装 `OpenSSL`、`SQLite3` 等依赖

## 构建方式

### 1. 获取源码

```bash
git clone --recursive git@github.com:august295/EasyPluginFramework.git
cd EasyPluginFramework
```

### 2. 配置工程

如果不是使用 `Qt Creator` 打开工程，需要确认 `cmake/module_qt.cmake` 中的 Qt 路径配置正确。

```bash
cmake -B"build" -G"Visual Studio 17 2022"
```

### 3. 编译

```bash
cmake --build .\build\ --config Release -j 4
```

编译完成后：

- 主程序输出到 `build/bin`
- 插件动态库也输出到 `build/bin`
- 资源文件会从 `resources/` 复制到输出目录

## 目录结构说明

### 根目录

- `CMakeLists.txt`：项目顶层构建入口，配置编译标准、输出目录和子模块
- `cmake/`：CMake 辅助模块，包括 Qt 与 vcpkg 相关配置
- `resources/`：运行时资源目录，当前主要包含插件配置文件
- `rules/`：仓库内使用的代码规范文件
- `3rdparty/`：第三方源码或依赖子模块
- `src/`：项目核心源码

### `resources/`

- `resources/configs/plugins.xml`：插件加载配置文件
  - 定义哪些插件需要加载
  - 主程序启动时由 `PluginManager` 读取
  - 修改后可控制插件默认启用状态

### `src/`

`src/CMakeLists.txt`` 负责组织三个主要模块：

- `src/Core/EventBus`：事件总线动态库
- `src/Plugin`：各个插件动态库
- `src/Widget`：主程序可执行文件

## 模块解析

### 1. `src/Common`

公共头文件和基础设施，供主程序与插件复用。

- `src/Common/IPlugin.hpp`：插件接口定义
  - 约定插件生命周期：`Init`、`InitAppFinish`、`Release`
  - 约定插件元数据：名称、版本、描述、图标、显示位置
  - 约定界面展示入口：`WidgetShow`
- `src/Common/Library.hpp`：跨平台动态库加载工具
  - 用于加载插件动态库
  - 用于获取导出函数地址
- `src/Common/Utils.hpp`：通用辅助函数
  - 当前主要提供编码转换和换行裁剪功能
- `src/Common/TSingleton.hpp`：单例模板
- `src/Common/ThreadPool.hpp`：线程池实现

### 2. `src/Core/EventBus`

事件总线模块，单独编译为动态库。

- `src/Core/EventBus/IEvent.hpp`：事件类型定义
- `src/Core/EventBus/IEventHandler.hpp`：事件处理接口
- `src/Core/EventBus/IEventBus.hpp`：事件总线接口
- `src/Core/EventBus/EventBus.cpp`：事件总线实现

作用说明：

- 插件可以订阅某个主题
- 插件或主程序可以向总线发布事件
- 事件按照优先级进入队列，由后台线程分发

### 3. `src/Plugin`

示例插件集合，每个子目录基本都是一个独立动态库。

#### `src/Plugin/PluginBase64`

Base64 编解码插件，带独立界面和资源文件。

- `PluginBase64.cpp/.h`：插件实现
- `Base64Helper.cpp/.h`：Base64 相关辅助功能
- `PluginBase64.ui`：Qt Designer 界面
- `resources.qrc`、`icons/`：插件资源

#### `src/Plugin/PluginCodeConvert`

文本编码转换插件，提供字符编码转换相关界面功能。

- `PluginCodeConvert.cpp/.h`：插件实现
- `PluginCodeConvert.ui`：Qt 界面定义
- `resources.qrc`、`icons/`：插件资源

#### `src/Plugin/PluginTest`

测试/示例插件，用于演示最基础的插件接入方式和事件处理流程。

- `PluginTest.cpp/.h`：插件实现
- 不依赖独立 UI 文件，更适合作为新插件模板参考

### 4. `src/Widget`

主程序模块，最终生成桌面应用程序。

- `src/Widget/main.cpp`：程序入口
- `src/Widget/MainWindow.cpp/.h/.ui`：主窗口
- `src/Widget/PluginWidget.cpp/.h/.ui`：插件列表与状态展示界面
- `src/Widget/Subscribe.cpp/.h`：日志订阅与 UI 转发
- `src/Widget/Manager/PluginManager.cpp/.h`：插件配置读取、动态库加载与插件实例管理
- `src/Widget/Manager/LoggerManager.cpp/.h`：日志管理
- `src/Widget/Manager/LoggerSQLSink.h`：SQLite 日志输出
- `src/Widget/CertView/`：证书查看功能页面及 OpenSSL 相关辅助实现

主程序职责：

- 读取插件配置
- 动态加载插件库
- 初始化插件
- 将插件挂载到菜单
- 显示日志输出和内置工具页面

## 插件加载流程

结合当前代码，主程序的大致流程如下：

1. `MainWindow` 启动
2. `PluginManager` 读取 `resources/configs/plugins.xml`
3. 根据插件名称拼接动态库文件名
4. 使用 `Library.hpp` 中的工具加载动态库
5. 查找导出函数 `CreatePlugin`
6. 创建插件实例并调用：
   - `Init()`
   - `InitAppFinish()`
7. 根据 `PluginLocation` 将插件挂载到菜单
8. 用户点击菜单后调用插件的 `WidgetShow()`

## 如何新增一个插件

可以参考 `src/Plugin/PluginTest` 或 `src/Plugin/PluginBase64`。

建议步骤：

1. 在 `src/Plugin/` 下新建插件目录
2. 实现一个继承 `IPlugin` 的类
3. 如需接收事件，同时实现 `IEventHandler`
4. 导出统一入口函数 `CreatePlugin`
5. 添加对应的 `CMakeLists.txt`
6. 在 `src/Plugin/CMakeLists.txt` 中加入子目录
7. 在 `resources/configs/plugins.xml` 中配置插件名和是否默认加载

## 配置文件说明

`resources/configs/plugins.xml` 负责声明插件列表，例如：

```xml
<Plugins>
    <Test>
        <PluginTest load="true" />
        <PluginCodeConvert load="true" />
        <PluginBase64 load="true" />
    </Test>
</Plugins>
```

字段含义：

- 分组节点如 `Test`：插件分组
- 子节点名称如 `PluginBase64`：插件动态库基础名称
- `load="true"`：启动时默认加载

## 当前代码现状说明

- 项目当前使用 `C++17`
- 自定义 `Any.hpp` 已移除，后续如需“任意类型容器”应直接使用标准库 `std::any`
- `EventBus` 已独立为动态库模块
- 证书查看功能目前位于主程序内置页面 `src/Widget/CertView/`
- `resources/configs/plugins.xml` 中出现了 `PluginCertView`，但源码目录中并没有对应的插件子目录；如果启动时需要完全一致，建议同步清理或补齐配置

## 适合继续完善的方向

- 增加统一的插件模板
- 为插件增加版本兼容性检查
- 为事件总线增加更丰富的事件类型
- 增加单元测试和集成测试
- 补充 Linux 平台构建说明

## 参考

- `Qt Widgets`
- `CMake`
- `OpenSSL`
- `SQLite3`
- `spdlog`
- `pugixml`
