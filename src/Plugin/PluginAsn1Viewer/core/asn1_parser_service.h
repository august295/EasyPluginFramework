#ifndef __ASN1_PARSER_SERVICE_H__
#define __ASN1_PARSER_SERVICE_H__

#include <optional>
#include <string>

#include "asn1_types.h"

namespace core
{
/**
 * @brief ASN.1 解析服务。
 */
class Asn1ParserService
{
public:
    /**
     * @brief 从文件解析 ASN.1 文档。
     * @param filePath 文件路径。
     * @return std::optional<Asn1Document> 成功时返回文档。
     */
    std::optional<Asn1Document> parseFile(const std::string& filePath) const;

    /**
     * @brief 从文本内容解析 ASN.1 文档。
     * @param sourceName 数据来源名称。
     * @param textContent 输入文本。
     * @return std::optional<Asn1Document> 成功时返回文档。
     */
    std::optional<Asn1Document> parseText(const std::string& sourceName, const std::string& textContent) const;

    /**
     * @brief 获取最近一次错误信息。
     * @return const std::string& 错误信息。
     */
    const std::string& getLastError() const;

private:
    std::optional<Asn1Document> parseBinaryContent(const std::string& sourceName, const std::string& binaryContent) const;

private:
    mutable std::string m_lastError;
};
}

#endif
