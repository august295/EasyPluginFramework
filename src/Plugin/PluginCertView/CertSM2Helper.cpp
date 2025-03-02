#include <fstream>

#include "CertSM2Helper.h"

struct CertSM2Helper::CertSM2HelperImpl
{
    std::map<std::string, SM_OID_MAPPING> m_x500Map;
    SM2Certificate*                       cert;
};

CertSM2Helper::CertSM2Helper()
    : m_impl(new CertSM2HelperImpl())
{
    InitX500Map();
    m_impl->cert = NULL;
}

CertSM2Helper::~CertSM2Helper()
{
    m_impl->m_x500Map.clear();
    sm2_cert_free(m_impl->cert);
}

void CertSM2Helper::InitX500Map()
{
    for (size_t i = 0; i < sizeof(sm_oid_mapping_x500) / sizeof(SM_OID_MAPPING); i++)
    {
        m_impl->m_x500Map[sm_oid_mapping_x500[i].oid_string] = sm_oid_mapping_x500[i];
    }
}

std::string CertSM2Helper::GetX500Name(const std::string& name)
{
    return m_impl->m_x500Map[name].oid_name;
}

bool CertSM2Helper::ParseCertSM2(const std::string& filename)
{
    bool ret = true;

    std::ifstream in(filename);

    std::string str;
    std::string line;
    while (std::getline(in, line))
    {
        if (line == PEM_CERT_BEGIN || line == PEM_CERT_END || line.empty())
        {
            continue;
        }
        str += line;
    }
    in.close();

    size_t   Base64CertLen = str.size();
    size_t   BinCertLen    = BASE64_DECODE_OUT_SIZE(Base64CertLen);
    uint8_t* BinCert       = (uint8_t*)calloc(1, BinCertLen + 1);
    base64_decode(str.c_str(), Base64CertLen, BinCert);
    m_impl->cert = sm2_cert_parse(BinCert, BinCertLen);

    if (m_impl->cert == NULL)
    {
        ret = false;
    }
    return ret;
}

bool CertSM2Helper::ParseCertSM2(const char* base64Cert)
{
    bool     ret           = true;
    size_t   Base64CertLen = strlen(base64Cert);
    size_t   BinCertLen    = BASE64_DECODE_OUT_SIZE(Base64CertLen);
    uint8_t* BinCert       = (uint8_t*)calloc(1, BinCertLen + 1);
    base64_decode(base64Cert, Base64CertLen, BinCert);
    m_impl->cert = sm2_cert_parse(BinCert, BinCertLen);

    if (m_impl->cert == NULL)
    {
        ret = false;
    }
    return ret;
}

bool CertSM2Helper::ParseCertSM2(const uint8_t* data, size_t datalen)
{
    bool ret     = true;
    m_impl->cert = sm2_cert_parse(data, datalen);

    if (m_impl->cert == NULL)
    {
        ret = false;
    }
    return ret;
}

bool CertSM2Helper::IsParse()
{
    return m_impl->cert != NULL;
}

std::string CertSM2Helper::GetCertInfo(size_t type)
{
    SM2Certificate* cert    = m_impl->cert;
    char*           info    = NULL;
    size_t          infoLen = 0;
    if (type == SGD_CERT_VERSION)
    {
        infoLen = cert->tbsCertificate.version.length + 1;
        info    = (char*)calloc(1, infoLen);
        snprintf(info, infoLen, "%d", cert->tbsCertificate.version.value[0]);
    }
    else if (type == SGD_CERT_SERIAL)
    {
        infoLen = cert->tbsCertificate.serialNumber.length * 2 + 1;
        info    = (char*)calloc(1, infoLen);
        for (size_t i = 0; i < cert->tbsCertificate.serialNumber.length; i++)
        {
            sprintf(info + i * 2, "%02X", cert->tbsCertificate.serialNumber.value[i]);
        }
    }
    else if (type == SGD_CERT_SIGNATURE_ALGORITHM)
    {
        info = (char*)calloc(1, 64);
        oid_to_string(cert->tbsCertificate.signature.algorithm.value, cert->tbsCertificate.signature.algorithm.length, info);
        infoLen = strlen(info);
    }
    else if (type == SGD_CERT_ISSUER || type == SGD_CERT_ISSUER_CN || type == SGD_CERT_ISSUER_O || type == SGD_CERT_ISSUER_OU)
    {
        Name        issuer = cert->tbsCertificate.issuer;
        std::string info_str;
        char        temp[64] = {0};
        for (size_t i = 0; i < issuer.count; i++)
        {
            for (size_t j = 0; j < issuer.names[i].count; j++)
            {
                AttributeTypeAndValue rdn = issuer.names[i].rdn[j];
                oid_to_string(rdn.type.value, rdn.type.length, temp);
                auto typeName = GetX500Name(temp);
                info_str += typeName;
                info_str += "=";
                info_str += std::string((char*)rdn.value.value, rdn.value.length);
                info_str += ";";
            }
        }

        if (type == SGD_CERT_ISSUER_CN)
        {
            info_str = Capture(info_str, "CN=");
        }
        else if (type == SGD_CERT_ISSUER_O)
        {
            info_str = Capture(info_str, "O=");
        }
        else if (type == SGD_CERT_ISSUER_OU)
        {
            info_str = Capture(info_str, "OU=");
        }
        infoLen = info_str.size();
        info    = (char*)calloc(1, infoLen + 1);
        memcpy(info, info_str.c_str(), info_str.size());
    }
    else if (type == SGD_CERT_VALID_TIME || type == SGD_CERT_NOTBEFORE_TIME || type == SGD_CERT_NOTAFTER_TIME)
    {
        Validity    time          = cert->tbsCertificate.validity;
        char        notBefore[20] = {0};
        char        notAfter[20]  = {0};
        size_t      utcOffset     = 8;
        std::string info_str;
        if (time.notBefore.tag == EASY_ASN1_UTCTIME)
        {
            convertUTCTimeToStandard((char*)time.notBefore.value, utcOffset, notBefore);
        }
        else if (time.notBefore.tag == EASY_ASN1_GENERALIZEDTIME)
        {
            convertGeneralizedTimeToStandard((char*)time.notBefore.value, utcOffset, notBefore);
        }
        info_str += notBefore;
        info_str += ";";
        if (time.notAfter.tag == EASY_ASN1_UTCTIME)
        {
            convertUTCTimeToStandard((char*)time.notAfter.value, utcOffset, notAfter);
        }
        else if (time.notAfter.tag == EASY_ASN1_GENERALIZEDTIME)
        {
            convertGeneralizedTimeToStandard((char*)time.notAfter.value, utcOffset, notAfter);
        }
        info_str += notAfter;

        if (type == SGD_CERT_NOTBEFORE_TIME)
        {
            info_str = notBefore;
        }
        else if (type == SGD_CERT_NOTAFTER_TIME)
        {
            info_str = notAfter;
        }
        infoLen = info_str.size();
        info    = (char*)calloc(1, infoLen + 1);
        memcpy(info, info_str.c_str(), info_str.size());
    }
    else if (type == SGD_CERT_SUBJECT || type == SGD_CERT_SUBJECT_CN || type == SGD_CERT_SUBJECT_O || type == SGD_CERT_SUBJECT_OU)
    {
        Name        subject = cert->tbsCertificate.subject;
        std::string info_str;
        char        temp[64] = {0};
        for (size_t i = 0; i < subject.count; i++)
        {
            for (size_t j = 0; j < subject.names[i].count; j++)
            {
                AttributeTypeAndValue rdn = subject.names[i].rdn[j];
                oid_to_string(rdn.type.value, rdn.type.length, temp);
                auto typeName = GetX500Name(temp);
                info_str += typeName;
                info_str += "=";
                info_str += std::string((char*)rdn.value.value, rdn.value.length);
                info_str += ";";
            }
        }

        if (type == SGD_CERT_SUBJECT_CN)
        {
            info_str = Capture(info_str, "CN=");
        }
        else if (type == SGD_CERT_SUBJECT_O)
        {
            info_str = Capture(info_str, "O=");
        }
        else if (type == SGD_CERT_SUBJECT_OU)
        {
            info_str = Capture(info_str, "OU=");
        }
        infoLen = info_str.size();
        info    = (char*)calloc(1, infoLen + 1);
        memcpy(info, info_str.c_str(), info_str.size());
    }
    else if (type == SGD_CERT_DER_PUBLIC_KEY)
    {
    }
    else if (type == SGD_CERT_DER_EXTENSIONS)
    {
    }

    return std::string(info, infoLen);
}

std::string CertSM2Helper::Capture(const std::string& str, const std::string& flag)
{
    std::string res;
    size_t      start = str.find(flag);
    size_t      end   = 0;
    if (start != std::string::npos)
    {
        size_t temp = start + flag.size();
        end         = str.substr(temp).find(";");
        if (end != std::string::npos)
        {
            res = str.substr(temp, end);
        }
    }
    return res;
}
