# 引入外部函数
include(${ROOT_DIR}/cmake/module.cmake)

# 预编译
add_compile_definitions(MANAGER_EXPORTS)
set(LibraryHeaderOnlyList "pugixml" "spdlog")

# 创建项目
CreateTarget("Manager" "Dll" "Core")
