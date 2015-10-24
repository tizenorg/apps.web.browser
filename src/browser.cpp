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

#include "browser.h"

#include <Elementary.h>
#include <Ecore_X.h>

#include "bookmark.h"
#include "bookmark-view.h"
#include "bookmark-add-view.h"
#include "bookmark-select-folder-view.h"
#include "bookmark-create-folder-view.h"
#include "bookmark-edit-view.h"
#include "browser-dlog.h"
#include "browser-view.h"
#include "cloud-sync-manager.h"
#include "download-manager.h"
#include "find-on-page.h"
#include "history.h"
#include "history-view.h"
#include "main-view.h"
#include "network-manager.h"
#include "platform-service.h"
#include "preference.h"
#include "search-engine-manager.h"
#include "settings.h"
#include "url-bar.h"
#include "url-input-bar.h"
#include "webview.h"
#include "webview-list.h"
#ifdef ENABLE_WEB_NOTIFICATION_API
#include "web-notification.h"
#endif // ENABLE_WEB_NOTIFICATION_API
#include "user-agent-manager.h"

browser::browser(Evas_Object *main_window)
	: m_main_view(NULL)
	, m_browser_view(NULL)
	, m_webview_list(NULL)
	, m_network_manager(NULL)
	, m_user_agent_manager(NULL)
	, m_download_manager(NULL)
	, m_bookmark(NULL)
	, m_bookmark_view(NULL)
	, m_bookmark_add_view(NULL)
	, m_history(NULL)
	, m_history_view(NULL)
	, m_developer_mode(EINA_FALSE)
	, m_certificate_view(NULL)
	, m_settings(NULL)
#ifdef ENABLE_WEB_NOTIFICATION_API
	, m_web_notification_manager(NULL)
#endif
	, m_bookmark_select_folder_view(NULL)
	, m_bookmark_create_folder_view(NULL)
	, m_bookmark_edit_view(NULL)
	, m_cloud_sync_manager(NULL)
	, m_tab_manager_view(NULL)
	, m_is_full_screen(EINA_FALSE)
	, m_app_paused(EINA_FALSE)
	, m_app_paused_by_display_off(EINA_FALSE)
	, m_common_view_list()
	, m_history_listener_list()
	, m_bookmark_listener_list()
	, m_referer_header(NULL)
	, m_is_app_control(EINA_FALSE)
{
	/* Set browser_object's main window to share the window */
	m_window = main_window;
	m_browser = this;

	m_preference = new preference();
	if (!m_preference->init())
		BROWSER_LOGE("m_preference->init failed!");
}

browser::~browser(void)
{
	delete m_network_manager;
	delete m_user_agent_manager;
	delete m_download_manager;
	delete m_bookmark;
	delete m_bookmark_view;
	delete m_bookmark_add_view;
	delete m_history;
	delete m_history_view;
	delete m_certificate_view;
	delete m_settings;
#ifdef ENABLE_WEB_NOTIFICATION_API
	delete m_web_notification_manager;
#endif
	delete m_bookmark_select_folder_view;
	delete m_bookmark_create_folder_view;
	delete m_bookmark_edit_view;
	delete m_tab_manager_view;
	/* Do not remove webview list first before browser view */
	delete m_browser_view;
	delete m_main_view;
	webview_context::cleanup();
	delete m_webview_list;
	delete m_preference;
	delete m_cloud_sync_manager;
	free(m_referer_header);
}

bookmark_view *browser::get_bookmark_view(void)
{
	if (!m_bookmark_view)
		m_bookmark_view = new bookmark_view();

	return m_bookmark_view;
}

bookmark_add_view *browser::create_bookmark_add_view(const char  *title, const char *uri, int folder_id_to_save, Eina_Bool edit_mode)
{
	if (!m_bookmark_add_view)
		m_bookmark_add_view = new bookmark_add_view(title, uri, folder_id_to_save, edit_mode);

	return m_bookmark_add_view;
}

bookmark_select_folder_view *browser::create_bookmark_select_folder_view(Evas_Smart_Cb cb_func, void *cb_data)
{
	if (!m_bookmark_select_folder_view)
		m_bookmark_select_folder_view = new bookmark_select_folder_view(cb_func, cb_data);

	return m_bookmark_select_folder_view;
}

bookmark_create_folder_view *browser::get_bookmark_create_folder_view(Evas_Smart_Cb cb_func, void *cb_data, int parent_id)
{
	if (!m_bookmark_create_folder_view)
		m_bookmark_create_folder_view = new bookmark_create_folder_view(cb_func, cb_data, parent_id);

	return m_bookmark_create_folder_view;
}

bookmark_edit_view *browser::create_bookmark_edit_folder_reorder_view(Evas_Smart_Cb cb_func, void *cb_data, int folder_id, const char *path_string)
{
	if (!m_bookmark_edit_view)
		m_bookmark_edit_view = new bookmark_edit_view(cb_func, cb_data, folder_id, path_string, EDIT_FOLDER_REORDER_VIEW);

	return m_bookmark_edit_view;
}

bookmark_edit_view *browser::create_bookmark_edit_folder_move_view(Evas_Smart_Cb cb_func, void *cb_data, int folder_id, const char *path_string)
{
	if (!m_bookmark_edit_view)
		m_bookmark_edit_view = new bookmark_edit_view(cb_func, cb_data, folder_id, path_string, EDIT_FOLDER_MOVE_VIEW);

	return m_bookmark_edit_view;
}

bookmark_edit_view *browser::create_bookmark_edit_folder_delete_view(Evas_Smart_Cb cb_func, void *cb_data, int folder_id, const char *path_string)
{
	if (!m_bookmark_edit_view)
		m_bookmark_edit_view = new bookmark_edit_view(cb_func, cb_data, folder_id, path_string, EDIT_FOLDER_DELETE_VIEW);

	return m_bookmark_edit_view;
}

certificate_view *browser::get_certificate_view(X509 *m_certificate, Evas_Smart_Cb cb_func, void *cb_data)
{
	if (!m_certificate_view && m_certificate)
		m_certificate_view = new certificate_view(m_certificate, cb_func, cb_data);

	return m_certificate_view;
}

void browser::delete_certificate_view()
{
	delete m_certificate_view;
	m_certificate_view = NULL;
}

settings *browser::get_settings(void)
{
	BROWSER_LOGD("");
	if (!m_settings)
		m_settings = new settings();

	return m_settings;
}

void browser::delete_settings(void)
{
	delete m_settings;
	m_settings = NULL;
}

void browser::delete_history_view_view(void)
{
	delete m_history_view;
	m_history_view = NULL;
}

#ifdef ENABLE_WEB_NOTIFICATION_API
web_notification_manager *browser::get_web_noti_manager(void)
{
	if (!m_web_notification_manager)
		m_web_notification_manager = new web_notification_manager();

	return m_web_notification_manager;
}

void browser::delete_web_noti_manager(void)
{
	delete m_web_notification_manager;
	m_web_notification_manager = NULL;
}
#endif

void browser::navi_frame_tree_focus_allow_set(Eina_Bool allow)
{
	elm_object_tree_focus_allow_set(get_browser_view()->get_naviframe(), allow);
}

void browser::delete_bookmark_view(void)
{
	delete m_bookmark_view;
	m_bookmark_view = NULL;
}

void browser::delete_bookmark_add_view(void)
{
	delete m_bookmark_add_view;
	m_bookmark_add_view = NULL;
}

void browser::delete_bookmark_select_folder_view(void)
{
	delete m_bookmark_select_folder_view;
	m_bookmark_select_folder_view = NULL;
}

void browser::delete_bookmark_create_folder_view(void)
{
	delete m_bookmark_create_folder_view;
	m_bookmark_create_folder_view = NULL;
}

void browser::delete_bookmark_edit_view(void)
{
	delete m_bookmark_edit_view;
	m_bookmark_edit_view = NULL;
}

network_manager *browser::get_network_manager(void)
{
	if (!m_network_manager)
		m_network_manager = new network_manager();

	return m_network_manager;
}

user_agent_manager *browser::get_user_agent_manager(void)
{
	if (!m_user_agent_manager)
		m_user_agent_manager = new user_agent_manager();

	return m_user_agent_manager;
}

download_manager *browser::get_download_manager(void)
{
	if (!m_download_manager)
		m_download_manager = new download_manager();

	return m_download_manager;
}

bookmark *browser::get_bookmark(void)
{
	if (!m_bookmark)
		m_bookmark = new bookmark();

	return m_bookmark;
}

history *browser::get_history(void)
{
	if (!m_history)
		m_history = new history();

	return m_history;
}

history_view *browser::get_history_view(void)
{
	BROWSER_LOGD("");
	if (!m_history_view)
		m_history_view = new history_view();

	return m_history_view;
}

tab_manager_view_lite *browser::get_tab_manager_view(void)
{
	if (!m_tab_manager_view)
		m_tab_manager_view = new tab_manager_view_lite();

	return m_tab_manager_view;
}

void browser::delete_tab_manager_view(void)
{
	BROWSER_LOGD("");

	url_bar *ub = get_browser_view()->get_url_bar();
	if (m_tab_manager_view){
		delete m_tab_manager_view;
		m_tab_manager_view = NULL;

		// Enable tab manager button.
		ub->enable_tab_manager_button(EINA_TRUE);

		// Resize if required.
		webview *wv = m_browser->get_browser_view()->get_current_webview();
		RET_MSG_IF(!wv, "current webview is NULL");

		ub->set_fixed_mode(EINA_FALSE);
		ub->update_secure_icon();
	}
}

void browser::clear_all_popups(void)
{
	for (unsigned int i = 0; i < m_common_view_list.size(); i++) {
		m_common_view_list[i]->clear_popups();
	}
}

void browser::reset(void)
{
	BROWSER_LOGD("");

	m_browser->get_main_view()->register_callbacks();
}

void browser::deregister(void)
{
        BROWSER_LOGD("");

        m_browser->get_main_view()->deregister_callbacks();
}

void browser::pause(void)
{
	BROWSER_LOGD("");
	if (m_browser_view->get_current_webview())
		m_browser_view->get_current_webview()->suspend();
	m_browser_view->get_url_bar()->get_more_menu_manager()->hide_more_context_popup();
	if (m_browser_view->is_show_find_on_page() && m_browser_view->get_current_webview()->has_focus()) {
		find_on_page *fop = m_browser_view->get_find_on_page();
		if (fop != NULL) {
			elm_object_focus_set(fop->get_entry(), EINA_FALSE);
			elm_object_focus_allow_set(fop->get_entry(), EINA_FALSE);
		}
	}
	_hide_browser_popups();
}

void browser::resume(void)
{
	BROWSER_LOGD("");

	webview *cur_wv = m_browser_view->get_current_webview();
	// If webview does not exist create a new one and load home page.
	if (cur_wv == NULL) {
		if (get_app_paused_by_display_off()) {
			set_app_paused_by_display_off(EINA_FALSE);
			m_browser_view->set_current_webview(NULL);
			return;
		}
		char *homepage_uri = m_preference->get_homepage_uri();
		BROWSER_LOGD("create webview with homepage");
		cur_wv = get_webview_list()->create_webview(EINA_TRUE);
		cur_wv->set_request_uri(homepage_uri);
		m_browser_view->set_current_webview(cur_wv);
		cur_wv->load_uri(homepage_uri);
		free(homepage_uri);
	}

	cur_wv->resume();
	if (cur_wv->has_focus() && m_browser_view->is_top_view()) {
		BROWSER_LOGD("** set focus on webview");
		cur_wv->give_focus();
	}

	// Focus handling in url input bar.
	if (m_browser_view->is_show_url_input_bar())
		m_browser_view->get_url_input_bar()->set_focus();

	/* handling focus for find on page entry */
	if (m_browser_view->is_show_find_on_page())
		elm_object_focus_allow_set(m_browser_view->get_find_on_page()->get_entry(), EINA_TRUE);

	// Set size of main scroller.
	main_view *mv = get_main_view();
	mv->register_callbacks();
	if ( is_tts_enabled() && mv->get_scroll_mode() == CONTENT_VIEW_SCROLL_ENABLE)
		mv->set_scroll_mode(CONTENT_VIEW_SCROLL_FIXED_TOOLBAR);
	else if (m_preference->get_hide_URL_toolbar_enabled() && !is_tts_enabled() && mv->get_scroll_mode() == CONTENT_VIEW_SCROLL_FIXED_TOOLBAR)
		mv->set_scroll_mode(CONTENT_VIEW_SCROLL_ENABLE);

	mv->resize_scroll_content();
}

void browser::register_common_view(common_view *view)
{
	BROWSER_LOGD("");
	m_common_view_list.push_back(view);
}

void browser::unregister_common_view(common_view *view)
{
	BROWSER_LOGD("");
	for (unsigned int i = 0; i < m_common_view_list.size(); i++) {
		if (view == m_common_view_list[i]) {
			m_common_view_list.erase(m_common_view_list.begin() + i);
			break;
		}
	}
}

void browser::register_history_listener(history_listener *hl)
{
	BROWSER_LOGD("");
	m_history_listener_list.push_back(hl);
}

void browser::unregister_history_listener(history_listener *hl)
{
	BROWSER_LOGD("");
	for (unsigned int i = 0; i < m_history_listener_list.size(); i++) {
		if (hl == m_history_listener_list[i]) {
			m_history_listener_list.erase(m_history_listener_list.begin() + i);
			break;
		}
	}
}

void browser::register_bookmark_listener(bookmark_listener *bl)
{
	BROWSER_LOGD("");
	m_bookmark_listener_list.push_back(bl);
}

void browser::unregister_bookmark_listener(bookmark_listener *bl)
{
	BROWSER_LOGD("");
	for (unsigned int i = 0; i < m_bookmark_listener_list.size(); i++) {
		if (bl == m_bookmark_listener_list[i]) {
			m_bookmark_listener_list.erase(m_bookmark_listener_list.begin() + i);
			break;
		}
	}
}

void browser::notify_bookmark_added(const char *uri,  int bookmark_id, int parent_id)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!uri, "uri is NULL");
	const char *bookmark_uri = eina_stringshare_add(uri);

	for (unsigned int i = 0; i < m_bookmark_listener_list.size(); i++) {
		m_bookmark_listener_list[i]->__bookmark_added(bookmark_uri, bookmark_id, parent_id);
	}

	eina_stringshare_del(bookmark_uri);
}

void browser::notify_bookmark_updated(const char *uri, const char *title,  int bookmark_id, int parent_id)
{
	BROWSER_LOGD("");

	const char *bookmark_uri = eina_stringshare_add(uri);
	const char *bookmark_title = eina_stringshare_add(title);

	for (unsigned int i = 0; i < m_bookmark_listener_list.size(); i++) {
		m_bookmark_listener_list[i]->__bookmark_updated(bookmark_uri, bookmark_title, bookmark_id, parent_id);
	}

	if (bookmark_uri)
		eina_stringshare_del(bookmark_uri);
	if (bookmark_title)
		eina_stringshare_del(bookmark_title);
}

void browser::notify_bookmark_removed(const char *uri, int bookmark_id, int parent_id)
{
	BROWSER_LOGD("");
	for (unsigned int i = 0; i < m_bookmark_listener_list.size(); i++) {
		m_bookmark_listener_list[i]->__bookmark_removed(uri, bookmark_id, parent_id);
	}
}

void browser::notify_history_added(int id, const char *title, const char *uri, Evas_Object *snapshot, Evas_Object *favicon, int visit_count)
{
	BROWSER_LOGD("");
	history_item item(id, title, uri, snapshot, visit_count, 0, favicon);

	for (unsigned int i = 0; i < m_history_listener_list.size(); i++) {
		m_history_listener_list[i]->__history_added(&item);
	}
}

void browser::notify_history_deleted(const char *uri)
{
	RET_MSG_IF(!uri, "uri is NULL");
	BROWSER_LOGD("");
	const char *history_uri = eina_stringshare_add(uri);

	for (unsigned int i = 0; i < m_history_listener_list.size(); i++) {
		m_history_listener_list[i]->__history_deleted(history_uri);
	}

	eina_stringshare_del(history_uri);
}

void browser::notify_history_cleared(Eina_Bool is_cancelled)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_history_listener_list.size(); i++) {
		m_history_listener_list[i]->__history_cleared(is_cancelled);
	}
}

void browser::rotate(int degree)
{
	BROWSER_LOGD("degree[%d]", degree);

	if (m_browser_view->get_current_webview())
		m_browser_view->get_current_webview()->orientation_send(degree);

	if (degree == 0 || degree == 180) {
		get_main_view()->set_landscape_mode(EINA_FALSE);
		m_browser_view->rotate(EINA_FALSE);
	} else {
		get_main_view()->set_landscape_mode(EINA_TRUE);
		m_browser_view->rotate(EINA_TRUE);
	}

	if (m_bookmark_view)
		m_bookmark_view->rotate();

	if (m_tab_manager_view){
		if (degree == 0 || degree == 180) {
			m_tab_manager_view->on_rotate(EINA_FALSE);
		}else{
			m_tab_manager_view->on_rotate(EINA_TRUE);
		}
	}

}

void browser::launch(const char *uri)
{
	BROWSER_LOGD("");

	m_browser_view->launch(uri);
}

void browser::launch_homepage(void)
{
	BROWSER_LOGD("");
	char *uri = m_preference->get_homepage_uri();
	m_browser_view->launch(uri);
	free(uri);
}

void browser::language_changed(void)
{
	BROWSER_LOGD("");

	if (m_browser_view)
		m_browser_view->apply_changed_language();

	if (m_bookmark_edit_view)
		m_bookmark_edit_view->apply_changed_language();
}

void browser::set_full_screen_enable(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);
	browser_view *bv = m_browser->get_browser_view();

	m_is_full_screen = enable;
	if (enable) {
		if (bv->get_view_mode() == browser_view_mode_normal)
			elm_win_wm_rotation_manual_rotation_done_set(m_window, EINA_TRUE);

		elm_win_indicator_opacity_set(m_window, ELM_WIN_INDICATOR_TRANSPARENT);

		BROWSER_LOGD("*** fullscreen mode");
		bv->set_window_view_mode(VIEW_MODE_FULLSCREEN);
	} else {
		elm_win_wm_rotation_manual_rotation_done_set(m_window, EINA_FALSE);

		if (!m_preference->get_indicator_fullscreen_view_enabled())
			elm_win_indicator_opacity_set(m_window, ELM_WIN_INDICATOR_OPAQUE);

		BROWSER_LOGD("*** normal mode");
		bv->set_window_view_mode(VIEW_MODE_NORMAL);
	}
}

Eina_Bool browser::is_keyboard_active(void)
{
	platform_service ps;
	bool keyboard_active = ps.check_hw_usb_keyboard_alive();
	BROWSER_LOGD("keyboard: %d", keyboard_active);
	return keyboard_active;
}

Eina_Bool browser::is_tts_enabled(void)
{
	Eina_Bool enabled = elm_config_access_get();
	BROWSER_LOGD("tts enabled: %d", enabled);
	return enabled;
}

void browser::launch_for_searching(const char *keyword)
{
	BROWSER_LOGD("");
	search_engine_manager *search_manager = new search_engine_manager();
	std::string query_uri = search_manager->query_string_get(m_preference->get_search_engine_type(), keyword);

	BROWSER_SECURE_LOGD("query URI: %s", query_uri.c_str());
	m_browser_view->launch(query_uri.c_str());

	delete search_manager;
}

void browser::_hide_browser_popups(void)
{
	/* hide / delete popups */
	m_browser_view->get_url_bar()->get_more_menu_manager()->hide_more_context_popup();
	if (m_history_view  != NULL)
		m_history_view->hide_history_popups();
	if (m_bookmark_view != NULL)
		m_bookmark_view->hide_bookmark_popups();
	for (unsigned int i = 0; i < m_common_view_list.size(); i++) {
		m_common_view_list[i]->on_pause();
	}
}

void browser::set_referer_header(char *header)
{
	if (m_referer_header != NULL) {
		free(m_referer_header);
		m_referer_header = NULL;
	}

	if (header != NULL)
		m_referer_header = strdup(header);
}

webview_list *browser::get_webview_list(void)
{
	if (!m_webview_list)
		m_webview_list = new webview_list();

	return m_webview_list;
}

cloud_sync_manager *browser::get_cloud_sync_manager(void)
{
	if (!m_cloud_sync_manager)
		m_cloud_sync_manager = new cloud_sync_manager();

	return m_cloud_sync_manager;
}

main_view *browser::get_main_view(void)
{
	if (!m_main_view)
		m_main_view = new main_view();

	return m_main_view;
}

browser_view *browser::get_browser_view(void)
{
	if (!m_browser_view)
		m_browser_view = new browser_view();

	return m_browser_view;
}
