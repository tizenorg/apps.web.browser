/*
  * Copyright 2012  Samsung Electronics Co., Ltd
  *
  * Licensed under the Flora License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *    http://www.tizenopensource.org/license
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */

#include "browser-notification-db.h"
#include "browser-notification-manager.h"

#include <libsoup/soup.h>
#include <fcntl.h>

Browser_Notification_Manager::Browser_Notification_Manager(void)
:	m_webview(NULL)
	,m_notification_db(NULL)
	,m_confirm_popup(NULL)
	,m_is_notification_cb_registered(EINA_FALSE)
	,m_noti_id(0)
{
	BROWSER_LOGD("[%s]", __func__);
	m_notification_db = new(nothrow) Browser_Notification_DB;
	if (!m_notification_db)
		BROWSER_LOGE("new(nothrow) Browser_Notification_DB failed");
}

Browser_Notification_Manager::~Browser_Notification_Manager(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_confirm_popup)
		evas_object_del(m_confirm_popup);
	if (m_notification_db)
		delete m_notification_db;
}

void Browser_Notification_Manager::init(Evas_Object *webview)
{
	BROWSER_LOGD("[%s]", __func__);
	deinit();

	m_webview = webview;

	Evas_Object *webkit = elm_webview_webkit_get(m_webview);
	evas_object_smart_callback_add(webkit, "notification,contents,show",
					__notification_contents_show_cb, this);
	evas_object_smart_callback_add(webkit, "notification,contents,cancel",
					__notification_contents_cancel_cb, this);
	evas_object_smart_callback_add(webkit, "notification,contents,requestPermission",
					__notification_contents_request_permission_cb, this);
}

void Browser_Notification_Manager::deinit(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!m_webview)
		return;

	m_is_notification_cb_registered = EINA_FALSE;

	Evas_Object *webkit = elm_webview_webkit_get(m_webview);
	evas_object_smart_callback_del(webkit, "notification,contents,show",
						__notification_contents_show_cb);
	evas_object_smart_callback_del(webkit, "notification,contents,cancel",
						__notification_contents_cancel_cb);
	evas_object_smart_callback_del(webkit, "notification,contents,requestPermission",
						__notification_contents_request_permission_cb);
}


Eina_Bool Browser_Notification_Manager::_register_notification(Ewk_Notification *ewk_notification)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!m_notification_db) {
		BROWSER_LOGE("m_notification_db is null");
		return EINA_FALSE;
	}

	int notification_id = -1;
	if (!m_notification_db->save_notification(ewk_notification, notification_id))
		BROWSER_LOGE("save_notification failed");
	if (notification_id < 0) {
		BROWSER_LOGE("invalid notification id");
		return EINA_FALSE;
	}

	_download_icon(notification_id, ewk_notification->iconURL);

	return EINA_TRUE;
}

void Browser_Notification_Manager::_download_icon(int noti_id, const char *icon_url)
{
	BROWSER_LOGD("[%s]", __func__);
	SoupSession *soup_session = NULL;
	SoupMessage *soup_msg = NULL;
	SoupMessageHeaders *headers = NULL;

	soup_session = soup_session_async_new();
	g_object_set(soup_session, SOUP_SESSION_TIMEOUT, 15, NULL);
	soup_msg = soup_message_new("GET", icon_url);
	headers = soup_msg->request_headers;

/* It seems that the user agent setting code is unncessary. */
/*
	soup_message_headers_append(headers, "User-Agent",
		"Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) AppleWebKit/525.13 (KHTML, like Gecko) Chrome/0.2.149.27 Safari/525.13");
*/
	m_noti_id = noti_id;
	soup_session_queue_message(soup_session, soup_msg, __http_finished_cb, (void *)this);
}

void Browser_Notification_Manager::__http_finished_cb(SoupSession *session,
						SoupMessage *msg, gpointer data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Notification_Manager *notification_manager = NULL;
	notification_manager = (Browser_Notification_Manager *)data;

	SoupBuffer *body = soup_message_body_flatten(msg->response_body);

	int noti_id = notification_manager->m_noti_id;

	int fd;
	int write_len = 0;
	char icon_path[256] = {0, };
	sprintf(icon_path, WEBKIT_SOUP_CACHE_DIR"%d.ico", noti_id);
	unlink(icon_path);

	if (!body->data || body->length <= 0) {
		soup_buffer_free(body);
		if (!notification_manager->_finalize_download_icon(noti_id, EINA_FALSE))
			BROWSER_LOGE("_finalize_download_icon failed");
		return;
	}
	if ((fd = open(icon_path, O_CREAT | O_WRONLY, S_IRUSR|S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		soup_buffer_free(body);
		if (!notification_manager->_finalize_download_icon(noti_id, EINA_FALSE))
			BROWSER_LOGE("_finalize_download_icon failed");
		return;
	}

	write_len = write(fd, body->data, body->length);
	close(fd);

	soup_buffer_free(body);

	if (write_len != body->length) {
		unlink(icon_path);
		if (!notification_manager->_finalize_download_icon(noti_id, EINA_FALSE))
			BROWSER_LOGE("_finalize_download_icon failed");
		return;
	}

	if (!notification_manager->_finalize_download_icon(noti_id, EINA_TRUE))
		BROWSER_LOGE("_finalize_download_icon failed");
}

Eina_Bool Browser_Notification_Manager::_print_grouping_list(void)
{
	BROWSER_LOGD("[%s]", __func__);
	notification_list_h noti_list = NULL;
	notification_list_h noti_list_head = NULL;
	notification_list_h noti_detail_list = NULL;
	notification_list_h noti_detail_list_head = NULL;
	notification_h noti = NULL;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;
	char *caller_pkgname = NULL;
	char *title = NULL;
	int group_id = -1;
	int priv_id = -1;
	int count = 0;

	noti_err = notification_get_grouping_list(NOTIFICATION_TYPE_NOTI, -1, &noti_list);
	if (noti_err != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGD("Fail to notification_get_grouping_list : %d\n", noti_err);
		return EINA_FALSE;
	}

	noti_list_head = noti_list;

	while (noti_list != NULL) {
		noti = notification_list_get_data(noti_list);
		BROWSER_LOGD("================================================================\n");
		notification_get_pkgname(noti, &caller_pkgname);
		notification_get_id(noti, &group_id, &priv_id);
		notification_get_count(NOTIFICATION_TYPE_NOTI, caller_pkgname, group_id, priv_id, &count);
		BROWSER_LOGD("caller pkgname : %s, group_id : %d, priv_id : %d, count : %d\n",
							caller_pkgname, group_id, priv_id, count);
		notification_get_detail_list(caller_pkgname, group_id, priv_id, -1, &noti_detail_list);
		noti_detail_list_head = noti_detail_list;
		int i = 0;
		while (noti_detail_list != NULL) {
			i++;
			noti = notification_list_get_data(noti_detail_list);

			notification_get_title(noti, &title, NULL);
			notification_get_id(noti, &group_id, &priv_id);
			BROWSER_LOGD("\t%d) %s (G:%d, P:%d)\n", i, title, group_id, priv_id);

			noti_detail_list = notification_list_get_next(noti_detail_list);
		}
		notification_free_list(noti_detail_list_head);
		noti_list = notification_list_get_next(noti_list);
	}

	notification_free_list(noti_list_head);

	return EINA_TRUE;
}

Eina_Bool Browser_Notification_Manager::_finalize_download_icon(int noti_id, Eina_Bool set_image)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!m_notification_db) {
		BROWSER_LOGE("m_notification_db is null");
		return EINA_FALSE;
	}

	notification_h noti_h = NULL;
	notification_error_e error = NOTIFICATION_ERROR_NONE;
	bundle *b = NULL;
	int priv_id = 0;

	if (!_print_grouping_list())
		BROWSER_LOGE("_print_grouping_list failed");

	noti_h = notification_new(NOTIFICATION_TYPE_NOTI,
					NOTIFICATION_GROUP_ID_NONE, noti_id);
	if (!noti_h) {
		BROWSER_LOGE("notification_new failed");
		return EINA_FALSE;
	}

	std::string title;
	if (!m_notification_db->get_title_by_id(noti_id, title)) {
		BROWSER_LOGE("get_title_by_id failed");
		return EINA_FALSE;
	}
	if (title.empty()) {
		BROWSER_LOGE("title is empty");
		return EINA_FALSE;
	}

	BROWSER_LOGD("title=[%s]", title.c_str());
	error = notification_set_text(noti_h, NOTIFICATION_TEXT_TYPE_TITLE, title.c_str(), NULL,
						NOTIFICATION_VARIABLE_TYPE_NONE);
	if (error != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGD("Fail to set title [%d]\n", error);
		notification_free(noti_h);
		return EINA_FALSE;
	}

	std::string msg_body;
	if (!m_notification_db->get_body_by_id(noti_id, msg_body))  {
		BROWSER_LOGD("get_body_by_id failed\n");
		return EINA_FALSE;
	}
	if (msg_body.empty()) {
		BROWSER_LOGE("msg_body is empty");
		return EINA_FALSE;
	}
	BROWSER_LOGD("msg_body=[%s]", msg_body.c_str());
	error = notification_set_text(noti_h, NOTIFICATION_TEXT_TYPE_CONTENT, msg_body.c_str(),
							NULL, NOTIFICATION_VARIABLE_TYPE_NONE);
	if (error != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGD("Fail to set content. [%d]\n", error);
		notification_free(noti_h);
		return EINA_FALSE;
	}

	char icon_path[512] = {0, };
	if (set_image) {
		sprintf(icon_path, WEBKIT_SOUP_CACHE_DIR"%d.ico", noti_id);
		error = notification_set_image(noti_h, NOTIFICATION_IMAGE_TYPE_ICON, icon_path);
		if (error != NOTIFICATION_ERROR_NONE) {
			BROWSER_LOGD("Fail to set icon. [%d]\n", error);
			return EINA_FALSE;
		}
	}

	error = notification_insert(noti_h, &priv_id);
	if (error != NOTIFICATION_ERROR_NONE) {
		BROWSER_LOGD("Fail to insert [%d]\n", error);
		notification_free(noti_h);
		return EINA_FALSE;
	}
	BROWSER_LOGD("Notification inserted successfully: priv id [%d], noti_id [%d]\n", priv_id, noti_id);

	if (!_print_grouping_list())
		BROWSER_LOGE("_print_grouping_list failed");

	if (!m_is_notification_cb_registered)
		if (notification_resister_changed_cb(__notification_changed_cb, this)
		    == NOTIFICATION_ERROR_NONE)
			m_is_notification_cb_registered = EINA_TRUE;

	if (noti_h) {
		error = notification_free(noti_h);
		if (error != NOTIFICATION_ERROR_NONE)
			BROWSER_LOGD("Fail to free noti data [%d]\n", error);
		noti_h = NULL;
	}

	if (set_image) {
		if (!m_notification_db->update_icon_validity(noti_id))
			BROWSER_LOGE("update_icon_validity failed");
	}

	return EINA_TRUE;
}

void Browser_Notification_Manager::__notification_changed_cb(void *data, notification_type_e type)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Notification_Manager *notification_manager = (Browser_Notification_Manager *)data;

	if (type == NOTIFICATION_TYPE_NOTI)
		BROWSER_LOGD("notificationChangedCb called [type: NOTI]\n");
	else if (type == NOTIFICATION_TYPE_ONGOING)
		BROWSER_LOGD("notificationChangedCb called [type: ONGOING]\n");
	else if (type == NOTIFICATION_TYPE_NONE)
		BROWSER_LOGD("notificationChangedCb called [type: NONE]\n");
	else
		BROWSER_LOGD("notificationChangedCb called [type: unknown]\n");

	int count = -1;
	notification_error_e error = NOTIFICATION_ERROR_NONE;

	error = notification_get_count(NOTIFICATION_TYPE_NONE, NULL,
				NOTIFICATION_GROUP_ID_NONE, NOTIFICATION_PRIV_ID_NONE, &count);

	if (error == NOTIFICATION_ERROR_NONE)
		BROWSER_LOGD("Notification count : %d\n", count);
	else
		BROWSER_LOGD("notification_get_count error\n");

	if (count > 0)
		return;

	if (!notification_manager->m_notification_db->delete_notifications())
		BROWSER_LOGE("delete_notifications failed");

	if (notification_manager->m_is_notification_cb_registered) {
		if (notification_unresister_changed_cb(__notification_changed_cb) == NOTIFICATION_ERROR_NONE)
			notification_manager->m_is_notification_cb_registered = EINA_FALSE;
	}
}

void Browser_Notification_Manager::__notification_contents_show_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data || !event_info)
		return;

	Browser_Notification_Manager *notification_manager = NULL;
	notification_manager = (Browser_Notification_Manager *)data;
	Ewk_Notification *ewk_notification = (Ewk_Notification *)event_info;
	Evas_Object *webkit = elm_webview_webkit_get(notification_manager->m_webview);

	if (!ewk_notification->iconURL || !ewk_notification->title || !ewk_notification->body) {
		BROWSER_LOGE("ewk_notification member is null");
		return;
	}

	if (!notification_manager->_register_notification(ewk_notification))
		BROWSER_LOGE("_register_notification failed");

	ewk_view_notification_displayed(webkit, ewk_notification);
}

void Browser_Notification_Manager::__notification_contents_cancel_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
}

void Browser_Notification_Manager::__notification_contents_request_permission_cb(void *data,
							Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
        if (!data || !event_info)
                return;

	Browser_Notification_Manager *notification_manager = NULL;
	notification_manager = (Browser_Notification_Manager *)data;
	if (!notification_manager->m_notification_db) {
		BROWSER_LOGE("m_notification_db is null");
		return;
	}

	char *domain = (char *)event_info;
	BROWSER_LOGD("domain(%s)", domain);

	if (notification_manager->m_notification_db->has_domain(domain)) {
		Evas_Object *webkit = elm_webview_webkit_get(notification_manager->m_webview);
		ewk_view_notification_allowed_set(webkit, EINA_TRUE, domain);
		BROWSER_LOGD("notification_manager->m_notification_db->has_domain() returns TRUE");
		return;
	} else {
		BROWSER_LOGD("notification_manager->m_notification_db->has_domain() returns FALSE");
	}

	if (notification_manager->m_confirm_popup)
		evas_object_del(notification_manager->m_confirm_popup);
	notification_manager->m_confirm_popup = elm_popup_add(notification_manager->m_webview);
	if (!notification_manager->m_confirm_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return;
	}
	evas_object_size_hint_weight_set(notification_manager->m_confirm_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(notification_manager->m_confirm_popup, BR_STRING_PERMISSION_CHECK, "elm.text");
	elm_object_text_set(notification_manager->m_confirm_popup, BR_STRING_WEB_NOTIFICATION_Q);

	notification_manager->m_domain_to_save = std::string(domain);

	Evas_Object *ok_button = elm_button_add(notification_manager->m_confirm_popup);
	elm_object_text_set(ok_button, BR_STRING_YES);
	elm_object_part_content_set(notification_manager->m_confirm_popup, "button1", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __confirm_popup_response_cb, notification_manager);

	Evas_Object *cancel_button = elm_button_add(notification_manager->m_confirm_popup);
	elm_object_text_set(cancel_button, BR_STRING_NO);
	elm_object_part_content_set(notification_manager->m_confirm_popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __confirm_popup_cancel_cb, notification_manager);

	evas_object_show(notification_manager->m_confirm_popup);
}

void Browser_Notification_Manager::__confirm_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Notification_Manager *notification_manager = NULL;
	notification_manager = (Browser_Notification_Manager *)data;

	if (!notification_manager->m_notification_db->save_domain(notification_manager->m_domain_to_save.c_str()))
		BROWSER_LOGE("save_domain failed");

	Evas_Object *webkit = elm_webview_webkit_get(notification_manager->m_webview);
	ewk_view_notification_allowed_set(webkit, EINA_TRUE, notification_manager->m_domain_to_save.c_str());

	if (notification_manager->m_confirm_popup != NULL) {
		evas_object_del(notification_manager->m_confirm_popup);
		notification_manager->m_confirm_popup = NULL;
	}
}

void Browser_Notification_Manager::__confirm_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Notification_Manager *notification_manager = NULL;
	notification_manager = (Browser_Notification_Manager *)data;

	Evas_Object *webkit = elm_webview_webkit_get(notification_manager->m_webview);
	ewk_view_notification_allowed_set(webkit, EINA_FALSE, notification_manager->m_domain_to_save.c_str());

	if (notification_manager->m_confirm_popup != NULL) {
		evas_object_del(notification_manager->m_confirm_popup);
		notification_manager->m_confirm_popup = NULL;
	}
}

