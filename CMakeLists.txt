# 要求最低Cmake版本
cmake_minimum_required(VERSION 3.15.0)

include(cmake/module_vcpkg.cmake)

# 项目名称
set(UseProjectName "EasyPluginFramework")
project(${UseProjectName})

# C++标准
set(CMAKE_CXX_STANDARD 11)

# 条件编译
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options("/utf-8")
elseif()
endif()

# 开启使用文件夹功能，将所有默认目标放入到名为 CMakePredefinedTargets 的文件夹中
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakePredefinedTargets")

# 设置根目录
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_DIR ${CMAKE_BINARY_DIR})

# 默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# 为多种构建类型设置输出目录
set(CONFIGURATION_TYPES "Debug" "Release" "MinSizeRel" "RelWithDebInfo")

foreach(CONFIGURATION_TYPE ${CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIGURATION_TYPE} TYPE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${TYPE} ${BUILD_DIR}/lib) # .lib and .a
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${TYPE} ${BUILD_DIR}/bin) # .so and .dylib
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${TYPE} ${BUILD_DIR}/bin) # .exe and .dll
endforeach()

# 类型后缀
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_MINSIZEREL_POSTFIX "")
set(CMAKE_RELWITHDEBINFO_POSTFIX "")

# 三方库
VCPKG_LOAD_3RDPARTY()
add_subdirectory(3rdparty)
include(${ROOT_DIR}/cmake/module_qt.cmake)
include(${ROOT_DIR}/cmake/module.cmake)

# 执行文件
add_subdirectory(src)

# 拷贝资源文件
add_custom_target(
    "${UseProjectName}CopyTask" ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ROOT_DIR}/resources/" "${BUILD_DIR}/bin/"
    COMMENT "Copying resources"
)
set_property(TARGET "${UseProjectName}CopyTask" PROPERTY FOLDER "CMakePredefinedTargets")
