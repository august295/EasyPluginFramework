#ifndef __ASN1_TYPES_H__
#define __ASN1_TYPES_H__

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace core
{
/**
 * @brief ASN.1 节点展示信息。
 */
struct Asn1NodeInfo
{
    std::size_t               offset;
    std::size_t               level;
    std::uint8_t              tag;
    std::string               tagName;
    std::string               className;
    std::size_t               length;
    std::size_t               encodedLength;
    bool                      isConstructed;
    std::string               displayValue;
    std::string               rawHex;
    std::vector<Asn1NodeInfo> children;
};

/**
 * @brief ASN.1 文档解析结果。
 */
struct Asn1Document
{
    std::string               sourceName;
    std::size_t               byteCount;
    std::string               sourceBytes;
    std::vector<Asn1NodeInfo> rootNodes;
};
}

#endif
