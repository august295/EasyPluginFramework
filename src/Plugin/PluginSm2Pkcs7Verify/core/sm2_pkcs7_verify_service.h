#ifndef __SM2_PKCS7_VERIFY_SERVICE_H__
#define __SM2_PKCS7_VERIFY_SERVICE_H__

#include "sm2_pkcs7_verify_types.h"

namespace core
{
class Sm2Pkcs7VerifyService
{
public:
    Sm2Pkcs7VerifyResult verify(const Sm2Pkcs7VerifyRequest& request) const;
};
}

#endif
