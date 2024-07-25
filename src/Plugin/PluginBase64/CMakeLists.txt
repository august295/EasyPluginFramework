# Qt 库
set(QT_LIBRARY_LIST "Core" "Widgets")
# 链接库
set(SELF_LIBRARY_LIST "Manager")
# 预编译
add_compile_definitions(PLUGINBASE64_EXPORTS)

# 创建项目
CreateTarget("PluginBase64" "Dll" "Plugin")
