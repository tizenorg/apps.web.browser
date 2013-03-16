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

#ifndef FIND_ON_PAGE_H
#define FIND_ON_PAGE_H

#include <Evas.h>

#include "browser-object.h"
#include "common-view.h"

class webview;
class find_on_page : public browser_object {
public:
	find_on_page(void);
	~find_on_page(void);

	Evas_Object *get_layout(void) { return m_main_layout; }

	void show_ime(void);
	// To hide entry ime
	void unfocus_entry(void);
	void clear_text(void);
	void set_text(const char *text);
	void set_webview(webview *wv) { m_webview = wv; }
private:
	Evas_Object *_create_entry_layout(Evas_Object *parent);
	Evas_Object *_create_main_layout(Evas_Object *parent);

	void _set_count(int index, int total);
	void _disable_down_button(Eina_Bool disable);
	void _disable_up_button(Eina_Bool disable);

	static void __delete_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

	static void __back_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __down_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __up_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __enter_key_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __text_found_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_main_layout;
	Evas_Object *m_entry;
	Evas_Object *m_entry_layout;
	Evas_Object *m_back_button;
	Evas_Object *m_down_button;
	Evas_Object *m_up_button;

	int m_total_count;
	int m_current_index;
	const char *m_input_word;
	webview *m_webview;
};

#endif /* FIND_ON_PAGE_H */

