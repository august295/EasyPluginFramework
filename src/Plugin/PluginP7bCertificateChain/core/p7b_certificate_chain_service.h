#ifndef __P7B_CERTIFICATE_CHAIN_SERVICE_H__
#define __P7B_CERTIFICATE_CHAIN_SERVICE_H__

#include "p7b_certificate_chain_types.h"

namespace core
{
class P7bCertificateChainService
{
public:
    P7bCertificateChainResult parse(const P7bCertificateChainRequest& request) const;
};
}

#endif
