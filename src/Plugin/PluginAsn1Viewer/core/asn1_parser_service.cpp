#include "asn1_parser_service.h"

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

#include <QtCore/QByteArray>

extern "C"
{
#include <endecode/asn1/asn1.h>
#include <endecode/asn1/asn1_helper.h>
}

namespace
{
using Asn1TreePtr = std::unique_ptr<easy_asn1_tree_st, decltype(&easy_asn1_free_tree)>;

std::string readFileContent(const std::string& filePath)
{
    std::ifstream input(filePath, std::ios::binary);
    if (!input.is_open())
    {
        return "";
    }
    return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

std::string trimText(const std::string& text)
{
    const std::size_t start = text.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        return "";
    }
    const std::size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1U);
}

bool isPemContent(const std::string& text)
{
    return text.find("-----BEGIN") != std::string::npos && text.find("-----END") != std::string::npos;
}

std::string decodePemContent(const std::string& text)
{
    std::stringstream stream(text);
    std::string line;
    std::string base64Text;
    while (std::getline(stream, line))
    {
        if (line.find("-----BEGIN") != std::string::npos || line.find("-----END") != std::string::npos)
        {
            continue;
        }
        base64Text += trimText(line);
    }
    const QByteArray decodedBytes = QByteArray::fromBase64(QByteArray::fromStdString(base64Text));
    return std::string(decodedBytes.constData(), static_cast<std::size_t>(decodedBytes.size()));
}

std::string stripPemMarkers(const std::string& text)
{
    std::string normalizedText = text;
    std::size_t markerStart    = normalizedText.find("-----BEGIN");
    while (markerStart != std::string::npos)
    {
        const std::size_t markerEnd = normalizedText.find("-----", markerStart + 10U);
        if (markerEnd == std::string::npos)
        {
            break;
        }

        normalizedText.erase(markerStart, markerEnd - markerStart + 5U);
        markerStart = normalizedText.find("-----BEGIN");
    }

    markerStart = normalizedText.find("-----END");
    while (markerStart != std::string::npos)
    {
        const std::size_t markerEnd = normalizedText.find("-----", markerStart + 8U);
        if (markerEnd == std::string::npos)
        {
            break;
        }

        normalizedText.erase(markerStart, markerEnd - markerStart + 5U);
        markerStart = normalizedText.find("-----END");
    }

    return normalizedText;
}

std::string sanitizeBase64Content(const std::string& text)
{
    const std::string normalizedText = stripPemMarkers(text);
    std::string       base64Text;
    base64Text.reserve(normalizedText.size());
    for (const unsigned char ch : normalizedText)
    {
        if (!std::isspace(ch))
        {
            base64Text.push_back(static_cast<char>(ch));
        }
    }
    return base64Text;
}

bool looksLikePemOrBase64(const std::string& text)
{
    if (isPemContent(text))
    {
        return true;
    }

    const std::string sanitizedText = sanitizeBase64Content(text);
    if (sanitizedText.empty() || sanitizedText.size() < 8U)
    {
        return false;
    }

    return sanitizedText.size() == trimText(text).size() || text.find('\n') != std::string::npos || text.find('\r') != std::string::npos;
}

bool isValidBase64Text(const std::string& text)
{
    if (text.empty() || (text.size() % 4U) != 0U)
    {
        return false;
    }

    bool        seenPadding  = false;
    std::size_t paddingCount = 0U;
    for (const char ch : text)
    {
        const unsigned char value = static_cast<unsigned char>(ch);
        if (std::isalnum(value) != 0 || ch == '+' || ch == '/')
        {
            if (seenPadding)
            {
                return false;
            }
            continue;
        }

        if (ch == '=')
        {
            seenPadding = true;
            ++paddingCount;
            if (paddingCount > 2U)
            {
                return false;
            }
            continue;
        }

        return false;
    }

    return true;
}

std::string decodeBase64Content(const std::string& text)
{
    const std::string sanitizedText = sanitizeBase64Content(text);
    if (!isValidBase64Text(sanitizedText))
    {
        return "";
    }

    const QByteArray decodedBytes = QByteArray::fromBase64(QByteArray::fromStdString(sanitizedText));
    return std::string(decodedBytes.constData(), static_cast<std::size_t>(decodedBytes.size()));
}

std::string normalizeParseContent(const std::string& text, bool* usedBase64)
{
    if (usedBase64 != nullptr)
    {
        *usedBase64 = false;
    }

    if (!looksLikePemOrBase64(text))
    {
        return text;
    }

    const std::string decodedText = decodeBase64Content(text);
    if (!decodedText.empty())
    {
        if (usedBase64 != nullptr)
        {
            *usedBase64 = true;
        }
        return decodedText;
    }

    return text;
}

std::string bytesToHex(const std::uint8_t* data, const std::size_t length)
{
    std::ostringstream stream;
    for (std::size_t index = 0; index < length; ++index)
    {
        if (index > 0U)
        {
            stream << ' ';
        }
        stream << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[index]);
    }
    return stream.str();
}

std::string buildTagName(const std::uint8_t tag)
{
    char* tagName = easy_asn1_tag_name(tag);
    if (tagName != nullptr)
    {
        return tagName;
    }
    std::ostringstream stream;
    stream << "TAG 0x" << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(tag);
    return stream.str();
}

std::string buildClassName(const std::uint8_t tag)
{
    const std::uint8_t tagClass = static_cast<std::uint8_t>(tag & 0xC0U);
    switch (tagClass)
    {
    case UNIVERSAL:
        return "UNIVERSAL";
    case APPLICATION:
        return "APPLICATION";
    case CONTEXT_SPECIFIC:
        return "CONTEXT_SPECIFIC";
    case PRIVATE:
        return "PRIVATE";
    default:
        return "UNKNOWN";
    }
}

std::string buildDisplayValue(const easy_asn1_string_st& value)
{
    if (value.length == 0U || value.value == nullptr)
    {
        return "";
    }
    switch (value.tag)
    {
    case EASY_ASN1_BOOLEAN:
        return value.value[0] == 0U ? "FALSE" : "TRUE";
    case EASY_ASN1_INTEGER: {
        std::unique_ptr<char, decltype(&std::free)> integerValue(easy_asn1_print_integer(value.value, value.length), &std::free);
        return integerValue ? integerValue.get() : "";
    }
    case EASY_ASN1_NULL:
        return "NULL";
    case EASY_ASN1_OBJECT: {
        char oidText[MAX_OID] = {0};
        oid_to_string(value.value, value.length, oidText);
        return oidText;
    }
    case EASY_ASN1_UTF8STRING:
    case EASY_ASN1_NUMERICSTRING:
    case EASY_ASN1_PRINTABLESTRING:
    case EASY_ASN1_T61STRING:
    case EASY_ASN1_VIDEOTEXSTRING:
    case EASY_ASN1_IA5STRING:
        return std::string(reinterpret_cast<const char*>(value.value), value.length);
    case EASY_ASN1_UTCTIME: {
        char timeText[21] = {0};
        convertUTCTimeToStandard(reinterpret_cast<const char*>(value.value), value.length, 8U, timeText);
        return timeText;
    }
    case EASY_ASN1_GENERALIZEDTIME: {
        char timeText[21] = {0};
        convertGeneralizedTimeToStandard(reinterpret_cast<const char*>(value.value), value.length, 8U, timeText);
        return timeText;
    }
    default:
        if (easy_asn1_print_string_try(value.value, value.length) == 1)
        {
            return std::string(reinterpret_cast<const char*>(value.value), value.length);
        }
        return bytesToHex(value.value, value.length);
    }
}

core::Asn1NodeInfo buildNodeInfo(const easy_asn1_tree_st* node)
{
    core::Asn1NodeInfo nodeInfo = {};
    nodeInfo.offset = node->offset;
    nodeInfo.level = node->level;
    nodeInfo.tag = node->value.tag;
    nodeInfo.tagName = buildTagName(node->value.tag);
    nodeInfo.className = buildClassName(node->value.tag);
    nodeInfo.length = node->value.length;
    nodeInfo.encodedLength = easy_asn1_serialize(const_cast<easy_asn1_tree_st*>(node), nullptr);
    nodeInfo.isConstructed = (node->value.tag & CONSTRUCTED) != 0U;
    nodeInfo.displayValue = buildDisplayValue(node->value);
    nodeInfo.rawHex = bytesToHex(node->value.value, node->value.length);
    for (easy_asn1_tree_st* child = node->first_child; child != nullptr; child = child->next_sibling)
    {
        nodeInfo.children.push_back(buildNodeInfo(child));
    }
    return nodeInfo;
}
}

namespace core
{
std::optional<Asn1Document> Asn1ParserService::parseFile(const std::string& filePath) const
{
    m_lastError.clear();
    const std::string fileContent = readFileContent(filePath);
    if (fileContent.empty())
    {
        m_lastError = "文件读取失败：无法打开文件或文件为空";
        return std::nullopt;
    }

    const std::string binaryContent = normalizeParseContent(fileContent, nullptr);
    return parseBinaryContent(filePath, binaryContent);
}

std::optional<Asn1Document> Asn1ParserService::parseText(const std::string& sourceName, const std::string& textContent) const
{
    m_lastError.clear();
    if (textContent.empty())
    {
        m_lastError = "输入失败：输入内容为空";
        return std::nullopt;
    }

    const std::string binaryContent = normalizeParseContent(textContent, nullptr);
    return parseBinaryContent(sourceName, binaryContent);
}

const std::string& Asn1ParserService::getLastError() const
{
    return m_lastError;
}

std::optional<Asn1Document> Asn1ParserService::parseBinaryContent(const std::string& sourceName, const std::string& binaryContent) const
{
    easy_asn1_tree_st* rawTree = static_cast<easy_asn1_tree_st*>(std::malloc(sizeof(easy_asn1_tree_st)));
    if (rawTree == nullptr)
    {
        m_lastError = "内存分配失败";
        return std::nullopt;
    }
    Asn1TreePtr tree(rawTree, &easy_asn1_free_tree);
    easy_asn1_parse(reinterpret_cast<const std::uint8_t*>(binaryContent.data()), binaryContent.size(), 0U, 0U, &rawTree);
    tree.release();
    tree.reset(rawTree);
    if (rawTree == nullptr)
    {
        m_lastError = "ASN.1 解析失败：输入数据不是有效的 ASN.1 结构";
        return std::nullopt;
    }
    Asn1Document document = {};
    document.sourceName = sourceName;
    document.byteCount = binaryContent.size();
    document.sourceBytes = binaryContent;
    for (easy_asn1_tree_st* node = rawTree; node != nullptr; node = node->next_sibling)
    {
        document.rootNodes.push_back(buildNodeInfo(node));
    }
    return document;
}
}
