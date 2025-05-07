#ifndef __CERTX509HELPER_H__
#define __CERTX509HELPER_H__

#include <string>
#include <vector>
#include <map>

#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/x509_vfy.h>
#include <openssl/objects.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bio.h>

class CertX509Helper
{
public:
    CertX509Helper();
    ~CertX509Helper();

    // 加载证书（PEM 或 DER 格式）
    bool loadFromFile(const std::string& filePath);
    bool loadFromString(const std::string& certData);
    bool isLoadCert();

    // 获取证书信息
    std::string getVersion() const;
    std::string getSerialNumber() const;
    std::string getIssuer() const;
    std::string getSubject() const;
    std::string getSubject(X509* cert) const;
    std::string getNotBefore() const;
    std::string getNotAfter() const;
    std::string getPublicKeyAlgorithm() const;
    std::string getPublicKeyValue() const; // 获取公钥（16进制，SM2/RSA）
    std::string getSignatureAlgorithm() const;
    std::string getSignatureValue() const;

    std::vector<std::pair<std::string, std::string>> parseExtensions();

    std::string Capture(const std::string& str, const std::string& flag, const std::string& separator);

    bool LoadSystemRootCerts(X509_STORE* store);
    bool LoadIntermediateCerts(X509_STORE* store, const std::string& dirPath);
    bool buildChain();

    const std::vector<X509*>& getChain() const;

private:
    struct CertX509HelperImpl;
    CertX509HelperImpl* m_impl;
};

#endif