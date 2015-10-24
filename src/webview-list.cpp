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

#include "webview-list.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-view.h"
#include "cloud-sync-manager.h"
#include "history.h"
#include "main-view.h"
#include "url-bar.h"
#include "webview.h"

using namespace std;

webview_list::webview_list(void)
{
	BROWSER_LOGD("");
}

webview_list::~webview_list(void)
{
	BROWSER_LOGD("");
	std::vector<webview *> wv_list;
	wv_list = m_webview_list;
	m_webview_list.clear();

	for (unsigned int i = 0; i < wv_list.size(); i++)
		delete wv_list[i];

	wv_list.clear();
}
#ifdef ENABLE_INCOGNITO_WINDOW
webview *webview_list::create_webview(Eina_Bool user_created, Eina_Bool is_incognito)
#else
webview *webview_list::create_webview(Eina_Bool user_created)
#endif
{
	BROWSER_LOGD("%d", user_created);

#ifdef ENABLE_INCOGNITO_WINDOW
	webview *wv = new webview(user_created, is_incognito);
#else
	webview *wv = new webview(user_created);
#endif
	std::vector<webview *>::iterator it = m_webview_list.begin();
	m_webview_list.insert(it, wv);

	main_view *mv = m_browser->get_main_view();
	mv->show_url_bar(wv->get_url_bar_shown());

	return wv;
}

webview *webview_list::create_renewed_webview(Eina_Bool user_created)
{
	BROWSER_LOGD("");
	if (get_count() >= BROWSER_WINDOW_MAX_SIZE) {
		webview *old_webview = get_old_webview();
		if (old_webview)
			delete_webview(old_webview);
	}

	return create_webview(user_created);
}

webview *webview_list::create_cloud_webview(const char *title, const char *uri, Eina_Bool incognito, int sync_id, Eina_Bool create_ewk_view)
{
	BROWSER_SECURE_LOGD("title=[%s], uri=[%s], incognito=[%d], sync_id=[%d], create_ewk_view=[%d]", title, uri, incognito, sync_id, create_ewk_view);
//	RETV_MSG_IF(!uri, NULL, "uri is NULL");
	webview *wv = new webview(title, uri, incognito, sync_id, create_ewk_view);
	m_webview_list.push_back(wv);

	return wv;
}

webview *webview_list::delete_webview(webview *wv)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!wv, NULL, "wv is NULL");

	int webview_count = m_webview_list.size();
	//if (webview_count <= 1)
		//return NULL;

	webview *return_wv = NULL;

	for (int i = 0 ; i < webview_count ; i++) {
		if (m_webview_list[i] == wv) {

			// If deleted webview is current webview, set the current webview of browser-view null.
			if (m_browser->get_browser_view()->get_current_webview() == m_webview_list[i])
				m_browser->get_browser_view()->set_current_webview(NULL);

			Eina_Bool is_user_created_wv = wv->is_user_created();

			if (!is_user_created_wv)
				return_wv = wv->get_parent_webview();

			BROWSER_LOGD("****** is_user_created_wv=[%d], return_wv=[%p]", is_user_created_wv, return_wv);
			m_browser->get_cloud_sync_manager()->unsync_webview(wv);

			if (m_webview_list.size() > 1) {
				if (!return_wv || !is_webview_exist(return_wv)) {
					if (i)
						return_wv = m_webview_list[i - 1];
					else
						return_wv = m_webview_list[i + 1];
				}
			}

			m_webview_list.erase(m_webview_list.begin() + i);

			// FIXME: make sure, webview marked for deletion will be no longer parenting for rest of the webviews.
			std::vector<webview*>::iterator view = m_webview_list.begin();
			for ( ; view != m_webview_list.end(); ) {
				if ((*view)->get_parent_webview() == wv)
					(*view)->set_parent_webview(NULL);
				++view;
			}

			delete wv;
			break;
		}
	}

	return return_wv;
}

void webview_list::unsync_webviews()
{
	BROWSER_LOGD("");

	for (unsigned int i = 0 ; i < m_webview_list.size(); i++) {
		m_browser->get_cloud_sync_manager()->unsync_webview(m_webview_list[i]);
	}
}

void webview_list::clean_up_webviews(void)
{
	BROWSER_LOGD("");
	browser_view *bv = m_browser->get_browser_view();
	RET_MSG_IF(!bv, "m_browser->get_browser_view() is NULL");

	webview *wv = m_browser->get_browser_view()->get_current_webview();
	for (unsigned int i = 0; i < m_webview_list.size(); i++) {
		if (m_webview_list[i] != wv) {
			m_webview_list[i]->get_certificate_manager()->
					set_cert_type(certificate_manager::NONE);
			m_webview_list[i]->delete_ewk_view();
			m_webview_list[i]->delete_all_idler();
		}
	}

	if (wv != NULL && wv->get_ewk_view() != NULL) {
		BROWSER_LOGD("create current webview again");
		const char *reload_uri = wv->get_uri();
		bv->set_current_webview(NULL);

		wv->get_certificate_manager()->set_cert_type(certificate_manager::NONE);
		wv->delete_ewk_view();
		wv->delete_all_idler();
		wv->load_low_mem_uri(reload_uri);
		bv->set_current_webview(wv);
	}
}

webview* webview_list::get_old_webview(void)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!m_browser->get_browser_view(),NULL, "m_browser->get_browser_view() is NULL");

	webview *current_webview = m_browser->get_browser_view()->get_current_webview();
	webview *old_webview = NULL;

	for (unsigned int i = 0; i < m_webview_list.size(); i++) {
		if (m_webview_list[i] == current_webview)
			continue;

		if (!old_webview || m_webview_list[i]->get_modified() < old_webview->get_modified())
			old_webview = m_webview_list[i];
	}
	return old_webview;
}

void webview_list::deactivate_old_webview(void)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!m_browser->get_browser_view(), "m_browser->get_browser_view() is NULL");

	webview *current_webview = m_browser->get_browser_view()->get_current_webview();
	webview *old_webview = NULL;

	for (unsigned int i = 0; i < m_webview_list.size(); i++) {
		if (m_webview_list[i] == current_webview)
			continue;

		if (m_webview_list[i] == current_webview->get_parent_webview())
			continue;

		if (!m_webview_list[i]->get_ewk_view())
			continue;

		if (!old_webview || m_webview_list[i]->get_modified() < old_webview->get_modified())
			old_webview = m_webview_list[i];
	}

	if (old_webview)
		old_webview->delete_ewk_view();
}

unsigned int webview_list::get_count(void)
{
	unsigned int count = (unsigned int)m_webview_list.size();
	//BROWSER_LOGD("webviews count = [%d]", count);

	return count;
}

unsigned int webview_list::get_active_count(void)
{
	unsigned int count = 0;
	for (unsigned int i = 0; i < m_webview_list.size(); i++)
		if (m_webview_list[i]->get_ewk_view())
			count++;

	return count;
}

webview *webview_list::get_webview(unsigned int index)
{
	//BROWSER_LOGD("");
	if (index >= get_count())
		return NULL;

	return m_webview_list[index];
}

int webview_list::get_index(webview *wv)
{
	for (unsigned int i = 0; i < m_webview_list.size(); i++) {
		if (m_webview_list[i] == wv)
			return i;
	}

	return -1;
}

webview *webview_list::get_webview(Evas_Object *ewk_view)
{
	RETV_MSG_IF(ewk_view == NULL, NULL, "ewk_view is NULL");

	for (unsigned int i = 0; i < m_webview_list.size(); i++) {
		if (m_webview_list[i] == NULL)
			continue;

		if (m_webview_list[i]->get_ewk_view() == ewk_view) {
			BROWSER_LOGD("m_webview_list[%d][%p]", i, m_webview_list[i]);
			return m_webview_list[i];
		}
	}

	return NULL;
}

Eina_Bool webview_list::is_ewk_view_exist(Evas_Object *ewk_view)
{
	BROWSER_LOGD("");
	for (unsigned int i = 0; i < m_webview_list.size(); i++) {
		if (m_webview_list[i]->get_ewk_view() == ewk_view) {
			return EINA_TRUE;
		}
	}

	BROWSER_LOGD("ewk_view does not exist!!");
	return EINA_FALSE;
}

Eina_Bool webview_list::is_webview_exist(webview *web_view)
{
	BROWSER_LOGD("");
	for (unsigned int i = 0; i < m_webview_list.size(); i++) {
		if (m_webview_list[i] == web_view)
			return EINA_TRUE;
	}

	BROWSER_LOGD("webview does not exist!!");
	return EINA_FALSE;
}
