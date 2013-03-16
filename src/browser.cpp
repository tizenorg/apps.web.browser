/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.0 (the License);
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

#include "browser.h"

#include <Elementary.h>

#if defined(BROWSER_TAG)
#include "add-tag-view.h"
#endif
#include "bookmark.h"
#include "bookmark-view.h"
#include "bookmark-add-view.h"
#include "bookmark-select-folder-view.h"
#include "bookmark-create-folder-view.h"
#include "bookmark-create-folder-save-view.h"
#include "bookmark-edit-view.h"
#include "browser-dlog.h"
#include "browser-view.h"
#include "geolocation-manager.h"
#include "history.h"
#include "history-view.h"
#include "download-manager.h"
#if defined(MDM)
#include "mdm-manager.h"
#endif
#include "multiwindow-view.h"
#include "network-manager.h"
#include "preference.h"
#include "scrap-view.h"
#include "setting-view.h"
#include "user-agent-manager.h"
#if defined(INSTALL_WEB_APP)
#include "webapp-install-manager.h"
#endif
#include "webview.h"
#include "webview-list.h"
#if defined(WEBCLIP)
#include "scissorbox-view.h"
#endif

#define APP_IN_APP_X	(50 * efl_scale)
#define APP_IN_APP_Y	(100 * efl_scale)
#define APP_IN_APP_W	(500 * efl_scale)
#define APP_IN_APP_H	(450 * efl_scale)

browser::browser(Evas_Object *main_window)
:
	m_network_manager(NULL)
	,m_user_agent_manager(NULL)
	,m_download_manager(NULL)
	,m_bookmark(NULL)
	,m_bookmark_view(NULL)
	,m_bookmark_add_view(NULL)
	,m_history(NULL)
	,m_geolocation_manager(NULL)
#if defined(INSTALL_WEB_APP)
	,m_webapp_install_manager(NULL)
#endif
	,m_history_view(NULL)
	,m_multiwindow_view(NULL)
	,m_setting_view(NULL)
	,m_bookmark_select_folder_view(NULL)
	,m_bookmark_create_folder_view(NULL)
	,m_bookmark_create_folder_save_view(NULL)
	,m_bookmark_edit_view(NULL)
	,m_scrap_view(NULL)
#if defined(BROWSER_TAG)
	,m_add_tag_view(NULL)
#endif
#if defined(WEBCLIP)
	,m_scissorbox_view(NULL)
#endif
{
	BROWSER_LOGD("");

	m_preference = new preference();
	if (!m_preference->init())
		BROWSER_LOGE("m_preference->init failed!");

	/* Set browser_object's main window to share the window */
	m_window = main_window;
	m_browser = this;

	m_webview_context = new webview_context();

	m_webview_list = new webview_list();

	m_browser_view = new browser_view();

#if defined(MDM)
	m_mdm_manager = new mdm_manager();
#endif
	get_network_manager()->init();
}

browser::~browser(void)
{
	BROWSER_LOGD("");

	if (m_network_manager)
		delete m_network_manager;

	if (m_user_agent_manager)
		delete m_user_agent_manager;

	if (m_bookmark)
		delete m_bookmark;

	if (m_bookmark_view)
		delete m_bookmark_view;

	if (m_bookmark_add_view)
		delete m_bookmark_add_view;

	if (m_history)
		delete m_history;

	if (m_geolocation_manager)
		delete m_geolocation_manager;
#if defined(INSTALL_WEB_APP)
	if (m_webapp_install_manager)
		delete m_webapp_install_manager;
#endif
	if (m_history_view)
		delete m_history_view;

	if (m_setting_view)
		delete m_setting_view;

	if (m_bookmark_select_folder_view)
		delete m_bookmark_select_folder_view;

	if (m_bookmark_create_folder_view)
		delete m_bookmark_create_folder_view;

	if (m_bookmark_create_folder_save_view)
		delete m_bookmark_create_folder_save_view;

	if (m_bookmark_edit_view)
		delete m_bookmark_edit_view;

	if (m_scrap_view)
		delete m_scrap_view;

	delete m_browser_view;

	delete m_webview_context;

	delete m_webview_list;

	delete m_preference;

#if defined(MDM)
	delete m_mdm_manager;
#endif
#if defined(WEBCLIP)
	delete m_scissorbox_view;
#endif
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

bookmark_select_folder_view *browser::create_bookmark_select_folder_view(Evas_Smart_Cb cb_func, void *cb_data, Eina_Bool enable_create_folder)
{
	if (!m_bookmark_select_folder_view)
		m_bookmark_select_folder_view = new bookmark_select_folder_view(cb_func, cb_data, enable_create_folder);

	return m_bookmark_select_folder_view;
}

bookmark_create_folder_view *browser::get_bookmark_create_folder_view(Evas_Smart_Cb cb_func, void *cb_data)
{
	if (!m_bookmark_create_folder_view)
		m_bookmark_create_folder_view = new bookmark_create_folder_view(cb_func, cb_data);

	return m_bookmark_create_folder_view;
}

bookmark_create_folder_save_view *browser::get_bookmark_create_folder_save_view(Evas_Smart_Cb cb_func, void *cb_data, int folder_id)
{
	if (!m_bookmark_create_folder_save_view)
		m_bookmark_create_folder_save_view = new bookmark_create_folder_save_view(cb_func, cb_data, folder_id);

	return m_bookmark_create_folder_save_view;
}

bookmark_edit_view *browser::create_bookmark_edit_view(bool tag_mode)
{
	if (!m_bookmark_edit_view)
		m_bookmark_edit_view = new bookmark_edit_view(tag_mode);

	return m_bookmark_edit_view;
}

setting_view *browser::get_setting_view(void)
{
	BROWSER_LOGD("");
	if (!m_setting_view)
		m_setting_view = new setting_view();

	return m_setting_view;
}

void browser::delete_setting_view(void)
{
	if (m_setting_view)
		delete m_setting_view;
	m_setting_view = NULL;
}

void browser::delete_history_view_view(void)
{
	if (m_history_view)
		delete m_history_view;
	m_history_view = NULL;

}

multiwindow_view *browser::get_multiwindow_view(Eina_Bool init_bookmark)
{
	if (!m_multiwindow_view)
		m_multiwindow_view = new multiwindow_view(init_bookmark);

	return m_multiwindow_view;
}

void browser::delete_bookmark_add_view(void)
{
	if (m_bookmark_add_view)
		delete m_bookmark_add_view;
	m_bookmark_add_view = NULL;
}

void browser::delete_bookmark_select_folder_view(void)
{
	if (m_bookmark_select_folder_view)
		delete m_bookmark_select_folder_view;
	m_bookmark_select_folder_view = NULL;
}

void browser::delete_bookmark_create_folder_view(void)
{
	if (m_bookmark_create_folder_view)
		delete m_bookmark_create_folder_view;
	m_bookmark_create_folder_view = NULL;
}

void browser::delete_bookmark_create_folder_save_view(void)
{
	if (m_bookmark_create_folder_save_view)
		delete m_bookmark_create_folder_save_view;
	m_bookmark_create_folder_save_view = NULL;
}

void browser::delete_bookmark_edit_view(void)
{
	if (m_bookmark_edit_view)
		delete m_bookmark_edit_view;
	m_bookmark_edit_view = NULL;
}

void browser::delete_multiwindow_view(void)
{
	// The bookmark view's lifecycle is same with multi window view.
	if (m_bookmark_view)
		delete m_bookmark_view;
	m_bookmark_view = NULL;

	if (m_multiwindow_view)
		delete m_multiwindow_view;
	m_multiwindow_view = NULL;
}

network_manager *browser::get_network_manager(void)
{
	BROWSER_LOGD("");
	if (!m_network_manager)
		m_network_manager = new network_manager();

	return m_network_manager;
}

user_agent_manager *browser::get_user_agent_manager(void)
{
	BROWSER_LOGD("");
	if (!m_user_agent_manager)
		m_user_agent_manager = new user_agent_manager();

	return m_user_agent_manager;
}

download_manager *browser::get_download_manager(void)
{
	BROWSER_LOGD("");
	if (!m_download_manager)
		m_download_manager = new download_manager();

	return m_download_manager;
}

bookmark *browser::get_bookmark(void)
{
	BROWSER_LOGD("");
	if (!m_bookmark)
		m_bookmark = new bookmark();

	return m_bookmark;
}

geolocation_manager *browser::get_geolocation_manager(void)
{
	BROWSER_LOGD("");
	if (!m_geolocation_manager)
		m_geolocation_manager = new geolocation_manager();

	return m_geolocation_manager;
}

#if defined(INSTALL_WEB_APP)
webapp_install_manager *browser::get_webapp_install_manager(void)
{
	BROWSER_LOGD("");
	if (!m_webapp_install_manager)
		m_webapp_install_manager = new webapp_install_manager();

	return m_webapp_install_manager;
}
#endif

history *browser::get_history(void)
{
	BROWSER_LOGD("");
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

scrap_view *browser::get_scrap_view(void)
{
	if (!m_scrap_view)
		m_scrap_view = new scrap_view();

	return m_scrap_view;
}

void browser::delete_scrap_view(void)
{
	if (m_scrap_view) {
		delete m_scrap_view;
		m_scrap_view = NULL;
	}
}

#if defined(BROWSER_TAG)
add_tag_view *browser::get_add_tag_view(Evas_Smart_Cb done_cb, void *done_cb_data, const char *tag)
{
	if (!m_add_tag_view)
		m_add_tag_view = new add_tag_view(done_cb, done_cb_data, tag);

	return m_add_tag_view;
}

void browser::delete_add_tag_view(void)
{
	if (m_add_tag_view) {
		delete m_add_tag_view;
		m_add_tag_view = NULL;
	}
}
#endif

#if defined(WEBCLIP)
scissorbox_view *browser::get_scissorbox_view(void)
{
	BROWSER_LOGD("");
	if (!m_scissorbox_view)
		m_scissorbox_view = new scissorbox_view();

	return m_scissorbox_view;
}
void browser::delete_scissorbox_view(void)
{
	if (m_scissorbox_view)
		delete m_scissorbox_view;
	m_scissorbox_view = NULL;
}
#endif
void browser::pause(void)
{
	BROWSER_LOGD("");
	m_browser_view->get_current_webview()->deactivate();
}

void browser::resume(void)
{
	BROWSER_LOGD("");
	m_browser_view->get_current_webview()->activate();
	if (m_browser_view->get_current_webview()->has_focus() && m_browser_view->is_top_view())
		m_browser_view->get_current_webview()->give_focus();

	m_network_manager->set_network_unlock(EINA_TRUE);
}

void browser::rotate(int degree)
{
	BROWSER_LOGD("degree[%d]", degree);
	webview_list *wl = get_webview_list();

	for (int i = 0 ; i < wl->get_count() ; i++)
		wl->get_webview(i)->orientation_send(degree);
}

void browser::launch(const char *uri)
{
	BROWSER_LOGD("");

	m_browser_view->launch(uri);
}

void browser::show_popup_browser(Eina_Bool popup)
{
	BROWSER_LOGD("enable = [%d]", popup);
}

void browser::set_full_screen_enable(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);
	if (enable) {
		elm_win_indicator_mode_set(m_window, ELM_WIN_INDICATOR_HIDE);
		m_browser_view->show_uri_bar(EINA_FALSE);
	} else {
		elm_win_indicator_mode_set(m_window, ELM_WIN_INDICATOR_SHOW);
		m_browser_view->show_uri_bar(EINA_TRUE);
	}
}

void browser::set_app_in_app_enable(Eina_Bool enable)
{
	BROWSER_LOGD("enable = [%d]", enable);

	elm_win_floating_mode_set(m_window, enable);

	set_full_screen_enable(enable);

	m_browser_view->disable_webview_event(enable);
	m_browser_view->show_resize_close_buttons(enable);

	if (enable) {
		int window_w = 0;
		int window_h = 0;
		evas_object_geometry_get(m_window, NULL, NULL, &window_w, &window_h);
		evas_object_size_hint_min_set(m_window, APP_IN_APP_W, APP_IN_APP_H);
		evas_object_move(m_window, APP_IN_APP_X,  APP_IN_APP_Y);
		evas_object_resize(m_window, APP_IN_APP_W, APP_IN_APP_H);
	}
}

Eina_Bool browser::get_app_in_app_enable(void)
{
	return elm_win_floating_mode_get(m_window);
}

