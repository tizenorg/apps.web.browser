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

#include "Certificate.h"

/*
 *This method will parse the string and fetch the data
 *which is inbetween '=' and '/' or '\0'.
 */
static const char *_get_value(char *token)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);

    if (!token)
        return NULL;

    BROWSER_LOGD("token %s", token);
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
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
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
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
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
    BROWSER_LOGD(" New Serial Number %s",printable);
    return strdup(printable);
}

/*
 *This method is to convert binary data to hexa decimal
 */
static const char *_bin2hex(unsigned char*bin, size_t bin_size , char delimiter)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
    char printable[100]={'\0',};
    for(size_t i=0; i < bin_size; i++) {
        char *l = (char*) (3*i + ((intptr_t) printable));
        sprintf(l, "%02x%c", bin[i],delimiter);
    }

    return strdup(printable);
}

void Certificates::_parse_certificate()
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
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
                BROWSER_LOGE("Getting SHA256 cryptographic fingerprint failed %d",len1);
            _generate_genlist_data(FINGERPRINTS_SHA_256_FP, BR_STRING_FINGERPRINTS_SHA256, _bin2hex((unsigned char*)sha256, SHA256LEN,' '));
            const EVP_MD *digestSHA1 = EVP_sha1();
            unsigned len2;
            retVal = X509_digest(m_certificate, digestSHA1,(unsigned char*) sha1, &len2);
            if (retVal == 0 || len2 != SHA1LEN)
                BROWSER_LOGE("Getting SHA1 cryptographic fingerprint failed %d",len2);
            _generate_genlist_data(FINGERPRINTS_SHA_1_FP, BR_STRING_FINGERPRINTS_SHA1, _bin2hex((unsigned char*)sha1, SHA1LEN,' '));
            field_count += 2;
        }
    }
}

void Certificate::_populate_certificate_field_data(char *data, certificate_field field)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
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

void Certificate::_generate_genlist_data(certificate_field field_type, const char *title, const char *value)
{
    BROWSER_LOGD("[%s:%d] ", __PRETTY_FUNCTION__, __LINE__);
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
