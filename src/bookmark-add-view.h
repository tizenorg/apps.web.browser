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

#ifndef BOOKMARK_ADD_VIEW_H
#define BOOKMARK_ADD_VIEW_H

#include <Elementary.h>
#include <iostream>

#include "browser-object.h"
#include "bookmark-common-view.h"

class bookmark_add_view : public bookmark_common_view {
public:
	typedef enum _menu_type
	{
		TITLE_INPUT_FIELD,
		URI_INPUT_FIELD,
		FOLDER_GROUP_TITLE,
		SEPARATOR,
		FOLDER_SELECT_MENU,

		MENU_UNKNOWN
	} menu_type;

	bookmark_add_view(const char *title, const char *uri,
								int folder_id_to_save,
								Eina_Bool edit_mode);
	~bookmark_add_view(void);

	void show();
	void set_folder_id(int folder_id) { m_folder_id = folder_id;}
	void back(void);
	void set_edtifield_focus_allow_set(Eina_Bool enable);
private:
	typedef struct _genlist_callback_data {
		menu_type type;
		void *user_data;
		void *cp;
		Eina_Bool is_checked;
		Elm_Object_Item *it;
	} genlist_callback_data;

	Evas_Object *_create_genlist(Evas_Object *parent);
	int _save_bookmark(void);
	void _back_to_previous_view(void);
	Eina_Bool _is_empty(const char* entry_text);

	static void __title_entry_changed_cb(void *data, Evas_Object *obj, void *eventInfo);
	static void __title_entry_focused_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_entry_eraser_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __uri_entry_changed_cb(void *data, Evas_Object *obj, void *eventInfo);
	static void __uri_entry_focused_cb(void *data, Evas_Object *obj, void *event_info);
	static void __uri_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info);
	static void __uri_entry_eraser_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_enter_key_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_next_key_cb(void *data, Evas_Object *obj, void *eventInfo);
	static void __editfield_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_clear_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static char *__genlist_get_text_cb(void *data, Evas_Object *obj, const char *part);
	static char *__genlist_get_title_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_realized_cb(void *data, Evas_Object *obj, void *event_info);

	static void __save_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __timer_popup_expired_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __select_folder_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_free_cb(void *data, Evas *e, Evas_Object *genlist, void *ei);

	Evas_Object *m_scroller;
	Evas_Object *m_box;
	Evas_Object *m_genlist;
	Evas_Object *m_title_edit_field;
	Evas_Object *m_uri_edit_field;
	Evas_Object *m_btn_save;
	Evas_Object *m_btn_cancel;
	char *m_bookmark_edit_title;
	char *m_bookmark_edit_url;
	
	Elm_Object_Item *m_naviframe_item;
	genlist_callback_data m_input_title_callback_data;
	genlist_callback_data m_input_uri_callback_data;

	genlist_callback_data m_group_title_callback_data;
	genlist_callback_data m_select_folder_callback_data;
	Elm_Entry_Filter_Limit_Size m_entry_limit_size;

	std::string m_uri_string;
	std::string m_input_uri_string;
	std::string m_input_title_string;
	std::string m_origin_uri_string;
	std::string m_origin_title_string;
	int m_folder_id;
	int m_origin_folder_id;
	Eina_Bool m_edit_mode;
	int m_bookmark_id;
};

#endif /* BOOKMARK_ADD_VIEW_H */

