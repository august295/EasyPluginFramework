#ifndef __SM2_CSR_VERIFY_TYPES_H__
#define __SM2_CSR_VERIFY_TYPES_H__

#include <cstddef>
#include <string>

namespace core
{
enum class Sm2CsrVerifyError
{
    None,
    Validation,
    Base64Decode,
    Asn1Parse,
    CsrStructure,
    Serialize,
    PublicKey,
    VerifyFailed,
};

struct Sm2CsrVerifyRequest
{
    std::string csrBase64;
    std::string userId;
};

struct Sm2CsrVerifyResult
{
    bool                success = false;
    Sm2CsrVerifyError   error = Sm2CsrVerifyError::None;
    std::string         message;
    std::string         subjectSummary;
    std::string         publicKeyHex;
    std::string         signatureRHex;
    std::string         signatureSHex;
    std::size_t         decodedCsrBytes = 0U;
    std::size_t         serializedRequestBytes = 0U;
    bool                parsedRequestInfo = false;
    bool                parsedSignature = false;
    bool                parsedPublicKey = false;
};
}

#endif
