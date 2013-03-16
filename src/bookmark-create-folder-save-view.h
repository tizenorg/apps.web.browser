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

#ifndef BOOKMARK_CREATE_FOLDER_SAVE_VIEW_H
#define BOOKMARK_CREATE_FOLDER_SAVE_VIEW_H

#include <Elementary.h>
#include <iostream>

#include "browser-object.h"
#include "common-view.h"

class bookmark_create_folder_save_view : public browser_object, public common_view {
public:
	bookmark_create_folder_save_view(Evas_Smart_Cb cb_func = NULL, void *cb_data = NULL, int folder_id_to_save=root_folder_id);
	~bookmark_create_folder_save_view(void);

	void show();
private:
	typedef struct _genlist_callback_data {
		void *user_data;
		void *cp;
		Elm_Object_Item *it;
	} genlist_callback_data;

	Evas_Object *_create_genlist(Evas_Object *parent);
	int _save_folder(void);
	void _back_to_previous_view(void);

	static void __title_entry_changed_cb(void *data, Evas_Object *obj, void *eventInfo);
	static void __title_entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info);

	static Evas_Object *__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	static void __save_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ime_show_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ime_hide_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_genlist;
	Evas_Object *m_back_button;
	Evas_Object *m_title_edit_field;
	Evas_Object *m_btn_save;
	Evas_Object *m_titlebar_btn_save;
	Evas_Object *m_titlebar_btn_back;
	
	Elm_Object_Item *m_naviframe_item;
	Elm_Genlist_Item_Class *m_itc_title;
	genlist_callback_data m_input_title_callback_data;

	std::string m_input_title_string;
	int m_folder_id;
	Evas_Smart_Cb m_cb_func;
	void *m_cb_data;

};

#endif /* BOOKMARK_CREATE_FOLDER_SAVE_VIEW_H */

