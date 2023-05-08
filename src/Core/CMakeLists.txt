# 引入外部函数
include(${ROOT_DIR}/module.cmake)

# 获取绝对路径
set(AbsolutePathProject ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(ProjectName ${AbsolutePathProject} NAME)
# Qt 库
set(QT_LIBRARY_LIST "Core" "Gui" "Widgets")
# 链接库
# set(SELF_LIBRARY_LIST "ParseHeader" "Manager")

# 创建项目
CreateTarget(${ProjectName} "Exe" "Core")
