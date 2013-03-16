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

#include "browser-view.h"

#include <Ecore_X.h>
#include <Elementary.h>
#include <string>
#include <ui-gadget.h>
#include <vconf.h>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "find-on-page.h"
#include "uri-bar.h"
#include "uri-input-bar.h"
#include "most-visited.h"
#include "multiwindow-view.h"
#include "preference.h"
#include "reader.h"
#include "webview.h"
#include "webview-list.h"
#if defined(ADD_TO_HOME)
#if 0
extern "C" {
#include <shortcut.h>
}
#define browser_default_app_icon_path browser_data_dir"/template/default_application_icon.png"
#endif
#endif
#if defined(WEBCLIP)
#if 0
#include "scissorbox-view.h"
#endif
#endif

#define browser_view_edj_path browser_edj_dir"/browser-view.edj"

#define FLICK_THRESHOLD	6000
#define JUMP_BUTTON_TIMEOUT	2
#define JUMP_CONTENT_SIZE	4000

#define APP_IN_APP_BUTTON_SIZE	(40 * efl_scale)

static Evas_Object *__create_bg(Evas_Object *win)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(win, NULL);

	Evas_Object *bg = elm_bg_add(win);
	if (!bg)
		return NULL;

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, bg);
	evas_object_show(bg);

	return bg;
}

static Evas_Object *__create_conformant(Evas_Object *win)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(win, NULL);

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
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

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
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *naviframe = elm_naviframe_add(parent);
	evas_object_show(naviframe);

	return naviframe;
}

browser_view::browser_view(void)
:
	m_webview(NULL)
	,m_uri_bar(NULL)
	,m_uri_input_bar(NULL)
	,m_is_ime(EINA_FALSE)
	,m_most_visited(NULL)
	,m_naviframe_item(NULL)
	,m_never_show_checkbox(NULL)
	,m_find_on_page(NULL)
	,m_reader(NULL)
	,m_is_ficked(EINA_FALSE)
	,m_jump_top_timer(NULL)
	,m_jump_bottom_timer(NULL)
	,m_mouse_down_handle(NULL)
	,m_mouse_up_handle(NULL)
	,m_mouse_move_handle(NULL)
	,m_app_in_app_mouse_down(EINA_FALSE)
	,m_app_in_app_sx(0)
	,m_app_in_app_sy(0)
	,m_app_in_app_event_id(0)
{
	BROWSER_LOGD("");
	elm_theme_extension_add(NULL, browser_view_edj_path);

	m_main_layout = _create_layout(m_window);
	if (!m_main_layout)
		BROWSER_LOGE("_create_layout failed");

	UG_INIT_EFL(m_window, UG_OPT_INDICATOR_ENABLE);
}

browser_view::~browser_view(void)
{
	BROWSER_LOGD("");

	m_preference->set_last_visited_uri(get_current_webview()->get_uri());

	if (m_uri_bar)
		delete m_uri_bar;
	if (m_uri_input_bar)
		delete m_uri_input_bar;
	if (m_find_on_page)
		delete m_find_on_page;
	if (m_reader)
		delete m_reader;
	if (m_mouse_down_handle)
		ecore_event_handler_del(m_mouse_down_handle);
	if (m_mouse_up_handle)
		ecore_event_handler_del(m_mouse_up_handle);
	if (m_mouse_move_handle)
		ecore_event_handler_del(m_mouse_move_handle);

	elm_theme_extension_del(NULL, browser_view_edj_path);
}

void browser_view::set_current_webview(webview *wv)
{
	BROWSER_LOGD("");

	if (m_webview == wv)
		return;

	if (wv == NULL) {
		m_webview = wv;
		return;
	}

	if (m_webview)
		m_webview->deactivate();

	m_webview = wv;
	m_webview->activate();

	Evas_Object *ewk_view = m_webview->get_ewk_view();
	Evas_Object *prev_ewk_view = elm_object_part_content_unset(m_main_layout, "elm.swallow.content");
	if (prev_ewk_view)
		evas_object_hide(prev_ewk_view);
	elm_object_part_content_set(m_main_layout, "elm.swallow.content", ewk_view);
	evas_object_show(ewk_view);

	m_uri_bar->set_uri(m_webview->get_title());

	m_uri_bar->show_reader_icon(m_webview->reader_enabled_get());

	if (!m_webview->forward_possible())
		m_uri_bar->disable_forward_button(EINA_TRUE);
	else
		m_uri_bar->disable_forward_button(EINA_FALSE);

	int low_memory = 0;
	vconf_get_int(VCONFKEY_SYSMAN_LOW_MEMORY, &low_memory);
	if (low_memory >= VCONFKEY_SYSMAN_LOW_MEMORY_SOFT_WARNING)
		m_browser->get_webview_list()->clean_up_webviews();
}

void browser_view::launch(const char *uri)
{
	BROWSER_LOGD("uri = [%s]", uri);

	webview *wv = NULL;

	if (m_browser->get_webview_list()->get_count() >= BROWSER_WINDOW_MAX_SIZE)
		wv = m_webview;
	else {
		wv = m_browser->get_webview_list()->create_webview(EINA_TRUE);
		set_current_webview(wv);
	}

	/* FIXME: */
	if (!uri || strlen(uri) == 0) {
		char *default_homepage = m_preference->get_homepage_uri();
		BROWSER_LOGD("default_homepage=[%s]", default_homepage);
		if (default_homepage && strlen(default_homepage)) {
			wv->load_uri(default_homepage);
			free(default_homepage);
		}
	} else
		wv->load_uri(uri);
}

Eina_Bool browser_view::is_top_view(void)
{
	if (m_naviframe_item == elm_naviframe_top_item_get(m_naviframe))
		return EINA_TRUE;

	return EINA_FALSE;
}

void browser_view::disable_webview_event(Eina_Bool disable)
{
	if (disable)
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "disable,webview_event,signal", "");
	else
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "enable,webview_event,signal", "");
}

void browser_view::__mini_back_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	browser_view *bv = (browser_view *)data;
	bv->m_webview->backward();
}

void browser_view::__mini_forward_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	browser_view *bv = (browser_view *)data;
	bv->m_webview->forward();
}

void browser_view::disable_mini_backward_button(Eina_Bool disable)
{
	if (m_browser->get_app_in_app_enable()) {
		Evas_Object *button = elm_object_part_content_get(m_main_layout, "elm.swallow.mini_back_button");
		if (button)
			elm_object_disabled_set(button, disable);
	}
}

void browser_view::disable_mini_forward_button(Eina_Bool disable)
{
	if (m_browser->get_app_in_app_enable()) {
		Evas_Object *button = elm_object_part_content_get(m_main_layout, "elm.swallow.mini_forward_button");
		if (button)
			elm_object_disabled_set(button, disable);
	}
}

void browser_view::show_resize_close_buttons(Eina_Bool show)
{
	if (show) {
		Evas_Object *resize_icon = evas_object_rectangle_add(evas_object_evas_get(m_main_layout));
		if (!resize_icon) {
			BROWSER_LOGE("elm_icon_add failed");
			return;
		}
		evas_object_color_set(resize_icon, 0, 0, 0, 0);
		evas_object_resize(resize_icon, APP_IN_APP_BUTTON_SIZE, APP_IN_APP_BUTTON_SIZE);
		elm_object_part_content_set(m_main_layout, "elm.swallow.resize_button", resize_icon);
		evas_object_event_callback_add(resize_icon, EVAS_CALLBACK_MOUSE_DOWN, __app_in_app_resize_cb, this);

		Evas_Object *close_icon = elm_icon_add(m_main_layout);
		if (!close_icon) {
			BROWSER_LOGE("elm_icon_add failed");
			return;
		}
		if (!elm_icon_file_set(close_icon, browser_img_dir"/L01_mini_mode_icon_close.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.");
			return;
		}
		evas_object_resize(close_icon, APP_IN_APP_BUTTON_SIZE, APP_IN_APP_BUTTON_SIZE);
		elm_object_part_content_set(m_main_layout, "elm.swallow.close_button", close_icon);

		evas_object_event_callback_add(close_icon, EVAS_CALLBACK_MOUSE_UP, __app_in_app_close_cb, this);

		Evas_Object *full_icon = elm_icon_add(m_main_layout);
		if (!full_icon) {
			BROWSER_LOGE("elm_icon_add failed");
			return;
		}
		if (!elm_icon_file_set(full_icon, browser_img_dir"/L01_mini_mode_icon_resize.png", NULL)) {
			BROWSER_LOGE("elm_icon_file_set is failed.");
			return;
		}
		evas_object_resize(full_icon, APP_IN_APP_BUTTON_SIZE, APP_IN_APP_BUTTON_SIZE);
		elm_object_part_content_set(m_main_layout, "elm.swallow.full_button", full_icon);

		evas_object_event_callback_add(full_icon, EVAS_CALLBACK_MOUSE_UP, __app_in_app_full_cb, this);

		Evas_Object *backward_button = elm_button_add(m_main_layout);
		if (!backward_button) {
			BROWSER_LOGE("elm_button_add failed");
			return;
		}
		elm_object_style_set(backward_button, "browser/mini_back");
		elm_object_part_content_set(m_main_layout, "elm.swallow.mini_back_button", backward_button);
		evas_object_smart_callback_add(backward_button, "clicked", __mini_back_clicked_cb, this);

		Evas_Object *forward_button = elm_button_add(m_main_layout);
		if (!forward_button) {
			BROWSER_LOGE("elm_button_add failed");
			return;
		}
		elm_object_style_set(forward_button, "browser/mini_forward");
		elm_object_part_content_set(m_main_layout, "elm.swallow.mini_forward_button", forward_button);
		evas_object_smart_callback_add(forward_button, "clicked", __mini_forward_clicked_cb, this);

		m_mouse_down_handle = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, __app_in_app_mouse_down_cb, this);
		m_mouse_move_handle = ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, __app_in_app_mouse_move_cb, this);
		m_mouse_up_handle = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, __app_in_app_mouse_up_cb, this);

		edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "app_in_app_title_text", m_webview->get_title());

		if (m_webview->forward_possible())
			disable_mini_forward_button(EINA_FALSE);
		else
			disable_mini_forward_button(EINA_TRUE);

		if (m_webview->backward_possible())
			disable_mini_backward_button(EINA_FALSE);
		else
			disable_mini_backward_button(EINA_TRUE);
	} else {
		Evas_Object *button = elm_object_part_content_unset(m_main_layout, "elm.swallow.resize_button");
		if (button)
			evas_object_del(button);
		button = elm_object_part_content_unset(m_main_layout, "elm.swallow.close_button");
		if (button)
			evas_object_del(button);
		button = elm_object_part_content_unset(m_main_layout, "elm.swallow.full_button");
		if (button)
			evas_object_del(button);

		button = elm_object_part_content_unset(m_main_layout, "elm.swallow.mini_back_button");
		if (button)
			evas_object_del(button);

		button = elm_object_part_content_unset(m_main_layout, "elm.swallow.mini_forward_button");
		if (button)
			evas_object_del(button);

		if (m_mouse_down_handle) {
			ecore_event_handler_del(m_mouse_down_handle);
			m_mouse_down_handle = NULL;
		}
		if (m_mouse_up_handle) {
			ecore_event_handler_del(m_mouse_up_handle);
			m_mouse_up_handle = NULL;
		}
		if (m_mouse_move_handle) {
			ecore_event_handler_del(m_mouse_move_handle);
			m_mouse_move_handle = NULL;
		}

		m_app_in_app_sx = m_app_in_app_sy = 0;
		m_app_in_app_mouse_down = EINA_FALSE;
	}
}

void browser_view::set_app_in_app_title(const char *title)
{
	EINA_SAFETY_ON_NULL_RETURN(title);
	edje_object_part_text_set(elm_layout_edje_get(m_main_layout), "app_in_app_title_text", title);
}

void browser_view::__app_in_app_full_cb(void *data, Evas* evas, Evas_Object *obj, void *event_info)
{
	m_browser->set_app_in_app_enable(EINA_FALSE);
}

void browser_view::__app_in_app_close_cb(void *data, Evas* evas, Evas_Object *obj, void *event_info)
{
	browser_view *bv = (browser_view *)data;

	if (bv->m_mouse_down_handle) {
		ecore_event_handler_del(bv->m_mouse_down_handle);
		bv->m_mouse_down_handle = NULL;
	}
	if (bv->m_mouse_up_handle) {
		ecore_event_handler_del(bv->m_mouse_up_handle);
		bv->m_mouse_up_handle = NULL;
	}
	if (bv->m_mouse_move_handle) {
		ecore_event_handler_del(bv->m_mouse_move_handle);
		bv->m_mouse_move_handle = NULL;
	}

	elm_exit();
}

void browser_view::__app_in_app_resize_cb(void *data, Evas* evas, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *)event_info;
	browser_view *bv = (browser_view *)data;

	int x, y;
	ecore_x_pointer_last_xy_get(&x, &y);
	Ecore_X_Window x_window = elm_win_xwindow_get(m_window);
	ecore_x_mouse_up_send(x_window, x, y, ev->button);
	ecore_x_pointer_ungrab();

	int direction = ECORE_X_NETWM_DIRECTION_SIZE_BR;
	ecore_x_netwm_moveresize_request_send(x_window, x, y, (Ecore_X_Netwm_Direction)direction, ev->button);

	bv->m_app_in_app_mouse_down = EINA_FALSE;
	bv->m_app_in_app_sx = bv->m_app_in_app_sy = 0;
}

Eina_Bool browser_view::__app_in_app_mouse_down_cb(void *data, int type, void *event)
{
	Ecore_Event_Mouse_Button *ev = (Ecore_Event_Mouse_Button *)event;
	browser_view *bv = (browser_view *)data;

	if (bv->m_app_in_app_mouse_down)
		return ECORE_CALLBACK_PASS_ON;

	bv->m_app_in_app_mouse_down = EINA_TRUE;
	bv->m_app_in_app_sx = ev->root.x;
	bv->m_app_in_app_sy = ev->root.y;
	bv->m_app_in_app_event_id = ev->buttons;

	BROWSER_LOGD("bv->m_app_in_app_sx=[%d], bv->m_app_in_app_sy=[%d]", bv->m_app_in_app_sx, bv->m_app_in_app_sy);

	if (bv->m_app_in_app_event_id == 0)
		bv->m_app_in_app_event_id = 1;

	return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool browser_view::__app_in_app_mouse_up_cb(void *data, int type, void *event)
{
	BROWSER_LOGD("");
	Ecore_Event_Mouse_Button *ev = (Ecore_Event_Mouse_Button *)event;
	browser_view *bv = (browser_view *)data;

	if (!bv->m_app_in_app_mouse_down)
		return ECORE_CALLBACK_PASS_ON;

	bv->m_app_in_app_mouse_down = EINA_FALSE;
	bv->m_app_in_app_sx = bv->m_app_in_app_sy = 0;

	return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool browser_view::__app_in_app_mouse_move_cb(void *data, int type, void *event)
{
	Ecore_Event_Mouse_Move *ev = (Ecore_Event_Mouse_Move *)event;
	browser_view *bv = (browser_view *)data;

	if (!bv->m_app_in_app_mouse_down)
		return ECORE_CALLBACK_PASS_ON;

	double l = sqrt(pow((float)(bv->m_app_in_app_sx - ev->root.x), 2) + pow((float)(bv->m_app_in_app_sy - ev->root.y), 2));
	if (l >= 30.0f && ev->multi.device == 0) {
		int x, y;
		Ecore_X_Window x_window = elm_win_xwindow_get(m_window);
		ecore_x_pointer_last_xy_get(&x, &y);
		ecore_x_mouse_up_send(x_window, x, y, bv->m_app_in_app_event_id);
		ecore_x_pointer_ungrab();

		// request to move window
		ecore_x_netwm_moveresize_request_send(x_window, x, y, ECORE_X_NETWM_DIRECTION_MOVE, bv->m_app_in_app_event_id);

		bv->m_app_in_app_mouse_down = EINA_FALSE;
		bv->m_app_in_app_sx = bv->m_app_in_app_sy = 0;
	}

	return ECORE_CALLBACK_PASS_ON;
}


void browser_view::_show_most_visited(Eina_Bool show)
{
	BROWSER_LOGD("");
	if (show) {
		if (m_most_visited) {
			BROWSER_LOGE("m_most_visited is already shown");
			return;
		}
		m_most_visited = new most_visited();
		Evas_Object *layout = m_most_visited->get_layout();
		if (!layout) {
			BROWSER_LOGD("most visited sites item is empty");
			delete m_most_visited;
			m_most_visited = NULL;
			return;
		}
		elm_object_part_content_set(m_main_layout, "elm.swallow.most-visited", layout);
	} else {
		elm_object_part_content_unset(m_main_layout, "elm.swallow.most-visited");
		if (m_most_visited) {
			delete m_most_visited;
			m_most_visited = NULL;
		}
	}
}

void browser_view::show_find_on_page(Eina_Bool show, const char *word, webview *wv)
{
	if (show) {
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "show,find-on-page,signal", "");

		if (!get_find_on_page())
			create_find_on_page();

		if (wv)
			get_find_on_page()->set_webview(wv);
		else
			get_find_on_page()->set_webview(get_current_webview());

		if (word) {
			get_find_on_page()->set_text(word);
			show_uri_bar(EINA_FALSE);
			if (is_show_reader())
				get_reader()->show_toolbar(EINA_FALSE);
		} else {
			get_find_on_page()->show_ime();
		}
	} else {
		get_find_on_page()->clear_text();
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "hide,find-on-page,signal", "");

		if (!m_is_ime) {
			show_uri_bar(EINA_TRUE);
			if (is_show_reader())
				get_reader()->show_toolbar(EINA_TRUE);
		}
	}
}

find_on_page *browser_view::create_find_on_page(void)
{
	if (!m_find_on_page) {
		m_find_on_page = new find_on_page();
		Evas_Object *layout = m_find_on_page->get_layout();
		elm_object_part_content_set(m_main_layout, "elm.swallow.find-on-page", layout);
	}

	return m_find_on_page;
}

reader *browser_view::create_reader(void)
{
	if (!m_reader)
		m_reader = new reader();

	return m_reader;
}

Eina_Bool browser_view::is_show_find_on_page(void)
{
	const char* state = edje_object_part_state_get(elm_layout_edje_get(m_main_layout), "elm.swallow.find-on-page", NULL);
	if (state && !strcmp(state, "visible"))
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

void browser_view::show_uri_input_bar(Eina_Bool show)
{
	if (show) {
		if (m_browser->get_app_in_app_enable())
			return;

		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "show,uri-input-bar,signal", "");

		_show_most_visited(EINA_TRUE);

		get_uri_input_bar()->show_ime();
	} else {
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "hide,uri-input-bar,signal", "");

		_show_most_visited(EINA_FALSE);

		if (!m_is_ime)
			show_uri_bar(EINA_TRUE);
	}
}

Eina_Bool browser_view::is_show_uri_input_bar(void)
{
	const char* state = edje_object_part_state_get(elm_layout_edje_get(m_main_layout), "elm.swallow.uri-input-bar", NULL);
	if (state && !strcmp(state, "visible"))
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

void browser_view::show_uri_bar(Eina_Bool show)
{
	if (show) {
		if (m_browser->get_app_in_app_enable())
			return;

		get_uri_bar()->disable_buttons(EINA_FALSE);
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "show,uri-bar,signal", "");
	} else
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "hide,uri-bar,signal", "");

	m_uri_bar->show_app_in_app(!show);
}

Eina_Bool browser_view::is_show_uri_bar(void)
{
	const char* state = edje_object_part_state_get(elm_layout_edje_get(m_main_layout), "elm.swallow.uri-bar", NULL);
	if (state && !strcmp(state, "default"))
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

uri_input_bar *browser_view::get_uri_input_bar(void)
{
	if (!m_uri_input_bar) {
		m_uri_input_bar = new uri_input_bar(m_naviframe);
		Evas_Object *layout = m_uri_input_bar->get_layout();
		elm_object_part_content_set(m_main_layout, "elm.swallow.uri-input-bar", layout);
	}

	return m_uri_input_bar;
}

Eina_Bool browser_view::is_show_reader(void)
{
	const char* state = edje_object_part_state_get(elm_layout_edje_get(m_main_layout), "elm.swallow.reader", NULL);
	if (state && !strcmp(state, "default"))
		return EINA_FALSE;
	else
		return EINA_TRUE;
}

void browser_view::show_reader(Eina_Bool show)
{
	if (show) {
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "show,reader,signal", "");
		if (!get_reader())
			create_reader();
		Evas_Object *layout = get_reader()->get_layout();
		elm_object_part_content_set(m_main_layout, "elm.swallow.reader", layout);

		edje_object_signal_callback_add(elm_layout_edje_get(m_main_layout), "hide,reader,finished", "", __hide_reader_finished_cb, this);

	} else {
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "hide,reader,signal", "");
		}
	}

void browser_view::__hide_reader_finished_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("");
	browser_view *bv = (browser_view *)data;
	elm_object_part_content_unset(bv->m_main_layout, "elm.swallow.reader");
	bv->get_reader()->delete_layout();
}

void browser_view::__prefered_homepage_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	browser_view *bv = (browser_view *)data;

	m_preference->set_user_homagepage(bv->get_current_webview()->get_uri());
	m_preference->set_homepage_type(HOMEPAGE_TYPE_USER_HOMEPAGE);

	if (elm_check_state_get(bv->m_never_show_checkbox))
		m_preference->set_frequent_homepage(EINA_TRUE);
}

void browser_view::__prefered_homepage_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
}

void browser_view::show_prefered_homepage_confirm_popup(void)
{
	if (m_preference->get_frequent_homepage())
		return;

	const char *uri = get_current_webview()->get_uri();
	if (!uri || strlen(uri) == 0)
		return;

	Evas_Object *popup_layout = elm_layout_add(m_window);
	elm_layout_file_set(popup_layout, browser_edj_dir"/browser-popup.edj", "prefered_homepage_popup");
	evas_object_size_hint_weight_set(popup_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *scroller = elm_scroller_add(popup_layout);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

	Evas_Object *label = elm_label_add(scroller);
	elm_label_line_wrap_set(label, ELM_WRAP_CHAR);

	// FIXME: Hard coded string
	char *markup_uri = elm_entry_utf8_to_markup(uri);
	if (!markup_uri)
		return;

	elm_object_text_set(label, BR_STRING_SET_CURRENT_PAGE_AS_HOMEPAGE);

	free(markup_uri);

	m_never_show_checkbox = elm_check_add(scroller);
	elm_check_state_set(m_never_show_checkbox, EINA_TRUE);
	elm_object_part_content_set(popup_layout, "elm.swallow.checkbox", m_never_show_checkbox);

	elm_object_content_set(scroller, label);
	elm_object_part_content_set(popup_layout, "elm.swallow.scroller", scroller);
	edje_object_part_text_set(elm_layout_edje_get(popup_layout), "elm.text.never_show", BR_STRING_NEVER_SHOW_AGAIN);

	show_content_popup(NULL, popup_layout, BR_STRING_OK, __prefered_homepage_ok_cb, BR_STRING_CANCEL, __prefered_homepage_cancel_cb, this);
}

void browser_view::__ime_show_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	browser_view *bv = (browser_view *)data;
	bv->show_uri_bar(EINA_FALSE);
	bv->m_is_ime = EINA_TRUE;
}

void browser_view::__ime_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	browser_view *bv = (browser_view *)data;

	if (!bv->is_show_uri_input_bar() && !bv->is_show_find_on_page())
		bv->show_uri_bar(EINA_TRUE);
	bv->m_is_ime = EINA_FALSE;
}

Evas_Object *browser_view::_create_layout(Evas_Object *window)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(window, NULL);

	m_bg = __create_bg(window);

	Evas_Object *conformant = __create_conformant(window);
	if (!conformant) {
		BROWSER_LOGE("__create_conformant failed");
		return NULL;
	}

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

	Evas_Object *main_layout = elm_layout_add(conformant);
	if (!main_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(main_layout, browser_edj_dir"/browser-view.edj", "webview-layout");
	evas_object_show(main_layout);

	evas_object_smart_callback_add(conformant, "virtualkeypad,state,on", __ime_show_cb, this);
	evas_object_smart_callback_add(conformant, "virtualkeypad,state,off", __ime_hide_cb, this);

	evas_object_smart_callback_add(conformant, "clipboard,state,on", __ime_show_cb, this);
	evas_object_smart_callback_add(conformant, "clipboard,state,off", __ime_hide_cb, this);

	m_uri_bar = new uri_bar(m_naviframe);
	Evas_Object *uri_bar = m_uri_bar->get_layout();
	elm_object_part_content_set(main_layout, "elm.swallow.uri-bar", uri_bar);

	m_naviframe_item = elm_naviframe_item_push(m_naviframe, NULL, NULL, NULL, main_layout, "empty");
	elm_object_item_signal_emit(m_naviframe_item, "elm,state,toolbar,close", "");

	evas_object_smart_callback_add(m_naviframe, "transition,finished", __naviframe_pop_cb, this);

	Evas_Object *top_button = elm_button_add(main_layout);
	if (!top_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(top_button, "browser/top_button");
	evas_object_repeat_events_set(top_button, EINA_FALSE);
	evas_object_propagate_events_set(top_button, EINA_FALSE);
	elm_object_part_content_set(main_layout, "elm.swallow.top_button", top_button);
	evas_object_smart_callback_add(top_button, "clicked", __jump_to_top_cb, this);

	Evas_Object *bottom_button = elm_button_add(main_layout);
	if (!bottom_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(bottom_button, "browser/bottom_button");
	evas_object_repeat_events_set(bottom_button, EINA_FALSE);
	evas_object_propagate_events_set(bottom_button, EINA_FALSE);
	elm_object_part_content_set(main_layout, "elm.swallow.bottom_button", bottom_button);
	evas_object_smart_callback_add(bottom_button, "clicked", __jump_to_bottom_cb, this);

	Evas_Object *event_rect = evas_object_rectangle_add(evas_object_evas_get(main_layout));
	if (!event_rect) {
		BROWSER_LOGD("evas_object_rectangle_add failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(event_rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_color_set(event_rect, 0, 0, 0, 0);
	elm_object_part_content_set(main_layout, "elm.swallow.gesture", event_rect);

	_create_gesture_layer(event_rect);

	m_conformant = conformant;

	return main_layout;
}

Evas_Object *browser_view::_create_gesture_layer(Evas_Object *parent)
{
	BROWSER_LOGD("");
	Evas_Object *gesture_layer = elm_gesture_layer_add(parent);
	if (!gesture_layer) {
		BROWSER_LOGD("elm_gesture_layer_add failed");
		return NULL;
	}
	elm_gesture_layer_attach(gesture_layer, parent);

	elm_gesture_layer_cb_set(gesture_layer, ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_START, __gesture_momentum_start, this);
	elm_gesture_layer_cb_set(gesture_layer, ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_MOVE, __gesture_momentum_move, this);

	return gesture_layer;
}

Evas_Event_Flags browser_view::__gesture_momentum_start(void *data, void *event_info)
{
	browser_view *bv = (browser_view *)data;

	Elm_Gesture_Momentum_Info *momentum_info = (Elm_Gesture_Momentum_Info *)event_info;
	bv->m_is_ficked = EINA_FALSE;

	return EVAS_EVENT_FLAG_NONE;
}

Evas_Event_Flags browser_view::__gesture_momentum_move(void *data, void *event_info)
{
	browser_view *bv = (browser_view *)data;
	Elm_Gesture_Momentum_Info *momentum_info = (Elm_Gesture_Momentum_Info *)event_info;

	if (momentum_info->my > FLICK_THRESHOLD && !bv->m_is_ficked) {
		BROWSER_LOGD("momentum_info->my=%d, bv->m_is_ficked=%d", momentum_info->my, bv->m_is_ficked);
		bv->m_is_ficked = EINA_TRUE;
		bv->show_jump_to_top_button(EINA_TRUE);
	} else if (momentum_info->my < (-1) * (FLICK_THRESHOLD) && !bv->m_is_ficked) {
		BROWSER_LOGD("momentum_info->my=%d, bv->m_is_ficked=%d", momentum_info->my, bv->m_is_ficked);

		bv->m_is_ficked = EINA_TRUE;
		bv->show_jump_to_bottom_button(EINA_TRUE);
	}

	return EVAS_EVENT_FLAG_NONE;
}

Eina_Bool browser_view::is_show_jump_to_top_button(void)
{
	const char* state = edje_object_part_state_get(elm_layout_edje_get(m_main_layout), "elm.swallow.top_button", NULL);
	if (state && !strcmp(state, "visible"))
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

Eina_Bool browser_view::is_show_jump_to_bottom_button(void)
{
	const char* state = edje_object_part_state_get(elm_layout_edje_get(m_main_layout), "elm.swallow.bottom_button", NULL);
	if (state && !strcmp(state, "visible"))
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

Eina_Bool browser_view::__jump_top_timer_cb(void *data)
{
	BROWSER_LOGD("");
	browser_view *bv = (browser_view *)data;
	bv->m_jump_top_timer = NULL;
	bv->show_jump_to_top_button(EINA_FALSE);

	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool browser_view::__jump_bottom_timer_cb(void *data)
{
	BROWSER_LOGD("");
	browser_view *bv = (browser_view *)data;
	bv->m_jump_bottom_timer = NULL;
	bv->show_jump_to_bottom_button(EINA_FALSE);

	return ECORE_CALLBACK_CANCEL;
}

void browser_view::show_jump_to_top_button(Eina_Bool show)
{
	if (show) {
		int h = 0;

		m_webview->scroll_size_get(NULL, &h);
		if (h < JUMP_CONTENT_SIZE)
			return;

		int y = 0;
		m_webview->scroll_position_get(NULL, &y);
		if (y == 0)
			return;

		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "show,top-button,signal", "");
		if (m_jump_top_timer)
			ecore_timer_del(m_jump_top_timer);
		m_jump_top_timer = ecore_timer_add(JUMP_BUTTON_TIMEOUT, __jump_top_timer_cb, this);
	}
	else
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "hide,top-button,signal", "");
}

void browser_view::show_jump_to_bottom_button(Eina_Bool show)
{
	if (show) {
		int h = 0;

		m_webview->scroll_size_get(NULL, &h);
		if (h < JUMP_CONTENT_SIZE)
			return;

		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "show,bottom-button,signal", "");
		if (m_jump_bottom_timer)
			ecore_timer_del(m_jump_bottom_timer);
		m_jump_bottom_timer = ecore_timer_add(JUMP_BUTTON_TIMEOUT, __jump_bottom_timer_cb, this);
	}
	else
		edje_object_signal_emit(elm_layout_edje_get(m_main_layout), "hide,bottom-button,signal", "");
}

void browser_view::__jump_to_top_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	browser_view *bv = (browser_view *)data;
	bv->_jump_to_top_bottom(EINA_TRUE);
	edje_object_signal_emit(elm_layout_edje_get(bv->m_main_layout), "hide,top-button,signal", "");
}

void browser_view::__jump_to_bottom_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	browser_view *bv = (browser_view *)data;
	bv->_jump_to_top_bottom(EINA_FALSE);
	edje_object_signal_emit(elm_layout_edje_get(bv->m_main_layout), "hide,bottom-button,signal", "");
}

void browser_view::_jump_to_top_bottom(Eina_Bool top)
{
	BROWSER_LOGD("");

	int w = 0;
	int h = 0;
	int x = 0;
	int y = 0;

	m_webview->scroll_size_get(&w, &h);
	m_webview->scroll_position_get(&x, &y);

	BROWSER_LOGD("h=%d, y=%d", h, y);

	if (top) {
		m_webview->scroll_position_set(x, 0);
	} else {
		m_webview->scroll_position_set(x, h + y);
	}
}

void browser_view::__naviframe_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	browser_view *bv = (browser_view *)data;
	if (bv->m_naviframe_item != elm_naviframe_top_item_get(m_naviframe))
		return;

	m_browser->delete_setting_view();
	m_browser->delete_history_view_view();
	m_browser->delete_bookmark_add_view();
	m_browser->delete_bookmark_edit_view();
	m_browser->delete_scrap_view();
#if defined(BROWSER_TAG)
	m_browser->delete_add_tag_view();
#endif
}

void browser_view::__back_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	browser_view *bv = (browser_view *)data;

	if (bv->m_webview->backward_possible())
		bv->m_webview->backward();
	else if (!bv->m_webview->is_user_created()) {
		webview *replace_wv = m_browser->get_webview_list()->delete_webview(bv->m_webview);
		if (replace_wv)
			bv->set_current_webview(replace_wv);
	} else
		elm_win_lower(m_window);
}
#if defined(ADD_TO_HOME)
#if 0
void browser_view::show_add_to_home_popup()
{
	Evas_Object *content_list = elm_list_add(m_window);
	if (!content_list) {
		BROWSER_LOGE("elm_list_add failed");
	}
	elm_list_mode_set(content_list, ELM_LIST_EXPAND);
	elm_list_item_append(content_list, BR_STRING_SHORTCUT, NULL, NULL, __shortcut_cb, this);
#if defined(WEBCLIP)
	elm_list_item_append(content_list, BR_STRING_WEB_CLIPPING, NULL, NULL, __webclip_cb, this);
#endif

	show_content_popup(BR_STRING_ADD_TO_HOME, content_list, BR_STRING_CANCEL, NULL, NULL, NULL, NULL);
}

void browser_view::__shortcut_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	browser_view *bv = (browser_view *)data;
	webview *wv = bv->m_webview;

	if (wv->is_loading() == EINA_TRUE) {
		bv->show_msg_popup(BR_STRING_LOADING_PLZ_WAIT, 2);
	} else {
		int add_to_home_result = 0;
		add_to_home_result = add_to_home_shortcut(sec_browser_app, wv->get_title(),
												LAUNCH_BY_URI, wv->get_uri(),
												browser_default_app_icon_path, NULL, NULL);
		if (add_to_home_result == 0) {
			bv->show_msg_popup(BR_STRING_SHORTCUT_CREATED, 2);
		} else
			bv->show_msg_popup(BR_STRING_FAILED, 2);
	}
	bv->destroy_popup(obj);
}

#if defined(WEBCLIP)
void browser_view::__webclip_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	browser_view *bv = (browser_view *)data;
	bv->m_browser->get_scissorbox_view()->show();
	bv->destroy_popup(obj);
}
#endif
#endif
#endif
