/*
Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved

This file is part of browser
Written by Hyerim Bae hyerim.bae@samsung.com.

PROPRIETARY/CONFIDENTIAL

This software is the confidential and proprietary information of
SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered
into with SAMSUNG ELECTRONICS.

SAMSUNG make no representations or warranties about the suitability
of the software, either express or implied, including but not limited
to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. SAMSUNG shall not be liable for any
damages suffered by licensee as a result of using, modifying or
distributing this software or its derivatives.
*/

#include "browser-authentication-manager.h"
#include "browser-certificate-manager.h"
#include "browser-class.h"
#include "browser-context-menu.h"
#include "browser-download-manager.h"
#include "browser-exscheme-handler.h"
#include "browser-find-word.h"
#include "browser-multi-window-view.h"
#include "browser-network-manager.h"
#include "browser-notification-manager.h"
#include "browser-picker-handler.h"
#include "browser-user-agent-db.h"
#include "browser-view.h"
#include "browser-window.h"

Browser_Class::Browser_Class(Evas_Object *win, Evas_Object *navi_bar, Evas_Object *bg, Evas_Object *layout)
:
	m_win(win)
	,m_navi_bar(navi_bar)
	,m_bg(bg)
	,m_layout(layout)
	,m_browser_view(NULL)
	,m_focused_window(NULL)
	,m_user_agent_db(NULL)
	,m_download_manager(NULL)
	,m_clean_up_windows_timer(NULL)
	,m_authentication_manager(NULL)
	,m_certificate_manager(NULL)
	,m_notification_manager(NULL)
	,m_network_manager(NULL)
{
	m_window_list.clear();
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Class::~Browser_Class(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_browser_view)
		delete m_browser_view;
	if (m_user_agent_db)
		delete m_user_agent_db;
	if (m_download_manager)
		delete m_download_manager;
	if (m_authentication_manager)
		delete m_authentication_manager;
	if (m_certificate_manager)
		delete m_certificate_manager;
	if (m_notification_manager)
		delete m_notification_manager;
	if (m_network_manager)
		delete m_network_manager;

	for (int i = 0 ; i < m_window_list.size() ; i++) {
		if (m_window_list[i])
			delete m_window_list[i];
		m_window_list.erase(m_window_list.begin() + i);
	}

	if (m_clean_up_windows_timer)
		ecore_timer_del(m_clean_up_windows_timer);

	if (!ewk_cache_dump())
		BROWSER_LOGE("ewk_cache_dump failed");

	ewk_shutdown();
}

Eina_Bool Browser_Class::init(void)
{
	BROWSER_LOGD("[%s]", __func__);
	ewk_init();

	m_user_agent_db = new(nothrow) Browser_User_Agent_DB;
	if (!m_user_agent_db) {
		BROWSER_LOGE("new Browser_User_Agent_DB failed");
		return EINA_FALSE;
	}

	m_browser_view = new(nothrow) Browser_View(m_win, m_navi_bar, m_bg, m_layout, this);
	/* Create browser view layout */
	if (m_browser_view) {
		if (!m_browser_view->init()) {
			BROWSER_LOGE("m_browser_view->init failed");
			return EINA_FALSE;
		}
	} else
		return EINA_FALSE;

	m_download_manager = new(nothrow) Browser_Download_Manager(m_navi_bar, m_browser_view);
	if (!m_download_manager) {
		BROWSER_LOGE("new Browser_Policy_Decision_Maker failed");
		return EINA_FALSE;
	}

	m_authentication_manager = new(nothrow) Browser_Authetication_Manager;
	if (!m_authentication_manager) {
		BROWSER_LOGE("new Browser_Authetication_Manager failed");
		return EINA_FALSE;
	}
 	m_authentication_manager->init();

	m_certificate_manager = new(nothrow) Browser_Certificate_Manager;
	if (!m_certificate_manager) {
		BROWSER_LOGE("new Browser_Certificate_Manager failed");
		return EINA_FALSE;
	}
	if (!m_certificate_manager->init()) {
		BROWSER_LOGE("m_certificate_manager->init failed");
		delete m_certificate_manager;
		m_certificate_manager = NULL;
		return EINA_FALSE;
	}

	m_notification_manager = new(nothrow) Browser_Notification_Manager;
	if (!m_notification_manager) {
		BROWSER_LOGE("new Browser_Notification_Manager failed");
		return EINA_FALSE;
	}

	m_network_manager = new(nothrow) Browser_Network_Manager;
	if (!m_network_manager) {
		BROWSER_LOGE("new Browser_Network_Manager failed");
		return EINA_FALSE;
	}

	if (!_set_ewk_view_options_listener()) {
		BROWSER_LOGE("_set_ewk_view_options_listener failed");
		return EINA_FALSE;
	}

	if (!ewk_settings_icon_database_path_set(DATABASE_DIR)) {
		BROWSER_LOGE("ewk_settings_icon_database_path_set failed");
		return EINA_FALSE;
	}

	if (!ewk_cache_init(WEBKIT_SOUP_CACHE_DIR, EINA_FALSE)) {
		BROWSER_LOGE("ewk_cache_init failed");
		return EINA_FALSE;
	}
	if (!ewk_cache_load()) {
		BROWSER_LOGE("ewk_cache_load failed");
		return EINA_FALSE;
	}

	if (!ewk_cookies_file_set(DATABASE_DIR"/"COOKIES_DATABASENAME)) {
		BROWSER_LOGE("ewk_cookies_file_set failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

void Browser_Class::__vconf_changed_cb(keynode_t *keynode, void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Class *browser = (Browser_Class *)data;
	char *key = vconf_keynode_get_name(keynode);
	if (!key || strlen(key) == 0) {
		BROWSER_LOGD("vconf_keynode_get_name failed");
		return;
	}

	if (!strncmp(key, USERAGENT_KEY, strlen(USERAGENT_KEY))) {
		for (int i = 0 ; i < browser->m_window_list.size() ; i++) {
			if (browser->m_window_list[i]->m_ewk_view) {
				if (!browser->_set_user_agent(browser->m_window_list[i]->m_ewk_view))
					BROWSER_LOGE("_set_user_agent failed");
			}
		}
	} else if (!strncmp(key, DEFAULT_VIEW_LEVEL_KEY, strlen(DEFAULT_VIEW_LEVEL_KEY))) {
		char *default_level = vconf_get_str(DEFAULT_VIEW_LEVEL_KEY);
		for (int i = 0 ; i < browser->m_window_list.size() ; i++) {			
			if (default_level && browser->m_window_list[i]->m_ewk_view) {
				if (!strncmp(default_level, FIT_TO_WIDTH, strlen(FIT_TO_WIDTH)))
					elm_webview_auto_fitting_set(browser->m_window_list[i]->m_ewk_view, EINA_TRUE);
				else
					elm_webview_auto_fitting_set(browser->m_window_list[i]->m_ewk_view, EINA_FALSE);
			}
		}
		if (default_level)
			free(default_level);
	} else if (!strncmp(key, RUN_JAVASCRIPT_KEY, strlen(RUN_JAVASCRIPT_KEY))) {
		int run_javascript = 1;
		if (vconf_get_bool(RUN_JAVASCRIPT_KEY, &run_javascript) < 0)
			BROWSER_LOGE("Can not get [%s] value.\n", RUN_JAVASCRIPT_KEY);
		for (int i = 0 ; i < browser->m_window_list.size() ; i++) {
			if (browser->m_window_list[i]->m_ewk_view) {
				Evas_Object *webkit = elm_webview_webkit_get(browser->m_window_list[i]->m_ewk_view);
				if (run_javascript) {
					if (!ewk_view_setting_enable_scripts_set(webkit, EINA_TRUE))
						BROWSER_LOGE("ewk_view_setting_enable_scripts_set failed");
				} else {
					if (!ewk_view_setting_enable_scripts_set(webkit, EINA_FALSE))
						BROWSER_LOGE("ewk_view_setting_enable_scripts_set failed");
				}
			}
		}
	} else if (!strncmp(key, DISPLAY_IMAGES_KEY, strlen(DISPLAY_IMAGES_KEY))) {
		int display_images = 1;
		if (vconf_get_bool(DISPLAY_IMAGES_KEY, &display_images) < 0)
			BROWSER_LOGE("Can not get [%s] value.\n", DISPLAY_IMAGES_KEY);
		for (int i = 0 ; i < browser->m_window_list.size() ; i++) {
			if (browser->m_window_list[i]->m_ewk_view) {
				Evas_Object *webkit = elm_webview_webkit_get(browser->m_window_list[i]->m_ewk_view);
				if (display_images) {
					if (!ewk_view_setting_auto_load_images_set(webkit, EINA_TRUE))
						BROWSER_LOGE("ewk_view_setting_auto_load_images_set failed");
				} else {
					if (!ewk_view_setting_auto_load_images_set(webkit, EINA_FALSE))
						BROWSER_LOGE("ewk_view_setting_auto_load_images_set failed");
				}
			}
		}
	} else if (!strncmp(key, BLOCK_POPUP_KEY, strlen(BLOCK_POPUP_KEY))) {
		int block_popup = 1;
		if (vconf_get_bool(BLOCK_POPUP_KEY, &block_popup) < 0)
			BROWSER_LOGE("Can not get [%s] value.\n", BLOCK_POPUP_KEY);
		for (int i = 0 ; i < browser->m_window_list.size() ; i++) {
			if (browser->m_window_list[i]->m_ewk_view) {
				Evas_Object *webkit = elm_webview_webkit_get(browser->m_window_list[i]->m_ewk_view);
				if (block_popup) {
					if (!ewk_view_setting_scripts_window_open_set(webkit, EINA_FALSE))
						BROWSER_LOGE("ewk_view_setting_scripts_window_open_set failed");
				} else {
					if (!ewk_view_setting_scripts_window_open_set(webkit, EINA_TRUE))
						BROWSER_LOGE("ewk_view_setting_scripts_window_open_set failed");
				}
			}
		}
	} else if (!strncmp(key, ACCEPT_COOKIES_KEY, strlen(ACCEPT_COOKIES_KEY))) {
		int accept_cookies = 1;
		if (vconf_get_bool(ACCEPT_COOKIES_KEY, &accept_cookies) < 0)
			BROWSER_LOGE("Can not get [%s] value.\n", ACCEPT_COOKIES_KEY);
		if (accept_cookies)
			ewk_cookies_policy_set(EWK_COOKIE_JAR_ACCEPT_ALWAYS);
		else
			ewk_cookies_policy_set(EWK_COOKIE_JAR_ACCEPT_NEVER);
	} else if (!strncmp(key, RUN_PLUGINS_KEY, strlen(RUN_PLUGINS_KEY))) {
		int run_plugins = 1;
		if (vconf_get_bool(RUN_PLUGINS_KEY, &run_plugins) < 0)
			BROWSER_LOGE("Can not get [%s] value.\n", RUN_PLUGINS_KEY);

		for (int i = 0 ; i < browser->m_window_list.size() ; i++) {
			if (browser->m_window_list[i]->m_ewk_view) {
				Evas_Object *webkit = elm_webview_webkit_get(browser->m_window_list[i]->m_ewk_view);
				if (run_plugins) {
					if (!ewk_view_setting_enable_plugins_set(webkit, EINA_TRUE))
						BROWSER_LOGE("ewk_view_setting_enable_plugins_set failed");
					if (!ewk_view_setting_enable_specified_plugin_set(webkit,
								EINA_TRUE, BROWSER_FLASH_MIME_TYPE)) {
						BROWSER_LOGE("ewk_view_setting_enable_specified_plugin_set failed");
					}
				} else {
					if (!ewk_view_setting_enable_plugins_set(webkit, EINA_FALSE))
						BROWSER_LOGE("ewk_view_setting_enable_plugins_set failed");
					if (!ewk_view_setting_enable_specified_plugin_set(webkit,
								EINA_FALSE, BROWSER_FLASH_MIME_TYPE)) {
						BROWSER_LOGE("ewk_view_setting_enable_specified_plugin_set failed");
					}
				}
			}
		}
	}else if (!strncmp(key, ACCELERATED_COMPOSITION_KEY, strlen(ACCELERATED_COMPOSITION_KEY))) {
		BROWSER_LOGD("ACCELERATED_COMPOSITION_KEY");
		int accelerated_composition = 1;
		if (vconf_get_bool(ACCELERATED_COMPOSITION_KEY, &accelerated_composition) < 0)
			BROWSER_LOGE("Can not get [%s] value.\n", ACCELERATED_COMPOSITION_KEY);

		int external_video_player = 0;
		if (vconf_get_bool(EXTERNAL_VIDEO_PLAYER_KEY, &external_video_player) < 0)
			BROWSER_LOGE("Can not get [%s] value.\n", EXTERNAL_VIDEO_PLAYER_KEY);

		for (int i = 0 ; i < browser->m_window_list.size() ; i++) {
			if (browser->m_window_list[i]->m_ewk_view) {
				Evas_Object *webkit = elm_webview_webkit_get(browser->m_window_list[i]->m_ewk_view);
				if (accelerated_composition) {
					if (!ewk_view_setting_accelerated_compositing_enable_set(webkit, EINA_TRUE))
						BROWSER_LOGE("ewk_view_setting_accelerated_compositing_enable_set failed");
					if (external_video_player) {
						if (!ewk_view_setting_html5video_external_player_enable_set(webkit, EINA_TRUE))
							BROWSER_LOGE("ewk_view_setting_html5video_external_player_enable_set failed");
					} else {
						if (!ewk_view_setting_html5video_external_player_enable_set(webkit, EINA_FALSE))
							BROWSER_LOGE("ewk_view_setting_html5video_external_player_enable_set failed");
					}
				} else {
					if (!ewk_view_setting_accelerated_compositing_enable_set(webkit, EINA_FALSE))
						BROWSER_LOGE("ewk_view_setting_accelerated_compositing_enable_set failed");
					if (!ewk_view_setting_html5video_external_player_enable_set(webkit, EINA_TRUE))
						BROWSER_LOGE("ewk_view_setting_html5video_external_player_enable_set failed");
				}
			}
		}
	}else if (!strncmp(key, EXTERNAL_VIDEO_PLAYER_KEY, strlen(EXTERNAL_VIDEO_PLAYER_KEY))) {
		int external_video_player = 0;
		if (vconf_get_bool(EXTERNAL_VIDEO_PLAYER_KEY, &external_video_player) < 0)
			BROWSER_LOGE("Can not get [%s] value.\n", EXTERNAL_VIDEO_PLAYER_KEY);

		for (int i = 0 ; i < browser->m_window_list.size() ; i++) {
			if (browser->m_window_list[i]->m_ewk_view) {
				Evas_Object *webkit = elm_webview_webkit_get(browser->m_window_list[i]->m_ewk_view);
				if (external_video_player) {
					if (!ewk_view_setting_html5video_external_player_enable_set(webkit, EINA_TRUE))
						BROWSER_LOGE("ewk_view_setting_html5video_external_player_enable_set failed");
				} else {
					if (!ewk_view_setting_html5video_external_player_enable_set(webkit, EINA_FALSE))
						BROWSER_LOGE("ewk_view_setting_html5video_external_player_enable_set failed");
				}
			}
		}
	}

}

Eina_Bool Browser_Class::_set_ewk_view_options_listener(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (vconf_notify_key_changed(USERAGENT_KEY, __vconf_changed_cb, this) < 0) {
		BROWSER_LOGE("user agent vconf_notify_key_changed failed");
		return EINA_FALSE;
	}
	if (vconf_notify_key_changed(DEFAULT_VIEW_LEVEL_KEY, __vconf_changed_cb, this) < 0) {
		BROWSER_LOGE("default view level vconf_notify_key_changed failed");
		return EINA_FALSE;
	}
	if (vconf_notify_key_changed(RUN_JAVASCRIPT_KEY, __vconf_changed_cb, this) < 0) {
		BROWSER_LOGE("run javascript level vconf_notify_key_changed failed");
		return EINA_FALSE;
	}
	if (vconf_notify_key_changed(DISPLAY_IMAGES_KEY, __vconf_changed_cb, this) < 0) {
		BROWSER_LOGE("display images level vconf_notify_key_changed failed");
		return EINA_FALSE;
	}
	if (vconf_notify_key_changed(BLOCK_POPUP_KEY, __vconf_changed_cb, this) < 0) {
		BROWSER_LOGE("BLOCK_POPUP_KEY vconf_notify_key_changed failed");
		return EINA_FALSE;
	}
	if (vconf_notify_key_changed(ACCEPT_COOKIES_KEY, __vconf_changed_cb, this) < 0) {
		BROWSER_LOGE("ACCEPT_COOKIES_KEY vconf_notify_key_changed failed");
		return EINA_FALSE;
	}
	if (vconf_notify_key_changed(RUN_PLUGINS_KEY, __vconf_changed_cb, this) < 0) {
		BROWSER_LOGE("RUN_PLUGINS_KEY vconf_notify_key_changed failed");
		return EINA_FALSE;
	}
	if (vconf_notify_key_changed(ACCELERATED_COMPOSITION_KEY, __vconf_changed_cb, this) < 0) {
		BROWSER_LOGE("ACCELERATED_COMPOSITION_KEY vconf_notify_key_changed failed");
		return EINA_FALSE;
	}
	if (vconf_notify_key_changed(EXTERNAL_VIDEO_PLAYER_KEY, __vconf_changed_cb, this) < 0) {
		BROWSER_LOGE("EXTERNAL_VIDEO_PLAYER_KEY vconf_notify_key_changed failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool Browser_Class::_set_ewk_view_options(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!_set_user_agent(ewk_view))
		BROWSER_LOGE("_set_user_agent failed");

	char *default_level = vconf_get_str(DEFAULT_VIEW_LEVEL_KEY);
	if (default_level) {
		if (!strncmp(default_level, FIT_TO_WIDTH, strlen(FIT_TO_WIDTH)))
			elm_webview_auto_fitting_set(ewk_view, EINA_TRUE);
		else
			elm_webview_auto_fitting_set(ewk_view, EINA_FALSE);

		free(default_level);
	}

	int run_javascript = 1;
	if (vconf_get_bool(RUN_JAVASCRIPT_KEY, &run_javascript) < 0)
		BROWSER_LOGE("Can not get [%s] value.\n", RUN_JAVASCRIPT_KEY);
	Evas_Object *webkit = elm_webview_webkit_get(ewk_view);
	if (run_javascript) {
		if (!ewk_view_setting_enable_scripts_set(webkit, EINA_TRUE))
			BROWSER_LOGE("ewk_view_setting_enable_scripts_set failed");
	} else {
		if (!ewk_view_setting_enable_scripts_set(webkit, EINA_FALSE))
			BROWSER_LOGE("ewk_view_setting_enable_scripts_set failed");
	}

	int display_images = 1;
	if (vconf_get_bool(DISPLAY_IMAGES_KEY, &display_images) < 0)
		BROWSER_LOGE("Can not get [%s] value.\n", DISPLAY_IMAGES_KEY);
	if (display_images) {
		if (!ewk_view_setting_auto_load_images_set(webkit, EINA_TRUE))
			BROWSER_LOGE("ewk_view_setting_auto_load_images_set failed");
	} else {
		if (!ewk_view_setting_auto_load_images_set(webkit, EINA_FALSE))
			BROWSER_LOGE("ewk_view_setting_auto_load_images_set failed");
	}

	int block_popup = 1;
	if (vconf_get_bool(BLOCK_POPUP_KEY, &block_popup) < 0)
		BROWSER_LOGE("Can not get [%s] value.\n", BLOCK_POPUP_KEY);
	if (block_popup) {
		if (!ewk_view_setting_scripts_window_open_set(webkit, EINA_FALSE))
			BROWSER_LOGE("ewk_view_setting_scripts_window_open_set failed");
	} else {
		if (!ewk_view_setting_scripts_window_open_set(webkit, EINA_TRUE))
			BROWSER_LOGE("ewk_view_setting_auto_load_images_set failed");
	}

	int accept_cookies = 1;
	if (vconf_get_bool(ACCEPT_COOKIES_KEY, &accept_cookies) < 0)
		BROWSER_LOGE("Can not get [%s] value.\n", ACCEPT_COOKIES_KEY);
	if (accept_cookies)
		ewk_cookies_policy_set(EWK_COOKIE_JAR_ACCEPT_ALWAYS);
	else
		ewk_cookies_policy_set(EWK_COOKIE_JAR_ACCEPT_NEVER);

	int run_plugins = 1;
	if (vconf_get_bool(RUN_PLUGINS_KEY, &run_plugins) < 0)
		BROWSER_LOGE("Can not get [%s] value.\n", RUN_PLUGINS_KEY);
	if (run_plugins) {
		if (!ewk_view_setting_enable_plugins_set(webkit, EINA_TRUE))
			BROWSER_LOGE("ewk_view_setting_enable_plugins_set failed");
		if (!ewk_view_setting_enable_specified_plugin_set(webkit,
					EINA_TRUE, BROWSER_FLASH_MIME_TYPE)) {
			BROWSER_LOGE("ewk_view_setting_enable_specified_plugin_set failed");
		}
	} else {
		if (!ewk_view_setting_enable_plugins_set(webkit, EINA_FALSE))
			BROWSER_LOGE("ewk_view_setting_enable_plugins_set failed");
		if (!ewk_view_setting_enable_specified_plugin_set(webkit,
					EINA_FALSE, BROWSER_FLASH_MIME_TYPE)) {
			BROWSER_LOGE("ewk_view_setting_enable_specified_plugin_set failed");
		}
	}
	int accelerated_composition = 1;
	if (vconf_get_bool(ACCELERATED_COMPOSITION_KEY, &accelerated_composition) < 0)
		BROWSER_LOGE("Can not get [%s] value.\n", ACCELERATED_COMPOSITION_KEY);
	if (accelerated_composition) {
		if (!ewk_view_setting_accelerated_compositing_enable_set(webkit, EINA_TRUE))
			BROWSER_LOGE("ewk_view_setting_accelerated_compositing_enable_set failed");
/*		if (!ewk_view_setting_html5video_external_player_enable_set(webkit, EINA_FALSE)) {
			BROWSER_LOGE("ewk_view_setting_html5video_external_player_enable_set failed");
		}
		*/
	} else {
		if (!ewk_view_setting_accelerated_compositing_enable_set(webkit, EINA_FALSE))
			BROWSER_LOGE("ewk_view_setting_accelerated_compositing_enable_set failed");
		if (!ewk_view_setting_html5video_external_player_enable_set(webkit, EINA_TRUE)) {
			BROWSER_LOGE("ewk_view_setting_html5video_external_player_enable_set failed");
		}
	}

	int external_video_player = 1;
	if (vconf_get_bool(EXTERNAL_VIDEO_PLAYER_KEY, &external_video_player) < 0)
		BROWSER_LOGE("Can not get [%s] value.\n", EXTERNAL_VIDEO_PLAYER_KEY);
	if (external_video_player) {
		if (!ewk_view_setting_html5video_external_player_enable_set(webkit, EINA_TRUE))
				BROWSER_LOGE("ewk_view_setting_html5video_external_player_enable_set failed");
	} else {
		if (!ewk_view_setting_html5video_external_player_enable_set(webkit, EINA_FALSE))
			BROWSER_LOGE("ewk_view_setting_html5video_external_player_enable_set failed");
	}
	elm_webview_enable_default_context_menu_set(ewk_view, EINA_TRUE);
	elm_webview_auto_suspend_set(ewk_view, EINA_TRUE);
	elm_webview_use_mouse_down_delay_set(ewk_view, EINA_TRUE);
	elm_webview_show_ime_on_autofocus_set(ewk_view, EINA_FALSE);

	ewk_view_visibility_state_set(webkit, EWK_PAGE_VISIBILITY_STATE_VISIBLE, EINA_TRUE);

	if (!ewk_view_setting_geolocation_set(webkit, EINA_TRUE)) {
		BROWSER_LOGE("ewk_view_setting_geolocation_set failed");
		return EINA_FALSE;
	}
	if (!ewk_view_setting_enable_frame_flattening_set(webkit, EINA_TRUE)) {
		BROWSER_LOGE("ewk_view_setting_enable_frame_flattening_set failed");
		return EINA_FALSE;
	}
	if (!ewk_view_setting_local_storage_database_path_set(webkit, DATABASE_DIR)) {
		BROWSER_LOGE("ewk_view_setting_local_storage_database_path_set failed");
		return EINA_FALSE;
	}
	if (!ewk_view_setting_enable_onscroll_event_suppression_set(webkit, EINA_TRUE)) {
		BROWSER_LOGE("ewk_view_setting_enable_onscroll_event_suppression_set failed");
		return EINA_FALSE;
	}
	if (!_set_http_accepted_language_header(ewk_view)) {
		BROWSER_LOGE("_set_http_accepted_language_header failed");
		return EINA_FALSE;
	}

	m_browser_view->m_exscheme_handler->init(m_browser_view, ewk_view);

	if (!m_network_manager->init(m_browser_view, ewk_view)) {
		BROWSER_LOGE("m_network_manager->init failed");
		return EINA_FALSE;
	}
	
	/* If not debug mode, set the default setting. */
	if (!ewk_view_setting_recording_surface_enable_set(webkit, EINA_FALSE))
		BROWSER_LOGE("ewk_view_setting_recording_surface_enable_set failed");
		if (!ewk_view_setting_accelerated_compositing_enable_set(webkit, EINA_TRUE))
		BROWSER_LOGE("ewk_view_setting_accelerated_compositing_enable_set failed");
	if (!ewk_view_setting_html5video_external_player_enable_set(webkit, EINA_TRUE))
		BROWSER_LOGE("ewk_view_setting_html5video_external_player_enable_set failed");
	if (!ewk_view_setting_layer_borders_enable_set(webkit, EINA_FALSE))
		BROWSER_LOGE("ewk_view_setting_layer_borders_enable_set failed");

	Ewk_Tile_Unused_Cache *unused_cache = ewk_view_tiled_unused_cache_get(webkit);
	if (unused_cache)
		ewk_tile_unused_cache_max_set(unused_cache, BACKING_STORE_CACHE_SIZE);

	return EINA_TRUE;
}

Eina_Bool Browser_Class::_set_http_accepted_language_header(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

	char *system_language_locale = NULL;
	char system_language[3] = {0,};
	Evas_Object *webkit = NULL;

	webkit = elm_webview_webkit_get(ewk_view);
	system_language_locale = vconf_get_str("db/menu_widget/language");
	BROWSER_LOGD("system language and locale is [%s]\n", system_language_locale);
	if (!system_language_locale) {
		BROWSER_LOGD("Failed to get system_language, set as English");
		strncpy(system_language, "en", 2); /* Copy language set as english */
	} else {
		/* Copy language set from system using 2byte, ex)ko */
		strncpy(system_language, system_language_locale, 2);
	}

	if (system_language_locale)
		free(system_language_locale);

	if (!ewk_view_setting_custom_header_add(webkit, "Accept-Language", system_language)) {
		BROWSER_LOGD("ewk_view_setting_custom_header_add is failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

void Browser_Class::ewk_view_deinit(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

	evas_object_smart_callback_del(ewk_view, "edge,top", Browser_View::__ewk_view_edge_top_cb);

	Evas_Object *webkit = elm_webview_webkit_get(ewk_view);
	evas_object_event_callback_del(webkit, EVAS_CALLBACK_MOUSE_MOVE, Browser_View::__ewk_view_mouse_move_cb);
	evas_object_event_callback_del(webkit, EVAS_CALLBACK_MOUSE_DOWN, Browser_View::__ewk_view_mouse_down_cb);
	evas_object_event_callback_del(webkit, EVAS_CALLBACK_MOUSE_UP, Browser_View::__ewk_view_mouse_up_cb);
	evas_object_event_callback_del(webkit, EVAS_CALLBACK_MOUSE_UP, Browser_View::__ewk_view_multi_down_cb);

	evas_object_smart_callback_del(webkit, "uri,changed", Browser_View::__uri_changed_cb);
	evas_object_smart_callback_del(webkit, "load,started", Browser_View::__load_started_cb);
	evas_object_smart_callback_del(webkit, "load,progress", Browser_View::__load_progress_cb);
	evas_object_smart_callback_del(webkit, "load,finished", Browser_View::__load_finished_cb);
	evas_object_smart_callback_del(ewk_view_frame_main_get(webkit), "load,nonemptylayout,finished",
							Browser_View::__load_nonempty_layout_finished_cb);
	evas_object_smart_callback_del(webkit, "create,webview", Browser_View::__create_webview_cb);
	evas_object_smart_callback_del(webkit, "window,close", Browser_View::__window_close_cb);

	evas_object_smart_callback_del(ewk_view_frame_main_get(webkit), "html,boundary,reached",
							Browser_View::__html_boundary_reached_cb);
	evas_object_smart_callback_del(webkit, "html5video,request", Browser_View::__html5_video_request_cb);
	evas_object_smart_callback_del(webkit, "vibrator,vibrate", Browser_View::__vibrator_vibrate_cb);
	evas_object_smart_callback_del(webkit, "vibrator,cancel", Browser_View::__vibrator_cancel_cb);

	m_browser_view->suspend_webview(ewk_view);

	m_download_manager->deinit();
	m_notification_manager->deinit();

	m_browser_view->deinit_personal_data_manager();

	m_browser_view->m_picker_handler->deinit();

	m_browser_view->m_context_menu->deinit();
}

void Browser_Class::ewk_view_init(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

	ewk_view_deinit(ewk_view);

	evas_object_smart_callback_add(ewk_view, "edge,top",
					Browser_View::__ewk_view_edge_top_cb, m_browser_view);

	Evas_Object *webkit = elm_webview_webkit_get(ewk_view);
	evas_object_event_callback_add(webkit, EVAS_CALLBACK_MOUSE_MOVE,
					Browser_View::__ewk_view_mouse_move_cb, m_browser_view);
	evas_object_event_callback_add(webkit, EVAS_CALLBACK_MOUSE_DOWN,
					Browser_View::__ewk_view_mouse_down_cb, m_browser_view);
	evas_object_event_callback_add(webkit, EVAS_CALLBACK_MOUSE_UP,
					Browser_View::__ewk_view_mouse_up_cb, m_browser_view);
	evas_object_event_callback_add(webkit, EVAS_CALLBACK_MULTI_DOWN,
					Browser_View::__ewk_view_multi_down_cb, m_browser_view);

	evas_object_smart_callback_add(webkit, "uri,changed", Browser_View::__uri_changed_cb, m_browser_view);
	evas_object_smart_callback_add(webkit, "load,started", Browser_View::__load_started_cb, m_browser_view);
	evas_object_smart_callback_add(webkit, "load,progress", Browser_View::__load_progress_cb, m_browser_view);
	evas_object_smart_callback_add(webkit, "load,finished", Browser_View::__load_finished_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view_frame_main_get(webkit), "load,nonemptylayout,finished",
					Browser_View::__load_nonempty_layout_finished_cb, m_browser_view);
	evas_object_smart_callback_add(webkit, "create,webview", Browser_View::__create_webview_cb, m_browser_view);
	evas_object_smart_callback_add(webkit, "window,close", Browser_View::__window_close_cb, m_browser_view);
	evas_object_smart_callback_add(ewk_view_frame_main_get(webkit), "html,boundary,reached",
					Browser_View::__html_boundary_reached_cb, m_browser_view);
	evas_object_smart_callback_add(webkit, "html5video,request",
				Browser_View::__html5_video_request_cb, m_browser_view);
	evas_object_smart_callback_add(webkit, "vibrator,vibrate", Browser_View::__vibrator_vibrate_cb, NULL);
	evas_object_smart_callback_add(webkit, "vibrator,cancel", Browser_View::__vibrator_cancel_cb, NULL);

	m_browser_view->resume_webview(ewk_view);

	m_download_manager->init(ewk_view);
	m_notification_manager->init(ewk_view);

	m_browser_view->init_personal_data_manager(ewk_view);
	m_browser_view->m_picker_handler->init(ewk_view);

	if (!m_browser_view->m_context_menu->init(ewk_view))
		BROWSER_LOGE("m_context_menu->init failed");
}

std::string Browser_Class::get_user_agent(void)
{
	BROWSER_LOGD("[%s]", __func__);
	char *user_agent_title = vconf_get_str(USERAGENT_KEY);
	if (!user_agent_title) {
		BROWSER_LOGE("vconf_get_str(USERAGENT_KEY) failed.");
		user_agent_title = strdup(BROWSER_DEFAULT_USER_AGENT_TITLE);
		if (!user_agent_title) {
			BROWSER_LOGE("strdup(BROWSER_DEFAULT_USER_AGENT_TITLE) failed.");
			return std::string();
		}
	}

	char *user_agent = NULL;
	if (!m_user_agent_db->get_user_agent(user_agent_title, user_agent)) {
		BROWSER_LOGE("m_user_agent_db->get_user_agent failed");
	}

	if (user_agent_title)
		free(user_agent_title);

	if (user_agent) {
		std::string return_string = std::string(user_agent);
		free(user_agent);
		return return_string;
	} else
		return std::string();
}

Eina_Bool Browser_Class::_set_user_agent(Evas_Object *ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);

	char *user_agent_title = vconf_get_str(USERAGENT_KEY);
	if (!user_agent_title) {
		BROWSER_LOGE("vconf_get_str(USERAGENT_KEY) failed.");
		user_agent_title = strdup("Tizen");
		if (!user_agent_title) {
			BROWSER_LOGE("strdup(BROWSER_DEFAULT_USER_AGENT_TITLE) failed.");
			return EINA_FALSE;
		}
	}
#define TIZEN_USER_AGENT	"Mozilla/5.0 (Linux; U; Tizen 1.0; en-us) AppleWebKit/534.46 (KHTML, like Gecko) Mobile Tizen Browser/1.0"
#define CHROME_USER_AGENT	"Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.16 (KHTML, like Gecko) Chrome/18.0.1003.1 Safari/535.16"
#define FIREFOX_USER_AGENT	"Mozilla/5.0 (Windows NT 6.1; rv:9.0.1) Gecko/20100101 Firefox/9.0.1"
	if (!strncmp(user_agent_title, "Firefox", strlen("Firefox"))) {
		if (!ewk_view_setting_user_agent_set(elm_webview_webkit_get(ewk_view), FIREFOX_USER_AGENT));
			BROWSER_LOGE("ewk_view_setting_user_agent_set failed");
	} else if (!strncmp(user_agent_title, "Chrome", strlen("Chrome"))) {
		if (!ewk_view_setting_user_agent_set(elm_webview_webkit_get(ewk_view), CHROME_USER_AGENT));
			BROWSER_LOGE("ewk_view_setting_user_agent_set failed");
	} else {
		if (!ewk_view_setting_user_agent_set(elm_webview_webkit_get(ewk_view), TIZEN_USER_AGENT));
			BROWSER_LOGE("ewk_view_setting_user_agent_set failed");
	}

	return EINA_TRUE;
}

void Browser_Class::set_focused_window(Browser_Window *window, Eina_Bool show_most_visited_sites)
{
	if (m_focused_window == window || !window)
		return;

	if (m_focused_window) {
		if (m_focused_window->m_favicon)
			evas_object_hide(m_focused_window->m_favicon);
		if (m_focused_window->m_option_header_favicon)
			evas_object_hide(m_focused_window->m_option_header_favicon);
		if (m_focused_window->m_ewk_view) {
			ewk_view_deinit(m_focused_window->m_ewk_view);
			evas_object_hide(m_focused_window->m_ewk_view);
		}
	}

	m_focused_window = window;

	/* If the ewk view is deleted because of unused case.(etc. low memory)
	  * create the ewk view and load url. */
	if (!m_focused_window->m_ewk_view) {
		int index = 0;
		for (index = 0 ; index < m_window_list.size() ; index++) {
			if (m_focused_window == m_window_list[index])
				break;
		}

		if (m_focused_window != create_deleted_window(index))
			BROWSER_LOGD("create_deleted_window failed");

		/* Workaround.
		  * If launch the browser by aul, the grey bg is displayed shortly. */
		edje_object_signal_emit(elm_layout_edje_get(m_browser_view->m_main_layout),
						"hide,grey_background,signal", "");

		ewk_view_init(m_focused_window->m_ewk_view);

		m_browser_view->set_focused_window(m_focused_window, show_most_visited_sites);
		m_browser_view->load_url(m_focused_window->m_url.c_str());
	} else {
		ewk_view_init(m_focused_window->m_ewk_view);
		m_browser_view->set_focused_window(m_focused_window, show_most_visited_sites);
	}
}

void Browser_Class::change_order(std::vector<Browser_Window *> window_list)
{
	BROWSER_LOGD("[%s]", __func__);
	m_window_list.clear();
	m_window_list = window_list;
}

void Browser_Class::delete_window(Browser_Window *delete_window, Browser_Window *parent)
{
	BROWSER_LOGD("[%s]", __func__);

	int index = 0;
	for (index = 0 ; index < m_window_list.size() ; index++) {
		if (delete_window == m_window_list[index])
			break;
	}

	for (int i = 0 ; i < m_window_list.size() ; i++) {
		if (delete_window == m_window_list[i]->m_parent)
			m_window_list[i]->m_parent = NULL;
	}

	if (parent)
		set_focused_window(parent, EINA_FALSE);

	delete m_window_list[index];
	m_window_list.erase(m_window_list.begin() + index);

	m_browser_view->_set_multi_window_controlbar_text(m_window_list.size());
}

void Browser_Class::delete_window(Browser_Window *window)
{
	BROWSER_LOGD("[%s]", __func__);

	int index = 0;
	for (index = 0 ; index < m_window_list.size() ; index++) {
		if (window == m_window_list[index])
			break;
	}

	if (window == m_focused_window) {
		if (index == 0)
			set_focused_window(m_window_list[index + 1]);
		else
			set_focused_window(m_window_list[index - 1]);
	}

	for (int i = 0 ; i < m_window_list.size() ; i++) {
		if (window == m_window_list[i]->m_parent)
			m_window_list[i]->m_parent = NULL;
	}

	delete m_window_list[index];
	m_window_list.erase(m_window_list.begin() + index);

	m_browser_view->_set_multi_window_controlbar_text(m_window_list.size());
}

/* This destroy all windows except current focused window.
  * However, keep the snapshot and url, title. */
void Browser_Class::clean_up_windows(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_browser_view->m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW)) {
		BROWSER_LOGD("close multi window");
		m_browser_view->m_data_manager->get_multi_window_view()->close_multi_window();
	}

	for (int i = 0 ; i < m_window_list.size() ; i++) {
		if (m_focused_window != m_window_list[i]) {
			m_window_list[i]->m_url = m_browser_view->get_url(m_window_list[i]);
			m_window_list[i]->m_title = m_browser_view->get_title(m_window_list[i]);
			BROWSER_LOGD("delete [%d:%s] window,", i, m_window_list[i]->m_url.c_str());

			if (m_window_list[i]->m_favicon)
				evas_object_hide(m_window_list[i]->m_favicon);
			if (m_window_list[i]->m_option_header_favicon)
				evas_object_hide(m_window_list[i]->m_option_header_favicon);

			if (m_window_list[i]->m_ewk_view) {
				evas_object_del(m_window_list[i]->m_ewk_view);
				m_window_list[i]->m_ewk_view = NULL;
			}
		}
	}

	/* Clear memory cache to reduce memory usage in case of low memory. */
	/* To do */
//	m_browser_view->show_msg_popup("This is a test message. Low memory - clean up windows.", 5);
}

Browser_Window *Browser_Class::create_deleted_window(int index)
{
	if (m_window_list[index]->m_ewk_view == NULL) {
		m_window_list[index]->m_ewk_view = elm_webview_add(m_win, EINA_TRUE);

		if (!m_window_list[index]->m_ewk_view) {
			BROWSER_LOGE("ewk_view_add failed");
			return NULL;
		}

		evas_object_color_set(m_window_list[index]->m_ewk_view, 255, 255, 255, 255);
		/* The webview is locked initially. */
		elm_webview_vertical_panning_hold_set(m_window_list[index]->m_ewk_view, EINA_TRUE);

		evas_object_size_hint_weight_set(m_window_list[index]->m_ewk_view,
						EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(m_window_list[index]->m_ewk_view,
						EVAS_HINT_FILL, EVAS_HINT_FILL);

		if (!_set_ewk_view_options(m_window_list[index]->m_ewk_view))
			BROWSER_LOGE("_set_ewk_view_options failed");

		return m_window_list[index];
	}

	return NULL;
}

Browser_Window *Browser_Class::create_new_window(Eina_Bool created_by_user)
{
	BROWSER_LOGD("[%s]", __func__);
	Browser_Window *window = new(nothrow) Browser_Window;
	if (!window) {
		BROWSER_LOGE("new Browser_Window failed");
		return NULL;
	}

	 window->m_ewk_view = elm_webview_add(m_win, EINA_TRUE);

	if (!window->m_ewk_view) {
		BROWSER_LOGE("ewk_view_add failed");
		return NULL;
	}

	evas_object_color_set(window->m_ewk_view, 255, 255, 255, 255);
	if (created_by_user)
		window->m_created_by_user = created_by_user;

	elm_webview_vertical_panning_hold_set(window->m_ewk_view, EINA_TRUE);
	elm_object_focus_allow_set(window->m_ewk_view, EINA_FALSE);

	evas_object_size_hint_weight_set(window->m_ewk_view, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(window->m_ewk_view, EVAS_HINT_FILL, EVAS_HINT_FILL);

	if (!_set_ewk_view_options(window->m_ewk_view))
		BROWSER_LOGE("_set_ewk_view_options failed");

	m_window_list.push_back(window);

	/* Workaround.
	  * If launch the browser by aul, the grey bg is displayed shortly. */
	edje_object_signal_emit(elm_layout_edje_get(m_browser_view->m_main_layout),
					"hide,grey_background,signal", "");
	return window;
}

Eina_Bool Browser_Class::launch(const char *url, Eina_Bool new_window_flag)
{
	BROWSER_LOGD("url=[%s], new_window=%d", url, new_window_flag);

	m_browser_view->delete_non_user_created_windows();

	if (new_window_flag && m_window_list.size() >= BROWSER_MULTI_WINDOW_MAX_COUNT)
		/* If the multi window is max, delete the first one in case of new window. */
		delete_window(m_window_list[0]);

	if (m_window_list.size() == 0 || new_window_flag) {
		Browser_Window *new_window = NULL;
		/* If browser is launched for the first time, create the first window. */
		if (m_window_list.size() == 0)
			new_window = create_new_window(EINA_TRUE);
		else
			new_window = create_new_window(EINA_FALSE);

		if (!new_window) {
			BROWSER_LOGD("create_new_window failed");
			return EINA_FALSE;
		}

		set_focused_window(new_window);
	}

	if (m_window_list.size())
		m_browser_view->launch(url);

	return EINA_TRUE;
}

Eina_Bool Browser_Class::__clean_up_windows_timer_cb(void *data)
{
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	BROWSER_LOGD("[%s]", __func__);

	Browser_Class *browser = (Browser_Class *)data;
	browser->m_clean_up_windows_timer = NULL;

	browser->clean_up_windows();
	return ECORE_CALLBACK_CANCEL;
}

void Browser_Class::pause(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_browser_view->pause();

	if (m_clean_up_windows_timer)
		ecore_timer_del(m_clean_up_windows_timer);
	m_clean_up_windows_timer = ecore_timer_add(BROWSER_CLEAN_UP_WINDOWS_TIMEOUT,
						__clean_up_windows_timer_cb, this);
}

void Browser_Class::resume(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_browser_view->resume();

	if (m_clean_up_windows_timer) {
		ecore_timer_del(m_clean_up_windows_timer);
		m_clean_up_windows_timer = NULL;
	}
}

void Browser_Class::reset(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_clean_up_windows_timer) {
		ecore_timer_del(m_clean_up_windows_timer);
		m_clean_up_windows_timer = NULL;
	}

	m_browser_view->reset();
}

