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

#include <Evas.h>
#include <Elementary.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <ewk_chromium.h>

#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

namespace tizen_browser
{

namespace base_ui
{

class CertificateContents
{
public:
    CertificateContents();
    ~CertificateContents();

    enum CERT_TYPE {
        NONE,
        VALID,
        INVALID
    };

    enum HOST_TYPE {
        SECURE_HOST = 1,
        UNSECURE_HOST_ALLOWED,
        UNSECURE_HOST_UNKNOWN,
        UNSECURE_HOST_ASK,
        HOST_ABSENT = -1
    };

    struct CurrentTabCertificateData {
        CERT_TYPE _cert_type;
        std::string _pem;
        Ewk_Certificate_Policy_Decision *_cert_policy;
    };

    void init(Evas_Object* parent);
    Evas_Object* getContent();
    void unsecureHostAllowed();

    void setCurrentTabCertData(const CurrentTabCertificateData& data);
    void setCertPolicy(Ewk_Certificate_Policy_Decision* policy) { m_certData._cert_policy = policy; }
    Eina_Bool isValidCertificate() const { return m_certData._cert_type == VALID; }

    static HOST_TYPE isCertExistForHost(const std::string& host);
    static void addToHostCertList(const std::string& host, HOST_TYPE type);

private:

    typedef enum _certificate_field{
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
    } certificate_field;

    struct genlist_callback_data {
        certificate_field type;
        const char *title;
        const char *value;
    };

    bool createCertificate(const char *cert_data);
    Evas_Object* createGenlist(Evas_Object* parent);
    Evas_Object* createLabel(Evas_Object* parent,  const std::string& msg);
    void setCertType(CERT_TYPE type) { m_certData._cert_type = type; }
    void _parse_certificate();
    void _populate_certificate_field_data(char *data, certificate_field field);
    void _generate_genlist_data(certificate_field type, const char *title, const char *value);

    static char* __auth_text_get_cb(void* data, Evas_Object* obj, const char *part);
    static char* __field_text_get_cb(void* data, Evas_Object* obj, const char *part);
    static char* __title_value_text_get_cb(void* data, Evas_Object* obj, const char *part);

    Evas_Object* m_mainLayout;
    Evas_Object* m_genlist;
    Evas_Object* m_parent;
    std::vector<std::shared_ptr<genlist_callback_data> > m_genlist_callback_data_list;
    std::string m_edjFilePath;

    static std::map<std::string, HOST_TYPE> m_host_cert_info;

    X509 *m_certificate;
    CurrentTabCertificateData m_certData;

};

}
}
