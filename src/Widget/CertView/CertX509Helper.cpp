#include <fstream>
#include <sstream>
#include <iomanip>
#include <time.h>

#ifdef _WIN32
    #include <Windows.h>
    #pragma comment(lib, "crypt32.lib")
#endif

#include "CertX509Helper.h"

// 跨平台时间转换工具函数
static time_t cross_platform_timegm(struct tm* tm)
{
#ifdef _WIN32
    return _mkgmtime(tm); // Windows专用
#else
    return timegm(tm); // POSIX系统
#endif
}

// 跨平台localtime_r实现
static struct tm* cross_platform_localtime_r(const time_t* timep, struct tm* result)
{
#ifdef _WIN32
    localtime_s(result, timep);
    return result;
#else
    return localtime_r(timep, result);
#endif
}

static struct tm* gmtime_r(const time_t* timep, struct tm* result)
{
#ifdef _WIN32
    return gmtime_s(result, timep) == 0 ? result : nullptr;
#else
    return ::gmtime_r(timep, result);
#endif
}

// 获取时区偏移(小时)
static int get_timezone_offset()
{
    time_t    t  = time(nullptr);
    struct tm lt = {0};
    struct tm gt = {0};

    cross_platform_localtime_r(&t, &lt);
    gmtime_r(&t, &gt);

    return (lt.tm_hour - gt.tm_hour) * 3600 + (lt.tm_min - gt.tm_min) * 60;
}

static std::string ASN1_TIME_toString(const ASN1_TIME* time)
{
    if (!time)
        return "";

    // 1. 转换为UTC时间
    struct tm tm_utc;
    if (!ASN1_TIME_to_tm(time, &tm_utc))
    {
        return "";
    }

    // 2. 转换为本地时间
    time_t    t = cross_platform_timegm(&tm_utc);
    struct tm tm_local;
    if (!cross_platform_localtime_r(&t, &tm_local))
    {
        return "";
    }

    // 3. 格式化为本地时间字符串
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_local);

    return buf;
}

static std::string BN_toHex(const BIGNUM* bn)
{
    if (!bn)
        return "";
    char*       hex = BN_bn2hex(bn);
    std::string result(hex);
    OPENSSL_free(hex);
    return result;
}

struct CertX509Helper::CertX509HelperImpl
{
    X509*              cert;
    std::vector<X509*> certChain;

    CertX509HelperImpl()
    {
        cert = nullptr;
        certChain.clear();
    }
    ~CertX509HelperImpl()
    {
        if (cert)
        {
            X509_free(cert);
        }
        cert = nullptr;
        certChain.clear();
    }
};

CertX509Helper::CertX509Helper()
    : m_impl(new CertX509HelperImpl())
{
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
}

CertX509Helper::~CertX509Helper()
{
    EVP_cleanup();
    delete m_impl;
}

bool CertX509Helper::loadFromFile(const std::string& filePath)
{
    FILE* fp = fopen(filePath.c_str(), "r");
    if (!fp)
        return false;

    // PEM
    m_impl->cert = PEM_read_X509(fp, nullptr, nullptr, nullptr);
    // DER
    if (m_impl->cert == nullptr)
    {
        fp           = fopen(filePath.c_str(), "r");
        m_impl->cert = d2i_X509_fp(fp, nullptr);
    }

    fclose(fp);
    return m_impl->cert != nullptr;
}

bool CertX509Helper::loadFromString(const std::string& certData)
{
    BIO* bio = BIO_new_mem_buf(certData.data(), certData.size());
    if (!bio)
        return false;

    m_impl->cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
    if (m_impl->cert == nullptr)
    {
        bio          = BIO_new_mem_buf(certData.data(), certData.size());
        m_impl->cert = d2i_X509_bio(bio, nullptr);
    }

    BIO_free(bio);
    return m_impl->cert != nullptr;
}

bool CertX509Helper::isLoadCert()
{
    return m_impl->cert != nullptr;
}

std::string CertX509Helper::getVersion() const
{
    if (!m_impl->cert)
        return "";
    std::string version = std::to_string(X509_get_version(m_impl->cert) + 1);
    return "V" + version;
}

std::string CertX509Helper::getSerialNumber() const
{
    if (!m_impl->cert)
        return "";
    ASN1_INTEGER* serial = X509_get_serialNumber(m_impl->cert);
    BIGNUM*       bn     = ASN1_INTEGER_to_BN(serial, nullptr);
    std::string   hex    = BN_toHex(bn);
    BN_free(bn);
    return hex;
}

std::string CertX509Helper::getIssuer() const
{
    if (!m_impl->cert)
        return "";

    BIO* bio = BIO_new(BIO_s_mem());
    X509_NAME_print_ex(bio, X509_get_issuer_name(m_impl->cert), 0, XN_FLAG_ONELINE & ~ASN1_STRFLGS_ESC_MSB);

    char buf[1024];
    int  len = BIO_read(bio, buf, sizeof(buf) - 1);
    BIO_free(bio);

    return len > 0 ? std::string(buf, len) : "";
}

std::string CertX509Helper::getSubject() const
{
    if (!m_impl->cert)
        return "";

    BIO* bio = BIO_new(BIO_s_mem());
    X509_NAME_print_ex(bio, X509_get_subject_name(m_impl->cert), 0, XN_FLAG_ONELINE & ~ASN1_STRFLGS_ESC_MSB);

    char buf[1024];
    int  len = BIO_read(bio, buf, sizeof(buf) - 1);
    BIO_free(bio);

    return len > 0 ? std::string(buf, len) : "";
}

std::string CertX509Helper::getSubject(X509* cert) const
{
    if (!cert)
        return "";

    BIO* bio = BIO_new(BIO_s_mem());
    X509_NAME_print_ex(bio, X509_get_subject_name(cert), 0, XN_FLAG_ONELINE & ~ASN1_STRFLGS_ESC_MSB);

    char buf[1024];
    int  len = BIO_read(bio, buf, sizeof(buf) - 1);
    BIO_free(bio);

    return len > 0 ? std::string(buf, len) : "";
}

std::string CertX509Helper::getNotBefore() const
{
    if (!m_impl->cert)
        return "";
    return ASN1_TIME_toString(X509_get_notBefore(m_impl->cert));
}

std::string CertX509Helper::getNotAfter() const
{
    if (!m_impl->cert)
        return "";
    return ASN1_TIME_toString(X509_get_notAfter(m_impl->cert));
}

std::string CertX509Helper::getPublicKeyAlgorithm() const
{
    if (!m_impl->cert)
        return "";

    EVP_PKEY* pkey = X509_get_pubkey(m_impl->cert);
    if (!pkey)
        return "";

    int         nid  = EVP_PKEY_id(pkey);
    const char* name = OBJ_nid2ln(nid);
    if (name != nullptr)
    {
        return name;
    }

    // 获取失败则返回OID字符串（如 "1.2.156.10197.1.301"）
    char oid_buf[128];
    OBJ_obj2txt(oid_buf, sizeof(oid_buf), OBJ_nid2obj(nid), 1);
    std::string algName = oid_buf;
    EVP_PKEY_free(pkey);

    return algName;
}

std::string CertX509Helper::getPublicKeyValue() const
{
    if (!m_impl->cert)
        return "";

    EVP_PKEY* pkey = X509_get_pubkey(m_impl->cert);
    if (!pkey)
        return "";

    // 将公钥转换为DER格式
    unsigned char* der = nullptr;
    int            len = i2d_PUBKEY(pkey, &der);
    EVP_PKEY_free(pkey);

    if (len <= 0 || !der)
        return "";

    // 转换为16进制字符串
    std::ostringstream oss;
    for (int i = 0; i < len; i++)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)der[i];
    }
    OPENSSL_free(der);

    return oss.str();
}

std::string CertX509Helper::getSignatureAlgorithm() const
{
    if (!m_impl->cert)
        return "";

    int         nid  = X509_get_signature_nid(m_impl->cert);
    const char* name = OBJ_nid2ln(nid);
    if (name != nullptr)
    {
        return name;
    }

    // 获取失败则返回OID字符串（如 "1.2.156.10197.1.301"）
    char oid_buf[128];
    OBJ_obj2txt(oid_buf, sizeof(oid_buf), OBJ_nid2obj(nid), 1);
    return oid_buf;
}

std::string CertX509Helper::getSignatureValue() const
{
    if (!m_impl->cert)
        return "";

    const ASN1_BIT_STRING* sig = nullptr;
    X509_get0_signature(&sig, nullptr, m_impl->cert);
    if (!sig)
        return "";

    std::ostringstream oss;
    for (int i = 0; i < sig->length; i++)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)sig->data[i];
    }

    return oss.str();
}

std::vector<std::pair<std::string, std::string>> CertX509Helper::parseExtensions()
{
    std::vector<std::pair<std::string, std::string>> extensions;

    if (!m_impl->cert)
        return extensions;

    // 可选：中文名称映射（你也可以外部传入）
    static const std::map<std::string, std::string> nameMap = {
        {"basicConstraints", "基本约束"},
        {"keyUsage", "密钥用途"},
        {"subjectKeyIdentifier", "主题密钥标识符"},
        {"authorityKeyIdentifier", "颁发者密钥标识符"},
        {"subjectAltName", "备用名称"},
        {"issuerAltName", "颁发者备用名称"},
        {"extKeyUsage", "扩展密钥用途"},
        {"crlDistributionPoints", "CRL 分发点"},
        {"certificatePolicies", "证书策略"},
        {"authorityInfoAccess", "授权信息访问"},
        {"SM2Sign", "SM2 签名"},
    };

    int ext_count = X509_get_ext_count(m_impl->cert);
    for (int i = 0; i < ext_count; ++i)
    {
        X509_EXTENSION* ext = X509_get_ext(m_impl->cert, i);
        if (!ext)
            continue;

        ASN1_OBJECT* obj = X509_EXTENSION_get_object(ext);

        std::string name;
        int         nid = OBJ_obj2nid(obj);
        if (nid != NID_undef)
        {
            const char* ln = OBJ_nid2sn(nid);
            if (ln && *ln)
            {
                auto it = nameMap.find(ln);
                name    = (it != nameMap.end()) ? it->second : ln;
            }
            else
            {
                char obj_txt[80];
                OBJ_obj2txt(obj_txt, sizeof(obj_txt), obj, 1);
                name = obj_txt;
            }
        }
        else
        {
            // 若无 NID，回退为 OID
            char obj_txt[80];
            OBJ_obj2txt(obj_txt, sizeof(obj_txt), obj, 1);
            name = obj_txt;
        }

        // 尝试打印为人类可读字符串
        BIO* bio = BIO_new(BIO_s_mem());
        if (X509V3_EXT_print(bio, ext, 0, 0))
        {
            char buf[1024] = {};
            int  len       = BIO_read(bio, buf, sizeof(buf) - 1);
            buf[len]       = '\0';
            extensions.emplace_back(name, std::string(buf));
        }
        else
        {
            // 失败则转为 hex
            ASN1_OCTET_STRING* data = X509_EXTENSION_get_data(ext);
            std::string        hex;
            for (int j = 0; j < data->length; ++j)
            {
                char byte[4];
                snprintf(byte, sizeof(byte), "%02X", data->data[j]);
                hex += byte;
            }
            extensions.emplace_back(name, hex);
        }

        BIO_free(bio);
    }

    return extensions;
}

std::string CertX509Helper::Capture(const std::string& str, const std::string& flag, const std::string& separator)
{
    std::string res;
    size_t      start = str.find(flag);
    size_t      end   = 0;
    if (start != std::string::npos)
    {
        size_t temp = start + flag.size();
        end         = str.substr(temp).find(separator);
        if (end != std::string::npos)
        {
            res = str.substr(temp, end);
        }
        else
        {
            res = str.substr(temp);
        }
    }
    return res;
}

bool CertX509Helper::buildChain()
{
    m_impl->certChain.clear();
    if (!m_impl->cert)
        return false;

    X509_STORE* store = X509_STORE_new();
    if (!store)
        return false;

    if (X509_STORE_set_default_paths(store) != 1)
    {
        X509_STORE_free(store);
        return false;
    }
    LoadIntermediateCerts(store, "certs");

#ifdef _WIN32
    if (!LoadSystemRootCerts(store))
    {
        X509_STORE_free(store);
        return false;
    }
#endif

    X509_STORE_CTX* ctx = X509_STORE_CTX_new();
    if (!ctx)
    {
        X509_STORE_free(store);
        return false;
    }

    if (X509_STORE_CTX_init(ctx, store, m_impl->cert, nullptr) != 1)
    {
        X509_STORE_CTX_free(ctx);
        X509_STORE_free(store);
        return false;
    }

    if (X509_verify_cert(ctx) != 1)
    {
        // 获取错误代码
        int err_code = X509_STORE_CTX_get_error(ctx);
        // 获取错误信息
        const char* err_msg = X509_verify_cert_error_string(err_code);
        printf("Certificate verification failed: %s\n", err_msg);
        X509_STORE_CTX_free(ctx);
        X509_STORE_free(store);
        return false;
    }

    STACK_OF(X509)* chain = X509_STORE_CTX_get1_chain(ctx);
    if (!chain)
    {
        X509_STORE_CTX_free(ctx);
        X509_STORE_free(store);
        return false;
    }

    for (int i = 0; i < sk_X509_num(chain); ++i)
    {
        X509* cert = sk_X509_value(chain, i);
        X509_up_ref(cert);
        m_impl->certChain.push_back(cert);
    }

    sk_X509_pop_free(chain, X509_free);
    X509_STORE_CTX_free(ctx);
    X509_STORE_free(store);
    return true;
}

const std::vector<X509*>& CertX509Helper::getChain() const
{
    return m_impl->certChain;
}

bool CertX509Helper::LoadSystemRootCerts(X509_STORE* store)
{
    HCERTSTORE hSystemStore = CertOpenSystemStoreA(0, "ROOT");
    if (!hSystemStore)
        return false;

    PCCERT_CONTEXT pContext = nullptr;
    while ((pContext = CertEnumCertificatesInStore(hSystemStore, pContext)) != nullptr)
    {
        const unsigned char* pData = pContext->pbCertEncoded;
        X509*                cert  = d2i_X509(NULL, &pData, pContext->cbCertEncoded);
        if (cert)
        {
            X509_STORE_add_cert(store, cert);
            X509_free(cert);
        }
    }

    CertCloseStore(hSystemStore, 0);
    return true;
}

// 从文件加载证书并添加到 store
bool AddCertificateFromFile(X509_STORE* store, const std::string& filePath)
{
    FILE* fp = fopen(filePath.c_str(), "r");
    if (!fp)
    {
        printf("Failed to open file: %s\n", filePath.c_str());
        return false;
    }

    // PEM
    X509* cert = PEM_read_X509(fp, nullptr, nullptr, nullptr);
    // DER
    if (cert == nullptr)
    {
        fp   = fopen(filePath.c_str(), "r");
        cert = d2i_X509_fp(fp, nullptr);
    }
    fclose(fp);

    if (!cert)
    {
        printf("Failed to parse certificate: %s\n", filePath.c_str());
        return false;
    }

    if (X509_STORE_add_cert(store, cert) != 1)
    {
        printf("Failed to add certificate to store: %s\n", filePath.c_str());
        X509_free(cert);
        return false;
    }

    X509_free(cert);
    return true;
}

bool CertX509Helper::LoadIntermediateCerts(X509_STORE* store, const std::string& dirPath)
{
    if (!store || dirPath.empty())
    {
        return false;
    }

#ifdef _WIN32
    // Windows 实现（使用 FindFirstFile/FindNextFile）
    WIN32_FIND_DATA findData;
    HANDLE          hFind = FindFirstFile((dirPath + "\\*").c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open directory: %s\n", dirPath.c_str());
        return false;
    }

    do
    {
        const std::string fileName = findData.cFileName;
        if (fileName == "." || fileName == "..")
        {
            continue;
        }

        const std::string filePath = dirPath + "\\" + fileName;
        AddCertificateFromFile(store, filePath);
    } while (FindNextFile(hFind, &findData) != 0);

    FindClose(hFind);

#else
    // Linux/macOS 实现（使用 opendir/readdir）
    DIR* dir = opendir(dirPath.c_str());
    if (!dir)
    {
        printf("Failed to open directory: %s\n", dirPath.c_str());
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        const std::string fileName = entry->d_name;
        if (fileName == "." || fileName == "..")
        {
            continue;
        }

        const std::string filePath = dirPath + "/" + fileName;
        AddCertificateFromFile(store, filePath);
    }

    closedir(dir);
#endif

    return true;
}
