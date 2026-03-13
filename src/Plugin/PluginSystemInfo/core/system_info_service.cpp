#include "system_info_service.h"

#include <windows.h>

#include <array>
#include <cstdint>
#include <ctime>
#include <cwchar>
#include <iomanip>
#include <sstream>

namespace
{
using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);

constexpr DWORD MAX_REG_VALUE_LENGTH = 512U;
constexpr ULONGLONG BYTES_IN_KILOBYTE = 1024ULL;
constexpr ULONGLONG BYTES_IN_MEGABYTE = BYTES_IN_KILOBYTE * 1024ULL;
constexpr ULONGLONG BYTES_IN_GIGABYTE = BYTES_IN_MEGABYTE * 1024ULL;

std::string convertWideToUtf8(const std::wstring& text)
{
    if (text.empty())
    {
        return "";
    }
    const int targetLength = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (targetLength <= 0)
    {
        return "";
    }
    std::string value(static_cast<size_t>(targetLength), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, value.data(), targetLength, nullptr, nullptr);
    value.pop_back();
    return value;
}

std::string formatMemoryInGigabytes(const ULONGLONG value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << static_cast<double>(value) / static_cast<double>(BYTES_IN_GIGABYTE) << " GB";
    return stream.str();
}

std::string readRegistryString(const HKEY rootKey, const wchar_t* subKey, const wchar_t* valueName)
{
    HKEY keyHandle = nullptr;
    if (RegOpenKeyExW(rootKey, subKey, 0, KEY_READ | KEY_WOW64_64KEY, &keyHandle) != ERROR_SUCCESS)
    {
        return "";
    }
    std::array<wchar_t, MAX_REG_VALUE_LENGTH> buffer = {};
    DWORD valueType = REG_NONE;
    DWORD bufferSize = static_cast<DWORD>(buffer.size() * sizeof(wchar_t));
    const LONG result = RegQueryValueExW(keyHandle, valueName, nullptr, &valueType, reinterpret_cast<LPBYTE>(buffer.data()), &bufferSize);
    RegCloseKey(keyHandle);
    if (result != ERROR_SUCCESS || (valueType != REG_SZ && valueType != REG_EXPAND_SZ))
    {
        return "";
    }
    return convertWideToUtf8(buffer.data());
}

DWORD readRegistryDword(const HKEY rootKey, const wchar_t* subKey, const wchar_t* valueName)
{
    HKEY keyHandle = nullptr;
    if (RegOpenKeyExW(rootKey, subKey, 0, KEY_READ | KEY_WOW64_64KEY, &keyHandle) != ERROR_SUCCESS)
    {
        return 0U;
    }
    DWORD value = 0U;
    DWORD valueType = REG_NONE;
    DWORD valueSize = sizeof(DWORD);
    const LONG result = RegQueryValueExW(keyHandle, valueName, nullptr, &valueType, reinterpret_cast<LPBYTE>(&value), &valueSize);
    RegCloseKey(keyHandle);
    if (result != ERROR_SUCCESS || valueType != REG_DWORD)
    {
        return 0U;
    }
    return value;
}

RTL_OSVERSIONINFOW queryOsVersion()
{
    RTL_OSVERSIONINFOW versionInfo = {};
    versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
    const HMODULE ntdllModule = GetModuleHandleW(L"ntdll.dll");
    if (ntdllModule == nullptr)
    {
        return versionInfo;
    }
    const auto rtlGetVersion = reinterpret_cast<RtlGetVersionFn>(GetProcAddress(ntdllModule, "RtlGetVersion"));
    if (rtlGetVersion == nullptr)
    {
        return versionInfo;
    }
    rtlGetVersion(&versionInfo);
    return versionInfo;
}

std::string buildVersionText()
{
    const RTL_OSVERSIONINFOW versionInfo = queryOsVersion();
    if (versionInfo.dwMajorVersion == 0U)
    {
        return "未知";
    }
    std::ostringstream stream;
    stream << versionInfo.dwMajorVersion << '.' << versionInfo.dwMinorVersion << '.' << versionInfo.dwBuildNumber;
    return stream.str();
}

std::string buildArchitectureShortText()
{
    SYSTEM_INFO systemInfo = {};
    GetNativeSystemInfo(&systemInfo);
    switch (systemInfo.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_AMD64:
        return "x64";
    case PROCESSOR_ARCHITECTURE_INTEL:
        return "x86";
    case PROCESSOR_ARCHITECTURE_ARM64:
        return "ARM64";
    default:
        return "未知";
    }
}

std::string buildComputerNameText()
{
    std::array<wchar_t, MAX_COMPUTERNAME_LENGTH + 1> buffer = {};
    DWORD length = static_cast<DWORD>(buffer.size());
    if (!GetComputerNameW(buffer.data(), &length))
    {
        return "";
    }
    return convertWideToUtf8(std::wstring(buffer.data(), length));
}

std::string buildSystemTypeText()
{
    const std::string architecture = buildArchitectureShortText();
    if (architecture == "x64")
    {
        return "64 位操作系统, 基于 x64 的处理器";
    }
    if (architecture == "x86")
    {
        return "32 位操作系统, 基于 x86 的处理器";
    }
    if (architecture == "ARM64")
    {
        return "64 位操作系统, 基于 ARM64 的处理器";
    }
    return "未知";
}

std::string buildInstallDateText(const DWORD installDate)
{
    if (installDate == 0U)
    {
        return "未知";
    }
    std::time_t installTime = static_cast<std::time_t>(installDate);
    std::tm localTime = {};
#if defined(_WIN32)
    if (localtime_s(&localTime, &installTime) != 0)
    {
        return "未知";
    }
#else
    if (localtime_r(&installTime, &localTime) == nullptr)
    {
        return "未知";
    }
#endif
    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(4) << localTime.tm_year + 1900 << '/'
           << std::setw(2) << localTime.tm_mon + 1 << '/'
           << std::setw(2) << localTime.tm_mday;
    return stream.str();
}

std::string buildOsBuildText()
{
    const std::string currentBuild = readRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"CurrentBuild");
    const std::string currentBuildNumber = readRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"CurrentBuildNumber");
    const DWORD ubr = readRegistryDword(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"UBR");
    std::string buildValue = currentBuild.empty() ? currentBuildNumber : currentBuild;
    if (buildValue.empty())
    {
        buildValue = buildVersionText();
    }
    if (ubr > 0U)
    {
        buildValue += "." + std::to_string(ubr);
    }
    return buildValue;
}

std::string buildPenAndTouchText()
{
    const int maximumTouchCount = GetSystemMetrics(SM_MAXIMUMTOUCHES);
    if (maximumTouchCount > 0)
    {
        return "支持触控, 最多 " + std::to_string(maximumTouchCount) + " 个触点";
    }
    return "此显示器不支持笔或触控输入";
}

std::string buildExperienceText()
{
    const std::string experiencePack = readRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"WindowsFeatureExperiencePackVersion");
    if (!experiencePack.empty())
    {
        return experiencePack;
    }
    return "未提供";
}

std::string buildDeviceIdText()
{
    const std::string machineGuid = readRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", L"MachineGuid");
    if (!machineGuid.empty())
    {
        return machineGuid;
    }
    return "未知";
}
}

namespace core
{
SystemInfoSnapshot SystemInfoService::collectSnapshot() const
{
    SystemInfoSnapshot snapshot;
    const std::string productName = readRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ProductName");
    const std::string displayVersion = readRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"DisplayVersion");
    const std::string releaseId = readRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ReleaseId");
    const std::string cpuName = readRegistryString(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", L"ProcessorNameString");
    MEMORYSTATUSEX memoryStatus = {};
    memoryStatus.dwLength = sizeof(memoryStatus);
    const bool hasMemoryStatus = GlobalMemoryStatusEx(&memoryStatus) != FALSE;
    const std::string versionText = !displayVersion.empty() ? displayVersion : releaseId;
    const std::string productId = readRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"ProductId");
    const DWORD installDate = readRegistryDword(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", L"InstallDate");

    snapshot.deviceEntries.push_back({"设备名称", buildComputerNameText()});
    snapshot.deviceEntries.push_back({"处理器", cpuName.empty() ? "未知" : cpuName});
    snapshot.deviceEntries.push_back({"已安装 RAM", hasMemoryStatus ? formatMemoryInGigabytes(memoryStatus.ullTotalPhys) : "未知"});
    snapshot.deviceEntries.push_back({"设备 ID", buildDeviceIdText()});
    snapshot.deviceEntries.push_back({"产品 ID", productId.empty() ? "未知" : productId});
    snapshot.deviceEntries.push_back({"系统类型", buildSystemTypeText()});
    snapshot.deviceEntries.push_back({"笔和触控", buildPenAndTouchText()});

    snapshot.windowsEntries.push_back({"版本", productName.empty() ? "Windows" : productName});
    snapshot.windowsEntries.push_back({"版本号", versionText.empty() ? buildVersionText() : versionText});
    snapshot.windowsEntries.push_back({"安装日期", buildInstallDateText(installDate)});
    snapshot.windowsEntries.push_back({"操作系统内部版本", buildOsBuildText()});
    snapshot.windowsEntries.push_back({"体验", buildExperienceText()});
    return snapshot;
}
}
