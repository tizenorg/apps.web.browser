/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *
 */

#ifndef CLOUD_SYNC_MANAGER_H
#define CLOUD_SYNC_MANAGER_H

#include "browser-object.h"
#include "webview_info.h"

#include <vector>

class webview;

class tab_sync_info : public browser_object {
public:
	tab_sync_info(int index = 0, const char *title = NULL, const char *uri = NULL, Eina_Bool activate = EINA_FALSE, Eina_Bool incognito = EINA_FALSE, int tab_id = -1, const char *device_name = NULL, const char *device_id = NULL);
	~tab_sync_info(void);

	int get_index(void) { return m_index; }
	const char *get_title(void) { return m_title; }
	const char *get_uri(void) { return m_uri; }
	Eina_Bool is_activated(void) { return m_is_activated; }
	Eina_Bool is_incognito(void) { return m_is_incognito; }
	int get_tab_id(void) { return m_tab_id; }
	const char *get_device_name(void) { return m_device_name; }
	const char *get_device_id(void) { return m_device_id; }

private:
	int m_index;
	char *m_title;
	char *m_uri;
	Eina_Bool m_is_activated;
	Eina_Bool m_is_incognito;
	int m_tab_id;
	char *m_device_name;
	char *m_device_id;
};

class cloud_sync_manager : public browser_object {
public:
	cloud_sync_manager(void);
	~cloud_sync_manager(void);

	std::vector<tab_sync_info *> get_tab_sync_info_list(Eina_Bool my_device = EINA_TRUE);
	void sync_webview(webview_info *wvi);
	void unsync_tab(int id);
	void unsync_webview(webview *wv);
	void activate_webview(int tab_id);
	Evas_Object *get_tab_snapshot(int tab_id);
	void set_tab_snapshot(int tab_id, Evas_Object *snapshot);

private:
	void _set_tab_favicon(int tab_id, char *url);

	static void __set_tab_snapshot_thread(void *data, Ecore_Thread *th);
	static void __thread_end_cb(void *data, Ecore_Thread *th);

	const char *m_device_name;
	const char *m_device_id;
	Ecore_Thread *m_snapshot_thread;
	webview *m_working_webview;
};

#endif /* CLOUD_SYNC_MANAGER_H */

