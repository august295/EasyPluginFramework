# Qt 库
set(QT_LIBRARY_LIST "Core" "Widgets")
# 链接库
set(SELF_LIBRARY_LIST "EventBus" "endecode")
# 预编译
add_compile_definitions(PLUGINSM2PKCS7VERIFY_EXPORTS)
add_compile_definitions(ENDECODE_STATIC)

# 创建项目
CreateTarget("PluginSm2Pkcs7Verify" "Dll" "Plugin")
set_property(TARGET PluginSm2Pkcs7Verify PROPERTY CXX_STANDARD 20)
set_property(TARGET PluginSm2Pkcs7Verify PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(PluginSm2Pkcs7Verify PRIVATE ${ROOT_DIR}/3rdparty/endecode/include)
target_sources(PluginSm2Pkcs7Verify PRIVATE
    ${ROOT_DIR}/3rdparty/endecode/src/ecc/ecc.c
    ${ROOT_DIR}/3rdparty/endecode/src/sm2/sm2.c
    ${ROOT_DIR}/3rdparty/endecode/src/sm3/sm3.c)

if(TARGET PkgConfig::gmp)
    target_link_libraries(PluginSm2Pkcs7Verify PRIVATE PkgConfig::gmp)
endif()

if(DEFINED VCPKG_ROOT AND DEFINED VCPKG_TARGET_TRIPLET)
    target_include_directories(PluginSm2Pkcs7Verify PRIVATE ${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include)
    target_link_libraries(PluginSm2Pkcs7Verify PRIVATE
        optimized ${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib/gmp.lib
        debug ${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/lib/gmp.lib)
endif()
