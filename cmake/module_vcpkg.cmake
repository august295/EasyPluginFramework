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
    set(VCPKG_TARGET_TRIPLET "x64-windows")
    set(VCPKG_MANIFEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
endif()
message(STATUS "VCPKG_ROOT: ${VCPKG_ROOT}")
message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")
message(STATUS "VCPKG_MANIFEST_DIR: ${VCPKG_MANIFEST_DIR}")

################################################################################
# 3RDPARTY
################################################################################
macro(VCPKG_LOAD_3RDPARTY)
    set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")
    message(STATUS "Loading 3rd party libraries from vcpkg...")

    # openssl
    find_package(OpenSSL REQUIRED)
    if (OpenSSL_FOUND)
        message(STATUS "Found OpenSSL: ${OPENSSL_INCLUDE_DIR}")
        message(STATUS "Found OpenSSL: ${OPENSSL_LIBRARIES}")
    else()
        message(FATAL_ERROR "Could not find OpenSSL")
    endif()

    # sqlite3
    find_package(unofficial-sqlite3 CONFIG REQUIRED)
    if (unofficial-sqlite3_FOUND)
        message(STATUS "Found SQLite3")
    else()
        message(FATAL_ERROR "Could not find SQLite3")
    endif()
endmacro()
