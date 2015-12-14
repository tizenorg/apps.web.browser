/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <list>
#include <memory>
#include "ContentPopup_mob.h"
#include <ewk_chromium.h>

#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

namespace tizen_browser
{

namespace base_ui
{

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

typedef struct _genlist_callback_data {
    certificate_field type;
    const char *title;
    const char *value;
} genlist_callback_data;

enum HOST_TYPE {
    SECURE_HOST = 1,
    UNSECURE_HOST_ALLOWED,
    UNSECURE_HOST_UNKNOWN,
    UNSECURE_HOST_ASK,
    HOST_ABSENT = -1
};

enum CERT_TYPE {
    NONE,
    VALID,
    INVALID
};

class CertificateContents
{
public:
    CertificateContents();
    ~CertificateContents();

    void init(Evas_Object* parent);
    Evas_Object* getContent();

    void setHostCertPolicy(const std::string& uri, Ewk_Certificate_Policy_Decision* policy) {
        m_uri = uri;
        m_certi_policy = policy;
    }

    static HOST_TYPE isCertExistForHost(const char *host);
    void setOwnerPopup(ContentPopup* popup) { m_popup = popup; }
    void set_certificate_data(const char *cert_data) { m_certificate_data.assign(cert_data); }
    void set_cert_type(CERT_TYPE type) { m_cert_type = type; }
    Eina_Bool isValidCertificate() const { return m_cert_type == VALID; }
    void unsecureHostAllowed();

    static Eina_Bool saveCertificateInfo(const char *pem, const char *host, HOST_TYPE allow);
    static Eina_Bool deleteAllSavedCertificates(void);
    static Eina_Bool updateCertificateInfo(const char *pem, const char *host, HOST_TYPE allow);

private:

    Eina_Bool createCertificate(const char *cert_data);
    Evas_Object* createCertificateLayout();
    Evas_Object* getCertificateStatusContent(Evas_Object* parentLayout);
    Evas_Object* createGenlist(Evas_Object* parent);
    static void loadHostCertInfo();
    void populateCertificateContents();
    void _parse_certificate();
    void _populate_certificate_field_data(char *data, certificate_field field);
    void _generate_genlist_data(certificate_field type, const char *title, const char *value);

    static void _view_certificate_clicked_cb(void* data, Evas_Object* obj, void* event_info);
    static char* __auth_text_get_cb(void* data, Evas_Object* obj, const char *part);
    static char* __field_text_get_cb(void* data, Evas_Object* obj, const char *part);
    static char* __title_value_text_get_cb(void* data, Evas_Object* obj, const char *part);

    Evas_Object* m_mainLayout;
    Evas_Object* m_genlist;
    Evas_Object* m_parent;
    Evas_Object* m_viewCertButton;
    ContentPopup* m_popup;
    std::string m_uri;

    X509 *m_certificate;
    CERT_TYPE m_cert_type;
    std::string m_certificate_data;
    Ewk_Certificate_Policy_Decision *m_certi_policy;
    static std::map<std::string, HOST_TYPE> m_host_cert_info;
    std::vector<genlist_callback_data *> m_genlist_callback_data_list;

    std::string m_edjFilePath;
};

}
}
