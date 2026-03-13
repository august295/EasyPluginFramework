#ifndef __SYSTEM_INFO_SERVICE_H__
#define __SYSTEM_INFO_SERVICE_H__

#include "system_info_types.h"

namespace core
{
/**
 * @brief 采集 Windows 系统信息。
 */
class SystemInfoService
{
public:
    /**
     * @brief 采集系统信息快照。
     * @return SystemInfoSnapshot 系统信息快照。
     */
    SystemInfoSnapshot collectSnapshot() const;
};
}

#endif
