# Qt 库
set(QT_LIBRARY_LIST "Core" "Widgets")
# 链接库
set(SELF_LIBRARY_LIST "EventBus")
# 预编译
add_compile_definitions(PLUGINP7BCERTIFICATECHAIN_EXPORTS)

# 创建项目
set(ProjectName "PluginP7bCertificateChain")
CreateTarget(${ProjectName} "Dll" "Plugin")

target_include_directories(${ProjectName} PRIVATE ${ROOT_DIR}/3rdparty/endecode/include)

target_link_libraries(${ProjectName} PRIVATE endecode)
