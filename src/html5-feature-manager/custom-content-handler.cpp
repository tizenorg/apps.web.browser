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

extern "C" {
#include "db-util.h"
}

#include "browser-common-view.h"
#include "custom-content-handler.h"

#define custom_protocol_handler_db_path BROWSER_DATA_DIR"/data/db/.custom-protocol-handler.db"

typedef struct _content_handler_data {
	char *base_uri;
	char *target;
	char *uri;
	void *data;
} content_handler_data;

static Eina_Bool is_registered_content_handler(const char *base_uri, const char *mime, int *allow)
{
	BROWSER_LOGD("[%s]", __func__);
	EINA_SAFETY_ON_NULL_RETURN_VAL(mime, EINA_FALSE);

	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_protocol_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(db_descriptor, "select allow from custom_content where base_uri=? and mime=?",								-1, &sqlite3_stmt, NULL);
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

	if (error == SQLITE_ROW)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

Custom_Content_Handler::Custom_Content_Handler(void)
:
	m_custom_content_uri(NULL)
	,m_redirect_uri(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Custom_Content_Handler::~Custom_Content_Handler(void)
{
	BROWSER_LOGD("[%s]", __func__);
	eina_stringshare_del(m_custom_content_uri);
	eina_stringshare_del(m_redirect_uri);
}

void Custom_Content_Handler::activate(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);
	EINA_SAFETY_ON_NULL_RETURN(ewk_view);
	evas_object_smart_callback_add(ewk_view, "contenthandler,registration,requested", __registration_requested_cb, this);
	evas_object_smart_callback_add(ewk_view, "contenthandler,isregistered", __isregistered_cb, this);
	evas_object_smart_callback_add(ewk_view, "contenthandler,unregistration,requested", __unregistration_requested_cb, this);
}

void Custom_Content_Handler::deactivate(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);
	EINA_SAFETY_ON_NULL_RETURN(ewk_view);
	evas_object_smart_callback_del(ewk_view, "contenthandler,registration,requested", __registration_requested_cb);
	evas_object_smart_callback_del(ewk_view, "contenthandler,isregistered", __isregistered_cb);
	evas_object_smart_callback_del(ewk_view, "contenthandler,unregistration,requested", __unregistration_requested_cb);
}

const char *Custom_Content_Handler::get_redirect_uri(const char *orgin, const char *base_uri, const char *mime)
{
	BROWSER_LOGD("orgin=[%s], base_uri=[%s], mime=[%s]", orgin, base_uri, mime);

	int allow = 0;
	if (is_registered_content_handler(base_uri, mime, &allow)) {
		if (!allow)
			return NULL;
		const char *content_uri = _get_content_uri(base_uri, mime);
		if (content_uri && strlen(content_uri)) {
			std::string query_uri_str = std::string(content_uri);
			if (query_uri_str.find("%s") != string::npos) {
				int pos = query_uri_str.find("%s");
				query_uri_str.replace(pos, strlen("%s"), orgin);
				eina_stringshare_replace(&m_redirect_uri, query_uri_str.c_str());

				return m_redirect_uri;
			}
		}
	}

	return NULL;
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

static void _destroy_content_handler(content_handler_data *chd)
{
	if (chd->base_uri)
		free(chd->base_uri);
	if (chd->target)
		free(chd->target);
	if (chd->uri)
		free(chd->uri);
}

void Custom_Content_Handler::__register_protocol_ok_cb(void* data, Evas_Object* obj, void* event_info)
{
	content_handler_data *chd = (content_handler_data *)data;
	Custom_Content_Handler *content_handler = (Custom_Content_Handler *)(chd->data);
	if (!content_handler->_register_content_handler(chd->base_uri, chd->target, chd->uri, EINA_TRUE))
		BROWSER_LOGE("_register_content_handler failed");

	_destroy_popup(obj);
	_destroy_content_handler(chd);
}

void Custom_Content_Handler::__register_protocol_cancel_cb(void* data, Evas_Object* obj, void* event_info)
{
	content_handler_data *chd = (content_handler_data *)data;
	Custom_Content_Handler *content_handler = (Custom_Content_Handler *)(chd->data);
	if (!content_handler->_register_content_handler(chd->base_uri, chd->target, chd->uri, EINA_FALSE))
		BROWSER_LOGE("_register_content_handler failed");

	_destroy_popup(obj);
	_destroy_content_handler(chd);
}

void Custom_Content_Handler::_show_register_content_handler_confirm_popup(const char *message, void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	EINA_SAFETY_ON_NULL_RETURN(message);

	Evas_Object *popup = elm_popup_add(m_win);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup, message);
	evas_object_show(popup);

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

void Custom_Content_Handler::__unregistration_requested_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	Ewk_Custom_Handlers_Data *handler_data = (Ewk_Custom_Handlers_Data *)event_info;
	const char *target = ewk_custom_handlers_data_target_get(handler_data);
	const char *uri = ewk_custom_handlers_data_url_get(handler_data);
	const char *base_uri = ewk_custom_handlers_data_base_url_get(handler_data);
	const char *title = ewk_custom_handlers_data_title_get(handler_data);

	BROWSER_LOGD("target=[%s], uri=[%s], base_uri=[%s], title=[%s]", target, uri, base_uri, title);

	Custom_Content_Handler *content_handler = (Custom_Content_Handler *)data;
	if (!content_handler->_unregister_content_handler(base_uri, target, uri))
		BROWSER_LOGE("_unregister_content_handler failed");
}

void Custom_Content_Handler::__isregistered_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	Ewk_Custom_Handlers_Data *handler_data = (Ewk_Custom_Handlers_Data *)event_info;
	const char *target = ewk_custom_handlers_data_target_get(handler_data);
	const char *uri = ewk_custom_handlers_data_url_get(handler_data);
	const char *base_uri = ewk_custom_handlers_data_base_url_get(handler_data);
	const char *title = ewk_custom_handlers_data_title_get(handler_data);

	BROWSER_LOGD("target=[%s], uri=[%s], base_uri=[%s], title=[%s]", target, uri, base_uri, title);

	Custom_Content_Handler *content_handler = (Custom_Content_Handler *)data;
	int is_allowed = 0;
	if (is_registered_content_handler(base_uri, target, &is_allowed)) {
		if (is_allowed)
			ewk_custom_handlers_data_result_set(handler_data, EWK_CUSTOM_HANDLERS_REGISTERED);
		else
			ewk_custom_handlers_data_result_set(handler_data, EWK_CUSTOM_HANDLERS_DECLINED);
	} else
		ewk_custom_handlers_data_result_set(handler_data, EWK_CUSTOM_HANDLERS_NEW);
}

void Custom_Content_Handler::__registration_requested_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	Ewk_Custom_Handlers_Data *handler_data = (Ewk_Custom_Handlers_Data *)event_info;
	const char *target = ewk_custom_handlers_data_target_get(handler_data);
	const char *uri = ewk_custom_handlers_data_url_get(handler_data);
	const char *base_uri = ewk_custom_handlers_data_base_url_get(handler_data);
	const char *title = ewk_custom_handlers_data_title_get(handler_data);

	BROWSER_LOGD("target=[%s], uri=[%s], base_uri=[%s], title=[%s]", target, uri, base_uri, title);

	std::string msg;
	msg = std::string(base_uri) + std::string(" is asking to register ") + std::string(target) + std::string(" content handler.");

	Custom_Content_Handler *content_handler = (Custom_Content_Handler *)data;

	content_handler_data *chd = (content_handler_data *)malloc(sizeof(content_handler_data));
	memset(chd, 0x00, sizeof(content_handler_data));
	chd->base_uri = strdup(base_uri);
	chd->target = strdup(target);
	chd->uri = strdup(uri);
	chd->data = (void *)content_handler;
	content_handler->_show_register_content_handler_confirm_popup(msg.c_str(), chd);
}

const char *Custom_Content_Handler::_get_content_uri(const char *base_uri, const char *mime)
{
	BROWSER_LOGD("base_uri=[%s], mime=[%s]", base_uri, mime);
	EINA_SAFETY_ON_NULL_RETURN_VAL(base_uri, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(mime, NULL);

	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_protocol_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return NULL;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(db_descriptor, "select uri from custom_content where base_uri=? and mime=?", 							-1, &sqlite3_stmt, NULL);
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

	eina_stringshare_replace(&m_custom_content_uri, uri);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(db_descriptor);
		return NULL;
	}

	return m_custom_content_uri;

}

Eina_Bool Custom_Content_Handler::_register_content_handler(const char *base_uri, const char *mime, const char *uri, Eina_Bool allow)
{
	BROWSER_LOGD("base_uri=[%s], mime=[%s], uri=[%s]", base_uri, mime, uri);

	EINA_SAFETY_ON_NULL_RETURN_VAL(mime, EINA_FALSE);
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
	if (is_registered_content_handler(base_uri, mime, &is_allowed)) {
		BROWSER_LOGD("update");
		error = sqlite3_prepare_v2(db_descriptor, "update custom_content set base_uri=?, uri=?, allow=? where mime=?", -1, &sqlite3_stmt, NULL);
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

		error = sqlite3_step(sqlite3_stmt);
	} else {
		BROWSER_LOGD("insert");
		error = sqlite3_prepare_v2(db_descriptor, "insert into custom_content (base_uri, mime, uri, allow) values(?,?,?,?)", -1, &sqlite3_stmt, NULL);
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

Eina_Bool Custom_Content_Handler::_unregister_content_handler(const char *base_uri, const char *mime, const char *uri)
{
	BROWSER_LOGD("protocol = [%s], uri=[%s]", mime, uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(mime, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	sqlite3 *db_descriptor = NULL;
	int error = db_util_open(custom_protocol_handler_db_path, &db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(db_descriptor);
		db_descriptor = NULL;
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(db_descriptor, "delete from custom_content where base_uri=? and mime=? and uri=?", -1, &sqlite3_stmt, NULL);
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
		BROWSER_LOGD("*** sqlite3_finalize failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

