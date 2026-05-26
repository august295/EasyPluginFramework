#include "certificate_chain_service.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <endecode/asn1/asn1.h>
#include <endecode/asn1/asn1_helper.h>
#include <endecode/asn1/cert.h>
#include <endecode/asn1/cert_sm2.h>
#include <endecode/base64/base64.h>
#include <endecode/ecc/ecc.h>
#include <endecode/sm2/sm2.h>

namespace
{
namespace fs = std::filesystem;

using CertificatePtr = std::unique_ptr<SM2Certificate, decltype(&sm2_cert_free)>;
using Asn1TreePtr = std::unique_ptr<easy_asn1_tree_st, decltype(&easy_asn1_free_tree)>;

constexpr std::size_t kInvalidNodeId = static_cast<std::size_t>(-1);

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

std::string trimAllWhitespace(const std::string& text)
{
    std::string trimmed;
    trimmed.reserve(text.size());
    for (const char ch : text)
    {
        if (std::isspace(static_cast<unsigned char>(ch)) == 0)
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
    for (std::size_t index = 0U; index < length; ++index)
    {
        stream << std::setw(2) << static_cast<int>(data[index]);
    }
    return stream.str();
}

std::string toLowerAscii(const std::string& text)
{
    std::string lowered = text;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](const unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return lowered;
}

bool hasSupportedExtension(const fs::path& path)
{
    const std::string extension = toLowerAscii(path.extension().string());
    return extension == ".pem" || extension == ".crt" || extension == ".cer" || extension == ".der";
}

std::string readFileContent(const std::string& filePath)
{
    std::ifstream input(filePath, std::ios::binary);
    if (!input.is_open())
    {
        return "";
    }

    return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

bool looksLikePem(const std::string& text)
{
    return text.find("-----BEGIN CERTIFICATE-----") != std::string::npos;
}

bool looksLikeBase64Certificate(const std::string& text)
{
    const std::string compact = trimAllWhitespace(text);
    if (compact.size() < 64U || (compact.size() % 4U) != 0U)
    {
        return false;
    }

    for (const unsigned char ch : compact)
    {
        if (std::isalnum(ch) != 0 || ch == '+' || ch == '/' || ch == '=')
        {
            continue;
        }
        return false;
    }

    return true;
}

std::string extractPemPayload(const std::string& text)
{
    const std::string beginMarker = "-----BEGIN CERTIFICATE-----";
    const std::string endMarker = "-----END CERTIFICATE-----";

    const std::size_t beginPos = text.find(beginMarker);
    const std::size_t endPos = text.find(endMarker);
    if (beginPos == std::string::npos || endPos == std::string::npos || endPos <= beginPos + beginMarker.size())
    {
        return "";
    }

    const std::size_t payloadStart = beginPos + beginMarker.size();
    return trimAllWhitespace(text.substr(payloadStart, endPos - payloadStart));
}

enum class DecodedContentKind
{
    Certificate,
    Pkcs7,
    Unknown,
};

std::string oidToString(const easy_asn1_string_st& oid)
{
    if (oid.value == nullptr || oid.length == 0U)
    {
        return "";
    }

    char oidText[MAX_OID] = {0};
    oid_to_string(oid.value, oid.length, oidText);
    return oidText;
}

DecodedContentKind detectDecodedContentKind(const std::vector<uint8_t>& decodedBytes)
{
    if (decodedBytes.empty())
    {
        return DecodedContentKind::Unknown;
    }

    easy_asn1_tree_st* rawTree = static_cast<easy_asn1_tree_st*>(std::malloc(sizeof(easy_asn1_tree_st)));
    if (rawTree == nullptr)
    {
        return DecodedContentKind::Unknown;
    }

    easy_asn1_parse(decodedBytes.data(), decodedBytes.size(), 0U, 0U, &rawTree);
    Asn1TreePtr tree(rawTree, &easy_asn1_free_tree);
    if (rawTree == nullptr)
    {
        return DecodedContentKind::Unknown;
    }

    easy_asn1_tree_st* firstNode = easy_asn1_get_tree_item(rawTree, 0);
    if (firstNode != nullptr && firstNode->value.tag == EASY_ASN1_OBJECT)
    {
        const std::string oid = oidToString(firstNode->value);
        if (oid == "1.2.840.113549.1.7.2")
        {
            return DecodedContentKind::Pkcs7;
        }
    }

    easy_asn1_tree_st* tbsTree = easy_asn1_get_tree_item(rawTree, 0);
    easy_asn1_tree_st* signatureAlgorithmTree = easy_asn1_get_tree_item(rawTree, 1);
    easy_asn1_tree_st* signatureValueTree = easy_asn1_get_tree_item(rawTree, 2);
    if (tbsTree != nullptr && signatureAlgorithmTree != nullptr && signatureValueTree != nullptr)
    {
        return DecodedContentKind::Certificate;
    }

    return DecodedContentKind::Unknown;
}

std::optional<std::vector<uint8_t>> decodePemOrDer(const std::string& fileContent)
{
    if (fileContent.empty())
    {
        return std::nullopt;
    }

    std::string base64Text;
    if (looksLikePem(fileContent))
    {
        base64Text = extractPemPayload(fileContent);
    }
    else if (looksLikeBase64Certificate(fileContent))
    {
        base64Text = trimAllWhitespace(fileContent);
    }
    else
    {
        return std::vector<uint8_t>(fileContent.begin(), fileContent.end());
    }
    if (base64Text.empty())
    {
        return std::nullopt;
    }

    std::vector<uint8_t> decoded(BASE64_DECODE_OUT_SIZE(base64Text.size()));
    const std::size_t decodedLength = base64_decode(base64Text.c_str(), base64Text.size(), decoded.data());
    if (decodedLength == 0U)
    {
        return std::nullopt;
    }

    decoded.resize(decodedLength);
    return decoded;
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
    for (std::size_t offset = 0U; offset < encodedLength; offset += 64U)
    {
        pem.append(base64.data() + offset, std::min<std::size_t>(64U, encodedLength - offset));
        pem.push_back('\n');
    }
    pem += std::string(PEM_CERT_END) + "\n";
    return pem;
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

std::string oidToLabel(const std::string& oid)
{
    static const std::map<std::string, std::string> kOidLabels = {
        {"2.5.4.3", "CN"},
        {"2.5.4.6", "C"},
        {"2.5.4.7", "L"},
        {"2.5.4.8", "ST"},
        {"2.5.4.10", "O"},
        {"2.5.4.11", "OU"},
        {"1.2.840.113549.1.9.1", "EMAIL"},
    };

    const auto iter = kOidLabels.find(oid);
    return iter == kOidLabels.end() ? oid : iter->second;
}

std::string formatName(const Name& name)
{
    std::ostringstream stream;
    bool hasValue = false;

    for (uint32_t nameIndex = 0U; nameIndex < name.count; ++nameIndex)
    {
        const RelativeDistinguishedName& rdn = name.names[nameIndex];
        for (uint32_t rdnIndex = 0U; rdnIndex < rdn.count; ++rdnIndex)
        {
            const AttributeTypeAndValue& item = rdn.rdn[rdnIndex];
            const std::string valueText = asn1StringToText(item.value);
            if (valueText.empty())
            {
                continue;
            }

            if (hasValue)
            {
                stream << ", ";
            }
            stream << oidToLabel(oidToString(item.type)) << "=" << valueText;
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

bool parseCompactTime(const std::string& text, std::tm* output)
{
    if (output == nullptr)
    {
        return false;
    }

    std::tm parsed = {};
    std::istringstream stream(text);
    stream >> std::get_time(&parsed, "%Y-%m-%d %H:%M:%S");
    if (stream.fail())
    {
        return false;
    }

    *output = parsed;
    return true;
}

bool isCurrentlyValid(const std::string& notBefore, const std::string& notAfter, core::CertificateValidityCode* code)
{
    if (code == nullptr)
    {
        return false;
    }

    std::tm beforeTm = {};
    std::tm afterTm = {};
    if (!parseCompactTime(notBefore, &beforeTm) || !parseCompactTime(notAfter, &afterTm))
    {
        *code = core::CertificateValidityCode::ParseFailed;
        return false;
    }

    const std::time_t now = std::time(nullptr);
    const std::time_t beforeTime = std::mktime(&beforeTm);
    const std::time_t afterTime = std::mktime(&afterTm);
    if (now < beforeTime)
    {
        *code = core::CertificateValidityCode::NotYetValid;
        return false;
    }
    if (now > afterTime)
    {
        *code = core::CertificateValidityCode::Expired;
        return false;
    }

    *code = core::CertificateValidityCode::Valid;
    return true;
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

easy_asn1_tree_st* childAt(easy_asn1_tree_st* node, int index)
{
    return node == nullptr ? nullptr : easy_asn1_get_tree_item(node, index);
}

std::string parseOctetStringPayloadText(easy_asn1_tree_st* octetStringNode)
{
    if (octetStringNode == nullptr || octetStringNode->value.value == nullptr || octetStringNode->value.length == 0U)
    {
        return "";
    }

    easy_asn1_tree_st* innerRawTree = static_cast<easy_asn1_tree_st*>(std::malloc(sizeof(easy_asn1_tree_st)));
    if (innerRawTree == nullptr)
    {
        return "";
    }

    easy_asn1_parse(octetStringNode->value.value, octetStringNode->value.length, 0U, 0U, &innerRawTree);
    Asn1TreePtr innerTree(innerRawTree, &easy_asn1_free_tree);
    if (innerRawTree == nullptr)
    {
        return "";
    }

    easy_asn1_tree_st* valueNode = innerRawTree;
    if (valueNode->value.tag == EASY_ASN1_OCTET_STRING)
    {
        return bytesToHex(valueNode->value.value, valueNode->value.length);
    }
    if (valueNode->value.tag == EASY_ASN1_OBJECT)
    {
        return oidToString(valueNode->value);
    }

    if (valueNode->value.tag == EASY_ASN1_SEQUENCE)
    {
        easy_asn1_tree_st* firstChild = valueNode->first_child;
        if (firstChild != nullptr && firstChild->value.tag == EASY_ASN1_OCTET_STRING)
        {
            return bytesToHex(firstChild->value.value, firstChild->value.length);
        }
    }

    if (easy_asn1_print_string_try(valueNode->value.value, valueNode->value.length) == 1)
    {
        return std::string(reinterpret_cast<const char*>(valueNode->value.value), valueNode->value.length);
    }

    return bytesToHex(valueNode->value.value, valueNode->value.length);
}

void parseCertificateExtensions(
    const easy_asn1_string_st& extensions,
    bool* isCa,
    std::string* subjectKeyIdentifier,
    std::string* authorityKeyIdentifier,
    std::string* summary)
{
    if (isCa != nullptr)
    {
        *isCa = false;
    }
    if (subjectKeyIdentifier != nullptr)
    {
        subjectKeyIdentifier->clear();
    }
    if (authorityKeyIdentifier != nullptr)
    {
        authorityKeyIdentifier->clear();
    }
    if (summary != nullptr)
    {
        summary->clear();
    }

    if (extensions.value == nullptr || extensions.length == 0U)
    {
        return;
    }

    easy_asn1_tree_st* rawTree = static_cast<easy_asn1_tree_st*>(std::malloc(sizeof(easy_asn1_tree_st)));
    if (rawTree == nullptr)
    {
        return;
    }

    easy_asn1_parse(extensions.value, extensions.length, 0U, 0U, &rawTree);
    Asn1TreePtr tree(rawTree, &easy_asn1_free_tree);
    if (rawTree == nullptr)
    {
        return;
    }

    std::vector<std::string> parts;
    easy_asn1_tree_st* sequenceNode = rawTree;
    if ((sequenceNode->value.tag & EASY_ASN1_SEQUENCE) == 0U && sequenceNode->first_child != nullptr)
    {
        sequenceNode = sequenceNode->first_child;
    }

    for (easy_asn1_tree_st* extensionNode = sequenceNode->first_child; extensionNode != nullptr; extensionNode = extensionNode->next_sibling)
    {
        easy_asn1_tree_st* oidNode = childAt(extensionNode, 0);
        easy_asn1_tree_st* valueNode = childAt(extensionNode, 1);
        if (valueNode != nullptr && valueNode->value.tag == EASY_ASN1_BOOLEAN)
        {
            valueNode = childAt(extensionNode, 2);
        }

        if (oidNode == nullptr || valueNode == nullptr)
        {
            continue;
        }

        const std::string oid = oidToString(oidNode->value);
        if (oid.empty())
        {
            continue;
        }

        if (oid == "2.5.29.19")
        {
            const std::string text = parseOctetStringPayloadText(valueNode);
            if (text.find("FF") != std::string::npos || text.find("01") != std::string::npos)
            {
                if (isCa != nullptr)
                {
                    *isCa = true;
                }
            }
            parts.push_back("BasicConstraints");
        }
        else if (oid == "2.5.29.14")
        {
            const std::string text = parseOctetStringPayloadText(valueNode);
            if (subjectKeyIdentifier != nullptr)
            {
                *subjectKeyIdentifier = text;
            }
            parts.push_back("SKI=" + text);
        }
        else if (oid == "2.5.29.35")
        {
            const std::string text = parseOctetStringPayloadText(valueNode);
            if (authorityKeyIdentifier != nullptr)
            {
                *authorityKeyIdentifier = text;
            }
            parts.push_back("AKI=" + text);
        }
    }

    if (summary != nullptr)
    {
        std::ostringstream stream;
        for (std::size_t index = 0U; index < parts.size(); ++index)
        {
            if (index > 0U)
            {
                stream << "; ";
            }
            stream << parts[index];
        }
        *summary = stream.str();
    }
}

core::CertificateFileIssue makeIssue(const std::string& path, const core::CertificateParseError error, const std::string& message)
{
    core::CertificateFileIssue issue = {};
    issue.path = path;
    issue.error = error;
    issue.message = message;
    return issue;
}

bool verifyCertificateTbsSignature(
    const core::CertificateNode& certificate,
    const core::CertificateNode& issuer,
    std::string* message)
{
    if (certificate.derBytes.empty() || issuer.derBytes.empty())
    {
        if (message != nullptr)
        {
            *message = "证书 DER 数据为空，无法执行 TBS 签名验证";
        }
        return false;
    }

    easy_asn1_tree_st* rawTree = static_cast<easy_asn1_tree_st*>(std::malloc(sizeof(easy_asn1_tree_st)));
    if (rawTree == nullptr)
    {
        if (message != nullptr)
        {
            *message = "ASN.1 树分配失败，无法执行 TBS 签名验证";
        }
        return false;
    }

    easy_asn1_parse(certificate.derBytes.data(), certificate.derBytes.size(), 0U, 0U, &rawTree);
    Asn1TreePtr tree(rawTree, &easy_asn1_free_tree);
    if (rawTree == nullptr)
    {
        if (message != nullptr)
        {
            *message = "待验证证书 ASN.1 解析失败";
        }
        return false;
    }

    easy_asn1_tree_st* tbsTree = easy_asn1_get_tree_item(rawTree, 0);
    easy_asn1_tree_st* signatureTree = easy_asn1_get_tree_item(rawTree, 2);
    easy_asn1_tree_st* signatureSequenceTree = signatureTree == nullptr ? nullptr : easy_asn1_get_tree_item(signatureTree, 0);
    easy_asn1_tree_st* rTree = signatureSequenceTree == nullptr ? nullptr : easy_asn1_get_tree_item(signatureSequenceTree, 0);
    easy_asn1_tree_st* sTree = signatureSequenceTree == nullptr ? nullptr : easy_asn1_get_tree_item(signatureSequenceTree, 1);

    if (tbsTree == nullptr || signatureTree == nullptr || signatureSequenceTree == nullptr || rTree == nullptr || sTree == nullptr)
    {
        if (message != nullptr)
        {
            *message = "证书结构不符合预期，无法定位 TBS 或签名值";
        }
        return false;
    }

    const std::size_t serializedLength = easy_asn1_serialize(tbsTree, nullptr);
    if (serializedLength == 0U)
    {
        if (message != nullptr)
        {
            *message = "TBSCertificate 序列化失败";
        }
        return false;
    }

    std::vector<std::uint8_t> serializedTbs(serializedLength);
    const std::size_t writtenLength = easy_asn1_serialize(tbsTree, serializedTbs.data());
    if (writtenLength == 0U)
    {
        if (message != nullptr)
        {
            *message = "TBSCertificate 序列化失败";
        }
        return false;
    }
    serializedTbs.resize(writtenLength);

    CertificatePtr issuerCertificate(sm2_cert_parse(issuer.derBytes.data(), issuer.derBytes.size()), &sm2_cert_free);
    if (!issuerCertificate)
    {
        if (message != nullptr)
        {
            *message = "树中签发者证书解析失败";
        }
        return false;
    }

    const easy_asn1_string_st publicKey = issuerCertificate->tbsCertificate.subjectPublicKeyInfo.subjectPublicKey;
    const easy_asn1_string_st rValue = rTree->value;
    const easy_asn1_string_st sValue = sTree->value;
    if (publicKey.value == nullptr || publicKey.length < 66U || rValue.value == nullptr || sValue.value == nullptr)
    {
        if (message != nullptr)
        {
            *message = "签发者公钥或证书签名值无效";
        }
        return false;
    }

    EccKeyHolder keyHolder;
    mpz_import(keyHolder.key.public_key.x, 32, 1, 1, 1, 0, publicKey.value + 2);
    mpz_import(keyHolder.key.public_key.y, 32, 1, 1, 1, 0, publicKey.value + 34);

    MpzHolder r;
    MpzHolder s;
    mpz_import(r.value, rValue.length, 1, 1, 1, 0, rValue.value);
    mpz_import(s.value, sValue.length, 1, 1, 1, 0, sValue.value);

    const char* defaultUserId = "1234567812345678";
    const int verifyRet = sm2_verify(
        serializedTbs.data(),
        serializedTbs.size(),
        reinterpret_cast<const uint8_t*>(defaultUserId),
        std::strlen(defaultUserId),
        &keyHolder.key,
        r.value,
        s.value);

    if (verifyRet == 1)
    {
        if (message != nullptr)
        {
            *message = "使用 endecode 完成 TBS 签名验证并通过";
        }
        return true;
    }

    if (message != nullptr)
    {
        *message = "使用 endecode 执行 TBS 签名验证失败";
    }
    return false;
}

bool isBetterIssuerCandidate(const core::CertificateNode& candidate, const core::CertificateNode* currentBest, const core::CertificateNode& child)
{
    if (currentBest == nullptr)
    {
        return true;
    }

    const bool candidateAkiMatch =
        !child.authorityKeyIdentifier.empty() && !candidate.subjectKeyIdentifier.empty() &&
        toLowerAscii(child.authorityKeyIdentifier) == toLowerAscii(candidate.subjectKeyIdentifier);
    const bool bestAkiMatch =
        !child.authorityKeyIdentifier.empty() && !currentBest->subjectKeyIdentifier.empty() &&
        toLowerAscii(child.authorityKeyIdentifier) == toLowerAscii(currentBest->subjectKeyIdentifier);
    if (candidateAkiMatch != bestAkiMatch)
    {
        return candidateAkiMatch;
    }

    if (candidate.isCa != currentBest->isCa)
    {
        return candidate.isCa;
    }

    const bool candidateSelfSigned = candidate.subjectKey == candidate.issuerKey;
    const bool bestSelfSigned = currentBest->subjectKey == currentBest->issuerKey;
    if (candidateSelfSigned != bestSelfSigned)
    {
        return !candidateSelfSigned;
    }

    return candidate.subject.size() > currentBest->subject.size();
}

core::CertificateAnalysisStatus evaluateNodeAgainstDataset(
    const core::CertificateNode& node,
    const std::vector<core::CertificateNode>& dataset,
    std::size_t* issuerNodeId)
{
    if (issuerNodeId != nullptr)
    {
        *issuerNodeId = kInvalidNodeId;
    }

    core::CertificateAnalysisStatus status = {};
    status.selfSigned = node.subjectKey == node.issuerKey;
    status.timeValid = isCurrentlyValid(node.notBefore, node.notAfter, &status.code);

    if (!status.timeValid && status.code == core::CertificateValidityCode::Expired)
    {
        status.summary = "证书已过期";
    }
    else if (!status.timeValid && status.code == core::CertificateValidityCode::NotYetValid)
    {
        status.summary = "证书尚未生效";
    }
    else if (!status.timeValid)
    {
        status.summary = "证书有效期无法识别";
    }

    const core::CertificateNode* bestCandidate = nullptr;
    std::size_t bestIndex = kInvalidNodeId;
    for (std::size_t index = 0U; index < dataset.size(); ++index)
    {
        const core::CertificateNode& candidate = dataset[index];
        if (!candidate.parseSuccess)
        {
            continue;
        }
        if (candidate.subjectKey != node.issuerKey)
        {
            continue;
        }
        if (candidate.subjectKey == node.subjectKey && candidate.serialNumber == node.serialNumber)
        {
            continue;
        }

        if (isBetterIssuerCandidate(candidate, bestCandidate, node))
        {
            bestCandidate = &candidate;
            bestIndex = index;
        }
    }

    if (bestCandidate == nullptr)
    {
        if (status.timeValid)
        {
            if (status.selfSigned)
            {
                if (node.isCa)
                {
                    status.code = core::CertificateValidityCode::Valid;
                    status.summary = "验证证书是自签根证书";
                }
                else
                {
                    status.code = core::CertificateValidityCode::SelfSignedLeaf;
                    status.summary = "验证证书自签但未标记 CA";
                }
            }
            else
            {
                status.code = core::CertificateValidityCode::MissingIssuer;
                status.summary = "证书链树中未找到匹配的签发者";
            }
        }
        return status;
    }

    if (issuerNodeId != nullptr)
    {
        *issuerNodeId = bestIndex;
    }
    status.issuerFound = true;
    status.issuerIsCa = bestCandidate->isCa;

    if (!status.timeValid)
    {
        return status;
    }

    if (!bestCandidate->isCa)
    {
        status.code = core::CertificateValidityCode::IssuerNotCa;
        status.summary = "匹配到树中证书，但该证书不是 CA";
    }
    else
    {
        status.code = core::CertificateValidityCode::Valid;
        status.summary = "验证通过：该证书由当前树中的证书签发";
    }

    return status;
}

std::optional<core::CertificateNode> parseCertificateFile(const std::string& filePath, core::CertificateFileIssue* issue)
{
    const std::string content = readFileContent(filePath);
    if (content.empty())
    {
        if (issue != nullptr)
        {
            *issue = makeIssue(filePath, core::CertificateParseError::FileReadFailed, "读取证书文件失败或文件为空");
        }
        return std::nullopt;
    }

    const std::optional<std::vector<uint8_t>> der = decodePemOrDer(content);
    if (!der.has_value() || der->empty())
    {
        if (issue != nullptr)
        {
            *issue = makeIssue(filePath, core::CertificateParseError::UnsupportedFormat, "文件不是可识别的 PEM/DER 证书");
        }
        return std::nullopt;
    }

    const DecodedContentKind contentKind = detectDecodedContentKind(*der);
    if (contentKind == DecodedContentKind::Pkcs7)
    {
        if (issue != nullptr)
        {
            *issue = makeIssue(filePath, core::CertificateParseError::UnsupportedFormat, "输入内容是 PKCS7/CMS 容器，不是单张证书，请使用证书链解析/P7B 插件");
        }
        return std::nullopt;
    }

    CertificatePtr certificate(sm2_cert_parse(der->data(), der->size()), &sm2_cert_free);
    if (!certificate)
    {
        if (issue != nullptr)
        {
            *issue = makeIssue(filePath, core::CertificateParseError::ParseFailed, "endecode 无法解析当前证书结构");
        }
        return std::nullopt;
    }

    core::CertificateNode node = {};
    node.sourcePath = filePath;
    node.sourceName = fs::path(filePath).filename().string();
    node.subject = formatName(certificate->tbsCertificate.subject);
    node.issuer = formatName(certificate->tbsCertificate.issuer);
    node.subjectKey = toLowerAscii(node.subject);
    node.issuerKey = toLowerAscii(node.issuer);
    node.serialNumber = bytesToHex(certificate->tbsCertificate.serialNumber.value, certificate->tbsCertificate.serialNumber.length);
    node.notBefore = formatTime(certificate->tbsCertificate.validity.notBefore);
    node.notAfter = formatTime(certificate->tbsCertificate.validity.notAfter);
    node.signatureAlgorithm = oidToString(certificate->signatureAlgorithm.algorithm);
    node.derBytes = *der;
    node.pem = derToPem(*der);
    node.parseSuccess = true;

    parseCertificateExtensions(
        certificate->tbsCertificate.extensions,
        &node.isCa,
        &node.subjectKeyIdentifier,
        &node.authorityKeyIdentifier,
        &node.extensionSummary);

    if (node.subject.empty())
    {
        node.subject = node.sourceName;
        node.subjectKey = toLowerAscii(node.subject);
    }
    if (node.issuer.empty())
    {
        node.issuer = node.subject;
        node.issuerKey = node.subjectKey;
    }

    return node;
}

void rebuildRelationships(core::CertificateAnalysisResult* result)
{
    if (result == nullptr)
    {
        return;
    }

    for (core::CertificateNode& node : result->nodes)
    {
        node.parentId = kInvalidNodeId;
        node.children.clear();
    }

    std::unordered_map<std::string, std::vector<std::size_t>> bySubject;
    for (std::size_t index = 0U; index < result->nodes.size(); ++index)
    {
        const core::CertificateNode& node = result->nodes[index];
        bySubject[node.subjectKey].push_back(index);
    }

    for (std::size_t index = 0U; index < result->nodes.size(); ++index)
    {
        core::CertificateNode& child = result->nodes[index];
        const auto candidatesIter = bySubject.find(child.issuerKey);
        if (candidatesIter == bySubject.end())
        {
            continue;
        }

        const core::CertificateNode* bestCandidate = nullptr;
        std::size_t bestIndex = kInvalidNodeId;
        for (const std::size_t candidateIndex : candidatesIter->second)
        {
            if (candidateIndex == index)
            {
                continue;
            }

            const core::CertificateNode& candidate = result->nodes[candidateIndex];
            if (!candidate.parseSuccess)
            {
                continue;
            }

            if (isBetterIssuerCandidate(candidate, bestCandidate, child))
            {
                bestCandidate = &candidate;
                bestIndex = candidateIndex;
            }
        }

        if (bestIndex != kInvalidNodeId)
        {
            child.parentId = bestIndex;
        }
    }

    for (std::size_t index = 0U; index < result->nodes.size(); ++index)
    {
        const std::size_t parentId = result->nodes[index].parentId;
        if (parentId != kInvalidNodeId && parentId < result->nodes.size())
        {
            result->nodes[parentId].children.push_back(index);
        }
    }

    result->rootCount = 0U;
    result->orphanCount = 0U;
    for (core::CertificateNode& node : result->nodes)
    {
        core::CertificateAnalysisStatus status = {};
        status.selfSigned = node.subjectKey == node.issuerKey;
        status.timeValid = isCurrentlyValid(node.notBefore, node.notAfter, &status.code);

        if (!status.timeValid && status.code != core::CertificateValidityCode::Valid)
        {
            if (status.code == core::CertificateValidityCode::Expired)
            {
                status.summary = "证书已过期";
            }
            else if (status.code == core::CertificateValidityCode::NotYetValid)
            {
                status.summary = "证书尚未生效";
            }
            else
            {
                status.summary = "证书有效期无法识别";
            }
        }

        if (node.parentId == kInvalidNodeId)
        {
            if (status.selfSigned)
            {
                node.kind = node.children.empty() ? core::CertificateNodeKind::Root : core::CertificateNodeKind::Root;
                ++result->rootCount;
                if (status.timeValid)
                {
                    if (node.isCa)
                    {
                        status.code = core::CertificateValidityCode::Valid;
                        status.summary = "根证书有效";
                    }
                    else
                    {
                        status.code = core::CertificateValidityCode::SelfSignedLeaf;
                        status.summary = "自签但未标记 CA";
                    }
                }
            }
            else
            {
                node.kind = core::CertificateNodeKind::Orphan;
                ++result->orphanCount;
                if (status.timeValid)
                {
                    status.code = core::CertificateValidityCode::MissingIssuer;
                    status.summary = "未在当前树中找到签发者";
                }
            }
        }
        else
        {
            const core::CertificateNode& parent = result->nodes[node.parentId];
            status.issuerFound = true;
            status.issuerIsCa = parent.isCa;

            if (!status.timeValid)
            {
                // keep time failure
            }
            else if (!parent.isCa)
            {
                status.code = core::CertificateValidityCode::IssuerNotCa;
                status.summary = "匹配到的签发者不是 CA 证书";
            }
            else
            {
                status.code = core::CertificateValidityCode::Valid;
                status.summary = "由当前树中的上级证书签发";
            }

            node.kind = node.children.empty() ? core::CertificateNodeKind::Leaf : core::CertificateNodeKind::Intermediate;
        }

        node.status = status;
    }
}

core::CertificateAnalysisResult buildResultFromPaths(const std::vector<std::string>& filePaths)
{
    core::CertificateAnalysisResult result = {};
    std::set<std::string> seenKeys;

    for (const std::string& filePath : filePaths)
    {
        core::CertificateFileIssue issue = {};
        const std::optional<core::CertificateNode> node = parseCertificateFile(filePath, &issue);
        if (!node.has_value())
        {
            result.issues.push_back(issue);
            continue;
        }

        const std::string dedupeKey = node->subjectKey + "|" + node->serialNumber;
        if (!seenKeys.insert(dedupeKey).second)
        {
            result.issues.push_back(makeIssue(filePath, core::CertificateParseError::None, "检测到重复证书，已忽略"));
            continue;
        }

        core::CertificateNode storedNode = *node;
        storedNode.id = result.nodes.size();
        result.nodes.push_back(storedNode);
    }

    rebuildRelationships(&result);

    result.loadedCertificateCount = result.nodes.size();
    result.success = !result.nodes.empty();
    if (result.success)
    {
        std::ostringstream stream;
        stream << "已构建证书链树：证书 " << result.loadedCertificateCount << " 张，根 " << result.rootCount << " 个，孤儿 " << result.orphanCount
               << " 个";
        if (!result.issues.empty())
        {
            stream << "，存在 " << result.issues.size() << " 条导入提示";
        }
        result.message = stream.str();
    }
    else if (!result.issues.empty())
    {
        result.message = "未能导入任何有效证书";
    }
    else
    {
        result.message = "目录中未找到可用证书";
    }

    return result;
}
}

namespace core
{
CertificateAnalysisResult CertificateChainService::load(const CertificateAnalysisRequest& request)
{
    if (request.path.empty())
    {
        m_currentResult = {};
        m_currentResult.message = "路径不能为空";
        return m_currentResult;
    }

    if (request.mode == CertificateLoadMode::ReplaceFromDirectory)
    {
        m_currentResult = loadDirectory(request.path);
    }
    else
    {
        m_currentResult = appendFile(request.path);
    }
    return m_currentResult;
}

const CertificateAnalysisResult& CertificateChainService::currentResult() const
{
    return m_currentResult;
}

CertificateValidationResult CertificateChainService::validateCertificate(const std::string& filePath) const
{
    CertificateValidationResult result = {};
    if (filePath.empty())
    {
        result.message = "待验证证书路径不能为空";
        return result;
    }

    core::CertificateFileIssue issue = {};
    const std::optional<core::CertificateNode> parsedNode = parseCertificateFile(filePath, &issue);
    if (!parsedNode.has_value())
    {
        result.message = issue.message.empty() ? "待验证证书解析失败" : issue.message;
        return result;
    }

    result.certificateParsed = true;
    result.certificate = *parsedNode;
    result.certificate.status = evaluateNodeAgainstDataset(result.certificate, m_currentResult.nodes, &result.issuerNodeId);
    result.message = result.certificate.status.summary;
    result.issuerMatched = result.certificate.status.issuerFound;
    result.issuerIsCa = result.certificate.status.issuerIsCa;

    if (result.issuerNodeId != static_cast<std::size_t>(-1) && result.issuerNodeId < m_currentResult.nodes.size())
    {
        const core::CertificateNode& issuerNode = m_currentResult.nodes[result.issuerNodeId];
        result.issuerSubject = issuerNode.subject;
        result.tbsSignatureCheckAttempted = true;
        result.tbsSignatureVerified = verifyCertificateTbsSignature(result.certificate, issuerNode, &result.tbsSignatureMessage);

        if (!result.tbsSignatureVerified)
        {
            result.message = "找到树中签发者，但证书 TBS 签名验证失败";
        }
        else if (result.certificate.status.code == core::CertificateValidityCode::Valid)
        {
            result.message = "验证通过：树中找到签发者，且证书 TBS 签名验证通过";
        }
    }
    else
    {
        result.tbsSignatureMessage = "未执行 TBS 签名验证，因为没有匹配到树中签发者";
    }

    result.success =
        result.certificate.status.code == core::CertificateValidityCode::Valid && result.issuerMatched && result.issuerIsCa &&
        result.tbsSignatureVerified;
    return result;
}

void CertificateChainService::clear()
{
    m_loadedPaths.clear();
    m_currentResult = {};
}

CertificateAnalysisResult CertificateChainService::loadDirectory(const std::string& directoryPath)
{
    std::vector<std::string> paths;
    std::error_code errorCode;
    if (!fs::exists(directoryPath, errorCode) || !fs::is_directory(directoryPath, errorCode))
    {
        CertificateAnalysisResult result = {};
        result.message = "目录不存在或不可访问";
        return result;
    }

    for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath, errorCode))
    {
        if (errorCode)
        {
            break;
        }
        if (!entry.is_regular_file())
        {
            continue;
        }
        if (!hasSupportedExtension(entry.path()))
        {
            continue;
        }
        paths.push_back(entry.path().string());
    }

    std::sort(paths.begin(), paths.end());
    m_loadedPaths = paths;
    return rebuildResult(m_loadedPaths);
}

CertificateAnalysisResult CertificateChainService::appendFile(const std::string& filePath)
{
    if (!fs::exists(filePath))
    {
        CertificateAnalysisResult result = m_currentResult;
        result.message = "待添加证书文件不存在";
        return result;
    }

    if (std::find(m_loadedPaths.begin(), m_loadedPaths.end(), filePath) == m_loadedPaths.end())
    {
        m_loadedPaths.push_back(filePath);
    }

    std::sort(m_loadedPaths.begin(), m_loadedPaths.end());
    return rebuildResult(m_loadedPaths);
}

CertificateAnalysisResult CertificateChainService::rebuildResult(const std::vector<std::string>& filePaths)
{
    m_currentResult = buildResultFromPaths(filePaths);
    return m_currentResult;
}
}
