#ifndef __CERTSM2HELPER_H__
#define __CERTSM2HELPER_H__

#include "endecode/asn1/cert_sm2.h"
#include "endecode/base64/base64.h"

#include <map>
#include <string>
#include <vector>

class CertSM2Helper
{
public:
    CertSM2Helper();
    ~CertSM2Helper();

    /**
     * @brief   OID 名称映射
     */
    void        InitX500Map();
    std::string GetX500Name(const std::string& name);

    /**
     * @brief   解析 SM2 证书
     */
    bool ParseCertSM2(const std::string& filename);
    bool ParseCertSM2(const char* base64Cert);
    bool ParseCertSM2(const uint8_t* data, size_t datalen);
    bool IsParse();

    /**
     * @brief   获取证书指定类型的信息
     * @param   Type                    [IN]        证书解析标识，应符合 GM/T 0006
     * @return  证书信息                            成功，返回证书指定类型的信息
     *          空串                                失败或证书中不存在该项内容
     */
    std::string GetCertInfo(size_t type);
    std::string Capture(const std::string& str, const std::string& flag);

private:
    struct CertSM2HelperImpl;
    CertSM2HelperImpl* m_impl;
};

#endif