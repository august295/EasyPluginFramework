#ifndef __LIBRARY_HPP__
#define __LIBRARY_HPP__

#include <string>

/**
 * @brief 跨平台加载 DLL 宏
 */
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #define LIB_HANDLE           HINSTANCE
    #define LIB_LOAD(x)          LoadLibrary(x)
    #define LIB_GET_PROC_ADDRESS GetProcAddress
    #define LIB_UNLOAD           FreeLibrary
    #define LIB_ERROR            GetLastErrorMsg
#elif defined(__linux__)
    #include <dlfcn.h>
    #define LIB_HANDLE           void*
    #define LIB_LOAD(x)          dlopen(x, RTLD_LAZY)
    #define LIB_GET_PROC_ADDRESS dlsym
    #define LIB_UNLOAD           dlclose
    #define LIB_ERROR            dlerrorMsg
#endif

#if defined(_WIN32) || defined(_WIN64)
inline const char* GetLastErrorMsg()
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
#elif defined(__linux__)
inline const char* dlerrorMsg()
{
    return dlerror();
}
#endif

/**
 * @brief 跨平台获取动态库全名
 */
#if defined(_WIN32) || defined(_WIN64)
    #define LIB_EXT    ".dll"
    #define LIB_PREFIX ""
    #ifdef _DEBUG
        #define LIB_SUFFIX "d"
    #else
        #define LIB_SUFFIX ""
    #endif
#elif defined(__linux__)
    #define LIB_EXT    ".so"
    #define LIB_PREFIX "lib"
    #ifdef _DEBUG
        #define LIB_SUFFIX "d"
    #else
        #define LIB_SUFFIX ""
    #endif
#endif

inline std::string GetSharedName(const std::string& baseName)
{
    return std::string(LIB_PREFIX) + baseName + std::string(LIB_SUFFIX) + std::string(LIB_EXT);
}

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
    return reinterpret_cast<T>(LIB_GET_PROC_ADDRESS(handle, funcName));
}

#endif
