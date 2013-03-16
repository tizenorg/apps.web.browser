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

#ifndef MOST_VISITED_H
#define MOST_VISITED_H

#include <Elementary.h>
#include <Evas.h>
#include <vector>
#include "browser-object.h"

#define most_visited_edj_path browser_edj_dir"/most-visited.edj"

#define most_visited_item_count	6

class most_visited_item;
class most_visited : public browser_object {
public:
	most_visited(void);
	~most_visited(void);

	Evas_Object *get_layout(void) { return m_main_layout; }
private:
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_gengrid(Evas_Object *parent);
	void _show_option_popup(most_visited_item *item, Elm_Object_Item *selected_it);

	static void __delete_cb(void *data, Evas_Object *obj, void *event_info);
	static void __item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __item_longpressed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __gengrid_scroll_cb(void *data, Evas_Object *obj, void *event_info);
	static void __option_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info);
	static void __private_open_cb(void *data, Evas_Object *obj, void *event_info);
	static void __set_as_homepage_cb(void *data, Evas_Object *obj, void *event_info);

	static Evas_Object *__get_content_cb(void *data, Evas_Object *obj, const char *part);
	static char *__get_text_cb(void *data, Evas_Object *obj, const char *part);

	std::vector<most_visited_item *> m_most_visited_item_list;

	Evas_Object *m_gengrid;
	Evas_Object *m_main_layout;
	Elm_Gengrid_Item_Class *m_gengrid_ic;
};

#endif /* MOST_VISITED_H */

