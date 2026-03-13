# Qt 库
set(QT_LIBRARY_LIST "Core" "Widgets")
# 链接库
set(SELF_LIBRARY_LIST "EventBus")
# 预编译
add_compile_definitions(PLUGINSYSTEMINFO_EXPORTS)

# 创建项目
CreateTarget("PluginSystemInfo" "Dll" "Plugin")

if(WIN32)
    target_link_libraries(PluginSystemInfo PRIVATE Iphlpapi Ws2_32)
endif()
