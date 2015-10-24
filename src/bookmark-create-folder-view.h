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
 * Contact: Sangpyo Kim <sangpyo7.kim@samsung.com>
 *
 */

#ifndef BOOKMARK_CREATE_FOLDER_VIEW_H
#define BOOKMARK_CREATE_FOLDER_VIEW_H

#include <Elementary.h>
#include <iostream>

#include "browser-object.h"
#include "bookmark-common-view.h"

class bookmark_create_folder_view : public bookmark_common_view {
public:
	typedef enum _menu_type
	{
		TITLE_INPUT_FIELD = 0,
		FOLDER_SELECT_MENU,
		FOLDER_GROUP_TITLE,
		MENU_UNKNOWN
	} menu_type;

	bookmark_create_folder_view(Evas_Smart_Cb cb_func, void *cb_data, int parent_id);
	~bookmark_create_folder_view(void);

	void show();
private:
	typedef struct _genlist_callback_data {
		menu_type type;
		void *user_data;
		void *cp;
		Elm_Object_Item *it;
		int index;
	} genlist_callback_data;

	int _save_folder(void);
	void _back_to_previous_view(void);
	void _do_before_naviframe_pop(void);

	static void __title_entry_changed_cb(void *data, Evas_Object *obj, void *eventInfo);
	static void __title_entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_entry_focused_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info);
	static void __editfield_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __editfield_clear_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	static char *__genlist_get_text_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part);

	static void __timer_popup_expired_cb(void *data, Evas_Object *obj, void *event_info);
	static void __save_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __naviframe_pop_cb(void *data, Elm_Object_Item *it);

	Evas_Object *_create_scroller(Evas_Object * parent);
	Evas_Object *_create_box(Evas_Object * parent);
	void _clear_genlist_item_data(Evas_Object *genlist);
	static void __group_index_lang_changed(void *data, Evas_Object * obj, void *event_info);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	Eina_Bool _set_genlist_item_by_folder(Evas_Object *genlist, int folder_id, Elm_Object_Item *parent_it);
	Eina_Bool _set_genlist_folder_tree(Evas_Object *genlist);
	Evas_Object *_create_genlist(Evas_Object *parent);
	static void __on_layout_resized_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
	void _create_list_items(Evas_Object * parent);

	Evas_Object *m_genlist;
	Evas_Object *m_btn_save;
	Evas_Object *m_scroller;
	Evas_Object *m_main_layout;
	Evas_Object *m_box;
	Evas_Object *m_title_edit_field;
	Evas_Object *m_group_index;
	
	Elm_Object_Item *m_naviframe_item;

	std::string m_input_title_string;
	int m_folder_id;
	int m_saved_id;
	Evas_Smart_Cb m_select_folder_cb_func;
	void *m_select_folder_cb_data;
	Evas_Object* m_title_entry;

	Elm_Genlist_Item_Class *m_itc_folder;
	int m_state_index;
	int m_index;

	Evas_Object *m_contents_layout;
};

#endif /* BOOKMARK_CREATE_FOLDER_VIEW_H */

