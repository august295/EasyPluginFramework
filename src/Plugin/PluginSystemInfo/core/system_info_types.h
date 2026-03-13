#ifndef __SYSTEM_INFO_TYPES_H__
#define __SYSTEM_INFO_TYPES_H__

#include <string>
#include <vector>

namespace core
{
/**
 * @brief 键值对信息条目。
 */
struct SystemInfoEntry
{
    std::string label;
    std::string value;
};

/**
 * @brief 系统信息快照。
 */
struct SystemInfoSnapshot
{
    std::vector<SystemInfoEntry> deviceEntries;
    std::vector<SystemInfoEntry> windowsEntries;
};
}

#endif
