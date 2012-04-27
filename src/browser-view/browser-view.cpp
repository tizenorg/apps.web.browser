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

#include "browser-add-to-bookmark-view.h"
#include "browser-bookmark-view.h"
#include "browser-certificate-manager.h"
#include "browser-class.h"
#include "browser-context-menu.h"
#include "browser-exscheme-handler.h"
#include "browser-history-db.h"
#include "browser-find-word.h"
#include "browser-multi-window-view.h"
#include "browser-personal-data-manager.h"
#include "browser-picker-handler.h"
#include "browser-predictive-history.h"
#include "browser-settings-class.h"
#include "browser-string.h"
#include "browser-view.h"
#include "browser-window.h"
#include <devman.h>
#include <url_download.h>

Browser_View::Browser_View(Evas_Object *win, Evas_Object *navi_bar,
						Evas_Object *bg, Evas_Object *layout, Browser_Class *browser)
:
	m_main_layout(NULL)
	,m_scroller(NULL)
	,m_content_box(NULL)
	,m_dummy_loading_progressbar(NULL)
	,m_conformant(NULL)
	,m_url_layout(NULL)
	,m_url_entry_layout(NULL)
	,m_url_edit_field(NULL)
	,m_url_progressbar(NULL)
	,m_url_progresswheel(NULL)
	,m_cancel_button(NULL)
	,m_control_bar(NULL)
	,m_backward_button(NULL)
	,m_forward_button(NULL)
	,m_add_bookmark_button(NULL)
	,m_more_button(NULL)
	,m_option_header_url_layout(NULL)
	,m_option_header_layout(NULL)
	,m_option_header_url_entry_layout(NULL)
	,m_option_header_cancel_button(NULL)
	,m_option_header_url_progressbar(NULL)
	,m_option_header_url_progresswheel(NULL)
	,m_edit_mode(BR_NO_EDIT_MODE)
	,m_homepage_mode(BR_START_MODE_UNKOWN)
	,m_scroller_region_y(0)
	,m_is_scrolling(EINA_FALSE)
	,m_focused_window(NULL)
	,m_more_context_popup(NULL)
	,m_predictive_history(NULL)
	,m_new_window_transit(NULL)
	,m_created_new_window(NULL)
	,m_browser_settings(NULL)
	,m_navi_it(NULL)
	,m_find_word(NULL)
	,m_is_scroll_up(EINA_FALSE)
	,m_is_multi_touch(EINA_FALSE)
	,m_context_menu(NULL)
	,m_exscheme_handler(NULL)
	,m_personal_data_manager(NULL)
	,m_picker_handler(NULL)
	,m_share_controlbar_button(NULL)
	,m_add_to_home_control_bar(NULL)
	,m_multi_window_button(NULL)
	,m_zoom_in_button(NULL)
	,m_zoom_out_button(NULL)
	,m_zoom_button_timer(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
	m_win = win;
	m_navi_bar = navi_bar;
	m_bg = bg;
	m_layout = layout;
	m_browser = browser;
}

Browser_View::~Browser_View()
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_title_back_button)
		evas_object_del(m_title_back_button);

	ug_destroy_all();

	if (vconf_set_str(LAST_VISITED_URL_KEY, m_last_visited_url.c_str()) != 0)
		BROWSER_LOGE("vconf_set_str failed");

	m_data_manager->destroy_history_db();

	if (m_data_manager) {
		delete m_data_manager;
		m_data_manager = NULL;
	}
	if (m_predictive_history) {
		delete m_predictive_history;
		m_predictive_history = NULL;
	}
	if (m_new_window_transit) {
		elm_transit_del(m_new_window_transit);
		m_new_window_transit = NULL;
	}
	if (m_browser_settings) {
		delete m_browser_settings;
		m_browser_settings = NULL;
	}
	if (m_find_word) {
		delete m_find_word;
		m_find_word = NULL;
	}
	if (m_context_menu) {
		delete m_context_menu;
		m_context_menu = NULL;
	}
	if (m_exscheme_handler) {
		delete m_exscheme_handler;
		m_exscheme_handler = NULL;
	}
	if (m_personal_data_manager) {
		delete m_personal_data_manager;
		m_personal_data_manager = NULL;
	}
	if (m_picker_handler) {
		delete m_picker_handler;
		m_picker_handler = NULL;
	}
}

Eina_Bool Browser_View::init(void)
{
	BROWSER_LOGD("[%s]", __func__);
	/* set homepage from homepage vconf */
	_set_homepage_mode();

	m_data_manager = new(nothrow) Browser_Data_Manager;
	if (!m_data_manager) {
		BROWSER_LOGE("new Browser_Data_Manager failed");
		return EINA_FALSE;
	}

	m_data_manager->set_browser_view(this);

	if (!m_data_manager->create_history_db()) {
		BROWSER_LOGE("m_data_manager->create_history_db failed");
		return EINA_FALSE;
	}
	m_find_word = new(nothrow) Browser_Find_Word(this);
	if (!m_find_word) {
		BROWSER_LOGE("new Browser_Find_Word failed");
		return EINA_FALSE;
	}

	m_context_menu = new(nothrow) Browser_Context_Menu(m_navi_bar, this);
	if (!m_context_menu) {
		BROWSER_LOGE("new Browser_Context_Menu failed");
		return EINA_FALSE;
	}

	m_exscheme_handler = new(nothrow) Browser_Exscheme_Handler();
	if (!m_exscheme_handler) {
		BROWSER_LOGE("new Browser_Exscheme_Handler failed");
		return EINA_FALSE;
	}
	m_personal_data_manager = new(nothrow) Browser_Personal_Data_Manager();
	if (!m_personal_data_manager) {
		BROWSER_LOGE("new Browser_Personal_Data_Manager failed");
		return EINA_FALSE;
	}
	m_picker_handler = new(nothrow) Browser_Picker_Handler(this);
	if (!m_picker_handler) {
		BROWSER_LOGE("new Browser_Picker_Handler failed");
		return EINA_FALSE;
	}
	UG_INIT_EFL(m_win, UG_OPT_INDICATOR_ENABLE);

	char *last_url = vconf_get_str(LAST_VISITED_URL_KEY);
	if (last_url) {
		m_last_visited_url = std::string(last_url);
		free(last_url);
	}

	/* create brower view layout */
	return _create_main_layout();
}

void Browser_View::stop_and_reload(void)
{
	BROWSER_LOGD("[%s]_is_loading=%d", __func__, _is_loading());
	if (_is_loading()) {
		_stop_loading();
		_reload();
	}
}

Evas_Object *Browser_View::get_focused_webview(void)
{
	return m_focused_window->m_ewk_view;
}

Eina_Bool Browser_View::_activate_url_entry_idler_cb(void *data)
{
	Browser_View* instance = (Browser_View*)data;

	instance->__url_entry_clicked_cb(instance, NULL, NULL, NULL);
	return ECORE_CALLBACK_CANCEL;
}
void Browser_View::launch(const char *url)
{
	/* Destroy all other views except browser view. */
	_pop_other_views();

	/*Workaround.
	  * When keypad is running via url entry, if browser goes to background by home key.
	  * Then relaunch the browser by aul. The webpage is loading but the keypad is still running.
	  * So give focus to cancel button not to invoke the keypad. */
	elm_object_focus_set(m_cancel_button, EINA_TRUE);

	if (url && strlen(url)) {
		load_url(url);
	 } else if (m_homepage_mode == BR_START_MODE_MOST_VISITED_SITES){
		load_url(BROWSER_MOST_VISITED_SITES_URL);
		ecore_idler_add(_activate_url_entry_idler_cb, this);
	 } else if (m_homepage_mode == BR_START_MODE_RECENTLY_VISITED_SITE) {
		char *homepage = vconf_get_str(LAST_VISITED_URL_KEY);
		if (homepage) {
			load_url(homepage);
			free(homepage);
		} else
			load_url(BROWSER_MOST_VISITED_SITES_URL);
	} else if (m_homepage_mode == BR_START_MODE_CUSTOMIZED_URL) {
		char *user_homepage = vconf_get_str(USER_HOMEPAGE_KEY);
		if (user_homepage) {
			load_url(user_homepage);
			free(user_homepage);
		} else
			load_url(BROWSER_MOST_VISITED_SITES_URL);
	}
}

void Browser_View::return_to_browser_view(Eina_Bool saved_most_visited_sites_item)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!_is_loading()) {
		if (get_title().empty())
			_set_navigationbar_title(get_url().c_str());
		else
			_set_navigationbar_title(get_title().c_str());
	}

	_set_secure_icon();

}

void Browser_View::init_personal_data_manager(Evas_Object *webview)
{
	BROWSER_LOGD("[%s]", __func__);
	m_personal_data_manager->init(webview);
}

void Browser_View::deinit_personal_data_manager(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_personal_data_manager->deinit();
}

void Browser_View::_pop_other_views(void)
{
	BROWSER_LOGD("[%s]", __func__);
	_destroy_more_context_popup();

	/* if browser is runing background behind ug which browser invoked,
	destroy the previous ug and show the browser view. */
	ug_destroy_all();

	/* If multi-window is running, destroy it to show browser view. */
	if (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW))
		m_data_manager->get_multi_window_view()->close_multi_window();

	/* Pop all other views except browser view. */
	elm_naviframe_item_pop_to(m_navi_it);

}

/* set homepage from homepage vconf */
void Browser_View::_set_homepage_mode(void)
{
	char *homepage = vconf_get_str(HOMEPAGE_KEY);
	BROWSER_LOGD("homepage=[%s]", homepage);

	if (!homepage) {
		BROWSER_LOGE("homepage is null");
		m_homepage_mode = BR_START_MODE_MOST_VISITED_SITES;
		return;
	}

	if (!strncmp(homepage, MOST_VISITED_SITES,
				strlen(MOST_VISITED_SITES)))
		m_homepage_mode = BR_START_MODE_MOST_VISITED_SITES;
	else if (!strncmp(homepage, RECENTLY_VISITED_SITE,
			strlen(RECENTLY_VISITED_SITE)))
		m_homepage_mode = BR_START_MODE_RECENTLY_VISITED_SITE;
	else
		m_homepage_mode = BR_START_MODE_CUSTOMIZED_URL;

	free(homepage);
	BROWSER_LOGD("m_homepage_mode=%d", m_homepage_mode);
}

void Browser_View::__new_window_transit_finished_cb(void *data, Elm_Transit *transit)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	if (!browser_view->m_new_window_transit)
		return;

	browser_view->m_new_window_transit = NULL;
	browser_view->m_browser->set_focused_window(browser_view->m_created_new_window, EINA_FALSE);
	browser_view->m_created_new_window = NULL;
}

Eina_Bool Browser_View::_show_new_window_effect(Evas_Object *current_ewk_view,
								Evas_Object *new_ewk_view)
{
	BROWSER_LOGD("[%s]", __func__);
	m_new_window_transit = elm_transit_add();
	if (!m_new_window_transit) {
		BROWSER_LOGE("elm_transit_add failed");
		return EINA_FALSE;
	}
	int scroller_x = 0;
	int scroller_w = 0;
	int scroller_h = 0;
	elm_scroller_region_get(m_scroller, &scroller_x, NULL, &scroller_w, &scroller_h);
	elm_scroller_region_show(m_scroller ,scroller_x, 0, scroller_w, scroller_h);

	/* Block event durring animation. */
	elm_transit_event_enabled_set(m_new_window_transit, EINA_TRUE);

	int url_layout_h = 0;
	int url_layout_y = 0;
	evas_object_geometry_get(m_url_layout, NULL, &url_layout_y, NULL, &url_layout_h);

	int current_ewk_view_x = 0;
	int current_ewk_view_w = 0;
	int current_ewk_view_h = 0;
	evas_object_geometry_get(current_ewk_view, &current_ewk_view_x, NULL,
								&current_ewk_view_w, &current_ewk_view_h);

	evas_object_resize(new_ewk_view, current_ewk_view_w, current_ewk_view_h - url_layout_h);
	evas_object_move(new_ewk_view, current_ewk_view_x + current_ewk_view_w, url_layout_y + url_layout_h);
	evas_object_show(new_ewk_view);

	elm_transit_object_add(m_new_window_transit, current_ewk_view);
	elm_transit_object_add(m_new_window_transit, new_ewk_view);
	elm_transit_tween_mode_set(m_new_window_transit, ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL);
	elm_transit_objects_final_state_keep_set(m_new_window_transit, EINA_FALSE);
	elm_transit_effect_translation_add(m_new_window_transit, 0, 0, (-1) * current_ewk_view_w, 0);
	elm_transit_del_cb_set(m_new_window_transit, __new_window_transit_finished_cb, this);
	elm_transit_duration_set(m_new_window_transit, 0.3);
	elm_transit_go(m_new_window_transit);

	return EINA_TRUE;
}

Evas_Object *Browser_View::get_favicon(const char *url)
{
	if (!url || !strlen(url))
		return NULL;

	Evas_Object *favicon = ewk_settings_icon_database_icon_object_add(url, evas_object_evas_get(m_win));
	return favicon;
}

Eina_Bool Browser_View::_set_favicon(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_edit_mode == BR_URL_ENTRY_EDIT_MODE
	    || m_edit_mode == BR_URL_ENTRY_EDIT_MODE_WITH_NO_IMF) {
		BROWSER_LOGD("Edit mode");
		return EINA_FALSE;
	}

	if (get_url().empty())
		return EINA_FALSE;

	double progress = ewk_view_load_progress_get(elm_webview_webkit_get(m_focused_window->m_ewk_view));
	if (progress < 1.0f && progress > 0.05f) {
		BROWSER_LOGD("loadin status");
		return EINA_FALSE;
	}

	Evas_Object *favicon = ewk_settings_icon_database_icon_object_add(get_url().c_str(),
								evas_object_evas_get(m_win));
	Evas_Object *option_header_favicon = ewk_settings_icon_database_icon_object_add(get_url().c_str(),
									evas_object_evas_get(m_win));
	if (favicon) {
		if (m_focused_window->m_favicon)
			evas_object_del(m_focused_window->m_favicon);

		m_focused_window->m_favicon = favicon;
	} else {
		Evas_Object *default_icon = elm_icon_add(m_focused_window->m_ewk_view);
		if (!default_icon) {
			BROWSER_LOGE("elm_icon_add is failed.");
			return EINA_FALSE;
		}
		if (!elm_icon_file_set(default_icon, BROWSER_IMAGE_DIR"/faviconDefault.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.");
			return EINA_FALSE;
		}
		evas_object_size_hint_aspect_set(default_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		if (m_focused_window->m_favicon)
			evas_object_del(m_focused_window->m_favicon);

		m_focused_window->m_favicon = default_icon;
	}

	if (option_header_favicon) {
		if (m_focused_window->m_option_header_favicon)
			evas_object_del(m_focused_window->m_option_header_favicon);

		m_focused_window->m_option_header_favicon = option_header_favicon;
	} else {
		Evas_Object *default_icon = elm_icon_add(m_focused_window->m_ewk_view);
		if (!default_icon) {
			BROWSER_LOGE("elm_icon_add is failed.");
			return EINA_FALSE;
		}
		if (!elm_icon_file_set(default_icon, BROWSER_IMAGE_DIR"/faviconDefault.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.");
			return EINA_FALSE;
		}
		evas_object_size_hint_aspect_set(default_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		if (m_focused_window->m_option_header_favicon)
			evas_object_del(m_focused_window->m_option_header_favicon);

		m_focused_window->m_option_header_favicon = default_icon;
	}

	if (elm_object_part_content_get(m_url_entry_layout, "elm.swallow.favicon"))
		elm_object_part_content_unset(m_url_entry_layout, "elm.swallow.favicon");
	if (elm_object_part_content_get(m_option_header_url_entry_layout,
						"elm.swallow.favicon"))
		elm_object_part_content_unset(m_option_header_url_entry_layout,
						"elm.swallow.favicon");

	elm_object_part_content_set(m_url_entry_layout, "elm.swallow.favicon",
							m_focused_window->m_favicon);
	elm_object_part_content_set(m_option_header_url_entry_layout, "elm.swallow.favicon",
							m_focused_window->m_option_header_favicon);

	edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout),
								"show,favicon,signal", "");
	edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
								"show,favicon,signal", "");
	return EINA_TRUE;
}

Eina_Bool Browser_View::__close_window_idler_cb(void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	Browser_View *browser_view = (Browser_View *)data;
	Browser_Class *browser = browser_view->m_browser;

	if (browser_view->m_focused_window->m_parent)
		browser->delete_window(browser_view->m_focused_window,
				browser_view->m_focused_window->m_parent);
	else
		browser->delete_window(browser_view->m_focused_window);

	return ECORE_CALLBACK_CANCEL;
}

void Browser_View::__html_boundary_reached_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->show_msg_popup(BR_STRING_MSG_BOUNDARY_LACK_OF_SPACE_TO_SAVE_HTML);
}

void Browser_View::__window_close_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Browser_Class *browser = browser_view->m_browser;

	/* The window should be deleted by idler. The webkit recommands to delete by idler. */
	ecore_idler_add(__close_window_idler_cb, browser_view);
}

/*
 * navigation_action type :
 * WEBKIT_WEB_NAVIGATION_REASON_LINK_CLICKED,
 * WEBKIT_WEB_NAVIGATION_REASON_FORM_SUBMITTED,
 * WEBKIT_WEB_NAVIGATION_REASON_BACK_FORWARD,
 * WEBKIT_WEB_NAVIGATION_REASON_RELOAD,
 * WEBKIT_WEB_NAVIGATION_REASON_FORM_RESUBMITTED,
 * WEBKIT_WEB_NAVIGATION_REASON_OTHER,
*/
void Browser_View::__create_webview_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Browser_Class *browser = browser_view->m_browser;

	std::vector<Browser_Window *> window_list = browser->get_window_list();
	if (window_list.size() >= BROWSER_MULTI_WINDOW_MAX_COUNT) {
		/* If the multi window is max, delete the first one. */
		if (browser_view->m_focused_window != window_list[0])
			browser->delete_window(window_list[0]);
		else
			browser->delete_window(window_list[BROWSER_MULTI_WINDOW_MAX_COUNT - 1]);
	}

	Elm_WebView_Create_Webview_Data *create_webview_data = (Elm_WebView_Create_Webview_Data *)event_info;
	if (create_webview_data->navigation_action == 5 && create_webview_data->javascript) {
		int block_popup = 1;
		const char *msg = BR_STRING_DISPLAY_POPUP_Q;
		if (vconf_get_bool(BLOCK_POPUP_KEY, &block_popup) < 0)
			BROWSER_LOGE("vconf_get_bool BLOCK_POPUP_KEY failed");
		if (!block_popup) {
			if (!browser_view->show_modal_popup(msg))
				return;
		}
	}

	browser_view->m_created_new_window = browser->create_new_window();
	if (!browser_view->m_created_new_window) {
		BROWSER_LOGE("create_new_window failed");
		return;
	}
	/* initialize the created webview first to connect ewk event callback functions such as load start, progress etc. */
	browser->ewk_view_init(browser_view->m_created_new_window->m_ewk_view);

	/* Set the caller window. */
	browser_view->m_created_new_window->m_parent = browser_view->m_focused_window;

	/* Destroy previous multi window item snapshot.
	  * This is because the snapshot of multi window item can't be captured in this case. */
	if (browser_view->m_focused_window->m_landscape_snapshot_image) {
		evas_object_del(browser_view->m_focused_window->m_landscape_snapshot_image);
		browser_view->m_focused_window->m_landscape_snapshot_image = NULL;
	}
	if (browser_view->m_focused_window->m_portrait_snapshot_image) {
		evas_object_del(browser_view->m_focused_window->m_portrait_snapshot_image);
		browser_view->m_focused_window->m_portrait_snapshot_image = NULL;
	}

	create_webview_data->webview = elm_webview_webkit_get(browser_view->m_created_new_window->m_ewk_view);

	if (!browser_view->_show_new_window_effect(browser_view->m_focused_window->m_ewk_view,
					browser_view->m_created_new_window->m_ewk_view))
		BROWSER_LOGE("_show_new_window_effect failed");
}

void Browser_View::_navigationbar_visible_set_signal(Eina_Bool visible)
{
	BROWSER_LOGD("visible=%d", visible);

	if (visible && m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW))
		return;

	if (m_navi_it != elm_naviframe_top_item_get(m_navi_bar))
		return;

	evas_object_data_set(m_navi_bar, "visible", (Eina_Bool *)visible);

	Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);

	if (visible)
		elm_object_item_signal_emit(top_it, ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_INSTANT_OPEN);
	else
		elm_object_item_signal_emit(top_it, ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_INSTANT_CLOSE);
}

void Browser_View::_navigationbar_visible_set(Eina_Bool visible)
{
	if (visible && m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW))
		return;

	if (m_navi_it != elm_naviframe_top_item_get(m_navi_bar))
		return;

	evas_object_data_set(m_navi_bar, "visible", (Eina_Bool *)visible);

	Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);
	if (visible)
		elm_object_item_signal_emit(top_it, ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_OPEN);
	else
		elm_object_item_signal_emit(top_it, ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_CLOSE);
}

Eina_Bool Browser_View::_navigationbar_visible_get(void)
{
	Eina_Bool* visible = (Eina_Bool *)evas_object_data_get(m_navi_bar, "visible");
	BROWSER_LOGD("visible = %d", visible);
	if (visible == (Eina_Bool *)EINA_TRUE)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

void Browser_View::_load_start(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_dummy_loading_progressbar) {
		elm_object_part_content_unset(m_main_layout, "elm.swallow.waiting_progress");
		evas_object_del(m_dummy_loading_progressbar);
		m_dummy_loading_progressbar = NULL;
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
					"hide,waiting_progressbar,signal", "");
	}

	if (!m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW))
		_set_navigationbar_title(BR_STRING_LOADING);

	edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "loading,on,signal", "");
	edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
				"loading,on,signal", "");

	_navigationbar_visible_set_signal(EINA_TRUE);

	/* Hide the secure lock icon in title bar. */
	elm_object_item_part_content_set(m_navi_it, ELM_NAVIFRAME_ITEM_ICON, NULL);

	/* Destroy & hide favicon when load start. */
	edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout),
						"hide,favicon,signal", "");
	edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
						"hide,favicon,signal", "");
	if (elm_object_part_content_get(m_url_entry_layout, "elm.swallow.favicon"))
		elm_object_part_content_unset(m_url_entry_layout, "elm.swallow.favicon");
	if (elm_object_part_content_get(m_option_header_url_entry_layout,
						"elm.swallow.favicon"))
		elm_object_part_content_unset(m_option_header_url_entry_layout,
						"elm.swallow.favicon");

	if (m_focused_window->m_favicon) {
		evas_object_del(m_focused_window->m_favicon);
		m_focused_window->m_favicon = NULL;
	}
	if (m_focused_window->m_option_header_favicon) {
		evas_object_del(m_focused_window->m_option_header_favicon);
		m_focused_window->m_option_header_favicon = NULL;
	}

	if (_get_edit_mode() != BR_NO_EDIT_MODE)
		_set_edit_mode(BR_NO_EDIT_MODE);
	/* For deleted window because of unused case like low memory. */
	m_focused_window->m_url.clear();
	m_focused_window->m_title.clear();

	m_browser->get_certificate_manager()->reset_certificate();

	m_picker_handler->destroy_picker_layout();
}

void Browser_View::__uri_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	if (browser_view->_get_edit_mode() != BR_NO_EDIT_MODE)
		return;
	const char *uri = (const char *)event_info;
	browser_view->_set_url_entry(uri);

	/* Workaround, give focus to option header cancel button to hide imf. */
	elm_object_focus_set(browser_view->m_option_header_cancel_button, EINA_TRUE);
}

void Browser_View::__load_started_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	Evas_Object *url_progressbar = browser_view->m_url_progressbar;
	Evas_Object *progressbar_wheel = browser_view->m_url_progresswheel;
	Evas_Object *option_header_url_progressbar = browser_view->m_option_header_url_progressbar;
	Evas_Object *option_header_progressbar_wheel = browser_view->m_option_header_url_progresswheel;

	double progress = ewk_view_load_progress_get(elm_webview_webkit_get(browser_view->m_focused_window->m_ewk_view));
	if (progress <= 0.0f)
		progress = 0.05f;
	elm_progressbar_pulse(progressbar_wheel, EINA_TRUE);
	elm_progressbar_pulse(option_header_progressbar_wheel, EINA_TRUE);

	elm_progressbar_value_set(url_progressbar, progress);
	elm_progressbar_value_set(option_header_url_progressbar, progress);

	browser_view->_load_start();
}

void Browser_View::__load_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Evas_Object *progressbar_wheel = browser_view->m_url_progresswheel;
	Evas_Object *option_header_progressbar_wheel = browser_view->m_option_header_url_progresswheel;

	elm_progressbar_pulse(progressbar_wheel, EINA_FALSE);
	elm_progressbar_pulse(option_header_progressbar_wheel, EINA_FALSE);

	if (browser_view->m_edit_mode != BR_URL_ENTRY_EDIT_MODE
	    && browser_view->m_edit_mode != BR_URL_ENTRY_EDIT_MODE_WITH_NO_IMF) {
		/* change the url layout for normal mode. (change the reload icon etc) */
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_url_entry_layout),
									"loading,off,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_option_header_url_entry_layout),
					"loading,off,signal", "");
	}

	if (!m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW)) {
		if (browser_view->get_title().empty())
			browser_view->_set_navigationbar_title(browser_view->get_url().c_str());
		else
			browser_view->_set_navigationbar_title(browser_view->get_title().c_str());
	}

	browser_view->_set_secure_icon();
	if (!browser_view->_set_favicon())
		BROWSER_LOGE("_set_favicon failed");

	browser_view->_load_finished();
}

void Browser_View::__load_progress_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	Evas_Object *url_progressbar = browser_view->m_url_progressbar;
	Evas_Object *option_header_url_progressbar = browser_view->m_option_header_url_progressbar;

	double progress = *((double *)event_info);
	BROWSER_LOGD("progress=%f", progress);
	elm_progressbar_value_set(url_progressbar, progress);
	elm_progressbar_value_set(option_header_url_progressbar, progress);
}

void Browser_View::__load_nonempty_layout_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	/* If the first content is displayed, hide the url layout in browser scroller like safari. */
	int scroller_x = 0;
	int scroller_w = 0;
	int scroller_h = 0;
	elm_scroller_region_get(browser_view->m_scroller, &scroller_x, NULL, &scroller_w, &scroller_h);
	elm_scroller_region_show(browser_view->m_scroller ,scroller_x, 0,
								scroller_w, scroller_h);
}

void Browser_View::__html5_video_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data || !event_info)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	html5_video_data *video_data = (html5_video_data *)event_info;

	if (!browser_view->_call_html5_video_streaming_player(video_data->path, video_data->cookie))
		BROWSER_LOGE("_call_html5_video_streaming_player");
}

void Browser_View::__vibrator_vibrate_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	int device_handle = 0;
	int ret_val = 0;
	const long vibration_time = *((const long *)event_info);

	BROWSER_LOGD("__vibrator_vibrate_cb : play time is [%ld]", vibration_time);

	device_handle = device_haptic_open(DEV_IDX_0, 0);

	if (device_handle < 0) {
		BROWSER_LOGD("Failed to get handle ID of vibration device");
		return;
	}

	ret_val = device_haptic_play_monotone(device_handle, vibration_time);
	if (ret_val != 0)
		BROWSER_LOGD("Failed to play vibration");

	ret_val = device_haptic_close(device_handle);
	if (ret_val != 0)
		BROWSER_LOGD("Failed to withdraw vibration handle");
}

void Browser_View::__vibrator_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	int device_handle = 0;
	int ret_val = 0;

	device_handle = device_haptic_open(DEV_IDX_0, 0);

	if (device_handle < 0) {
		BROWSER_LOGD("Failed to get handle ID of vibration device");
		return;
	}

	ret_val = device_haptic_stop_play(device_handle);
	if (ret_val != 0)
		BROWSER_LOGD("Failed to stop vibration");

	ret_val = device_haptic_close(device_handle);
	if (ret_val != 0)
		BROWSER_LOGD("Failed to withdraw vibration handle");
}

Eina_Bool Browser_View::_call_html5_video_streaming_player(const char *url, const char *cookie)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!url || !strlen(url)) {
		BROWSER_LOGE("url is null");
		return EINA_FALSE;
	}

	bool is_running = false;
	if (app_manager_is_running(SEC_VT_CALL, &is_running)) {
			BROWSER_LOGE("Fail to get app running information\n");
			return EINA_FALSE;
	}

	if (is_running) {
		BROWSER_LOGE("org.tizen.vtmain is running......\n");
		show_msg_popup(BR_STRING_WARNING_VIDEO_PLAYER);
		return EINA_FALSE;
	}

	service_h service_handle = NULL;

	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (!service_handle) {
		BROWSER_LOGE("service handle is NULL");
		return EINA_FALSE;
	}

	if (service_set_uri(service_handle, url) < 0) {
		BROWSER_LOGE("Fail to set uri");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (cookie && strlen(cookie)) {
		if (service_add_extra_data(service_handle, "cookie", cookie) < 0) {
			BROWSER_LOGE("Fail to set extra data as cookie");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
	}

	if (service_set_package(service_handle, SEC_STREAMING_PLAYER) < 0) {
		BROWSER_LOGE("Fail to create service_set_package as org.tizen.video-player");
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	service_destroy(service_handle);

	return EINA_TRUE;
}

void Browser_View::_set_secure_icon(void)
{
	std::string url = get_url();
	BROWSER_LOGD("url=[%s]", url.c_str());

	if (url.c_str() && url.length()) {
		if (!strncmp(url.c_str(), BROWSER_HTTPS_SCHEME, strlen(BROWSER_HTTPS_SCHEME))) {
			edje_object_signal_emit(elm_layout_edje_get(m_url_layout),
									"show,secure_icon,signal", "");
			edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_layout),
									"show,secure_icon,signal", "");
			return;
		}
	}

	edje_object_signal_emit(elm_layout_edje_get(m_url_layout), "hide,secure_icon,signal", "");
	edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_layout),
								"hide,secure_icon,signal", "");
}

void Browser_View::_load_finished(void)
{
	BROWSER_LOGD("[%s]", __func__);

	/* enable or disable back,forward controlbar buttons */
	_set_controlbar_back_forward_status();

	_set_secure_icon();
	if (!_set_favicon())
		BROWSER_LOGE("_set_favicon failed");

	/* Add current url to history */
	Eina_Bool is_full = EINA_FALSE;

	/* Save last visited url to save this when browser exits. */
	m_last_visited_url = get_url();

	if (m_data_manager->get_history_db()) {
		m_data_manager->get_history_db()->save_history(m_last_visited_url.c_str(),
							get_title().c_str(), &is_full);
		if (is_full)
			BROWSER_LOGE("history is full, delete the first one");
	}

	if (!m_personal_data_manager->set_personal_data(get_url().c_str()))
		BROWSER_LOGE("set_personal_data failed");

	_hide_scroller_url_layout();
}

void Browser_View::_hide_scroller_url_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (_get_edit_mode() != BR_NO_EDIT_MODE)
		return;

	_navigationbar_visible_set_signal(EINA_FALSE);

	int browser_scroller_x = 0;
	int browser_scroller_w = 0;
	int browser_scroller_h = 0;
	elm_scroller_region_get(m_scroller, &browser_scroller_x, NULL,
					&browser_scroller_w, &browser_scroller_h);

	int url_layout_h = 0;
	evas_object_geometry_get(m_url_layout, NULL, NULL, NULL, &url_layout_h);
	elm_scroller_region_show(m_scroller, browser_scroller_x, url_layout_h,
					browser_scroller_w, browser_scroller_h);
}

void Browser_View::__ewk_view_mouse_down_cb(void* data, Evas* evas, Evas_Object* obj, void* ev)
{
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Evas_Event_Mouse_Down event = *(Evas_Event_Mouse_Down *)ev;
	Evas_Object *main_layout = browser_view->m_main_layout;
	Evas_Object *ewk_view = browser_view->m_focused_window->m_ewk_view;

	if (!browser_view->_is_loading() && browser_view->_get_edit_mode() != BR_FIND_WORD_MODE)
		browser_view->_navigationbar_visible_set(EINA_FALSE);

	browser_view->m_is_scrolling = EINA_TRUE;

	Evas_Object *webkit = elm_webview_webkit_get(browser_view->m_focused_window->m_ewk_view);
	float max_zoom_rate = ewk_view_zoom_range_max_get(webkit);
	float min_zoom_rate = ewk_view_zoom_range_min_get(webkit);
	float current_zoom_rate = ewk_view_zoom_get(webkit);

	Eina_Bool can_zoom_in = EINA_TRUE;
	Eina_Bool can_zoom_out = EINA_TRUE;
	if (current_zoom_rate >= max_zoom_rate)
		can_zoom_in = EINA_FALSE;
	if (current_zoom_rate <= min_zoom_rate)
		can_zoom_out = EINA_FALSE;
	if (can_zoom_in || can_zoom_out) {
		if (browser_view->m_zoom_button_timer)
			ecore_timer_del(browser_view->m_zoom_button_timer);
		browser_view->m_zoom_button_timer = ecore_timer_add(3, __zoom_button_timeout_cb, browser_view);
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout), "show,zoom_buttons,signal", "");
	}

	if (!can_zoom_in)
		elm_object_disabled_set(browser_view->m_zoom_in_button ,EINA_TRUE);
	else
		elm_object_disabled_set(browser_view->m_zoom_in_button ,EINA_FALSE);

	if (!can_zoom_out)
		elm_object_disabled_set(browser_view->m_zoom_out_button ,EINA_TRUE);
	else
		elm_object_disabled_set(browser_view->m_zoom_out_button ,EINA_FALSE);
}

void Browser_View::__ewk_view_multi_down_cb(void *data, Evas *evas, Evas_Object *obj, void *ev)
{
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->m_is_multi_touch = EINA_TRUE;
	browser_view->_enable_webview_scroll();
}

void Browser_View::__ewk_view_mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *ev)
{
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	browser_view->m_is_multi_touch = EINA_FALSE;
	browser_view->m_is_scrolling = EINA_FALSE;
}

void Browser_View::__ewk_view_edge_top_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;
	Browser_View *browser_view = (Browser_View *)data;

	int browser_scroller_x = 0;
	int browser_scroller_y = 0;
	int browser_scroller_w = 0;
	int browser_scroller_h = 0;
	elm_scroller_region_get(browser_view->m_scroller, &browser_scroller_x, &browser_scroller_y,
					&browser_scroller_w, &browser_scroller_h);

	int url_layout_h = 0;
	evas_object_geometry_get(browser_view->m_url_layout, NULL, NULL, NULL, &url_layout_h);

	if (browser_scroller_y < url_layout_h)
		return;

	if (browser_view->m_is_scroll_up)
		return;

	BROWSER_LOGD("[%s]", __func__);
	browser_view->_enable_browser_scroller_scroll();

	/* If user do flicking the mouse with scroll up, bring in the browser scroller to y=0.	*/
	if (!browser_view->m_is_scrolling && !browser_view->m_is_scroll_up) {
		BROWSER_LOGD("<< elm_scroller_region_bring_in >>");
		elm_scroller_region_bring_in(browser_view->m_scroller, browser_scroller_x, 0,
						browser_scroller_w, browser_scroller_h);
	}
}

void Browser_View::__scroller_edge_bottom_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_enable_webview_scroll();
}

void Browser_View::__ewk_view_mouse_move_cb(void *data, Evas *evas, Evas_Object *obj, void *ev)
{
	if (!data)
		return;

	Evas_Event_Mouse_Move *point = (Evas_Event_Mouse_Move *)ev;

	Browser_View *browser_view = (Browser_View *)data;
	if (point->cur.output.y < point->prev.output.y) {
		browser_view->m_is_scroll_up = EINA_TRUE;
		__ewk_view_scroll_down_cb(data, NULL, NULL);
	} else {
		browser_view->m_is_scroll_up = EINA_FALSE;
		/* scroll up */
		__ewk_view_scroll_up_cb(data, NULL, NULL);
	}
}

void Browser_View::__ewk_view_scroll_down_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;
	Browser_View *browser_view = (Browser_View *)data;

	int browser_scroller_y = 0;
	int url_layout_h = 0;
	elm_scroller_region_get(browser_view->m_scroller, NULL, &browser_scroller_y, NULL, NULL);
	evas_object_geometry_get(browser_view->m_url_layout, NULL, NULL, NULL, &url_layout_h);

	if (browser_scroller_y < url_layout_h)
		browser_view->_enable_browser_scroller_scroll();
	else
		browser_view->_enable_webview_scroll();
}

void Browser_View::__ewk_view_scroll_up_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	int frame_position_y = 0;
	Evas_Object *webkit = elm_webview_webkit_get(browser_view->m_focused_window->m_ewk_view);
	Evas_Object *frame = ewk_view_frame_main_get(webkit);
	ewk_frame_scroll_pos_get(frame, NULL, &frame_position_y);

	int browser_scroller_y = 0;
	elm_scroller_region_get(browser_view->m_scroller, NULL, &browser_scroller_y, NULL, NULL);

	if (frame_position_y == 0 && browser_scroller_y)
		browser_view->_enable_browser_scroller_scroll();
	else
		browser_view->_enable_webview_scroll();
}

void Browser_View::_set_navigationbar_title(const char *title)
{
	BROWSER_LOGD("title=[%s]", title);
	edje_object_part_text_set(elm_layout_edje_get(m_url_layout),
					"title_text", title);
	edje_object_part_text_set(elm_layout_edje_get(m_option_header_url_layout),
					"title_text", title);
}


void Browser_View::_set_url_entry(const char *url, Eina_Bool set_secrue_icon)
{
	BROWSER_LOGD("url=[%s]", url);
	if (url && strlen(url)) {
		std::string url_without_http_scheme;
		if (strstr(url, BROWSER_HTTP_SCHEME) && strlen(url) > strlen(BROWSER_HTTP_SCHEME))
			url_without_http_scheme = std::string(url + strlen(BROWSER_HTTP_SCHEME));
		else
			url_without_http_scheme = std::string(url);

		char *mark_up_url = elm_entry_utf8_to_markup(url_without_http_scheme.c_str());
		if (mark_up_url) {
			Evas_Object *entry = br_elm_editfield_entry_get(m_url_edit_field);
			elm_entry_entry_set(entry, mark_up_url);
			entry = br_elm_editfield_entry_get(m_option_header_url_edit_field);
			elm_entry_entry_set(entry, mark_up_url);
			free(mark_up_url);
		}
	} else {
		Evas_Object *entry = br_elm_editfield_entry_get(m_url_edit_field);
		elm_entry_entry_set(entry, BROWSER_MOST_VISITED_SITES_URL);
		entry = br_elm_editfield_entry_get(m_option_header_url_edit_field);
		elm_entry_entry_set(entry, BROWSER_MOST_VISITED_SITES_URL);
	}

	if (set_secrue_icon)
		_set_secure_icon();
}

void Browser_View::_stop_loading(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_focused_window->m_ewk_view) {
		if (!ewk_view_stop(elm_webview_webkit_get(m_focused_window->m_ewk_view)))
			BROWSER_LOGE("ewk_view_stop failed.\n");
	}
}

void Browser_View::_reload(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_focused_window->m_ewk_view) {
		if (!ewk_view_reload_full(elm_webview_webkit_get(m_focused_window->m_ewk_view)))
			BROWSER_LOGE("ewk_view_reload_full failed.\n");
	}
}

void Browser_View::set_focused_window(Browser_Window *window, Eina_Bool show_most_visited_sites)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_focused_window && m_focused_window->m_ewk_view) {
		evas_object_hide(m_focused_window->m_ewk_view);
	}

	if (m_focused_window && m_focused_window->m_favicon) {
		if (elm_object_part_content_get(m_url_entry_layout, "elm.swallow.favicon"))
			elm_object_part_content_unset(m_url_entry_layout, "elm.swallow.favicon");

		evas_object_hide(m_focused_window->m_favicon);
	}
	if (m_focused_window && m_focused_window->m_option_header_favicon) {
		if (elm_object_part_content_get(m_option_header_url_entry_layout, "elm.swallow.favicon"))
			elm_object_part_content_unset(m_option_header_url_entry_layout, "elm.swallow.favicon");

		evas_object_hide(m_focused_window->m_option_header_favicon);
	}

	m_focused_window = window;

	elm_box_unpack_all(m_content_box);

	elm_box_pack_end(m_content_box, m_focused_window->m_ewk_view);
	elm_box_pack_start(m_content_box, m_url_layout);

	evas_object_show(m_focused_window->m_ewk_view);
	/* Workaround.
	  * The webview layout is not resized whenever repack to content box.
	  * So resize the webview layout whenever repack. */
	ecore_idler_add(__webview_layout_resize_idler_cb, this);

	if (!_is_loading()) {
		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "loading,off,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
					"loading,off,signal", "");
		elm_progressbar_pulse(m_url_progresswheel, EINA_FALSE);
		elm_progressbar_pulse(m_option_header_url_progresswheel, EINA_FALSE);
	}

	/* show or hide favicon in url layout. */
	if (m_focused_window->m_favicon) {
		if (elm_object_part_content_get(m_url_entry_layout, "elm.swallow.favicon"))
			elm_object_part_content_unset(m_url_entry_layout, "elm.swallow.favicon");

		elm_object_part_content_set(m_url_entry_layout, "elm.swallow.favicon",
							m_focused_window->m_favicon);
		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout),
							"show,favicon,signal", "");
		evas_object_show(m_focused_window->m_favicon);
	} else {
		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout),
							"hide,favicon,signal", "");
	}

	/* show or hide favicon in option header url layout. */
	if (m_focused_window->m_option_header_favicon) {
		if (elm_object_part_content_get(m_option_header_url_entry_layout, "elm.swallow.favicon"))
			elm_object_part_content_unset(m_option_header_url_entry_layout, "elm.swallow.favicon");

		elm_object_part_content_set(m_option_header_url_entry_layout, "elm.swallow.favicon",
							m_focused_window->m_option_header_favicon);
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
							"show,favicon,signal", "");
		evas_object_show(m_focused_window->m_option_header_favicon);
	} else {
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
							"hide,favicon,signal", "");
	}

	std::vector<Browser_Window *> window_list = m_browser->get_window_list();
	_set_multi_window_controlbar_text(window_list.size());

	if (!_set_favicon())
		BROWSER_LOGE("_set_favicon failed");

	/* Without this code, the url is empty shortly when create deleted-window in multi window. */
	if (get_url().empty() && !m_focused_window->m_url.empty())
		_set_url_entry(m_focused_window->m_url.c_str());
	else
		_set_url_entry(get_url().c_str());
}

void Browser_View::load_url(const char *url)
{
	BROWSER_LOGD("[%s]url=[%s]", __func__, url);
	if (!url || !strlen(url)) {
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
				"show,control_bar,no_animation,signal", "");

		if (m_dummy_loading_progressbar) {
			elm_object_part_content_unset(m_main_layout, "elm.swallow.waiting_progress");
			evas_object_del(m_dummy_loading_progressbar);
			m_dummy_loading_progressbar = NULL;
			edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
						"hide,waiting_progressbar,signal", "");
		}

		ecore_idler_add(_activate_url_entry_idler_cb, this);
		return;
	}

	Evas_Object *edit_field_entry = br_elm_editfield_entry_get(_get_activated_url_entry());
	evas_object_smart_callback_del(edit_field_entry, "changed", __url_entry_changed_cb);

	std::string full_url;
	if (_has_url_sheme(url))
		full_url = url;
	else
		full_url = std::string(BROWSER_HTTP_SCHEME) + std::string(url);

	BROWSER_LOGD("full_url = [%s]", full_url.c_str());

	_set_url_entry(full_url.c_str());

	elm_webview_uri_set(m_focused_window->m_ewk_view, full_url.c_str());
 }

string Browser_View::get_title(Browser_Window *window)
{
	BROWSER_LOGD("[%s]", __func__);
	string title = "";

	if (!window->m_ewk_view && !window->m_title.empty())
		return window->m_title;

	if (!window->m_ewk_view)
		return title;

	Evas_Object *webkit = elm_webview_webkit_get(window->m_ewk_view);
	Evas_Object *main_frame = ewk_view_frame_main_get(webkit);
	if (main_frame) {
		const char *frame_title = ewk_frame_title_get(main_frame);
		if (frame_title)
			title = std::string(frame_title);
	}

	return title;
}

string Browser_View::get_title(void)
{
	BROWSER_LOGD("[%s]", __func__);
	string title = "";

	Evas_Object *webkit = elm_webview_webkit_get(m_focused_window->m_ewk_view);
	Evas_Object *main_frame = ewk_view_frame_main_get(webkit);
	if (main_frame) {
		const char *frame_title = ewk_frame_title_get(main_frame);
		if (frame_title)
			title = std::string(frame_title);
	}

	BROWSER_LOGD("m_focused_window->m_title=[%s]", m_focused_window->m_title.c_str());
	if (title.empty() && !m_focused_window->m_title.empty())
		return m_focused_window->m_title;
	else
		return title;
}

string Browser_View::get_url(Browser_Window *window)
{
	BROWSER_LOGD("[%s]", __func__);
	string url = "";

	if (!window->m_ewk_view && !window->m_url.empty())
		return window->m_url;

	if (!window->m_ewk_view)
		return url;

	Evas_Object *webkit = elm_webview_webkit_get(window->m_ewk_view);
	Evas_Object *main_frame = ewk_view_frame_main_get(webkit);
	if (main_frame) {
		const char *frame_url = ewk_frame_uri_get(main_frame);
		if (frame_url)
			url = std::string(frame_url);
	}

	return url;
}

string Browser_View::get_url(void)
{
	BROWSER_LOGD("[%s]", __func__);
	string url = "";

	Evas_Object *webkit = elm_webview_webkit_get(m_focused_window->m_ewk_view);
	Evas_Object *main_frame = ewk_view_frame_main_get(webkit);
	if (main_frame) {
		const char *frame_url = ewk_frame_uri_get(main_frame);
		if (frame_url)
			url = std::string(frame_url);
	}
	return url;
}

/* If multi window is running, unset the navigation title object,
  * else set the navigation title object to show title object arrow.  */
void Browser_View::unset_navigationbar_title_object(Eina_Bool is_unset)
{
	BROWSER_LOGD("[%s] is_unset =%d", __func__, is_unset);
	Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);
	if (is_unset) {
		_navigationbar_visible_set_signal(EINA_FALSE);
		elm_object_item_part_content_unset(top_it, ELM_NAVIFRAME_ITEM_OPTIONHEADER);
		evas_object_hide(m_option_header_layout);
	} else {
		Evas_Object *title_object = NULL;
		title_object = elm_object_item_part_content_get(top_it, ELM_NAVIFRAME_ITEM_OPTIONHEADER);
		if (!title_object) {
			elm_object_item_part_content_set(top_it, ELM_NAVIFRAME_ITEM_OPTIONHEADER,
									m_option_header_layout);
			evas_object_show(m_option_header_layout);
		}

		_navigationbar_visible_set_signal(EINA_FALSE);
	}
}

void Browser_View::__go_to_bookmark_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->suspend_webview(browser_view->m_focused_window->m_ewk_view);
	if (!m_data_manager->create_bookmark_view()) {
		BROWSER_LOGE("m_data_manager->create_bookmark_view failed");
		return;
	}

	if (!m_data_manager->get_bookmark_view()->init()) {
		BROWSER_LOGE("m_data_manager->get_bookmark_view()->init failed");
		m_data_manager->destroy_bookmark_view();
	}

	browser_view->m_context_menu->destroy_context_popup();
	browser_view->_destroy_more_context_popup();
}

void Browser_View::__backward_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	__title_back_button_clicked_cb(data, obj, event_info);
	browser_view->_destroy_more_context_popup();
	browser_view->m_context_menu->destroy_context_popup();
}

void Browser_View::__forward_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_destroy_more_context_popup();
	if (browser_view->m_focused_window->m_ewk_view) {
		Evas_Object *webkit = elm_webview_webkit_get(browser_view->m_focused_window->m_ewk_view);
		if (ewk_view_forward_possible(webkit)) {
			if (!ewk_view_forward(webkit))
				BROWSER_LOGE("ewk_view_forward failed");
		}
	}
}

void Browser_View::__expand_option_header_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	browser_view->_navigationbar_title_clicked();
}

void Browser_View::__add_bookmark_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	if (!m_data_manager->create_add_to_bookmark_view(browser_view->get_title(), browser_view->get_url())) {
		BROWSER_LOGE("m_data_manager->create_add_to_bookmark_view failed");
		return;
	}

	if (!m_data_manager->get_add_to_bookmark_view()->init())
		m_data_manager->destroy_add_to_bookmark_view();
}

void Browser_View::__multi_window_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	Browser_View *browser_view = (Browser_View *)data;

	browser_view->m_context_menu->destroy_context_popup();
	browser_view->_destroy_more_context_popup();

	if (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW)) {
		BROWSER_LOGD("close multi window");
		m_data_manager->get_multi_window_view()->close_multi_window();
		return;
	}

	if (browser_view->_is_loading())
		browser_view->_stop_loading();

	if (!m_data_manager->create_multi_window_view()) {
		BROWSER_LOGE("m_data_manager->create_multi_window_view failed");
		return;
	}

	if (!m_data_manager->get_multi_window_view()->init()) {
		m_data_manager->destroy_multi_window_view();
		BROWSER_LOGE("get_multi_window_view()->init failed");
		return;
	}

	browser_view->suspend_webview(browser_view->m_focused_window->m_ewk_view);

	browser_view->_navigationbar_visible_set_signal(EINA_FALSE);

	/* Hide the secure lock icon in title bar. */
	Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);
	elm_object_item_part_content_set(top_it, ELM_NAVIFRAME_ITEM_ICON, NULL);

	if (browser_view->_get_edit_mode() != BR_NO_EDIT_MODE)
		browser_view->_set_edit_mode(BR_NO_EDIT_MODE);
}

Eina_Bool Browser_View::_call_internet_settings(void)
{
	BROWSER_LOGD("[%s]", __func__);

	m_browser_settings = new(nothrow) Browser_Settings_Class;
	if (!m_browser_settings) {
		BROWSER_LOGE("new Browser_Settings_Class failed");
		return EINA_FALSE;
	}
	if (!m_browser_settings->init()) {
		BROWSER_LOGE("m_browser_settings->init failed");
		delete m_browser_settings;
		m_browser_settings = NULL;

		return EINA_FALSE;
	}
	return EINA_TRUE;
}

void Browser_View::__internet_settings_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_destroy_more_context_popup();

	if (!browser_view->_call_internet_settings())
		BROWSER_LOGE("_call_internet_settings failed");
}

void Browser_View::__find_word_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_destroy_more_context_popup();

	if (browser_view->_get_edit_mode() == BR_FIND_WORD_MODE)
		return;

	/* find word layout is only in naviframe optino header.
	  * So expand the option header to show find word layout. */
	browser_view->_navigationbar_visible_set_signal(EINA_TRUE);

	browser_view->_set_edit_mode(BR_FIND_WORD_MODE);

	browser_view->m_find_word->init(browser_view->m_focused_window->m_ewk_view);

}

Eina_Bool Browser_View::_call_download_manager(void)
{
	service_h service_handle = NULL;
	BROWSER_LOGD("[%s]", __func__);

	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}
	
	if (!service_handle) {
		BROWSER_LOGE("service handle is NULL");
		return EINA_FALSE;
	}

	if (service_set_operation(service_handle, SERVICE_OPERATION_DOWNLOAD) < 0) {
		BROWSER_LOGE("Fail to set service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_add_extra_data(service_handle, "mode", "view") < 0) {
		BROWSER_LOGE("Fail to set extra data");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	service_destroy(service_handle);

	return EINA_TRUE;
}

void Browser_View::__download_manager_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_destroy_more_context_popup();

	if (!browser_view->_call_download_manager())
		BROWSER_LOGE("_call_download_manager failed");
}

void Browser_View::_destroy_more_context_popup(void)
{
	if (m_more_context_popup) {
		evas_object_del(m_more_context_popup);
		m_more_context_popup = NULL;
	}
}

void Browser_View::__more_context_popup_dismissed_cb(void *data, Evas_Object *obj,
									void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->_destroy_more_context_popup();
}

Eina_Bool Browser_View::_show_more_context_popup(void)
{
	BROWSER_LOGD("[%s]", __func__);

	_destroy_more_context_popup();

	m_more_context_popup = elm_ctxpopup_add(m_win);
	if (!m_more_context_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_more_context_popup, EVAS_HINT_EXPAND,
								EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(m_more_context_popup, "dismissed",
					__more_context_popup_dismissed_cb, this);

	Elm_Object_Item *sub_menu = elm_ctxpopup_item_append(m_more_context_popup,
						BR_STRING_FORWARD, NULL, __forward_cb, this);
	if (get_url().empty())
		elm_object_item_disabled_set(sub_menu, EINA_TRUE);
	else {
		Evas_Object *webkit = elm_webview_webkit_get(m_focused_window->m_ewk_view);
		if (!ewk_view_forward_possible(webkit))
			elm_object_item_disabled_set(sub_menu, EINA_TRUE);
	}

	elm_ctxpopup_item_append(m_more_context_popup, BR_STRING_DOWNLOAD_MANAGER, NULL,
							__download_manager_cb, this);

	sub_menu = elm_ctxpopup_item_append(m_more_context_popup, BR_STRING_FIND_WORD, NULL,
							__find_word_cb, this);
	if (get_url().empty()
	    || _get_edit_mode() == BR_FIND_WORD_MODE)
		elm_object_item_disabled_set(sub_menu, EINA_TRUE);

	elm_ctxpopup_item_append(m_more_context_popup, BR_STRING_SETTINGS, NULL,
							__internet_settings_cb, this);

	elm_ctxpopup_hover_parent_set(m_more_context_popup, m_navi_bar);

	Evas_Coord navibar_width = 0;
	Evas_Coord navibar_height = 0;
	evas_object_geometry_get(m_navi_bar, NULL, NULL, &navibar_width, &navibar_height);

	evas_object_move(m_more_context_popup, BROWSER_MORE_CTX_POPUP_MARGIN * 7,
					navibar_height - BROWSER_MORE_CTX_POPUP_MARGIN);
	evas_object_show(m_more_context_popup);

	return EINA_TRUE;
}

void Browser_View::__more_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	browser_view->m_context_menu->destroy_context_popup();

	if (browser_view->m_more_context_popup == NULL) {
		if (!browser_view->_show_more_context_popup())
			BROWSER_LOGE("_show_more_context_popup failed");
	} else {
		browser_view->_destroy_more_context_popup();
	}
}

void Browser_View::_set_controlbar_back_forward_status(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *ewk_view = m_focused_window->m_ewk_view;
	if (!ewk_view)
		return;

	if (get_url().empty())
		elm_object_item_disabled_set(m_share_controlbar_button, EINA_TRUE);
	else
		elm_object_item_disabled_set(m_share_controlbar_button, EINA_FALSE);
}

void Browser_View::_set_multi_window_controlbar_text(int count)
{
	BROWSER_LOGD("[%s]", __func__);
	if (count == 1) {
		elm_toolbar_item_icon_set(m_multi_window_button, BROWSER_IMAGE_DIR"/01_controlbar_icon_multiview.png");
	} else {
		char icon_path[100] = {0, };
		snprintf(icon_path, sizeof(icon_path) - 1, "%s/01_controlbar_icon_multiview_0%d.png", BROWSER_IMAGE_DIR, count);
		elm_toolbar_item_icon_set(m_multi_window_button, icon_path);
	}
}

Evas_Object *Browser_View::_create_control_bar(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *control_bar;
	control_bar = elm_toolbar_add(m_navi_bar);
	if (control_bar) {
		elm_object_style_set(control_bar, "browser/default");

		elm_toolbar_shrink_mode_set(control_bar, ELM_TOOLBAR_SHRINK_EXPAND);

		m_more_button = elm_toolbar_item_append(control_bar,
					BROWSER_IMAGE_DIR"/I01_controlbar_icon_more.png", NULL, __more_cb, this);
		if (!m_more_button) {
			BROWSER_LOGE("elm_toolbar_item_append failed");
			return NULL;
		}

		m_multi_window_button = elm_toolbar_item_append(control_bar,
					BROWSER_IMAGE_DIR"/01_controlbar_icon_multiview.png", NULL, __multi_window_cb, this);
		elm_toolbar_item_append(control_bar, BROWSER_IMAGE_DIR"/I01_controlbar_icon_bookmark.png",
						NULL, __go_to_bookmark_cb, this);


		m_backward_button = elm_toolbar_item_append(control_bar,
					BROWSER_IMAGE_DIR"/01_controlbar_icon_back.png", NULL, __backward_cb, this);
		if (!m_backward_button) {
			BROWSER_LOGE("elm_toolbar_item_append failed");
			return NULL;
		}

		evas_object_show(control_bar);
	}

	return control_bar;
}

/*
* Create two same url layouts similar with other browsers like android & safari.
* The one(by _create_url_layout) is in the browser scroller and
* the other(by _create_option_header_url_layout) is in the navigation bar option header.
*/
Evas_Object *Browser_View::_create_option_header_url_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *url_layout;
	url_layout = elm_layout_add(m_navi_bar);
	if (url_layout) {
		if (!elm_layout_file_set(url_layout, BROWSER_EDJE_DIR"/browser-view-url-layout.edj",
					"browser/url_layout")) {
			BROWSER_LOGE("Can not set layout theme[browser/url_layout]\n");
			return NULL;
		}
		evas_object_size_hint_weight_set(url_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(url_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(url_layout);

		m_option_header_url_entry_layout = elm_layout_add(m_navi_bar);
		if (!m_option_header_url_entry_layout) {
			BROWSER_LOGE("elm_layout_add failed");
			return NULL;
		}
		if (!elm_layout_file_set(m_option_header_url_entry_layout, BROWSER_EDJE_DIR"/browser-view-url-layout.edj",
					"elm/browser/urlentry/default")) {
			BROWSER_LOGE("Can not set layout theme[browser, urlentry, default]\n");
			return NULL;
		}
		elm_object_part_content_set(url_layout, "elm.swallow.url", m_option_header_url_entry_layout);
		edje_object_signal_callback_add(elm_layout_edje_get(m_option_header_url_entry_layout),
						"mouse,clicked,1", "elm.swallow.entry", __url_entry_clicked_cb, this);

		edje_object_signal_callback_add(elm_layout_edje_get(m_option_header_url_entry_layout),
						"refresh_stop", "*", __refresh_button_clicked_cb, this);

		evas_object_show(m_option_header_url_entry_layout);

		m_option_header_url_edit_field = br_elm_url_editfield_add(m_navi_bar);
		if (!m_option_header_url_edit_field) {
			BROWSER_LOGE("elm_editfield_add failed");
			return NULL;
		}

		elm_object_part_content_set(m_option_header_url_entry_layout, "elm.swallow.entry", m_option_header_url_edit_field);
		br_elm_editfield_entry_single_line_set(m_option_header_url_edit_field, EINA_TRUE);
		br_elm_editfield_eraser_set(m_option_header_url_edit_field, EINA_FALSE);

		Evas_Object *edit_field_entry = br_elm_editfield_entry_get(m_option_header_url_edit_field);
		elm_entry_input_panel_layout_set(edit_field_entry, ELM_INPUT_PANEL_LAYOUT_URL);
		ecore_imf_context_input_panel_event_callback_add((Ecore_IMF_Context *)elm_entry_imf_context_get(edit_field_entry),
								ECORE_IMF_INPUT_PANEL_STATE_EVENT, __url_entry_imf_event_cb, this);

		evas_object_smart_callback_add(edit_field_entry, "activated", __url_entry_enter_key_cb, this);
		evas_event_callback_add(evas_object_evas_get(m_option_header_url_edit_field), EVAS_CALLBACK_CANVAS_FOCUS_OUT,
					__url_entry_focus_out_cb, this);
		evas_object_show(m_option_header_url_edit_field);

		m_option_header_url_progressbar = elm_progressbar_add(m_navi_bar);
		if (!m_option_header_url_progressbar) {
			BROWSER_LOGE("elm_progressbar_add failed");
			return NULL;
		}
		elm_object_style_set(m_option_header_url_progressbar, "browser/loading");
		elm_object_part_content_set(m_option_header_url_entry_layout, "elm.swallow.progressbar", m_option_header_url_progressbar);
		elm_progressbar_value_set(m_option_header_url_progressbar, 0);
		evas_object_show(m_option_header_url_progressbar);

		m_option_header_url_progresswheel = elm_progressbar_add(m_navi_bar);
		if (!m_option_header_url_progresswheel) {
			BROWSER_LOGE("elm_progressbar_add failed");
			return NULL;
		}
		elm_object_style_set(m_option_header_url_progresswheel, "browser/loading_wheel");
		elm_progressbar_pulse(m_option_header_url_progresswheel, EINA_FALSE);
		elm_object_part_content_set(m_option_header_url_entry_layout, "elm.swallow.progress", m_option_header_url_progresswheel);
		evas_object_show(m_option_header_url_progresswheel);

		m_option_header_cancel_button = elm_button_add(m_navi_bar);
		if (!m_option_header_cancel_button) {
			BROWSER_LOGE("elm_button_add failed");
			return NULL;
		}
		evas_object_size_hint_weight_set(m_option_header_cancel_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(m_option_header_cancel_button, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_object_style_set(m_option_header_cancel_button, "text_only/style2");
		elm_object_text_set(m_option_header_cancel_button, BR_STRING_CANCEL);
		elm_object_part_content_set(url_layout, "elm.swallow.cancel", m_option_header_cancel_button);
		evas_object_smart_callback_add(m_option_header_cancel_button, "clicked", __cancel_button_clicked_cb, this);
		evas_object_show(m_option_header_cancel_button);

		/* for jump to top. */
		evas_object_event_callback_add(url_layout, EVAS_CALLBACK_MOUSE_DOWN, __option_header_url_layout_mouse_down_cb, this);
	}

	return url_layout;
}

Evas_Object *Browser_View::_get_activated_url_entry(void)
{
	/* The edit field in option header url layout is only valid for edit.
	  * If the edit field in browser scroller can have focus, there is so many focus issue.
	  * So just make the edit field in option header editable. */
	return m_option_header_url_edit_field;
}

void Browser_View::__url_entry_imf_event_cb(void *data, Ecore_IMF_Context *ctx, int value)
{
	BROWSER_LOGD("value=%d", value);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	if (value == ECORE_IMF_INPUT_PANEL_STATE_HIDE) {
		Evas_Object *edit_field_entry;
		edit_field_entry = br_elm_editfield_entry_get(browser_view->_get_activated_url_entry());
		elm_object_focus_set(edit_field_entry, EINA_FALSE);

		if (browser_view->m_edit_mode != BR_URL_ENTRY_EDIT_MODE_WITH_NO_IMF
		    && browser_view->m_edit_mode != BR_FIND_WORD_MODE)
			browser_view->_set_edit_mode(BR_NO_EDIT_MODE);

		browser_view->_set_url_entry(browser_view->get_url().c_str());
	} else if (value == ECORE_IMF_INPUT_PANEL_STATE_SHOW)
		/* If the focus of url entry is set automatically, the keypad is also displayed automatically. */
		/* eg. At url edit mode, lock the screen -> then unlock, the url entry will get focus. */
		/* The keypad is also invoked, so set the edit mode. */
		browser_view->_set_edit_mode(BR_URL_ENTRY_EDIT_MODE);
}

void Browser_View::__url_entry_focus_out_cb(void *data, Evas *e, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Evas_Object *edit_field_entry;
	edit_field_entry = br_elm_editfield_entry_get(browser_view->_get_activated_url_entry());

	Ecore_IMF_Context *ic = (Ecore_IMF_Context *)elm_entry_imf_context_get(edit_field_entry);
	if (!ic)
		return;

	ecore_imf_context_input_panel_hide(ic);

	if (browser_view->m_edit_mode != BR_FIND_WORD_MODE)
		browser_view->_set_edit_mode(BR_NO_EDIT_MODE);
}

void Browser_View::_set_edit_mode(edit_mode mode)
{
	BROWSER_LOGD("mode = %d", mode);

	if (m_edit_mode == mode)
		return;

	m_context_menu->destroy_context_popup();
	m_picker_handler->destroy_picker_layout();

	if (mode == BR_URL_ENTRY_EDIT_MODE || mode == BR_FIND_WORD_MODE
	    || mode == BR_URL_ENTRY_EDIT_MODE_WITH_NO_IMF) {
		/* If edit mode, lock the browser scroller */
		_enable_webview_scroll();

		/* Make the browser scroller region y = 0 to show url bar all at edit mode. */
		int scroller_x = 0;
		int scroller_w = 0;
		int scroller_h = 0;
		elm_scroller_region_get(m_scroller, &scroller_x, NULL, &scroller_w, &scroller_h);
		elm_scroller_region_show(m_scroller ,scroller_x, 0, scroller_w, scroller_h);
	}

	if (mode == BR_URL_ENTRY_EDIT_MODE) {
		/* change layout of url layout for edit mode. */
		edje_object_signal_emit(elm_layout_edje_get(m_url_layout), "edit,url,on,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_layout),
					"edit,url,on,signal", "");

		/* change refresh icon in url entry for edit mode. */
		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "edit,url,on,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
					"edit,url,on,signal", "");

		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "hide,favicon,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
										"hide,favicon,signal", "");

		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "rss,off,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
									"rss,off,signal", "");

		/* change the browser main view layout for edit mode.
		 *  Display content dim*/
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
								"edit,url,on,signal", "");
		if (m_predictive_history)
			delete m_predictive_history;
		m_predictive_history = new(nothrow) Browser_Predictive_History(m_navi_bar,
						m_data_manager->get_history_db(), this);
		if (!m_predictive_history) {
			BROWSER_LOGE("new Browser_Predictive_History failed");
			return;
		}
		Evas_Object *predictive_history_layout = m_predictive_history->create_predictive_history_layout();
		if (!predictive_history_layout) {
			BROWSER_LOGE("create_predictive_history_layout failed");
			delete m_predictive_history;
			m_predictive_history = NULL;
			return;
		}

		/* Becaue of predictive hisotry. */
		Evas_Object *edit_field_entry = br_elm_editfield_entry_get(_get_activated_url_entry());
		evas_object_smart_callback_del(edit_field_entry, "changed", __url_entry_changed_cb);
		evas_object_smart_callback_add(edit_field_entry, "changed", __url_entry_changed_cb, this);

		elm_object_part_content_set(m_main_layout, "elm.swallow.predictive_history", predictive_history_layout);
	} else if (mode == BR_NO_EDIT_MODE || mode == BR_URL_ENTRY_EDIT_MODE_WITH_NO_IMF) {
		if (m_edit_mode == BR_FIND_WORD_MODE) {
			edje_object_signal_emit(elm_layout_edje_get(m_option_header_layout), "hide,find_word_layout,signal", "");
			m_find_word->deinit();
		} else {
			/* change layout of url layout for normal mode. */
			edje_object_signal_emit(elm_layout_edje_get(m_url_layout), "edit,url,off,signal", "");
			edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_layout),
						"edit,url,off,signal", "");

			/* change refresh icon in url entry for normal mode. */
			edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "edit,url,off,signal", "");
			edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
						"edit,url,off,signal", "");

			if (m_focused_window->m_favicon)
				edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout),
										"show,favicon,signal", "");
			if (m_focused_window->m_option_header_favicon)
				edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
										"show,favicon,signal", "");

			/* change the browser main view layout for normal mode.
			 *  Hide content dim */
			edje_object_signal_emit(elm_layout_edje_get(m_main_layout),
						"edit,url,off,signal", "");

			if (_is_loading()) {
				edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "loading,on,signal", "");
				edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
							"loading,on,signal", "");
			}

			Evas_Object *edit_field_entry = br_elm_editfield_entry_get(_get_activated_url_entry());
			/* Becaue of predictive hisotry. */
			evas_object_smart_callback_del(edit_field_entry, "changed", __url_entry_changed_cb);
			edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "hide,predictive_history,signal", "");
			elm_object_part_content_unset(m_main_layout, "elm.swallow.predictive_history");
			if (m_predictive_history) {
				delete m_predictive_history;
				m_predictive_history = NULL;
			}
		}
	} else if (mode == BR_FIND_WORD_MODE) {
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_layout), "show,find_word_layout,signal", "");
	}

	m_edit_mode = mode;

	/* To show favicon, if click url entry while loading, then cancel case. */
	_set_favicon();
}

void Browser_View::__url_entry_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	if (browser_view->_is_loading())
		browser_view->_stop_loading();

	if (!browser_view->_navigationbar_visible_get()) {
		/* The edit field in option header is only for edit url. */
		browser_view->_navigationbar_visible_set_signal(EINA_TRUE);
	}

	edit_mode mode = browser_view->_get_edit_mode();

	browser_view->_set_edit_mode(BR_URL_ENTRY_EDIT_MODE);

	if (mode == BR_NO_EDIT_MODE) {
		elm_object_focus_set(browser_view->m_option_header_url_edit_field, EINA_TRUE);

		Evas_Object *entry = br_elm_editfield_entry_get(browser_view->m_option_header_url_edit_field);
		elm_entry_cursor_end_set(entry);
	}
}

Eina_Bool Browser_View::_is_option_header_expanded(void)
{
	return _navigationbar_visible_get();
}

void Browser_View::__cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Evas_Object *ewk_view = browser_view->m_focused_window->m_ewk_view;
	if (!ewk_view)
		return;

	browser_view->_set_edit_mode(BR_NO_EDIT_MODE);

	browser_view->_set_url_entry(browser_view->get_url().c_str());
}

void Browser_View::__refresh_button_clicked_cb(void *data, Evas_Object *obj,
						const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	if (browser_view->_get_edit_mode() == BR_URL_ENTRY_EDIT_MODE) {
		elm_entry_entry_set(br_elm_editfield_entry_get(browser_view->m_url_edit_field), "");
		elm_entry_entry_set(br_elm_editfield_entry_get(browser_view->m_option_header_url_edit_field), "");
	} else if(browser_view->_is_loading())
		browser_view->_stop_loading();
	else
		browser_view->_reload();
}

void Browser_View::__url_entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Evas_Object *ewk_view = browser_view->m_focused_window->m_ewk_view;
	Evas_Object *edit_field_entry;
	edit_field_entry = br_elm_editfield_entry_get(browser_view->_get_activated_url_entry());

	/* Workaround.
	  * Give focus to option header cancel button to hide imf. */
	elm_object_focus_set(browser_view->m_option_header_cancel_button, EINA_TRUE);

	char *url = elm_entry_markup_to_utf8(elm_entry_entry_get(edit_field_entry));
	BROWSER_LOGD("input url = [%s]", url);

	if (url && strlen(url)) {
		browser_view->load_url(url);
		free(url);
	}
}

void Browser_View::__url_entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Browser_View *browser_view = (Browser_View *)data;

	Evas_Object *entry = br_elm_editfield_entry_get(browser_view->_get_activated_url_entry());
	const char *input_text = elm_entry_entry_get(entry);
	BROWSER_LOGD("[%s]", input_text);
	if (browser_view->m_predictive_history)
		browser_view->m_predictive_history->url_changed(input_text);
}

void Browser_View::__url_layout_mouse_down_cb(void *data, Evas* evas, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	edit_mode mode = browser_view->_get_edit_mode();
	if (mode == BR_URL_ENTRY_EDIT_MODE || mode == BR_FIND_WORD_MODE
		    || mode == BR_URL_ENTRY_EDIT_MODE_WITH_NO_IMF) {
		BROWSER_LOGD("<< lock browser scroller >>");
		elm_object_scroll_freeze_pop(browser_view->m_scroller);
		elm_object_scroll_freeze_push(browser_view->m_scroller);
	}
}

void Browser_View::__option_header_url_layout_mouse_down_cb(void *data, Evas* evas, Evas_Object *obj,
										void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Evas_Event_Mouse_Down event = *(Evas_Event_Mouse_Down *)event_info;
	Browser_View *browser_view = (Browser_View *)data;

	int jump_y = 0;
	evas_object_geometry_get(browser_view->m_option_header_cancel_button, NULL, &jump_y,
											NULL, NULL);
	if (event.output.y < jump_y)
		browser_view->_jump_to_top();
}

Evas_Object *Browser_View::_create_url_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Evas_Object *url_layout;
	url_layout = elm_layout_add(m_navi_bar);
	if (url_layout) {
		if (!elm_layout_file_set(url_layout, BROWSER_EDJE_DIR"/browser-view-url-layout.edj",
					"browser/url_layout")) {
			BROWSER_LOGE("Can not set layout theme[browser/url_layout]\n");
			return NULL;
		}
		evas_object_size_hint_weight_set(url_layout, EVAS_HINT_EXPAND, 0.0);
		evas_object_size_hint_align_set(url_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(url_layout);

		/* create url entry layout in url layout */
		m_url_entry_layout = elm_layout_add(m_navi_bar);
		if (!m_url_entry_layout) {
			BROWSER_LOGE("elm_layout_add failed");
			return NULL;
		}
		if (!elm_layout_file_set(m_url_entry_layout, BROWSER_EDJE_DIR"/browser-view-url-layout.edj",
					"elm/browser/urlentry/default")) {
			BROWSER_LOGE("Can not set layout theme[browser, urlentry, default]\n");
			return NULL;
		}
		elm_object_part_content_set(url_layout, "elm.swallow.url", m_url_entry_layout);
		evas_object_show(m_url_entry_layout);

		m_url_edit_field = br_elm_url_editfield_add(m_navi_bar);
		if (!m_url_edit_field) {
			BROWSER_LOGE("elm_editfield_add failed");
			return NULL;
		}
		elm_object_part_content_set(m_url_entry_layout, "elm.swallow.entry", m_url_edit_field);
		br_elm_editfield_entry_single_line_set(m_url_edit_field, EINA_TRUE);
		br_elm_editfield_eraser_set(m_url_edit_field, EINA_FALSE);

		Evas_Object *edit_field_entry = br_elm_editfield_entry_get(m_url_edit_field);
		elm_entry_editable_set(edit_field_entry, EINA_FALSE);
		/* The edit field in browser scroller is only for display.
		  * The edit url is only supported in option header url entry. */
		elm_object_focus_allow_set(m_url_edit_field, EINA_FALSE);
		elm_object_focus_allow_set(edit_field_entry, EINA_FALSE);
		elm_entry_input_panel_enabled_set(edit_field_entry, EINA_FALSE);
		evas_object_show(m_url_edit_field);

		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "disable_entry,signal", "");
		edje_object_signal_callback_add(elm_layout_edje_get(m_url_entry_layout), "mouse,clicked,1", "block_entry",
						__url_entry_clicked_cb, this);

		edje_object_signal_callback_add(elm_layout_edje_get(m_url_entry_layout), "refresh_stop", "*",
						__refresh_button_clicked_cb, this);

		m_url_progressbar = elm_progressbar_add(m_navi_bar);
		if (!m_url_progressbar) {
			BROWSER_LOGE("elm_progressbar_add failed");
			return NULL;
		}
		elm_object_style_set(m_url_progressbar, "browser/loading");
		elm_object_part_content_set(m_url_entry_layout, "elm.swallow.progressbar", m_url_progressbar);
		elm_progressbar_value_set(m_url_progressbar, 0);
		evas_object_show(m_url_progressbar);

		m_url_progresswheel = elm_progressbar_add(m_navi_bar);
		if (!m_url_progresswheel) {
			BROWSER_LOGE("elm_progressbar_add failed");
			return NULL;
		}
		elm_object_style_set(m_url_progresswheel, "browser/loading_wheel");
		elm_progressbar_pulse(m_url_progresswheel, EINA_FALSE);
		elm_object_part_content_set(m_url_entry_layout, "elm.swallow.progress", m_url_progresswheel);
		evas_object_show(m_url_progresswheel);

		m_cancel_button = elm_button_add(m_navi_bar);
		if (!m_cancel_button) {
			BROWSER_LOGE("elm_button_add failed");
			return NULL;
		}
		evas_object_size_hint_weight_set(m_cancel_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(m_cancel_button, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_object_style_set(m_cancel_button, "text_only/style2");
		elm_object_text_set(m_cancel_button, BR_STRING_CANCEL);
		elm_object_part_content_set(url_layout, "elm.swallow.cancel", m_cancel_button);
		evas_object_smart_callback_add(m_cancel_button, "clicked", __cancel_button_clicked_cb, this);
		evas_object_show(m_cancel_button);

		/* Workaround.
		 * When edit mode, if scroll down on url layout in browser view,
		 * the browser can be scrolled even though scroll locked.
		 * So, lock the browser scroller whenever touch on it if edit mode.
		 */
		evas_object_event_callback_add(url_layout, EVAS_CALLBACK_MOUSE_DOWN, __url_layout_mouse_down_cb, this);
	}

	return url_layout;
}

Eina_Bool Browser_View::_search_keyword_from_search_engine(const char *keyword)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!keyword || !strlen(keyword)) {
		BROWSER_LOGE("keyword is null");
		return EINA_FALSE;
	}

	Eina_Bool only_has_space = EINA_FALSE;
	int space_count = 0;
	for (int i = 0 ; i < strlen(keyword) ; i++) {
		if (keyword[i] == ' ')
			space_count++;
	}
	if (space_count == strlen(keyword))
		only_has_space = EINA_TRUE;

	if (only_has_space) {
		BROWSER_LOGE("keyword has only spaces");
		return EINA_FALSE;
	}

	char *search_engine = vconf_get_str(BROWSER_SEARCH_ENGINE_KEY);
	if (!search_engine) {
		search_engine = strdup(BROWSER_GOOGLE);
		if (!search_engine) {
			BROWSER_LOGE("strdup failed");
			return EINA_FALSE;
		}
	}

	std::string search_url_prefix;
	if (!strncmp(search_engine, BROWSER_GOOGLE, strlen(BROWSER_GOOGLE)))
		search_url_prefix = std::string(BROWSER_SEARCH_URL_GOOGLE);
	else if (!strncmp(search_engine, BROWSER_YAHOO, strlen(BROWSER_YAHOO)))
		search_url_prefix = std::string(BROWSER_SEARCH_URL_YAHOO);

	if (search_url_prefix.empty()) {
		BROWSER_LOGE("search_url_prefix is empty");
		return EINA_FALSE;
	}

	std::string search_url = search_url_prefix + std::string(keyword);
	load_url(search_url.c_str());

	return EINA_TRUE;
}

void Browser_View::_enable_browser_scroller_scroll(void)
{
	if (!elm_webview_vertical_panning_hold_get(m_focused_window->m_ewk_view)
	     && !m_is_multi_touch && !_is_loading()
	     && !elm_webview_fixed_position_get(m_focused_window->m_ewk_view)) {
		BROWSER_LOGD("<< unlock browser scroller, lock ewk view >>");
		elm_object_scroll_freeze_pop(m_scroller);
		elm_webview_vertical_panning_hold_set(m_focused_window->m_ewk_view, EINA_TRUE);
	}
}

void Browser_View::_enable_webview_scroll(void)
{
//	if (elm_webview_vertical_panning_hold_get(m_focused_window->m_ewk_view)) 
	{
//		BROWSER_LOGD("<< lock browser scroller, unlock ewk view >>");
		elm_object_scroll_freeze_pop(m_scroller);
		elm_object_scroll_freeze_push(m_scroller);
		elm_webview_vertical_panning_hold_set(m_focused_window->m_ewk_view, EINA_FALSE);
	}
}

void Browser_View::_navigationbar_title_clicked(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);
	Evas_Object *content = elm_object_item_content_get(top_it);
	if (content != m_main_layout)
		return;

	if (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW)
	    || m_edit_mode != BR_NO_EDIT_MODE)
	    return;

	int scroller_x = 0;
	int scroller_y = 0;
	int scroller_w = 0;
	int scroller_h = 0;
	elm_scroller_region_get(m_scroller, &scroller_x, &scroller_y, &scroller_w, &scroller_h);

	if (scroller_y == 0) {
		/* scroller is on top, the url bar in scroller is fully being displayed.
		 * Then, just show/hide the tool bar. */
		const char* state = edje_object_part_state_get(elm_layout_edje_get(m_main_layout),
								"elm.swallow.control_bar", NULL);
		if(state && !strncmp(state, "default", strlen("default")))
			edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "show,control_bar,signal", "");
		else {
			int url_layout_h = 0;
			evas_object_geometry_get(m_url_layout, NULL, NULL, NULL, &url_layout_h);
			elm_scroller_region_bring_in(m_scroller ,scroller_x, url_layout_h, scroller_w, scroller_h);
		}
	} else {
		if (_is_loading()) {
		} else {
			Eina_Bool visible = _navigationbar_visible_get();
			_navigationbar_visible_set(!visible);
		}
	}
}

void Browser_View::_jump_to_top(void)
{
	BROWSER_LOGD("[%s]", __func__);
	Elm_Object_Item *top_it = elm_naviframe_top_item_get(m_navi_bar);
	Evas_Object *content = elm_object_item_content_get(top_it);
	if (content != m_main_layout)
		return;

	if (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW)
	    || m_edit_mode != BR_NO_EDIT_MODE)
		return;

	int browser_scroller_x = 0;
	int browser_scroller_w = 0;
	int browser_scroller_h = 0;
	elm_scroller_region_get(m_scroller, &browser_scroller_x, NULL,
				&browser_scroller_w, &browser_scroller_h);
	elm_scroller_region_show(m_scroller, browser_scroller_x, 0, browser_scroller_w, browser_scroller_h);

	Evas_Object *webkit = elm_webview_webkit_get(m_focused_window->m_ewk_view);
	int frame_x = 0;
	int frame_y = 0;
	if (!ewk_frame_scroll_pos_get(ewk_view_frame_main_get(webkit), &frame_x, &frame_y))
		BROWSER_LOGE("_scroll_pos_get is failed.\n");
	if (!ewk_frame_scroll_set(ewk_view_frame_main_get(webkit), frame_x, 0))
		BROWSER_LOGE("ewk_frame_scroll_set is failed.\n");
}

void Browser_View::delete_non_user_created_windows(void)
{
	BROWSER_LOGD("[%s]", __func__);
	/* Delete no-backward history window. */
	std::vector<Browser_Window *> window_list = m_browser->get_window_list();
	int window_count = window_list.size();

	if (window_count <= 1)
		return;

	Browser_Window *focusable_window = NULL;
	int i = 0;
	for (i = 0 ; i < window_count ; i++) {
		if (window_list[i]->m_ewk_view && window_list[i]->m_created_by_user == EINA_TRUE) {
			focusable_window = window_list[i];
			break;
		}
	}

	if (!focusable_window)
		focusable_window = window_list[0];

	for (i = 0 ; i < window_count ; i++) {
		BROWSER_LOGD("focusable_window = %d, window[%d]=%d", focusable_window, i, window_list[i]);
		if (window_list[i]->m_ewk_view
		     &&!ewk_view_back_possible(elm_webview_webkit_get(window_list[i]->m_ewk_view))
		     && window_list[i] != focusable_window
		     && window_list[i]->m_created_by_user == EINA_FALSE) {
			BROWSER_LOGD("delete window index=[%d]", i);
			m_browser->set_focused_window(focusable_window);
			m_browser->delete_window(window_list[i]);

			/* Set title & url with the focused window. */
			_set_navigationbar_title(get_title().c_str());
			_set_url_entry(get_url().c_str());
			_set_controlbar_back_forward_status();
			if (!_set_favicon())
				BROWSER_LOGE("_set_favicon failed");
		}
	}
}

void Browser_View::__title_back_button_clicked_cb(void *data , Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	if (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW)) {
		BROWSER_LOGD("close multi window");
		m_data_manager->get_multi_window_view()->close_multi_window();
		return;
	}

	Browser_View *browser_view = (Browser_View *)data;

	if (browser_view->m_focused_window->m_ewk_view
	    && ewk_view_back_possible(elm_webview_webkit_get(browser_view->m_focused_window->m_ewk_view))) {
		if (!ewk_view_back(elm_webview_webkit_get(browser_view->m_focused_window->m_ewk_view)))
			BROWSER_LOGE("ewk_view_back failed");
	} else
	{
		if (browser_view->m_focused_window->m_parent) {
			/* Save current window pointer to delete later. */
			Browser_Window *delete_window = browser_view->m_focused_window;
			browser_view->m_browser->set_focused_window(browser_view->m_focused_window->m_parent);
			browser_view->m_browser->delete_window(delete_window);

			/* Set title & url with the focused window. */
			browser_view->_set_navigationbar_title(browser_view->get_title().c_str());
			browser_view->_set_url_entry(browser_view->get_url().c_str());
			browser_view->_set_controlbar_back_forward_status();
			if (!browser_view->_set_favicon())
				BROWSER_LOGE("_set_favicon failed");
		} else {
			elm_win_lower(browser_view->m_win);

			browser_view->delete_non_user_created_windows();
		}
	}
}

void Browser_View::__scroller_scroll_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	int browser_scroller_x = 0;
	int browser_scroller_y = 0;
	int browser_scroller_w = 0;
	int browser_scroller_h = 0;
	elm_scroller_region_get(browser_view->m_scroller, &browser_scroller_x, &browser_scroller_y,
				&browser_scroller_w, &browser_scroller_h);

	int url_layout_h = 0;
	evas_object_geometry_get(browser_view->m_url_layout, NULL, NULL, NULL, &url_layout_h);

	/* Workaround.
	 * If user scrolls up/down near the url bar edge, the movement of scroller is not smooth.
	 * It's because the browser scroller region y is also bouncing.
	 * So if the scroller region y is bigger than the height of url bar(65 pixel),
	 * make the region y to url bar height by force. */
	if (browser_scroller_y > url_layout_h) {
		BROWSER_LOGE("== elm_scroller_region_show / hide url bar ==");
		elm_scroller_region_show(browser_view->m_scroller, browser_scroller_x, url_layout_h,
					browser_scroller_w, browser_scroller_h);
	}
}

Eina_Bool Browser_View::_is_loading(void)
{
	if (!m_focused_window || !m_focused_window->m_ewk_view)
		return EINA_FALSE;

	Evas_Object *webkit = elm_webview_webkit_get(m_focused_window->m_ewk_view);
	if (!webkit) {
		BROWSER_LOGE("elm_webview_webkit_get is failed\n");
		return EINA_FALSE;
	}
	double progress = ewk_view_load_progress_get(webkit);

	if (progress == 1.0f || progress < 0.05f)
		return EINA_FALSE;
	else
		return EINA_TRUE;
}

void Browser_View::suspend_webview(Evas_Object *webview)
{
	BROWSER_LOGD("[%s]", __func__);

	Evas_Object *webkit = elm_webview_webkit_get(webview);
	ewk_view_visibility_state_set(webkit, EWK_PAGE_VISIBILITY_STATE_VISIBLE, EINA_FALSE);
	ewk_view_pause_or_resume_plugins(webkit, EINA_TRUE);
	ewk_view_pause_or_resume_video_audio(webkit, EINA_TRUE);
	ewk_view_javascript_suspend(webkit);
	ewk_view_disable_render(webkit);
	ewk_view_suspend_request(webkit);
}

void Browser_View::resume_webview(Evas_Object *webview)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_data_manager->is_in_view_stack(BR_MULTI_WINDOW_VIEW))
		return;

	Evas_Object *webkit = elm_webview_webkit_get(webview);
	ewk_view_visibility_state_set(webkit, EWK_PAGE_VISIBILITY_STATE_VISIBLE, EINA_TRUE);
	ewk_view_pause_or_resume_plugins(webkit, EINA_FALSE);
	ewk_view_pause_or_resume_video_audio(webkit, EINA_FALSE);
	ewk_view_javascript_resume(webkit);
	ewk_view_enable_render(webkit);
	ewk_view_resume_request(webkit);
}

void Browser_View::pause(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_is_loading()) {
		/* Because the progress wheel in url bar has about 15% cpu consumption.
		  * So, pause the animation when browser goes to background. It's nonsense!*/
		if (m_option_header_url_progresswheel)
			elm_progressbar_pulse(m_option_header_url_progresswheel, EINA_FALSE);
		if (m_url_progresswheel)
			elm_progressbar_pulse(m_url_progresswheel, EINA_FALSE);
	}

	ug_pause();

	suspend_webview(m_focused_window->m_ewk_view);
}

void Browser_View::resume(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_is_loading()) {
		/* Because the progress wheel in url bar has about 15% cpu consumption.
		  * So, pause the animation when browser goes to background. It's nonsense! */
		if (m_option_header_url_progresswheel)
			elm_progressbar_pulse(m_option_header_url_progresswheel, EINA_TRUE);
		if (m_url_progresswheel)
			elm_progressbar_pulse(m_url_progresswheel, EINA_TRUE);
	}

	ug_resume();

	resume_webview(m_focused_window->m_ewk_view);
}

void Browser_View::reset(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_focused_window && m_focused_window->m_ewk_view)
		resume();
}

Eina_Bool Browser_View::__webview_layout_resize_idler_cb(void *data)
{
	BROWSER_LOGD("\n");
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	Browser_View *browser_view = (Browser_View *)data;

	int content_w = 0;
	int content_h = 0;
	edje_object_part_geometry_get(elm_layout_edje_get(browser_view->m_main_layout),
				"elm.swallow.content", NULL, NULL, &content_w, &content_h);
	evas_object_size_hint_min_set(browser_view->m_focused_window->m_ewk_view,
				content_w, content_h);
	evas_object_resize(browser_view->m_focused_window->m_ewk_view,
				content_w, content_h);

	/*
	* For the first time, the background color is white initially.
	* If the background is not displayed yet, show the grey background.
	* This code is executed only one time at launching time.
	*/
	const char* state = edje_object_part_state_get(elm_layout_edje_get(browser_view->m_main_layout),
							"contents_bg", NULL);
	if(state && !strncmp(state, "default", strlen("default")))
		edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout),
						"show,grey_background,signal", "");

	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool Browser_View::__scroller_bring_in_idler_cb(void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return ECORE_CALLBACK_CANCEL;
	Browser_View *browser_view = (Browser_View *)data;

	int url_layout_h = 0;
	evas_object_geometry_get(browser_view->m_url_layout, NULL, NULL, NULL, &url_layout_h);

	BROWSER_LOGD("<< elm_scroller_region_bring_in , url_layout_h=%d >>", url_layout_h);
	int browser_scroller_x = 0;
	int browser_scroller_w = 0;
	int browser_scroller_h = 0;
	elm_scroller_region_get(browser_view->m_scroller, &browser_scroller_x, NULL,
					&browser_scroller_w, &browser_scroller_h);
	elm_scroller_region_show(browser_view->m_scroller, browser_scroller_x, url_layout_h,
					browser_scroller_w, browser_scroller_h);

	return ECORE_CALLBACK_CANCEL;
}

/* Workaround.
 * If user invokes the keypad via input field or url entry, resize the webview.
 * The only scroller resize is called when the keypad is launched.
 * Other elements like layout, conformant resize event doesn't come. */
void Browser_View::__scoller_resize_cb(void* data, Evas* evas, Evas_Object* obj, void* ev)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	int scroller_w = 0;
	int scroller_h = 0;
	evas_object_geometry_get(browser_view->m_scroller, NULL, NULL, &scroller_w, &scroller_h);
	if (browser_view->m_focused_window && browser_view->m_focused_window->m_ewk_view) {
		evas_object_size_hint_min_set(browser_view->m_focused_window->m_ewk_view,
									scroller_w, scroller_h);
		evas_object_resize(browser_view->m_focused_window->m_ewk_view,
								scroller_w, scroller_h);
	}

	Evas_Object *edit_field_entry;
	edit_field_entry = br_elm_editfield_entry_get(browser_view->_get_activated_url_entry());
	Ecore_IMF_Context *ic = (Ecore_IMF_Context *)elm_entry_imf_context_get(edit_field_entry);
	/* If the keypad from webkit is invoked in landscape mode, the visible viewport is too narrow.
	  * So, hide the url bar in browser scroller by bring in.
	  * The direct bring in call doesn't work, so do it by idler. */
	if (ic && browser_view->is_landscape()) {
		Ecore_IMF_Input_Panel_State imf_state = ecore_imf_context_input_panel_state_get(ic);
		/* Strangley, The state of webkit's imf is show-state even if the ic is url entry.
		  * However, this is good for browser without webkit's ic get API. */
		if (imf_state == ECORE_IMF_INPUT_PANEL_STATE_SHOW) {
			browser_view->_enable_browser_scroller_scroll();
			ecore_idler_add(__scroller_bring_in_idler_cb, browser_view);
		}
	}
}

Eina_Bool Browser_View::__zoom_button_timeout_cb(void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return ECORE_CALLBACK_CANCEL;

	Browser_View *browser_view = (Browser_View *)data;
	browser_view->m_zoom_button_timer = NULL;

	edje_object_signal_emit(elm_layout_edje_get(browser_view->m_main_layout), "hide,zoom_buttons,signal", "");

	return ECORE_CALLBACK_CANCEL;
}

void Browser_View::__zoom_out_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	Evas_Object *webkit = elm_webview_webkit_get(browser_view->m_focused_window->m_ewk_view);
	float current_zoom_rate = ewk_view_zoom_get(webkit);
	ewk_view_zoom_set(webkit, current_zoom_rate - 0.5f, 0, 0);

	if (browser_view->m_zoom_button_timer)
		ecore_timer_del(browser_view->m_zoom_button_timer);
	browser_view->m_zoom_button_timer = ecore_timer_add(3, __zoom_button_timeout_cb, browser_view);

	current_zoom_rate = ewk_view_zoom_get(webkit);
	float max_zoom_rate = ewk_view_zoom_range_max_get(webkit);
	float min_zoom_rate = ewk_view_zoom_range_min_get(webkit);

	Eina_Bool can_zoom_in = EINA_TRUE;
	Eina_Bool can_zoom_out = EINA_TRUE;
	if (current_zoom_rate >= max_zoom_rate)
		can_zoom_in = EINA_FALSE;
	if (current_zoom_rate <= min_zoom_rate)
		can_zoom_out = EINA_FALSE;

	if (!can_zoom_in)
		elm_object_disabled_set(browser_view->m_zoom_in_button ,EINA_TRUE);
	else
		elm_object_disabled_set(browser_view->m_zoom_in_button ,EINA_FALSE);

	if (!can_zoom_out)
		elm_object_disabled_set(browser_view->m_zoom_out_button ,EINA_TRUE);
	else
		elm_object_disabled_set(browser_view->m_zoom_out_button ,EINA_FALSE);
}

void Browser_View::__zoom_in_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	Evas_Object *webkit = elm_webview_webkit_get(browser_view->m_focused_window->m_ewk_view);
	float current_zoom_rate = ewk_view_zoom_get(webkit);
	ewk_view_zoom_set(webkit, current_zoom_rate + 0.5f, 0, 0);

	if (browser_view->m_zoom_button_timer)
		ecore_timer_del(browser_view->m_zoom_button_timer);
	browser_view->m_zoom_button_timer = ecore_timer_add(3, __zoom_button_timeout_cb, browser_view);

	current_zoom_rate = ewk_view_zoom_get(webkit);
	float max_zoom_rate = ewk_view_zoom_range_max_get(webkit);
	float min_zoom_rate = ewk_view_zoom_range_min_get(webkit);

	Eina_Bool can_zoom_in = EINA_TRUE;
	Eina_Bool can_zoom_out = EINA_TRUE;
	if (current_zoom_rate >= max_zoom_rate)
		can_zoom_in = EINA_FALSE;
	if (current_zoom_rate <= min_zoom_rate)
		can_zoom_out = EINA_FALSE;

	if (!can_zoom_in)
		elm_object_disabled_set(browser_view->m_zoom_in_button ,EINA_TRUE);
	else
		elm_object_disabled_set(browser_view->m_zoom_in_button ,EINA_FALSE);

	if (!can_zoom_out)
		elm_object_disabled_set(browser_view->m_zoom_out_button ,EINA_TRUE);
	else
		elm_object_disabled_set(browser_view->m_zoom_out_button ,EINA_FALSE);
}

Eina_Bool Browser_View::_create_zoom_buttons(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_zoom_out_button = elm_button_add(m_navi_bar);
	if (!m_zoom_out_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_zoom_out_button, "browser/zoom_out");
	elm_object_part_content_set(m_main_layout, "elm.swallow.zoom_out_button", m_zoom_out_button);
	evas_object_smart_callback_add(m_zoom_out_button, "clicked", __zoom_out_clicked_cb, this);
	evas_object_show(m_zoom_out_button);

	m_zoom_in_button = elm_button_add(m_navi_bar);
	if (!m_zoom_in_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}

	elm_object_style_set(m_zoom_in_button, "browser/zoom_in");
	elm_object_part_content_set(m_main_layout, "elm.swallow.zoom_in_button", m_zoom_in_button);
	evas_object_smart_callback_add(m_zoom_in_button, "clicked", __zoom_in_clicked_cb, this);
	evas_object_show(m_zoom_in_button);

	return EINA_TRUE;
}
void Browser_View::__naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	if (browser_view->m_navi_it != elm_naviframe_top_item_get(m_navi_bar)) {
		browser_view->suspend_webview(browser_view->m_focused_window->m_ewk_view);
		return;
	}

	m_data_manager->destroy_bookmark_view();
	m_data_manager->destroy_history_layout();
	/* Add to bookmark, then cancel. */
	m_data_manager->destroy_add_to_bookmark_view();

	/* If return from browser settings. */
	if (browser_view->m_browser_settings) {
		delete browser_view->m_browser_settings;
		browser_view->m_browser_settings = NULL;
	}

	browser_view->resume_webview(browser_view->m_focused_window->m_ewk_view);
}

void Browser_View::__dim_area_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_View *browser_view = (Browser_View *)data;

	elm_object_focus_set(m_data_manager->get_browser_view()->m_option_header_cancel_button, EINA_TRUE);
}

Eina_Bool Browser_View::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);
	m_main_layout = elm_layout_add(m_navi_bar);
	if (!m_main_layout) {
		BROWSER_LOGE("elm_layout_add failed!");
		return EINA_FALSE;
	}

	if (!elm_layout_file_set(m_main_layout, BROWSER_EDJE_DIR"/browser-view-main.edj",
				"browser/browser-view-main")) {
		BROWSER_LOGE("elm_layout_file_set failed", BROWSER_EDJE_DIR);
		return EINA_FALSE;
	}

	evas_object_size_hint_weight_set(m_main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_main_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_main_layout);

	m_title_back_button = elm_button_add(m_navi_bar);
	if (!m_title_back_button) {
		BROWSER_LOGE("elm_button_add failed!");
		return EINA_FALSE;
	}
	evas_object_smart_callback_add(m_title_back_button, "clicked", __title_back_button_clicked_cb, this);

	m_navi_it = elm_naviframe_item_push(m_navi_bar, "", NULL, NULL, m_main_layout, NULL);

	evas_object_smart_callback_add(m_navi_bar, "transition,finished", __naviframe_pop_finished_cb, this);

	elm_object_style_set(m_title_back_button, "browser/title_back");
	elm_object_item_part_content_set(m_navi_it, ELM_NAVIFRAME_ITEM_TITLE_LEFT_BTN, m_title_back_button);

	evas_object_show(m_title_back_button);
	evas_object_show(m_navi_bar);

	m_scroller = elm_scroller_add(m_navi_bar);
	if (!m_scroller) {
		BROWSER_LOGE("elm_scroller_add failed!");
		return EINA_FALSE;
	}
	/* Do not propagate event to scroller's ancestor */
	/* not to call unnecessary other callbacks. */
	evas_object_propagate_events_set(m_scroller, EINA_FALSE);

	/* Do not use scroller's scrollbar, use webview's scrollbar instead */
	elm_scroller_policy_set(m_scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	evas_object_size_hint_align_set(m_scroller, EVAS_HINT_FILL, 0.0);
	evas_object_size_hint_weight_set(m_scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_scroller_bounce_set(m_scroller, EINA_FALSE, EINA_FALSE);

	evas_object_show(m_scroller);

	evas_object_smart_callback_add(m_scroller, "scroll", __scroller_scroll_cb, this);
	evas_object_event_callback_add(m_scroller, EVAS_CALLBACK_RESIZE, __scoller_resize_cb, this);
	evas_object_smart_callback_add(m_scroller, "edge,bottom", __scroller_edge_bottom_cb, this);

	/* create content box which contains navigation layout & webview */
	m_content_box = elm_box_add(m_main_layout);
	if (!m_content_box) {
		BROWSER_LOGE("elm_box_add failed!");
		return EINA_FALSE;
	}

	elm_box_horizontal_set(m_content_box, EINA_FALSE);
	evas_object_size_hint_weight_set(m_content_box, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(m_content_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(m_scroller, m_content_box);
	evas_object_show(m_content_box);

	/* create dummy loading progress bar which is displayed at launching time shortly */
	m_dummy_loading_progressbar = elm_progressbar_add(m_navi_bar);
	if (!m_dummy_loading_progressbar) {
		BROWSER_LOGE("elm_progressbar_add failed!");
		return EINA_FALSE;
	}

	elm_object_style_set(m_dummy_loading_progressbar, "browser/loading_wheel");
	elm_progressbar_pulse(m_dummy_loading_progressbar, EINA_TRUE);
	evas_object_size_hint_weight_set(m_dummy_loading_progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_dummy_loading_progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(m_main_layout, "elm.swallow.waiting_progress", m_dummy_loading_progressbar);
	evas_object_show(m_dummy_loading_progressbar);

	/* create conformant */
	elm_win_conformant_set(m_win, EINA_TRUE);
	m_conformant = elm_conformant_add(m_main_layout);
	if (!m_conformant) {
		BROWSER_LOGE("elm_conformant_add failed!");
		return EINA_FALSE;
	}

	elm_object_style_set(m_conformant, "internal_layout");
	evas_object_size_hint_weight_set(m_conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_conformant, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(m_conformant, m_scroller);
	elm_object_part_content_set(m_main_layout, "elm.swallow.content", m_conformant);
	evas_object_show(m_conformant);

	m_url_layout = _create_url_layout();
	if (!m_url_layout) {
		BROWSER_LOGE("_create_url_layout failed!");
		return EINA_FALSE;
	}
	elm_box_pack_start(m_content_box, m_url_layout);

	m_control_bar = _create_control_bar();
	if (!m_control_bar) {
		BROWSER_LOGE("_create_control_bar failed!");
		return EINA_FALSE;
	}
	elm_object_part_content_set(m_main_layout, "elm.swallow.control_bar", m_control_bar);

	m_option_header_url_layout = _create_option_header_url_layout();
	if (!m_option_header_url_layout) {
		BROWSER_LOGE("_create_option_header_url_layout failed!");
		return EINA_FALSE;
	}

	m_option_header_layout = elm_layout_add(m_navi_bar);
	if (!m_option_header_layout) {
		BROWSER_LOGE("elm_layout_add failed!");
		return EINA_FALSE;
	}
	if (!elm_layout_file_set(m_option_header_layout, BROWSER_EDJE_DIR"/browser-view-url-layout.edj",
				"browser-view/option_header")) {
		BROWSER_LOGE("Can not set layout theme[browser/url_layout]\n");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_option_header_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_option_header_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_part_content_set(m_option_header_layout, "elm.swallow.url_layout", m_option_header_url_layout);
	evas_object_show(m_option_header_layout);

	Evas_Object *find_word_layout = m_find_word->get_layout();
	if (!find_word_layout) {
		BROWSER_LOGE("_create_find_word_layout failed");
		return EINA_FALSE;
	}
	elm_object_part_content_set(m_option_header_layout, "elm.swallow.find_word_layout", find_word_layout);

	elm_object_item_part_content_set(m_navi_it, ELM_NAVIFRAME_ITEM_OPTIONHEADER, m_option_header_layout);
	_navigationbar_visible_set_signal(EINA_FALSE);

	if (!_create_zoom_buttons()) {
		BROWSER_LOGE("_create_zoom_buttons failed");
		return EINA_FALSE;
	}

	edje_object_signal_callback_add(elm_layout_edje_get(m_main_layout),
					"mouse,clicked,1", "elm.rect.content_dim", __dim_area_clicked_cb, this);

	const char *current_theme = elm_theme_get(NULL);
	if (current_theme && strstr(current_theme, "white")) {
	} else {
		edje_object_signal_emit(elm_layout_edje_get(m_url_entry_layout), "black_theme,signal", "");
		edje_object_signal_emit(elm_layout_edje_get(m_option_header_url_entry_layout),
										"black_theme,signal", "");
	}

	return EINA_TRUE;
}

