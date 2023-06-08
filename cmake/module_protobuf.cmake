# 设置 protobuf 目录
if(CMAKE_CXX_PLATFORM_ID MATCHES "Windows")
    set(Protobuf_ROOT "C:/Program Files/protobuf")
elseif(CMAKE_CXX_PLATFORM_ID MATCHES "MinGW")
    set(Protobuf_ROOT "")
elseif(CMAKE_CXX_PLATFORM_ID MATCHES "Linux")
    set(Protobuf_ROOT "")
endif()
set(CMAKE_PREFIX_PATH ${Protobuf_ROOT})

macro(AddProtobufInc)
    find_package(Protobuf REQUIRED)
    include_directories(${PROTOBUF_INCLUDE_DIRS})

    if(${ARGC} GREATER_EQUAL 1)
        protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${ARGV0}
            EXPORT_MACRO "PROTO_EXPORT"
            PROTOC_OUT_DIR ${CURRENT_PATH}
        )
        list(APPEND HEADER_FILES ${PROTO_HDRS})
        list(APPEND SOURCE_FILES ${PROTO_SRCS})
    endif()
endmacro()

macro(AddProtobufLib)
    # add_compile_definitions(Protobuf_USE_STATIC_LIBS)
    add_compile_definitions(PROTOBUF_USE_DLLS)
    add_compile_definitions(PROTO_EXPORT=__declspec\(dllexport\))
    target_link_libraries(${PROJECT_NAME} ${Protobuf_LIBRARIES})

    get_filename_component(protobuf_dir ${Protobuf_PROTOC_EXECUTABLE} DIRECTORY)
    file(COPY ${protobuf_dir}/libprotobufd.dll DESTINATION ${ROOT_DIR}/build/bin/)
endmacro()
