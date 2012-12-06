/*
 *  browser
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
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

#include "browser-common-view.h"
#include "custom-protocol-handler.h"

typedef struct _custom_protocol {
	char *target;
	char *uri;
	void *data;
} custom_protocol;

#define custom_protocol_handler_db_path BROWSER_DATA_DIR"/data/db/.custom-protocol-handler.db"

static Eina_Bool is_registered_custom_protocol(const char *protocol, int *allow)
{
	BROWSER_LOGD("[%s]", __func__);
	EINA_SAFETY_ON_NULL_RETURN_VAL(protocol, EINA_FALSE);

	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_protocol_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(db_descriptor, "select allow from custom_protocol where protocol=?",								-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		return EINA_FALSE;
	}

	error = sqlite3_bind_text(sqlite3_stmt, 1, protocol, -1, NULL);
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

	if (error == SQLITE_ROW)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

Custom_Protocol_Handler::Custom_Protocol_Handler(void)
:
	m_protocol_uri(NULL)
	,m_protocol_converted_uri(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Custom_Protocol_Handler::~Custom_Protocol_Handler(void)
{
	BROWSER_LOGD("[%s]", __func__);
	eina_stringshare_del(m_protocol_uri);
	eina_stringshare_del(m_protocol_converted_uri);
}

void Custom_Protocol_Handler::activate(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);
	EINA_SAFETY_ON_NULL_RETURN(ewk_view);
	evas_object_smart_callback_add(ewk_view, "protocolhandler,registration,requested", __registration_requested_cb, this);
	evas_object_smart_callback_add(ewk_view, "protocolhandler,isregistered", __isregistered_cb, this);
	evas_object_smart_callback_add(ewk_view, "protocolhandler,unregistration,requested", __unregistration_requested_cb, this);
}

void Custom_Protocol_Handler::deactivate(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);
	EINA_SAFETY_ON_NULL_RETURN(ewk_view);
	evas_object_smart_callback_del(ewk_view, "protocolhandler,registration,requested", __registration_requested_cb);
	evas_object_smart_callback_del(ewk_view, "protocolhandler,isregistered", __isregistered_cb);
	evas_object_smart_callback_del(ewk_view, "protocolhandler,unregistration,requested", __unregistration_requested_cb);
}

void Custom_Protocol_Handler::__unregistration_requested_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	Ewk_Custom_Handlers_Data *handler_data = (Ewk_Custom_Handlers_Data *)event_info;
	const char *target = ewk_custom_handlers_data_target_get(handler_data);
	const char *uri = ewk_custom_handlers_data_url_get(handler_data);
	const char *base_uri = ewk_custom_handlers_data_base_url_get(handler_data);
	const char *title = ewk_custom_handlers_data_title_get(handler_data);

	BROWSER_LOGD("target=[%s], uri=[%s], base_uri=[%s], title=[%s]", target, uri, base_uri, title);

	Custom_Protocol_Handler *protocol_handler = (Custom_Protocol_Handler *)data;
	if (!protocol_handler->unregister_protocol(target, uri))
		BROWSER_LOGE("unregister_protocol failed");
}

void Custom_Protocol_Handler::__isregistered_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	Ewk_Custom_Handlers_Data *handler_data = (Ewk_Custom_Handlers_Data *)event_info;
	const char *target = ewk_custom_handlers_data_target_get(handler_data);
	const char *uri = ewk_custom_handlers_data_url_get(handler_data);
	const char *base_uri = ewk_custom_handlers_data_base_url_get(handler_data);
	const char *title = ewk_custom_handlers_data_title_get(handler_data);

	BROWSER_LOGD("target=[%s], uri=[%s], base_uri=[%s], title=[%s]", target, uri, base_uri, title);

	Custom_Protocol_Handler *protocol_handler = (Custom_Protocol_Handler *)data;
	int is_allowed = 0;
	if (is_registered_custom_protocol(target, &is_allowed)) {
		if (is_allowed)
			ewk_custom_handlers_data_result_set(handler_data, EWK_CUSTOM_HANDLERS_REGISTERED);
		else
			ewk_custom_handlers_data_result_set(handler_data, EWK_CUSTOM_HANDLERS_DECLINED);
	} else
		ewk_custom_handlers_data_result_set(handler_data, EWK_CUSTOM_HANDLERS_NEW);
}

const char *Custom_Protocol_Handler::get_registered_converted_protocol(const char *uri)
{
	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_protocol_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return NULL;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(db_descriptor, "select protocol, uri, allow from custom_protocol",
					-1,&sqlite3_stmt,NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			db_util_close(db_descriptor);
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
		if ((strlen(uri) >= strlen(protocol_scheme.c_str())) && !strncmp(uri, protocol_scheme.c_str(), strlen(protocol_scheme.c_str()))) {
			std::string query_uri_str = std::string(query_uri);
			if (query_uri_str.find("%s") != string::npos) {
				int pos = query_uri_str.find("%s");
				query_uri_str.replace(pos, strlen("%s"), uri + strlen(protocol_scheme.c_str()));
				eina_stringshare_replace(&m_protocol_converted_uri, query_uri_str.c_str());

				if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
					db_util_close(db_descriptor);
					return NULL;
				}

				return m_protocol_converted_uri;
			}
		}
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(db_descriptor);
		return NULL;
	}

	return NULL;
}

Eina_Bool Custom_Protocol_Handler::unregister_protocol(const char *protocol, const char *uri)
{
	BROWSER_LOGD("protocol = [%s], uri=[%s]", protocol, uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(protocol, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_protocol_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(db_descriptor, "delete from custom_protocol where protocol=? and uri=?", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		return EINA_FALSE;
	}

	error = sqlite3_bind_text(sqlite3_stmt, 1, protocol, -1, NULL);
	if (error != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
	}
	error = sqlite3_bind_text(sqlite3_stmt, 2, uri, -1, NULL);
	if (error != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
	}

	error = sqlite3_step(sqlite3_stmt);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(db_descriptor);
		BROWSER_LOGD("*** sqlite3_finalize failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

const char *Custom_Protocol_Handler::get_protocol_uri(const char *protocol)
{
	BROWSER_LOGD("protocol = [%s]", protocol);
	EINA_SAFETY_ON_NULL_RETURN_VAL(protocol, NULL);

	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_protocol_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return NULL;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(db_descriptor, "select uri from custom_protocol where protocol=?", 							-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		return NULL;
	}

	error = sqlite3_bind_text(sqlite3_stmt, 1, protocol, -1, NULL);
	if (error != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			db_util_close(db_descriptor);
			return NULL;
		}
	}

	error = sqlite3_step(sqlite3_stmt);

	const char *uri = (const char *)sqlite3_column_text(sqlite3_stmt, 0);
	BROWSER_LOGD("protocol uri= [%s]", uri);

	eina_stringshare_replace(&m_protocol_uri, uri);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(db_descriptor);
		return NULL;
	}

	return m_protocol_uri;
}

static void _destroy_popup(Evas_Object *sub_obj)
{
	Evas_Object *parent = elm_object_parent_widget_get(sub_obj);
	Evas_Object *popup = elm_popup_add(parent);
	while (parent) {
		const char *type = elm_object_widget_type_get(parent);
		if (type && !strcmp(type, elm_object_widget_type_get(popup))) {
			evas_object_del(parent);
			break;
		}
		parent = elm_object_parent_widget_get(parent);
	}

	evas_object_del(popup);
}

static void _destroy_custom_protocol(custom_protocol* cp)
{
	if (cp->target)
		free(cp->target);
	if (cp->uri)
		free(cp->uri);
}

void Custom_Protocol_Handler::__register_protocol_ok_cb(void* data, Evas_Object* obj, void* event_info)
{
	custom_protocol *cp = (custom_protocol *)data;
	Custom_Protocol_Handler *protocol_handler = (Custom_Protocol_Handler *)(cp->data);
	if (!protocol_handler->_save_protocol_handler(cp->target, cp->uri, EINA_TRUE))
		BROWSER_LOGE("_save_protocol_handler failed");

	_destroy_popup(obj);
	_destroy_custom_protocol(cp);
}

void Custom_Protocol_Handler::__register_protocol_cancel_cb(void* data, Evas_Object* obj, void* event_info)
{
	custom_protocol *cp = (custom_protocol *)data;
	Custom_Protocol_Handler *protocol_handler = (Custom_Protocol_Handler *)(cp->data);
	if (!protocol_handler->_save_protocol_handler(cp->target, cp->uri, EINA_FALSE))
		BROWSER_LOGE("_save_protocol_handler failed");

	_destroy_popup(obj);
	_destroy_custom_protocol(cp);
}

void Custom_Protocol_Handler::_show_register_protocol_confirm_popup(const char *message, void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	EINA_SAFETY_ON_NULL_RETURN(message);

	Evas_Object *popup = elm_popup_add(m_win);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup, message);
	evas_object_show(popup);
	BROWSER_LOGD("************** popup = %d", popup);

	Evas_Object *ok_button = elm_button_add(popup);
	elm_object_text_set(ok_button, BR_STRING_OK);
	elm_object_part_content_set(popup, "button1", ok_button);
	elm_object_style_set(ok_button, "popup_button/default");
	evas_object_smart_callback_add(ok_button, "clicked", __register_protocol_ok_cb,  data);

	Evas_Object *cancel_button = elm_button_add(popup);
	elm_object_text_set(cancel_button, BR_STRING_CANCEL);
	elm_object_part_content_set(popup, "button2", cancel_button);
	elm_object_style_set(cancel_button, "popup_button/default");
	evas_object_smart_callback_add(cancel_button, "clicked", __register_protocol_cancel_cb, data);
}

void Custom_Protocol_Handler::__registration_requested_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	Ewk_Custom_Handlers_Data *handler_data = (Ewk_Custom_Handlers_Data *)event_info;
	const char *target = ewk_custom_handlers_data_target_get(handler_data);
	const char *uri = ewk_custom_handlers_data_url_get(handler_data);
	const char *base_uri = ewk_custom_handlers_data_base_url_get(handler_data);
	const char *title = ewk_custom_handlers_data_title_get(handler_data);

	BROWSER_LOGD("target=[%s], uri=[%s], base_uri=[%s], title=[%s]", target, uri, base_uri, title);

	std::string msg;
	msg = std::string(base_uri) + std::string(" is asking to register ") + std::string(target) + std::string(" protocol handler.");

	Custom_Protocol_Handler *protocol_handler = (Custom_Protocol_Handler *)data;

	custom_protocol *cp = (custom_protocol *)malloc(sizeof(custom_protocol));
	memset(cp, 0x00, sizeof(custom_protocol));
	cp->target = strdup(target);
	cp->uri = strdup(uri);
	cp->data = (void *)protocol_handler;
	protocol_handler->_show_register_protocol_confirm_popup(msg.c_str(), cp);
#if 0
	if (!protocol_handler->_save_protocol_handler(target, uri))
		BROWSER_LOGE("_save_protocol_handler failed");
#endif
}

Eina_Bool Custom_Protocol_Handler::_save_protocol_handler(const char *protocol, const char *uri, Eina_Bool allow)
{
	BROWSER_LOGD("protocol=[%s], uri=[%s]", protocol, uri);

	EINA_SAFETY_ON_NULL_RETURN_VAL(protocol, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	sqlite3_stmt *sqlite3_stmt = NULL;
	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_protocol_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return EINA_FALSE;
	}

	int is_allowed = 0;
	if (is_registered_custom_protocol(protocol, &is_allowed)) {
		error = sqlite3_prepare_v2(db_descriptor, "update custom_protocol set uri=?, allow=? where protocol=?", -1, &sqlite3_stmt, NULL);
		if (error != SQLITE_OK) {
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 1, uri, -1, NULL) != SQLITE_OK) {
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_int(sqlite3_stmt, 2, (int)allow) != SQLITE_OK){
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 3, protocol, -1, NULL) != SQLITE_OK){
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}

		error = sqlite3_step(sqlite3_stmt);
	} else {
		error = sqlite3_prepare_v2(db_descriptor, "insert into custom_protocol (protocol, uri, allow) values(?,?,?)", -1, &sqlite3_stmt, NULL);
		if (error != SQLITE_OK){
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 1, protocol, -1, NULL) != SQLITE_OK){
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_text(sqlite3_stmt, 2, uri, -1, NULL) != SQLITE_OK){
			db_util_close(db_descriptor);
			return EINA_FALSE;
		}
		if (sqlite3_bind_int(sqlite3_stmt, 3, (int)allow) != SQLITE_OK){
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

