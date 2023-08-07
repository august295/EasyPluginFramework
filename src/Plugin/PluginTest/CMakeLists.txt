# 引入外部函数
include(${ROOT_DIR}/cmake/module.cmake)

# Qt 库
set(QT_LIBRARY_LIST "Core")
# 链接库
set(SELF_LIBRARY_LIST "Manager" "ProtoTest")
set(THIRD_LIBRARY_LIST "Protobuf")
# 预编译
add_compile_definitions(PLUGINTEST_EXPORTS)
# proto 文件添加符号导出，这里使用符号导入
add_compile_definitions(PROTO_EXPORT=__declspec\(dllimport\))

# 创建项目
CreateTarget("PluginTest" "Dll" "Plugin")
