/*
Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved

This file is part of browser
Written by Hyerim Bae hyerim.bae@samsung.com.

PROPRIETARY/CONFIDENTIAL

This software is the confidential and proprietary information of
SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered
into with SAMSUNG ELECTRONICS.

SAMSUNG make no representations or warranties about the suitability
of the software, either express or implied, including but not limited
to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. SAMSUNG shall not be liable for any
damages suffered by licensee as a result of using, modifying or
distributing this software or its derivatives.
*/

#include <cstdio>
#include <ctime>
#include <dirent.h>
#include <fstream>
#include <iostream>

#include "browser-class.h"
#include "browser-certificate-manager.h"
#include "browser-view.h"
#include "browser-window.h"

Browser_Certificate_Manager::Browser_Certificate_Manager(void)
:	m_certificate_list(NULL)
	,m_certificate(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Certificate_Manager::~Browser_Certificate_Manager(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_certificate) {
		delete [] m_certificate->data;
		delete m_certificate;
		m_certificate = NULL;
	}

	_destroy_certificate_list();

	remove(CERTIFICATES_TMP_FILE);
}

Eina_Bool Browser_Certificate_Manager::init(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!_create_crt_file(CERTIFICATES_DIR, CERTIFICATES_TMP_FILE)) {
		BROWSER_LOGE("_create_crt_file failed");
		return EINA_FALSE;
	}

	ewk_set_certificate_file(CERTIFICATES_TMP_FILE);
	ewk_set_certificate_confirmation_callback(__certificate_confirm_cb);

	return EINA_TRUE;
}

void Browser_Certificate_Manager::reset_certificate(void)
{
	BROWSER_LOGD("[%s]", __func__);
	_destroy_certificate_list();
}

Eina_Bool Browser_Certificate_Manager::__certificate_confirm_cb(Eina_Bool is_trused, const char *uri,
									char *certificate_source, int error)
{
	BROWSER_LOGD("[%s]", __func__);

	std::string domain = m_data_manager->get_browser_view()->get_domain_name(uri);
	BROWSER_LOGD("domain=[%s], error=%d", domain.c_str(), error);

	Evas_Object *webview = m_data_manager->get_browser_view()->get_focused_webview();
	Evas_Object *webkit = elm_webview_webkit_get(webview);

	Browser_Certificate_Manager *certificate_manager = NULL;
	certificate_manager = m_browser->get_certificate_manager();
	gnutls_datum_t *certificate = certificate_manager->_create_certificate(certificate_source);
	if (!certificate)
		BROWSER_LOGE("_create_certificate failed");

	std::string error_string;
	if (error & UNKNOWN_CA) {
		/* SS_UNKNOWN_CERTIFICATE_AUTHORITY */
		error_string = "Unknown certificate authority";
	} else if (error & BAD_IDENTITY) {
		error_string = "Certificate : Bad-Identity";
	} else if (error & NOT_ACTIVATED) {
		error_string = "Certificate is not activated";
	} else if (error & EXPIRED) {
		/* SS_CERTIFICATE_EXPIRED */
		error_string = "Certificate expired";
	} else if (error & REVOKED) {
		/* SS_CERTIFICATE_REVOKED */
		error_string = "Certificate revoked";
	} else if (error & INSECURE) {
		error_string = "Certificate insecured";
	} else if (error & GENERIC_ERROR) {
		/* SS_CERTIFICATE_ERROR */
		error_string = "Certificate error";
	}

	if (certificate)
		certificate_manager->_print_certificate(certificate);

	if (!error_string.empty()) {
		ewk_view_suspend_request(webkit);

		if (!certificate_manager->show_modal_popup(error_string.c_str()))
			BROWSER_LOGE("show_modal_popup failed");

		ewk_view_resume_request(webkit);
	}

	if (certificate)
		delete certificate;

	return EINA_TRUE;
}

void Browser_Certificate_Manager::_destroy_certificate_list(void)
{
	BROWSER_LOGD("[%s]", __func__);
	void *item_data = NULL;
	EINA_LIST_FREE(m_certificate_list, item_data) {
		if (item_data) {
			gnutls_datum_t *certificate_item = (gnutls_datum_t *)item_data;
			delete [] certificate_item->data;
			delete certificate_item;
		}
	}
}

static int _pem_file_selector(const struct dirent *dir_entity)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!dir_entity->d_name)
		return 0;

	int len = strlen(dir_entity->d_name);
	if (len < 4)
		return 0;

	if (strncmp(dir_entity->d_name + (len - 4), ".pem", 4 ) == 0)
		return 1;

	return 0;
}

Eina_Bool Browser_Certificate_Manager::_create_crt_file(const char *dir_path, const char *des_file_path)
{
	BROWSER_LOGD("[%s]", __func__);
	struct dirent **name_list = NULL;
	int result = scandir(dir_path, &name_list, _pem_file_selector, alphasort);
	if (result < 0)
		return EINA_FALSE;

	ofstream dest_file(des_file_path);
	dest_file.seekp (0, ios::end);
	std::string line;

	if (!dest_file.is_open())
		return EINA_FALSE;

	while (result--) {
		std::string full_patch = std::string(dir_path) + std::string(name_list[result]->d_name);
		ifstream src_file(full_patch.c_str());
		if( src_file.is_open()) {
			while (!src_file.eof()) {
				getline(src_file, line);
				dest_file << line;
				if (!src_file.eof())
					dest_file << std::endl;
			}
			src_file.close();
		}

		free(name_list[result]);
	}

	free(name_list);
	dest_file.close();

	return EINA_TRUE;
}

void Browser_Certificate_Manager::_print_certificate(const gnutls_datum_t *certificate)
{
	BROWSER_LOGD("[%s]", __func__);
	gnutls_datum_t cinfo;
	gnutls_x509_crt_t cert;

	gnutls_x509_crt_init(&cert);
	gnutls_x509_crt_import(cert, certificate, GNUTLS_X509_FMT_DER);

	int ret = gnutls_x509_crt_print(cert, GNUTLS_CRT_PRINT_ONELINE, &cinfo);
	if (!ret) {
		BROWSER_LOGD ("%s", cinfo.data);
		gnutls_free (cinfo.data);
	}
}

gnutls_datum_t *Browser_Certificate_Manager::_create_certificate(char *certificate_source)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!certificate_source || !strlen(certificate_source)) {
		BROWSER_LOGE("certificate_source is null");
		return NULL;
	}

	gnutls_datum_t *certificate = new(nothrow) gnutls_datum_t;
	if (!certificate) {
		BROWSER_LOGE("new(nothrow) gnutls_datum_t failed");
		return NULL;
	}
	certificate->size = strlen(certificate_source);
	certificate->data = new(nothrow) unsigned char[certificate->size];
	if (!certificate->data) {
		BROWSER_LOGE("new(nothrow) failed");
		return NULL;
	}
	memcpy(certificate->data, certificate_source, certificate->size);

	return certificate;
}

Eina_Bool Browser_Certificate_Manager::_create_certificate_list(Eina_List *certificate_list)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!certificate_list) {
		BROWSER_LOGE("certificate_list failed");
		return EINA_FALSE;
	}

	const gnutls_datum_t *certificate;
	Eina_List *list = NULL;
	void *item_data = NULL;

	EINA_LIST_FOREACH(certificate_list, list, item_data) {
		certificate = reinterpret_cast<const gnutls_datum_t *>(item_data);
		gnutls_datum_t *certificate_item = new(nothrow) gnutls_datum_t;
		if (!certificate_item) {
			BROWSER_LOGE("new gnutls_datum_t failed");
			return EINA_FALSE;
		}
		certificate_item->size = certificate->size;
		certificate_item->data = new(nothrow) unsigned char[certificate_item->size];
		if (!certificate_item->data) {
			BROWSER_LOGE("new unsigned char failed");
			return EINA_FALSE;
		}
		memcpy(certificate_item->data, certificate->data, certificate_item->size);

		m_certificate_list = eina_list_append(m_certificate_list, certificate_item);
	}

	return EINA_TRUE;
}
