#ifndef __CERTIFICATE_CHAIN_SERVICE_H__
#define __CERTIFICATE_CHAIN_SERVICE_H__

#include <string>
#include <vector>

#include "certificate_chain_types.h"

namespace core
{
class CertificateChainService
{
public:
    CertificateAnalysisResult load(const CertificateAnalysisRequest& request);
    const CertificateAnalysisResult& currentResult() const;
    CertificateValidationResult validateCertificate(const std::string& filePath) const;
    void clear();

private:
    CertificateAnalysisResult loadDirectory(const std::string& directoryPath);
    CertificateAnalysisResult appendFile(const std::string& filePath);
    CertificateAnalysisResult rebuildResult(const std::vector<std::string>& filePaths);

private:
    std::vector<std::string> m_loadedPaths;
    CertificateAnalysisResult m_currentResult;
};
}

#endif
