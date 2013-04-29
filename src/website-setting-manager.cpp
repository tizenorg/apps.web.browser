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

#include "website-setting-manager.h"

#include "browser.h"
#include "browser-dlog.h"
#include "geolocation-item.h"
#include "setting-view.h"
#include "webview.h"

website_setting_manager::website_setting_manager(void)
{
	BROWSER_LOGD("");
}

website_setting_manager::~website_setting_manager(void)
{
	BROWSER_LOGD("");

	delete_all_geolocation_data();
	delete_all_web_storate_data();
}

std::vector<website_setting_item *> website_setting_manager::get_website_setting_list(void)
{
	BROWSER_LOGD("");

	if (m_website_setting_list.size() != 0)
		return m_website_setting_list;

	/* Make lists for web storages */
	Ewk_Context *context = ewk_context_default_get();
	ewk_context_application_cache_origins_get(context, __application_cache_origin_get_cb, this);
	ewk_context_web_storage_origins_get(context, __web_storage_origin_get_cb, this);
	ewk_context_web_database_origins_get(context, __web_database_origin_get_cb, this);

	/* Make lists for geolocations */
	m_geolocation_list = m_browser->get_geolocation_manager()->get_geolocation_list();

	/* make entire website setting list */
	std::vector<website_setting_item *> website_setting_list;

	for (int i = 0 ; i < m_geolocation_list.size() ; i++) {
		website_setting_item *item = new website_setting_item(m_geolocation_list[i]->get_uri());
		item->set_has_geolocation_data(EINA_TRUE);
		item->set_geolocation_allow(m_geolocation_list[i]->get_allow());
		item->set_title(m_geolocation_list[i]->get_title());
		item->set_date(m_geolocation_list[i]->get_date());

		website_setting_list.push_back(item);
	}
	m_website_setting_list = website_setting_list;

	return website_setting_list;
}

Eina_Bool website_setting_manager::delete_all_website_settings(void)
{
	BROWSER_LOGD("");
	m_browser->get_geolocation_manager()->delete_all();

	Ewk_Context *ewk_context = ewk_context_default_get();
	ewk_context_cache_clear(ewk_context);
	ewk_context_web_indexed_database_delete_all(ewk_context);
	ewk_context_application_cache_delete_all(ewk_context);
	ewk_context_web_storage_delete_all(ewk_context);
	ewk_context_web_database_delete_all(ewk_context);

	return EINA_TRUE;
}

Eina_Bool website_setting_manager::delete_all_geolocation_data(void)
{
	BROWSER_LOGD("");

	for (int i = 0; i < m_geolocation_list.size(); i++) {
		if (m_geolocation_list[i])
			delete m_geolocation_list[i];
	}
	m_geolocation_list.clear();

	return EINA_TRUE;
}

Eina_Bool website_setting_manager::delete_all_web_storate_data(void)
{
	BROWSER_LOGD("");

	for (int i = 0; i < m_website_setting_list.size(); i++) {
		if (m_website_setting_list[i])
			delete m_website_setting_list[i];
	}
	m_website_setting_list.clear();

	return EINA_TRUE;
}

void website_setting_manager::__application_cache_origin_get_cb(Eina_List* origins, void* user_data)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(origins);
	EINA_SAFETY_ON_NULL_RETURN(user_data);

	website_setting_manager *wsm = (website_setting_manager *)user_data;
	website_setting_view *wsv = m_browser->get_setting_view()->get_website_setting_view();

	Eina_List *list = NULL;
	void *list_data = NULL;

	EINA_LIST_FOREACH(origins, list, list_data) {
		if (list_data) {
			Ewk_Security_Origin *origin = (Ewk_Security_Origin *)list_data;;
			Eina_Bool duplicated = EINA_FALSE;
			const char *host_uri = ewk_security_origin_host_get(origin);
			for (int i = 0 ; i < wsm->m_website_setting_list.size() ; i++) {
				const char *stored_uri = wsm->m_website_setting_list[i]->get_uri();
				if (stored_uri && host_uri && !strcmp(stored_uri, host_uri)) {
					duplicated = EINA_TRUE;
					wsm->m_website_setting_list[i]->set_has_web_storage_data(EINA_TRUE);
					wsm->m_website_setting_list[i]->set_origin(origin);
					elm_genlist_item_update((Elm_Object_Item *)(wsm->m_website_setting_list[i]->get_user_data()));
					break;
				}
			}

			if (duplicated == EINA_FALSE) {
				website_setting_item *item = new website_setting_item(host_uri);
				item->set_has_web_storage_data(EINA_TRUE);
				item->set_origin(origin);

				wsv->add_website_setting_item_in_genlist(item);
				wsm->m_website_setting_list.push_back(item);
			}
		}
	}

	if ((wsm->get_website_setting_list()).size() > 0)
		wsv->delete_all_button_disabled_set(EINA_FALSE);
	else
		wsv->delete_all_button_disabled_set(EINA_TRUE);
}

void website_setting_manager::__web_storage_origin_get_cb(Eina_List* origins, void* user_data)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(origins);
	EINA_SAFETY_ON_NULL_RETURN(user_data);

	website_setting_manager *wsm = (website_setting_manager *)user_data;
	website_setting_view *wsv = m_browser->get_setting_view()->get_website_setting_view();

	Eina_List *list = NULL;
	void *list_data = NULL;

	EINA_LIST_FOREACH(origins, list, list_data) {
		if (list_data) {
			Ewk_Security_Origin *origin = (Ewk_Security_Origin *)list_data;;
			Eina_Bool duplicated = EINA_FALSE;
			const char *host_uri = ewk_security_origin_host_get(origin);
			for (int i = 0 ; i < wsm->m_website_setting_list.size() ; i++) {
				const char *stored_uri = wsm->m_website_setting_list[i]->get_uri();
				if (stored_uri && host_uri && !strcmp(stored_uri, host_uri)) {
					duplicated = EINA_TRUE;
					wsm->m_website_setting_list[i]->set_has_web_storage_data(EINA_TRUE);
					wsm->m_website_setting_list[i]->set_origin(origin);
					elm_genlist_item_update((Elm_Object_Item *)(wsm->m_website_setting_list[i]->get_user_data()));
					break;
				}
			}

			if (duplicated == EINA_FALSE) {
				website_setting_item *item = new website_setting_item(host_uri);
				item->set_has_web_storage_data(EINA_TRUE);
				item->set_origin(origin);

				wsv->add_website_setting_item_in_genlist(item);
				wsm->m_website_setting_list.push_back(item);
			}
		}
	}

	if ((wsm->get_website_setting_list()).size() > 0)
		wsv->delete_all_button_disabled_set(EINA_FALSE);
	else
		wsv->delete_all_button_disabled_set(EINA_TRUE);
}

void website_setting_manager::__web_database_origin_get_cb(Eina_List* origins, void* user_data)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(origins);
	EINA_SAFETY_ON_NULL_RETURN(user_data);

	website_setting_manager *wsm = (website_setting_manager *)user_data;
	website_setting_view *wsv = m_browser->get_setting_view()->get_website_setting_view();

	Eina_List *list = NULL;
	void *list_data = NULL;

	EINA_LIST_FOREACH(origins, list, list_data) {
		if (list_data) {
			Ewk_Security_Origin *origin = (Ewk_Security_Origin *)list_data;;
			Eina_Bool duplicated = EINA_FALSE;
			const char *host_uri = ewk_security_origin_host_get(origin);
			for (int i = 0 ; i < wsm->m_website_setting_list.size() ; i++) {
				const char *stored_uri = wsm->m_website_setting_list[i]->get_uri();
				if (stored_uri && host_uri && !strcmp(stored_uri, host_uri)) {
					duplicated = EINA_TRUE;
					wsm->m_website_setting_list[i]->set_has_web_storage_data(EINA_TRUE);
					wsm->m_website_setting_list[i]->set_origin(origin);
					elm_genlist_item_update((Elm_Object_Item *)(wsm->m_website_setting_list[i]->get_user_data()));
					break;
				}
			}

			if (duplicated == EINA_FALSE) {
				website_setting_item *item = new website_setting_item(host_uri);
				item->set_has_web_storage_data(EINA_TRUE);
				item->set_origin(origin);

				wsv->add_website_setting_item_in_genlist(item);
				wsm->m_website_setting_list.push_back(item);
			}
		}
	}

	if ((wsm->get_website_setting_list()).size() > 0)
		wsv->delete_all_button_disabled_set(EINA_FALSE);
	else
		wsv->delete_all_button_disabled_set(EINA_TRUE);

}
