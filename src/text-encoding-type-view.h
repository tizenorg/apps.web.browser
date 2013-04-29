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

#ifndef TEXT_ENCODING_TYPE_VIEW_H
#define TEXT_ENCODING_TYPE_VIEW_H

#include <Evas.h>
#include <vector>

#include "common-view.h"
#include "preference.h"

class text_encoding_type_view : public common_view {
public:
	typedef struct _genlist_callback_data {
		text_encoding_type type;
		void *user_data;
		Elm_Object_Item *it;
	} genlist_callback_data;

	text_encoding_type_view(void);
	~text_encoding_type_view(void);
	void show(void);
	char *get_menu_label(text_encoding_type type);
private:
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_genlist(Evas_Object *parent);

	static char *__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	text_encoding_type m_selected_type;
	Evas_Object *m_genlist;
	Evas_Object *m_radio_main;
	Elm_Genlist_Item_Class *m_item_ic;
	Eina_List* m_genlist_callback_data_list;
protected:
};

#endif /* TEXT_ENCODING_TYPE_VIEW_H */
