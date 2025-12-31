# 引入外部函数
include(${ROOT_DIR}/cmake/module.cmake)

# Qt 库
set(QT_LIBRARY_LIST "Core")
# 链接库
set(SELF_LIBRARY_LIST "EventBus")
# 预编译
add_compile_definitions(PLUGINTEST_EXPORTS)

# 创建项目
CreateTarget("PluginTest" "Dll" "Plugin")
