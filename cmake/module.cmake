# 引入宏
include(${ROOT_DIR}/cmake/module_qt.cmake)
include(${ROOT_DIR}/cmake/module_protobuf.cmake)

# LinkLibraryHeaderOnly  宏名称
# LibraryHeaderOnlyList  三方库仅头文件
macro(LinkLibraryHeaderOnly LibraryHeaderOnlyList)
    foreach(lib ${LibraryHeaderOnlyList})
        include_directories(${ROOT_DIR}/3rdparty/${lib}/include)
    endforeach()
endmacro()

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
    include_directories(${ROOT_DIR}/src/include)
    LinkLibraryHeaderOnly("${LibraryHeaderOnlyList}")

    # 生成项目
    if(${Type} STREQUAL "Exe")
        # 生成可执行文件
        add_executable(${PROJECT_NAME}
            WIN32
            ${HEADER_FILES} ${SOURCE_FILES}
            ${FORM_FILES} ${RESOURCE_FILES}
            ${CURRENT_PATH}/app_win32.rc
        )
        set_target_properties(${PROJECT_NAME} PROPERTIES
            DEBUG_POSTFIX "d"
            VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
        )
    else()
        # 生成链接库
        if(${Type} STREQUAL "Lib")
            add_library(${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES} ${FORM_FILES} ${RESOURCE_FILES})
        elseif(${Type} STREQUAL "Dll")
            add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${FORM_FILES} ${RESOURCE_FILES})
        endif()

        if(NOT(${Group} STREQUAL "Plugin"))
            # 拷贝头文件
            set(TargetInclude "${ROOT_DIR}/src/include/${ProjectName}/")
            file(MAKE_DIRECTORY ${TargetInclude})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${HEADER_FILES}
                ${TargetInclude})
        endif()
    endif()

    # 项目分组
    set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${Group})

    # 添加 qt 链接库
    if(NOT("${QT_LIBRARY_LIST}" STREQUAL ""))
        AddQtLib("${QT_LIBRARY_LIST}")
    endif()

    # 添加项目生成的链接库
    foreach(library ${SELF_LIBRARY_LIST})
        target_link_libraries(${PROJECT_NAME} ${library})
    endforeach()

    foreach(library ${THIRD_LIBRARY_LIST})
        target_include_directories(${PROJECT_NAME} PUBLIC ${${library}_ROOT}/include)

        if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
            find_library(lib
                NAMES ${library}d lib${library}d
                PATHS ${${library}_ROOT}/lib)
            target_link_libraries(${PROJECT_NAME} ${lib})
            unset(lib CACHE)
        elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
            find_library(lib
                NAMES ${library} lib${library}
                PATHS ${${library}_ROOT}/lib)
            target_link_libraries(${PROJECT_NAME} ${lib})
            unset(lib CACHE)
        endif()
    endforeach()
endmacro()

# CreateTarget  宏名称
# ProjectName   项目名称
# Type          项目类型
# Group         项目分组
macro(CreateProto ProjectName Type Group)
    # 项目名称
    message(STATUS ${ProjectName})
    project(${ProjectName})

    # 将当前目录下所有源码文件添加到变量
    set(CURRENT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
    set(HEADER_FILES "")
    set(SOURCE_FILES "")

    # 自动生成 proto
    AddProtobufInc(${ProtoFiles})

    # 文件分类
    if(CMAKE_CXX_PLATFORM_ID MATCHES "Windows")
        source_group(TREE ${CURRENT_PATH} PREFIX "Header Files" FILES ${HEADER_FILES})
        source_group(TREE ${CURRENT_PATH} PREFIX "Source Files" FILES ${SOURCE_FILES})
    elseif(CMAKE_CXX_PLATFORM_ID MATCHES "MinGW")
        source_group("Header Files" FILES ${HEADER_FILES})
        source_group("Source Files" FILES ${SOURCE_FILES})
    elseif(CMAKE_CXX_PLATFORM_ID MATCHES "Linux")
    endif()

    # 生成链接库
    if(${Type} STREQUAL "Lib")
        add_library(${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES} ${FORM_FILES} ${RESOURCE_FILES})
    elseif(${Type} STREQUAL "Dll")
        add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${FORM_FILES} ${RESOURCE_FILES})
    endif()

    # 拷贝头文件
    set(TargetInclude "${ROOT_DIR}/src/include/${ProjectName}/")
    file(MAKE_DIRECTORY ${TargetInclude})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${HEADER_FILES}
        ${TargetInclude})

    # 项目分组
    set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${Group})

    # 添加 protobuf
    AddProtobufLib()
endmacro()