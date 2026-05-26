#ifndef __P7B_CERTIFICATE_CHAIN_TYPES_H__
#define __P7B_CERTIFICATE_CHAIN_TYPES_H__

#include <cstddef>
#include <string>
#include <vector>

namespace core
{
enum class P7bCertificateChainError
{
    None,
    Validation,
    Base64Decode,
    Pkcs7Parse,
    Pkcs7Structure,
    CertificateParse,
    EmptyCertificates,
    ExportFailed,
};

struct P7bCertificateChainRequest
{
    std::string chainData;
};

struct P7bCertificateSummary
{
    std::size_t index = 0U;
    std::string subject;
    std::string issuer;
    std::string serialNumber;
    std::string notBefore;
    std::string notAfter;
    std::vector<uint8_t> derBytes;
    std::string pem;
};

struct P7bCertificateChainResult
{
    bool success = false;
    P7bCertificateChainError error = P7bCertificateChainError::None;
    std::string message;
    std::string fullChainPem;
    std::size_t parsedPkcs7Bytes = 0U;
    std::size_t parsedCertificateCount = 0U;
    std::vector<P7bCertificateSummary> certificates;
};
}

#endif
