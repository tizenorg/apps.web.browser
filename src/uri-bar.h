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

#ifndef URI_BAR_H
#define URI_BAR_H

#include <Evas.h>

#include "browser-object.h"
#include "common-view.h"
#include "webview.h"

class uri_bar : public browser_object {
public:
	uri_bar(Evas_Object *parent);
	~uri_bar(void);

	Evas_Object *get_layout(void) { return m_main_layout; }
	void set_uri(const char *uri);
	const char *get_uri(void);
	/* private icon will be set, and the uri bar background will be gray. */
	void set_private_mode(Eina_Bool enable, Eina_Bool update_history = EINA_FALSE);
	void update_progress_bar(double rate);
	void set_multi_window_button_count(int window_count);
	void disable_forward_button(Eina_Bool disable);
	void disable_backward_button(Eina_Bool disable);
	void show_reader_icon(Eina_Bool show);
	void show_app_in_app(Eina_Bool app_in_app);
	void disable_buttons(Eina_Bool disable);
private:
	Evas_Object *_create_entry_layout(Evas_Object *parent);
	Evas_Object *_create_main_layout(Evas_Object *parent);
	void _show_loading_status(Eina_Bool loading);
	void _show_more_context_popup(Evas_Object *parent);
	void _set_entry_icon(Eina_Bool private_mode);
	void _show_longpress_back_popup(Evas_Object *parent);

	static void __uri_entry_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
	static void __reload_stop_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

	static void __backward_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __backward_pressed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __backward_unpressed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __bookmark_cb(void *data, Evas_Object *obj, void *event_info);
	static void __bookmark_add_cb(void *data, Evas_Object *obj, void *event_info);
	static void __close_window_cb(void *data, Evas_Object *obj, void *event_info);
	static void __desktop_view_cb(void *data, Evas_Object *obj, void *event_info);
	static void __find_on_page_cb(void *data, Evas_Object *obj, void *event_info);
	static void __forward_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __menu_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __multi_window_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __history_cb(void *data, Evas_Object *obj, void *event_info);
	static void __print_cb(void *data, Evas_Object *obj, void *event_info);
	static void __private_on_off_cb(void *data, Evas_Object *obj, void *event_info);
	static void __reader_cb(void *data, Evas_Object *obj, void *event_info);
	static void __scrap_cb(void *data, Evas_Object *obj, void *event_info);
	static void __scrapbook_cb(void *data, Evas_Object *obj, void *event_info);
	static void __setting_cb(void *data, Evas_Object *obj, void *event_info);
#if defined(INSTALL_WEB_APP)
	static void __install_web_app_cb(void *data, Evas_Object *obj, void *event_info);
#endif
	static void __context_popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_scrap_done_cb(void *data, Evas_Object *obj, void *event_info);
#if defined(ADD_TO_HOME)
	static void __add_to_home_cb(void *data, Evas_Object *obj, void *event_info);
#endif
	static Eina_Bool __back_longpressed_timer_cb(void *data);
	static void __minimize_cb(void *data, Evas_Object *obj, void *event_info);

	static void __mht_contents_get_cb(Evas_Object *ewk_view, const char *data, void *user_data);

	Evas_Object *m_backward_button;
	Evas_Object *m_forward_button;
	Evas_Object *m_main_layout;
	Evas_Object *m_multi_window_button;
	Evas_Object *m_progress_bar;
	Evas_Object *m_progress_bar_bg;
	Evas_Object *m_uri_entry;
	Evas_Object *m_uri_entry_layout;
	Eina_Bool m_is_private_mode;
	Ecore_Timer *m_back_longpressed_timer;
	char *m_scrap_tag;
};

#endif /* URI_BAR_H */

