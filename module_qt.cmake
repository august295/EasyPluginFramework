# 设置 qt 目录
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # 方法一
    set(Qt5_DIR "C:\\Qt\\Qt5.14.2\\5.14.2\\msvc2017_64\\lib\\cmake\\Qt5")
    # 方法二
    # set(CMAKE_PREFIX_PATH "C:\\Qt\\Qt5.14.2\\5.14.2\\msvc2017_64")
else()
    set(CMAKE_PREFIX_PATH "C:\\Qt\\Qt5.14.2\\5.14.2\\mingw73_64\\lib\\cmake\\Qt5")
endif()
# 自动生成
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ARGV0 QT 依赖文件
# ARGV1 UI 文件
# ARGV2 QRC 文件
macro(AddQtInc)
    # 添加 qt include
    if(${ARGC} GREATER_EQUAL 1)
        foreach(qt_library ${ARGV0})
            find_package(Qt5 COMPONENTS ${qt_library} REQUIRED)
            include_directories(${Qt5${qt_library}_INCLUDE_DIRS})
            include_directories(${Qt5${qt_library}_PRIVATE_INCLUDE_DIRS})
        endforeach()
    endif()

    # 编译 ui 文件
    if(${ARGC} GREATER_EQUAL 2)
        qt5_wrap_ui(FORM_FILES ${ARGV1})
    endif()

    # 编译 ui 文件
    if(${ARGC} GREATER_EQUAL 3)
        qt5_add_resources(RESOURCE_FILES "${ARGV2}")
    endif()
endmacro()

macro(AddQtLib)
    # 添加 lib
    foreach(qt_library ${ARGV0})
        link_directories(${QTDIR}/lib)
        target_link_libraries(${PROJECT_NAME} Qt5::${qt_library})
    endforeach()
endmacro()

macro(QtCopy)
    # 拷贝文件
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory 
        ${QTDIR}/plugins/platforms 
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/platforms)
endmacro()
