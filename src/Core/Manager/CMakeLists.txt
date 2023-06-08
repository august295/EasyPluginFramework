# 引入外部函数
include(${ROOT_DIR}/cmake/module.cmake)

# 获取绝对路径
set(AbsolutePathProject ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(ProjectName ${AbsolutePathProject} NAME)
# 预编译
add_compile_definitions(MANAGER_EXPORTS)

# 创建项目
CreateTarget(${ProjectName} "Dll" "Core")
