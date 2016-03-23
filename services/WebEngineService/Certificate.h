/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef CERTIFICATE_H
#define CERTIFICATE_H

#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>


class Certificate
{
public:
    enum class CERT_TYPE {
        NONE,
        VALID,
        INVALID
    };

    enum class HOST_TYPE {
        SECURE_HOST = 1,
        UNSECURE_HOST_ALLOWED,
        UNSECURE_HOST_UNKNOWN,
        UNSECURE_HOST_ASK,
        HOST_ABSENT = -1
    };
    
private:
    enum class certificate_field {
        ISSUED_TO_HEADER = 0,
        ISSUED_TO_CN,
        ISSUED_TO_ORG,
        ISSUED_TO_ORG_UNIT,
        ISSUED_TO_SERIAL_NO,
        ISSUED_BY_HEADER,
        ISSUED_BY_CN,
        ISSUED_BY_ORG,
        ISSUED_BY_ORG_UNIT,
        VALIDITY_HEADER,
        VALIDITY_ISSUED_ON,
        VALIDITY_EXPIRES_ON,
        FINGERPRINTS_HEADER,
        FINGERPRINTS_SHA_256_FP,
        FINGERPRINTS_SHA_1_FP,
        FIELD_END
    };

    struct genlist_callback_data {
        certificate_field type;
        const char *title;
        const char *value;
    };

    X509 *m_certificate;
};

#endif // CERTIFICATE_H
