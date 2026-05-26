#include "p7b_certificate_chain_service.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

extern "C"
{
#include <endecode/asn1/asn1.h>
#include <endecode/asn1/asn1_helper.h>
#include <endecode/asn1/cert.h>
#include <endecode/asn1/cert_sm2.h>
#include <endecode/base64/base64.h>
}

namespace
{
using Asn1TreePtr = std::unique_ptr<easy_asn1_tree_st, decltype(&easy_asn1_free_tree)>;
using CertificatePtr = std::unique_ptr<SM2Certificate, decltype(&sm2_cert_free)>;

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

std::string trimEdges(const std::string& text)
{
    const auto begin = std::find_if_not(text.begin(), text.end(), [](const unsigned char ch) { return std::isspace(ch) != 0; });
    if (begin == text.end())
    {
        return "";
    }

    const auto end = std::find_if_not(text.rbegin(), text.rend(), [](const unsigned char ch) { return std::isspace(ch) != 0; }).base();
    return std::string(begin, end);
}

std::string normalizeBase64(const std::string& text)
{
    std::stringstream stream(text);
    std::string line;
    std::string normalized;

    while (std::getline(stream, line))
    {
        const std::string trimmed = trimEdges(line);
        if (trimmed.empty())
        {
            continue;
        }

        if (trimmed.rfind("-----BEGIN ", 0) == 0 || trimmed.rfind("-----END ", 0) == 0)
        {
            continue;
        }

        normalized += trimmed;
    }

    if (normalized.empty())
    {
        normalized = trimWhitespace(text);
    }
    return normalized;
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

std::string formatTime(const easy_asn1_string_st& value)
{
    if (value.value == nullptr || value.length == 0U)
    {
        return "";
    }

    char timeText[21] = {0};
    if (value.tag == EASY_ASN1_UTCTIME)
    {
        convertUTCTimeToStandard(reinterpret_cast<const char*>(value.value), value.length, 8U, timeText);
        return timeText;
    }
    if (value.tag == EASY_ASN1_GENERALIZEDTIME)
    {
        convertGeneralizedTimeToStandard(reinterpret_cast<const char*>(value.value), value.length, 8U, timeText);
        return timeText;
    }

    return asn1StringToText(value);
}

easy_asn1_tree_st* getTreeItem(easy_asn1_tree_st* node, const int index)
{
    return node == nullptr ? nullptr : easy_asn1_get_tree_item(node, index);
}

easy_asn1_tree_st* findCertificatesTree(easy_asn1_tree_st* signedDataTree)
{
    if (signedDataTree == nullptr)
    {
        return nullptr;
    }

    for (easy_asn1_tree_st* child = signedDataTree->first_child; child != nullptr; child = child->next_sibling)
    {
        if ((child->value.tag & CONTEXT_SPECIFIC) != 0U && child->first_child != nullptr)
        {
            return child->first_child;
        }
    }

    return nullptr;
}

std::optional<std::vector<uint8_t>> decodePkcs7Input(const std::string& chainData)
{
    const std::string normalized = normalizeBase64(chainData);
    if (normalized.empty())
    {
        return std::nullopt;
    }

    std::vector<uint8_t> decoded(BASE64_DECODE_OUT_SIZE(normalized.size()));
    const std::size_t decodedLength = base64_decode(normalized.c_str(), normalized.size(), decoded.data());
    if (decodedLength == 0U)
    {
        return std::nullopt;
    }

    decoded.resize(decodedLength);
    return decoded;
}

std::vector<uint8_t> serializeNode(easy_asn1_tree_st* node)
{
    if (node == nullptr)
    {
        return {};
    }

    const std::size_t serializedLength = easy_asn1_serialize(node, nullptr);
    if (serializedLength == 0U)
    {
        return {};
    }

    std::vector<uint8_t> der(serializedLength);
    easy_asn1_serialize(node, der.data());
    return der;
}

std::string derToPem(const std::vector<uint8_t>& der)
{
    if (der.empty())
    {
        return "";
    }

    std::vector<char> base64(BASE64_ENCODE_OUT_SIZE(der.size()));
    const std::size_t encodedLength = base64_encode(der.data(), der.size(), base64.data());
    if (encodedLength == 0U)
    {
        return "";
    }

    std::string pem = std::string(PEM_CERT_BEGIN) + "\n";
    for (std::size_t offset = 0; offset < encodedLength; offset += 64U)
    {
        pem.append(base64.data() + offset, std::min<std::size_t>(64U, encodedLength - offset));
        pem.push_back('\n');
    }
    pem += std::string(PEM_CERT_END) + "\n";
    return pem;
}

core::P7bCertificateChainResult makeError(const core::P7bCertificateChainError error, const std::string& message)
{
    core::P7bCertificateChainResult result = {};
    result.error = error;
    result.message = message;
    return result;
}
}

namespace core
{
P7bCertificateChainResult P7bCertificateChainService::parse(const P7bCertificateChainRequest& request) const
{
    const std::string chainData = trimEdges(request.chainData);
    if (chainData.empty())
    {
        return makeError(P7bCertificateChainError::Validation, "证书链输入不能为空");
    }

    const std::optional<std::vector<uint8_t>> decoded = decodePkcs7Input(chainData);
    if (!decoded.has_value())
    {
        return makeError(P7bCertificateChainError::Base64Decode, "证书链 Base64 解码失败，请检查 p7b 内容是否完整");
    }

    easy_asn1_tree_st* rawTree = static_cast<easy_asn1_tree_st*>(std::malloc(sizeof(easy_asn1_tree_st)));
    if (rawTree == nullptr)
    {
        return makeError(P7bCertificateChainError::Pkcs7Parse, "ASN.1 树分配失败");
    }

    easy_asn1_parse(decoded->data(), decoded->size(), 0U, 0U, &rawTree);
    Asn1TreePtr tree(rawTree, &easy_asn1_free_tree);
    if (rawTree == nullptr)
    {
        return makeError(P7bCertificateChainError::Pkcs7Parse, "证书链不是受支持的 p7b/pkcs7 ASN.1 结构");
    }

    easy_asn1_tree_st* contentTypeTree = getTreeItem(rawTree, 0);
    easy_asn1_tree_st* contentTree = getTreeItem(rawTree, 1);
    easy_asn1_tree_st* signedDataTree = getTreeItem(contentTree, 0);
    easy_asn1_tree_st* certificatesTree = findCertificatesTree(signedDataTree);

    if (contentTypeTree == nullptr || contentTree == nullptr || signedDataTree == nullptr || certificatesTree == nullptr)
    {
        return makeError(P7bCertificateChainError::Pkcs7Structure, "PKCS7 结构不符合预期，未找到 SignedData 证书集合");
    }

    P7bCertificateChainResult result = {};
    result.parsedPkcs7Bytes = decoded->size();

    for (easy_asn1_tree_st* certificateNode = certificatesTree; certificateNode != nullptr; certificateNode = certificateNode->next_sibling)
    {
        std::vector<uint8_t> der = serializeNode(certificateNode);
        if (der.empty())
        {
            return makeError(P7bCertificateChainError::ExportFailed, "证书节点序列化失败");
        }

        CertificatePtr certificate(sm2_cert_parse(der.data(), der.size()), &sm2_cert_free);
        if (!certificate)
        {
            return makeError(P7bCertificateChainError::CertificateParse, "证书解析失败，当前实现仅支持 endecode 可识别的证书结构");
        }

        P7bCertificateSummary summary = {};
        summary.index = result.certificates.size();
        summary.subject = formatName(certificate->tbsCertificate.subject);
        summary.issuer = formatName(certificate->tbsCertificate.issuer);
        summary.serialNumber = bytesToHex(certificate->tbsCertificate.serialNumber.value, certificate->tbsCertificate.serialNumber.length);
        summary.notBefore = formatTime(certificate->tbsCertificate.validity.notBefore);
        summary.notAfter = formatTime(certificate->tbsCertificate.validity.notAfter);
        summary.derBytes = der;
        summary.pem = derToPem(summary.derBytes);

        if (summary.pem.empty())
        {
            return makeError(P7bCertificateChainError::ExportFailed, "证书已解析，但 PEM 编码失败");
        }

        result.fullChainPem += summary.pem;
        result.certificates.push_back(summary);
    }

    if (result.certificates.empty())
    {
        return makeError(P7bCertificateChainError::EmptyCertificates, "证书链中未包含任何可导出的证书");
    }

    result.success = true;
    result.error = P7bCertificateChainError::None;
    result.message = "证书链解析成功（endecode ASN.1）";
    result.parsedCertificateCount = result.certificates.size();
    return result;
}
}
