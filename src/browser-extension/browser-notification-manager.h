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

#ifndef BROWSER_NOTIFICATION_MANAGER_H
#define BROWSER_NOTIFICATION_MANAGER_H

#include "browser-config.h"

#include <notification.h>

using namespace std;

class Browser_Notification_DB;
class Browser_Notification_Manager {
public:
	Browser_Notification_Manager(void);
	~Browser_Notification_Manager(void);

	void init(Evas_Object *webview);
	void deinit(void);
private:
	Eina_Bool _register_notification(Ewk_Notification *ewk_notification);
	void _download_icon(int noti_id, const char *icon_url);
	Eina_Bool _print_grouping_list(void);
	Eina_Bool _finalize_download_icon(int noti_id, Eina_Bool set_image);

	static void __http_finished_cb(SoupSession *session, SoupMessage *msg, gpointer data);
	static void __notification_changed_cb(void *data, notification_type_e type);
	static void __confirm_popup_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __confirm_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info);

	static void __notification_contents_show_cb(void *data,
					Evas_Object *obj, void *event_info);
	static void __notification_contents_cancel_cb(void *data,
					Evas_Object *obj, void *event_info);
	static void __notification_contents_request_permission_cb(void *data,
					Evas_Object *obj, void *event_info);

	Evas_Object *m_webview;
	Evas_Object *m_confirm_popup;
	Browser_Notification_DB *m_notification_db;
	Eina_Bool m_is_notification_cb_registered;
	std::string m_domain_to_save;
	int m_noti_id;
};

#endif /* BROWSER_NOTIFICATION_MANAGER_H */

