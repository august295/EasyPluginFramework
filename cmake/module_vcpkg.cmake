# 设置 vcpkg 配置
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    if(NOT DEFINED VCPKG_ROOT)
        if(DEFINED ENV{VCPKG_ROOT})
            set(VCPKG_ROOT "$ENV{VCPKG_ROOT}")
        else()
            set(VCPKG_ROOT "C:/dev/vcpkg")
        endif()
    endif()

    # 设置工具链文件
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()
message(STATUS "VCPKG_ROOT: ${VCPKG_ROOT}")
message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

################################################################################
# 3RDPARTY
################################################################################
macro(VCPKG_LOAD_3RDPARTY)
    message(STATUS "Loading 3rd party libraries from vcpkg...")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(VCPKG_TARGET_TRIPLET "x86-windows")
    else()
        set(VCPKG_TARGET_TRIPLET "x64-windows")
    endif()
    set(PKG_CONFIG_PATH "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib/pkgconfig")
    message(STATUS "VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")

    # openssl
    find_package(OpenSSL REQUIRED)
    if(OpenSSL_FOUND)
        message(STATUS "Found OpenSSL: ${OPENSSL_INCLUDE_DIR}")
        message(STATUS "Found OpenSSL: ${OPENSSL_LIBRARIES}")
    else()
        message(FATAL_ERROR "Could not find OpenSSL")
    endif()

    # sqlite3
    find_package(unofficial-sqlite3 CONFIG REQUIRED)
    if(unofficial-sqlite3_FOUND)
        message(STATUS "Found SQLite3")
    else()
        message(FATAL_ERROR "Could not find SQLite3")
    endif()

    # gmp
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(gmp REQUIRED IMPORTED_TARGET gmp)
endmacro()
