# 引入外部函数
include(${ROOT_DIR}/cmake/module.cmake)

# 获取文件名称
set(AbsolutePathProject ${CMAKE_CURRENT_SOURCE_DIR})

# 创建项目
file(GLOB ProtoFiles "${AbsolutePathProject}/*.proto")
CreateProto("ProtoTest" "Dll" "Proto")
