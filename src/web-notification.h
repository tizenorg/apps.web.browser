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

#ifndef WEB_NOTIFICATION_H
#define WEB_NOTIFICATION_H

#include <libsoup/soup.h>
#include <notification.h>
#include <vector>

#include "browser.h"
#include "browser-object.h"
#include "common-view.h"
#include "preference.h"
#include "webview.h"

typedef struct _web_noti_permission_origin_data {
	const Ewk_Security_Origin *permission_origin;
	webview *parent_webview;
} web_noti_permission_origin_data;

typedef struct _web_noti_popup_ask_callback_data {
	web_notification_enable_type type;
	Ewk_Notification_Permission_Request *permission_request;
	Elm_Object_Item *it;
	void *user_data;
} web_noti_popup_ask_callback_data;

class web_notification_setting_ask_popup : public common_view {
public:
	web_notification_setting_ask_popup(void);
	~web_notification_setting_ask_popup(void);

	Eina_Bool show(void);
private:
	Evas_Object *_create_genlist(Evas_Object *parent);

	void _destroy_genlist_callback_datas(void);

	static char *__genlist_text_get(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_contents_get(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	static void __radio_icon_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ok_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_realized_cb(void *data, Evas_Object *obj, void *event_info);

	std::vector<Elm_Genlist_Item_Class *> m_genlist_item_class_list;
	std::vector<web_noti_popup_ask_callback_data *> m_genlist_callback_data_list;

	web_notification_enable_type m_selected_type;

	Evas_Object *m_radio_main;
	Evas_Object *m_popup;
	Elm_Object_Item *m_popup_last_item;

protected:
};

class webview;
class web_notification_item : public common_view {
public:
	web_notification_item(webview *wv, Ewk_Notification *ewk_web_noti);
	~web_notification_item(void);

	friend bool operator==(web_notification_item item1, web_notification_item item2) {
		return ((item1.m_origin && item1.m_title && item1.m_body)
				&& (item2.m_origin && item2.m_title && item2.m_body)
				&& (!strcmp(item1.m_origin, item2.m_origin))
				&& (!strcmp(item1.m_title, item2.m_title))
				&& (!strcmp(item1.m_body, item2.m_body)));
	}

	Eina_Bool make_notification(void);
	Eina_Bool webview_validate_check(void);
	Eina_Bool deactivate_item(void);
	void delete_notification(void);
	const char *get_origin(void) { return m_origin; }
	const char *get_title(void) { return m_title; }
	const char *get_body(void) { return m_body; }
	int get_ewk_noti_id(void) { return m_ewk_noti_id; }
	int get_noti_id(void) { return m_noti_id; }
	webview *get_webview(void) { return m_webview; }

private:
	void _init(void);
	Eina_Bool _create_notification(Eina_Bool has_icon);
	Eina_Bool _download_notification_icon_start(void);
	Eina_Bool _save_notification_icon(SoupBuffer *body);
	void _remove_icon_download_timer(void);

	static void __download_notification_icon_finished_cb(SoupSession *session, SoupMessage *msg, gpointer data);
	static Eina_Bool __icon_download_timer_expired_cb(void *data);

	Ewk_Notification *m_ewk_noti;
	webview *m_webview;
	const char *m_origin;
	const char *m_title;
	const char *m_body;
	const char *m_icon_uri;
	const char *m_icon_path;
	int m_ewk_noti_id;
	int m_noti_id;
	double m_time_stamp; /* time stamped for unique */
	notification_h m_noti_handle;
	Ecore_Timer *m_icon_download_timer;
};


class web_notification_manager : public common_view {
public:
	web_notification_manager(void);
	~web_notification_manager(void);
	Eina_Bool handle_permission_request(webview *parent_webview, Ewk_Notification_Permission_Request *permission_request);
	Eina_Bool set_web_notification(webview *wv, Ewk_Notification *ewk_web_noti);
	Eina_Bool unset_web_notification(int ewk_noti_id);
	Eina_Bool web_noti_selected(int ewk_noti_id);
	Eina_Bool save_web_notification(const char *uri, bool accept, const char *title = NULL);
	Eina_Bool get_web_notification_allow(const char *uri);
	Eina_Bool delete_saved_origins(const char *uri);
	Eina_Bool delete_all_saved_origins(void);
	Eina_Bool delete_all_notifications(void);
	int get_web_noti_count(void);
	void reset_policy(void);

	web_notification_setting_ask_popup *get_setting_ask_popup(void);
	void delete_web_notification_setting_ask_popup(void);

private:
	Eina_Bool _show_permission_request_popup(const char *uri, Ewk_Notification_Permission_Request *permission_request);
	void _clear_permission_origin_data(void);

	static void __notification_permission_destroy_cb(void *data, Evas_Object *obj, void *event_info);
	static void __notification_permission_request_allow_cb(void *data, Evas_Object *obj, void *event_info);
	static void __notification_permission_request_cancel_cb(void *data, Evas_Object *obj, void *event_info);
	static void __web_noti_ask_popup_language_changed(void *data, Evas_Object *obj, void *event_info);

	web_notification_setting_ask_popup *m_web_notification_setting_ask_popup;

	std::vector<web_notification_item *> m_web_noti_item_list;
	std::vector<web_noti_permission_origin_data *> m_permission_origin_list;
	Evas_Object *m_never_ask_check_box;
	web_noti_popup_ask_callback_data m_ask_popup_callback_data;
};

#endif /* WEB_NOTIFICATION_H */
