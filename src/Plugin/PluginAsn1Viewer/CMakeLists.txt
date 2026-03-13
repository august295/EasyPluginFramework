# Qt 库
set(QT_LIBRARY_LIST "Core" "Widgets")
# 链接库
set(SELF_LIBRARY_LIST "EventBus" "endecode")
# 预编译
add_compile_definitions(PLUGINASN1VIEWER_EXPORTS)

# 创建项目
CreateTarget("PluginAsn1Viewer" "Dll" "Plugin")

target_include_directories(PluginAsn1Viewer PRIVATE ${ROOT_DIR}/3rdparty/endecode/include)
