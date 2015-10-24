/*
 *  browser
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Karthick R <karthick.r@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <Elementary.h>
#include <app.h>

#include "certificate-view.h"
#include "browser.h"
#include "browser-view.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include <openssl/asn1.h>
#include <openssl/bn.h>
#include "platform-service.h"
#include "webview.h"

#if defined(HW_MORE_BACK_KEY)
#include <efl_extension.h>
#endif
#define SHA256LEN 32
#define SHA1LEN 20
#define browser_genlist_edj_path browser_edj_dir"/browser-genlist.edj"
#define browser_popup_edj_path browser_edj_dir"/browser-popup-lite.edj"

certificate_view::certificate_view(X509 *cert, Evas_Smart_Cb cb_func, void *cb_data)
:
	m_genlist_callback_data_list()
	, m_genlist(NULL)
	, m_certificate_info_popup(NULL)
	, m_theme(NULL)
	, m_naviframe_item(NULL)
	, m_certificate(cert)
	, m_cb_func(cb_func)
	, m_cb_data(cb_data)
{
	BROWSER_LOGD("");
	m_theme = elm_theme_new();
	elm_theme_ref_set(m_theme, NULL);
	elm_theme_extension_add(m_theme, browser_popup_edj_path);
}

certificate_view::~certificate_view(void)
{
	BROWSER_LOGD("");

	int size = m_genlist_callback_data_list.size();
	for (int i = 0; i < size; i++) {
		if (m_genlist_callback_data_list[i])
			if (m_genlist_callback_data_list[i]->value)
				free((char *)m_genlist_callback_data_list[i]->value);
			free(m_genlist_callback_data_list[i]);
		m_genlist_callback_data_list[i] = NULL;
	}
	m_genlist_callback_data_list.clear();

	elm_theme_extension_del(m_theme, browser_popup_edj_path);
	elm_theme_free(m_theme);

	if (m_certificate_info_popup)
		evas_object_del(m_certificate_info_popup);
}

void certificate_view::__certifciate_popup_cancel_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	certificate_view *cert_view = (certificate_view *)data;
	if (cert_view && cert_view->m_certificate_info_popup) {
		evas_object_del(cert_view->m_certificate_info_popup);
		cert_view->m_certificate_info_popup = NULL;
	}
	m_browser->delete_certificate_view();
}

void certificate_view::show_certificate_info_popup(void)
{

	BROWSER_LOGD("");
	_parse_certificate();

	Evas_Object *popup = brui_popup_add(m_window);
	RET_MSG_IF(!popup, "popup is NULL");
	elm_popup_align_set(popup, -1.0, 1.0);
	m_certificate_info_popup = popup;

	/* Popup layout */
	Evas_Object *popup_layout = elm_layout_add(popup);
	elm_layout_file_set(popup_layout, browser_edj_dir"/browser-popup-lite.edj", "certificate_content_layout");
	evas_object_size_hint_weight_set(popup_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
#if defined(HW_MORE_BACK_KEY)
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __certifciate_popup_cancel_btn_clicked_cb, this);
#endif
	/* scroller */
	Evas_Object *scroller = elm_scroller_add(popup_layout);
	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_object_part_content_set(popup_layout, "elm.swallow.content", scroller);

	/* genlist content */
	Evas_Object *genlist = _create_genlist(scroller);
	elm_object_content_set(scroller, genlist);
	elm_object_content_set(popup, popup_layout);

	m_browser->get_browser_view()->show_content_popup(popup, BR_STRING_SECURITY_CERTIFICATE,
									popup_layout,
									NULL,
									"IDS_BR_SK_OK", __certifciate_popup_cancel_btn_clicked_cb,
#if defined(HW_MORE_BACK_KEY)
									NULL, NULL,
#else
									"IDS_BR_SK_CANCEL", __certifciate_popup_cancel_btn_clicked_cb,
#endif
									this);
}

void certificate_view::show(void)
{
	BROWSER_LOGD ("");
	_parse_certificate();
	m_genlist = _create_genlist(m_naviframe);

	m_naviframe_item = elm_naviframe_item_push(m_naviframe, "IDS_BR_HEADER_SECURITY_CERTIFICATE", NULL, NULL, m_genlist, NULL);
	elm_object_item_domain_text_translatable_set(m_naviframe_item, BROWSER_DOMAIN_NAME, EINA_TRUE);

	elm_naviframe_item_pop_cb_set(m_naviframe_item, __naviframe_pop_cb, this);
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
static const char *_get_formatted_serial_no(ASN1_INTEGER *bs ){
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
		if(m_browser->get_browser_view()->get_current_webview()->get_certificate_manager()->__is_valid_certificate())
			return strdup(BR_STRING_TRUSTED_AUTHORITY);
		else
			return strdup(BR_STRING_UNTRUSTED_AUTHORITY);
	}
	return NULL;
}

Evas_Object *certificate_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	elm_object_theme_set(genlist, m_theme);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Elm_Genlist_Item_Class *title_item_class = elm_genlist_item_class_new();
	if (!title_item_class) {
		BROWSER_LOGE("elm_genlist_item_class_new for title_item_class failed");
		return EINA_FALSE;
	}
	title_item_class->item_style = "browser/certificate_view/groupindex";
	title_item_class->func.text_get = __genlist_label_get_cb;
	title_item_class->func.content_get = NULL;
	title_item_class->func.state_get = NULL;
	title_item_class->func.del = NULL;

	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	if(!item_ic) {
		if(title_item_class)
			elm_genlist_item_class_free(title_item_class);
		return NULL;
	}
	item_ic->item_style = "browser/certificate_view/multiline_sub.main";
	item_ic->decorate_item_style = NULL;
	item_ic->func.text_get = __genlist_label_get_cb;
	item_ic->func.content_get = NULL;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;

	Elm_Genlist_Item_Class *status_item_class = elm_genlist_item_class_new();
	if(!status_item_class) {
		if(item_ic)
			elm_genlist_item_class_free(item_ic);
		if(title_item_class)
			elm_genlist_item_class_free(title_item_class);
		return NULL;
	}
	status_item_class->item_style = "browser/certificate_view/multiline_main";
	status_item_class->decorate_item_style = NULL;
	status_item_class->func.text_get = __genlist_status_label_get_cb;
	status_item_class->func.content_get = NULL;
	status_item_class->func.state_get = NULL;
	status_item_class->func.del = NULL;
	Elm_Object_Item *status_it = NULL;
	status_it = elm_genlist_item_append(genlist, status_item_class, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(status_it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	int no_of_items = m_genlist_callback_data_list.size();
	for (int i = 0; i < no_of_items; i++) {
		Elm_Object_Item *it = NULL;
		genlist_callback_data *callback_data = (genlist_callback_data *)m_genlist_callback_data_list[i];
		if (callback_data->type == ISSUED_BY_HEADER ||callback_data->type == ISSUED_TO_HEADER ||
							callback_data->type == VALIDITY_HEADER ||callback_data->type == FINGERPRINTS_HEADER) {
			it = elm_genlist_item_append(genlist, title_item_class, callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, callback_data);
		} else
			it = elm_genlist_item_append(genlist, item_ic, callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, callback_data);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

	evas_object_show(genlist);
	if(item_ic)
		elm_genlist_item_class_free(item_ic);
	if(title_item_class)
		elm_genlist_item_class_free(title_item_class);
	return genlist;
}

char *certificate_view::__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part)
{
	RETV_MSG_IF(!data, NULL, "data is NULL");
	RETV_MSG_IF(!part, NULL, "part is NULL");

	genlist_callback_data *callback_data = (genlist_callback_data *)data;

	if (!strcmp(part, "elm.text.main")) {
		return strdup(callback_data->title);
	} else if (!strcmp(part,"elm.text.multiline")) {
		return strdup(callback_data->value);
	}
	return NULL;
}

Eina_Bool certificate_view::__naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, EINA_FALSE, "data is NULL");

	certificate_view *cp = (certificate_view *)data;

	if (cp->m_cb_func) {
		cp->m_cb_func(cp->m_cb_data, NULL, NULL);
	}

	return EINA_TRUE;
}
