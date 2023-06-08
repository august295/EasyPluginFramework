# 引入外部函数
include(${ROOT_DIR}/cmake/module.cmake)

# 获取文件名称
set(AbsolutePathProject ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(ProjectName ${AbsolutePathProject} NAME)

# 创建项目
file(GLOB ProtoFiles "${AbsolutePathProject}/*.proto")
CreateProto(${ProjectName} "Dll" "Proto")
