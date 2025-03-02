# Qt 库
set(QT_LIBRARY_LIST "Core" "Widgets")
# 链接库
set(SELF_LIBRARY_LIST "Manager")
# 三方库
include_directories(${ROOT_DIR}/3rdparty/endecode/include)

# 预编译
add_compile_definitions(PLUGINCERTVIEW_EXPORTS)
# 创建项目
set(ProjectName "PluginCertView")
CreateTarget(${ProjectName} "Dll" "Plugin")

# 链接库
target_link_libraries(${ProjectName} "endecode")
