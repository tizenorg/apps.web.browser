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

#ifndef READER_H
#define READER_H

#include <Elementary.h>
#include <Evas.h>

#include "browser-object.h"
#include "common-view.h"
#include "webview.h"

class reader : public browser_object, public common_view {
public:
	reader(void);
	~reader(void);

	Evas_Object *get_layout(void);
	Eina_Bool execute_reader_js(void);
	Eina_Bool execute_recognizearticle_js(void);
	void delete_layout(void);
	void show_toolbar(Eina_Bool show);
private:
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_toolbar_layout(Evas_Object *parent);
	void _show_more_context_popup(Evas_Object *parent);

	static void __more_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_cb(void *data, Evas_Object *obj, void *event_info);
	static void __small_font_cb(void *data, Evas_Object *obj, void *event_info);
	static void __large_font_cb(void *data, Evas_Object *obj, void *event_info);
	static void __print_cb(void *data, Evas_Object *obj, void *event_info);
	static void __execute_reader_js_cb(Evas_Object *obj, const char *javascript_result, void *data);
	static void __execute_recognizearticle_js_cb(Evas_Object *obj, const char *javascript_result, void *data);

	static void __load_commited_cb(void *data, Evas_Object *obj, void *event_info);
	static void __load_finished_cb(void *data, Evas_Object *obj, void *event_info);

	static void __ime_show_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ime_hide_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_to_scrap_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_scrap_done_cb(void *data, Evas_Object *obj, void *event_info);
	static void __mht_contents_get_cb(Evas_Object *ewk_view, const char *data, void *user_data);

	webview *m_webview;
	Evas_Object *m_main_layout;
	const char *m_reader_html;
	char *m_scrap_tag;
	Evas_Object *m_small_font_button;
	Evas_Object *m_large_font_button;
};

#endif /* READER_H */

