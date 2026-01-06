# 引入外部函数
include(${ROOT_DIR}/cmake/module.cmake)

# 头文件
include_directories(${ROOT_DIR}/3rdparty/spdlog/include)
add_compile_definitions(PUGIXML_HEADER_ONLY)
include_directories(${ROOT_DIR}/3rdparty/pugixml/src)
include_directories(${ROOT_DIR}/3rdparty/sqlite)
include_directories(${ROOT_DIR}/3rdparty/filesystem/include)

# Qt 库
set(QT_LIBRARY_LIST "Core" "Gui" "Widgets")
# 链接库
set(SELF_LIBRARY_LIST "EventBus")
# 链接库
set(LibraryList "openssl")
AddLibrary("${LibraryList}")

# 创建项目
set(ProjectName "MainWidget")
CreateTarget(${ProjectName} "Exe" "Widget")

target_link_libraries(${ProjectName} "libssl" "libcrypto")
target_link_libraries(${ProjectName} "sqlite")
