# 引入外部函数
include(${ROOT_DIR}/module.cmake)

# 获取文件名称
set(AbsolutePathProject ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(ProjectName ${AbsolutePathProject} NAME)
# 链接库
set(PROTOBUF TRUE)

# 创建项目
file(GLOB ProtoFiles "${AbsolutePathProject}/*.proto")
CreateTarget(${ProjectName} "Dll" "Proto")
