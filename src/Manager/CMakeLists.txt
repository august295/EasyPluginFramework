# 三方库
include_directories(${ROOT_DIR}/3rdparty/spdlog/include)
add_compile_definitions(PUGIXML_HEADER_ONLY)
include_directories(${ROOT_DIR}/3rdparty/pugixml/src)

# 预编译
add_compile_definitions(MANAGER_EXPORTS)

# 创建项目
CreateTarget("Manager" "Dll" "Manager")
