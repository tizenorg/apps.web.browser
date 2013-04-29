/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
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

#ifndef URI_INPUT_BAR_H
#define URI_INPUT_BAR_H

#include <Evas.h>

#include "browser-object.h"
#include "common-view.h"

class most_visited;
class uri_input_bar : public browser_object {
public:
	uri_input_bar(Evas_Object *parent);
	~uri_input_bar(void);

	Evas_Object *get_layout(void) { return m_main_layout; }
	void set_uri(const char *uri);
	void show_ime(void);
	// To hide entry ime
	void unfocus_entry(void);
private:
	Evas_Object *_create_entry_layout(Evas_Object *parent);
	Evas_Object *_create_main_layout(Evas_Object *parent);
	void _show_search_context_popup(Evas_Object *parent);

	static void __delete_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

	static void __back_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __enter_key_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __search_engine_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __google_engine_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __yahoo_engine_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __bing_engine_selected_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_main_layout;
	Evas_Object *m_uri_entry;
	Evas_Object *m_back_button;
};

#endif /* URI_INPUT_BAR_H */

