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

#ifndef BOOKMARK_EDIT_VIEW_H
#define BOOKMARK_EDIT_VIEW_H

#include <Elementary.h>
#include <iostream>

#include "browser-object.h"
#include "common-view.h"

typedef enum _edit_view_mode {
#if defined(BROWSER_TAG)
	EDIT_TAG_VIEW	= 0,
#endif
	EDIT_FOLDER_VIEW
} edit_view_mode;

class bookmark_item;
class bookmark;

class bookmark_edit_view : public browser_object, public common_view {
public:

	bookmark_edit_view(bool tag_mode);
	~bookmark_edit_view(void);

	void show();
	void refresh();
private:
	typedef struct _folder_info {
		int folder_id;
		char *folder_name;
	} folder_info;

	Evas_Object *_create_genlist(Evas_Object *parent);
	Evas_Object *_create_toolbar_btn(Evas_Object *parent, const char *text, Evas_Smart_Cb func, void *data);
	Evas_Object *_create_no_content(Evas_Object *parent, const char *text);
	Evas_Object *_create_box(Evas_Object * parent);
	void _set_contents(void);
	void _set_path_info_layout(void);
	Eina_Bool _set_genlist_folder_view(Evas_Object *genlist);
	Eina_Bool _set_genlist_by_folder(int folder_id, Evas_Object *genlist);
	Eina_Bool _set_folder_genlist(int folder_id, Evas_Object *genlist);
	void _back_to_previous_view(void);
	void _show_more_context_popup(Evas_Object *parent);
	void _show_delete_confirm_popup(void);
	void _show_move_confirm_popup(void);
	void _clear_genlist_item_data(Evas_Object *genlist, edit_view_mode mode);
	void _stat_checked_item(Eina_Bool check_state, void *data);
	void _delete_selected_items(void);
	void _move_selected_items(void);
	void _go_into_sub_folder(int folder_id, const char *folder_name);
	Eina_Bool _go_to_upper_folder();
	void _reorder_bookmark_items(int order_index, Eina_Bool is_move_down);
	void _show_selection_info(Evas_Object *parent, unsigned int count);

	static char *__genlist_get_text_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_moved_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_realized_cb(void *data, Evas_Object *obj, void *event_info);

	static void __chk_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __move_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __select_all_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __more_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_entry_changed_cb(void * data, Evas_Object * obj, void * event_info);
	static void __ok_btn_clicked_cb(void * data, Evas_Object * obj, void * event_info);
	static void __rename_folder_cb(void * data, Evas_Object * obj, void * event_info);
	static void __cancel_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ctxpopup_rename_folder_by_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ctxpopup_add_new_folder_by_cb(void *data, Evas_Object *obj, void *event_info);
	static void __create_folder_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ok_response_delete_confirm_popup_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ok_response_move_confirm_popup_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_bookmark_btn_cb(void *data, Evas_Object *obj, void *event_info);
	static void __go_into_sub_folder_btn_cb(void *data, Evas_Object *obj, void *event_info);
	static void __select_folder_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_box;
	Evas_Object *m_main_layout;
	Evas_Object *m_path_info_layout;
	Evas_Object *m_genlist;
	Evas_Object *m_titlebar_btn_select_all;
	Evas_Object *m_toolbar_btn_more; // folder view
	Evas_Object *m_toolbar_btn_move; // folder view
	Evas_Object *m_toolbar_btn_delete; // folder view
	Evas_Object *m_toolbar_btn_back; // folder view
	Evas_Object *m_popup_selection_info;
	Evas_Object *m_rename_folder_select_popup;
	Evas_Object *m_rename_folder_popup;
	Elm_Object_Item *m_naviframe_item;

	Elm_Genlist_Item_Class m_itc_folder;
	Elm_Genlist_Item_Class m_itc_bookmark_folder;

	std::vector<bookmark_item *> m_bookmark_list;
	edit_view_mode m_view_mode;
	bookmark *m_bookmark;
	unsigned int m_curr_folder;
	unsigned int m_count_checked_item;
	unsigned int m_count_editable_item;
	unsigned int m_count_folder_item;
	unsigned int m_folder_id_to_move;
	std::string m_rename_folder_string;
	std::vector<folder_info *> m_path_history;
	std::string m_path_string;

#if defined(BROWSER_TAG)
	Eina_Bool _set_genlist_tag_view(Evas_Object *genlist);
	static char *__genlist_get_tag_text_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_get_tag_content_cb(void *data, Evas_Object *obj, const char *part);
	static void __remove_tag_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_toolbar_btn_remove_tag; // tag view
	Elm_Genlist_Item_Class m_itc_tag;
	Elm_Genlist_Item_Class m_itc_bookmark_tag;
#endif
};

#endif /* BOOKMARK_EDIT_VIEW_H */

