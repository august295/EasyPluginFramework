#ifndef __CERTIFICATE_CHAIN_TYPES_H__
#define __CERTIFICATE_CHAIN_TYPES_H__

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace core
{
enum class CertificateLoadMode
{
    ReplaceFromDirectory,
    AppendSingleFile,
};

enum class CertificateParseError
{
    None,
    EmptyInput,
    FileReadFailed,
    UnsupportedFormat,
    ParseFailed,
};

enum class CertificateNodeKind
{
    Root,
    Intermediate,
    Leaf,
    Orphan,
};

enum class CertificateValidityCode
{
    Valid,
    ParseFailed,
    Expired,
    NotYetValid,
    MissingIssuer,
    IssuerNotCa,
    SelfSignedLeaf,
};

struct CertificateAnalysisRequest
{
    CertificateLoadMode mode = CertificateLoadMode::ReplaceFromDirectory;
    std::string path;
};

struct CertificateFileIssue
{
    std::string path;
    CertificateParseError error = CertificateParseError::None;
    std::string message;
};

struct CertificateAnalysisStatus
{
    CertificateValidityCode code = CertificateValidityCode::ParseFailed;
    bool timeValid = false;
    bool issuerFound = false;
    bool issuerIsCa = false;
    bool selfSigned = false;
    std::string summary;
};

struct CertificateNode
{
    std::size_t id = 0U;
    std::size_t parentId = static_cast<std::size_t>(-1);
    std::string sourcePath;
    std::string sourceName;
    std::string subject;
    std::string issuer;
    std::string subjectKey;
    std::string issuerKey;
    std::string serialNumber;
    std::string notBefore;
    std::string notAfter;
    std::string subjectKeyIdentifier;
    std::string authorityKeyIdentifier;
    std::string signatureAlgorithm;
    std::string extensionSummary;
    std::vector<std::uint8_t> derBytes;
    std::string pem;
    bool isCa = false;
    bool parseSuccess = false;
    CertificateNodeKind kind = CertificateNodeKind::Orphan;
    CertificateAnalysisStatus status;
    std::vector<std::size_t> children;
};

struct CertificateAnalysisResult
{
    bool success = false;
    std::string message;
    std::size_t loadedCertificateCount = 0U;
    std::size_t rootCount = 0U;
    std::size_t orphanCount = 0U;
    std::vector<CertificateNode> nodes;
    std::vector<CertificateFileIssue> issues;
};

struct CertificateValidationResult
{
    bool success = false;
    bool certificateParsed = false;
    bool issuerMatched = false;
    bool issuerIsCa = false;
    bool tbsSignatureCheckAttempted = false;
    bool tbsSignatureVerified = false;
    std::string message;
    std::string issuerSubject;
    std::string tbsSignatureMessage;
    CertificateNode certificate;
    std::size_t issuerNodeId = static_cast<std::size_t>(-1);
};
}

#endif
