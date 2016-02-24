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

#include "browser-view.h"

#include <Ecore_X.h>
#include <Elementary.h>
#include <string>
#include <ui-gadget.h>
#include <utilX.h>
#include <vconf.h>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "cloud-sync-manager.h"
#include "find-on-page.h"
#include "url-bar.h"
#include "url-input-bar.h"
#include "main-view.h"
#include "platform-service.h"
#include "preference.h"
#include "webview.h"
#include "webview-list.h"
#include "bookmark-add-view.h"
#include "platform-service.h"
#include "progress-bar.h"

#if defined(HW_MORE_BACK_KEY)
#include <efl_extension.h>
#endif

#define APP_IN_APP_BUTTON_SIZE	(40 * efl_scale)
#define APP_IN_APP_BUTTON_EVENT_SIZE	(60 * efl_scale)

#define BROWSER_KEY_CATCH_PAGE_UP "Prior"
#define BROWSER_KEY_CATCH_PAGE_DOWN "Next"
#define BROWSER_KEY_CATCH_HOME "Home"
#define BROWSER_KEY_CATCH_END "End"

#define BROWSER_KEY_CATCH_KP_PAGE_UP "KP_Prior"
#define BROWSER_KEY_CATCH_KP_PAGE_DOWN "KP_Next"
#define BROWSER_KEY_CATCH_KP_HOME "KP_Home"
#define BROWSER_KEY_CATCH_KP_END "KP_End"

#define common_edj_path browser_edj_dir"/browser-common.edj"
#define browser_genlist_edj_path browser_edj_dir"/browser-genlist.edj"

static Evas_Object *__create_conformant(Evas_Object *win)
{
	RETV_MSG_IF(!win, NULL, "win is NULL");

	Evas_Object *conformant = NULL;
	conformant = elm_conformant_add(win);
	if (!conformant)
		return NULL;

	evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, conformant);
	evas_object_show(conformant);

	return conformant;
}

static Evas_Object *__create_top_layout(Evas_Object *parent)
{
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *layout = elm_layout_add(parent);
	if (!layout)
		return NULL;

	if (!elm_layout_theme_set(layout, "layout", "application", "default"))
		BROWSER_LOGE("elm_layout_theme_set is failed.\n");

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);

	return layout;
}

static Evas_Object *_create_naviframe(Evas_Object *parent)
{
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *naviframe = elm_naviframe_add(parent);

#if defined(HW_MORE_BACK_KEY)
	// Add HW Key callbacks.
	elm_naviframe_prev_btn_auto_pushed_set(naviframe, EINA_FALSE);
	eext_object_event_callback_add(naviframe, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(naviframe, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);
#endif
	evas_object_show(naviframe);

	return naviframe;
}

browser_view::browser_view(void)
	: m_main_layout(NULL)
	, m_button_layout(NULL)
	, m_webview(NULL)
	, m_url_bar(NULL)
	, m_url_input_bar(NULL)
	, m_find_on_page(NULL)
	, m_progress_bar(NULL)
	, m_is_keypad(EINA_FALSE)
	, m_is_clipboard(EINA_FALSE)
	, m_naviframe_item(NULL)
	, m_is_landscape(EINA_FALSE)
	, m_browser_terminate_popup_win(NULL)
	, m_browser_terminate_popup(NULL)
	, m_max_window_popup(NULL)
	, m_view_mode(browser_view_mode_normal)
	, m_notipopup_check(EINA_FALSE)
	, m_view_width(0)
	, m_view_height(0)
	, m_keep_urlbar_on_resume(EINA_FALSE)
{
	BROWSER_LOGD("");
	elm_theme_extension_add(NULL, browser_genlist_edj_path);
	elm_theme_extension_add(NULL, browser_view_edj_path);

	m_main_layout = _create_layout(m_window);
	if (!m_main_layout)
		BROWSER_LOGE("_create_layout failed");

	UG_INIT_EFL(m_window, UG_OPT_INDICATOR_ENABLE);
}

browser_view::~browser_view(void)
{
	BROWSER_LOGD("");

	elm_object_part_content_unset(m_main_layout, "elm.swallow.top-tool-bar");
	elm_object_part_content_unset(m_main_layout, "elm.swallow.mini-url-bar");

	if (m_url_bar)
		delete m_url_bar;
	if (m_url_input_bar)
		delete m_url_input_bar;
	if (m_progress_bar) {
		elm_object_part_content_unset(m_main_layout, "elm.swallow.progressbar");
		delete m_progress_bar;
	}
	if (m_find_on_page)
		delete m_find_on_page;
	if (m_browser_terminate_popup_win)
		evas_object_del(m_browser_terminate_popup_win);
	if (m_button_layout)
		evas_object_del(m_button_layout);
	elm_theme_extension_del(NULL, browser_view_edj_path);
}

void browser_view::set_current_webview(webview *wv)
{
	BROWSER_LOGD("");

	main_view *mv = m_browser->get_main_view();

	if (wv && m_webview == wv) {
		if (wv->is_suspended())
			wv->resume();
		return;
	}

	webview *prev_webview = m_webview;
	m_webview = wv;
	if (prev_webview)
		prev_webview->deactivate();

	if (m_webview == NULL) {
		m_browser->get_browser_view()->get_progress_bar()->hide();
		set_webview_layout_content(NULL);
		return;
	}

	elm_naviframe_item_style_set(m_naviframe_item, "tabbar/notitle");
	set_window_view_mode(VIEW_MODE_NORMAL);

	if (wv->get_ewk_view()) {
		EWK_VIEW_SD_GET(wv->get_ewk_view(), sd);
		if (!sd)
			wv->reset_ewk_view();
	}

	m_webview->activate();
	set_webview_layout_content(wv);

	// Keep prevvious url bar status.
	mv->show_url_bar_animator(wv->get_url_bar_shown());

	if (prev_webview)
		m_browser->get_cloud_sync_manager()->activate_webview(m_webview->sync_id_get());

	// Set rotation degree.
	int degree = elm_win_rotation_get(m_window);
	m_webview->orientation_send(degree);

	// Set title.
	const char *title = wv->get_title();
	const char *url = wv->get_uri();
	if (title && strlen(title) > 0)
		get_url_bar()->set_text(title);
	else if (url)
		get_url_bar()->set_text(url);
	else
		get_url_bar()->set_text("");

	// Update progress bar if required.
	if (m_webview->is_loading()) {
		get_url_bar()->set_loading_status(EINA_TRUE);
		get_progress_bar()->update_progress(m_webview->get_progress());
	} else
		get_url_bar()->set_loading_status(EINA_FALSE);

	m_browser->get_main_view()->get_main_toolbar()->enable_backward_button(m_webview->backward_possible());
	m_browser->get_main_view()->get_main_toolbar()->enable_forward_button(m_webview->forward_possible());

	set_notipopup_check(EINA_FALSE);
#ifdef ENABLE_INCOGNITO_WINDOW
	get_url_bar()->enable_private_mode(wv->is_incognito());
#endif
}

webview *browser_view::get_current_webview(void)
{
	return m_webview;
}

void browser_view::launch(const char *uri)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);

	webview *wv = NULL;
	std::vector<tab_sync_info *> tab_sync_list = m_browser->get_cloud_sync_manager()->get_tab_sync_info_list();

	if (tab_sync_list.size()) {
		webview *current_wv = NULL;
		const char *current_uri = NULL;
		BROWSER_LOGD("tab_sync_list.size()=%d", tab_sync_list.size());
		for (unsigned int i = 0 ; i < tab_sync_list.size() ; i++) {
			BROWSER_SECURE_LOGD("index=[%d], title=[%s], uri=[%s], activated=[%d], incognito=[%d]", tab_sync_list[i]->get_index(), tab_sync_list[i]->get_title(), tab_sync_list[i]->get_uri(), tab_sync_list[i]->is_activated(), tab_sync_list[i]->is_incognito());
			Eina_Bool duplicated = EINA_FALSE;
			for (unsigned int j = 0 ; j < m_browser->get_webview_list()->get_count() ; j++) {
				if(!m_browser->get_webview_list()->get_webview(j))
					return;
				if (m_browser->get_webview_list()->get_webview(j)->sync_id_get() == tab_sync_list[i]->get_tab_id()) {
					duplicated = EINA_TRUE;
					break;
				}
			}

			if (!duplicated) {
				const char *url = tab_sync_list[i]->get_uri();
				if (url) {
					if (tab_sync_list[i]->is_activated() && !uri) {
						wv = m_browser->get_webview_list()->create_cloud_webview(tab_sync_list[i]->get_title(), url, tab_sync_list[i]->is_incognito(), tab_sync_list[i]->get_tab_id(), EINA_TRUE);
						current_uri = tab_sync_list[i]->get_uri();
						current_wv = wv;
					} else {
						wv = m_browser->get_webview_list()->create_cloud_webview(tab_sync_list[i]->get_title(), url, tab_sync_list[i]->is_incognito(), tab_sync_list[i]->get_tab_id());

						// Current webview can be NULL if the uri of activated tab is NULL.
						if (!current_wv) {
							// Make first tab as current.
							current_wv = wv;
							current_uri = tab_sync_list[i]->get_uri();
						}
					}
				} else {
					// Uri is NULL. Do not make empty tab.
					m_browser->get_cloud_sync_manager()->unsync_tab(tab_sync_list[i]->get_tab_id());
				}
			}
		}

		if (uri && strlen(uri)) {
			// Hide url input bar if exist.
			if (is_show_url_input_bar() == EINA_TRUE)
				show_url_bar();

			Eina_Bool is_exist = EINA_FALSE;

			// First, check the input uri is on the tab list. If so, just change the current webview and reload.
			for (unsigned int i = 0; i < m_browser->get_webview_list()->get_count(); i++) {
				wv = m_browser->get_webview_list()->get_webview(i);
				if(!wv)
					return;
				char *request_uri = wv->get_request_uri();
				if (request_uri == NULL)
					continue;

				unsigned int init_uri_len = strlen(request_uri);
				unsigned int uri_len = strlen(uri);
				if ((init_uri_len == uri_len && strcmp(request_uri, uri) == 0)) {
					// The tab is already exist.
					BROWSER_LOGD("Already exist. change current webview.");
					if (wv == m_browser->get_browser_view()->get_current_webview()) {
						BROWSER_LOGD("already current webview");
						wv->resume();
						wv->activate();
					} else {
						BROWSER_LOGD("Set current webview with input uri");
						set_current_webview(wv);
					}
					is_exist = EINA_TRUE;
					break;
				}
			}
			// Create new webview if the uri is not in the tab list.
			if (is_exist == EINA_FALSE) {
				if (m_browser->get_webview_list()->get_count() >= BROWSER_WINDOW_MAX_SIZE) {
					webview *old_wv = m_browser->get_webview_list()->get_old_webview();
					m_browser->get_webview_list()->delete_webview(old_wv);
				}

				wv = m_browser->get_webview_list()->create_webview(EINA_FALSE);
				wv->set_requested_webview(EINA_TRUE);
				wv->set_request_uri(uri);
				set_current_webview(wv);
			}
				wv->load_uri(uri);
		} else {
			if (current_wv) {
				set_current_webview(current_wv);
				current_wv->load_uri(current_uri);
			} else {
				if (!m_webview)
					set_current_webview(m_webview);

				if (m_webview) {
					m_webview->resume();
					m_webview->activate();
				}
			}
		}

		for (unsigned int i = 0 ; i < tab_sync_list.size() ; i++)
			delete tab_sync_list[i];
	} else {
		if (!uri && m_webview) {
			set_current_webview(m_webview);
			if (m_naviframe_item != elm_naviframe_top_item_get(m_naviframe))
				elm_naviframe_item_pop_to(m_naviframe_item);
			m_browser->clear_all_popups();
			if (m_max_window_popup)
				m_max_window_popup = NULL;

			// Set size of main scroller.
			main_view *mv = m_browser->get_main_view();
			if ( m_browser->is_tts_enabled() && mv->get_scroll_mode() == CONTENT_VIEW_SCROLL_ENABLE)
				mv->set_scroll_mode(CONTENT_VIEW_SCROLL_FIXED_TOOLBAR);
			else if (m_preference->get_hide_URL_toolbar_enabled() && !m_browser->is_tts_enabled() && mv->get_scroll_mode() == CONTENT_VIEW_SCROLL_FIXED_TOOLBAR)
				mv->set_scroll_mode(CONTENT_VIEW_SCROLL_ENABLE);

			mv->resize_scroll_content();

			return;
		} else {
			if (m_browser->get_webview_list()->get_count() >= BROWSER_WINDOW_MAX_SIZE)
				wv = m_webview;
			else if (uri) {
				wv = m_browser->get_webview_list()->create_webview(EINA_TRUE);
				wv->set_request_uri(uri);
				set_current_webview(wv);
			} else if (!m_webview) {
					wv = m_browser->get_webview_list()->create_webview(EINA_TRUE);
					set_current_webview(wv);
				}

			if (wv != NULL) {
				if (!uri || strlen(uri) == 0) {
					char *default_homepage = m_preference->get_homepage_uri();
					if (wv && default_homepage && strlen(default_homepage))
						wv->load_uri(default_homepage);
					if (default_homepage)
						free(default_homepage);
				} else
					wv->load_uri(uri);
			}
		}
	}

	// Set size of main scroller.
	main_view *mv = m_browser->get_main_view();
	if ((!m_preference->get_hide_URL_toolbar_enabled() || m_browser->is_tts_enabled()) &&
		mv->get_scroll_mode() == CONTENT_VIEW_SCROLL_ENABLE)
		mv->set_scroll_mode(CONTENT_VIEW_SCROLL_FIXED_TOOLBAR);
	mv->resize_scroll_content();

	if (m_naviframe_item != elm_naviframe_top_item_get(m_naviframe))
		elm_naviframe_item_pop_to(m_naviframe_item);
	m_browser->clear_all_popups();
	if (m_max_window_popup)
		m_max_window_popup = NULL;
}

void browser_view::on_pause(void)
{
	BROWSER_LOGD("");
	if (!m_browser->get_app_paused_by_display_off())
		m_browser->get_main_view()->deregister_callbacks();
}

Eina_Bool browser_view::is_top_view(void)
{
	if (m_naviframe_item == elm_naviframe_top_item_get(m_naviframe))
		return EINA_TRUE;

	return EINA_FALSE;
}

progress_bar *browser_view::get_progress_bar(void)
{
	if (!m_progress_bar) {
		m_progress_bar = new progress_bar(m_main_layout);
		elm_object_part_content_set(m_main_layout, "elm.swallow.progressbar", m_progress_bar->get_layout());
	}

	return m_progress_bar;
}

void browser_view::rotate(Eina_Bool landscape)
{
	BROWSER_LOGD("landscape=[%d]", landscape);

	m_is_landscape = landscape;

	if (is_show_url_input_bar())
		get_url_input_bar()->set_landscape_mode(landscape);

	if (m_is_landscape) {
		elm_object_signal_emit(m_main_layout, "show,landscape-mode,signal", "");
	} else {
		elm_object_signal_emit(m_main_layout, "show,portrait-mode,signal", "");
	}

	get_url_bar()->set_landscape_mode(landscape);
	if (m_find_on_page)
		m_find_on_page->set_landscape_mode(landscape);
}

Eina_Bool browser_view::is_show_url_bar(void)
{
	RETV_MSG_IF(!m_main_layout, EINA_FALSE, "m_main_layout is NULL");
	RETV_IF(!m_url_bar, EINA_FALSE);

	Evas_Object *toolbar = elm_object_part_content_get(m_main_layout, "elm.swallow.top-tool-bar");
	if (toolbar && toolbar == get_url_bar()->get_layout())
		return EINA_TRUE;

	return EINA_FALSE;
}

Eina_Bool browser_view::is_show_url_input_bar(void)
{
	RETV_MSG_IF(!m_main_layout, EINA_FALSE, "m_main_layout is NULL");
	RETV_IF(!m_url_input_bar, EINA_FALSE);

	Evas_Object *toolbar = elm_object_part_content_get(m_main_layout, "elm.swallow.top-tool-bar");
	if (toolbar && toolbar == get_url_input_bar()->get_layout())
		return EINA_TRUE;

	return EINA_FALSE;
}

Eina_Bool browser_view::is_show_find_on_page(void)
{
	RETV_MSG_IF(!m_main_layout, EINA_FALSE, "m_main_layout is NULL");

	if (!m_find_on_page)
		return EINA_FALSE;

	Evas_Object *toolbar = elm_object_part_content_get(m_main_layout, "elm.swallow.top-tool-bar");
	if (toolbar && toolbar == get_find_on_page()->get_layout())
		return EINA_TRUE;

	return EINA_FALSE;
}

void browser_view::hide_url_bar(void)
{
	BROWSER_LOGD("");

	if (is_show_url_bar() == EINA_FALSE) {
		BROWSER_LOGD("already hidden");

		// Just hide url input bar or find on page.
		Evas_Object *toolbar = elm_object_part_content_unset(m_main_layout, "elm.swallow.top-tool-bar");
		if (toolbar) {
			if (m_url_input_bar && toolbar == get_url_input_bar()->get_layout()) {
				// Hide url input bar.
				get_url_input_bar()->unset_entry_changed_callback();
				get_url_input_bar()->reset_string();

				// Hide dim area.
				elm_object_signal_emit(m_main_layout, "hide,uri_input_bar_dim,signal", "");
			} else if (toolbar == get_find_on_page()->get_layout()) {
				get_find_on_page()->clear_text();
			}

			evas_object_hide(toolbar);
		}

		return;
	}

	main_view *mv = m_browser->get_main_view();

	Evas_Object *toolbar = elm_object_part_content_unset(m_main_layout, "elm.swallow.top-tool-bar");
	if (toolbar)
		evas_object_hide(toolbar);

	mv->set_scroll_mode(CONTENT_VIEW_SCROLL_FIXED);
	mv->resize_scroll_content();

	// Hide dim area.
	elm_object_signal_emit(m_main_layout, "hide,uri_input_bar_dim,signal", "");
}

void browser_view::show_url_bar(void)
{
	BROWSER_LOGD("");

	main_view *mv = m_browser->get_main_view();

	if (is_show_url_bar() == EINA_TRUE) {
		BROWSER_LOGD("already show");
		return;
	}

	// Unset previous used layout.
	Evas_Object *toolbar = elm_object_part_content_unset(m_main_layout, "elm.swallow.top-tool-bar");
	if (toolbar) {
		if (m_url_input_bar && toolbar == get_url_input_bar()->get_layout()) {
			// Hide url input bar.
			get_url_input_bar()->unset_entry_changed_callback();
			get_url_input_bar()->reset_string();

			// Hide dim area.
			elm_object_signal_emit(m_main_layout, "hide,uri_input_bar_dim,signal", "");
		} else if (toolbar == get_find_on_page()->get_layout()) {
			get_find_on_page()->clear_text();
			get_find_on_page()->unset_focus();
		}

		evas_object_hide(toolbar);
	}

	if (!m_preference->get_hide_URL_toolbar_enabled() || m_browser->is_tts_enabled())
		mv->set_scroll_mode(CONTENT_VIEW_SCROLL_FIXED_TOOLBAR);
	else
		mv->set_scroll_mode(CONTENT_VIEW_SCROLL_ENABLE);
	mv->resize_scroll_content();

	// Set url bar as top tool bar.
	elm_object_part_content_set(m_main_layout, "elm.swallow.top-tool-bar", get_url_bar()->get_layout());
	evas_object_show(get_url_bar()->get_layout());

	elm_object_focus_allow_set(m_button_layout, EINA_TRUE);
}

void browser_view::show_url_input_bar(Eina_Bool enable_voice)
{
	BROWSER_LOGD("");

	main_view *mv = m_browser->get_main_view();

	Evas_Object *toolbar = elm_object_part_content_unset(m_main_layout, "elm.swallow.top-tool-bar");
	if (toolbar)
		evas_object_hide(toolbar);

	// Set url input bar as top tool bar.
	elm_object_part_content_set(m_main_layout, "elm.swallow.top-tool-bar", get_url_input_bar()->get_layout());
	evas_object_show(get_url_input_bar()->get_layout());

	mv->set_scroll_mode(CONTENT_VIEW_SCROLL_ENABLE);
	mv->resize_scroll_content();
	mv->show_toolbar(EINA_FALSE);

	webview *wv = get_current_webview();
	if (wv != NULL)
		wv->give_focus(EINA_FALSE);

	// Display dim area.
	elm_object_signal_emit(m_main_layout, "show,uri_input_bar_dim,signal", "");

	// Focus on url input bar.
	get_url_input_bar()->show_ime(enable_voice);
	get_url_input_bar()->set_entry_changed_callback();

	elm_object_focus_allow_set(m_button_layout, EINA_FALSE);
}

void browser_view::show_find_on_page(const char *word, webview *wv, Eina_Bool ime)
{
	BROWSER_LOGD("");

	main_view *mv = m_browser->get_main_view();

	Evas_Object *toolbar = elm_object_part_content_unset(m_main_layout, "elm.swallow.top-tool-bar");
	if (toolbar)
		evas_object_hide(toolbar);

	// Set find on page as top tool bar.
	elm_object_part_content_set(m_main_layout, "elm.swallow.top-tool-bar", get_find_on_page()->get_layout());
	evas_object_show(get_find_on_page()->get_layout());

	if (wv)
		get_find_on_page()->set_webview(wv);
	else
		get_find_on_page()->set_webview(get_current_webview());

	if (word)
		get_find_on_page()->set_text(word);

	// Resize scroller not to scroll find on page.
	mv->set_scroll_mode(CONTENT_VIEW_SCROLL_ENABLE);
	mv->set_scroll_position(0);
	mv->resize_scroll_content();
	mv->show_toolbar(EINA_FALSE);

	if (ime)
		get_find_on_page()->show_ime();
}

url_input_bar *browser_view::get_url_input_bar(void)
{
	if (!m_url_input_bar)
		m_url_input_bar = new url_input_bar(m_main_layout);

	return m_url_input_bar;
}

find_on_page *browser_view::get_find_on_page(void)
{
	if (!m_find_on_page)
		m_find_on_page = new find_on_page(m_main_layout);

	return m_find_on_page;
}

void browser_view::set_window_view_mode(VIEW_MODE mode)
{
	BROWSER_LOGD("%d", mode);

	if (mode == VIEW_MODE_NORMAL) {
		elm_object_signal_emit(m_main_layout, "normal_mode,on,signal", "");
		show_url_bar();
	} else if (mode == VIEW_MODE_FULLSCREEN) {
		elm_object_signal_emit(m_main_layout, "fullscreen,on,signal", "");
		hide_url_bar();
	}
}

void browser_view::__mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	browser_view *bv = static_cast<browser_view *>(data);
	edje_object_signal_emit(elm_layout_edje_get(bv->m_main_layout), "hide,block_popup_ask,signal", "");

	if (bv->is_show_find_on_page())
		bv->get_find_on_page()->unset_focus();
}

void browser_view::__ime_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	browser_view *bv = static_cast<browser_view *>(data);
	if (!bv->is_top_view())
		return;

	// Show toolbar.
	if (!bv->is_show_url_input_bar() && !bv->is_show_find_on_page() && !m_browser->get_full_screen_enable()) {
		main_view *mv = m_browser->get_main_view();
		if (!m_preference->get_hide_URL_toolbar_enabled())
			mv->set_scroll_mode(CONTENT_VIEW_SCROLL_FIXED_TOOLBAR);
	}

	if (bv->is_show_find_on_page()) {
		std::string msg = std::string(" ") + std::string(BR_STRING_TEXT_FIELD_T) + std::string(BR_STRING_DOUBLE_TAP_TO_OPEN_KEYBOARD_T );
		if (elm_entry_entry_get(bv->m_find_on_page->get_entry()))
			msg = std::string(elm_entry_entry_get(bv->m_find_on_page->get_entry())) + msg ;
		else
			msg = std::string(BR_STRING_FIND_ON_PAGE) + msg;
		elm_access_say(msg.c_str());
	}
}

void browser_view::__ime_show_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	browser_view *bv = static_cast<browser_view *>(data);
	if (!bv->is_top_view())
		return;

	Eina_Bool enable_hide_urlbar = m_preference->get_hide_URL_toolbar_enabled();
	main_view *mv = m_browser->get_main_view();

	// Do not scroll url bar during ime is displayed.
	if (enable_hide_urlbar && bv->m_keep_urlbar_on_resume) {
		if (bv->is_show_url_bar() == EINA_TRUE) {
			BROWSER_LOGD("ime with url bar show");
			mv->show_url_bar(EINA_FALSE);
		}
	}

	// Hide toolbar.
	if (!enable_hide_urlbar)
		mv->set_scroll_mode(CONTENT_VIEW_SCROLL_FIXED);
	mv->show_toolbar(EINA_FALSE);

	if (bv->is_show_url_input_bar()) {
		// Select all text in url input bar.
		BROWSER_LOGD("focus set");
		bv->get_url_input_bar()->set_focus();
	}

    // Hide url bar when IME is shown and focused on web content.
    if(!bv->get_url_input_bar()->get_focus() && !elm_object_focus_get(bv->get_find_on_page()->get_entry()))
        mv->show_url_bar(EINA_FALSE);
}

void browser_view::__keypad_show_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	browser_view *bv = static_cast<browser_view *>(data);

	if (bv->m_is_keypad)
		return;

	bv->m_is_keypad = EINA_TRUE;

	if (bv->m_is_clipboard)
		return;

	__ime_show_cb(data, obj, event_info);
}

void browser_view::__keypad_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	browser_view *bv = static_cast<browser_view *>(data);

	if (!bv->m_is_keypad)
		return;

	bv->m_is_keypad = EINA_FALSE;

	if (bv->m_is_clipboard)
		return;

	__ime_hide_cb(data, obj, event_info);
}

void browser_view::__clipboard_show_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	browser_view *bv = static_cast<browser_view *>(data);

	if (bv->m_is_clipboard)
		return;

	bv->m_is_clipboard = EINA_TRUE;

	if (bv->m_is_keypad)
		return;

	__ime_show_cb(data, obj, event_info);
	m_browser->get_main_view()->resize_scroll_content();
}

void browser_view::__clipboard_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	browser_view *bv = static_cast<browser_view *>(data);

	if (!bv->m_is_clipboard)
		return;

	bv->m_is_clipboard = EINA_FALSE;

	if (bv->m_is_keypad)
		return;

	__ime_hide_cb(data, obj, event_info);
	m_browser->get_main_view()->resize_scroll_content();
}

void browser_view::resize_main_layout(int width, int height)
{
	if (m_browser->get_main_view()->get_scroll_mode() != CONTENT_VIEW_SCROLL_ENABLE &&
		!m_browser->get_browser_view()->is_ime_on()) {
		BROWSER_LOGD("not scroll mode");
		Evas_Object *bg = elm_object_part_content_get(m_main_layout, "bg");
		if (bg) {
			evas_object_size_hint_min_set(bg, 0, 0);
			evas_object_size_hint_max_set(bg, 9999, 9999); // Enough values over than screen resolution.
			m_view_width = 0;
			m_view_height = 0;
		}

		return;
	}

	BROWSER_LOGD("%d, %d", width, height);

	if ((m_view_width == width) && (m_view_height == height)) {
		BROWSER_LOGD("Main layout size is the same as before");
		return;
	}

	m_view_width = width;
	m_view_height = height;

	Evas_Object *bg = elm_object_part_content_get(m_main_layout, "bg");
	if (bg) {
		evas_object_size_hint_min_set(bg, width, height);
		evas_object_size_hint_max_set(bg, width, height);
	}
}

Eina_Bool browser_view::is_valid_uri(const char *uri)
{
	BROWSER_SECURE_LOGD("uri=[%s]", uri);
	int len = strlen(uri);
	if (*uri != '.' && *(uri + len - 1) != '.' && strstr(uri, ".")) {
		BROWSER_LOGD("url tmp expression");
		if (!strncasecmp("www.", uri, strlen("www.")) || !strncasecmp("http://www.", uri, strlen("http://www.")) || !strncasecmp("https://www.", uri, strlen("https://www."))) {
			std::string str(uri);
			std::size_t first_occurance = str.find_first_of(".");
			std::size_t last_occurance = str.find_last_of(".");
			if ((first_occurance != std::string::npos) && (last_occurance != std::string::npos) && (first_occurance != last_occurance))
				return EINA_TRUE;
			else
				return EINA_FALSE;
		}
		return EINA_TRUE;
	}

	return EINA_FALSE;
}

void browser_view::set_focus_on_main_layout(void)
{
	elm_object_focus_set(m_main_layout, EINA_TRUE);
}

void browser_view::set_focus_on_content(void)
{
	BROWSER_LOGD("");

	if (m_browser->is_keyboard_active() == EINA_FALSE) {
		elm_object_focus_allow_set(m_button_layout, EINA_FALSE);
		return;
	}

	if (get_view_mode() == browser_view_mode_normal) {
		// Focus on webview.
		BROWSER_LOGD("focus on webview");
		elm_object_focus_allow_set(m_button_layout, EINA_TRUE);
		elm_object_focus_set(m_button_layout, EINA_TRUE);

		get_url_bar()->set_focus_on_text();
	} else {
		BROWSER_LOGD("native layout");
		elm_object_focus_allow_set(m_button_layout, EINA_FALSE);
	}
}

Evas_Object *browser_view::_create_layout(Evas_Object *window)
{
	RETV_MSG_IF(!window, NULL, "parent is NULL");

	Evas_Object *conformant = __create_conformant(m_window);
	if (!conformant) {
		BROWSER_LOGE("__create_conformant failed");
		return NULL;
	}

	Evas_Object * conformant_bg = elm_bg_add(conformant);
	elm_object_style_set(conformant_bg, "indicator/headerbg");
	elm_object_part_content_set(conformant, "elm.swallow.indicator_bg", conformant_bg);
	evas_object_show(conformant_bg);

	Evas_Object *top_layout = __create_top_layout(conformant);
	if (!top_layout) {
		BROWSER_LOGE("__create_top_layout failed");
		return NULL;
	}

	m_naviframe = _create_naviframe(top_layout);
	if (!m_naviframe) {
		BROWSER_LOGE("_create_naviframe failed");
		return NULL;
	}

	elm_object_part_content_set(top_layout, "elm.swallow.content", m_naviframe);
	elm_object_content_set(conformant, top_layout);

	m_browser->get_main_view()->create_layout(conformant);
	Evas_Object *main_view_layout = m_browser->get_main_view()->get_layout();
	Evas_Object *main_layout = elm_layout_add(main_view_layout);

	if (!main_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(main_layout, browser_edj_dir"/browser-view.edj", "webview-layout");

	evas_object_smart_callback_add(conformant, "virtualkeypad,state,on", __keypad_show_cb, this);
	evas_object_smart_callback_add(conformant, "virtualkeypad,state,off", __keypad_hide_cb, this);

	evas_object_smart_callback_add(conformant, "clipboard,state,on", __clipboard_show_cb, this);
	evas_object_smart_callback_add(conformant, "clipboard,state,off", __clipboard_hide_cb, this);

	// Gesture rect covers content area.
	Evas_Object *gesture_rect = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(main_layout), "gesture_rect");
	evas_object_event_callback_add(gesture_rect, EVAS_CALLBACK_MOUSE_UP, __mouse_up_cb, this);

	// Button layout for focus UI.
	m_button_layout = _create_button_layout(main_layout);
	if (m_button_layout)
		evas_object_show(m_button_layout);

	// Url bar.
	m_url_bar = new url_bar(main_layout);
	elm_object_part_content_set(main_layout, "elm.swallow.top-tool-bar", m_url_bar->get_layout());
	evas_object_show(m_url_bar->get_layout());

	m_browser->get_main_view()->set_scroll_content(main_layout);
	m_naviframe_item = elm_naviframe_item_push(m_naviframe, NULL, NULL, NULL, main_view_layout, "tabbar/notitle");

	elm_object_item_signal_emit(m_naviframe_item, "elm,state,toolbar,close", "");
	elm_naviframe_item_pop_cb_set(m_naviframe_item, __naviframe_pop_cb, this);

	evas_object_smart_callback_add(m_naviframe, "transition,finished", __naviframe_pop_finished_cb, this);

	// Attach background rectangle for scroller.
	Evas_Object *background = evas_object_rectangle_add(evas_object_evas_get(main_layout));
	RETV_MSG_IF(!background, NULL, "evas_object_rectangle_add failed.");

	evas_object_color_set(background, 0, 0, 0, 0);
	elm_object_part_content_set(main_layout, "bg", background);

	Evas_Object *uri_input_bar_dim = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(main_layout), "uri_input_bar_dim");
	Evas_Object *uri_input_bar_dim_object = elm_access_object_register(uri_input_bar_dim, main_layout);
	elm_access_info_set(uri_input_bar_dim_object, ELM_ACCESS_INFO, BR_STRING_DOUBLE_TAP_TO_CANCEL_THE_SEARCH_T_TALKBACK);
	elm_access_activate_cb_set(uri_input_bar_dim_object, __uri_input_bar_access_clicked_cb, this);
	edje_object_signal_callback_add(elm_layout_edje_get(main_layout), "mouse,clicked,1", "uri_input_bar_dim", __uri_input_bar_dim_clicked_cb, this);

	elm_win_conformant_set(m_window, EINA_TRUE);

	// Set fullscreen view mode.
	BROWSER_LOGD("set fullscreen view mode");
	edje_object_signal_emit(elm_layout_edje_get(main_layout), "hide,find-on-page_fullscreen,signal", "");
	elm_win_indicator_mode_set(m_window, ELM_WIN_INDICATOR_SHOW);
	if (m_preference->get_indicator_fullscreen_view_enabled() == EINA_TRUE)
		elm_win_indicator_opacity_set(m_window, ELM_WIN_INDICATOR_TRANSPARENT);
	else
		elm_win_indicator_opacity_set(m_window, ELM_WIN_INDICATOR_OPAQUE);

	elm_object_signal_emit(conformant, "elm,state,indicator,overlap", "");
	evas_object_data_set(conformant, "overlap", (void *)EINA_TRUE);

	m_conformant = conformant;

	return main_layout;
}

void browser_view::show_max_window_limit_reached_popup(void)
{
	BROWSER_LOGD("");

	if (m_max_window_popup)
		return;

	m_max_window_popup = m_browser->get_browser_view()->show_msg_popup("IDS_BR_HEADER_WINDOW_LIMIT_REACHED",
								"IDS_BR_BODY_TAP_THE_WINDOW_MANAGER_ICON_AND_CLOSE_SOME_WINDOWS_THEN_TRY_AGAIN",
								__max_window_limit_reached_ok_cb,
								"IDS_BR_SK_OK",
								__max_window_limit_reached_ok_cb,
								NULL,
								NULL,
								NULL);
}

void browser_view::hide_all_views_return_main_view(void)
{
	BROWSER_LOGD("");

	if (m_naviframe_item != elm_naviframe_top_item_get(m_naviframe))
		elm_naviframe_item_pop_to(m_naviframe_item);
}

void browser_view::__max_window_limit_reached_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	if (m_browser->get_browser_view()->m_max_window_popup)
		m_browser->get_browser_view()->m_max_window_popup = NULL;
}

void browser_view::set_webview_layout_content(webview *wv)
{
	BROWSER_LOGD("");

	Evas_Object *prev_view = elm_object_part_content_unset(m_main_layout, "elm.swallow.content");
	if (prev_view) {
		evas_object_hide(prev_view);
		/* prev_view is as ewk_view on button layout */
		if (m_browser->get_webview_list()->get_webview(prev_view) != NULL) {
			elm_access_object_unregister(prev_view);
		}
	}

	if (!wv || !wv->get_ewk_view()) {
		BROWSER_LOGD("!wv || !wv->get_ewk_view()");
		return;
	}

	Evas_Object *ewk_view = wv->get_ewk_view();
	elm_object_part_content_set(m_main_layout, "elm.swallow.content", ewk_view);
	elm_access_object_register(ewk_view, m_main_layout);

	evas_object_focus_set(ewk_view, EINA_TRUE);
	evas_object_show(ewk_view);
}

void browser_view::__button_layout_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	if (!m_browser->is_keyboard_active())
		return;

	browser_view *bv = static_cast<browser_view *>(data);
	webview *wv = bv->get_current_webview();
	if (!wv) {
		BROWSER_LOGD("webview is null");
		return;
	}

	wv->give_focus(EINA_TRUE);
}

void browser_view::__button_layout_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	if (!m_browser->is_keyboard_active())
		return;

	browser_view *bv = static_cast<browser_view *>(data);
	webview *wv = bv->get_current_webview();
	if (!wv) {
		BROWSER_LOGD("webview is null");
		return;
	}

	wv->give_focus(EINA_FALSE);
}

void browser_view::__uri_input_bar_access_clicked_cb(void *data, Evas_Object *obj, Elm_Object_Item *item)
{
	BROWSER_LOGD("");
	browser_view *bv = static_cast<browser_view *>(data);
	bv->show_url_bar();
}

void browser_view::__uri_input_bar_dim_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("");

	m_browser->get_browser_view()->show_url_bar();
}

Evas_Object *browser_view::_create_button_layout(Evas_Object *parent)
{
	if (m_browser->is_tts_enabled())
		return NULL;

	Evas_Object *button_layout = elm_button_add(parent);
	if (!button_layout) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(button_layout, "transparent");
	evas_object_smart_callback_add(button_layout, "focused", __button_layout_focused_cb, this);
	evas_object_smart_callback_add(button_layout, "unfocused", __button_layout_unfocused_cb, this);
	elm_object_part_content_set(parent, "elm.swallow.button-layout", button_layout);
	elm_access_info_set(button_layout, ELM_ACCESS_TYPE, "");

	return button_layout;
}

Eina_Bool browser_view::__naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, EINA_FALSE, "data is NULL");

	browser_view *bv = static_cast<browser_view *>(data);
	webview *wv = m_browser->get_browser_view()->get_current_webview();

	if (wv && wv->get_text_selection_mode_clear() == EINA_TRUE) {
		BROWSER_LOGD("text selection mode cleared");
		return EINA_FALSE;
	}

	// Check fullscreen mode.
	if (wv && m_browser->get_full_screen_enable()) {
		// Exit fullscreen mode.
		wv->exit_fullscreen_mode();

		return EINA_FALSE;
	}

	if (bv->is_top_view() == EINA_TRUE) {
		if (!wv) {
			elm_win_lower(m_window);
			return EINA_FALSE;
		}
		// Go back or hide window. This happens when the back key is pressed.

		if (bv->get_notipopup_check() == EINA_TRUE) {
			if (wv->backward_possible()) {
				bv->set_notipopup_check(EINA_FALSE);
				wv->backward();
			} else {
				bv->set_notipopup_check(EINA_FALSE);
				elm_win_lower(m_window);
			}
		} else if (wv->backward_possible())
			wv->backward();
		else if (wv->is_user_created() || (m_browser->get_webview_list()->get_count() == 1)) {
			if (bv->get_notipopup_check() == EINA_FALSE)
				elm_win_lower(m_window);
			bv->set_notipopup_check(EINA_TRUE);
		}
		else {
			if (wv->get_requested_webview() == EINA_TRUE) {
				BROWSER_LOGD("requested webview");
				elm_win_lower(m_window);
			}

			webview *replace_wv = m_browser->get_webview_list()->delete_webview(wv);
			BROWSER_LOGD("Replace webview: %p", replace_wv);
			if (replace_wv)
				m_browser->get_browser_view()->set_current_webview(replace_wv);
		}
	}

	return EINA_FALSE;
}

void browser_view::__naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	browser_view *bv = static_cast<browser_view *>(data);
	webview *wv = bv->get_current_webview();
	if (bv->m_naviframe_item != elm_naviframe_top_item_get(m_naviframe)) {
		if (wv)
			wv->set_page_visibility_state(EINA_FALSE);
		return;
	}

	m_browser->delete_settings();
	m_browser->delete_bookmark_add_view();
	m_browser->delete_certificate_view();
	m_browser->delete_history_view_view();
	m_browser->delete_bookmark_view();
	m_browser->delete_tab_manager_view();

	url_bar *ub = bv->get_url_bar();
	if (ub) {
		// Update star icon status.
		ub->update_star_icon();
		// Update secure icon status.
		ub->update_secure_icon();
		// Update fav icon status.
		ub->update_fav_icon();
	}
	// Set toolbar visibility.
	main_view *mv = m_browser->get_main_view();
	if (mv && wv) {
		mv->show_toolbar(wv->get_url_bar_shown());
		wv->set_page_visibility_state(EINA_TRUE);
	}
}

void browser_view::apply_changed_language(void)
{
	BROWSER_LOGD("");
	browser_view *bv = m_browser->get_browser_view();
	webview *wv = bv->get_current_webview();
	if(wv)
		wv->reload();
}

void browser_view::pop_to_browser_view(void)
{
	BROWSER_LOGD("");
	if (m_naviframe_item != elm_naviframe_top_item_get(m_naviframe))
		elm_naviframe_item_pop_to(m_naviframe_item);
}
