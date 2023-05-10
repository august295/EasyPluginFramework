# 设置 qt 目录
if(CMAKE_CXX_PLATFORM_ID MATCHES "Windows")
    # 方法一
    set(Qt5_DIR "C:\\Qt\\Qt5.14.2\\5.14.2\\msvc2017_64\\lib\\cmake\\Qt5")
    # 方法二
    # set(CMAKE_PREFIX_PATH "C:\\Qt\\Qt5.14.2\\5.14.2\\msvc2017_64")
elseif(CMAKE_CXX_PLATFORM_ID MATCHES "MinGW")
    set(CMAKE_PREFIX_PATH "C:\\Qt\\Qt5.14.2\\5.14.2\\mingw73_64\\lib\\cmake\\Qt5")
elseif(CMAKE_CXX_PLATFORM_ID MATCHES "Linux")
endif()

# 自动生成
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ARGV0 QT 依赖文件
macro(AddQtInc QtLibraryList)
    # 添加 qt include
    foreach(qt_library ${QtLibraryList})
        find_package(Qt5 COMPONENTS ${qt_library} REQUIRED)
        include_directories(${Qt5${qt_library}_INCLUDE_DIRS})
        include_directories(${Qt5${qt_library}_PRIVATE_INCLUDE_DIRS})
    endforeach()
endmacro()

macro(AddQtLib QtLibraryList)
    # 添加 lib
    foreach(qt_library ${QtLibraryList})
        link_directories(${QTDIR}/lib)
        target_link_libraries(${PROJECT_NAME} Qt5::${qt_library})
    endforeach()
endmacro()
