#ifndef __LIBRARY_HPP__
#define __LIBRARY_HPP__

#include <spdlog/spdlog.h>

/**
 * @brief 跨平台加载 DLL 宏
 */
#ifdef _WIN32
    #include <windows.h>
    #define LIB_HANDLE           HINSTANCE
    #define LIB_LOAD(x)          LoadLibrary(x)
    #define LIB_GET_PROC_ADDRESS GetProcAddress
    #define LIB_UNLOAD           FreeLibrary
    #define LIB_ERROR            GetLastErrorMsg
#else
    #include <dlfcn.h>
    #define LIB_HANDLE           void*
    #define LIB_LOAD(x)          dlopen(x, RTLD_LAZY)
    #define LIB_GET_PROC_ADDRESS dlsym
    #define LIB_UNLOAD           dlclose
    #define LIB_ERROR            dlerrorMsg
#endif

#ifdef _WIN32
const char* GetLastErrorMsg()
{
    static char errorMsg[256];
    DWORD       errorCode = GetLastError();
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        errorMsg,
        sizeof(errorMsg),
        NULL);
    return errorMsg;
}
#else
const char* dlerrorMsg()
{
    return dlerror();
}
#endif

/**
 * @brief 加载 DLL 函数模板
 * @tparam T           函数类型
 * @param handle       动态库句柄
 * @param funcName     函数名称
 * @return T           函数类型
 */
template <typename T>
T LoadFunction(LIB_HANDLE handle, const char* funcName)
{
    T func = reinterpret_cast<T>(LIB_GET_PROC_ADDRESS(handle, funcName));
    if (!func) {
        SPDLOG_ERROR("Error: Failed to load function {}", funcName);
    }
    return func;
}

#endif
