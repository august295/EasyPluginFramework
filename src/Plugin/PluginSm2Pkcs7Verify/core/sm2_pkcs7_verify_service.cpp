#include "sm2_pkcs7_verify_service.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <endecode/asn1/asn1.h>
#include <endecode/asn1/asn1_helper.h>
#include <endecode/asn1/cert_sm2.h>
#include <endecode/base64/base64.h>
#include <endecode/ecc/ecc.h>
#include <endecode/sm2/sm2.h>

namespace
{
using Asn1TreePtr = std::unique_ptr<easy_asn1_tree_st, decltype(&easy_asn1_free_tree)>;
using CertificatePtr = std::unique_ptr<SM2Certificate, decltype(&sm2_cert_free)>;
struct MpzHolder
{
    mpz_t value;

    MpzHolder()
    {
        mpz_init(value);
    }

    ~MpzHolder()
    {
        mpz_clear(value);
    }
};

struct EccKeyHolder
{
    ecc_key_st key;
    bool       initialized;

    EccKeyHolder()
        : initialized(false)
    {
        sm2_init_set(&key.curve, &key.G);
        ecc_point_init(&key.public_key);
        mpz_init(key.private_key);
        initialized = true;
    }

    ~EccKeyHolder()
    {
        if (initialized)
        {
            ecc_key_st_clear(&key);
        }
    }
};

std::string trimWhitespace(const std::string& text)
{
    std::string trimmed;
    trimmed.reserve(text.size());
    for (const char ch : text)
    {
        if (!std::isspace(static_cast<unsigned char>(ch)))
        {
            trimmed.push_back(ch);
        }
    }
    return trimmed;
}

std::string bytesToHex(const uint8_t* data, const std::size_t length)
{
    if (data == nullptr || length == 0U)
    {
        return "";
    }
    std::ostringstream stream;
    stream << std::uppercase << std::hex << std::setfill('0');
    for (std::size_t index = 0; index < length; ++index)
    {
        stream << std::setw(2) << static_cast<int>(data[index]);
    }
    return stream.str();
}

std::string asn1StringToText(const easy_asn1_string_st& value)
{
    if (value.value == nullptr || value.length == 0U)
    {
        return "";
    }
    if (easy_asn1_print_string_try(value.value, value.length) == 1)
    {
        return std::string(reinterpret_cast<const char*>(value.value), value.length);
    }
    return bytesToHex(value.value, value.length);
}

std::string extractRawContent(const uint8_t* data, const std::size_t length)
{
    if (data == nullptr || length == 0U)
    {
        return "";
    }

    const std::size_t prefixLength = data[0] == 0U ? 1U : 0U;
    const uint8_t*    textData = data + prefixLength;
    const std::size_t textLength = length - prefixLength;
    if (textLength == 0U)
    {
        return "";
    }

    return std::string(reinterpret_cast<const char*>(textData), textLength);
}

std::string rawContentToDisplayText(const std::string& rawContent)
{
    if (rawContent.empty())
    {
        return "";
    }

    bool printable = true;
    for (const unsigned char ch : rawContent)
    {
        if (!std::isprint(ch) && !std::isspace(ch))
        {
            printable = false;
            break;
        }
    }

    if (printable)
    {
        return rawContent;
    }
    return bytesToHex(reinterpret_cast<const uint8_t*>(rawContent.data()), rawContent.size());
}

std::string formatName(const Name& name)
{
    std::ostringstream stream;
    bool hasValue = false;
    for (uint32_t nameIndex = 0; nameIndex < name.count; ++nameIndex)
    {
        const RelativeDistinguishedName& rdn = name.names[nameIndex];
        for (uint32_t rdnIndex = 0; rdnIndex < rdn.count; ++rdnIndex)
        {
            const std::string text = asn1StringToText(rdn.rdn[rdnIndex].value);
            if (text.empty())
            {
                continue;
            }
            if (hasValue)
            {
                stream << ", ";
            }
            stream << text;
            hasValue = true;
        }
    }
    return hasValue ? stream.str() : "";
}

easy_asn1_tree_st* getTreeItem(easy_asn1_tree_st* node, const int index)
{
    return node == nullptr ? nullptr : easy_asn1_get_tree_item(node, index);
}

easy_asn1_tree_st* findEmbeddedContentValueTree(easy_asn1_tree_st* contentInfoTree)
{
    easy_asn1_tree_st* explicitContentTree = getTreeItem(contentInfoTree, 1);
    if (explicitContentTree == nullptr)
    {
        return nullptr;
    }

    easy_asn1_tree_st* current = explicitContentTree;
    while (current != nullptr && current->first_child != nullptr)
    {
        current = current->first_child;
    }
    return current;
}

core::Sm2Pkcs7VerifyResult makeError(const core::Sm2Pkcs7VerifyError error, const std::string& message)
{
    core::Sm2Pkcs7VerifyResult result = {};
    result.error = error;
    result.message = message;
    return result;
}
}

namespace core
{
Sm2Pkcs7VerifyResult Sm2Pkcs7VerifyService::verify(const Sm2Pkcs7VerifyRequest& request) const
{
    const std::string signatureBase64 = trimWhitespace(request.signatureBase64);
    if (signatureBase64.empty())
    {
        return makeError(Sm2Pkcs7VerifyError::Validation, "签名数据不能为空");
    }
    if (!request.hasEmbeddedOriginal && request.originalData.empty())
    {
        return makeError(Sm2Pkcs7VerifyError::Validation, "原文数据不能为空");
    }
    if (request.userId.empty())
    {
        return makeError(Sm2Pkcs7VerifyError::Validation, "SM2 ID 不能为空");
    }

    std::vector<uint8_t> decodedData(BASE64_DECODE_OUT_SIZE(signatureBase64.size()));
    const std::size_t decodedLength = base64_decode(signatureBase64.c_str(), signatureBase64.size(), decodedData.data());
    if (decodedLength == 0U)
    {
        return makeError(Sm2Pkcs7VerifyError::Base64Decode, "签名数据 Base64 解码失败");
    }
    decodedData.resize(decodedLength);

    easy_asn1_tree_st* rawTree = static_cast<easy_asn1_tree_st*>(std::malloc(sizeof(easy_asn1_tree_st)));
    if (rawTree == nullptr)
    {
        return makeError(Sm2Pkcs7VerifyError::Asn1Parse, "ASN.1 树分配失败");
    }

    easy_asn1_parse(decodedData.data(), decodedData.size(), 0U, 0U, &rawTree);
    Asn1TreePtr tree(rawTree, &easy_asn1_free_tree);
    if (rawTree == nullptr)
    {
        return makeError(Sm2Pkcs7VerifyError::Asn1Parse, "PKCS7 ASN.1 解析失败");
    }

    easy_asn1_tree_st* contentTree = getTreeItem(rawTree, 1);
    easy_asn1_tree_st* signedDataTree = getTreeItem(contentTree, 0);
    easy_asn1_tree_st* contentInfoTree = getTreeItem(signedDataTree, 2);
    easy_asn1_tree_st* signerInfosTree = getTreeItem(signedDataTree, 4);
    easy_asn1_tree_st* signerInfoTree = getTreeItem(signerInfosTree, 0);
    easy_asn1_tree_st* signatureTree = getTreeItem(signerInfoTree, 4);
    easy_asn1_tree_st* signatureSequenceTree = getTreeItem(signatureTree, 0);
    easy_asn1_tree_st* rTree = getTreeItem(signatureSequenceTree, 0);
    easy_asn1_tree_st* sTree = getTreeItem(signatureSequenceTree, 1);
    easy_asn1_tree_st* certTree = getTreeItem(signedDataTree, 3);

    if (contentTree == nullptr || signedDataTree == nullptr || signerInfosTree == nullptr || signerInfoTree == nullptr ||
        contentInfoTree == nullptr || signatureTree == nullptr || signatureSequenceTree == nullptr || rTree == nullptr || sTree == nullptr ||
        certTree == nullptr)
    {
        return makeError(Sm2Pkcs7VerifyError::Pkcs7Structure, "PKCS7 结构不符合预期，无法定位原文、签名值或签名证书");
    }

    const easy_asn1_string_st rValue = rTree->value;
    const easy_asn1_string_st sValue = sTree->value;
    const easy_asn1_string_st certValue = certTree->value;
    if (rValue.value == nullptr || sValue.value == nullptr || certValue.value == nullptr)
    {
        return makeError(Sm2Pkcs7VerifyError::Pkcs7Structure, "PKCS7 结构中的签名值或证书为空");
    }

    CertificatePtr certificate(sm2_cert_parse(certValue.value, certValue.length), &sm2_cert_free);
    if (!certificate)
    {
        return makeError(Sm2Pkcs7VerifyError::CertificateParse, "签名证书解析失败");
    }

    const easy_asn1_string_st publicKey = certificate->tbsCertificate.subjectPublicKeyInfo.subjectPublicKey;
    if (publicKey.value == nullptr || publicKey.length < 66U)
    {
        return makeError(Sm2Pkcs7VerifyError::PublicKey, "签名证书中未找到有效的 SM2 公钥");
    }

    EccKeyHolder keyHolder;
    mpz_import(keyHolder.key.public_key.x, 32, 1, 1, 1, 0, publicKey.value + 2);
    mpz_import(keyHolder.key.public_key.y, 32, 1, 1, 1, 0, publicKey.value + 34);

    MpzHolder r;
    MpzHolder s;
    mpz_import(r.value, rValue.length, 1, 1, 1, 0, rValue.value);
    mpz_import(s.value, sValue.length, 1, 1, 1, 0, sValue.value);

    Sm2Pkcs7VerifyResult result = {};
    result.decodedSignatureBytes = decodedData.size();
    result.signerSubject = formatName(certificate->tbsCertificate.subject);
    result.signerIssuer = formatName(certificate->tbsCertificate.issuer);
    result.publicKeyHex = bytesToHex(publicKey.value, publicKey.length);
    result.signatureRHex = bytesToHex(rValue.value, rValue.length);
    result.signatureSHex = bytesToHex(sValue.value, sValue.length);
    result.parsedSignature = true;
    result.parsedPublicKey = true;

    std::string verifyOriginalData = request.originalData;
    if (request.hasEmbeddedOriginal)
    {
        easy_asn1_tree_st* embeddedContentTree = findEmbeddedContentValueTree(contentInfoTree);
        if (embeddedContentTree == nullptr || embeddedContentTree->value.value == nullptr)
        {
            return makeError(Sm2Pkcs7VerifyError::Pkcs7Structure, "PKCS7 中未找到携带的原文数据");
        }

        verifyOriginalData = extractRawContent(embeddedContentTree->value.value, embeddedContentTree->value.length);
        if (verifyOriginalData.empty())
        {
            return makeError(Sm2Pkcs7VerifyError::Pkcs7Structure, "PKCS7 中携带的原文数据为空");
        }

        result.embeddedOriginalText = rawContentToDisplayText(verifyOriginalData);
        result.parsedEmbeddedOriginal = true;
    }

    const int verifyRet = sm2_verify(
        reinterpret_cast<const uint8_t*>(verifyOriginalData.data()),
        verifyOriginalData.size(),
        reinterpret_cast<const uint8_t*>(request.userId.data()),
        request.userId.size(),
        &keyHolder.key,
        r.value,
        s.value);

    if (verifyRet == 1)
    {
        result.success = true;
        result.error = Sm2Pkcs7VerifyError::None;
        result.message = "PKCS7 验签成功";
        return result;
    }

    result.error = Sm2Pkcs7VerifyError::VerifyFailed;
    result.message = request.hasEmbeddedOriginal ? "PKCS7 验签失败，请检查签名数据中携带的原文或 SM2 ID 是否匹配"
                                                 : "PKCS7 验签失败，请检查原文数据或 SM2 ID 是否匹配";
    return result;
}
}
