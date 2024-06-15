# 设置 qt 目录
if(CMAKE_CXX_PLATFORM_ID MATCHES "Windows")
    set(Qt5_ROOT "C:\\Qt\\5.15.2\\msvc2019_64")
elseif(CMAKE_CXX_PLATFORM_ID MATCHES "MinGW")
    set(Qt5_ROOT "C:\\Qt\\5.15.2\\mingw81_64")
elseif(CMAKE_CXX_PLATFORM_ID MATCHES "Linux")
endif()
set(CMAKE_PREFIX_PATH ${Qt5_ROOT})

# ARGV0 QT 依赖文件
macro(AddQtInc QtLibraryList)
    # 添加 qt include
    foreach(qt_library ${QtLibraryList})
        find_package(Qt5 COMPONENTS ${qt_library} REQUIRED)
        include_directories(${Qt5${qt_library}_INCLUDE_DIRS})
        include_directories(${Qt5${qt_library}_PRIVATE_INCLUDE_DIRS})
    endforeach()
endmacro()

macro(AddQtLib ProjectName QtLibraryList)
    # 添加 lib
    foreach(qt_library ${QtLibraryList})
        target_link_libraries(${ProjectName} Qt5::${qt_library})
    endforeach()
endmacro()
