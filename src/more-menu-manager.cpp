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
 * Contact: Jiwon Lee <jiwonear.lee@samsung.com>
 *
 */

#include "more-menu-manager.h"

#include <Elementary.h>
#include <app_control.h>
#include <efl_extension.h>
#include <fcntl.h>
#include <string>
#include <utilX.h>
#include <vconf.h>

#include "browser.h"
#include "bookmark-view.h"
#include "bookmark-add-view.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "history.h"
#include "history-view.h"
#include "main-view.h"
#include "platform-service.h"
#include "preference.h"
#include "progress-bar.h"
#include "settings.h"
#include "settings-basic.h"
#include "webview.h"
#include "webview-list.h"
#include "url-bar.h"

#define MORE_MENU_MANAGER_EDJ browser_edj_dir"/more-menu-manager.edj"

#define PRINT_PDF_W	210
#define PRINT_PDF_H	297

more_menu_manager::more_menu_manager(void)
	: m_more_context_popup(NULL)
	, m_is_dismissed_by_back_key(EINA_FALSE)
	, m_is_minimize_popup_shown(EINA_FALSE)
	, m_private_mode_allow_ask_popup(NULL)
	, m_private_mode_allow_ask_check(NULL)
{

}

more_menu_manager::~more_menu_manager(void)
{
	BROWSER_LOGD("");
}

Eina_Bool more_menu_manager::__resize_more_ctxpopup_cb(void *data)
{
	RETV_MSG_IF(!data, ECORE_CALLBACK_CANCEL, "data is NULL");

	more_menu_manager *cp = (more_menu_manager *)data;

	if (!cp->m_more_context_popup)
		return ECORE_CALLBACK_CANCEL;

	BROWSER_LOGD("");

	// Move ctxpopup to proper position.
	int x = 0;
	int y = 0;
	platform_service ps;
	ps.get_more_ctxpopup_position(&x, &y);

	evas_object_move(cp->m_more_context_popup, x, y);

	return ECORE_CALLBACK_CANCEL;
}

void more_menu_manager::__rotate_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	__resize_more_ctxpopup_cb(data);
}

void more_menu_manager::hide_more_context_popup(void)
{
	BROWSER_LOGD("");

	if (m_more_context_popup) {
		evas_object_smart_callback_del(elm_object_top_widget_get(m_more_context_popup), "rotation,changed", __rotate_ctxpopup_cb);
		evas_object_del(m_more_context_popup);
		m_more_context_popup = NULL;
	}
}

void more_menu_manager::__context_popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	RET_MSG_IF(!data, "data is NULL");
	more_menu_manager *cp = (more_menu_manager *)data;
	cp->hide_more_context_popup();

	cp->m_is_dismissed_by_back_key = EINA_FALSE;
	//Give the highlight to the webview after the popup dismissed and TTS will say "Internet"(same behaviour as Android)

	if (m_browser->get_browser_view()->get_current_webview())
		elm_access_highlight_set(elm_access_object_get(m_browser->get_browser_view()->get_current_webview()->get_ewk_view()));

	elm_access_say(dgettext(BROWSER_DOMAIN_NAME, "IDS_HELP_BODY_INTERNET"));
}

void more_menu_manager::__context_popup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	more_menu_manager *cp = (more_menu_manager *)data;
	cp->m_is_dismissed_by_back_key = EINA_TRUE;

	eext_ctxpopup_back_cb(data, obj, event_info);
}

Evas_Object *more_menu_manager::_create_more_context_popup()
{
	Evas_Object *more_popup = elm_ctxpopup_add(m_window);
	evas_object_smart_callback_add(elm_object_top_widget_get(more_popup), "rotation,changed", __rotate_ctxpopup_cb, this);
	RETV_MSG_IF(more_popup == NULL, NULL, "Failed to add ctx popup");

	elm_object_style_set(more_popup, "more/default");
	//evas_object_data_set(more_popup, "memu_button", parent);
	evas_object_size_hint_weight_set(more_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(more_popup, "dismissed", __context_popup_dismissed_cb, this);
	elm_ctxpopup_auto_hide_disabled_set(more_popup, EINA_TRUE);
	elm_ctxpopup_direction_priority_set(more_popup, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);

#if defined(HW_MORE_BACK_KEY)
	// Add HW key callback.
	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_BACK, __context_popup_back_cb, this);
	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_MORE, __context_popup_back_cb, this);
#endif

	return more_popup;
}

void more_menu_manager::__new_window_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	if (m_browser->get_webview_list()->get_count() >= BROWSER_WINDOW_MAX_SIZE) {
		m_browser->get_browser_view()->show_max_window_limit_reached_popup();
		__context_popup_dismissed_cb(data, obj, event_info);
		return;
	}
	webview *new_wv = NULL;
		char *default_homepage = m_preference->get_homepage_uri();

		new_wv = m_browser->get_webview_list()->create_renewed_webview(EINA_TRUE);
		m_browser->get_browser_view()->set_current_webview(new_wv);

		if (default_homepage && strlen(default_homepage))
			new_wv->load_uri(default_homepage);

		if (default_homepage)
			free(default_homepage);

	__context_popup_dismissed_cb(data, obj, event_info);
}
#ifdef ENABLE_INCOGNITO_WINDOW
void more_menu_manager::__new_incognito_window_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	if (m_browser->get_webview_list()->get_count() >= BROWSER_WINDOW_MAX_SIZE) {
		m_browser->get_browser_view()->show_max_window_limit_reached_popup();
		__context_popup_dismissed_cb(data, obj, event_info);
		return;
	}
	webview *new_wv = NULL;
	char *default_homepage = m_preference->get_homepage_uri();

	new_wv = m_browser->get_webview_list()->create_webview(EINA_TRUE, EINA_TRUE);
	m_browser->get_browser_view()->set_current_webview(new_wv);

	if (default_homepage && strlen(default_homepage))
		new_wv->load_uri(default_homepage);

	free(default_homepage);

	__context_popup_dismissed_cb(data, obj, event_info);
}
#endif

void more_menu_manager::__settings_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	more_menu_manager* cp = (more_menu_manager*)data;
	cp->m_is_minimize_popup_shown = EINA_FALSE;

	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("Fail to create app_control handle");
		return;
	}

	if (app_control_set_app_id(app_control, "setting-accessibility-efl") < 0) {
		BROWSER_LOGE("Fail to app_control_set_app_id");
		app_control_destroy(app_control);
		return;
	}

	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return ;
	}

	app_control_destroy(app_control);
}

void more_menu_manager::__cancel_popup_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	more_menu_manager* cp = (more_menu_manager*)data;
	cp->m_is_minimize_popup_shown = EINA_FALSE;
}

void more_menu_manager::__find_on_page_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	__context_popup_dismissed_cb(data, obj, event_info);

	m_browser->get_browser_view()->show_find_on_page(NULL, NULL, EINA_TRUE);
}

void more_menu_manager::__private_mode_allow_ask_popup_language_changed(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	elm_object_text_set(obj, _("IDS_BR_POP_PAGES_THAT_YOU_VIEW_WILL_NOT_APPEAR_IN_YOUR_BROWSER_HISTORY_OR_SEARCH_HISTORY_AND_THEY_WILL_NOT_LEAVE_OTHER_TRACES_LIKE_COOKIES"));
}

void more_menu_manager::__private_mode_allow_no_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	more_menu_manager *cp = (more_menu_manager *)data;
	cp->m_private_mode_allow_ask_popup = NULL;
}

void more_menu_manager::set_private_mode(Eina_Bool enable, Eina_Bool update_history)
{
	m_preference->set_incognito_mode_enabled(enable);

	browser_view *bv = m_browser->get_browser_view();
	webview *wv = bv->get_current_webview();

	// Set url bar private mode.
	/* It'll in preference.cpp */
//	bv->get_url_bar()->enable_private_mode(enable);

	if (update_history) {
		if (m_preference->get_incognito_mode_enabled() == EINA_TRUE)
			m_browser->get_history()->delete_history(wv->get_uri());
		else {
			int visit_count = 0;
			Evas_Object *favicon = wv->get_favicon();
			m_browser->get_history()->save_history(wv->get_title(), wv->get_uri(), NULL, favicon, &visit_count);
			if (m_browser->get_history()->get_memory_full())
				bv->show_noti_popup(BR_STRING_DISK_FULL);
			SAFE_FREE_OBJ(favicon);
		}
	}

	m_browser->get_webview_list()->unsync_webviews();
}

void more_menu_manager::__private_mode_allow_allow_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	more_menu_manager *cp = (more_menu_manager *)data;

	Evas_Object *popup = cp->m_private_mode_allow_ask_popup;
	RET_MSG_IF(!popup, "popup is NULL");

	Evas_Object *popup_layout = elm_object_content_get(popup);
	Evas_Object *never_ask_check = elm_object_part_content_get(popup_layout, "elm.swallow.end");

	Eina_Bool checkbox_state = elm_check_state_get(never_ask_check);
	BROWSER_LOGD("checkbox_state[%d]", checkbox_state);

	if (checkbox_state == EINA_TRUE)
		m_preference->set_incognito_mode_ask_again_enabled(EINA_FALSE);

	cp->set_private_mode(!(m_preference->get_incognito_mode_enabled()), EINA_TRUE);
	cp->m_private_mode_allow_ask_popup = NULL;
}

void more_menu_manager::_private_mode_allow_ask_popup(void)
{
	BROWSER_LOGD("");

	if (m_preference->get_incognito_mode_ask_again_enabled() == EINA_FALSE) {
		BROWSER_LOGD("already asked and user gives do not ask again");
		return;
	}

	if (m_private_mode_allow_ask_popup) {
		m_browser->get_browser_view()->hide_common_view_popups();
		m_private_mode_allow_ask_popup = NULL;
	}

	m_private_mode_allow_ask_popup = brui_popup_add(m_window);
	RET_MSG_IF(!m_private_mode_allow_ask_popup, "m_private_mode_allow_ask_popup is null");

	/* Popup layout */
	Evas_Object *popup_layout = elm_layout_add(m_private_mode_allow_ask_popup);
	elm_layout_file_set(popup_layout, browser_edj_dir"/browser-popup-lite.edj", "popup_checkview_layout");
	evas_object_size_hint_weight_set(popup_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/* check */
	Evas_Object *check = elm_check_add(m_private_mode_allow_ask_popup);
	elm_object_style_set(check, "popup");
	elm_object_domain_translatable_part_text_set(popup_layout, "elm.text.popup", BROWSER_DOMAIN_NAME, "IDS_BR_POP_DO_NOT_SHOW_AGAIN");
	evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(popup_layout, "elm.swallow.end", check);

	/* scroller */
	Evas_Object *scroller = elm_scroller_add(popup_layout);
	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_object_part_content_set(popup_layout, "elm.swallow.content", scroller);

	/* label */
	Evas_Object *label = elm_label_add(scroller);
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_domain_translatable_text_set(label, BROWSER_DOMAIN_NAME,
        		BR_STRING_ENABLE_SECRET_MODE_POPUP_MSG);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(label);
	elm_object_content_set(scroller, label);
	elm_object_content_set(m_private_mode_allow_ask_popup, popup_layout);

	m_browser->get_browser_view()->show_content_popup(m_private_mode_allow_ask_popup, "IDS_BR_OPT_ENABLE_SECRET_MODE_ABB",
									popup_layout,
									__private_mode_allow_no_cb,
									"IDS_BR_SK_OK", __private_mode_allow_allow_cb,
									NULL, NULL,
									this);
}

void more_menu_manager::__private_on_off_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	more_menu_manager *cp = (more_menu_manager *)data;
	if (m_preference->get_incognito_mode_enabled() == EINA_FALSE && m_preference->get_incognito_mode_ask_again_enabled() == EINA_TRUE)
		cp->_private_mode_allow_ask_popup();
	else
		cp->set_private_mode(!(m_preference->get_incognito_mode_enabled()), EINA_TRUE);

	__context_popup_dismissed_cb(data, obj, event_info);
}

void more_menu_manager::__setting_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	__context_popup_dismissed_cb(data, obj, event_info);

	m_browser->get_settings()->show();
}

Eina_Bool more_menu_manager::_append_normal_mode_more_context_popup(Evas_Object *more_popup)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(more_popup == NULL, EINA_FALSE, "more_popup is NULL");

	webview *wv = m_browser->get_browser_view()->get_current_webview();
	RETV_MSG_IF(wv == NULL, EINA_FALSE, "current wv is NULL");

	Elm_Object_Item *it = NULL;
	const char *current_uri = wv->get_uri();

	// New window
	brui_ctxpopup_item_append(more_popup, BR_STRING_NEW_WINDOW,
			__new_window_cb, MORE_MENU_MANAGER_EDJ,
			"I01_more_popup_icon_new_window.png", this);

#ifdef ENABLE_INCOGNITO_WINDOW
	// New incognito window
	brui_ctxpopup_item_append(more_popup, BR_STRING_NEW_INCOGNITO_WINDOW,
			__new_incognito_window_cb, MORE_MENU_MANAGER_EDJ,
			"I01_more_popup_icon_new_incognito_window.png", this);
#endif
	// find on page
	it = brui_ctxpopup_item_append(more_popup, BR_STRING_FIND_ON_PAGE,
						__find_on_page_cb, MORE_MENU_MANAGER_EDJ,
						"I01_more_popup_icon_find_on_page.png", this);

	if (!current_uri || strlen(current_uri) == 0 || m_browser->get_browser_view()->get_url_bar()->is_loading_status() == EINA_TRUE)
		elm_object_item_disabled_set(it, EINA_TRUE);

#ifndef ENABLE_INCOGNITO_WINDOW
	// private mode
	#if 0
	if (m_preference->get_incognito_mode_enabled() == EINA_TRUE)
		brui_ctxpopup_item_append(more_popup, BR_STRING_PRIVATE_OFF,
						__private_on_off_cb, MORE_MENU_MANAGER_EDJ,
						"I01_more_popup_icon_public_view.png", this);
	else
		brui_ctxpopup_item_append(more_popup, BR_STRING_PRIVATE_ON,
						__private_on_off_cb, MORE_MENU_MANAGER_EDJ,
						"I01_more_popup_icon_private_view.png", this);
	#endif
#endif
	// settings
	brui_ctxpopup_item_append(more_popup, BR_STRING_SETTINGS,
						__setting_cb, MORE_MENU_MANAGER_EDJ,
						"I01_more_popup_icon_setting.png", this);

	return EINA_TRUE;
}

Eina_Bool more_menu_manager::_append_empty_page_mode_more_context_popup(Evas_Object *more_popup)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(more_popup == NULL, EINA_FALSE, "more_popup is NULL");

	brui_ctxpopup_item_append(more_popup, BR_STRING_NEW_WINDOW,
						__new_window_cb, MORE_MENU_MANAGER_EDJ,
						"I01_more_popup_icon_new_window.png", this);

#ifdef ENABLE_INCOGNITO_WINDOW
	brui_ctxpopup_item_append(more_popup, BR_STRING_NEW_INCOGNITO_WINDOW,
			__new_incognito_window_cb, MORE_MENU_MANAGER_EDJ,
			"I01_more_popup_icon_new_incognito_window.png", this);
#endif

	return EINA_TRUE;
}

Eina_Bool more_menu_manager::_append_more_context_popup(Evas_Object *more_popup, uri_bar_more_menu_type type)
{
	RETV_MSG_IF(more_popup == NULL, EINA_FALSE, "more_popup is empty");

	Eina_Bool retv = EINA_FALSE;

	switch(type) {
		case more_menu_normal:
			retv = _append_normal_mode_more_context_popup(more_popup);
		break;

		case more_menu_epmty_webview:
			retv = _append_empty_page_mode_more_context_popup(more_popup);
		break;

		default:
			BROWSER_LOGD("Failed to recognize to append ctx popup items");
		break;
	}

	return retv;
}

void more_menu_manager::_show_more_context_popup()
{
	BROWSER_LOGD("");

	hide_more_context_popup();

	RET_MSG_IF(m_browser->get_browser_view()->is_show_find_on_page() == EINA_TRUE, "It's on find on page mode");
	RET_MSG_IF(m_browser->get_full_screen_enable() == EINA_TRUE, "It's on full screen mode");
	RET_MSG_IF(m_browser->get_browser_view()->is_top_view() == EINA_FALSE, "naviframe for browser is not on top");
	RET_MSG_IF(m_browser->is_tab_manager_view_exist()== EINA_TRUE, "tabmanager view is on displaying");

	Evas_Object *more_popup = _create_more_context_popup();
	RET_MSG_IF(more_popup == NULL, "Failed to _create_more_context_popup");

	uri_bar_more_menu_type type = more_menu_unknown;

	if (m_browser->get_webview_list()->get_count() == 0)
		type = more_menu_epmty_webview;
	else
		type = more_menu_normal;
	if (_append_more_context_popup(more_popup, type) == EINA_FALSE) {
		BROWSER_LOGE("Failed to append ctx popup");
		evas_object_smart_callback_del(elm_object_top_widget_get(more_popup), "rotation,changed", __rotate_ctxpopup_cb);
		evas_object_del(more_popup);
		more_popup = NULL;
		return;
	}

	m_more_context_popup = more_popup;

	__resize_more_ctxpopup_cb(this);

	evas_object_show(more_popup);
}

void more_menu_manager::show_more_menu(void)
{
	BROWSER_LOGD("");

	_show_more_context_popup();
}

Eina_Bool more_menu_manager::is_show_more_menu(void)
{
	return m_more_context_popup ? EINA_TRUE : EINA_FALSE;
}

void more_menu_manager::enable_browser_view_focus(Eina_Bool enable)
{
	if (enable)
		m_browser->navi_frame_tree_focus_allow_set(EINA_TRUE);
	else if (m_browser->is_tts_enabled())
		m_browser->navi_frame_tree_focus_allow_set(EINA_FALSE);
}
