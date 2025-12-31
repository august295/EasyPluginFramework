# 引入外部函数
include(${ROOT_DIR}/cmake/module.cmake)

# 预编译
add_compile_definitions(EVENTBUS_EXPORTS)

# 创建项目
CreateTarget("EventBus" "Dll" "Core")
