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

#ifndef __URL_INPUT_BAR_H__
#define __URL_INPUT_BAR_H__

#include <app_control.h>
#include <string>

#include "browser-object.h"

class url_input_bar : public browser_object {
public:
	url_input_bar(Evas_Object *parent);
	~url_input_bar(void);

	Evas_Object *get_layout(void) { return m_main_layout; }
	void set_uri(const char *uri);
	void show_ime(Eina_Bool enable_voice = EINA_FALSE);
	void select_all_text(void);
	void set_entry_changed_callback(void);
	void unset_entry_changed_callback(void);
	void update_search_engine(int type);
	void resize_search_ctxpopup(void);
	void set_landscape_mode(Eina_Bool landscape);
	Eina_Bool get_focus();
	void set_focus();
	void set_focus(Eina_Bool focus);
	void reset_string(void);
	std::string get_string(void) { return m_saved_string; }

private:
	void _create_main_layout(Evas_Object *parent);
	void _show_search_context_popup(Evas_Object *parent);
	void _calculate_ctx_popup_height(Evas_Object *ctx_popup);
	void _remove_preedit_tags(std::string &kw);
	void _remove_preedit_sel_section(std::string &kw);
	void _remove_underline_color_tags(std::string &kw);
	char *_search_keyword_get(Evas_Object *obj);
	std::string _get_string(void) { return m_saved_string; }
	void _set_string(std::string &kw);

#if defined(HW_MORE_BACK_KEY)
	static void __back_cb(void *data, Evas_Object *obj, void *event_info);
	static void __menu_clicked_cb(void *data, Evas_Object *obj, void *event_info);
#endif
	static Eina_Bool __select_all_text_cb(void *data);
	static void __search_engine_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __resize_search_ctxpopup_cb(void *data);
	static void __dismissed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __google_engine_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __yahoo_engine_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __bing_engine_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __share_cb(void *data, Evas_Object *obj, void *event_info);
	static void __enter_key_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_double_clicked_cb(void* data, Evas_Object* obj, void* event_info);
	static void __entry_longpressed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_language_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __dim_area_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

	static Eina_Bool m_double_or_long_click;
	Evas_Object *m_main_layout;
	Evas_Object *m_search_engine_button;
	Evas_Object *m_context_popup;
	Evas_Object *m_entry;
	std::string m_saved_string;
};

#endif // __URL_INPUT_BAR_H__
