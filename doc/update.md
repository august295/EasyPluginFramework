# 修改记录



## 1. 20230609

- 优化目录结构
- 优化插件加载界面，实现控制加载插件



## 2. 20230604

- 添加跨平台获取动态库全名
- 添加统一日志管理器
- 创建 `Qt`，添加插件加载界面



## 3. 20230517

- 解决动态加载 `proto` 文件出现重复加载问题，将 `proto` 文件编译成动态库，因为 `proto` 自动生成的 `pb.h` `pb.cc` 加载会自动注册数据类型到 `protobuf` 数据池中且是全局的，导致冲突。
- 添加注释和文档。



## 4. 20230514

- 创建数据总线模式，进行数据转发
- 引入线程池 `ThreadPool`（head-only）
- 引入 `protobuf`，为了实现动态插件加载，`protobuf`是以动态库的形式编译的



## 5. 20230510

- 创建数据管理单例类
- 添加日志功能，引入日志库 `spdlog` （head-only）
- 读取插件配置文件 `XML`，引入解析库 `pugixml` （head-only），动态加载插件



## 6. 20230508

- 创建工程，编写文档
- 搭建基本插件框架，实现加载动态库