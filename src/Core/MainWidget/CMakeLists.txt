# 引入外部函数
include(${ROOT_DIR}/cmake/module.cmake)

# 获取绝对路径
set(AbsolutePathProject ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(ProjectName ${AbsolutePathProject} NAME)
# Qt 库
set(QT_LIBRARY_LIST "Core" "Gui" "Widgets")
# 链接库
set(SELF_LIBRARY_LIST "Manager" "PluginTest" "ProtoTest")
set(THIRD_LIBRARY_LIST "Protobuf")
# proto 文件添加符号导出，这里使用符号导入
add_compile_definitions(PROTO_EXPORT=__declspec\(dllimport\))

# 创建项目
CreateTarget(${ProjectName} "Exe" "Core")
