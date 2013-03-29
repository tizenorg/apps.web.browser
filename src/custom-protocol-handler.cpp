/*
 *  browser
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
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

#include "custom-protocol-handler.h"

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

typedef struct _protocol_handler_data {
	char *base_uri;
	char *target;
	char *uri;
	void *data;
} protocol_handler_data;

static Eina_Bool _is_registered_protocol_handler(const char *base_uri, const char *protocol, int *allow)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(base_uri, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(protocol, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(allow, EINA_FALSE);

	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_scheme_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(db_descriptor, "select allow from custom_protocol_handler where base_uri=? and protocol=?",								-1, &sqlite3_stmt, NULL);
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

	error = sqlite3_bind_text(sqlite3_stmt, 2, protocol, -1, NULL);
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

static Eina_Bool _register_protocol_handler(const char *base_uri, const char *protocol, const char *uri, Eina_Bool allow)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(base_uri, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(protocol, EINA_FALSE);
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
	if (_is_registered_protocol_handler(base_uri, protocol, &is_allowed)) {
		BROWSER_LOGD("update");
		error = sqlite3_prepare_v2(db_descriptor, "update custom_protocol_handler set base_uri=?, uri=?, allow=? where protocol=?", -1, &sqlite3_stmt, NULL);
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
		if (sqlite3_bind_text(sqlite3_stmt, 4, protocol, -1, NULL) != SQLITE_OK){
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
	} else {
		BROWSER_LOGD("insert");
		error = sqlite3_prepare_v2(db_descriptor, "insert into custom_protocol_handler (base_uri, protocol, uri, allow) values(?,?,?,?)", -1, &sqlite3_stmt, NULL);
		if (error != SQLITE_OK){
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 1, base_uri, -1, NULL) != SQLITE_OK){
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 2, protocol, -1, NULL) != SQLITE_OK){
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

static Eina_Bool _unregister_protocol_handler(const char *base_uri, const char *protocol, const char *uri)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(base_uri, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(protocol, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_scheme_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(db_descriptor, "delete from custom_protocol_handler where base_uri=? and protocol=? and uri=?", -1, &sqlite3_stmt, NULL);
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
	error = sqlite3_bind_text(sqlite3_stmt, 2, protocol, -1, NULL);
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

static char *_get_target_uri(const char *base_uri, const char *protocol)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(base_uri, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(protocol, NULL);

	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_scheme_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return NULL;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(db_descriptor, "select uri from custom_protocol_handler where base_uri=? and protocol=?", 							-1, &sqlite3_stmt, NULL);
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
	error = sqlite3_bind_text(sqlite3_stmt, 2, protocol, -1, NULL);
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

custom_protocol_handler::custom_protocol_handler(webview *wv)
:
	m_webview(wv)
	,m_protocol_converted_uri(NULL)
{
	BROWSER_LOGD("");
	m_webview->attach_event("protocolhandler,registration,requested", __registration_requested_cb, this);
	m_webview->attach_event("protocolhandler,isregistered", __is_registered_cb, this);
	m_webview->attach_event("protocolhandler,unregistration,requested", __unregistration_requested_cb, this);
}

custom_protocol_handler::~custom_protocol_handler(void)
{
	BROWSER_LOGD("");
	m_webview->detach_event("protocolhandler,registration,requested", __registration_requested_cb);
	m_webview->detach_event("protocolhandler,isregistered", __is_registered_cb);
	m_webview->detach_event("protocolhandler,unregistration,requested", __unregistration_requested_cb);

	eina_stringshare_del(m_protocol_converted_uri);
}

// FIXME : base uri may be get from webkit.
static char *_get_base_uri(const char *uri)
{
	if (!uri || !strlen(uri))
		return NULL;

	std::string uri_str = std::string(uri);
	int found = uri_str.rfind("/");
	if (found != std::string::npos) {
		std::string sub_str = uri_str.substr(0, found + 1);
		return strdup(sub_str.c_str());
	}

	return NULL;
}

const char *custom_protocol_handler::get_protocol_from_uri(const char *origin_uri)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(origin_uri, NULL);
	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_scheme_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return NULL;
	}

	const char *current_uri = m_browser->get_browser_view()->get_current_webview()->get_uri();
	char *base_uri = _get_base_uri(current_uri);
	BROWSER_LOGD("current_uri=[%s], base_uri=[%s]", current_uri, base_uri);
	if (!base_uri)
		return NULL;

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(db_descriptor, "select protocol, uri, allow from custom_protocol_handler where base_uri=?", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			db_util_close(db_descriptor);
			free(base_uri);
			return NULL;
		}
	}
	error = sqlite3_bind_text(sqlite3_stmt, 1, base_uri, -1, NULL);
	if (error != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			db_util_close(db_descriptor);
			free(base_uri);
			return NULL;
		}
	}

	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		const char *protocol = (const char *)sqlite3_column_text(sqlite3_stmt, 0);
		const char *query_uri = (const char *)sqlite3_column_text(sqlite3_stmt, 1);
		int allow = sqlite3_column_int(sqlite3_stmt, 2);

		BROWSER_LOGD("protocol=[%s], allow=%d", protocol, allow);
		if (!allow)
			break;

		std::string protocol_scheme = std::string(protocol) + std::string(":");
		BROWSER_LOGD("protocol_scheme=[%s]", protocol_scheme.c_str());
		if ((strlen(origin_uri) >= strlen(protocol_scheme.c_str())) && !strncmp(origin_uri, protocol_scheme.c_str(), strlen(protocol_scheme.c_str()))) {
			std::string query_uri_str = std::string(query_uri);
			if (query_uri_str.find("%s") != std::string::npos) {
				int pos = query_uri_str.find("%s");
				query_uri_str.replace(pos, strlen("%s"), origin_uri + strlen(protocol_scheme.c_str()));
				eina_stringshare_replace(&m_protocol_converted_uri, query_uri_str.c_str());

				if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
					db_util_close(db_descriptor);
					free(base_uri);
					return NULL;
				}

				return m_protocol_converted_uri;
			}
		}
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(db_descriptor);
		free(base_uri);
		return NULL;
	}

	db_util_close(db_descriptor);

	free(base_uri);

	return NULL;
}

void custom_protocol_handler::__register_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	protocol_handler_data *handler_data = (protocol_handler_data *)data;

	_register_protocol_handler(handler_data->base_uri, handler_data->target, handler_data->uri, EINA_TRUE);

	if (handler_data->base_uri)
		free(handler_data->base_uri);
	if (handler_data->target)
		free(handler_data->target);
	if (handler_data->uri)
		free(handler_data->uri);
	free(handler_data);
}

void custom_protocol_handler::__register_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	protocol_handler_data *handler_data = (protocol_handler_data *)data;

	_register_protocol_handler(handler_data->base_uri, handler_data->target, handler_data->uri, EINA_FALSE);

	if (handler_data->base_uri)
		free(handler_data->base_uri);
	if (handler_data->target)
		free(handler_data->target);
	if (handler_data->uri)
		free(handler_data->uri);
	free(handler_data);
}

void custom_protocol_handler::__registration_requested_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Ewk_Custom_Handlers_Data *custom_handler_data = (Ewk_Custom_Handlers_Data *)event_info;
	const char *target = ewk_custom_handlers_data_target_get(custom_handler_data);
	const char *uri = ewk_custom_handlers_data_url_get(custom_handler_data);
	const char *base_uri = ewk_custom_handlers_data_base_url_get(custom_handler_data);

	// FIXME: hard coded string.
	std::string msg;
	msg = std::string(base_uri) + std::string(" is asking to register ") + std::string(target) + std::string(" protocol handler.");

	protocol_handler_data *handler_data = (protocol_handler_data *)malloc(sizeof(protocol_handler_data));
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

void custom_protocol_handler::__is_registered_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Ewk_Custom_Handlers_Data *custom_handler_data = (Ewk_Custom_Handlers_Data *)event_info;
	const char *target = ewk_custom_handlers_data_target_get(custom_handler_data);
	const char *base_uri = ewk_custom_handlers_data_base_url_get(custom_handler_data);

	int is_allowed = 0;
	if (_is_registered_protocol_handler(base_uri, target, &is_allowed)) {
		if (is_allowed)
			ewk_custom_handlers_data_result_set(custom_handler_data, EWK_CUSTOM_HANDLERS_REGISTERED);
		else
			ewk_custom_handlers_data_result_set(custom_handler_data, EWK_CUSTOM_HANDLERS_DECLINED);
	} else {
		ewk_custom_handlers_data_result_set(custom_handler_data, EWK_CUSTOM_HANDLERS_NEW);
	}
}

void custom_protocol_handler::__unregistration_requested_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Ewk_Custom_Handlers_Data *custom_handler_data = (Ewk_Custom_Handlers_Data *)event_info;
	const char *target = ewk_custom_handlers_data_target_get(custom_handler_data);
	const char *uri = ewk_custom_handlers_data_url_get(custom_handler_data);
	const char *base_uri = ewk_custom_handlers_data_base_url_get(custom_handler_data);

	_unregister_protocol_handler(base_uri, target, uri);
}

