# 设置插件的输出目录
set(CONFIGURATION_TYPES "Debug" "Release" "MinSizeRel" "RelWithDebInfo")

foreach(CONFIGURATION_TYPE ${CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIGURATION_TYPE} TYPE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${TYPE} ${BUILD_DIR}/lib/epf_plugins) # .lib and .a
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${TYPE} ${BUILD_DIR}/bin/epf_plugins) # .so and .dylib
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${TYPE} ${BUILD_DIR}/bin/epf_plugins) # .exe and .dll
endforeach()

# 添加子目录，子目录下必须有CMakeLists.txt
add_subdirectory(PluginTest)
add_subdirectory(PluginCodeConvert)
add_subdirectory(PluginBase64)
add_subdirectory(PluginSystemInfo)
add_subdirectory(PluginAsn1Viewer)
add_subdirectory(PluginSm2Pkcs7Verify)
add_subdirectory(PluginSm2CsrVerify)
