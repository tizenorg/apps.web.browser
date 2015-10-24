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

#include "main-view.h"

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "more-menu-manager.h"
#include "url-bar.h"
#include "url-input-bar.h"
#include "platform-service.h"
#include "preference.h"

#define TOOLBAR_ANIMATOR_DELTA_Y (ELM_SCALE_SIZE(8))
#define FLICK_THRESHOLD	3000

#define SCALE_SIZE(x) (x * elm_config_scale_get() / elm_app_base_scale_get())
#define LANDSCAPE_TOOLBAR_HEIGHT (SCALE_SIZE(66))
#define PORTRAIT_TOOLBAR_HEIGHT (SCALE_SIZE(66))

main_view::main_view(void)
	: m_main_layout(NULL)
	, m_scroller(NULL)
	, m_main_toolbar(NULL)
	, m_is_touched(false)
	, m_scroll_lock(EINA_TRUE)
	, m_url_bar_animator(NULL)
	, m_last_touch_y(0)
	, m_toolbar_height(PORTRAIT_TOOLBAR_HEIGHT)
	, m_scroll_mode(CONTENT_VIEW_SCROLL_ENABLE)
	, m_is_flicked(EINA_TRUE)
{
	BROWSER_LOGD("");

	m_main_toolbar = new main_toolbar();
}

main_view::~main_view(void)
{
	BROWSER_LOGD("");

	if (m_url_bar_animator)
		ecore_animator_del(m_url_bar_animator);

	if (m_main_layout)
		evas_object_del(m_main_layout);

	delete m_main_toolbar;
}

void main_view::create_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");

	// Main view layout.
	m_main_layout = elm_layout_add(parent);
	RET_MSG_IF(!m_main_layout, "elm_layout_add failed.");

	elm_layout_file_set(m_main_layout, browser_edj_dir"/main-view.edj", "main-layout");
	evas_object_size_hint_weight_set(m_main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_event_callback_add(m_main_layout, EVAS_CALLBACK_MOUSE_DOWN, __mouse_down_cb, this);
	evas_object_event_callback_add(m_main_layout, EVAS_CALLBACK_MOUSE_UP, __mouse_up_cb, this);
	evas_object_event_callback_add(m_main_layout, EVAS_CALLBACK_MULTI_DOWN, __multi_down_cb, this);
	evas_object_event_callback_add(m_main_layout, EVAS_CALLBACK_MULTI_UP, __multi_up_cb, this);
	evas_object_event_callback_add(m_main_layout, EVAS_CALLBACK_MOUSE_UP, __delayed_toolbar_creation_cb, this);

	// Main view scroller.
	m_scroller = elm_scroller_add(m_main_layout);
	RET_MSG_IF(!m_scroller, "elm_scroller_add failed.");

	elm_scroller_policy_set(m_scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	evas_object_size_hint_weight_set(m_scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_bounce_set(m_scroller, EINA_FALSE, EINA_FALSE);
	elm_object_scroll_freeze_push(m_scroller);

	set_scroll_mode(CONTENT_VIEW_SCROLL_ENABLE);

	// Scroller callbacks.
	evas_object_event_callback_add(m_scroller, EVAS_CALLBACK_RESIZE, __scroller_resize_cb, this);

	evas_object_smart_callback_add(m_scroller, "edge,top", __scroller_edge_top_cb, this);
	evas_object_smart_callback_add(m_scroller, "edge,bottom", __scroller_edge_bottom_cb, this);

	elm_object_part_content_set(m_main_layout, "elm.swallow.scroller", m_scroller);
}

void main_view::deregister_callbacks(void)
{
	BROWSER_LOGD("");
	evas_object_event_callback_del(m_scroller, EVAS_CALLBACK_RESIZE, __scroller_resize_cb);
	evas_object_smart_callback_del(m_scroller, "edge,top", __scroller_edge_top_cb);
	evas_object_smart_callback_del(m_scroller, "edge,bottom", __scroller_edge_bottom_cb);
}

void main_view::register_callbacks(void)
{
	BROWSER_LOGD("");
	deregister_callbacks();
	evas_object_event_callback_add(m_scroller, EVAS_CALLBACK_RESIZE, __scroller_resize_cb, this);
	evas_object_smart_callback_add(m_scroller, "edge,top", __scroller_edge_top_cb, this);
	evas_object_smart_callback_add(m_scroller, "edge,bottom", __scroller_edge_bottom_cb, this);
}

void main_view::set_scroll_content(Evas_Object *scroll_content)
{
	RET_MSG_IF(!m_scroller, "m_scroller is NULL");
	RET_MSG_IF(!scroll_content, "scroll_content is NULL");

	evas_object_size_hint_weight_set(scroll_content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroll_content, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_content_set(m_scroller, scroll_content);
}

void main_view::resize_scroll_content()
{
	BROWSER_LOGD("%d", m_scroll_mode);

	browser_view *bv = m_browser->get_browser_view();
	RET_MSG_IF(!bv, "browser view is NULL");

	int w = 0, h = 0;
	evas_object_geometry_get(m_scroller, NULL, NULL, &w, &h);
	BROWSER_LOGD("----- w, h: %d, %d -----", w, h);

	platform_service ps;
	ps.clip_obj_size_to_clipboard(m_browser->get_main_view()->get_main_scroller(), &w, &h);

	int toolbar_h = 0;
	webview *wv = bv->get_current_webview();

	switch (m_scroll_mode) {
		case CONTENT_VIEW_SCROLL_ENABLE:
			// Content size: Fullscreen
			if (bv->is_landscape())
				toolbar_h = LANDSCAPE_TOOLBAR_HEIGHT;
			else
				toolbar_h = PORTRAIT_TOOLBAR_HEIGHT;

			bv->resize_main_layout(w, h + toolbar_h);
			elm_object_signal_emit(m_main_layout, "set,fullscreen-content,signal", "");

			// Keep url bar status after change scroller size.
			if (wv)
				show_url_bar(wv->get_url_bar_shown());

			break;
		case CONTENT_VIEW_SCROLL_FIXED:
			// Content size: Fullscreen - URL bar
			bv->resize_main_layout(w, h);
			elm_object_signal_emit(m_main_layout, "set,fullscreen-content,signal", "");

			show_toolbar(EINA_FALSE);
			break;
		default:
			// Content size: Fullscreen - URL bar - Toolbar
			bv->resize_main_layout(0, 0);
			elm_object_signal_emit(m_main_layout, "set,resized-content,signal", "");
			break;
	}

	ecore_timer_add(0.1, __set_toolbar_scroll_position_cb, this);
}

void main_view::show_url_bar_animator(Eina_Bool show)
{
	BROWSER_LOGD("%d", show);
	RET_MSG_IF(m_url_bar_animator, "url bar animator is already exist.");
	RET_MSG_IF(!m_browser->get_browser_view()->get_url_bar(), "url bar is NULL");
	RET_MSG_IF(m_browser->get_browser_view()->get_url_bar()->get_more_menu_manager()->is_show_more_menu(), "more menu is on");

	if (show && m_browser->get_browser_view()->is_ime_on()) {
		BROWSER_LOGD("IME is on, do not show URL bar when input field is focused");
		return;
	}

	// Return if scroll is not enabled. In case of app in app, fullscreen, mini mode, etc.
	if (m_scroll_mode != CONTENT_VIEW_SCROLL_ENABLE)
	{
		BROWSER_LOGD("scroll is NOT enabled");
		return;
	}

	if (show)
		m_url_bar_animator = ecore_animator_add(__show_url_bar_animator_cb, this);
	else
		m_url_bar_animator = ecore_animator_add(__hide_url_bar_animator_cb, this);

	// Disable toolbar event.
	get_main_toolbar()->enable_toolbar_event(EINA_FALSE);
}

void main_view::show_toolbar(Eina_Bool show)
{
	BROWSER_LOGD("------- %d -------", show);

	if (show)
		elm_object_signal_emit(m_main_layout, "show,main-toolbar,signal", "");
	else
		elm_object_signal_emit(m_main_layout, "hide,main-toolbar,signal", "");
}

void main_view::set_landscape_mode(Eina_Bool landscape)
{
	BROWSER_LOGD("%d", landscape);

	if (landscape) {
		m_toolbar_height = LANDSCAPE_TOOLBAR_HEIGHT;
		elm_object_signal_emit(m_main_layout, "show,landscape-mode,signal", "");
	} else {
		m_toolbar_height = PORTRAIT_TOOLBAR_HEIGHT;
		elm_object_signal_emit(m_main_layout, "show,portrait-mode,signal", "");
	}

	m_main_toolbar->set_landscape_mode(landscape);
}

void main_view::set_scroll_mode(main_view_scroll_mode mode)
{
	BROWSER_LOGD("------- %d -------", mode);

	m_scroll_mode = mode;

	// Remove toolbar scroller in case of keyboard is attached.
	if (m_scroll_mode == CONTENT_VIEW_SCROLL_FIXED_TOOLBAR && m_browser->is_keyboard_active())
		m_main_toolbar->enable_fixed_toolbar(EINA_TRUE);
	else
		m_main_toolbar->enable_fixed_toolbar(EINA_FALSE);
}

void main_view::set_toolbar_scroller()
{
	// Replace layout with scroller.
	elm_object_part_content_unset(m_main_layout, "elm.swallow.toolbar-scroller");
	elm_object_part_content_set(m_main_layout, "elm.swallow.toolbar-scroller", m_main_toolbar->get_scroller());
}

int main_view::get_scroll_position(void)
{
	int y = 0;
	elm_scroller_region_get(m_scroller, NULL, &y, NULL, NULL);

	return y;
}

void main_view::set_scroll_position(int new_y)
{
	BROWSER_LOGD("%d", new_y);

	int x, w, h;
	elm_scroller_region_get(m_scroller, &x, NULL, &w, &h);
	elm_scroller_region_show(m_scroller, x, new_y, w, h);
}

void main_view::set_scroll_position_delta(int delta)
{
	BROWSER_LOGD("%d", delta);

	webview *wv = m_browser->get_browser_view()->get_current_webview();

	// Do not handling delta without touch event.
	if (!m_is_touched) {
		if (wv != NULL && wv->get_ewk_view() != NULL) {
			BROWSER_LOGD("***** remain delta: %d", delta);
			evas_object_smart_callback_call(wv->get_ewk_view(), "urlbar,scroll,remain", &delta);
		}
		return;
	}

	int y = 0;
	elm_scroller_region_get(m_scroller, NULL, &y, NULL, NULL);

	int new_y = y + delta;
	set_scroll_position(new_y);

	int scroller_y = new_y;
	if (scroller_y < 0)
		scroller_y = 0;
	else if (scroller_y > m_toolbar_height)
		scroller_y = m_toolbar_height;
	m_main_toolbar->set_scroll_position(m_toolbar_height - scroller_y);

	if (wv != NULL && wv->get_ewk_view() != NULL) {
		if (new_y < 0) {
			BROWSER_LOGD("***** remain: %d", new_y);
			evas_object_smart_callback_call(wv->get_ewk_view(), "urlbar,scroll,remain", &new_y);

			if (delta == new_y)
				wv->set_url_bar_shown(EINA_TRUE);
		} else if (new_y > m_toolbar_height) {
			new_y -= m_toolbar_height;
			BROWSER_LOGD("***** remain: %d", new_y);
			evas_object_smart_callback_call(wv->get_ewk_view(), "urlbar,scroll,remain", &new_y);

			if (delta == new_y)
				wv->set_url_bar_shown(EINA_FALSE);
		}
	}
}

void main_view::set_scroll_lock(Eina_Bool lock)
{
	BROWSER_LOGD("------- %d -------", lock);

	m_scroll_lock = lock;
}

void main_view::show_url_bar(Eina_Bool show)
{
	BROWSER_LOGD("%d", show);

	if (show)
		set_scroll_position(0);
	else
		set_scroll_position(PORTRAIT_TOOLBAR_HEIGHT);
}

Eina_Bool main_view::is_show_url_bar(void)
{
	// Check if url bar is completely shown.
	if (get_scroll_position() == 0)
		return EINA_TRUE;

	return EINA_FALSE;
}

Eina_Bool main_view::is_hide_url_bar(void)
{
	// Check if url bar is completely hidden.
	if (get_scroll_position() == m_toolbar_height)
		return EINA_TRUE;

	return EINA_FALSE;
}

void main_view::_set_toolbar_scroll_position(void)
{
	if (m_scroll_mode != CONTENT_VIEW_SCROLL_ENABLE) {
		// Show toolbar.
		BROWSER_LOGD("scroll fixed");
		m_main_toolbar->set_scroll_position(m_toolbar_height);
		return;
	}
	int scroller_y = 0;
	elm_scroller_region_get(m_scroller, NULL, &scroller_y, NULL, NULL);
	m_main_toolbar->set_scroll_position(m_toolbar_height - scroller_y);
}

Eina_Bool main_view::__resize_scroll_content_cb(void *data)
{
	BROWSER_LOGD("");

	main_view *mv = (main_view *)data;
	mv->resize_scroll_content();

	return ECORE_CALLBACK_CANCEL;
}

void main_view::__mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	main_view *mv = (main_view *)data;
	mv->m_is_touched = true;

	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *)event_info;
	mv->_handle_mouse_down(ev->output.y);
}

void main_view::__mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	main_view *mv = (main_view *)data;
	mv->m_is_touched = false;

	Evas_Event_Mouse_Up *ev = (Evas_Event_Mouse_Up *)event_info;
	mv->_handle_mouse_up(ev->output.y);
}

void main_view::__multi_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	__mouse_down_cb(data, evas, obj, event_info);
}

void main_view::__multi_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	__mouse_up_cb(data, evas, obj, event_info);
}

void main_view::_handle_mouse_down(int touch_y)
{
	BROWSER_LOGD("");

	m_last_touch_y = touch_y;
	set_is_flicked(EINA_TRUE);

	webview *wv = m_browser->get_browser_view()->get_current_webview();
	if (wv && wv->get_ewk_view()) {
		if (!evas_object_focus_get(wv->get_ewk_view())) {
			BROWSER_LOGD("focus set");
			m_browser->get_browser_view()->set_focus_on_main_layout();
		}
	}
}

void main_view::_handle_mouse_up(int touch_y)
{
	BROWSER_LOGD("");

	// Return if scroll is not enabled. In case of app in app, fullscreen, mini mode, etc.
	if (m_scroll_mode != CONTENT_VIEW_SCROLL_ENABLE)
		return;

	int x, y, w, h;
	elm_scroller_region_get(m_scroller, &x, &y, &w, &h);

	if (m_scroll_lock) {
		if (touch_y - m_last_touch_y > h / 4) {
			BROWSER_LOGD("---------- SHOW URL BAR (lock) -----------");
			show_url_bar_animator(EINA_TRUE);
			return;
		} else if (m_last_touch_y - touch_y > h / 4) {
			BROWSER_LOGD("----------- HIDE URL BAR (lock) ----------");
			browser_view *bv = m_browser->get_browser_view();
			if(!bv->is_show_find_on_page())
				show_url_bar_animator(EINA_FALSE);
			return;
		}
	}
	set_scroll_lock(EINA_TRUE);

	ecore_idler_add(__move_url_bar_cb, this);
}

Eina_Bool main_view::__move_url_bar_cb(void *data)
{
	BROWSER_LOGD("");

	main_view *mv = (main_view *)data;
	int y;
	elm_scroller_region_get(mv->m_scroller, NULL, &y, NULL, NULL);

	if (y != 0 && y != mv->m_toolbar_height) {
		webview *wv = m_browser->get_browser_view()->get_current_webview();
		if (wv != NULL) {
			if (mv->m_is_flicked == EINA_TRUE) {
				// Move url bar to webview scroll direction.
				if (wv->get_url_bar_shown() == EINA_TRUE) {
					BROWSER_LOGD("----------- HIDE URL BAR (scroll) ----------");
					mv->show_url_bar_animator(EINA_FALSE);
				} else {
					BROWSER_LOGD("---------- SHOW URL BAR (scroll) -----------");
					mv->show_url_bar_animator(EINA_TRUE);
				}
			} else {
				// Move url bar nearest post(completely show or hide).
				if (mv->get_scroll_position() > mv->m_toolbar_height / 2) {
					BROWSER_LOGD("----------- HIDE URL BAR ----------");
					mv->show_url_bar_animator(EINA_FALSE);
				} else {
					BROWSER_LOGD("---------- SHOW URL BAR -----------");
					mv->show_url_bar_animator(EINA_TRUE);
				}
			}
		}
	}

	return ECORE_CALLBACK_CANCEL;
}

void main_view::__scroller_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	main_view *mv = (main_view *)data;
	mv->resize_scroll_content();

	browser_view *bv = m_browser->get_browser_view();
	if (bv && bv->is_show_url_input_bar())
		bv->get_url_input_bar()->resize_search_ctxpopup();
}

void main_view::__scroller_edge_top_cb(void *data, Evas_Object *obj, void *event_info)
{
	browser_view *bv = m_browser->get_browser_view();
	bv->get_url_bar()->focus_allow_set(EINA_TRUE);

	webview *wv = bv->get_current_webview();
	if (wv != NULL)
		wv->set_url_bar_shown(EINA_TRUE);
}

void main_view::__scroller_edge_bottom_cb(void *data, Evas_Object *obj, void *event_info)
{
	browser_view *bv = m_browser->get_browser_view();
	bv->get_url_bar()->focus_allow_set(EINA_FALSE);

	webview *wv = bv->get_current_webview();
	if (wv != NULL)
		wv->set_url_bar_shown(EINA_FALSE);
}

Eina_Bool main_view::__show_url_bar_animator_cb(void *data)
{
	BROWSER_LOGD("");

	main_view *mv = (main_view *)data;

	int x, y, w, h;
	elm_scroller_region_get(mv->m_scroller, &x, &y, &w, &h);
	if (y == 0 || mv->m_is_touched || mv->m_scroll_mode != CONTENT_VIEW_SCROLL_ENABLE) {
		mv->m_url_bar_animator = NULL;
		BROWSER_LOGD("end of animator");
		mv->get_main_toolbar()->enable_toolbar_event(EINA_TRUE);
		mv->resize_scroll_content();
		return ECORE_CALLBACK_CANCEL;
	}

	int new_y = y - TOOLBAR_ANIMATOR_DELTA_Y;
	if (new_y < 0)
		new_y = 0;

	elm_scroller_region_show(mv->m_scroller, x, new_y, w, h);
	mv->get_main_toolbar()->set_scroll_position(mv->m_toolbar_height - new_y);

	return ECORE_CALLBACK_RENEW;
}

Eina_Bool main_view::__hide_url_bar_animator_cb(void *data)
{
	BROWSER_LOGD("");

	main_view *mv = (main_view *)data;

	int x, y, w, h;
	elm_scroller_region_get(mv->m_scroller, &x, &y, &w, &h);
	if (y == mv->m_toolbar_height || mv->m_is_touched || mv->m_scroll_mode != CONTENT_VIEW_SCROLL_ENABLE) {
		mv->m_url_bar_animator = NULL;
		BROWSER_LOGD("end of animator");
		mv->get_main_toolbar()->enable_toolbar_event(EINA_TRUE);
		mv->resize_scroll_content();
		return ECORE_CALLBACK_CANCEL;
	}
	int new_y = y + TOOLBAR_ANIMATOR_DELTA_Y;
	if (new_y > mv->m_toolbar_height)
		new_y = mv->m_toolbar_height;

	elm_scroller_region_show(mv->m_scroller, x, new_y, w, h);
	mv->get_main_toolbar()->set_scroll_position(mv->m_toolbar_height - new_y);
	webview * wv = m_browser->get_browser_view()->get_current_webview();
	if (wv)
		wv->give_focus(EINA_TRUE);
	return ECORE_CALLBACK_RENEW;
}

Eina_Bool main_view::__set_toolbar_scroll_position_cb(void *data)
{
	BROWSER_LOGD("");

	main_view *mv = (main_view *)data;
	mv->_set_toolbar_scroll_position();

	return ECORE_CALLBACK_CANCEL;
}

void main_view::__delayed_toolbar_creation_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	main_view *mv = (main_view *)data;

	mv->m_main_toolbar->create_layout(mv->m_main_layout);
	mv->set_toolbar_scroller();

	ecore_idler_add(__set_toolbar_scroll_position_cb, mv);
	evas_object_event_callback_del(mv->m_main_layout, EVAS_CALLBACK_MOUSE_UP, __delayed_toolbar_creation_cb);

	webview *wv = m_browser->get_browser_view()->get_current_webview();
	if(!wv || !m_browser->get_main_view()->get_main_toolbar())
		return;

	m_browser->get_main_view()->get_main_toolbar()->enable_backward_button(wv->backward_possible());
	m_browser->get_main_view()->get_main_toolbar()->enable_forward_button(wv->forward_possible());
}
