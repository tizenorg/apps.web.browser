/*
 * Copyright 2014  Samsung Electronics Co., Ltd
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
 * Contact: Kwangyong Choi <ky0.choi@samsung.com>
 *
 */

#include "url-bar.h"

#include "bookmark.h"
#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "history.h"
#include "main-view.h"
#include "preference.h"
#include "progress-bar.h"
#include "certificate-manager.h"

#if defined(HW_MORE_BACK_KEY)
#include <efl_extension.h>
#endif

#define URL_BAR_EDJ browser_edj_dir"/url-bar.edj"
#define TEXT_LAYOUT_BUTTON_SIZE 25

url_bar::url_bar(Evas_Object *parent)
	: m_more_menu_manager(NULL)
	, m_main_layout(NULL)
	, m_text_layout(NULL)
	, m_entry(NULL)
	, m_tab_manager_button(NULL)
	, m_secure_icon(NULL)
	, m_star_icon(NULL)
	, m_uri_box_object(NULL)
	, m_tab_manager_access(NULL)
	, m_reload_cancel_icon(NULL)
	, m_fixed_mode(EINA_FALSE)
	, m_loading(EINA_FALSE)
	, m_is_show_star_icon(EINA_TRUE)
{
	BROWSER_LOGD("");

	elm_theme_extension_add(NULL, URL_BAR_EDJ);

	m_more_menu_manager= new more_menu_manager();
	_create_main_layout(parent);

	vconf_notify_key_changed(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, __font_size_changed_cb, this);
}

url_bar::~url_bar(void)
{
	BROWSER_LOGD("");

	vconf_ignore_key_changed(VCONFKEY_SETAPPL_ACCESSIBILITY_FONT_SIZE, __font_size_changed_cb);

	delete m_more_menu_manager;

	elm_theme_extension_del(NULL, URL_BAR_EDJ);
}

void url_bar::set_text(const char *text, Eina_Bool update_icon)
{
	BROWSER_SECURE_LOGD("%s", text);
	RET_MSG_IF(!text, "text is NULL");

	if (m_fixed_mode) {
		elm_object_part_text_set(m_text_layout, "uri_text", BR_STRING_URL_GUIDE_TEXT);

		if (m_uri_box_object) {
			std::string msg = std::string(BR_STRING_SEARCH_FIELD_T) + " " + BR_STRING_DOUBLE_TAP_TO_OPEN_KEYBOARD_T;
			elm_access_info_set(m_uri_box_object, ELM_ACCESS_INFO, msg.c_str());
		}

		return;
	}

	browser_view *bv = m_browser->get_browser_view();
	char *visible_text = NULL;
	visible_text = strndup(text, URI_VISIBLE_LENGTH);

	if (!visible_text || strlen(visible_text) == 0) {
		if (visible_text != NULL)
			free(visible_text);
		return;
	}

	char *markup = elm_entry_utf8_to_markup(visible_text);
	char *markup_text = markup;
	if (markup_text == NULL)
		markup_text = visible_text;

	if (!strncmp(markup_text, http_scheme, strlen(http_scheme)) || !strncmp(markup_text, https_scheme, strlen(https_scheme))) {
		char *ptr = strstr(markup_text,"://");
		if (ptr != NULL)
			markup_text = ptr + 3;	// to remove protocols with ://
	}

	std::string font_size_tag = "<" + bv->get_font_size_tag() + ">";
	elm_object_part_text_set(m_text_layout, "uri_text", (std::string(font_size_tag) + markup_text + "</font>").c_str());

	if (m_uri_box_object) {
		std::string msg;
		msg = markup_text;
		msg = msg + " " + BR_STRING_SEARCH_FIELD_T + " " + BR_STRING_DOUBLE_TAP_TO_OPEN_KEYBOARD_T;
		elm_access_info_set(m_uri_box_object, ELM_ACCESS_INFO, msg.c_str());
	}

	free(markup);
	free(visible_text);

	if (update_icon) {
		update_star_icon();
		update_secure_icon();
		update_fav_icon();
	}
}

void url_bar::update_star_icon(void)
{
	BROWSER_LOGD("");

	if (m_fixed_mode) {
		BROWSER_LOGD("fixed mode");
		return;
	}

	webview *wv = m_browser->get_browser_view()->get_current_webview();
	if (!wv) {
		elm_access_info_set(m_star_icon, ELM_ACCESS_STATE, BR_STRING_OFF);
		elm_object_style_set(m_star_icon, "browser/text-layout/star_off");

		return;
	}

	// Do not show star icon in case of saved page or about:blank.
	if (wv->is_empty_page_check()) {
		BROWSER_LOGD("hide");
		show_star_icon(EINA_FALSE);
		return;
	}

	if (wv->get_uri() != NULL && strncmp(wv->get_uri(), file_scheme, strlen(file_scheme)) == 0) {
		BROWSER_LOGD("file_scheme");
		show_star_icon(EINA_FALSE);
		return;
	} else {
		BROWSER_LOGD("http scheme");
		show_star_icon(EINA_TRUE);
	}

	const char *current_uri = wv->get_uri();
	if (m_browser->get_bookmark()->is_in_bookmark(current_uri)) {
		elm_object_style_set(m_star_icon, "browser/text-layout/star_on");
		elm_access_info_set(m_star_icon, ELM_ACCESS_STATE, BR_STRING_ON);
	} else {
		elm_object_style_set(m_star_icon, "browser/text-layout/star_off");
		elm_access_info_set(m_star_icon, ELM_ACCESS_STATE, BR_STRING_OFF);
	}
}

void url_bar::show_star_icon(Eina_Bool show)
{
	BROWSER_LOGD("%d", show);

	// Do not show star icon in case of saved page or about:blank.
	webview *wv = m_browser->get_browser_view()->get_current_webview();
	if (wv && wv->is_empty_page_check()) {
		BROWSER_LOGD("hide");
		show = EINA_FALSE;
	}

	m_is_show_star_icon = show;

	if (show) {
		// Show star icon. Do not change other icons.
		elm_object_signal_emit(m_text_layout, "set,star-icon,signal", "");
	} else {
		// Hide star icon. Do not change other icons.
		elm_object_signal_emit(m_text_layout, "set,no-star-icon,signal", "");
	}
}

void url_bar::show_secure_icon(Eina_Bool show)
{
	BROWSER_LOGD("%d", show);

	if (m_is_show_star_icon == EINA_FALSE) {
		BROWSER_LOGD("no star icon");
		return;
	}

	// Do not show secure icon in case of saved page or about:blank.
	webview *wv = m_browser->get_browser_view()->get_current_webview();
	if (wv && (
#if defined(ENABLE_SAVED_PAGE)
	wv->is_saved_pages_check() ||
#endif
	wv->is_empty_page_check())) {
		BROWSER_LOGD("do not update");
		return;
	}

	if (m_fixed_mode) {
		BROWSER_LOGD("fixed mode");
		return;
	}

#if defined(QUICK_FEED)
	if (wv && wv->is_page_rss_scrap()) {
		BROWSER_LOGD("hide");
		show_star_icon(EINA_FALSE);
		return;
	}
#endif

	if (show) {
		// Show secure icon. Do not change other icons.
		elm_object_signal_emit(m_text_layout, "set,secure-icon,signal", "");
	} else {
		// Hide secure icon. Do not change other icons.
		elm_object_signal_emit(m_text_layout, "set,no-secure-icon,signal", "");
	}
}

void url_bar::update_secure_icon(void)
{
	BROWSER_LOGD("");

	if (m_fixed_mode) {
		BROWSER_LOGD("fixed mode");
		return;
	}

	if (m_is_show_star_icon == EINA_FALSE) {
		BROWSER_LOGD("no star icon");
		return;
	}

	browser_view *bv = m_browser->get_browser_view();
	webview *wv = bv->get_current_webview();
	RET_MSG_IF(!wv, "current webview is null");

	certificate_manager::CERT_TYPE cert_type = wv->get_certificate_manager()->get_cert_type();
	const char *pcUri = wv->get_uri();
	if ((pcUri && strncmp(http_scheme, pcUri, strlen(http_scheme)) == 0)
		|| wv->is_error_page()) {
		BROWSER_LOGD("Http Page Or error Page");
		cert_type = certificate_manager::NONE;
	}

	BROWSER_LOGD("cert_type: %d", cert_type);

	if (cert_type != certificate_manager::NONE) {
		show_secure_icon (EINA_TRUE);
		if (cert_type == certificate_manager::VALID) {
			elm_object_style_set(m_secure_icon,
				"browser/text-layout/secure_on");
			elm_access_info_set(m_secure_icon, ELM_ACCESS_INFO,
				BR_STRING_SECURED_PAGE);
		} else {
			elm_object_style_set(m_secure_icon,
				"browser/text-layout/secure_off");
			elm_access_info_set(m_secure_icon, ELM_ACCESS_INFO,
				BR_STRING_UNTRUSTED_PAGE);
		}
		elm_access_info_set(m_secure_icon, ELM_ACCESS_STATE,
			BR_STRING_DOUBLE_TAP_VIEW_CERTIFICATE);
	} else {
		show_secure_icon (EINA_FALSE);
	}
}

void url_bar::update_fav_icon(void)
{
	BROWSER_LOGD("");

	if (m_fixed_mode) {
		BROWSER_LOGD("fixed mode");
		return;
	}

	webview *wv = m_browser->get_browser_view()->get_current_webview();
	if (!wv) {
		show_fav_icon(false);
		return;
	}

	Evas_Object *favicon = wv->get_favicon();
	if (favicon) {
		elm_object_part_content_set(m_text_layout, "elm.swallow.fav-icon", favicon);
		show_fav_icon(true);
	} else {
		show_fav_icon(false);
	}
}

void url_bar::show_fav_icon(Eina_Bool show)
{
	BROWSER_LOGD("%d", show);

	if (show) {
		// Show fav icon. Do not change other icons.
		elm_object_signal_emit(m_text_layout, "set,fav-icon,signal", "");
	} else {
		// Hide fav icon. Do not change other icons.
		elm_object_signal_emit(m_text_layout, "set,no-fav-icon,signal", "");
	}
}

void url_bar::show_reload_cancel_icon(Eina_Bool show)
{
	BROWSER_LOGD("%d", show);

	if (show)
		elm_object_signal_emit(m_text_layout, "show,reload-cancel-icon,signal", "");
	else
		elm_object_signal_emit(m_text_layout, "hide,reload-cancel-icon,signal", "");
}

void url_bar::set_loading_status(Eina_Bool loading)
{
	BROWSER_LOGD("%d", loading);

	m_loading = loading;

	browser_view *bv = m_browser->get_browser_view();

	if (loading) {
		// Set cancel icon.
		elm_access_info_set(m_reload_cancel_icon, ELM_ACCESS_INFO, BR_STRING_CTXMENU_STOP);
		elm_object_style_set(m_reload_cancel_icon, "browser/text-layout/cancel");

		// Show progress bar.
		bv->get_progress_bar()->show();
	} else {
		// Set reload icon.
		elm_access_info_set(m_reload_cancel_icon, ELM_ACCESS_INFO, BR_STRING_REFRESH);
		elm_object_style_set(m_reload_cancel_icon, "browser/text-layout/reload");

		// Hide progress bar.
		bv->get_progress_bar()->hide();
	}
}


#ifdef ENABLE_INCOGNITO_WINDOW
void url_bar::enable_private_mode(Eina_Bool enable)
{
	BROWSER_LOGD("%d", enable);
	if (enable) {
		// Set private mode url bar.
		elm_object_signal_emit(m_main_layout, "show,private-mode,signal", "");
		elm_object_style_set(m_tab_manager_button, "browser/url-bar/tab_manager_pr");
	} else {
		// Set normal mode url bar.
		elm_object_signal_emit(m_main_layout, "hide,private-mode,signal", "");
		elm_object_style_set(m_tab_manager_button, "browser/url-bar/tab_manager");
	}
}
#endif /* ENABLE_INCOGNITO_WINDOW */

void url_bar::set_landscape_mode(Eina_Bool landscape)
{
	if (landscape) {
		elm_object_signal_emit(m_main_layout, "show,landscape-mode,signal", "");
	} else {
		elm_object_signal_emit(m_main_layout, "show,portrait-mode,signal", "");
	}
}

void url_bar::set_fixed_mode(Eina_Bool mode)
{
	BROWSER_LOGD("%d", mode);

	if (m_fixed_mode == mode)
		return;

	main_view *mv = m_browser->get_main_view();

	m_fixed_mode = mode;

	if (mode) {
		show_star_icon(EINA_FALSE);
		show_reload_cancel_icon(EINA_FALSE);

		elm_object_signal_emit(m_text_layout, "set,fixed-bar,signal", "");

		m_browser->get_browser_view()->get_progress_bar()->hide();

		mv->set_scroll_mode(CONTENT_VIEW_SCROLL_FIXED_TOOLBAR);

		webview *wv = m_browser->get_browser_view()->get_current_webview();
		if (wv != NULL)
			wv->set_url_bar_shown(EINA_TRUE);
	} else {
		show_star_icon(EINA_TRUE);
		show_reload_cancel_icon(EINA_TRUE);

		elm_object_signal_emit(m_text_layout, "set,default-bar,signal", "");
		if (!m_preference->get_hide_URL_toolbar_enabled() || m_browser->is_keyboard_active() || m_browser->is_tts_enabled())
			mv->set_scroll_mode(CONTENT_VIEW_SCROLL_FIXED_TOOLBAR);
		else
			mv->set_scroll_mode(CONTENT_VIEW_SCROLL_ENABLE);
	}

	mv->resize_scroll_content();
}

void url_bar::set_focus_on_text(void)
{
	elm_object_focus_set(m_uri_box_object, EINA_TRUE);
}

void url_bar::set_highlight_on_text(void)
{
	BROWSER_LOGD("");
	elm_access_highlight_set(m_uri_box_object);
}

void url_bar::focus_allow_set(Eina_Bool show)
{
	BROWSER_LOGD("%d", show);

	if (show)
		elm_object_tree_focus_allow_set(m_main_layout, EINA_TRUE);
	else
		elm_object_tree_focus_allow_set(m_main_layout, EINA_FALSE);
}

void url_bar::enable_tab_manager_button(Eina_Bool enable)
{
	elm_object_disabled_set(m_tab_manager_button, !enable);
	if (m_browser->is_tts_enabled()) {
		if (enable) {
			elm_access_highlight_set(m_browser->get_main_view()->get_layout());
			elm_access_info_set(m_tab_manager_button, ELM_ACCESS_STATE, BR_STRING_OPT_ENABLED);
		}
		else
			elm_access_info_set(m_tab_manager_button, ELM_ACCESS_STATE, BR_STRING_OPT_DISABLED);
	}
}

void url_bar::language_changed(void)
{
	BROWSER_LOGD("");

	elm_object_part_text_set(m_tab_manager_button, "elm.text", BR_STRING_TABS);
}

void url_bar::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");

	// Top url bar layout.
	m_main_layout = elm_layout_add(parent);
	RET_MSG_IF(!m_main_layout, "elm_layout_add failed.");

	elm_layout_file_set(m_main_layout, URL_BAR_EDJ, "url-bar-layout");

#if defined(HW_MORE_BACK_KEY)
	// Add HW key callbacks.
	eext_object_event_callback_add(m_main_layout, EEXT_CALLBACK_MORE, __menu_clicked_cb, this);
	eext_object_event_callback_add(m_main_layout, EEXT_CALLBACK_BACK, __back_cb, NULL);
#endif

	// Create text layout.
	_create_text_layout(m_main_layout);

	// Tab manager button.
	m_tab_manager_button = elm_button_add(m_main_layout);
	RET_MSG_IF(!m_tab_manager_button, "elm_button_add failed.");

	elm_object_style_set(m_tab_manager_button, "browser/url-bar/tab_manager");
	elm_object_part_text_set(m_tab_manager_button, "elm.text", BR_STRING_TABS);
	evas_object_smart_callback_add(m_tab_manager_button, "clicked", __tab_manager_clicked_cb, this);
	elm_object_part_content_set(m_main_layout, "elm.swallow.tab-manager-button", m_tab_manager_button);
	elm_access_object_unregister(m_tab_manager_button);

	Evas_Object *tab_manager_access = elm_access_object_register(m_tab_manager_button,m_text_layout);
	elm_access_activate_cb_set(tab_manager_access, __access_tab_manager_clicked_cb, this);
	elm_access_info_set(tab_manager_access, ELM_ACCESS_INFO, BR_STRING_OPEN_WINDOWS_T);
	elm_access_info_set(tab_manager_access, ELM_ACCESS_TYPE, BR_STRING_BUTTON_T);
	m_tab_manager_access = tab_manager_access;

	elm_object_focus_custom_chain_append(m_text_layout, m_star_icon, NULL);
	elm_object_focus_custom_chain_append(m_text_layout, m_uri_box_object, NULL);
	elm_object_focus_custom_chain_append(m_text_layout, m_reload_cancel_icon, NULL);
	elm_object_focus_custom_chain_append(m_text_layout, tab_manager_access, NULL);

#ifdef ENABLE_INCOGNITO_WINDOW
	if (m_preference->get_incognito_mode_enabled())
		enable_private_mode(EINA_TRUE);
#endif /*ENABLE_INCOGNITO_WINDOW*/

	if (m_browser->is_tts_enabled()) {
		set_highlight_on_text();
	}
}

void url_bar::_create_text_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");

	// Set finger size to make small buttons.
	Evas_Coord f_size = TEXT_LAYOUT_BUTTON_SIZE;
	elm_config_finger_size_set(f_size);

	// Text display layout.
	m_text_layout = elm_layout_add(m_main_layout);
	RET_MSG_IF(!m_text_layout, "elm_layout_add failed.");

	elm_layout_file_set(m_text_layout, URL_BAR_EDJ, "text-layout");
	elm_object_part_content_set(m_main_layout, "elm.swallow.text-layout", m_text_layout);

	elm_object_signal_callback_add(m_text_layout, "mouse,clicked,1", "uri_text", __text_clicked_cb, this);

	Evas_Object *uri_box = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(m_text_layout), "uri_text,access");
	m_uri_box_object = elm_access_object_register(uri_box, m_text_layout);
	elm_access_info_set(m_uri_box_object, ELM_ACCESS_INFO, BR_STRING_URL_GUIDE_TEXT);
	elm_object_focus_allow_set(m_uri_box_object, EINA_TRUE);
	elm_access_activate_cb_set(m_uri_box_object, __access_item_clicked_cb, this);
	evas_object_event_callback_add(m_uri_box_object, EVAS_CALLBACK_KEY_DOWN, __uri_box_key_down_cb, this);

	// Star icon.
	m_star_icon = elm_button_add(m_text_layout);
	RET_MSG_IF(!m_star_icon, "elm_button_add failed.");

	elm_access_info_set(m_star_icon, ELM_ACCESS_INFO, BR_STRING_BOOKMARKS);
	elm_object_style_set(m_star_icon, "browser/text-layout/star_off");
	evas_object_smart_callback_add(m_star_icon, "clicked", __star_icon_clicked_cb, this);
	elm_object_part_content_set(m_text_layout, "elm.swallow.star-icon", m_star_icon);
	elm_object_focus_allow_set(m_star_icon, EINA_TRUE);

  // Secure button.
  m_secure_icon = elm_button_add(m_text_layout);
  RET_MSG_IF(!m_secure_icon, "elm_button_add failed.");

  elm_object_style_set(m_secure_icon, "browser/text-layout/secure_on");
  evas_object_smart_callback_add(m_secure_icon, "clicked", __secure_icon_clicked_cb, this);
  elm_object_part_content_set(m_text_layout, "elm.swallow.secure-icon", m_secure_icon);
  elm_object_focus_allow_set(m_secure_icon, EINA_TRUE);

	// Reload/cancel button.
	m_reload_cancel_icon = elm_button_add(m_text_layout);
	RET_MSG_IF(!m_reload_cancel_icon, "elm_button_add failed.");

	elm_access_info_set(m_reload_cancel_icon, ELM_ACCESS_INFO, BR_STRING_REFRESH);
	elm_object_style_set(m_reload_cancel_icon, "browser/text-layout/reload");
	evas_object_smart_callback_add(m_reload_cancel_icon, "clicked", __reload_cancel_icon_clicked_cb, this);
	elm_object_part_content_set(m_text_layout, "elm.swallow.reload-cancel-icon", m_reload_cancel_icon);
	elm_object_focus_allow_set(m_reload_cancel_icon, EINA_TRUE);
}

#if defined(HW_MORE_BACK_KEY)
void url_bar::__back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	more_menu_manager *menu_manager= m_browser->get_browser_view()->get_url_bar()->get_more_menu_manager();
	if (menu_manager->is_show_more_menu())
		menu_manager->hide_more_context_popup();

	// Back should be handled uri-bar because naviframe back callback is not called.
	Evas_Object *naviframe = m_browser->get_browser_view()->get_naviframe();
	elm_naviframe_item_pop(naviframe);
}

void url_bar::__menu_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	url_bar *ub = (url_bar *)data;
	ub->get_more_menu_manager()->show_more_menu();
}
#endif

void url_bar::__text_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("");

	if (m_browser->is_tab_manager_view_exist()) {
		BROWSER_LOGD("tab manager");
		return;
	}

	url_bar *ub = (url_bar *)data;
	elm_object_signal_emit(ub->m_main_layout, "play,touch_sound,signal", "");

	m_browser->get_browser_view()->show_url_input_bar();

	webview *wv = m_browser->get_browser_view()->get_current_webview();
	if (wv && wv->get_text_selection_mode_clear() == EINA_TRUE) {
		BROWSER_LOGD("text selection mode cleared");
		return ;
	}
}

void url_bar::__access_item_clicked_cb(void *data, Evas_Object *obj, Elm_Object_Item *item)
{
	BROWSER_LOGD("");

	if (m_browser->is_tab_manager_view_exist()) {
		BROWSER_LOGD("tab manager");
		return;
	}

	url_bar *ub = (url_bar *)data;
	elm_object_signal_emit(ub->m_main_layout, "play,touch_sound,signal", "");

	m_browser->get_browser_view()->show_url_input_bar();
}

void url_bar::__uri_box_key_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	if (m_browser->is_tab_manager_view_exist()) {
		BROWSER_LOGD("tab manager");
		return;
	}

	Evas_Event_Key_Down *ev = (Evas_Event_Key_Down *)event_info;
	if (!ev)
		return;

	if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
		return;

	if ((!strcmp(ev->keyname, "Return")) || (!strcmp(ev->keyname, "KP_Enter")))
		__access_item_clicked_cb(data, NULL, NULL);
}

void url_bar::__tab_manager_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	if (m_browser->is_tab_manager_view_exist()) {
		BROWSER_LOGD("tab manager");
		return;
	}

	RET_MSG_IF((m_browser->get_browser_view()->get_url_bar()->get_more_menu_manager()->is_show_more_menu()) == EINA_TRUE, "context menu is on displayin");

	m_browser->get_tab_manager_view()->show();
}

Eina_Bool url_bar:: __tab_manager_clicked_idler_cb(void *data)
{
	url_bar *ub = (url_bar *)data;
	browser_view *bv = m_browser->get_browser_view();
	//Wait until keypad is closed completely
	if (bv->is_ime_on()) {
		if (m_browser->is_tts_enabled())
			bv->get_current_webview()->give_focus(EINA_FALSE);
		else
			elm_object_focus_set(bv->get_main_layout(), EINA_FALSE);
		return ECORE_CALLBACK_RENEW;
	}

	ub->_show_tab_manager();

	return ECORE_CALLBACK_CANCEL;
}

void url_bar::_show_tab_manager(void)
{

	// Show tab manager.
	m_browser->get_tab_manager_view()->show();
}

void url_bar::__secure_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	if (m_browser->is_tab_manager_view_exist()) {
		BROWSER_LOGD("tab manager");
		return;
	}

	webview *wv = m_browser->get_browser_view()->get_current_webview();
	RET_MSG_IF(!wv, "current webview is NULL");
	wv->show_certificate_status_popup(EINA_FALSE);
}

void url_bar::__star_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	if (m_browser->is_tab_manager_view_exist()) {
		BROWSER_LOGD("tab manager");
		return;
	}

	url_bar *ub = (url_bar *)data;
	browser_view *bv = m_browser->get_browser_view();
	webview *wv = bv->get_current_webview();
	const char* title = wv->get_title();
	RET_MSG_IF(!wv, "current webview is NULL");

	int bookmark_id = -1;
	if (m_browser->get_bookmark()->is_in_bookmark(wv->get_uri())) {
		if (m_browser->get_bookmark()->delete_by_uri(wv->get_uri())) {
			if (m_browser->is_tts_enabled()) {
				std::string msg = wv->get_title();
				msg = msg + " " + BR_STRING_NOTI_DELETED_BOOKMARK;
				elm_access_say(msg.c_str());
			} else
				bv->show_noti_popup(BR_STRING_BOOKMARK_DELETED);
		} else
			bv->show_noti_popup(BR_STRING_FAILED);
	} else {
		int result = -1;
		if (!title || !strlen(title))
			title = wv->get_uri();
		result = m_browser->get_bookmark()->save_bookmark(title,
									wv->get_uri(), &bookmark_id,
									m_browser->get_bookmark()->get_root_folder_id());

		if (m_browser->get_bookmark()->get_memory_full())
			bv->show_noti_popup(BR_STRING_DISK_FULL);

		if (result > 0) {
			if (m_browser->is_tts_enabled()) {
				std::string msg = wv->get_title();
				msg = msg + " " + BR_STRING_SAVED_TO_BOOKMARK;
				elm_access_say(msg.c_str());
			} else
				bv->show_noti_popup(BR_STRING_SAVED_TO_BOOKMARK);

			Evas_Object *snapshot = m_browser->get_history()->get_snapshot(wv->get_uri());
			if (snapshot) {
				m_browser->get_bookmark()->set_thumbnail(bookmark_id, snapshot);
				evas_object_del(snapshot);
			}
			/* set bookmark favicon from history if there is same URI */
			Evas_Object *favicon = m_browser->get_history()->get_history_favicon(wv->get_uri());
			if (favicon) {
				m_browser->get_bookmark()->set_favicon(bookmark_id, favicon);
				evas_object_del(favicon);
			}
		} else {
			if (!m_browser->get_bookmark()->get_memory_full())
				bv->show_noti_popup(BR_STRING_FAILED);
		}
	}

	ub->update_star_icon();
	ub->update_secure_icon();
}

void url_bar::__reload_cancel_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	if (m_browser->is_tab_manager_view_exist()) {
		BROWSER_LOGD("tab manager");
		return;
	}

	browser_view *bv = m_browser->get_browser_view();
	webview *wv = bv->get_current_webview();
	RET_MSG_IF(!wv, "current webview is NULL");

	if (wv->is_loading())
		wv->load_stop();
	else
		wv->reload();
}

void url_bar::__font_size_changed_cb(keynode_t *keynode, void *data)
{
	BROWSER_LOGD("");

	url_bar *ub = (url_bar *)data;
	webview *wv = m_browser->get_browser_view()->get_current_webview();
	RET_MSG_IF(!wv, "current webview is null");

	const char *title = wv->get_title();
	if (title && strlen(title) > 0)
		ub->set_text(title);
	else
		ub->set_text(wv->get_uri());
}

void url_bar::__access_tab_manager_clicked_cb(void *data, Evas_Object *obj, Elm_Object_Item *item)
{
	BROWSER_LOGD("");

	if (m_browser->is_tab_manager_view_exist()) {
		BROWSER_LOGD("tab manager");
		return;
	}

	RET_MSG_IF((m_browser->get_browser_view()->get_url_bar()->get_more_menu_manager()->is_show_more_menu()) == EINA_TRUE, "context menu is on displayin");

	url_bar *ub = (url_bar *)data;
	// Disable tab manager button.
	ub->enable_tab_manager_button(EINA_FALSE);

	browser_view *bv = m_browser->get_browser_view();
	//Close the keypad and launch the tab manager after the keypad is hidden, to get proper snapshot in tab manager
	bv->get_current_webview()->give_focus(EINA_FALSE);
	ecore_idler_add(__tab_manager_clicked_idler_cb, data);
}
