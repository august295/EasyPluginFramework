option(QT_PATH "The Qt install path")

# 设置 qt 目录
if(QT_PATH)
    set(Qt_ROOT ${QT_PATH})
else()
    if(CMAKE_CXX_PLATFORM_ID MATCHES "Windows")
        set(Qt_ROOT "C:\\Qt\\5.15.2\\msvc2019_64")
    elseif(CMAKE_CXX_PLATFORM_ID MATCHES "MinGW")
    elseif(CMAKE_CXX_PLATFORM_ID MATCHES "Linux")
    elseif(CMAKE_CXX_PLATFORM_ID MATCHES "Darwin")
    endif()
endif()

set(CMAKE_PREFIX_PATH ${Qt_ROOT})
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)

# ARGV0 QT 依赖文件
macro(AddQtInc QtLibraryList)
    # 添加 qt include
    foreach(qt_library ${QtLibraryList})
        find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${qt_library} REQUIRED)
        include_directories(${Qt${QT_VERSION_MAJOR}${qt_library}_INCLUDE_DIRS})
        include_directories(${Qt${QT_VERSION_MAJOR}${qt_library}_PRIVATE_INCLUDE_DIRS})
    endforeach()
endmacro()

macro(AddQtLib ProjectName QtLibraryList)
    # 添加 lib
    foreach(qt_library ${QtLibraryList})
        target_link_libraries(${ProjectName} Qt${QT_VERSION_MAJOR}::${qt_library})
    endforeach()
endmacro()

