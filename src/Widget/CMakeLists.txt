# 引入外部函数
include(${ROOT_DIR}/cmake/module.cmake)

# 头文件
include_directories(${ROOT_DIR}/3rdparty/spdlog/include)
add_compile_definitions(PUGIXML_HEADER_ONLY)
include_directories(${ROOT_DIR}/3rdparty/pugixml/src)
include_directories(${ROOT_DIR}/3rdparty/filesystem/include)
include_directories(${ROOT_DIR}/3rdparty/cpptrace/include)

# Qt 库
set(QT_LIBRARY_LIST "Core" "Gui" "Widgets")
# 链接库
set(SELF_LIBRARY_LIST "EventBus")

# 创建项目
set(ProjectName "MainWidget")
CreateTarget(${ProjectName} "Exe" "Widget")

target_link_libraries(${ProjectName} PRIVATE OpenSSL::SSL)
target_link_libraries(${ProjectName} PRIVATE OpenSSL::Crypto)
target_link_libraries(${ProjectName} PRIVATE unofficial::sqlite3::sqlite3)
target_link_libraries(${ProjectName} PRIVATE PkgConfig::gmp)
target_link_libraries(${ProjectName} PRIVATE cpptrace-lib)
