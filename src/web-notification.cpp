/*
*  browser
*
* Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
*
* Contact: Junghwan Kang <junghwan.kang@samsung.com>
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

#include "web-notification.h"

#include <app_control.h>
#include <app_control_internal.h>
#include <appsvc.h>
#include <Ecore.h>
#include <eina_list.h>
#include <fcntl.h>
#include <notification_internal.h>
#include <string>

extern "C" {
#include "db-util.h"
}

#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "platform-service.h"
#include "settings.h"
#include "settings-advanced.h"
#include "webview-list.h"
#include "platform-service.h"

#if defined(HW_MORE_BACK_KEY)
#include <efl_extension.h>
#endif

#define PATH_WEB_NOTI_ICON_PNG browser_data_dir"/"
#define ICON_FILE_NAME_MAX_LENGTH 10
#define ICON_DOWNLOAD_TIME_LIMIT 5
#define PATH_WEB_NOTI_DB_PATH browser_data_dir"/db/.browser-web-notification.db"

static Eina_Bool _check_web_notification_exist(const char *uri)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	sqlite3 *descriptor = NULL;
	int error = db_util_open(PATH_WEB_NOTI_DB_PATH, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select accept from web_notification where address=?", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGE("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	error = sqlite3_bind_text(sqlite3_stmt, 1, uri, -1, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGE("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (error == SQLITE_ROW) {
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

static Eina_Bool _update_web_notification(const char *uri, bool accept)
{
	BROWSER_SECURE_LOGD("uri[%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	sqlite3 *descriptor = NULL;
	int error = db_util_open(PATH_WEB_NOTI_DB_PATH, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "update web_notification set accept=?, updatedate=DATETIME('now') where address=?", -1, &sqlite3_stmt, NULL);
	if (sqlite3_bind_int(sqlite3_stmt, 1, accept) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize for accept is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 2, uri, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGE("SQL error=%d", error);
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

static Eina_Bool _save_web_notification(const char *uri, const char *title, bool accept)
{
	BROWSER_SECURE_LOGD("uri[%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	sqlite3 *descriptor = NULL;
	int error = db_util_open(PATH_WEB_NOTI_DB_PATH, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "insert into web_notification (address, title, accept, updatedate) values(?, ?, ?, DATETIME('now'))", -1, &sqlite3_stmt, NULL);

	if (sqlite3_bind_text(sqlite3_stmt, 1, uri, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGE("uri save : SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (title) {
		if (sqlite3_bind_text(sqlite3_stmt, 2, title, -1, NULL) != SQLITE_OK) {
			BROWSER_LOGE("title save : SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			db_util_close(descriptor);
			return EINA_FALSE;
		}
	} else {
		if (sqlite3_bind_text(sqlite3_stmt, 2, "", -1, NULL) != SQLITE_OK) {
			BROWSER_LOGE("title save : SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			db_util_close(descriptor);
			return EINA_FALSE;
		}
	}
	if (sqlite3_bind_int(sqlite3_stmt, 3, accept) != SQLITE_OK) {
		BROWSER_LOGE("accept save :SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_step - sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	db_util_close(descriptor);

	return EINA_TRUE;
}

static Eina_Bool _delete_web_notification(const char *uri)
{
	BROWSER_SECURE_LOGD("uri[%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	sqlite3 *descriptor = NULL;
	int error = db_util_open(PATH_WEB_NOTI_DB_PATH, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "delete from web_notification where address=?", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGE("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	if (sqlite3_bind_text(sqlite3_stmt, 1, uri, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.");

	error = sqlite3_step(sqlite3_stmt);
	if (error != SQLITE_OK && error != SQLITE_DONE) {
		BROWSER_LOGE("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	db_util_close(descriptor);

	return EINA_TRUE;
}

static Eina_Bool _delete_all_web_notification(void)
{
	BROWSER_LOGD("");

	sqlite3 *descriptor = NULL;
	int error = db_util_open(PATH_WEB_NOTI_DB_PATH, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "delete from web_notification", -1, &sqlite3_stmt, NULL);
	if (sqlite3_step(sqlite3_stmt) != SQLITE_ROW)
		BROWSER_LOGE("sqlite3_step failed");

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_finalize failed");
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	db_util_close(descriptor);

	return EINA_TRUE;
}

static int _get_count(void)
{
	BROWSER_LOGD("");

	sqlite3 *descriptor = NULL;
	int count = 0;
	int error = db_util_open(PATH_WEB_NOTI_DB_PATH, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return -1;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select count(*) from web_notification", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGE("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return -1;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (error != SQLITE_ROW) {
		BROWSER_LOGE("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return -1;
	}

	count = sqlite3_column_int(sqlite3_stmt, 0);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return -1;
	}

	BROWSER_LOGD("total num of web-notification count[%d]", count);
	return count;
}

static Eina_Bool _get_web_notification_allow(const char *uri, bool *accept)
{
	BROWSER_SECURE_LOGD("uri[%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	if (_check_web_notification_exist(uri) == EINA_FALSE) {
		BROWSER_LOGE("No data or database error\n");
		return EINA_FALSE;
	}

	sqlite3 *descriptor = NULL;
	int error = db_util_open(PATH_WEB_NOTI_DB_PATH, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select accept from web_notification where address=?", -1, &sqlite3_stmt, NULL);

	if (error != SQLITE_OK) {
		BROWSER_LOGE("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	error = sqlite3_bind_text(sqlite3_stmt, 1, uri, -1, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGE("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (error == SQLITE_ROW) {
		*accept = sqlite3_column_int(sqlite3_stmt, 0);
		BROWSER_SECURE_LOGD("uri: %s accept %d", uri, *accept);
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	db_util_close(descriptor);

	if (error == SQLITE_DONE || error == SQLITE_ROW)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}





web_notification_setting_ask_popup::web_notification_setting_ask_popup(void)
:
	m_selected_type(WEB_NOTI_ENABLE_TYPE_ON_DEMAND),
	m_radio_main(NULL),
	m_popup(NULL),
	m_popup_last_item(NULL)
{
	BROWSER_LOGD("");

}

web_notification_setting_ask_popup::~web_notification_setting_ask_popup(void)
{
	BROWSER_LOGD("");

	_destroy_genlist_callback_datas();

	if (m_popup)
		evas_object_del(m_popup);
	m_popup = NULL;
}

Eina_Bool web_notification_setting_ask_popup::show(void)
{
	BROWSER_LOGD("");

	Evas_Object *popup = brui_popup_add(m_window);
	if (!popup) {
		BROWSER_LOGE("failed to brui_popup_add");
		return EINA_FALSE;
	}
	m_popup = popup;

#if defined(HW_MORE_BACK_KEY)
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __cancel_button_cb, this);
#endif

	elm_object_style_set(popup,"default");
	elm_object_domain_translatable_part_text_set(popup, "title,text", BROWSER_DOMAIN_NAME, "IDS_BR_BODY_NOTIFICATIONS");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(popup, "block,clicked", __cancel_button_cb, this);

	Evas_Object *genlist = _create_genlist(popup);
	if (!genlist) {
		BROWSER_LOGE("failed to _create_genlist");
		return EINA_FALSE;
	}

	elm_scroller_content_min_limit(genlist, EINA_FALSE, EINA_TRUE);
	evas_object_show(genlist);
	elm_object_content_set(popup, genlist);

	evas_object_show(popup);

	return EINA_TRUE;
}

void web_notification_setting_ask_popup::__genlist_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");
	RET_MSG_IF(!(event_info), "event_info is NULL");

	web_notification_setting_ask_popup *popup_class = (web_notification_setting_ask_popup *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	if (it == popup_class->m_popup_last_item)
		elm_object_item_signal_emit(it, "elm,state,bottomline,hide", "");
}

Evas_Object *web_notification_setting_ask_popup::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return EINA_FALSE;
	}
	evas_object_smart_callback_add(genlist, "realized", __genlist_realized_cb, this);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	m_radio_main = elm_radio_add(genlist);
	if (!m_radio_main) {
		BROWSER_LOGE("elm_radio_add failed");
		return NULL;
	}
	elm_radio_state_value_set(m_radio_main, 0);
	elm_radio_value_set(m_radio_main, 0);

	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();

	item_ic->item_style = "1line";
	item_ic->func.text_get = __genlist_text_get;
	item_ic->func.content_get = __genlist_contents_get;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;

	for (int i = 0; i < WEB_NOTI_ENABLE_TYPE_NUM; i++) {
		web_noti_popup_ask_callback_data *cb_data = new web_noti_popup_ask_callback_data;
		memset(cb_data, 0x00, sizeof(web_noti_popup_ask_callback_data));

		cb_data->type = (web_notification_enable_type)i;
		cb_data->user_data = this;
		cb_data->it =  elm_genlist_item_append(genlist, item_ic, cb_data, NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, cb_data);
		m_popup_last_item = cb_data->it;
		m_genlist_callback_data_list.push_back(cb_data);
	}

	elm_genlist_item_class_free(item_ic);

	return genlist;
}

void web_notification_setting_ask_popup::_destroy_genlist_callback_datas(void)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_genlist_callback_data_list.size(); i++) {
		if (m_genlist_callback_data_list[i])
			free(m_genlist_callback_data_list[i]);
		m_genlist_callback_data_list[i] = NULL;
	}

	m_genlist_callback_data_list.clear();
}

char *web_notification_setting_ask_popup::__genlist_text_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, NULL, "data is NULL");

	web_noti_popup_ask_callback_data *callback_data = (web_noti_popup_ask_callback_data *)data;
	web_notification_enable_type type = (web_notification_enable_type)callback_data->type;

	char *label = NULL;

	if (!strcmp(part, "elm.text.main.left")) {
		if (type == WEB_NOTI_ENABLE_TYPE_ALWAYS)
			label = strdup(BR_STRING_SETTINGS_ENABLE_NOTIFICATIONS_ALWAYS);
		else if (type == WEB_NOTI_ENABLE_TYPE_ON_DEMAND)
			label = strdup(BR_STRING_SETTINGS_ENABLE_NOTIFICATIONS_ON_DEMAND);
		else if (type == WEB_NOTI_ENABLE_TYPE_OFF)
			label = strdup(BR_STRING_OPT_DISABLE);
	}

	return label;
}

Evas_Object *web_notification_setting_ask_popup::__genlist_contents_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, NULL, "data is NULL");

	web_noti_popup_ask_callback_data *callback_data = (web_noti_popup_ask_callback_data *)data;
	web_notification_setting_ask_popup *popup = (web_notification_setting_ask_popup *)(callback_data->user_data);

	Evas_Object *content = NULL;

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.icon.right")) {
		Evas_Object *radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, callback_data->type);
		elm_radio_group_add(radio, popup->m_radio_main);

		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_propagate_events_set(radio, EINA_FALSE);

		elm_radio_value_set(popup->m_radio_main, m_preference->get_web_notification_accept_type());

		evas_object_smart_callback_add(radio, "changed", __radio_icon_changed_cb, (void *)data);

		content = radio;
	}

	return content;
}

void web_notification_setting_ask_popup::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	web_noti_popup_ask_callback_data *callback_data = (web_noti_popup_ask_callback_data *)data;
	web_notification_enable_type type = (web_notification_enable_type)callback_data->type;
	web_notification_setting_ask_popup *popup_class = (web_notification_setting_ask_popup *)(callback_data->user_data);
	Elm_Object_Item *item = callback_data->it;

	if (item) {
		elm_genlist_item_selected_set(item, EINA_FALSE);

		Evas_Object *radio = elm_object_item_part_content_get(item, "elm.icon");
		if (radio) {
			elm_radio_state_value_set(radio, type);
			elm_radio_value_set(popup_class->m_radio_main, type);
		}
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}

	popup_class->m_selected_type = type;
	m_preference->set_web_notification_accept_type(popup_class->m_selected_type);
	m_browser->get_settings()->get_contents_settings_edit()->realize_genlist();

	if (popup_class->m_popup) {
		evas_object_del(popup_class->m_popup);
		popup_class->m_popup = NULL;
	}
	evas_object_freeze_events_set(m_browser->get_settings()->get_contents_settings_edit()->get_genlist(), EINA_FALSE);
}

void web_notification_setting_ask_popup::__radio_icon_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	web_noti_popup_ask_callback_data *callback_data = (web_noti_popup_ask_callback_data *)data;
	web_notification_enable_type type = (web_notification_enable_type)callback_data->type;
	web_notification_setting_ask_popup *popup_class = (web_notification_setting_ask_popup *)(callback_data->user_data);
	Elm_Object_Item *item = callback_data->it;

	if (item) {
		elm_genlist_item_selected_set(item, EINA_FALSE);

		Evas_Object *radio = elm_object_item_part_content_get(item, "elm.icon");
		if (radio) {
			elm_radio_state_value_set(radio, type);
			elm_radio_value_set(popup_class->m_radio_main, type);
		}
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}

	popup_class->m_selected_type = type;
	m_preference->set_web_notification_accept_type(popup_class->m_selected_type);
	m_browser->get_settings()->get_contents_settings_edit()->realize_genlist();

	if (popup_class->m_popup) {
		evas_object_del(popup_class->m_popup);
		popup_class->m_popup = NULL;
	}
	evas_object_freeze_events_set(m_browser->get_settings()->get_contents_settings_edit()->get_genlist(), EINA_FALSE);
}

void web_notification_setting_ask_popup::__cancel_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	web_notification_setting_ask_popup *popup_class = (web_notification_setting_ask_popup*)data;
	if (popup_class->m_popup)
		evas_object_del(popup_class->m_popup);
	popup_class->m_popup = NULL;
	evas_object_freeze_events_set(m_browser->get_settings()->get_contents_settings_edit()->get_genlist(), EINA_FALSE);
}

void web_notification_setting_ask_popup::__ok_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	web_notification_setting_ask_popup *popup_class = (web_notification_setting_ask_popup *)data;
	m_preference->set_web_notification_accept_type(popup_class->m_selected_type);

	m_browser->get_settings()->get_contents_settings_edit()->realize_genlist();

	if (popup_class->m_popup)
		evas_object_del(popup_class->m_popup);
	popup_class->m_popup = NULL;
	evas_object_freeze_events_set(m_browser->get_settings()->get_contents_settings_edit()->get_genlist(), EINA_FALSE);
}

web_notification_item::web_notification_item(webview *wv, Ewk_Notification *ewk_web_noti)
:
	m_ewk_noti(ewk_web_noti)
	,m_webview(wv)
	,m_origin(NULL)
	,m_title(NULL)
	,m_body(NULL)
	,m_icon_uri(NULL)
	,m_icon_path(NULL)
	,m_ewk_noti_id(-1)
	,m_noti_id(-1)
	,m_time_stamp(-1.0)
	,m_noti_handle(NULL)
	,m_icon_download_timer(NULL)
{
	_init();
}

web_notification_item::~web_notification_item(void)
{
	if (m_icon_path)
		unlink(m_icon_path);

	BROWSER_SECURE_LOGD("m_origin[%s]", m_origin);

	eina_stringshare_del(m_origin);
	eina_stringshare_del(m_title);
	eina_stringshare_del(m_body);
	eina_stringshare_del(m_icon_uri);
	eina_stringshare_del(m_icon_path);

	if (m_noti_handle) {
		notification_delete(m_noti_handle);
		notification_free(m_noti_handle);
	}
	_remove_icon_download_timer();
}

Eina_Bool web_notification_item::make_notification(void)
{
	Eina_Bool make_result = EINA_FALSE;

	/* check duplicated */

	if (m_icon_uri && strlen(m_icon_uri)) {
		/* download icon */
		make_result = _download_notification_icon_start();
		if (make_result == EINA_FALSE)
			make_result = _create_notification(EINA_FALSE);
	} else
		make_result = _create_notification(EINA_FALSE);
#ifdef WEBKIT_EFL
	Ewk_Context* context = ewk_context_default_get();
	ewk_notification_showed(context, m_ewk_noti_id);
#else
	ewk_notification_showed(m_ewk_noti_id);
#endif // WEBKIT_EFL
	return make_result;
}

Eina_Bool web_notification_item::webview_validate_check(void)
{
	BROWSER_LOGD("");
	for (unsigned int i = 0; i < m_browser->get_webview_list()->get_count(); i++) {
		if (m_webview && (m_browser->get_webview_list()->get_webview(i) == m_webview))
			return EINA_TRUE;
	}

	return EINA_FALSE;
}

Eina_Bool web_notification_item::deactivate_item(void)
{
	BROWSER_LOGD("");

	if (webview_validate_check() == EINA_TRUE) {
		Eina_List *list = NULL;
		list = eina_list_append(list, m_ewk_noti);
		ewk_view_notification_closed(m_webview->get_ewk_view(), list);
		eina_list_free(list);
	} else
		BROWSER_LOGE("The webview, which this notification item had, is not valid any more");

	return EINA_TRUE;
}

void web_notification_item::delete_notification(void)
{
	BROWSER_LOGD("");

	int err = NOTIFICATION_ERROR_NONE;
	err = notification_delete_by_priv_id(sec_browser_app, NOTIFICATION_TYPE_NOTI, get_noti_id());
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("Fail to delete NOTIFICATION_TYPE_NOTI with err[%d]", err);
	}

	m_noti_id = -1;
}

void web_notification_item::_init(void)
{
	/* init origin */
	const char *origin = NULL;
	origin = ewk_security_origin_host_get(ewk_notification_security_origin_get(m_ewk_noti));
	if (origin && strlen(origin))
		m_origin = eina_stringshare_add(origin);
	else
		m_origin = eina_stringshare_add(BR_STRING_NO_NAME);

	/* init title */
	const char *title = NULL;
	title = ewk_notification_title_get(m_ewk_noti);
	if (title && strlen(title))
		m_title = eina_stringshare_add(title);
	else
		m_title = eina_stringshare_add(BR_STRING_NO_NAME);

	/* init body */
	const char *body = NULL;
	body = ewk_notification_body_get(m_ewk_noti);
	if (body && strlen(body))
		m_body = eina_stringshare_add(body);
	else
		m_body = eina_stringshare_add(BR_STRING_NO_NAME);

	/* init icon_url */
	const char *icon_url = NULL;
	icon_url = ewk_notification_icon_url_get(m_ewk_noti);
	if (icon_url && strlen(icon_url))
		m_icon_uri = eina_stringshare_add(icon_url);

	/* init ewk noti id */
	m_ewk_noti_id = (int)ewk_notification_id_get(m_ewk_noti);

	/* notification unique ID */
	m_time_stamp = ecore_loop_time_get();

	/* set current webview */
	if (!m_webview)
		m_webview = m_browser->get_browser_view()->get_current_webview();
}

Eina_Bool web_notification_item::_create_notification(Eina_Bool has_icon)
{
	BROWSER_LOGD("has_icon[%d]", has_icon);

	int priv_id = 0;
	char ewk_noti_id[10] = {0, };
	bundle *selected_data = NULL;
	app_control_h app_control = NULL;

	int err = NOTIFICATION_ERROR_NONE;
	notification_h handle = notification_create(NOTIFICATION_TYPE_NOTI);
	if (!handle) {
		BROWSER_LOGE("Failed to create notification");
		return EINA_FALSE;
	}

	err = notification_set_layout(handle, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("notification_set_layout NOTIFICATION_LY_NOTI_EVENT_MULTIPLE failed with err[%d]", err);
		goto ERROR;
	}

	err = notification_set_text(handle, NOTIFICATION_TEXT_TYPE_TITLE, BR_STRING_INTERNET, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("Failed to set title in notification with err[%d]", err);
		goto ERROR;
	}

	if (m_origin && strlen(m_origin))
		err = notification_set_text(handle, NOTIFICATION_TEXT_TYPE_CONTENT, m_origin, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		err = notification_set_text(handle, NOTIFICATION_TEXT_TYPE_CONTENT, BR_STRING_NO_NAME, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("notification_set_text m_origin failed with err[%d]", err);
		goto ERROR;
	}

	if (m_title && strlen(m_title))
		err = notification_set_text(handle, NOTIFICATION_TEXT_TYPE_INFO_1, m_title, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		err = notification_set_text(handle, NOTIFICATION_TEXT_TYPE_INFO_1, BR_STRING_NO_NAME, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);

	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("notification_set_text m_title failed with err[%d]", err);
		goto ERROR;
	}

	if (m_body && strlen(m_body))
		err = notification_set_text(handle, NOTIFICATION_TEXT_TYPE_INFO_2, m_body, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	else
		err = notification_set_text(handle, NOTIFICATION_TEXT_TYPE_INFO_2, BR_STRING_NO_NAME, NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("notification_set_text m_title failed with err[%d]", err);
		goto ERROR;
	}

	if (has_icon == EINA_TRUE && m_icon_path && strlen(m_icon_path)) {
		err = notification_set_image(handle, NOTIFICATION_IMAGE_TYPE_ICON, m_icon_path);
		if (err != NOTIFICATION_ERROR_NONE)
			BROWSER_LOGE("notification_set_image failed with err[%d]", err);
	}

	if (app_control_create(&app_control) != APP_CONTROL_ERROR_NONE) {
		BROWSER_LOGE("Fail to create app_control handle");
		goto ERROR;
	}

	if (app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW) != APP_CONTROL_ERROR_NONE) {
		BROWSER_LOGE("Fail to set app_control operation");
		goto ERROR;
	}

	if (app_control_set_app_id(app_control, sec_browser_app) != APP_CONTROL_ERROR_NONE) {
		BROWSER_LOGE("Fail to app_control_set_app_id");
		goto ERROR;
	}

	snprintf(ewk_noti_id, 10, "%d", m_ewk_noti_id);

	if (app_control_add_extra_data(app_control, "web_noti_launch", ewk_noti_id) != APP_CONTROL_ERROR_NONE) {
		BROWSER_LOGE("app_control_add_extra_data is failed.");
		goto ERROR;
	}

	if (m_origin && strlen(m_origin)) {
		if (app_control_set_uri(app_control, m_webview->get_uri()) != APP_CONTROL_ERROR_NONE) {
			BROWSER_LOGE("Fail to app_control_set_uri");
			goto ERROR;
		}
	}

	if (app_control_to_bundle(app_control, &selected_data) != APP_CONTROL_ERROR_NONE) {
		BROWSER_LOGE("Fail to app_control_to_bundle");
		goto ERROR;
	}

	if (selected_data) {
		err = notification_set_execute_option(handle, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, NULL, selected_data);
		if(err != NOTIFICATION_ERROR_NONE)
			BROWSER_LOGE("notification_set_execute_option is failed with err[%d]", err);
	}

	err = notification_insert(handle, &priv_id);
	if(err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGE("notification_insert is failed with err[%d]", err);
		goto ERROR;
	}

	app_control_destroy(app_control);

	m_noti_id = priv_id;
	m_noti_handle = handle;

	return EINA_TRUE;

ERROR:
	err = notification_free(handle);
	if(err != NOTIFICATION_ERROR_NONE)
		BROWSER_LOGE("notification_free is failed with err[%d]", err);

	if (app_control != NULL)
		app_control_destroy(app_control);

	return EINA_FALSE;
}

Eina_Bool web_notification_item::_download_notification_icon_start(void)
{
	BROWSER_SECURE_LOGD("icon uri[%s]", m_icon_uri);
	RETV_MSG_IF(!m_icon_uri, EINA_FALSE, "m_icon_uri is NULL");

	SoupSession *soup_session = NULL;
	SoupMessage *soup_msg = NULL;

	soup_session = soup_session_async_new();
	if (!soup_session) {
		BROWSER_LOGE("Failed to soup_session_async_new");
		return EINA_FALSE;
	}
	g_object_set(soup_session, SOUP_SESSION_TIMEOUT, 15, (char *)NULL);

	const char *default_proxy_uri = ewk_context_proxy_uri_get(ewk_context_default_get());
	if (default_proxy_uri) {
		std::string proxy_uri = std::string(http_scheme) + std::string(default_proxy_uri);
		SoupURI *soup_uri = soup_uri_new(proxy_uri.c_str());
		g_object_set(soup_session, SOUP_SESSION_PROXY_URI, soup_uri, (char *)NULL);
		if (soup_uri)
			soup_uri_free(soup_uri);
	}
	soup_msg = soup_message_new("GET", m_icon_uri);
	soup_session_queue_message(soup_session, soup_msg, __download_notification_icon_finished_cb, (void *)this);

	_remove_icon_download_timer();
	m_icon_download_timer = ecore_timer_add(ICON_DOWNLOAD_TIME_LIMIT, __icon_download_timer_expired_cb, this);

	return EINA_TRUE;
}

Eina_Bool web_notification_item::_save_notification_icon(SoupBuffer *body)
{
	BROWSER_LOGD("body[%p]", body);
	RETV_MSG_IF(!body, EINA_FALSE, "body is NULL");

	if (!body->data || body->length <= 0) {
		BROWSER_LOGE("body has no valid data");
		soup_buffer_free(body);
		return EINA_FALSE;
	}

	int fd = 0;
	int write_len = 0;

	char time_stamp[ICON_FILE_NAME_MAX_LENGTH + 1] = {0, };
	snprintf(time_stamp, ICON_FILE_NAME_MAX_LENGTH, "%lf", m_time_stamp);

	std::string file_name = std::string(PATH_WEB_NOTI_ICON_PNG) + std::string(time_stamp) + std::string(".png");

	if ((fd = open(file_name.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		soup_buffer_free(body);
		return EINA_FALSE;
	}
	write_len = write(fd, body->data, body->length);
	close(fd);

	soup_buffer_free(body);

	if (write_len != (int)body->length)
		return EINA_FALSE;

	m_icon_path = eina_stringshare_add(file_name.c_str());

	return EINA_TRUE;
}

void web_notification_item::_remove_icon_download_timer(void)
{
	if (m_icon_download_timer)
		ecore_timer_del(m_icon_download_timer);
	m_icon_download_timer = NULL;
}

void web_notification_item::__download_notification_icon_finished_cb(SoupSession *session, SoupMessage *msg, gpointer data)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!msg, "msg is NULL");
	RET_MSG_IF(!data, "data is NULL");

	SoupBuffer *body = soup_message_body_flatten(msg->response_body);

	web_notification_item *wni = (web_notification_item *)data;
	if (wni->m_icon_download_timer) {
		wni->_remove_icon_download_timer();
		wni->_create_notification(wni->_save_notification_icon(body));
	} else
		BROWSER_LOGD("Icon data is received too late, and it's already been made by default icon");
}

Eina_Bool web_notification_item::__icon_download_timer_expired_cb(void *data)
{
	BROWSER_LOGD("");
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	web_notification_item *wni = (web_notification_item *)data;
	wni->_remove_icon_download_timer();
	wni->_create_notification(EINA_FALSE);

	wni->show_noti_popup(BR_STRING_FAILED_TO_GET_WEB_NOTI_ICON);

	return ECORE_CALLBACK_CANCEL;
}

web_notification_manager::web_notification_manager(void)
:
	m_web_notification_setting_ask_popup(NULL),
	m_never_ask_check_box(NULL)
{
	BROWSER_LOGD("");
}

web_notification_manager::~web_notification_manager(void)
{
	BROWSER_LOGD("m_web_noti_item_list.size()[%d]", m_web_noti_item_list.size());

	for (unsigned int i = 0; i < m_web_noti_item_list.size(); i++) {
		if (m_web_noti_item_list[i]) {
			delete m_web_noti_item_list[i];
			m_web_noti_item_list.erase(m_web_noti_item_list.begin() + i);
		}
	}
	m_web_noti_item_list.clear();
	_clear_permission_origin_data();
	delete_web_notification_setting_ask_popup();
}

Eina_Bool web_notification_manager::save_web_notification(const char *uri, bool accept, const char *title)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	int error = 0;

	if (_check_web_notification_exist(uri) == EINA_TRUE)
		error = _update_web_notification(uri, accept);
	else
		error = _save_web_notification(uri, title, accept);

	return (error == SQLITE_DONE || error == SQLITE_ROW);
}

Eina_Bool web_notification_manager::get_web_notification_allow(const char *uri)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	bool accept = false;
	if (_get_web_notification_allow(uri, &accept) == EINA_FALSE)
		return EINA_FALSE;
	if (accept == true)
		return EINA_TRUE;

	return EINA_FALSE;
}

Eina_Bool web_notification_manager::delete_saved_origins(const char *uri)
{
	BROWSER_LOGD("");

	return _delete_web_notification(uri);
}

Eina_Bool web_notification_manager::delete_all_saved_origins(void)
{
	BROWSER_LOGD("");

	return _delete_all_web_notification();
}

Eina_Bool web_notification_manager::delete_all_notifications(void)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(m_web_noti_item_list.size() == 0, EINA_FALSE, "there is no active notification");

	for (unsigned int i = 0; i < m_web_noti_item_list.size(); i++) {
		if (m_web_noti_item_list[i])
			delete m_web_noti_item_list[i];
	}
	m_web_noti_item_list.clear();

	return EINA_TRUE;
}

int web_notification_manager::get_web_noti_count(void)
{
	return _get_count();
}

void web_notification_manager::reset_policy(void)
{
	BROWSER_LOGD("");

	Eina_List *list = NULL;
	Ewk_Context *context = ewk_context_default_get();
	RET_MSG_IF(!context, "ewk_context_default_get is NULL");

	for (unsigned int i = 0; i < m_permission_origin_list.size(); i++) {
		web_noti_permission_origin_data *data = m_permission_origin_list[i];
		if (data != NULL) {
			if (!(data->parent_webview != NULL && m_browser->get_webview_list()->is_webview_exist(data->parent_webview) == EINA_TRUE))
				continue;

			if (data->permission_origin == NULL)
				continue;

			list = eina_list_append(list, m_permission_origin_list[i]->permission_origin);
		}
	}
	if (list) {
#ifdef WEBKIT_EFL
		Ewk_Context* context = ewk_context_default_get();
		ewk_notification_policies_removed(context, list);
#else
		ewk_notification_policies_removed(list);
#endif
	}

	_clear_permission_origin_data();
}

web_notification_setting_ask_popup *web_notification_manager::get_setting_ask_popup(void)
{
	if (!m_web_notification_setting_ask_popup)
		m_web_notification_setting_ask_popup = new web_notification_setting_ask_popup();

	return m_web_notification_setting_ask_popup;
}

void web_notification_manager::delete_web_notification_setting_ask_popup(void)
{
	if (m_web_notification_setting_ask_popup)
		delete m_web_notification_setting_ask_popup;
	m_web_notification_setting_ask_popup = NULL;
}

Eina_Bool web_notification_manager::handle_permission_request(webview *parent_webview, Ewk_Notification_Permission_Request *permission_request)
{
	BROWSER_SECURE_LOGD("parent_webview[%p], permission_request[%p]", parent_webview, permission_request);
	RETV_MSG_IF(!parent_webview, EINA_FALSE, "parent_webview is NULL");
	RETV_MSG_IF(!permission_request, EINA_FALSE, "permission_request is NULL");

	const Ewk_Security_Origin *security_origin = ewk_notification_permission_request_origin_get(permission_request);
	if (security_origin == NULL) {
		ewk_notification_permission_reply(permission_request, EINA_FALSE);
		return EINA_TRUE;
	}

	web_noti_permission_origin_data *origin_data = (web_noti_permission_origin_data *)calloc(1, sizeof(web_noti_permission_origin_data));
	if (origin_data == NULL) {
		ewk_notification_permission_reply(permission_request, EINA_FALSE);
		return EINA_TRUE;
	}

	origin_data->parent_webview = parent_webview;
	origin_data->permission_origin = security_origin;

	m_permission_origin_list.push_back(origin_data);
	const char *origin = ewk_security_origin_host_get(security_origin);
	if (!origin) {
		BROWSER_LOGE("origin is NULL");
		return EINA_FALSE;
	}

	web_notification_enable_type type = m_preference->get_web_notification_accept_type();
	if (type == WEB_NOTI_ENABLE_TYPE_ALWAYS) {
		ewk_notification_permission_reply(permission_request, EINA_TRUE);
		return EINA_TRUE;
	} else if (type == WEB_NOTI_ENABLE_TYPE_OFF) {
		ewk_notification_permission_reply(permission_request, EINA_FALSE);
		return EINA_TRUE;
	} else {
		if (get_web_notification_allow(origin) == EINA_TRUE) {
			ewk_notification_permission_reply(permission_request, EINA_FALSE);
			return EINA_TRUE;
		}
	}

	return _show_permission_request_popup(origin, permission_request);
}

Eina_Bool web_notification_manager::set_web_notification(webview *wv, Ewk_Notification *ewk_web_noti)
{
	BROWSER_LOGD("wv[%p], ewk_web_noti[%p]", wv, ewk_web_noti);
	RETV_MSG_IF(!wv, EINA_FALSE, "wv is NULL");
	RETV_MSG_IF(!ewk_web_noti, EINA_FALSE, "ewk_web_noti is NULL");

	web_notification_item *item = new web_notification_item(wv, ewk_web_noti);
	Eina_Bool is_duplicated = EINA_FALSE;
	for (unsigned int i = 0; i < m_web_noti_item_list.size(); i++) {
		if (m_web_noti_item_list[i] == item) {
			BROWSER_SECURE_LOGD("origin[%s], title[%s], body[%s] is already exist. Not making on noti bar", item->get_origin(), item->get_title(), item->get_body());
			is_duplicated = EINA_TRUE;
			break;
		}
	}
	if (is_duplicated == EINA_TRUE) {
		item->deactivate_item();
		delete item;
	} else {
		item->make_notification();
		m_web_noti_item_list.push_back(item);
	}

	return EINA_TRUE;
}

Eina_Bool web_notification_manager::unset_web_notification(int ewk_noti_id)
{
	BROWSER_LOGD("ewk_noti_id[%d]", ewk_noti_id);
	if (ewk_noti_id == 0) {
		BROWSER_LOGE("ewk_noti_id is unrecognizable as 0");
		return EINA_FALSE;
	}

	for (unsigned int i = 0; i < m_web_noti_item_list.size(); i++) {
		if (m_web_noti_item_list[i]->get_ewk_noti_id() == ewk_noti_id) {
			m_web_noti_item_list[i]->deactivate_item();
			delete m_web_noti_item_list[i];
			m_web_noti_item_list.erase(m_web_noti_item_list.begin() + i);
			break;
		}
	}
	return EINA_TRUE;
}

Eina_Bool web_notification_manager::web_noti_selected(int ewk_noti_id)
{
	BROWSER_LOGD("ewk_noti_id[%d]", ewk_noti_id);
	if (ewk_noti_id == 0) {
		BROWSER_LOGE("ewk_noti_id is unrecognizable as 0");
		return EINA_FALSE;
	}

	Eina_Bool return_val = EINA_FALSE;

	for (unsigned int i = 0; i < m_web_noti_item_list.size(); i++) {
		if (m_web_noti_item_list[i]->get_ewk_noti_id() == ewk_noti_id) {
#ifdef WEBKIT_EFL
			Ewk_Context* context = ewk_context_default_get();
			ewk_notification_clicked(context, ewk_noti_id);
#else
			ewk_notification_clicked(ewk_noti_id);
#endif
			if (m_web_noti_item_list[i]->get_webview()) {
				for (unsigned int i = 0; i < m_browser->get_webview_list()->get_count(); i++) {
					if (m_web_noti_item_list.size() > i && m_browser->get_webview_list()->get_webview(i) == m_web_noti_item_list[i]->get_webview()) {
						m_browser->get_browser_view()->set_current_webview(m_web_noti_item_list[i]->get_webview());
						m_web_noti_item_list[i]->get_webview()->activate();
						return_val = EINA_TRUE;
						break;
					}
				}
			}
			delete m_web_noti_item_list[i];
			m_web_noti_item_list.erase(m_web_noti_item_list.begin() + i);
			break;
		}
	}
	return return_val;
}

Eina_Bool web_notification_manager::_show_permission_request_popup(const char *uri, Ewk_Notification_Permission_Request *permission_request)
{
	BROWSER_SECURE_LOGD("uri[%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	/* Make popup for permission request to user */
	char *request_msg = NULL;
	int prefix_len = strlen(BR_STRING_REQUEST_WEBPAGE_PREFIX);
	int host_string_len = strlen(uri);
	int popup_string_len = strlen(BR_STRING_WEB_NOTI_PERMISSION_POPUP_DESC) + 8;// margin of %1s and %2s

	request_msg = (char *)malloc((prefix_len + host_string_len + popup_string_len + 1) * sizeof(char));// 1 is reserves for '\0'
	if (!request_msg) {
		BROWSER_LOGE("Failed to allocate memory for request_msg");
		return EINA_FALSE;
	}
	memset(request_msg, 0, (prefix_len + host_string_len + popup_string_len + 1));
	snprintf(request_msg, (prefix_len + host_string_len + popup_string_len), BR_STRING_WEB_NOTI_PERMISSION_POPUP_DESC, BR_STRING_REQUEST_WEBPAGE_PREFIX, uri);

	Evas_Object *popup = brui_popup_add(m_window);
	if (!popup) {
		BROWSER_LOGE("popup is null");
		free(request_msg);
		return EINA_FALSE;
	}
	Evas_Object *popup_layout = elm_layout_add(popup);
	if (!popup_layout) {
		BROWSER_LOGE("Failed to elm_layout_add");
		free(request_msg);
		return EINA_FALSE;
	}

	elm_layout_file_set(popup_layout, browser_edj_dir"/browser-popup-lite.edj", "popup_checkview_layout");
	evas_object_size_hint_weight_set(popup_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	m_never_ask_check_box = elm_check_add(popup);
	if (!m_never_ask_check_box) {
		BROWSER_LOGE("Failed to elm_check_add");
		free(request_msg);
		return EINA_FALSE;
	}
	elm_object_style_set(m_never_ask_check_box, "popup");
	elm_object_domain_translatable_text_set(m_never_ask_check_box, BROWSER_DOMAIN_NAME, "IDS_BR_POP_DO_NOT_SHOW_AGAIN");
	elm_check_state_set(m_never_ask_check_box, EINA_FALSE);
	evas_object_size_hint_align_set(m_never_ask_check_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(m_never_ask_check_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(popup_layout, "elm.swallow.end", m_never_ask_check_box);
	elm_object_part_text_set(popup, "title,text", uri);
	Evas_Object *label = elm_label_add(popup_layout);
	if (!label) {
		BROWSER_LOGE("Failed to elm_label_add");
		free(request_msg);
		return EINA_FALSE;
	}
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label, request_msg);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(label);
	elm_object_part_content_set(popup_layout, "elm.swallow.content", label);
	elm_object_content_set(popup, popup_layout);

	memset(&m_ask_popup_callback_data, 0x00, sizeof(web_noti_popup_ask_callback_data));
	m_ask_popup_callback_data.user_data = (void *)this;
	m_ask_popup_callback_data.permission_request = permission_request;

	evas_object_smart_callback_add(label, "language,changed", __web_noti_ask_popup_language_changed, this);
	ewk_notification_permission_request_suspend(permission_request);

	m_browser->get_web_noti_manager()->show_content_popup(popup, NULL,
														popup_layout,
														__notification_permission_destroy_cb,
														"IDS_BR_SK_CANCEL",
														__notification_permission_request_cancel_cb,
														"IDS_BR_BUTTON_ALLOW",
														__notification_permission_request_allow_cb,
														&m_ask_popup_callback_data);
	free (request_msg);
	request_msg = NULL;

	return EINA_TRUE;
}

void web_notification_manager::_clear_permission_origin_data(void)
{
	unsigned int count = m_permission_origin_list.size();
	for (unsigned int i = 0; i < count; i++) {
		if (m_permission_origin_list[i] != NULL)
			free(m_permission_origin_list[i]);
		m_permission_origin_list[i] = NULL;
	}

	m_permission_origin_list.clear();
}

void web_notification_manager::__notification_permission_destroy_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("data[%p]");
	RET_MSG_IF(!data, "data is NULL");

	web_noti_popup_ask_callback_data *callback_data = (web_noti_popup_ask_callback_data *)data;
	Ewk_Notification_Permission_Request *permission_request = callback_data->permission_request;

	ewk_notification_permission_reply(permission_request, EINA_FALSE);
}

void web_notification_manager::__notification_permission_request_allow_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	web_noti_popup_ask_callback_data *callback_data = (web_noti_popup_ask_callback_data *)data;
	web_notification_manager *wnm = (web_notification_manager *)callback_data->user_data;
	Ewk_Notification_Permission_Request *permission_request = callback_data->permission_request;
	const char *origin = ewk_security_origin_host_get(ewk_notification_permission_request_origin_get(permission_request));

	Eina_Bool checkbox_state = elm_check_state_get(wnm->m_never_ask_check_box);
	if (checkbox_state == EINA_TRUE) {
		if (origin && strlen(origin))
			wnm->save_web_notification(origin, true);
	}

	ewk_notification_permission_reply(permission_request, EINA_TRUE);
}

void web_notification_manager::__notification_permission_request_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("data[%p]");
	RET_MSG_IF(!data, "data is NULL");

	web_noti_popup_ask_callback_data *callback_data = (web_noti_popup_ask_callback_data *)data;
	web_notification_manager *wnm = (web_notification_manager *)callback_data->user_data;
	Ewk_Notification_Permission_Request *permission_request = callback_data->permission_request;
	const char *origin = ewk_security_origin_host_get(ewk_notification_permission_request_origin_get(permission_request));

	Eina_Bool checkbox_state = elm_check_state_get(wnm->m_never_ask_check_box);
	if (checkbox_state == EINA_TRUE) {
		if (origin && strlen(origin))
			wnm->save_web_notification(origin, false);
	}

	ewk_notification_permission_reply(permission_request, EINA_FALSE);
}

void web_notification_manager::__web_noti_ask_popup_language_changed(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	web_noti_popup_ask_callback_data *callback_data = (web_noti_popup_ask_callback_data *)data;
	Ewk_Notification_Permission_Request *permission_request = callback_data->permission_request;
	const char *origin = ewk_security_origin_host_get(ewk_notification_permission_request_origin_get(permission_request));
	RET_MSG_IF(origin == NULL || strlen(origin) == 0, "origin is NULL");

	char *request_msg = NULL;
	unsigned short prefix_len = strlen(BR_STRING_REQUEST_WEBPAGE_PREFIX);
	unsigned short origin_len = strlen(origin);
	unsigned short msg_len = strlen(BR_STRING_WEB_NOTI_PERMISSION_POPUP_DESC);

	request_msg = (char *)malloc((prefix_len + origin_len + msg_len + 1) * sizeof(char));
	if (!request_msg) {
		BROWSER_LOGE("Failed to allocate memory for request_msg");
		return;
	}
	memset(request_msg, 0, (prefix_len + origin_len + msg_len + 1));
	if (!strncmp(origin, http_scheme, strlen(http_scheme)) && strlen(origin) > strlen(http_scheme))
		snprintf(request_msg, (prefix_len + msg_len + origin_len), BR_STRING_WEB_NOTI_PERMISSION_POPUP_DESC, BR_STRING_REQUEST_WEBPAGE_PREFIX, origin + strlen(http_scheme));
	else if (!strncmp(origin, https_scheme, strlen(https_scheme)) && strlen(origin) > strlen(https_scheme))
		snprintf(request_msg, (prefix_len + msg_len + origin_len), BR_STRING_WEB_NOTI_PERMISSION_POPUP_DESC, BR_STRING_REQUEST_WEBPAGE_PREFIX, origin + strlen(https_scheme));
	else
		snprintf(request_msg, (prefix_len + msg_len + origin_len), BR_STRING_WEB_NOTI_PERMISSION_POPUP_DESC, BR_STRING_REQUEST_WEBPAGE_PREFIX, origin);

	elm_object_text_set(obj, request_msg);

	free(request_msg);
	request_msg = NULL;
}
