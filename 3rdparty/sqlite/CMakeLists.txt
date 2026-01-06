cmake_minimum_required(VERSION 3.15)

set(ProjectName "sqlite")
project(${ProjectName}
    DESCRIPTION "SQLite Database Library"
    LANGUAGES C
)

# =========================
# 基础设置
# =========================
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# =========================
# 构建选项
# =========================
option(BUILD_SHARED_LIBS "Build shared library (DLL on Windows)" OFF)
option(SQLITE_ENABLE_RTREE "Enable r-tree support" ON)
option(SQLITE_ENABLE_FTS4 "Enable FTS4 support" ON)
option(SQLITE_ENABLE_FTS5 "Enable FTS5 support" ON)
option(SQLITE_ENABLE_JSON1 "Enable JSON1 support" ON)
option(SQLITE_ENABLE_RBU "Enable RBU support" ON)
option(SQLITE_ENABLE_STAT4 "Enable STAT4 support" ON)

# =========================
# 生成库
# =========================
add_library(${ProjectName} sqlite3.c)

# 根据选项添加定义
macro(add_sqlite_definition OPTION VALUE)
    if(${OPTION})
        if(${VALUE} STREQUAL "ON" OR ${VALUE} STREQUAL "OFF")
            target_compile_definitions(${ProjectName} PRIVATE ${OPTION}=$<BOOL:${VALUE}>)
        else()
            target_compile_definitions(${ProjectName} PRIVATE ${OPTION}=${VALUE})
        endif()
    endif()
endmacro()

# 应用所有选项
add_sqlite_definition(SQLITE_ENABLE_RTREE ${SQLITE_ENABLE_RTREE})
add_sqlite_definition(SQLITE_ENABLE_FTS4 ${SQLITE_ENABLE_FTS4})
add_sqlite_definition(SQLITE_ENABLE_FTS5 ${SQLITE_ENABLE_FTS5})
add_sqlite_definition(SQLITE_ENABLE_JSON1 ${SQLITE_ENABLE_JSON1})
add_sqlite_definition(SQLITE_ENABLE_RBU ${SQLITE_ENABLE_RBU})
add_sqlite_definition(SQLITE_ENABLE_STAT4 ${SQLITE_ENABLE_STAT4})
add_sqlite_definition(SQLITE_THREADSAFE "1")

# =========================
# SQLITE_API 设置
# =========================
if(WIN32)
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(${ProjectName} PRIVATE [[SQLITE_API=__declspec(dllexport)]])
    else()
        target_compile_definitions(${ProjectName} PRIVATE [[SQLITE_API=__declspec(dllimport)]])
    endif()
else()
    target_compile_definitions(${ProjectName} PRIVATE SQLITE_API=)
endif()

# 判断编译器类型
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options(/utf-8)
else()
endif()
