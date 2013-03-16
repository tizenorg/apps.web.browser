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

#include "multiwindow-view.h"

#include <Elementary.h>

#include "bookmark-view.h"
#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "multiwindow-item.h"
#include "preference.h"
#include "webview.h"
#include "webview-list.h"

#define ZOOM_THRESHOLD	0.3
#define MOMENTUM_THRESHOLD	800

#define PANES_EFFECT_RATE	0.07
#define SHRINK_EFFECT_DURATION	0.2

#define EXPAND_ITEM_BOUNDARY	0.6
#define SHRINK_ITEM_BOUNDARY	0.5

#define CLOSE_MULTIWINDOW_BOUNDARY	0.2

#define MULTIWINDOW_MIN_BOUNDARY	0.4
#define MULTIWINDOW_MAX_BOUNDARY	0.8

#define multiwindow_item_key	"multiwindow_item"
#define bookmark_item_key	"bookmark_item"

static void _context_popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	evas_object_del(obj);
}

multiwindow_view::multiwindow_view(Eina_Bool init_bookmark)
:
	m_gesture_layer(NULL)
	,m_main_layout(NULL)
	,m_panes(NULL)
	,m_scroller(NULL)
	,m_scroll_tilting(EINA_FALSE)
	,m_scroll_spring(EINA_FALSE)
	,m_selected_index(0)
	,m_open_effect_animator(NULL)
	,m_close_effect_animator(NULL)
	,m_max_effect_animator(NULL)
	,m_min_effect_animator(NULL)
	,m_is_draging(EINA_FALSE)
	,m_prev_size(1.0)
	,m_container_layout(NULL)
	,m_is_multiwindow_showing(!init_bookmark)
	,m_toolbar_layout(NULL)
	,m_is_zooming(EINA_FALSE)
	,m_plus_button(NULL)
	,m_never_show_checkbox(NULL)
	,m_move_transit(NULL)
{
	BROWSER_LOGD("");

	elm_theme_extension_add(NULL, multiwindow_view_edj_path);
	elm_theme_extension_add(NULL, multiwindow_panes_edj_path);

	int rate = m_preference->get_multiwindow_handle_rate();
	m_handle_rate = (float)((float)rate / (float)10);
	m_main_layout = _create_main_layout(m_window);

	if (!is_multiwindow_showing())
		change_view(EINA_FALSE);
}

multiwindow_view::~multiwindow_view(void)
{
	BROWSER_LOGD("");
	elm_object_part_content_unset(m_main_layout, "elm.swallow.toolbar");
	if (m_toolbar_layout)
		evas_object_del(m_toolbar_layout);

	if (m_open_effect_animator) {
		ecore_animator_del(m_open_effect_animator);
		m_open_effect_animator = NULL;
	}

	if (m_close_effect_animator) {
		ecore_animator_del(m_close_effect_animator);
		m_close_effect_animator = NULL;
	}

	if (m_min_effect_animator) {
		ecore_animator_del(m_min_effect_animator);
		m_min_effect_animator = NULL;
	}
	if (m_max_effect_animator) {
		ecore_animator_del(m_max_effect_animator);
		m_max_effect_animator = NULL;
	}

	if (m_move_transit) {
		elm_transit_del(m_move_transit);
		m_move_transit = NULL;
	}

	if (m_gesture_layer)
		evas_object_del(m_gesture_layer);

	for (int i = 0 ; i < m_multiwindow_item_list.size() ; i++)
		delete m_multiwindow_item_list[i];

	m_multiwindow_item_list.clear();

	for (int i = 0 ; i < m_padding_layout_list.size() ; i++)
		evas_object_del(m_padding_layout_list[i]);

	if (m_main_layout)
		evas_object_del(m_main_layout);

	elm_theme_extension_del(NULL, multiwindow_view_edj_path);
	elm_theme_extension_del(NULL, multiwindow_panes_edj_path);
}

void multiwindow_view::show(void)
{
	elm_win_resize_object_add(m_window, m_main_layout);
	evas_object_show(m_main_layout);

	_show_open_effect();
}

Eina_Bool multiwindow_view::__open_effect_animator_cb(void *data)
{
	multiwindow_view *mv = (multiwindow_view *)data;
	Evas_Object *panes = mv->m_panes;

	if (!mv->m_open_effect_animator)
		return ECORE_CALLBACK_CANCEL;

	for (int i = 0 ; i < mv->m_padding_layout_list.size() ; i++)
		elm_layout_sizing_eval(mv->m_padding_layout_list[i]);

	for (int i = 0 ; i < mv->m_multiwindow_item_list.size(); i++)
		elm_layout_sizing_eval(mv->m_multiwindow_item_list[i]->get_layout());


	double size = elm_panes_content_right_size_get(panes);
	if (size >= mv->m_handle_rate)
		elm_panes_content_right_size_set(panes, mv->m_handle_rate);

	if (size + PANES_EFFECT_RATE > mv->m_handle_rate)
		size = mv->m_handle_rate - PANES_EFFECT_RATE;

	elm_panes_content_right_size_set(panes, (double)(size + PANES_EFFECT_RATE));

	return ECORE_CALLBACK_RENEW;
}

void multiwindow_view::_show_open_effect(void)
{
	BROWSER_LOGD("");
	m_open_effect_animator = ecore_animator_add(__open_effect_animator_cb, this);
}

Eina_Bool multiwindow_view::__close_effect_animator_cb(void *data)
{
	multiwindow_view *mv = (multiwindow_view *)data;
	Evas_Object *panes = mv->m_panes;

	double size = elm_panes_content_right_size_get(panes);
	if (size <= 0.05) {
//		elm_panes_content_right_size_set(panes, 0.0);
		mv->m_close_effect_animator = NULL;

		m_browser->delete_multiwindow_view();

		return ECORE_CALLBACK_CANCEL;
	}

	if (size - PANES_EFFECT_RATE < 0.0)
		size = PANES_EFFECT_RATE;

	elm_panes_content_right_size_set(panes, (double)(size - PANES_EFFECT_RATE));

	return ECORE_CALLBACK_RENEW;
}

void multiwindow_view::_show_min_effect(void)
{
	BROWSER_LOGD("");
	m_min_effect_animator = ecore_animator_add(__min_effect_animator_cb, this);
}

Eina_Bool multiwindow_view::__min_effect_animator_cb(void *data)
{
	multiwindow_view *mv = (multiwindow_view *)data;
	Evas_Object *panes = mv->m_panes;

	double size = elm_panes_content_right_size_get(panes);
	if (size <= MULTIWINDOW_MIN_BOUNDARY) {
		elm_panes_content_right_size_set(panes, 0.4);
		mv->m_min_effect_animator = NULL;

		return ECORE_CALLBACK_CANCEL;
	}

	if (size - PANES_EFFECT_RATE < 0.0)
		size = PANES_EFFECT_RATE;

	elm_panes_content_right_size_set(panes, (double)(size - PANES_EFFECT_RATE));

	return ECORE_CALLBACK_RENEW;
}


Eina_Bool multiwindow_view::__max_effect_animator_cb(void *data)
{
	multiwindow_view *mv = (multiwindow_view *)data;
	Evas_Object *panes = mv->m_panes;

	double size = elm_panes_content_right_size_get(panes);
	if (size > 0.95f) {
		elm_panes_content_right_size_set(panes, 1.0);
		mv->m_max_effect_animator = NULL;

		return ECORE_CALLBACK_CANCEL;
	}

	if (size + PANES_EFFECT_RATE > 1.0f)
		size = 1.0f - PANES_EFFECT_RATE;

	elm_panes_content_right_size_set(panes, (double)(size + PANES_EFFECT_RATE));

	return ECORE_CALLBACK_RENEW;
}

void multiwindow_view::_show_max_effect(void)
{
	BROWSER_LOGD("");
	m_max_effect_animator = ecore_animator_add(__max_effect_animator_cb, this);
}

void multiwindow_view::_show_close_effect(void)
{
	m_close_effect_animator = ecore_animator_add(__close_effect_animator_cb, this);
}

Evas_Object *multiwindow_view::_create_toolbar_layout(Evas_Object *parent)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
	Evas_Object *toolbar_layout = elm_layout_add(parent);
	if (!toolbar_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(toolbar_layout, multiwindow_view_edj_path, "toolbar-layout");
	evas_object_size_hint_weight_set(toolbar_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *more_button = elm_button_add(toolbar_layout);
	if (!more_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(more_button, "browser/toolbar_menu");
	elm_object_part_content_set(toolbar_layout, "elm.swallow.more_button", more_button);
	evas_object_smart_callback_add(more_button, "clicked", __more_cb, this);

	Evas_Object *plus_button = elm_button_add(toolbar_layout);
	if (!plus_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(plus_button, "naviframe/toolbar/default");
	elm_object_text_set(plus_button, BR_STRING_NEW_WINDOW);
	elm_object_part_content_set(toolbar_layout, "elm.swallow.plus_button", plus_button);
	evas_object_smart_callback_add(plus_button, "clicked", __plus_cb, this);

	if (m_browser->get_webview_list()->get_count() >= BROWSER_WINDOW_MAX_SIZE)
		elm_object_disabled_set(plus_button, EINA_TRUE);

	m_plus_button = plus_button;

	Evas_Object *back_button = elm_button_add(toolbar_layout);
	if (!back_button) {
		BROWSER_LOGE("elm_button_add failed");
		return NULL;
	}
	elm_object_style_set(back_button, "naviframe/back_btn/default");
	elm_object_part_content_set(toolbar_layout, "elm.swallow.back_button", back_button);
	evas_object_smart_callback_add(back_button, "clicked", __back_cb, this);

	return toolbar_layout;
}

static Eina_Bool __animator_cb(void *data)
{
	std::vector<multiwindow_item *> list = *((std::vector<multiwindow_item *> *)data);
	for (int i = 0 ; i < list.size() ; i++) {
		elm_layout_sizing_eval(list[i]->get_layout());
	}

	return ECORE_CALLBACK_RENEW;
}

static Eina_Bool __animator_kill_timer_cb(void *data)
{
	ecore_animator_del((Ecore_Animator *)data);

	return ECORE_CALLBACK_CANCEL;
}

void multiwindow_view::__layout_resize_cb(void* data, Evas* evas, Evas_Object* obj, void* ev)
{
	multiwindow_view *mv = (multiwindow_view *)data;
	if (!mv->_is_draging())
		return;

	double size = elm_panes_content_right_size_get(mv->m_panes);

	if (size <= SHRINK_ITEM_BOUNDARY) {
		if (!mv->m_multiwindow_item_list[0]->is_shrink()) {
			ecore_timer_add(SHRINK_EFFECT_DURATION, __animator_kill_timer_cb, ecore_animator_add(__animator_cb, &(mv->m_multiwindow_item_list)));

			for (int i = 0 ; i < mv->m_multiwindow_item_list.size() ; i++)
				mv->m_multiwindow_item_list[i]->shrink_layout(EINA_TRUE);
		}
	} else if (size >= EXPAND_ITEM_BOUNDARY && size < 1.0 && mv->m_prev_size < size) {
		if (mv->m_multiwindow_item_list[0]->is_shrink()) {
			ecore_timer_add(SHRINK_EFFECT_DURATION, __animator_kill_timer_cb, ecore_animator_add(__animator_cb, &(mv->m_multiwindow_item_list)));

			for (int i = 0 ; i < mv->m_multiwindow_item_list.size() ; i++)
				mv->m_multiwindow_item_list[i]->shrink_layout(EINA_FALSE);
		}
	}

	mv->m_prev_size = size;
}

void multiwindow_view::__handle_press_cb(void *data, Evas_Object *obj, void *event_info)
{
	multiwindow_view *mv = (multiwindow_view *)data;
	mv->m_is_draging = EINA_TRUE;
}

void multiwindow_view::__handle_unpress_cb(void *data, Evas_Object *obj, void *event_info)
{
	multiwindow_view *mv = (multiwindow_view *)data;
	mv->m_is_draging = EINA_FALSE;

	double size = elm_panes_content_right_size_get(mv->m_panes);

	if (size > MULTIWINDOW_MAX_BOUNDARY && size < 1.0f) {
		mv->_show_max_effect();
		m_preference->set_multiwindow_handle_rate(10);
		return;
	}

	if (size < EXPAND_ITEM_BOUNDARY && size > MULTIWINDOW_MIN_BOUNDARY) {
		mv->_show_min_effect();
		m_preference->set_multiwindow_handle_rate(4);
		return;
	}

	if (size < CLOSE_MULTIWINDOW_BOUNDARY)
		mv->_show_close_effect();
	else {
		int int_size = (int)(size * 10);
		BROWSER_LOGD("size=[%f], int_size=[%d]", size, int_size);
		m_preference->set_multiwindow_handle_rate(int_size);
	}
}

void multiwindow_view::__bg_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("");
	multiwindow_view *mv = (multiwindow_view *)data;
	mv->_show_close_effect();
}

Evas_Object *multiwindow_view::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *main_layout = elm_layout_add(parent);
	if (!main_layout) {
		BROWSER_LOGD("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(main_layout, multiwindow_view_edj_path, "main-layout");
	evas_object_size_hint_weight_set(main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(main_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	edje_object_signal_callback_add(elm_layout_edje_get(main_layout), "mouse,clicked,1", "bg", __bg_clicked_cb, this);

	Evas_Object *multiwindow_layout = _create_multiwindow_layout(main_layout);

	evas_object_event_callback_add(multiwindow_layout, EVAS_CALLBACK_RESIZE, __layout_resize_cb, this);

	Evas_Object *panes = elm_panes_add(main_layout);
	evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(panes, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_panes_horizontal_set(panes, EINA_TRUE);

	evas_object_smart_callback_add(panes, "press", __handle_press_cb, this);
	evas_object_smart_callback_add(panes, "unpress", __handle_unpress_cb, this);

	Evas_Object *container_layout = elm_layout_add(main_layout);
	if (!container_layout) {
		BROWSER_LOGD("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(container_layout, multiwindow_view_edj_path, "container-layout");
	evas_object_size_hint_weight_set(container_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(container_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_part_content_set(container_layout, "elm.swallow.multiwindow-layout", multiwindow_layout);

	Evas_Object *bookmark_layout = m_browser->get_bookmark_view()->get_layout();

	elm_object_part_content_set(container_layout, "elm.swallow.bookmark-view", bookmark_layout);

	elm_object_part_content_set(panes, "right", container_layout);
	elm_panes_content_right_size_set(panes, 0.0);

	elm_object_style_set(panes, "browser/multiwindow");

	elm_object_part_content_set(main_layout, "elm.swallow.panes", panes);

	m_toolbar_layout = _create_toolbar_layout(main_layout);
	elm_object_part_content_set(main_layout, "elm.swallow.toolbar", m_toolbar_layout);

	m_panes = panes;

	m_container_layout = container_layout;

	return main_layout;
}

void multiwindow_view::_renew_homepage(void)
{
	int count = m_browser->get_webview_list()->get_count();
	int index = 0;
	for (index = 0 ; index < count ; index++) {
		if (m_browser->get_webview_list()->get_webview(index) == m_browser->get_browser_view()->get_current_webview())
			break;
	}

	m_browser->get_browser_view()->get_current_webview()->set_user_created_flag(EINA_TRUE);

	// Caution : When delete_webview() of webview_list, the get_webview_list() is also decreased.
	// So 'i--' is needed.
	for (int i = 0 ; i < m_browser->get_webview_list()->get_count() ; i++) {
		webview *wv = m_browser->get_webview_list()->get_webview(i);
		if (wv != m_browser->get_browser_view()->get_current_webview()) {
			m_browser->get_webview_list()->delete_webview(wv);
			i--;
		}
	}

	BROWSER_LOGD("current index = [%d]", index);

	multiwindow_item *mi = NULL;
	for (int i = 0 ; i < count ; i++) {
		if (i != index)
			delete m_multiwindow_item_list[i];
		else
			mi = m_multiwindow_item_list[i];
	}
	m_multiwindow_item_list.clear();

	if (mi) {
		m_multiwindow_item_list.push_back(mi);
		mi->show_delete_button(EINA_FALSE);
	}

	elm_object_disabled_set(m_plus_button, EINA_FALSE);

	char *default_homepage = m_preference->get_homepage_uri();
	if (default_homepage) {
		m_browser->get_browser_view()->get_current_webview()->load_uri(default_homepage);
		free(default_homepage);
	} else {
		m_browser->get_browser_view()->get_current_webview()->load_uri(blank_page);
	}

	_show_close_effect();
}

void multiwindow_view::__close_all_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	multiwindow_view *mv = (multiwindow_view *)data;

	if (obj)
		evas_object_del(obj);

	if (!m_preference->get_renew_homepage()) {
		mv->_show_renew_homepage_confirm_popup();
		return;
	}

	mv->_renew_homepage();
}

void multiwindow_view::__renew_homepage_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	multiwindow_view *mv = (multiwindow_view *)data;

	if (elm_check_state_get(mv->m_never_show_checkbox))
		m_preference->set_renew_homepage(EINA_TRUE);

	mv->_renew_homepage();
}

void multiwindow_view::_show_renew_homepage_confirm_popup(void)
{
	Evas_Object *popup_layout = elm_layout_add(m_window);
	elm_layout_file_set(popup_layout, browser_edj_dir"/browser-popup.edj", "prefered_homepage_popup");
	evas_object_size_hint_weight_set(popup_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *scroller = elm_scroller_add(popup_layout);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

	Evas_Object *label = elm_label_add(scroller);
	elm_label_line_wrap_set(label, ELM_WRAP_WORD);

	char *markup = elm_entry_utf8_to_markup(BR_STRING_POP_CLOSE_ALL_OPENED_WINDOWS_AND_GO_TO_THE_HOMEPAGE);
	if (!markup)
		return;

	elm_object_text_set(label, markup);
	free(markup);

	m_never_show_checkbox = elm_check_add(scroller);
	elm_check_state_set(m_never_show_checkbox, EINA_TRUE);
	elm_object_part_content_set(popup_layout, "elm.swallow.checkbox", m_never_show_checkbox);

	elm_object_content_set(scroller, label);
	elm_object_part_content_set(popup_layout, "elm.swallow.scroller", scroller);
	edje_object_part_text_set(elm_layout_edje_get(popup_layout), "elm.text.never_show", BR_STRING_NEVER_SHOW_AGAIN);

	show_content_popup(NULL, popup_layout, BR_STRING_OK, __renew_homepage_ok_cb, BR_STRING_CANCEL, NULL, this);
}

void multiwindow_view::_show_more_context_popup(Evas_Object *parent)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(parent);

	Evas_Object *more_popup = elm_ctxpopup_add(m_window);
	if (!more_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return;
	}
	evas_object_size_hint_weight_set(more_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(more_popup, "dismissed", _context_popup_dismissed_cb, NULL);

	elm_ctxpopup_item_append(more_popup, BR_STRING_CLOSE_ALL, NULL, __close_all_cb, this);

	int x, y, w, h;
	evas_object_geometry_get(parent, &x, &y, &w, &h);
	evas_object_move(more_popup, x + (w / 2), y + (h /2));
	evas_object_show(more_popup);
}

Eina_Bool multiwindow_view::_is_effect_running(void)
{
	if (m_open_effect_animator || m_close_effect_animator || m_min_effect_animator || m_max_effect_animator)
		return EINA_TRUE;

	return EINA_FALSE;
}

void multiwindow_view::close_multiwindow_view(void)
{
	if (_is_effect_running())
		return;

	_show_close_effect();
}

void multiwindow_view::__back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	multiwindow_view *mv = (multiwindow_view *)data;

	if (mv->_is_effect_running())
		return;

	mv->_show_close_effect();
}

void multiwindow_view::__more_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	multiwindow_view *mv = (multiwindow_view *)data;

	if (mv->_is_effect_running())
		return;

	mv->_show_more_context_popup(obj);
}

void multiwindow_view::__plus_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");	
	multiwindow_view *mv = (multiwindow_view *)data;

	if (mv->_is_effect_running())
		return;

	webview *new_wv = m_browser->get_webview_list()->create_webview(EINA_TRUE);
	m_browser->get_browser_view()->set_current_webview(new_wv);

	char *default_homepage = m_preference->get_homepage_uri();
	if (default_homepage && strlen(default_homepage)) {
		new_wv->load_uri(default_homepage);
		free(default_homepage);
	}

	mv->_show_close_effect();
}

void multiwindow_view::__item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	multiwindow_item *mi = (multiwindow_item *)event_info;
	multiwindow_view *mv = (multiwindow_view *)data;

	int selected_index = 0;
	for (int i = 0 ; i < mv->m_multiwindow_item_list.size() ; i++) {
		if (mv->m_multiwindow_item_list[i] == mi) {
			selected_index = i;
			break;
		}
	}

	webview *selected_wv = m_browser->get_webview_list()->get_webview(selected_index);
	m_browser->get_browser_view()->set_current_webview(selected_wv);

	mv->_show_close_effect();
}

void multiwindow_view::__transit_move_finished_cb(void *data, Elm_Transit *transit)
{
	BROWSER_LOGD("");
	multiwindow_view *mv = (multiwindow_view *)data;

	delete mv->m_multiwindow_item_list[mv->m_selected_index];
	mv->m_multiwindow_item_list.erase(mv->m_multiwindow_item_list.begin() + mv->m_selected_index);

	if (mv->m_multiwindow_item_list.size() == 1)
		mv->m_multiwindow_item_list[0]->show_delete_button(EINA_FALSE);

	webview *current_wv = m_browser->get_browser_view()->get_current_webview();
	webview *selected_wv = m_browser->get_webview_list()->get_webview(mv->m_selected_index);
	webview *replace_wv = m_browser->get_webview_list()->delete_webview(selected_wv);

	if (selected_wv == current_wv)
		m_browser->get_browser_view()->set_current_webview(replace_wv);

	mv->m_selected_index = 0;
	mv->m_move_transit = NULL;
}

void multiwindow_view::__item_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	multiwindow_item *mi = (multiwindow_item *)event_info;
	multiwindow_view *mv = (multiwindow_view *)data;

	if (mv->m_multiwindow_item_list.size() == 1 || mv->m_move_transit)
		return;

	mv->m_selected_index = 0;
	for (int i = 0 ; i < mv->m_multiwindow_item_list.size() ; i++) {
		if (mv->m_multiwindow_item_list[i] == mi) {
			mv->m_selected_index = i;
			break;
		}
	}

	evas_object_hide(mi->get_layout());

	elm_object_disabled_set(mv->m_plus_button, EINA_FALSE);

	mv->m_move_transit = elm_transit_add();
	elm_transit_event_enabled_set(mv->m_move_transit, EINA_TRUE);

	for (int i = mv->m_selected_index + 1 ; i < mv->m_multiwindow_item_list.size() ; i++)
		elm_transit_object_add(mv->m_move_transit, mv->m_multiwindow_item_list[i]->get_layout());

	elm_transit_tween_mode_set(mv->m_move_transit, ELM_TRANSIT_TWEEN_MODE_SINUSOIDAL);
	elm_transit_objects_final_state_keep_set(mv->m_move_transit, EINA_FALSE);

	int item_h = 0;
	evas_object_geometry_get(mi->get_layout(), NULL, NULL, NULL, &item_h);

	elm_transit_effect_translation_add(mv->m_move_transit, 0, 0, 0, (-1) * item_h);

	elm_transit_duration_set(mv->m_move_transit, 0.1);
	elm_transit_del_cb_set(mv->m_move_transit, __transit_move_finished_cb, mv);
	elm_transit_go(mv->m_move_transit);
}

Evas_Event_Flags multiwindow_view::__gesture_zoom_move(void *data, void *event_info)
{
	multiwindow_view *mv = (multiwindow_view *)data;
	Elm_Gesture_Zoom_Info *zoom_info = (Elm_Gesture_Zoom_Info *)event_info;

	mv->m_scroll_spring = EINA_FALSE;
	mv->m_scroll_tilting = EINA_FALSE;

	ecore_timer_add(SHRINK_EFFECT_DURATION, __animator_kill_timer_cb, ecore_animator_add(__animator_cb, &(mv->m_multiwindow_item_list)));

	if (zoom_info->zoom >= 1.0 + ZOOM_THRESHOLD && mv->m_multiwindow_item_list[0]->is_shrink()) {
		for (int i = 0 ; i < mv->m_multiwindow_item_list.size() ; i++)
			mv->m_multiwindow_item_list[i]->shrink_layout(EINA_FALSE);
			
	} else if (zoom_info->zoom <= 1.0 - ZOOM_THRESHOLD && !(mv->m_multiwindow_item_list[0]->is_shrink())) {
		for (int i = 0 ; i < mv->m_multiwindow_item_list.size() ; i++)
			mv->m_multiwindow_item_list[i]->shrink_layout(EINA_TRUE);
	}

	return EVAS_EVENT_FLAG_NONE;
}

Evas_Event_Flags multiwindow_view::__gesture_zoom_end(void *data, void *event_info)
{
	multiwindow_view *mv = (multiwindow_view *)data;

	mv->m_is_zooming = EINA_FALSE;

	return EVAS_EVENT_FLAG_NONE;
}

Evas_Event_Flags multiwindow_view::__gesture_zoom_start(void *data, void *event_info)
{
	multiwindow_view *mv = (multiwindow_view *)data;

	mv->m_is_zooming = EINA_TRUE;

	return EVAS_EVENT_FLAG_NONE;
}

Evas_Event_Flags multiwindow_view::__gesture_momentum_start(void *data, void *event_info)
{
	multiwindow_view *mv = (multiwindow_view *)data;
	Elm_Gesture_Momentum_Info *momentum_info = (Elm_Gesture_Momentum_Info *)event_info;

	int y, h;
	elm_scroller_region_get(mv->m_scroller, NULL, &y, NULL, &h);

	int ch;
	elm_scroller_child_size_get(mv->m_scroller, NULL, &ch);

	mv->m_scroll_spring = EINA_FALSE;
	mv->m_scroll_tilting = EINA_FALSE;

	if (y == 0)
		mv->m_scroll_tilting = EINA_TRUE;
	if (y + h == ch)
		mv->m_scroll_spring = EINA_TRUE;

	return EVAS_EVENT_FLAG_NONE;
}

void multiwindow_view::change_view(Eina_Bool show_multiwindow_view)
{
	BROWSER_LOGD("show_multiwindow_view=[%d]", show_multiwindow_view);

	m_is_multiwindow_showing = show_multiwindow_view;

	Evas_Object *prev_toolbar = elm_object_part_content_unset(m_main_layout, "elm.swallow.toolbar");
	if (prev_toolbar)
		evas_object_hide(prev_toolbar);

	if (show_multiwindow_view) {
		elm_object_part_content_set(m_main_layout, "elm.swallow.toolbar", m_toolbar_layout);

		edje_object_signal_emit(elm_layout_edje_get(m_container_layout), "show,multiwindow_view,signal", "");
	} else {
		elm_object_part_content_set(m_main_layout, "elm.swallow.toolbar", m_browser->get_bookmark_view()->get_toolbar_layout());

		edje_object_signal_emit(elm_layout_edje_get(m_container_layout), "show,bookmark_view,signal", "");
	}
}

Evas_Event_Flags multiwindow_view::__gesture_momentum_move(void *data, void *event_info)
{
	multiwindow_view *mv = (multiwindow_view *)data;
	Elm_Gesture_Momentum_Info *momentum_info = (Elm_Gesture_Momentum_Info *)event_info;

	if (!mv->m_is_zooming && mv->is_multiwindow_showing() && momentum_info->mx < (-1) * FLICK_MOMENTUM_THRESHOLD) {
		mv->change_view(EINA_FALSE);
		return EVAS_EVENT_FLAG_NONE;
	}

	int y, h;
	elm_scroller_region_get(mv->m_scroller, NULL, &y, NULL, &h);

	int ch;
	elm_scroller_child_size_get(mv->m_scroller, NULL, &ch);

	if (momentum_info->my > MOMENTUM_THRESHOLD && mv->m_scroll_tilting && y == 0) {
		for (int i = 0 ; i < mv->m_multiwindow_item_list.size() ; i++)
			mv->m_multiwindow_item_list[i]->show_tilting_effect();

		mv->m_scroll_tilting = EINA_FALSE;
		mv->m_scroll_spring = EINA_FALSE;
	} else if (momentum_info->my < (-1) * MOMENTUM_THRESHOLD && mv->m_scroll_spring && y + h == ch) {
		for (int i = 0 ; i < mv->m_multiwindow_item_list.size() ; i++)
			mv->m_multiwindow_item_list[i]->show_spring_effect();

		mv->m_scroll_tilting = EINA_FALSE;
		mv->m_scroll_spring = EINA_FALSE;
	}

	return EVAS_EVENT_FLAG_NONE;
}

Evas_Object *multiwindow_view::_create_gesture_layer(Evas_Object *parent)
{
	BROWSER_LOGD("");
	Evas_Object *gesture_layer = elm_gesture_layer_add(parent);
	if (!gesture_layer) {
		BROWSER_LOGD("elm_gesture_layer_add failed");
		return NULL;
	}
	elm_gesture_layer_attach(gesture_layer, parent);
	elm_gesture_layer_cb_set(gesture_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_MOVE, __gesture_zoom_move, this);
	elm_gesture_layer_cb_set(gesture_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_START, __gesture_zoom_start, this);
	elm_gesture_layer_cb_set(gesture_layer, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_END, __gesture_zoom_end, this);

	elm_gesture_layer_cb_set(gesture_layer, ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_START, __gesture_momentum_start, this);
	elm_gesture_layer_cb_set(gesture_layer, ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_MOVE, __gesture_momentum_move, this);

	return gesture_layer;
}

void multiwindow_view::__scroller_edge_top_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	multiwindow_view *mv = (multiwindow_view *)data;
	for (int i = 0 ; i < mv->m_multiwindow_item_list.size() ; i++)
		mv->m_multiwindow_item_list[i]->show_tilting_effect();
}

void multiwindow_view::__scroller_edge_bottom_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	multiwindow_view *mv = (multiwindow_view *)data;
	for (int i = 0 ; i < mv->m_multiwindow_item_list.size() ; i++)
		mv->m_multiwindow_item_list[i]->show_spring_effect();
}

Evas_Object *multiwindow_view::_create_multiwindow_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");	
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGD("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(layout, multiwindow_view_edj_path, "multiwindow-layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *scroller = elm_scroller_add(layout);
	if (!scroller) {
		BROWSER_LOGD("elm_scroller_add failed");
		return NULL;
	}
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	evas_object_show(scroller);

	evas_object_smart_callback_add(scroller, "edge,top", __scroller_edge_top_cb, this);
	evas_object_smart_callback_add(scroller, "edge,bottom", __scroller_edge_bottom_cb, this);

	Evas_Object *box = elm_box_add(layout);
	if (!box) {
		BROWSER_LOGD("elm_box_add failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_horizontal_set(box, EINA_FALSE);
	elm_box_align_set(box, 0.5, 0.0);

	elm_object_content_set(scroller, box);
	evas_object_show(box);

	elm_object_part_content_set(layout, "elm.swallow.scroller", scroller);
	
	int count = m_browser->get_webview_list()->get_count();
	int snapshot_w;
	int snapshot_h = multiwindow_item_snapshot_h;

	for (int i = 0 ; i < count ; i++) {
		webview *wv = m_browser->get_webview_list()->get_webview(i);
		if (!wv)
			break;

		Evas_Object *snapshot = wv->get_snapshot();
		multiwindow_item *item = new multiwindow_item(snapshot, wv->get_title(), wv->get_uri(), __item_clicked_cb, __item_delete_cb, this);
		Evas_Object *item_layout = item->get_layout();

		if (count > 1)
			item->show_delete_button(EINA_TRUE);

		if (m_multiwindow_item_list.size() < 4) {
			Evas_Object *padding_layout = _create_padding_layout(box, i);
			elm_box_pack_end(box, padding_layout);
			evas_object_show(padding_layout);
			m_padding_layout_list.push_back(padding_layout);
		}

		elm_box_pack_end(box, item_layout);
		evas_object_show(item_layout);

#if !defined(MULTIWINDOW_PINCH_EFFECT_DISABLE)
		if (m_handle_rate <= SHRINK_ITEM_BOUNDARY)
#endif
			item->shrink_layout(EINA_TRUE);

		m_multiwindow_item_list.push_back(item);
	}

	edje_object_signal_callback_add(elm_layout_edje_get(m_padding_layout_list[m_padding_layout_list.size() - 1]), "padding,shrink,finished", "", __hide_padding_layout_finished_cb, this);

	Evas_Object *event_rect = evas_object_rectangle_add(evas_object_evas_get(layout));
	if (!event_rect) {
		BROWSER_LOGD("evas_object_rectangle_add failed");
		return NULL;
	}
	evas_object_size_hint_weight_set(event_rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_color_set(event_rect, 0, 0, 0, 0);
	elm_object_part_content_set(layout, "elm.swallow.gesture", event_rect);

	m_gesture_layer = _create_gesture_layer(event_rect);

	m_scroller = scroller;

	return layout;
}

void multiwindow_view::__hide_padding_layout_finished_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	BROWSER_LOGD("");
	multiwindow_view *mv = (multiwindow_view *)data;
	if (mv->m_open_effect_animator) {
		ecore_animator_del(mv->m_open_effect_animator);
		mv->m_open_effect_animator = NULL;
	}
}

Evas_Object *multiwindow_view::_create_padding_layout(Evas_Object *parent, int index)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	char group_title[50] = {0, };
	sprintf(group_title, "item_padding_%d", index);
	elm_layout_file_set(layout, multiwindow_view_edj_path, group_title);

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	edje_object_signal_emit(elm_layout_edje_get(layout), "padding,shrink,signal", "");

	return layout;
}

