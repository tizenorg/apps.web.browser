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

#include "main-toolbar.h"

#include "bookmark.h"
#include "bookmark-view.h"
#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "main-view.h"
#include "preference.h"
#include "webview.h"
#include "webview-list.h"
#include "url-bar.h"
#include "more-menu-manager.h"

#define MAIN_TOOLBAR_EDJ browser_edj_dir"/main-toolbar.edj"

main_toolbar::main_toolbar()
	: m_main_layout(NULL)
	, m_toolbar_scroller(NULL)
	, m_backward_button(NULL)
	, m_forward_button(NULL)
	, m_home_button(NULL)
	, m_saved_page_button(NULL)
	, m_bookmark_button(NULL)
	, m_is_fixed_mode(EINA_FALSE)
{
	BROWSER_LOGD("");

	elm_theme_extension_add(NULL, MAIN_TOOLBAR_EDJ);
}

main_toolbar::~main_toolbar(void)
{
	BROWSER_LOGD("");

	elm_theme_extension_del(NULL, MAIN_TOOLBAR_EDJ);
}

void main_toolbar::create_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");

	// Toolbar scroller.
	m_toolbar_scroller = elm_scroller_add(parent);
	RET_MSG_IF(!m_toolbar_scroller, "elm_scroller_add failed.");

	elm_scroller_policy_set(m_toolbar_scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	evas_object_size_hint_weight_set(m_toolbar_scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_toolbar_scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_bounce_set(m_toolbar_scroller, EINA_FALSE, EINA_FALSE);
	elm_scroller_movement_block_set(m_toolbar_scroller, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL);
	elm_scroller_page_relative_set(m_toolbar_scroller, 0, 0.1);

	// Toolbar layout.
	m_main_layout = elm_layout_add(m_toolbar_scroller);
	RET_MSG_IF(!m_main_layout, "elm_layout_add failed.");

	elm_layout_file_set(m_main_layout, MAIN_TOOLBAR_EDJ, "main-toolbar-layout");

	evas_object_size_hint_weight_set(m_main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_main_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_main_layout);

	// Backward button.
	m_backward_button = elm_button_add(m_main_layout);
	RET_MSG_IF(!m_backward_button, "elm_button_add failed.");

	elm_access_info_set(m_backward_button, ELM_ACCESS_INFO, BR_STRING_GOBACKWARD);
	elm_object_style_set(m_backward_button, "browser/main-toolbar/backward");
	elm_object_part_text_set(m_backward_button, "elm.text", BR_STRING_MAIN_TOOLBAR_BACK);
	evas_object_smart_callback_add(m_backward_button, "clicked", __backward_clicked_cb, this);
	elm_object_part_content_set(m_main_layout, "elm.swallow.backward-button", m_backward_button);
	elm_object_focus_custom_chain_append(m_main_layout, m_backward_button, NULL);

	// Forward button.
	m_forward_button = elm_button_add(m_main_layout);
	RET_MSG_IF(!m_forward_button, "elm_button_add failed.");

	elm_access_info_set(m_forward_button, ELM_ACCESS_INFO, BR_STRING_GOFORWARD);
	elm_object_style_set(m_forward_button, "browser/main-toolbar/forward");
	elm_object_part_text_set(m_forward_button, "elm.text", BR_STRING_MAIN_TOOLBAR_FORWARD);
	evas_object_smart_callback_add(m_forward_button, "clicked", __forward_clicked_cb, this);
	elm_object_part_content_set(m_main_layout, "elm.swallow.forward-button", m_forward_button);
	elm_object_focus_custom_chain_append(m_main_layout, m_forward_button, NULL);

	// Home button.
	m_home_button = elm_button_add(m_main_layout);
	RET_MSG_IF(!m_home_button, "elm_button_add failed.");

	elm_access_info_set(m_home_button, ELM_ACCESS_INFO, BR_STRING_HOMEPAGE);
	elm_object_style_set(m_home_button, "browser/main-toolbar/home");
	elm_object_part_text_set(m_home_button, "elm.text", BR_STRING_MAIN_TOOLBAR_HOME);
	evas_object_smart_callback_add(m_home_button, "clicked", __home_clicked_cb, this);
	elm_object_part_content_set(m_main_layout, "elm.swallow.home-button", m_home_button);
	elm_object_focus_custom_chain_append(m_main_layout, m_home_button, NULL);

	// Bookmark button.
	m_bookmark_button = elm_button_add(m_main_layout);
	RET_MSG_IF(!m_bookmark_button, "elm_button_add failed.");

	elm_access_info_set(m_bookmark_button, ELM_ACCESS_INFO, BR_STRING_BOOKMARK);
	elm_object_style_set(m_bookmark_button, "browser/main-toolbar/bookmark");
	elm_object_part_text_set(m_bookmark_button, "elm.text", BR_STRING_MAIN_TOOLBAR_BOOKMARKS);
	evas_object_smart_callback_add(m_bookmark_button, "clicked", __bookmark_clicked_cb, this);
	elm_object_part_content_set(m_main_layout, "elm.swallow.bookmark-button", m_bookmark_button);
	elm_object_focus_custom_chain_append(m_main_layout, m_bookmark_button, NULL);

	elm_object_content_set(m_toolbar_scroller, m_main_layout);

	evas_object_smart_callback_add(m_main_layout, "language,changed", __language_changed_cb, NULL);
}

void main_toolbar::set_scroll_position(int y)
{
	RET_MSG_IF(!has_layout(), "m_main_toolbar has no layout");

	BROWSER_LOGD("%d", y);
	browser_view *bv = m_browser->get_browser_view();
	main_view *mv = m_browser->get_main_view();

	if (bv->is_ime_on() || mv->get_scroll_mode() == CONTENT_VIEW_SCROLL_FIXED) {
		BROWSER_LOGD("ime on || fixed scroll");
		mv->show_toolbar(EINA_FALSE);
		return;
	}
	if (bv->is_show_url_input_bar() == EINA_TRUE) {
		BROWSER_LOGD("In app in app mode, no need to show toolbar in any case");
		mv->show_toolbar(EINA_FALSE);
	} else if (!m_browser->get_full_screen_enable() && !bv->is_show_find_on_page()) {
		mv->show_toolbar(EINA_TRUE);
	}

	// Let webkit know toolbar rect to hide ctxpopup under toolbar.
	webview *wv = bv->get_current_webview();
	if (wv && wv->get_ewk_view()) {
		Evas_Object *toolbar = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(mv->get_main_layout()), "bottom-toolbar");
		RET_MSG_IF(!toolbar, "toolbar is NULL");
		int toolbar_x, toolbar_y, toolbar_w, toolbar_h;
		evas_object_geometry_get(toolbar, &toolbar_x, &toolbar_y, &toolbar_w, &toolbar_h);
		int scroller_y = 0;
		elm_scroller_region_get(mv->get_main_scroller(), NULL, &scroller_y, NULL, NULL);
		Eina_Rectangle rect;
		rect.x = toolbar_x;
		rect.y = toolbar_y + scroller_y;
		rect.w = toolbar_w;
		rect.h = toolbar_h;
		evas_object_smart_callback_call(wv->get_ewk_view(), "browser,toolbar,changed", &rect);
	}

	int x, w, h;
	elm_scroller_region_get(m_toolbar_scroller, &x, NULL, &w, &h);
	elm_scroller_region_show(m_toolbar_scroller, x, y, w, h);

	if (y == 0)
		evas_object_repeat_events_set(m_toolbar_scroller, EINA_TRUE); // Enable webview touch.
	else
		evas_object_repeat_events_set(m_toolbar_scroller, EINA_FALSE); // Disable webview touch.
}

void main_toolbar::enable_forward_button(Eina_Bool enable)
{
	BROWSER_LOGD("");
	elm_object_disabled_set(m_forward_button, !enable);
}

void main_toolbar::enable_backward_button(Eina_Bool enable)
{
	BROWSER_LOGD("");
	elm_object_disabled_set(m_backward_button, !enable);
}

void main_toolbar::set_focus_on_backward_button(void)
{
	elm_object_focus_set(m_backward_button, EINA_TRUE);
}

void main_toolbar::set_focus_on_forward_button(void)
{
	elm_object_focus_set(m_forward_button, EINA_TRUE);
}

void main_toolbar::set_focus_on_home_button(void)
{
	elm_object_focus_set(m_home_button, EINA_TRUE);
}

void main_toolbar::set_landscape_mode(Eina_Bool landscape)
{
	if (m_is_fixed_mode == EINA_TRUE) {
		if (landscape == EINA_TRUE)
			elm_object_signal_emit(m_main_layout, "show,fixed,landscape-mode,signal", "");
		else
			elm_object_signal_emit(m_main_layout, "show,fixed,portrait-mode,signal", "");
	} else {
		if (landscape == EINA_TRUE)
			elm_object_signal_emit(m_main_layout, "show,landscape-mode,signal", "");
		else
			elm_object_signal_emit(m_main_layout, "show,portrait-mode,signal", "");
	}
}

void main_toolbar::enable_fixed_toolbar(Eina_Bool enable)
{
	BROWSER_LOGD("%d", enable);

	if (m_is_fixed_mode == enable)
		return;

	m_is_fixed_mode = enable;

	Eina_Bool is_landscape = m_browser->get_browser_view()->is_landscape();
	set_landscape_mode(is_landscape);
}

void main_toolbar::enable_toolbar_event(Eina_Bool enable)
{
	RET_MSG_IF(!has_layout(), "m_main_toolbar has no layout");

	BROWSER_LOGD("%d", enable);

	if (enable)
		elm_object_signal_emit(m_main_layout, "enable,event,signal", "");
	else
		elm_object_signal_emit(m_main_layout, "disable,event,signal", "");
}

void main_toolbar::__language_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	browser_view *bv = m_browser->get_browser_view();
	main_view *mv = m_browser->get_main_view();

	elm_object_part_text_set(mv->get_main_toolbar()->m_backward_button, "elm.text", BR_STRING_MAIN_TOOLBAR_BACK);
	elm_object_part_text_set(mv->get_main_toolbar()->m_forward_button, "elm.text", BR_STRING_MAIN_TOOLBAR_FORWARD);
	elm_object_part_text_set(mv->get_main_toolbar()->m_home_button, "elm.text", BR_STRING_MAIN_TOOLBAR_HOME);
	elm_object_part_text_set(mv->get_main_toolbar()->m_bookmark_button, "elm.text", BR_STRING_MAIN_TOOLBAR_BOOKMARKS);
	bv->get_url_bar()->language_changed();
}

void main_toolbar::__forward_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	browser_view *bv = m_browser->get_browser_view();
	webview *wv = bv->get_current_webview();
	RET_MSG_IF(!wv, "current webview is NULL");

	wv->forward();
}

void main_toolbar::__backward_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	browser_view *bv = m_browser->get_browser_view();
	webview *wv = bv->get_current_webview();

	if (bv->get_notipopup_check() == EINA_TRUE) {
		bv->set_notipopup_check(EINA_FALSE);
		if (wv && wv->backward_possible())
			wv->backward();
		else
			elm_win_lower(m_window);
	} else if (wv && wv->backward_possible())
		wv->backward();
	else if ((wv && wv->is_user_created()) || (m_browser->get_webview_list()->get_count() <= 1)) {
		if (bv->get_notipopup_check() == EINA_FALSE)
			elm_win_lower(m_window);
		bv->set_notipopup_check(EINA_TRUE);
	} else {
		if (!wv)
			return;

		webview *replace_wv = m_browser->get_webview_list()->delete_webview(wv);
		BROWSER_LOGD("Replace webview: %p", replace_wv);
		if (replace_wv)
			bv->set_current_webview(replace_wv);
	}
}

void main_toolbar::__home_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	browser_view *bv = m_browser->get_browser_view();
	webview *wv = bv->get_current_webview();

	char *homepage_uri = m_preference->get_homepage_uri();

	// Create webview if current webview is NULL.
	if (!wv) {
		BROWSER_LOGD("create webview");
		wv = m_browser->get_webview_list()->create_webview(EINA_TRUE);
		wv->set_request_uri(homepage_uri);
		m_browser->get_browser_view()->set_current_webview(wv);
	}

	wv->load_uri(homepage_uri);

	if (homepage_uri)
		free(homepage_uri);
}

void main_toolbar::__bookmark_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	RET_MSG_IF((m_browser->get_browser_view()->get_url_bar()->get_more_menu_manager()->is_show_more_menu()) == EINA_TRUE, "context menu is on displaying");
	m_browser->get_bookmark_view()->show();
}
