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

#ifndef BOOKMARK_VIEW_H
#define BOOKMARK_VIEW_H

#include <Elementary.h>
#include <vector>
#include "browser-object.h"
#include "common-view.h"
#include "bookmark-edit-view.h"
#include "preference.h"
#include "bookmark-listener.h"

class bookmark_item;
class bookmark;

class bookmark_view : public common_view, public bookmark_listener  {
public:
	bookmark_view(void);
	~bookmark_view(void);

	void show();
	Evas_Object *get_layout(void) { return m_main_layout; }
	Elm_Object_Item *get_bookmark_naviframe_item(void){ return m_naviframe_item; }
	void rotate(void);
	void refresh(void);
	Eina_Bool get_bookmark_on_off_icon_clicked(void){return m_is_bookmark_on_off_icon_clicked_history; }
	void  set_bookmark_on_off_icon_clicked(Eina_Bool bookmark_flag){ m_is_bookmark_on_off_icon_clicked_history = bookmark_flag ; }
	void hide_bookmark_popups();
	void stop_scroll_timer(void);
	void delete_bookmark_more_context_popup(void);
	virtual void __bookmark_added(const char *uri,  int bookmark_id, int parent_id);
	virtual void __bookmark_removed(const char *uri, int id, int parent_id);
	virtual void __bookmark_updated(const char *uri, const char *title, int bookmark_id, int parent_id);

private:
	Evas_Object *_create_genlist(Evas_Object *parent);
	static void __no_content_lang_changed(void *data, Evas_Object * obj, void *event_info);

	Eina_Bool _set_genlist_folder_view(Evas_Object *genlist);
	Eina_Bool _set_genlist_folder_tree_recursive(int folder_id,
							Evas_Object *genlist,Elm_Object_Item *parent_item);
	Eina_Bool _set_genlist_by_folder(int folder_id,
							Evas_Object *genlist,Elm_Object_Item *parent_item);
	void _set_view_mode(bookmark_view_type mode);
	bookmark_view_type _get_view_mode(void) { return m_view_mode; }
	void _show_more_context_popup(void);
	void _go_into_sub_folder(int folder_id, const char *folder_name);
	Eina_Bool _go_to_upper_folder();
	void _delete_ctx_popup(void);
	void _set_focus(void);
	Eina_Bool _is_valid_homepage (const char *uri);
	void _add_tabbar();

	//void _create_navigator(void);
	void _go_into_sub_path(int folder_id);
	void _go_to_clicked_path(Elm_Object_Item *it);
	static void __navigator_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	static void __back_cb(void *data, Evas_Object *obj, void *event_info);
	static void __more_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_bookmark_cb(void *data);
	static char *__genlist_get_label_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_cont_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_exp_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_del_cb(void *data, Evas *e, Evas_Object *genlist, void *ei);
	static void __genlist_realized_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ctxpopup_add_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ctxpopup_create_folder_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ctxpopup_reorder_edit_view_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ctxpopup_move_edit_view_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ctxpopup_delete_edit_view_cb(void *data, Evas_Object *obj, void *event_info);
	static void __view_by_popup_folder_cb(void *data, Evas_Object *obj, void *event_info);
	static void __refresh_bookmark_view_cb(void *data, Evas_Object *obj, void *event_info);
	static void __launch_create_folder_cb(void *data, Evas_Object *obj, void *event_info);
	static void __launch_reorder_cb(void *data, Evas_Object *obj, void *event_info);
	static void __launch_move_cb(void *data, Evas_Object *obj, void *event_info);
	static void __launch_delete_cb(void *data, Evas_Object *obj, void *event_info);
	static void __launch_add_bookmark_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __resize_more_ctxpopup_cb(void *data);
	Eina_Bool _set_genlist_by_dividing(int folder_id, Evas_Object *genlist, Elm_Object_Item *parent_item);
	static void __scroller_edge_bottom_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __history_clicked_cb(void* data, Evas_Object* obj, void* event_info);
	static char *__genlist_text_get_cb(void *data, Evas_Object *obj, const char *part);

	Evas_Object *m_main_layout;
	Evas_Object *m_genlist;
	Evas_Object *m_no_contents_scroller;
	Evas_Object *m_ctx_popup_more_menu;
	Evas_Object *m_tabbar;
	Evas_Object *m_history;
	Evas_Object *m_bookmarks;
	Elm_Object_Item *m_naviframe_item;
	Elm_Object_Item *m_genlist_item;
	Ecore_Idler *m_clear_selected_idler;

	std::vector<bookmark_item *> m_bookmark_list;
	bookmark *m_bookmark;
	bookmark_view_type m_view_mode;
	int m_curr_folder_id;
	int m_prev_momentum_y;

	Eina_Bool m_is_bookmark_on_off_icon_clicked_history;
	int m_curr_view_folder_count;
	int m_curr_view_editable_bookmark_count;

	int m_list_item_count;
	Eina_Bool m_operator_genlist_set;

	Evas_Object *m_navigationbar;
	std::vector<int> m_path_list;
};

#endif /* BOOKMARK_VIEW_H */
