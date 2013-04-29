/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
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

#include "custom-content-handler.h"

extern "C" {
#include "db-util.h"
}

#include <string>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "webview.h"

#define custom_scheme_handler_db_path "/opt/usr/apps/org.tizen.browser/data/db/.html5-custom-handler.db"

typedef struct _content_handler_data {
	char *base_uri;
	char *target;
	char *uri;
	void *data;
} content_handler_data;

static Eina_Bool _is_registered_content_handler(const char *base_uri, const char *mime, int *allow)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(base_uri, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(mime, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(allow, EINA_FALSE);

	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_scheme_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(db_descriptor, "select allow from custom_content_handler where base_uri=? and mime=?",								-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		return EINA_FALSE;
	}
	error = sqlite3_bind_text(sqlite3_stmt, 1, base_uri, -1, NULL);
	if (error != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
	}

	error = sqlite3_bind_text(sqlite3_stmt, 2, mime, -1, NULL);
	if (error != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
	}

	error = sqlite3_step(sqlite3_stmt);
	if (error == SQLITE_ROW || error == SQLITE_DONE)
		*allow = sqlite3_column_int(sqlite3_stmt, 0);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(db_descriptor);
		return EINA_FALSE;
	}

	db_util_close(db_descriptor);

	if (error == SQLITE_ROW)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

static Eina_Bool _register_content_handler(const char *base_uri, const char *mime, const char *uri, Eina_Bool allow)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(base_uri, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(mime, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	sqlite3_stmt *sqlite3_stmt = NULL;
	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_scheme_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return EINA_FALSE;
	}
	int is_allowed = 0;
	if (_is_registered_content_handler(base_uri, mime, &is_allowed)) {
		BROWSER_LOGD("update");
		error = sqlite3_prepare_v2(db_descriptor, "update custom_content_handler set base_uri=?, uri=?, allow=? where mime=?", -1, &sqlite3_stmt, NULL);
		if (error != SQLITE_OK) {
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 1, base_uri, -1, NULL) != SQLITE_OK) {
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}

		if (sqlite3_bind_text(sqlite3_stmt, 2, uri, -1, NULL) != SQLITE_OK) {
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_int(sqlite3_stmt, 3, (int)allow) != SQLITE_OK){
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 4, mime, -1, NULL) != SQLITE_OK){
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
	} else {
		BROWSER_LOGD("insert");
		error = sqlite3_prepare_v2(db_descriptor, "insert into custom_content_handler (base_uri, mime, uri, allow) values(?,?,?,?)", -1, &sqlite3_stmt, NULL);
		if (error != SQLITE_OK){
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 1, base_uri, -1, NULL) != SQLITE_OK){
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 2, mime, -1, NULL) != SQLITE_OK){
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 3, uri, -1, NULL) != SQLITE_OK){
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_int(sqlite3_stmt, 4, (int)allow) != SQLITE_OK){
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}

		error = sqlite3_step(sqlite3_stmt);
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(db_descriptor);
		return EINA_FALSE;
	}

	db_util_close(db_descriptor);

	return (error == SQLITE_DONE);

}

static Eina_Bool _unregister_content_handler(const char *base_uri, const char *mime, const char *uri)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(base_uri, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(mime, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_scheme_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(db_descriptor, "delete from custom_content_handler where base_uri=? and mime=? and uri=?", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		return EINA_FALSE;
	}
	error = sqlite3_bind_text(sqlite3_stmt, 1, base_uri, -1, NULL);
	if (error != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
	}
	error = sqlite3_bind_text(sqlite3_stmt, 2, mime, -1, NULL);
	if (error != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
	}
	error = sqlite3_bind_text(sqlite3_stmt, 3, uri, -1, NULL);
	if (error != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
	}

	error = sqlite3_step(sqlite3_stmt);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(db_descriptor);
		return EINA_FALSE;
	}

	db_util_close(db_descriptor);

	return EINA_TRUE;

}

static char *_get_target_uri(const char *base_uri, const char *mime)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(base_uri, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(mime, NULL);

	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_scheme_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return NULL;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(db_descriptor, "select uri from custom_content_handler where base_uri=? and mime=?", 							-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		return NULL;
	}

	error = sqlite3_bind_text(sqlite3_stmt, 1, base_uri, -1, NULL);
	if (error != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			db_util_close(db_descriptor);
			return NULL;
		}
	}
	error = sqlite3_bind_text(sqlite3_stmt, 2, mime, -1, NULL);
	if (error != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			db_util_close(db_descriptor);
			return NULL;
		}
	}

	error = sqlite3_step(sqlite3_stmt);

	const char *uri = (const char *)sqlite3_column_text(sqlite3_stmt, 0);
	BROWSER_LOGD("content uri= [%s]", uri);
	char *return_uri = strdup(uri);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(db_descriptor);
		if (return_uri)
			free(return_uri);

		return NULL;
	}

	db_util_close(db_descriptor);

	return return_uri;
}

custom_content_handler::custom_content_handler(webview *wv)
:
	m_webview(wv)
	,m_redirect_uri(NULL)
{
	BROWSER_LOGD("");
	m_webview->attach_event("contenthandler,registration,requested", __registration_requested_cb, this);
	m_webview->attach_event("contenthandler,isregistered", __is_registered_cb, this);
	m_webview->attach_event("contenthandler,unregistration,requested", __unregistration_requested_cb, this);
}

custom_content_handler::~custom_content_handler(void)
{
	BROWSER_LOGD("");
	m_webview->detach_event("contenthandler,registration,requested", __registration_requested_cb);
	m_webview->detach_event("contenthandler,isregistered", __is_registered_cb);
	m_webview->detach_event("contenthandler,unregistration,requested", __unregistration_requested_cb);

	eina_stringshare_del(m_redirect_uri);
}

const char *custom_content_handler::get_redirect_uri(const char *origin_uri, const char *base_uri, const char *mime)
{
	BROWSER_LOGD("origin_uri=[%s], base_uri=[%s], mime=[%s]", origin_uri, base_uri, mime);
	EINA_SAFETY_ON_NULL_RETURN_VAL(origin_uri, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(base_uri, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(mime, NULL);

	int allow = 0;
	if (_is_registered_content_handler(base_uri, mime, &allow)) {
		if (!allow)
			return NULL;

		char *content_uri = _get_target_uri(base_uri, mime);
		if (content_uri && strlen(content_uri)) {
			std::string query_uri_str = std::string(content_uri);

			free(content_uri);
			content_uri = NULL;

			if (query_uri_str.find("%s") != std::string::npos) {
				int pos = query_uri_str.find("%s");
				query_uri_str.replace(pos, strlen("%s"), origin_uri);
				eina_stringshare_replace(&m_redirect_uri, query_uri_str.c_str());

				return m_redirect_uri;
			}
		}

		if (content_uri)
			free(content_uri);
	}

	return NULL;
}

void custom_content_handler::__register_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	content_handler_data *handler_data = (content_handler_data *)data;

	_register_content_handler(handler_data->base_uri, handler_data->target, handler_data->uri, EINA_TRUE);

	if (handler_data->base_uri)
		free(handler_data->base_uri);
	if (handler_data->target)
		free(handler_data->target);
	if (handler_data->uri)
		free(handler_data->uri);
	free(handler_data);
}

void custom_content_handler::__register_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	content_handler_data *handler_data = (content_handler_data *)data;

	_register_content_handler(handler_data->base_uri, handler_data->target, handler_data->uri, EINA_FALSE);

	if (handler_data->base_uri)
		free(handler_data->base_uri);
	if (handler_data->target)
		free(handler_data->target);
	if (handler_data->uri)
		free(handler_data->uri);

	free(handler_data);
}

void custom_content_handler::__registration_requested_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Ewk_Custom_Handlers_Data *custom_handler_data = (Ewk_Custom_Handlers_Data *)event_info;
	const char *target = ewk_custom_handlers_data_target_get(custom_handler_data);
	const char *uri = ewk_custom_handlers_data_url_get(custom_handler_data);
	const char *base_uri = ewk_custom_handlers_data_base_url_get(custom_handler_data);
	const char *title = ewk_custom_handlers_data_title_get(custom_handler_data);

	// FIXME: hard coded string.
	std::string msg;
	msg = std::string(base_uri) + std::string(" is asking to register ") + std::string(target) + std::string(" content handler.");

	content_handler_data *handler_data = (content_handler_data *)malloc(sizeof(content_handler_data));
	if (!handler_data) {
		BROWSER_LOGE("malloc failed");
		return;
	}
	handler_data->base_uri = strdup(base_uri);
	handler_data->target = strdup(target);
	handler_data->uri = strdup(uri);
	handler_data->data = (void *)data;

	m_browser->get_browser_view()->show_msg_popup(NULL, msg.c_str(), BR_STRING_OK, __register_ok_cb, BR_STRING_CANCEL, __register_cancel_cb, handler_data);
}

void custom_content_handler::__is_registered_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Ewk_Custom_Handlers_Data *custom_handler_data = (Ewk_Custom_Handlers_Data *)event_info;
	const char *target = ewk_custom_handlers_data_target_get(custom_handler_data);
	const char *uri = ewk_custom_handlers_data_url_get(custom_handler_data);
	const char *base_uri = ewk_custom_handlers_data_base_url_get(custom_handler_data);
	const char *title = ewk_custom_handlers_data_title_get(custom_handler_data);

	int is_allowed = 0;
	if (_is_registered_content_handler(base_uri, target, &is_allowed)) {
		if (is_allowed)
			ewk_custom_handlers_data_result_set(custom_handler_data, EWK_CUSTOM_HANDLERS_REGISTERED);
		else
			ewk_custom_handlers_data_result_set(custom_handler_data, EWK_CUSTOM_HANDLERS_DECLINED);
	} else {
		ewk_custom_handlers_data_result_set(custom_handler_data, EWK_CUSTOM_HANDLERS_NEW);
	}
}

void custom_content_handler::__unregistration_requested_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Ewk_Custom_Handlers_Data *custom_handler_data = (Ewk_Custom_Handlers_Data *)event_info;
	const char *target = ewk_custom_handlers_data_target_get(custom_handler_data);
	const char *uri = ewk_custom_handlers_data_url_get(custom_handler_data);
	const char *base_uri = ewk_custom_handlers_data_base_url_get(custom_handler_data);
	const char *title = ewk_custom_handlers_data_title_get(custom_handler_data);

	_unregister_content_handler(base_uri, target, uri);
}

