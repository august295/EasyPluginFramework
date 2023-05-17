# 引入外部函数
include(${ROOT_DIR}/module.cmake)

# 获取文件名称
set(AbsolutePathProject ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(ProjectName ${AbsolutePathProject} NAME)
# Qt 库
set(QT_LIBRARY_LIST "Core")
# 链接库
set(SELF_LIBRARY_LIST "Manager" "ProtoTest")
# 预编译
add_compile_definitions(PLUGINTEST_EXPORTS)
# proto 文件添加符号导出，这里使用符号导入
add_compile_definitions(PROTO_EXPORT=__declspec\(dllimport\))

# 创建项目
CreateTarget(${ProjectName} "Dll" "Plugin")
