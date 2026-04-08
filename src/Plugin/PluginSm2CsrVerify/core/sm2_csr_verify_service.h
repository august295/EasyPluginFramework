#ifndef __SM2_CSR_VERIFY_SERVICE_H__
#define __SM2_CSR_VERIFY_SERVICE_H__

#include "sm2_csr_verify_types.h"

namespace core
{
class Sm2CsrVerifyService
{
public:
    Sm2CsrVerifyResult verify(const Sm2CsrVerifyRequest& request) const;
};
}

#endif
