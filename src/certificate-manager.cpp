
/*
 *  browser
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Tiwari Deepak <t.deepak@samsung.com>
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

#include "certificate-manager.h"

#include <Elementary.h>
#include <string>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "common-view.h"
#include "certificate-view.h"
#include "preference.h"
#include "platform-service.h"
#include "url-input-bar.h"
#include "webview.h"

#if defined(HW_MORE_BACK_KEY)
#include <efl_extension.h>
#endif

extern "C" {
#include "db-util.h"
}

#define certificate_db_path	"/opt/usr/apps/"BROWSER_APP_NAME"/data/db/.certificate.db"

map<string,HOST_TYPE> certificate_manager::m_host_cert_info;

int const certificate_manager::STATUS_FONT_SIZE = 25;

static void _sqlite_finalize_error(sqlite3_stmt* stmt, sqlite3* descriptor, int error)
{
	BROWSER_LOGE("SQL error=%d", error);
	if (sqlite3_finalize(stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	db_util_close(descriptor);
}

HOST_TYPE certificate_manager::__is_exist_cert_for_host(const char *host)
{
	/*Returns the host type if a cert. exists for the host */

	BROWSER_LOGD("");
	RETV_MSG_IF(!host, HOST_ABSENT, "host is NULL");

	string key(host);
	map<string, HOST_TYPE>::const_iterator lookup_host = m_host_cert_info.find(key);
	if (lookup_host == m_host_cert_info.end())
		return HOST_ABSENT;
	else
		return m_host_cert_info[key];
}

void certificate_manager::load_host_cert_info()
{
	BROWSER_LOGD("");
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
			string host((char *)sqlite3_column_text(sqlite3_stmt, 0));
			HOST_TYPE allow = (HOST_TYPE)sqlite3_column_int(sqlite3_stmt, 1);
			BROWSER_SECURE_LOGD("Adding Host:%s , Host Type:%d,", host.c_str(), allow);
			m_host_cert_info[host] = allow;
		}
	} while (error == SQLITE_ROW);
	sqlite3_finalize(sqlite3_stmt);
	db_util_close(descriptor);
}

Eina_Bool certificate_manager::_update_certificate_info(const char *pem, const char *host,HOST_TYPE allow)
{
	/*In case request_certi_cb or set_certificate_data_cb are called again for a host that already
	exists in the DB, we will simply update the row instead of adding a new row to the DB*/

	RETV_MSG_IF(!pem, EINA_FALSE, "pem is NULL");

	sqlite3 *descriptor = NULL;
	int error = db_util_open(certificate_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "update certificate set pem = ?, allow = ? where host = ?", -1, &sqlite3_stmt, NULL);
	if (sqlite3_bind_text(sqlite3_stmt, 1, pem, -1, NULL) != SQLITE_OK) {
		_sqlite_finalize_error(sqlite3_stmt, descriptor, error);
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 2, allow) != SQLITE_OK) {
		_sqlite_finalize_error(sqlite3_stmt, descriptor, error);
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 3, host, -1, NULL) != SQLITE_OK) {
		_sqlite_finalize_error(sqlite3_stmt, descriptor, error);
		return EINA_FALSE;
	}
	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	db_util_close(descriptor);

	m_host_cert_info[host] = allow;
	return EINA_TRUE;
}

Eina_Bool certificate_manager::_save_certificate_info(const char *pem, const char *host, HOST_TYPE allow)
{
	RETV_MSG_IF(!pem, EINA_FALSE, "pem is NULL");

	/* Adding support to save certificates in browser
	cert DB since WebKit doesn't support cert. caching :-(
	*/
	sqlite3 *descriptor = NULL;
	int error = db_util_open(certificate_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "insert into certificate (pem, host, allow) values(?, ?, ?)", -1, &sqlite3_stmt, NULL);
	if (sqlite3_bind_text(sqlite3_stmt, 1, pem, -1, NULL) != SQLITE_OK) {
		_sqlite_finalize_error(sqlite3_stmt, descriptor, error);
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 2, host, -1, NULL) != SQLITE_OK) {
		_sqlite_finalize_error(sqlite3_stmt, descriptor, error);
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 3, allow) != SQLITE_OK) {
		_sqlite_finalize_error(sqlite3_stmt, descriptor, error);
		return EINA_FALSE;
	}
	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	db_util_close(descriptor);

	m_host_cert_info[host] = allow;
	return EINA_TRUE;
}

Eina_Bool certificate_manager::_delete_all(void)
{
	BROWSER_LOGD("");
	sqlite3 *descriptor = NULL;
	int error = db_util_open(certificate_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "delete from certificate", -1, &sqlite3_stmt, NULL);
	if (sqlite3_step(sqlite3_stmt) != SQLITE_ROW)
		BROWSER_LOGE("sqlite3_step failed");

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_finalize failed");
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	db_util_close(descriptor);
	m_host_cert_info.clear();

	return EINA_TRUE;
}

int certificate_manager::m_instance_count;

certificate_manager::certificate_manager(webview *wv)
:
	m_webview(wv)
	, m_certificate_data()
	, m_certificate(NULL)
	, m_certificate_popup(NULL)
	, m_certificate_popup_layout(NULL)
	, m_certi_policy(NULL)
	, m_cert_type(NONE)
	, m_untrusted_cert_data_idler(NULL)
{
	BROWSER_LOGD("");
	m_webview->attach_event("request,certificate,confirm", __request_certi_cb, this);
	if (m_instance_count == 0) {
		certificate_manager::load_host_cert_info();
	}
	m_instance_count++;
}

certificate_manager::~certificate_manager(void)
{
	BROWSER_LOGD("");

	if (m_certificate_popup)
		evas_object_smart_callback_del(m_certificate_popup, "language,changed", __certificate_popup_lang_changed_cb);
	m_certificate_popup_layout = NULL;
	m_webview->detach_event("request,certificate,confirm", __request_certi_cb);
	evas_object_smart_callback_del(m_webview->get_ewk_view(), "load,finished", __load_finished_cb);
	if(m_certificate) {
		X509_free(m_certificate);
		m_certificate = NULL;
	}
	m_instance_count--;
	if (m_untrusted_cert_data_idler)
		ecore_idler_del(m_untrusted_cert_data_idler);
}

void certificate_manager::on_pause(void)
{
	BROWSER_LOGD("");
	if (m_certificate_popup && m_certi_policy && m_cert_type == INVALID) {
		__certi_deny_cb(this, NULL, NULL);
		common_view::clear_popups();
		m_certificate_popup = NULL;
	}
}

void certificate_manager::__certi_allow_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");
	certificate_manager *cm = (certificate_manager*)data;
	ewk_certificate_policy_decision_allowed_set(cm->m_certi_policy, EINA_TRUE);
	const char *pem = ewk_certificate_policy_decision_certificate_pem_get(cm->m_certi_policy);
	cm->m_certi_policy = NULL;
	cm->m_certificate_popup = NULL;
	if (__is_exist_cert_for_host(cm->m_webview->get_uri()) == HOST_ABSENT)
		certificate_manager::_save_certificate_info(pem, cm->m_webview->get_uri(), UNSECURE_HOST_ALLOWED);
	else
		certificate_manager::_update_certificate_info(pem, cm->m_webview->get_uri(), UNSECURE_HOST_ALLOWED);
}

void certificate_manager::__certi_deny_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");
	certificate_manager *cm = (certificate_manager*)data;

	m_host_cert_info[cm->m_webview->get_uri()] = HOST_ABSENT;	// will ask again to accept certificate next time
	ewk_certificate_policy_decision_allowed_set(cm->m_certi_policy, EINA_FALSE);
	cm->m_certi_policy = NULL;
	cm->m_certificate_popup = NULL;
	evas_object_smart_callback_add(cm->m_webview->get_ewk_view(), "load,finished", __load_finished_cb, cm);
}

void certificate_manager::__request_certi_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	RET_MSG_IF(!(data), "data is NULL");
	RET_MSG_IF(!(event_info), "event_info is NULL");
	certificate_manager *cm = (certificate_manager *)data;
	Ewk_Certificate_Policy_Decision *certi_policy = (Ewk_Certificate_Policy_Decision *)event_info;
	evas_object_smart_callback_del(cm->m_webview->get_ewk_view(), "load,finished", __load_finished_cb);

	/*If the warnings are turned off we will store the cert as UNSECURE_HOST_UNKNOWN else
	we respect the user actions i.e to load or cancel. In case the user selects load, we will be storing
	the cert in __certi_allow_cb*/
	if (!m_preference->get_certificate_warnings_enabled()) {
		const char *pem = ewk_certificate_policy_decision_certificate_pem_get(certi_policy);
		if (__is_exist_cert_for_host(cm->m_webview->get_uri()) == HOST_ABSENT)
			certificate_manager::_save_certificate_info(pem, cm->m_webview->get_uri(), UNSECURE_HOST_UNKNOWN);
		else
			certificate_manager::_update_certificate_info(pem, cm->m_webview->get_uri(), UNSECURE_HOST_UNKNOWN);
		ewk_certificate_policy_decision_allowed_set(certi_policy, EINA_TRUE);
		return;
	}

	HOST_TYPE type = __is_exist_cert_for_host(cm->m_webview->get_uri());
	if (type == UNSECURE_HOST_ALLOWED) {
		ewk_certificate_policy_decision_allowed_set(certi_policy, EINA_TRUE);
	} else if (type != UNSECURE_HOST_ASK) {		// do not ask user two times for the same certificate
		m_host_cert_info[cm->m_webview->get_uri()] = UNSECURE_HOST_ASK;
		cm->m_certi_policy = certi_policy;
		const char *pem = ewk_certificate_policy_decision_certificate_pem_get(certi_policy);
		cm->set_certificate_data(pem);
		ewk_certificate_policy_decision_suspend(cm->m_certi_policy);
		if (cm->m_untrusted_cert_data_idler)
			ecore_idler_del(cm->m_untrusted_cert_data_idler);
		cm->m_untrusted_cert_data_idler =
			ecore_idler_add(inititialize_untrusted_certificate_data, cm);
	}
}

void certificate_manager::__load_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	RET_MSG_IF(!data, "data is NULL");
	BROWSER_LOGD("");
	certificate_manager *cm = (certificate_manager*)data;
	evas_object_smart_callback_del(cm->m_webview->get_ewk_view(), "load,finished", __load_finished_cb);
	cm->m_webview->backward();
}

Eina_Bool certificate_manager::_create_certificate(const char *certificate_data)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!certificate_data, EINA_FALSE ,"certificate data is NULL");

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

std::string certificate_manager::_get_certificate_status()
{
	BROWSER_LOGD("");
	/*
	 *Checking Whether is it a secure url or not.
	 */
	std::string msg;
	char buff[64] = {0,};
	int fontSize = ELM_SCALE_SIZE(STATUS_FONT_SIZE);
	snprintf(buff, 64, "%d", fontSize);
	if (m_cert_type == VALID)
		msg = std::string("<font_size=")+std::string(buff)+std::string("><color=#000000>") + std::string(BR_STRING_TRUSTED_AUTHORITY) + std::string("</color></font_size>");
	else
		msg = std::string("<font_size=")+std::string(buff)+std::string("><color=#000000>") + std::string(BR_STRING_UNTRUSTED_AUTHORITY) + std::string("</color></font_size>");
	return msg;
}

void certificate_manager::show_certificate_status_popup(Eina_Bool ask)
{
	BROWSER_SECURE_LOGD("");

	std::string msg = "";
	bool certificate_created = false;

	if (!_create_certificate(m_certificate_data.c_str())) {
		msg = BR_STRING_UNABLE_TO_VIEW_THE_CERTIFICATE_THE_PAGE_INFORMATION_HAS_BEEN_CHANGED;
		ewk_certificate_policy_decision_allowed_set(m_certi_policy, EINA_FALSE);
	} else {
		certificate_created = true;
		msg = _get_certificate_status();
	}

	if (m_certificate_popup) {
		evas_object_del(m_certificate_popup);
		m_certificate_popup = NULL;
	}

	Evas_Object *popup = brui_popup_add(m_window);
	RET_MSG_IF(!popup, "popup is null");
	elm_object_part_text_set(popup, "title,text","IDS_BR_HEADER_ADVANCEDCERTIFICATES");

	/* main layout */
	Evas_Object *layout = elm_layout_add(popup);
	RET_MSG_IF(!layout, "layout is null");
	elm_layout_file_set(layout, browser_edj_dir"/browser-popup-lite.edj", "certificate_popup_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *label = elm_label_add(layout);
	elm_label_line_wrap_set(label, ELM_WRAP_WORD);
	elm_object_domain_translatable_text_set(label, BROWSER_DOMAIN_NAME, msg.c_str());
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, 0.0);
	elm_object_part_content_set(layout, "elm.swallow.content", label);
	m_certificate_popup_layout = label;
	evas_object_show(label);

	/*View Certificat*/
	if (certificate_created) {
		set_trans_text_to_object(layout, "IDS_BR_OPT_VIEW_CERTIFICATE", "elm.text");
		edje_object_signal_callback_add(elm_layout_edje_get(layout),
				"mouse,clicked,1", "elm.text", __view_certificate_cb, this);
		edje_object_signal_callback_add(elm_layout_edje_get(layout),
				"delete_popup", "", __delete_certificate_popup_cb, this);
	}
	elm_object_content_set(popup, layout);

	std::string header = BR_STRING_SECURITY_WARNING_HEADER;
	if (m_cert_type == VALID)
		header = BR_STRING_SECURITY_CERTIFICATE;

	if (ask)
		m_certificate_popup = show_content_popup(popup, header.c_str(), layout,
				__certi_deny_cb,
				BR_STRING_CANCEL, __certi_deny_cb,
				BR_STRING_CONTINUE, __certi_allow_cb,
				this,
				EINA_FALSE);
	else
		m_certificate_popup = show_content_popup(popup, header.c_str(), layout,
				__popup_destroy_cb,
				"IDS_BR_SK_OK", __ok_button_cb,
				NULL, NULL,
				this,
				EINA_FALSE);

	evas_object_smart_callback_add(m_certificate_popup, "language,changed", __certificate_popup_lang_changed_cb, this);
#if defined(HW_MORE_BACK_KEY)
	eext_object_event_callback_add(m_certificate_popup, EEXT_CALLBACK_BACK, __cancel_button_cb, this);
#endif
	evas_object_show(m_certificate_popup);
}

void certificate_manager::__delete_certificate_popup_cb(void *data, Evas_Object *obj, const char *emission, const char *source) {
	BROWSER_LOGD("");
	certificate_manager *cm = (certificate_manager*)data;
	cm->clear_popups();
	cm->m_certificate_popup_layout = NULL;
	cm->m_certificate_popup =  NULL;
}
void certificate_manager::__popup_destroy_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	certificate_manager *cm = (certificate_manager *)data;
	if (cm->m_certificate_popup)
		evas_object_smart_callback_del(cm->m_certificate_popup, "language,changed", cm->__certificate_popup_lang_changed_cb);
	cm->m_certificate_popup_layout = NULL;
	cm->m_certificate_popup = NULL;
}

void certificate_manager::__certificate_popup_lang_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	certificate_manager *cm = (certificate_manager *)data;
	std::string msg = cm->_get_certificate_status();
	if (cm->m_certificate_popup_layout)
		elm_object_text_set(cm->m_certificate_popup_layout, msg.c_str());
}

void certificate_manager::destroy_certificate_status_popup()
{
	BROWSER_LOGD("");

	//May be the case the current webview is replaced by new webview, so close the certificate popup
	if (m_certificate_popup) {
		evas_object_smart_callback_del(m_certificate_popup, "language,changed", __certificate_popup_lang_changed_cb);
		m_certificate_popup_layout = NULL;
		common_view::clear_popups();
		m_certificate_popup = NULL;
	}

}

Eina_Bool certificate_manager::inititialize_untrusted_certificate_data(void *data)
{
	RETV_MSG_IF(!data, ECORE_CALLBACK_CANCEL ,"data is NULL");
	certificate_manager *cm = (certificate_manager*)data;
	cm->m_untrusted_cert_data_idler = NULL;
	cm->show_certificate_status_popup();
	return ECORE_CALLBACK_CANCEL;
}

void certificate_manager::__view_certificate_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	certificate_manager *cm = (certificate_manager*)data;
	certificate_view *certificate_view = m_browser->get_certificate_view(cm->m_certificate, NULL, NULL);

	if (cm->m_certificate_popup) {
		Evas_Object *popup_layout = elm_object_content_get(cm->m_certificate_popup);
		edje_object_signal_emit(elm_layout_edje_get(popup_layout), "play,view_certificate_sound,signal", "");
		evas_object_smart_callback_del(cm->m_certificate_popup, "language,changed", __certificate_popup_lang_changed_cb);
	}
	certificate_view->show_certificate_info_popup();
}

void certificate_manager::__cancel_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	certificate_manager *cm = (certificate_manager*)data;
	if (cm ->m_certificate_popup) {
		cm->m_certificate_popup = NULL;
	}
}

void certificate_manager::__ok_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	certificate_manager *cm = (certificate_manager*)data;
	if (cm ->m_certificate_popup) {
		evas_object_smart_callback_del(cm->m_certificate_popup, "language,changed", __certificate_popup_lang_changed_cb);
		cm->m_certificate_popup_layout = NULL;
		cm->m_certificate_popup = NULL;
	}
}
