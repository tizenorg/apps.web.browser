#include "CertificateContents.h"
#include "BrowserLogger.h"
#include <openssl/asn1.h>
#include <openssl/bn.h>

extern "C" {
#include <db-util/db-util.h>
}

namespace tizen_browser
{

namespace base_ui
{

#define certificate_db_path     "/opt/usr/apps/org.tizen.browser/data/db/.certificate.db"

std::map<std::string, HOST_TYPE> CertificateContents::m_host_cert_info;

const std::string SECURITY_CERTIFICATE_WARNING = "There are problems with the security certificate for this site.";
const std::string SECURITY_CERTIFICATE_CONTENTS = "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. " \
                                                  "There are problems with the security certificate for this site. ";

static void _sqlite_finalize_error(sqlite3_stmt* stmt, sqlite3* descriptor, int error)
{
    BROWSER_LOGE("SQL error=%d", error);
    if (sqlite3_finalize(stmt) != SQLITE_OK)
        BROWSER_LOGE("sqlite3_finalize is failed.\n");
    db_util_close(descriptor);
}

CertificateContents::CertificateContents()
    : m_mainLayout(NULL)
    , m_parent(NULL)
    , m_viewCertButton(NULL)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_edjFilePath = EDJE_DIR;
    m_edjFilePath.append("SimpleUI/CertificatePopup.edj");
    elm_theme_extension_add(nullptr, m_edjFilePath.c_str());
}

CertificateContents::~CertificateContents()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
}

void CertificateContents::init(Evas_Object* parent)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    m_parent = parent;
    loadHostCertInfo();
}

Evas_Object* CertificateContents::getContent()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    if (!m_mainLayout)
        m_mainLayout = createCertificateLayout();

    return m_mainLayout;
}

HOST_TYPE CertificateContents::isCertExistForHost(const char *host)
{
    /*Returns the host type if a cert. exists for the host */
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    std::string key(host);
    std::map<std::string, HOST_TYPE>::const_iterator lookup_host = m_host_cert_info.find(key);
    if (lookup_host == m_host_cert_info.end())
        return HOST_ABSENT;
    else
        return m_host_cert_info[key];
}

void CertificateContents::loadHostCertInfo()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    sqlite3 *descriptor = NULL;
    int error = db_util_open(certificate_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
    if (error != SQLITE_OK) {
        db_util_close(descriptor);
        BROWSER_LOGE("Unable to open DB for loading host/cert. info");
        return;
    }
    sqlite3_stmt *sqlite3_stmt = NULL;
    error = sqlite3_prepare_v2(descriptor, "select host, allow from certificate", -1, &sqlite3_stmt, NULL);
    if (error != SQLITE_OK) {
        _sqlite_finalize_error(sqlite3_stmt, descriptor, error);
        return;
    }
    do {
        error = sqlite3_step(sqlite3_stmt);
        if (error == SQLITE_ROW) {
            std::string host((char *)sqlite3_column_text(sqlite3_stmt, 0));
            HOST_TYPE allow = (HOST_TYPE)sqlite3_column_int(sqlite3_stmt, 1);
            BROWSER_LOGD("Adding Host:%s , Host Type:%d,", host.c_str(), allow);
            m_host_cert_info[host] = allow;
        }
    } while (error == SQLITE_ROW);
    sqlite3_finalize(sqlite3_stmt);
    db_util_close(descriptor);
}

Eina_Bool CertificateContents::createCertificate(const char *certificate_data)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    /*
     *Bring the PEM cert. data into an OpenSSL memory BIO
     *This memory BIO will be used to convert the PEM cert
     *data to X509 format
     */
    BIO *cert_mem_bio = NULL;
    cert_mem_bio = BIO_new(BIO_s_mem());
    if (cert_mem_bio == NULL) {
        BROWSER_LOGE("Failed to create OpenSSL memory BIO");
        return EINA_FALSE;
    }
    BIO_puts(cert_mem_bio, certificate_data);

    /*
     *Convert from PEM to x509
     */
    m_certificate = PEM_read_bio_X509(cert_mem_bio, NULL, 0 , NULL);

    if (!m_certificate) {
        BROWSER_LOGE("PEM to x509 conversion failed");
        return EINA_FALSE;
    }

    BIO_free(cert_mem_bio);
    return EINA_TRUE;
}

Evas_Object* CertificateContents::createCertificateLayout()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    Evas_Object* layout = elm_layout_add(m_parent);
    elm_layout_file_set(layout, m_edjFilePath.c_str(), "popup_content");
    elm_layout_content_set(layout, "content_swallow", getCertificateStatusContent(layout));
    return layout;
}

Evas_Object* CertificateContents::getCertificateStatusContent(Evas_Object* parentLayout)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    std::string msg = "";
    bool certificate_created = false;

    m_host_cert_info[m_uri] = UNSECURE_HOST_ASK;
    const char *pem = ewk_certificate_policy_decision_certificate_pem_get(m_certi_policy);
    set_certificate_data(pem);
    ewk_certificate_policy_decision_suspend(m_certi_policy);

    if (!createCertificate(m_certificate_data.c_str())) {
        msg = std::string("Unable to view the certificate, the page information has been changed");
        ewk_certificate_policy_decision_allowed_set(m_certi_policy, EINA_FALSE);
    } else {
        certificate_created = true;
        if (m_cert_type == VALID)
            msg = std::string("This certificate is from a trusted authority");
        else
            msg = std::string("This certificate is not from a trusted authority");
    }

    std::string header = m_cert_type == VALID ? "Security Certificate" : "Security warning";
    m_popup->updateTitle(header.c_str());

    Evas_Object *status_content = elm_layout_add(parentLayout);
    elm_layout_file_set(status_content, m_edjFilePath.c_str(), "status_content");
    elm_layout_text_set(status_content, "main_text", msg.c_str());

    if (certificate_created) {
        m_viewCertButton = elm_button_add(status_content);
        elm_object_style_set(m_viewCertButton, "basic_button");
        evas_object_smart_callback_add(m_viewCertButton, "clicked", _view_certificate_clicked_cb, this);
        elm_object_part_content_set(status_content, "view_cert_button", m_viewCertButton);
    }
    return status_content;
}

void CertificateContents::_view_certificate_clicked_cb(void* data, Evas_Object* /*obj*/, void* /*event_info*/)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    CertificateContents* cc = static_cast<CertificateContents*>(data);
    evas_object_hide(elm_object_part_content_get(cc->m_mainLayout, "content_swallow"));
    elm_object_part_content_unset(cc->m_mainLayout, "content_swallow");
    cc->m_popup->updateTitle("Certificate contents");
    cc->m_popup->enlarge();
    //cc->populateCertificateContents();
}

#if 0
void CertificateContents::populateCertificateContents()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    /*_parse_certificate()

    Evas_Object *scroller = elm_scroller_add(m_mainLayout);
    elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_TRUE);
    elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
    evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    Evas_Object *genlist = elm_genlist_add(scroller);
    evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

    Elm_Genlist_Item_Class *item_class = elm_genlist_item_class_new();
    if (!item_class) {
            BROWSER_LOGE("elm_genlist_item_class_new for item_class failed");
            return;
    }
    item_class->item_style = "cert_content";
    item_class->func.text_get = __text_get_cb;
    item_class->func.content_get = NULL;
    item_class->func.state_get = NULL;
    item_class->func.del = NULL;
    elm_genlist_item_append(genlist, item_class, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

    elm_object_content_set(scroller, genlist);
    elm_layout_content_set(m_mainLayout, "content_swallow", scroller);*/
}

char *CertificateContents::__text_get_cb(void* /*data*/, Evas_Object* /*obj*/, const char *part)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    BROWSER_LOGD("part[%s]", part);
    return strdup(SECURITY_CERTIFICATE_CONTENTS.c_str());
}

/*
 *This method will parse the string and fetch the data
 *which is inbetween '=' and '/' or '\0'.
 */
static const char *_get_value(char *token)
{
    RETV_MSG_IF(!token, NULL, "token is NULL");

    int start = 0;
    int end = 0;
    int len = strlen(token);

    while (start < len && token[start] != '=') {
        start++;
    }
    start++; //to skip '=' char
    if (start >= len) return NULL; //couldnt find the proper value so dont add this item in genlist

    for (int i = start; i < len; i++) {
        if ((token[i] == '/') || (token[i] == '\0')) {
            break;
        }
        end++; //If no string found return the whole string
    }
    std::string token_str = token;
    std::string value = token_str.substr(start, end);
    return strdup(value.c_str());
}

/*
 *This method will format the ANS1_TIME struct to readable time format
 */
static const char *_get_formatted_time(ASN1_TIME* tm)
{
    char timeBuf[128] = {'\0', };
    BIO *sBio = BIO_new(BIO_s_mem());
    if (sBio) {
    int retVal = ASN1_TIME_print(sBio, tm);
    if (retVal <= 0) {
        BROWSER_LOGE("ASN1_TIME_print failed or wrote no data.\n");
        BIO_free(sBio);
        return NULL;
    }
    retVal = BIO_gets(sBio, timeBuf, 128);
    if (retVal <= 0) {
        BROWSER_LOGE("Failed to transfer contents to TimeBuffer");
        BIO_free(sBio);
        return NULL;
    }
    BIO_free(sBio);
    }
    return strdup(timeBuf);
}
/*
 *This method will convert the serial number in required format.
 */
static const char *_get_formatted_serial_no(ASN1_INTEGER *bs )
{
    BROWSER_LOGD("");
    char printable[100]={'\0',};
    BIGNUM *bn = ASN1_INTEGER_to_BN(bs, NULL);
    unsigned char *binSerial = NULL;
    unsigned int outsz;
    outsz = BN_num_bytes(bn);
    if (BN_is_negative(bn)) {
        outsz++;
    if (!(binSerial = (unsigned char *)malloc(outsz))) return 0;
        BN_bn2bin(bn, binSerial + 1);
        binSerial[0] = 0x80;
    } else {
        if (!(binSerial = (unsigned char *)malloc(outsz))) return 0;
        BN_bn2bin(bn, binSerial);
    }
    for(size_t i=0; i < outsz; i++) {
        char *l = (char*) (3*i + ((intptr_t) printable));
        if(i< (outsz -1))
        sprintf(l, "%02x%c", binSerial[i],':');
        else
        sprintf(l, "%02x", binSerial[i]);
    }
    free(binSerial);
    BN_free(bn);
    BROWSER_SECURE_LOGD(" New Serial Number %s",printable);
    return strdup(printable);
}

/*
 *This method is to convert binary data to hexa decimal
 */
static const char *_bin2hex (unsigned char*bin, size_t bin_size , char delimiter)
{
    BROWSER_LOGD("");
    char printable[100]={'\0',};
    for(size_t i=0; i < bin_size; i++) {
        char *l = (char*) (3*i + ((intptr_t) printable));
        sprintf(l, "%02x%c", bin[i],delimiter);
    }

    return strdup(printable);
}

void certificate_view::_parse_certificate()
{
    BROWSER_LOGD("");

    char issued_to[1024] = {'\0', };
    char issued_by[1024] = {'\0', };
    char sha256[SHA256LEN] = {'\0', };
    char sha1[SHA1LEN] = {'\0', };
    size_t size;

    for (int field_count = 0; field_count < FIELD_END; field_count++) {
        if (field_count == ISSUED_TO_HEADER) {
            //Issued to
            size = sizeof(issued_to);
            X509_NAME_oneline(X509_get_subject_name(m_certificate), issued_to, size);
            _populate_certificate_field_data(issued_to, ISSUED_TO_HEADER);
            //Serial no:
            ASN1_INTEGER *bs = X509_get_serialNumber(m_certificate);
            _generate_genlist_data(ISSUED_TO_SERIAL_NO, BR_STRING_CERTIFICATE_SERIAL_NUMBER,_get_formatted_serial_no(bs));
            field_count += 4;
        } else if (field_count == ISSUED_BY_HEADER) {
            //Issued by
            size = sizeof(issued_by);
            X509_NAME_oneline(X509_get_issuer_name(m_certificate), issued_by, size);
            _populate_certificate_field_data(issued_by, ISSUED_BY_HEADER);
            field_count += 3;
        } else if (field_count == VALIDITY_HEADER) {
            _populate_certificate_field_data(NULL, VALIDITY_HEADER);
            //Issued On
            ASN1_TIME *issuedTime = X509_get_notBefore(m_certificate);
            _generate_genlist_data(VALIDITY_ISSUED_ON, BR_STRING_ISSUED_ON, _get_formatted_time(issuedTime));
            //Expires on
            ASN1_TIME *expiresTime = X509_get_notAfter(m_certificate);
            _generate_genlist_data(VALIDITY_EXPIRES_ON, BR_STRING_EXPIRES_ON_C, _get_formatted_time(expiresTime));
            field_count += 2;
        } else if (field_count == FINGERPRINTS_HEADER) {
            _populate_certificate_field_data(NULL, FINGERPRINTS_HEADER);
            const EVP_MD *digestSHA256 = EVP_sha256();
            unsigned len1;
            int retVal = X509_digest(m_certificate, digestSHA256,(unsigned char*) sha256, &len1);
            if (retVal == 0 || len1 != SHA256LEN)
                BROWSER_SECURE_LOGE("Getting SHA256 cryptographic fingerprint failed %d",len1);
            _generate_genlist_data(FINGERPRINTS_SHA_256_FP, BR_STRING_FINGERPRINTS_SHA256, _bin2hex((unsigned char*)sha256, SHA256LEN,' '));
            const EVP_MD *digestSHA1 = EVP_sha1();
            unsigned len2;
            retVal = X509_digest(m_certificate, digestSHA1,(unsigned char*) sha1, &len2);
            if (retVal == 0 || len2 != SHA1LEN)
                BROWSER_SECURE_LOGE("Getting SHA1 cryptographic fingerprint failed %d",len2);
            _generate_genlist_data(FINGERPRINTS_SHA_1_FP, BR_STRING_FINGERPRINTS_SHA1, _bin2hex((unsigned char*)sha1, SHA1LEN,' '));
            field_count += 2;
        }
    }
}

void certificate_view::_populate_certificate_field_data(char *data, certificate_field field)
{
    const char *value = NULL;
    switch (field) {
        case ISSUED_BY_HEADER:
            _generate_genlist_data(ISSUED_BY_HEADER , BR_STRING_ISSUED_BY_C, strdup(BR_STRING_ISSUED_BY_C));
            //Get Common name
            value = _get_value(strstr(data, "CN="));
            _generate_genlist_data(ISSUED_BY_CN, BR_STRING_COMMON_NAME, value);
            //Get Orgnization
            value = _get_value(strstr(data, "O="));
            _generate_genlist_data(ISSUED_BY_ORG, BR_STRING_ORGANIZATION, value);
            //Get Orgnization UNIT
            value = _get_value(strstr(data, "OU="));
            _generate_genlist_data(ISSUED_BY_ORG_UNIT, BR_STRING_ORGANIZATION_UNIT, value);
            break;

        case ISSUED_TO_HEADER:
            _generate_genlist_data(ISSUED_TO_HEADER , BR_STRING_ISSUED_TO_C, strdup(BR_STRING_ISSUED_TO_C));
            //Get Common name
            value = _get_value(strstr(data, "CN="));
            _generate_genlist_data(ISSUED_TO_CN, BR_STRING_COMMON_NAME, value);
            //Get Orgnization
            value = _get_value(strstr(data, "O="));
            _generate_genlist_data(ISSUED_TO_ORG, BR_STRING_ORGANIZATION, value);
            //Get Orgnization UNIT
            value = _get_value(strstr(data, "OU="));
            _generate_genlist_data(ISSUED_TO_ORG_UNIT, BR_STRING_ORGANIZATION_UNIT, value);
            break;

        case VALIDITY_HEADER:
            _generate_genlist_data(VALIDITY_HEADER , BR_STRING_VALIDITY_C, strdup(BR_STRING_VALIDITY_C));
            break;

        case FINGERPRINTS_HEADER:
            _generate_genlist_data(FINGERPRINTS_HEADER , BR_STRING_FINGERPRINTS, strdup(BR_STRING_FINGERPRINTS));
            break;

        default:
            break;
    }
}

void certificate_view::_generate_genlist_data(certificate_field field_type, const char *title, const char *value)
{
    if (!value)
        return;
    genlist_callback_data *gl_data = (genlist_callback_data *)malloc(sizeof(genlist_callback_data));
    if(!gl_data)
        return;
    memset(gl_data, 0x00, sizeof(genlist_callback_data));
    gl_data->type = field_type;
    gl_data->title = title;
    gl_data->value = value;
    m_genlist_callback_data_list.push_back(gl_data);
}

char *certificate_view::__genlist_status_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
    BROWSER_LOGD("Part %s",part);
    if (!strcmp(part, "elm.text.multiline")) {
        if(m_browser->get_browser_view()->get_current_webview()->get_CertificateContents()->__is_valid_certificate())
            return strdup(BR_STRING_TRUSTED_AUTHORITY);
        else
            return strdup(BR_STRING_UNTRUSTED_AUTHORITY);
    }
    return NULL;
}

char *certificate_view::__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
    genlist_callback_data *callback_data = (genlist_callback_data *)data;

    if (!strcmp(part, "elm.text.main")) {
        return strdup(callback_data->title);
    } else if (!strcmp(part,"elm.text.multiline")) {
        return strdup(callback_data->value);
    }
    return NULL;
}
#endif

}
}
