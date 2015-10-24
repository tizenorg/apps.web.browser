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
 * Contact: Jiwon Lee <jiwonear.lee@samsung.com>
 *
 */

#ifndef BOOKMARK_EDIT_VIEW_H
#define BOOKMARK_EDIT_VIEW_H

#include <Elementary.h>
#include <iostream>

#include "browser-object.h"
#include "bookmark-common-view.h"
#include "platform-service.h"

typedef enum _edit_view_mode {
	EDIT_FOLDER_REORDER_VIEW
	,EDIT_FOLDER_MOVE_VIEW
	,EDIT_FOLDER_DELETE_VIEW
	,EDIT_UNKNOWN_VIEW
} edit_view_mode;

class bookmark_item;
class bookmark;

class bookmark_edit_view : public bookmark_common_view {
public:

	bookmark_edit_view(Evas_Smart_Cb cb_func, void *cb_data, int folder_id, const char *path_string, edit_view_mode view_mode);
	~bookmark_edit_view(void);

	void show();
	void refresh();
	void apply_changed_language();
private:
	Evas_Object *_create_genlist(Evas_Object *parent);
	Evas_Object *_create_no_content(Evas_Object *parent, const char *text);
	Evas_Object *_create_box(Evas_Object * parent);
	void _set_contents(void);
	void _set_select_all_layout(void);
	Eina_Bool _set_genlist_folder_view(Evas_Object *genlist);
	Eina_Bool _set_genlist_by_folder(int folder_id, Evas_Object *genlist);
	void _stay_current_folder(void);
	void _back_to_previous_view(void);
	void _back_to_browser_view(void);
	void _do_before_naviframe_pop(void);
	void _stat_checked_item(Eina_Bool check_state, void *data);
	void _reorder_bookmark_items(bookmark_item *moved_item, int order_index);
	void _reorder_bookmark_items_all(void);
	Eina_Bool _is_disable_done_button(bookmark_item *moved_item, int order_index);
	void _set_selected_title();
	void _unset_selected_title();
	void _show_processing_popup(void);
	void _close_processing_popup(void);
	void _clear_delete_confirm_popup(void);
	void _show_delete_confirm_popup(void);

	static char *__genlist_get_text_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_moved_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_realized_cb(void *data, Evas_Object *obj, void *event_info);
	static void __chk_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_delete_confirm_popup_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ok_delete_confirm_popup_cb(void *data, Evas_Object *obj, void *event_info);
	static void __done_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __select_all_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info, Eina_Bool checkbox_flow_switch=EINA_FALSE);
	static void __more_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __naviframe_pop_cb(void *data, Elm_Object_Item *it);
	static void __processing_popup_thread_cb(void *data, Ecore_Thread *th);
	static void __thread_end_cb(void *data, Ecore_Thread *th);
	static void __thread_cancel_cb(void *data, Ecore_Thread *th);
	static void __genlist_del_cb(void *data, Evas *e, Evas_Object *genlist, void *ei);
	static void __select_all_key_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
	static void __select_all_key_down_checkbox_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
	static void __reorder_button_mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
	static void __reorder_button_mouse_up_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);

	Evas_Object *m_outer_layout;
	Evas_Object *m_box;
	Evas_Object *m_main_layout;
	Evas_Object *m_genlist;
	Evas_Object *m_no_contents_layout;
	Evas_Object *m_select_all_layout;
	Evas_Object *m_ctx_popup_more_menu;
	Evas_Object *m_popup_processing;
	Evas_Object *m_progressbar;
	Evas_Object *m_btn_done;
	Evas_Object *m_btn_cancel;
	Evas_Object *m_delete_confirm_popup;
	Elm_Object_Item *m_naviframe_item;

	std::vector<bookmark_item *> m_bookmark_list;
	edit_view_mode m_view_mode;
	bookmark *m_bookmark;
	int m_curr_folder;
	unsigned int m_count_checked_item;
	int m_reorder_dirty_flag;
	Eina_Bool m_check_registered;
	std::string m_sub_title;
	std::string m_popup_text;

	Evas_Smart_Cb m_cb_func;
	void *m_cb_data;
	Elm_Theme *m_theme;

	static Ecore_Thread *m_current_thread;
};

#endif /* BOOKMARK_EDIT_VIEW_H */


