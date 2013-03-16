/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "certificate-manager.h"

#include <Elementary.h>
#include <string>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "webview.h"

extern "C" {
#include "db-util.h"
}

#define certificate_db_path	"/opt/usr/apps/org.tizen.browser/data/db/.certificate.db"

static Eina_Bool _is_exist(const char *pem, int *allow)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(pem, EINA_FALSE);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(certificate_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select allow from certificate where pem=?", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGE("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	error = sqlite3_bind_text(sqlite3_stmt, 1, pem, -1, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (error == SQLITE_ROW) {
		if (allow) {
			*allow = sqlite3_column_int(sqlite3_stmt, 0);
			BROWSER_LOGD("allow=[%d]", *allow);
		}

		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize failed");

		db_util_close(descriptor);

		return EINA_TRUE;
	} else {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize failed");
		db_util_close(descriptor);
	}

	return EINA_FALSE;
}

static Eina_Bool _save_certificate_info(const char *pem, int allow)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(pem, EINA_FALSE);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(certificate_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "insert into certificate (pem, allow) values(?, ?)", -1, &sqlite3_stmt, NULL);
	if (sqlite3_bind_text(sqlite3_stmt, 1, pem, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 2, allow) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	db_util_close(descriptor);

	return EINA_TRUE;
}

static Eina_Bool _delete_all(void)
{
	sqlite3 *descriptor = NULL;
	int error = db_util_open(certificate_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "delete from certificate", -1, &sqlite3_stmt, NULL);
	sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_finalize failed");
		return EINA_FALSE;
	}

	db_util_close(descriptor);

	return EINA_TRUE;
}

int certificate_manager::m_instance_count;

certificate_manager::certificate_manager(webview *wv)
:
	m_webview(wv)
{
	BROWSER_LOGD("");
	m_webview->attach_event("request,certificate,confirm", __request_certi_cb, this);
	m_instance_count++;
}

certificate_manager::~certificate_manager(void)
{
	BROWSER_LOGD("");

	m_webview->detach_event("request,certificate,confirm", __request_certi_cb);

	m_instance_count--;
	if (m_instance_count == 0)
		_delete_all();
}

void certificate_manager::__certi_allow_cb(void *data, Evas_Object *obj, void *event_info)
{
	Ewk_Certificate_Policy_Decision *certi_policy = (Ewk_Certificate_Policy_Decision *)data;
	ewk_certificate_policy_decision_allowed_set(certi_policy, EINA_TRUE);
	const char *pem = ewk_certificate_policy_decision_certificate_pem_get(certi_policy);
	_save_certificate_info(pem, 1);
}

void certificate_manager::__certi_deny_cb(void *data, Evas_Object *obj, void *event_info)
{
	Ewk_Certificate_Policy_Decision *certi_policy = (Ewk_Certificate_Policy_Decision *)data;
	ewk_certificate_policy_decision_allowed_set(certi_policy, EINA_FALSE);
	const char *pem = ewk_certificate_policy_decision_certificate_pem_get(certi_policy);
	_save_certificate_info(pem, 0);
}

void certificate_manager::__request_certi_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	certificate_manager *cm = (certificate_manager *)data;
	Ewk_Certificate_Policy_Decision *certi_policy = (Ewk_Certificate_Policy_Decision *)event_info;

	const char *pem = ewk_certificate_policy_decision_certificate_pem_get(certi_policy);
	BROWSER_LOGD("pem=[%s]", pem);

	ewk_certificate_policy_decision_suspend(certi_policy);

	int allow = 0;
	if (_is_exist(pem, &allow)) {
		BROWSER_LOGD("allow = %d", allow);
		if (allow)
			ewk_certificate_policy_decision_allowed_set(certi_policy, EINA_TRUE);
		else
			ewk_certificate_policy_decision_allowed_set(certi_policy, EINA_FALSE);
	} else {
		char *markup_uri = elm_entry_utf8_to_markup(cm->m_webview->get_uri());
		if (!markup_uri)
			return;
		// FIXME: hard coded string.
		std::string msg = "There are problems with the secuirty certificate for this site.<br>" + std::string(markup_uri);
		m_browser->get_browser_view()->show_msg_popup("Security warning", msg.c_str(), BR_STRING_ALLOW, __certi_allow_cb, BR_STRING_CANCEL, __certi_deny_cb, certi_policy);
		free(markup_uri);
	}
}

