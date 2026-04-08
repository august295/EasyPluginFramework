#include "sm2_csr_verify_service.h"

#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <endecode/asn1/asn1.h>
#include <endecode/asn1/asn1_helper.h>
#include <endecode/base64/base64.h>
#include <endecode/ecc/ecc.h>
#include <endecode/sm2/sm2.h>

namespace
{
using Asn1TreePtr = std::unique_ptr<easy_asn1_tree_st, decltype(&easy_asn1_free_tree)>;

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

    EccKeyHolder()
    {
        sm2_init_set(&key.curve, &key.G);
        ecc_point_init(&key.public_key);
        mpz_init(key.private_key);
    }

    ~EccKeyHolder()
    {
        ecc_key_st_clear(&key);
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

std::string bytesToDisplayText(const uint8_t* data, const std::size_t length)
{
    if (data == nullptr || length == 0U)
    {
        return "";
    }
    if (easy_asn1_print_string_try(data, length) == 1)
    {
        return std::string(reinterpret_cast<const char*>(data), length);
    }
    return bytesToHex(data, length);
}

easy_asn1_tree_st* getTreeItem(easy_asn1_tree_st* node, const int index)
{
    return node == nullptr ? nullptr : easy_asn1_get_tree_item(node, index);
}

void collectLeafTexts(easy_asn1_tree_st* node, std::vector<std::string>& values)
{
    if (node == nullptr)
    {
        return;
    }

    if (node->first_child == nullptr)
    {
        const std::string text = bytesToDisplayText(node->value.value, node->value.length);
        if (!text.empty())
        {
            values.push_back(text);
        }
        return;
    }

    for (easy_asn1_tree_st* child = node->first_child; child != nullptr; child = child->next_sibling)
    {
        collectLeafTexts(child, values);
    }
}

std::string summarizeSubject(easy_asn1_tree_st* subjectTree)
{
    std::vector<std::string> values;
    collectLeafTexts(subjectTree, values);

    std::ostringstream stream;
    bool first = true;
    for (const std::string& value : values)
    {
        if (!first)
        {
            stream << ", ";
        }
        stream << value;
        first = false;
    }
    return stream.str();
}

core::Sm2CsrVerifyResult makeError(const core::Sm2CsrVerifyError error, const std::string& message)
{
    core::Sm2CsrVerifyResult result = {};
    result.error = error;
    result.message = message;
    return result;
}
}

namespace core
{
Sm2CsrVerifyResult Sm2CsrVerifyService::verify(const Sm2CsrVerifyRequest& request) const
{
    const std::string csrBase64 = trimWhitespace(request.csrBase64);
    if (csrBase64.empty())
    {
        return makeError(Sm2CsrVerifyError::Validation, "CSR 数据不能为空");
    }
    if (request.userId.empty())
    {
        return makeError(Sm2CsrVerifyError::Validation, "SM2 ID 不能为空");
    }

    std::vector<uint8_t> decodedData(BASE64_DECODE_OUT_SIZE(csrBase64.size()));
    const std::size_t decodedLength = base64_decode(csrBase64.c_str(), csrBase64.size(), decodedData.data());
    if (decodedLength == 0U)
    {
        return makeError(Sm2CsrVerifyError::Base64Decode, "CSR Base64 解码失败");
    }
    decodedData.resize(decodedLength);

    easy_asn1_tree_st* rawTree = static_cast<easy_asn1_tree_st*>(std::malloc(sizeof(easy_asn1_tree_st)));
    if (rawTree == nullptr)
    {
        return makeError(Sm2CsrVerifyError::Asn1Parse, "ASN.1 树分配失败");
    }

    easy_asn1_parse(decodedData.data(), decodedData.size(), 0U, 0U, &rawTree);
    Asn1TreePtr tree(rawTree, &easy_asn1_free_tree);
    if (rawTree == nullptr)
    {
        return makeError(Sm2CsrVerifyError::Asn1Parse, "CSR ASN.1 解析失败");
    }

    easy_asn1_tree_st* certRequestInfoTree = getTreeItem(rawTree, 0);
    easy_asn1_tree_st* subjectTree = getTreeItem(certRequestInfoTree, 1);
    easy_asn1_tree_st* subjectPublicKeyInfoTree = getTreeItem(certRequestInfoTree, 2);
    easy_asn1_tree_st* publicKeyTree = getTreeItem(subjectPublicKeyInfoTree, 1);
    easy_asn1_tree_st* signatureTree = getTreeItem(rawTree, 2);
    easy_asn1_tree_st* signatureSequenceTree = getTreeItem(signatureTree, 0);
    easy_asn1_tree_st* rTree = getTreeItem(signatureSequenceTree, 0);
    easy_asn1_tree_st* sTree = getTreeItem(signatureSequenceTree, 1);

    if (certRequestInfoTree == nullptr || subjectTree == nullptr || subjectPublicKeyInfoTree == nullptr || publicKeyTree == nullptr ||
        signatureTree == nullptr || signatureSequenceTree == nullptr || rTree == nullptr || sTree == nullptr)
    {
        return makeError(Sm2CsrVerifyError::CsrStructure, "CSR 结构不符合预期，无法定位请求信息、公钥或签名值");
    }

    const std::size_t serializedLength = easy_asn1_serialize(certRequestInfoTree, nullptr);
    if (serializedLength == 0U)
    {
        return makeError(Sm2CsrVerifyError::Serialize, "CertificationRequestInfo 序列化失败");
    }

    std::vector<uint8_t> serializedRequest(serializedLength);
    const std::size_t writtenLength = easy_asn1_serialize(certRequestInfoTree, serializedRequest.data());
    if (writtenLength == 0U)
    {
        return makeError(Sm2CsrVerifyError::Serialize, "CertificationRequestInfo 序列化失败");
    }
    serializedRequest.resize(writtenLength);

    const easy_asn1_string_st publicKeyValue = publicKeyTree->value;
    const easy_asn1_string_st rValue = rTree->value;
    const easy_asn1_string_st sValue = sTree->value;
    if (publicKeyValue.value == nullptr || publicKeyValue.length < 66U || rValue.value == nullptr || sValue.value == nullptr)
    {
        return makeError(Sm2CsrVerifyError::PublicKey, "CSR 中未找到有效的 SM2 公钥或签名值");
    }

    EccKeyHolder keyHolder;
    mpz_import(keyHolder.key.public_key.x, 32, 1, 1, 1, 0, publicKeyValue.value + 2);
    mpz_import(keyHolder.key.public_key.y, 32, 1, 1, 1, 0, publicKeyValue.value + 34);

    MpzHolder r;
    MpzHolder s;
    mpz_import(r.value, rValue.length, 1, 1, 1, 0, rValue.value);
    mpz_import(s.value, sValue.length, 1, 1, 1, 0, sValue.value);

    Sm2CsrVerifyResult result = {};
    result.decodedCsrBytes = decodedData.size();
    result.serializedRequestBytes = serializedRequest.size();
    result.subjectSummary = summarizeSubject(subjectTree);
    result.publicKeyHex = bytesToHex(publicKeyValue.value, publicKeyValue.length);
    result.signatureRHex = bytesToHex(rValue.value, rValue.length);
    result.signatureSHex = bytesToHex(sValue.value, sValue.length);
    result.parsedRequestInfo = true;
    result.parsedSignature = true;
    result.parsedPublicKey = true;

    const int verifyRet = sm2_verify(
        serializedRequest.data(),
        serializedRequest.size(),
        reinterpret_cast<const uint8_t*>(request.userId.data()),
        request.userId.size(),
        &keyHolder.key,
        r.value,
        s.value);

    if (verifyRet == 1)
    {
        result.success = true;
        result.error = Sm2CsrVerifyError::None;
        result.message = "CSR 验证成功";
        return result;
    }

    result.error = Sm2CsrVerifyError::VerifyFailed;
    result.message = "CSR 验证失败，请检查 CSR 数据或 SM2 ID 是否匹配";
    return result;
}
}
