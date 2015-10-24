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
 * Contact: Mahesh Domakuntla <mahesh.d@samsung.com>
 *
 */

#ifndef WEBVIEW_INFO_H
#define WEBVIEW_INFO_H

#include "webview.h"

class webview_info {
public:
	webview_info(webview *wv, Eina_Bool is_activated = EINA_FALSE);
	~webview_info();

	//For all getter methods: Caller should not free the returned memory
	const char *get_uri(void) { return m_uri; }
	const char *get_title(void) { return m_title; }
	Evas_Object *get_favicon(void) { return m_favicon; }
	Evas_Object *get_snapshot(void) { return m_snapshot; }

	int get_sync_id(void) { return m_sync_id; }
	const char* get_url_for_bookmark_update(void) { return m_url_for_bookmark_update; }

	Eina_Bool is_private_mode_enabled(void) {return m_is_incognito; }
	Eina_Bool is_user_created(void) { return m_is_user_created; }
	Eina_Bool is_activated(void) { return m_is_activated; }
	webview *get_webview(void) { return m_webview; }

private:
	const char *m_uri;
	const char *m_title;
	Evas_Object *m_favicon;
	Evas_Object *m_snapshot;

	int m_sync_id;
	const char *m_url_for_bookmark_update;

	Eina_Bool m_is_incognito;
	Eina_Bool m_is_user_created;
	Eina_Bool m_is_activated;

	webview *m_webview;
};

#endif /* WEBVIEW_INFO_H */


