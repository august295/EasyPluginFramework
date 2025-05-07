# 引入外部函数
include(${ROOT_DIR}/cmake/module.cmake)

# Qt 库
set(QT_LIBRARY_LIST "Core" "Gui" "Widgets")
# 链接库
set(SELF_LIBRARY_LIST "Manager")
set(LibraryList "openssl")
AddLibrary("${LibraryList}")

# 创建项目
set(ProjectName "MainWidget")
CreateTarget(${ProjectName} "Exe" "Widget")

target_link_libraries(${ProjectName} "libssl" "libcrypto")