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

#ifndef BROWSER_VIEW_H
#define BROWSER_VIEW_H

#include <Elementary.h>
#include <Evas.h>

#include "browser-object.h"
#include "common-view.h"

class find_on_page;
class most_visited;
class reader;
class uri_bar;
class uri_input_bar;
class webview;

class browser_view : public browser_object, public common_view {
public:
	browser_view(void);
	~browser_view(void);

	void set_current_webview(webview *wv);
	webview *get_current_webview(void) { return m_webview; }
	uri_bar *get_uri_bar(void) { return m_uri_bar; }
	uri_input_bar *get_uri_input_bar(void);
	find_on_page *get_find_on_page(void) { return m_find_on_page; }
	find_on_page *create_find_on_page(void);
	reader *get_reader(void) { return m_reader; }
	reader *create_reader(void);
	void launch(const char *uri);
	void show_uri_input_bar(Eina_Bool show);
	void show_uri_bar(Eina_Bool show);
	void show_find_on_page(Eina_Bool show, const char *word = NULL, webview *wv = NULL);
	void show_reader(Eina_Bool show);
	Eina_Bool is_show_reader(void);
	Eina_Bool is_show_uri_input_bar(void);
	Eina_Bool is_show_find_on_page(void);
	Eina_Bool is_show_uri_bar(void);
	Eina_Bool is_ime_on(void) { return m_is_ime; }
	void show_prefered_homepage_confirm_popup(void);
	Eina_Bool is_top_view(void);
	void show_jump_to_top_button(Eina_Bool show);
	void show_jump_to_bottom_button(Eina_Bool show);
	Eina_Bool is_show_jump_to_top_button(void);
	Eina_Bool is_show_jump_to_bottom_button(void);
	void disable_webview_event(Eina_Bool disable);
	void show_resize_close_buttons(Eina_Bool show);
	void set_app_in_app_title(const char *title);
	void disable_mini_backward_button(Eina_Bool disable);
	void disable_mini_forward_button(Eina_Bool disable);
#if defined(ADD_TO_HOME)
#if 0
	void show_add_to_home_popup();
#endif
#endif
private:
	Evas_Object *_create_layout(Evas_Object *window);
	void _show_most_visited(Eina_Bool show);
	Evas_Object *_create_gesture_layer(Evas_Object *parent);
	void _jump_to_top_bottom(Eina_Bool top);

	static Evas_Event_Flags __gesture_momentum_start(void *data, void *event_info);
	static Evas_Event_Flags __gesture_momentum_move(void *data, void *event_info);
	static Eina_Bool __flick_timer_cb(void *data);
	static Eina_Bool __jump_top_timer_cb(void *data);
	static Eina_Bool __jump_bottom_timer_cb(void *data);

	static void __back_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ime_show_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ime_hide_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_cb(void *data, Evas_Object *obj, void *event_info);
	static void __prefered_homepage_ok_cb(void *data, Evas_Object *obj, void *event_info);
	static void __prefered_homepage_cancel_cb(void *data, Evas_Object *obj, void *event_info);
	static void __jump_to_top_cb(void *data, Evas_Object *obj, void *event_info);
	static void __jump_to_bottom_cb(void *data, Evas_Object *obj, void *event_info);

	static void __hide_reader_finished_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
#if defined(ADD_TO_HOME)
#if 0
	static void __shortcut_cb(void *data, Evas_Object *obj, void *event_info);
#if defined(WEBCLIP)
	static void __webclip_cb(void *data, Evas_Object *obj, void *event_info);
#endif
#endif
#endif

	static void __app_in_app_close_cb(void *data, Evas* evas, Evas_Object *obj, void *event_info);
	static void __app_in_app_resize_cb(void *data, Evas* evas, Evas_Object *obj, void *event_info);
	static Eina_Bool __app_in_app_mouse_down_cb(void *data, int type, void *event);
	static Eina_Bool __app_in_app_mouse_up_cb(void *data, int type, void *event);
	static Eina_Bool __app_in_app_mouse_move_cb(void *data, int type, void *event);
	static void __app_in_app_full_cb(void *data, Evas* evas, Evas_Object *obj, void *event_info);
	static void __mini_back_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __mini_forward_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_main_layout;
	webview *m_webview;
	uri_bar *m_uri_bar;
	uri_input_bar *m_uri_input_bar;
	find_on_page *m_find_on_page;
	Eina_Bool m_is_ime;
	most_visited *m_most_visited;
	Elm_Object_Item *m_naviframe_item;
	reader *m_reader;

	Evas_Object *m_never_show_checkbox;
	Ecore_Timer *m_jump_top_timer;
	Ecore_Timer *m_jump_bottom_timer;
	Eina_Bool m_is_ficked;
	Ecore_Event_Handler *m_mouse_down_handle;
	Ecore_Event_Handler *m_mouse_move_handle;
	Ecore_Event_Handler *m_mouse_up_handle;
	Eina_Bool m_app_in_app_mouse_down;
	int m_app_in_app_sx;
	int m_app_in_app_sy;
	int m_app_in_app_event_id;
};

#endif /* BROWSER_VIEW_H */

