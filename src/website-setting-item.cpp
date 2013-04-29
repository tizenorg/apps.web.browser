/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "website-setting-item.h"

#include <app.h>

#include "browser.h"
#include "browser-dlog.h"
#include "geolocation-manager.h"

website_setting_item::website_setting_item(const char *uri)
:
	m_title(NULL)
	,m_date(NULL)
	,m_has_web_storage_data(EINA_FALSE)
	,m_has_geolocation_data(EINA_FALSE)
	,m_geolocation_allow(false)
	,m_need_update(EINA_FALSE)
	,m_origin(NULL)
	,m_user_data(NULL)
	,m_item_view(NULL)
{
	BROWSER_LOGD("");

	if (uri)
		m_uri = eina_stringshare_add(uri);
}

website_setting_item::~website_setting_item(void)
{
	BROWSER_LOGD("");

	eina_stringshare_del(m_uri);
	eina_stringshare_del(m_title);
	eina_stringshare_del(m_date);

	if (m_item_view)
		delete m_item_view;
}

void website_setting_item::set_title(const char *title)
{
	BROWSER_LOGD("title = [%s]", title);
	EINA_SAFETY_ON_NULL_RETURN(title);

	if (m_title)
		eina_stringshare_replace(&m_title, title);
	else
		m_title = eina_stringshare_add(title);
}

void website_setting_item::set_date(const char *date)
{
	BROWSER_LOGD("date = [%s]", date);
	EINA_SAFETY_ON_NULL_RETURN(date);

	if (m_date)
		eina_stringshare_replace(&m_date, date);
	else
		m_title = eina_stringshare_add(date);
}

void website_setting_item::show(void)
{
	if (!m_item_view)
		m_item_view = new website_setting_item_view(this);

	if (m_item_view)
		m_item_view->show();
	else {
		BROWSER_LOGD("Failed to allocate memory for website_setting_item_view");
		return;
	}
}

Eina_Bool website_setting_item::delete_geolocation_item(const char *uri)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	geolocation_manager *gm = m_browser->get_geolocation_manager();
	set_has_geolocation_data(EINA_FALSE);
	set_geolocation_allow(false);

	return gm->delete_geolocation_setting(uri);
}

Eina_Bool website_setting_item::delete_web_storage_item(Ewk_Security_Origin *origin)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(origin, EINA_FALSE);

	/* Clear cache */
	Ewk_Context *ewk_context = ewk_context_default_get();
	ewk_context_application_cache_delete(ewk_context, origin);
	ewk_context_web_database_delete(ewk_context, origin);
	ewk_context_web_storage_origin_delete(ewk_context, origin);
	set_has_web_storage_data(EINA_FALSE);

	return EINA_TRUE;
}

