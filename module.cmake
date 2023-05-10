# CreateTarget  宏名称
# ProjectName   项目名称
# Type          项目类型
# Group         项目分组
macro(CreateTarget ProjectName Type Group)
    # 引入 qt 宏
    if(NOT("${QT_LIBRARY_LIST}" STREQUAL ""))
        include(${ROOT_DIR}/module_qt.cmake)
    endif()

    # 项目名称
    message(STATUS ${ProjectName})
    project(${ProjectName})

    # 将当前目录下所有源码文件添加到变量
    set(CURRENT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
    set(HEADER_FILES "")
    set(SOURCE_FILES "")
    set(FORM_FILES "")
    set(RESOURCE_FILES "")
    file(GLOB_RECURSE HEADER_FILES "${CURRENT_PATH}/*.h" "${CURRENT_PATH}/*.hpp")
    file(GLOB_RECURSE SOURCE_FILES "${CURRENT_PATH}/*.c" "${CURRENT_PATH}/*.cpp")
    file(GLOB_RECURSE FORM_FILES "${CURRENT_PATH}/*.ui")
    file(GLOB_RECURSE RESOURCE_FILES "${CURRENT_PATH}/*.qrc")

    # 文件分类
    if(CMAKE_CXX_PLATFORM_ID MATCHES "Windows")
        source_group(TREE ${CURRENT_PATH} PREFIX "Header Files" FILES ${HEADER_FILES})
        source_group(TREE ${CURRENT_PATH} PREFIX "Source Files" FILES ${SOURCE_FILES})
        source_group(TREE ${CURRENT_PATH} PREFIX "Form Files" FILES ${FORM_FILES})
        source_group(TREE ${CURRENT_PATH} PREFIX "Resource Files" FILES ${RESOURCE_FILES})
    elseif(CMAKE_CXX_PLATFORM_ID MATCHES "MinGW")
        source_group("Header Files" FILES ${HEADER_FILES})
        source_group("Source Files" FILES ${SOURCE_FILES})
        source_group("Form Files" FILES ${FORM_FILES})
        source_group("Resource Files" FILES ${RESOURCE_FILES})
    elseif(CMAKE_CXX_PLATFORM_ID MATCHES "Linux")
    endif()

    # 头文件搜索的路径
    include_directories(${CURRENT_PATH})
    include_directories(${ROOT_DIR}/include)

    # 添加 qt 头文件
    if(NOT("${QT_LIBRARY_LIST}" STREQUAL ""))
        AddQtInc("${QT_LIBRARY_LIST}")
    endif()

    # 生成项目
    if(${Type} STREQUAL "Exe")
        # 生成可执行文件
        add_executable(${PROJECT_NAME}
            ${HEADER_FILES} ${SOURCE_FILES} ${FORM_FILES} ${RESOURCE_FILES})
        set_target_properties(${PROJECT_NAME} PROPERTIES
            DEBUG_POSTFIX "d"
            VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
    else()
        # 生成链接库
        if(${Type} STREQUAL "Lib")
            add_library(${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES} ${FORM_FILES} ${RESOURCE_FILES})
        elseif(${Type} STREQUAL "Dll")
            add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${FORM_FILES} ${RESOURCE_FILES})
        endif()
    endif()
    # 项目分组
    set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${Group})

    # 添加 qt 链接库
    if(NOT("${QT_LIBRARY_LIST}" STREQUAL ""))
        AddQtLib("${QT_LIBRARY_LIST}")
    endif()

    # 添加项目生成的链接库
    foreach(_lib ${SELF_LIBRARY_LIST})
        include_directories(${CURRENT_PATH}/../)
        target_link_libraries(${PROJECT_NAME} ${_lib})
    endforeach()
endmacro()
