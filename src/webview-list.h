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

#ifndef WEBVIEW_LIST_H
#define WEBVIEW_LIST_H

#include <Evas.h>
#include <vector>

#include "browser-object.h"
#define BROWSER_WINDOW_MAX_NEW_WINDOW	10
#define BROWSER_WINDOW_MAX_SIZE	10
#define BROWSER_WINDOW_MAX_ACTIVE_SIZE	2

class webview;
class webview_list : public browser_object {
public:
	webview_list(void);
	~webview_list(void);

#ifdef ENABLE_INCOGNITO_WINDOW
	webview *create_webview(Eina_Bool user_created = EINA_FALSE, Eina_Bool is_incognito = EINA_FALSE);
#else
	webview *create_webview(Eina_Bool user_created = EINA_FALSE);
#endif
	webview *create_renewed_webview(Eina_Bool user_created = EINA_FALSE);
	webview *create_cloud_webview(const char *title, const char *uri, Eina_Bool incognito, int sync_id, Eina_Bool create_ewk_view = EINA_FALSE);
	/* @ return - The previous webview of deleted window will be returned to be able to replace it. */
	webview *delete_webview(webview *wv);
	void unsync_webviews();
	void clean_up_webviews(void);
	webview* get_old_webview(void);
	void deactivate_old_webview(void);
	unsigned int get_count(void);
	unsigned int get_active_count(void);
	webview *get_webview(unsigned int index);
	int get_index(webview *wv);
	webview *get_webview(Evas_Object *ewk_view);
	Eina_Bool is_ewk_view_exist(Evas_Object *ewk_view);
	Eina_Bool is_webview_exist(webview *web_view);

private:
	std::vector<webview *> m_webview_list;
};

#endif /* WEBVIEW_LIST_H */
