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

 #include "webview_info.h"
 #include "browser-dlog.h"
 #include "platform-service.h"

webview_info:: webview_info(webview *wv, Eina_Bool is_activated)
	: m_uri(NULL)
	, m_title(NULL)
	, m_favicon(NULL)
	, m_snapshot(NULL)
	, m_sync_id(-1)
	, m_url_for_bookmark_update(NULL)
	, m_is_incognito(EINA_FALSE)
	, m_is_user_created(EINA_TRUE)
	, m_is_activated(is_activated)
	, m_webview(wv)
{
	BROWSER_LOGD("");

	const char *uri = wv->get_uri();
	if (uri)
		m_uri = eina_stringshare_add(uri);
	else if (wv->get_init_url())
		m_uri = eina_stringshare_add(wv->get_init_url());

	const char *title = wv->get_title();
	if (title)
		m_title = eina_stringshare_add(title);

	Evas_Object *favicon = wv->get_favicon();
	if (favicon) {
		platform_service ps;
		m_favicon = ps.copy_evas_image(favicon);

		//Free the webkit favicon data
		SAFE_FREE_OBJ(favicon);
	}

	m_sync_id = wv->sync_id_get();

	const char *url_for_bookmark_update = wv->get_url_for_bookmark_update();
	if (url_for_bookmark_update)
		m_url_for_bookmark_update = eina_stringshare_add(url_for_bookmark_update);

	m_is_incognito = wv->private_browsing_enabled_get();
	m_is_user_created = wv->is_user_created();

	BROWSER_LOGD("webview_info : End");
}

webview_info::~webview_info()
{
	BROWSER_LOGD("");

	if (m_title)
		eina_stringshare_del(m_title);

	if (m_uri)
		eina_stringshare_del(m_uri);

	SAFE_FREE_OBJ(m_snapshot);

	SAFE_FREE_OBJ(m_favicon);

	if (m_url_for_bookmark_update)
		eina_stringshare_del(m_url_for_bookmark_update);
}
