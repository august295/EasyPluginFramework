# CreateTarget  宏名称
# ProjectName   项目名称
# Type          项目类型
# Group         项目分组
macro(CreateTarget ProjectName Type Group)
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

    # 添加 qt 头文件
    if(NOT("${QT_LIBRARY_LIST}" STREQUAL ""))
        # 自动生成
        set(CMAKE_INCLUDE_CURRENT_DIR ON)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTOUIC ON)
        set(CMAKE_AUTORCC ON)
        AddQtInc("${QT_LIBRARY_LIST}")
        file(GLOB_RECURSE FORM_FILES "${CURRENT_PATH}/*.ui")
        file(GLOB_RECURSE RESOURCE_FILES "${CURRENT_PATH}/*.qrc")
    endif()

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
    include_directories(${ROOT_DIR}/src)

    # 生成项目
    if(${Type} STREQUAL "Exe")
        # 生成可执行文件
        add_executable(${ProjectName}
            # WIN32
            ${HEADER_FILES} ${SOURCE_FILES}
            ${FORM_FILES} ${RESOURCE_FILES}
            ${CURRENT_PATH}/app_win32.rc
        )
    else()
        # 生成链接库
        if(${Type} STREQUAL "Lib")
            add_library(${ProjectName} STATIC ${HEADER_FILES} ${SOURCE_FILES} ${FORM_FILES} ${RESOURCE_FILES})
        elseif(${Type} STREQUAL "Dll")
            add_library(${ProjectName} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${FORM_FILES} ${RESOURCE_FILES})
        endif()
    endif()

    set_target_properties(${ProjectName} PROPERTIES
        DEBUG_POSTFIX "d"
        VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
    )

    # 项目分组
    set_property(TARGET ${ProjectName} PROPERTY FOLDER ${Group})

    # 添加 qt 链接库
    if(NOT("${QT_LIBRARY_LIST}" STREQUAL ""))
        AddQtLib(${ProjectName} "${QT_LIBRARY_LIST}")
    endif()

    # 添加项目生成的链接库
    foreach(library ${SELF_LIBRARY_LIST})
        target_link_libraries(${ProjectName} ${library})
    endforeach()
endmacro()

# 添加头文件和链接库路径
macro(AddLibrary LibraryList)
    foreach(library ${LibraryList})
        # 添加 include
        include_directories(${ROOT_DIR}/3rdparty/${library})
        include_directories(${ROOT_DIR}/3rdparty/${library}/include)
        # 添加链接库路径
        if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
            if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
                link_directories(${ROOT_DIR}/3rdparty/${library}/lib/win32/${CMAKE_BUILD_TYPE})
            else()
                link_directories(${ROOT_DIR}/3rdparty/${library}/lib/win64/${CMAKE_BUILD_TYPE})
            endif()
        endif()
    endforeach()
endmacro()
