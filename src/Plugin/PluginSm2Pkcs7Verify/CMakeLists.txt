# Qt 库
set(QT_LIBRARY_LIST "Core" "Widgets")
# 链接库
set(SELF_LIBRARY_LIST "EventBus" "endecode")
# 预编译
add_compile_definitions(PLUGINSM2PKCS7VERIFY_EXPORTS)

# 创建项目
set(ProjectName "PluginSm2Pkcs7Verify")
CreateTarget(${ProjectName} "Dll" "Plugin")

# 包含目录
target_include_directories(${ProjectName} PRIVATE ${ROOT_DIR}/3rdparty/endecode/include)

# 链接库
target_link_libraries(${ProjectName} PRIVATE endecode)
