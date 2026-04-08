#ifndef __SM2_PKCS7_VERIFY_TYPES_H__
#define __SM2_PKCS7_VERIFY_TYPES_H__

#include <cstddef>
#include <string>

namespace core
{
enum class Sm2Pkcs7VerifyError
{
    None,
    Validation,
    Base64Decode,
    Asn1Parse,
    Pkcs7Structure,
    CertificateParse,
    PublicKey,
    VerifyFailed,
};

struct Sm2Pkcs7VerifyRequest
{
    std::string signatureBase64;
    std::string originalData;
    std::string userId;
};

struct Sm2Pkcs7VerifyResult
{
    bool                  success = false;
    Sm2Pkcs7VerifyError   error = Sm2Pkcs7VerifyError::None;
    std::string           message;
    std::string           signerSubject;
    std::string           signerIssuer;
    std::string           publicKeyHex;
    std::string           signatureRHex;
    std::string           signatureSHex;
    std::size_t           decodedSignatureBytes = 0U;
    bool                  parsedSignature = false;
    bool                  parsedPublicKey = false;
};
}

#endif
