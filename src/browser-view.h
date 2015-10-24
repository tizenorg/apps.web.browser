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

#ifndef BROWSER_VIEW_H
#define BROWSER_VIEW_H

#include <Elementary.h>
#include <Evas.h>
#include <vconf.h>

#include "browser-object.h"
#include "common-view.h"

typedef enum _view_mode {
	VIEW_MODE_NORMAL,
	VIEW_MODE_APP_IN_APP,
	VIEW_MODE_FULLSCREEN,
	VIEW_MODE_APP_IN_APP_FULLSCREEN
} VIEW_MODE;

class find_on_page;
class url_bar;
class url_input_bar;
class progress_bar;
class webview;

class browser_view : public common_view {
public:
	browser_view(void);
	~browser_view(void);
	void set_current_webview(webview *wv);
	webview *get_current_webview(void);
	url_bar *get_url_bar(void) { return m_url_bar; }
	url_input_bar *get_url_input_bar(void);
	find_on_page *get_find_on_page(void);
	progress_bar *get_progress_bar(void);
	void rotate(Eina_Bool landscape);
	void launch(const char *uri);
	void on_pause(void);
	Eina_Bool is_show_url_bar(void);
	Eina_Bool is_show_url_input_bar(void);
	Eina_Bool is_show_find_on_page(void);
	void show_url_bar(void);
	void hide_url_bar(void);
	void show_url_input_bar(Eina_Bool enable_voice = EINA_FALSE);
	void show_find_on_page(const char *word, webview *wv, Eina_Bool ime);
	void set_window_view_mode(VIEW_MODE mode);
	Eina_Bool is_ime_on(void) { return m_is_keypad || m_is_clipboard; }
	Eina_Bool is_top_view(void);
	Eina_Bool is_landscape(void) { return m_is_landscape; }
	Evas_Object *get_main_layout(void) { return m_main_layout; }
	void resize_main_layout(int width, int height);
	void set_focus_on_main_layout(void);
	void set_focus_on_content(void);
	Eina_Bool is_valid_uri(const char *uri);
	void set_webview_layout_content(webview *wv);
	Eina_Bool get_notipopup_check(void) { return m_notipopup_check; }
	void set_notipopup_check(Eina_Bool check) { m_notipopup_check = check; }
	void apply_changed_language(void);
	void show_max_window_limit_reached_popup(void);
	void hide_all_views_return_main_view(void);
	browser_view_mode get_view_mode(void) { return m_view_mode; }
	void set_view_mode(browser_view_mode view_mode) { m_view_mode = view_mode; }
	void pop_to_browser_view(void);
	void set_keep_urlbar_on_resume(Eina_Bool status) { m_keep_urlbar_on_resume = status; }

private:
	Evas_Object *_create_layout(Evas_Object *window);
	Evas_Object *_create_button_layout(Evas_Object *parent);

	static void __ime_show_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ime_hide_cb(void *data, Evas_Object *obj, void *event_info);
	static void __keypad_show_cb(void *data, Evas_Object *obj, void *event_info);
	static void __keypad_hide_cb(void *data, Evas_Object *obj, void *event_info);
	static void __clipboard_show_cb(void *data, Evas_Object *obj, void *event_info);
	static void __clipboard_hide_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
	static void __uri_input_bar_dim_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
	static void __uri_input_bar_access_clicked_cb(void *data, Evas_Object *obj, Elm_Object_Item *item);
	static void __button_layout_focused_cb(void *data, Evas_Object *obj, void *event_info);
	static void __button_layout_unfocused_cb(void *data, Evas_Object *obj, void *event_info);
	static void __max_window_limit_reached_ok_cb(void *data, Evas_Object *obj, void *event_info);
	static void __browser_terminate_popup_cancel_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __browser_terminate_popup_ok_button_cb(void *data, Evas_Object *obj, void *event_info);

	static Eina_Bool __naviframe_pop_cb(void *data, Elm_Object_Item *it);

	Evas_Object *m_main_layout;
	Evas_Object *m_button_layout;
	webview *m_webview;
	url_bar *m_url_bar;
	url_input_bar *m_url_input_bar;
	find_on_page *m_find_on_page;
	progress_bar *m_progress_bar;
	Eina_Bool m_is_keypad;
	Eina_Bool m_is_clipboard;
	Elm_Object_Item *m_naviframe_item;
	Eina_Bool m_is_landscape;
	Evas_Object *m_browser_terminate_popup_win;
	Evas_Object *m_browser_terminate_popup;
	Evas_Object *m_max_window_popup;
	browser_view_mode m_view_mode;
	Eina_Bool m_notipopup_check;
	int m_view_width;
	int m_view_height;
	Eina_Bool m_keep_urlbar_on_resume;
};

#endif /* BROWSER_VIEW_H */

