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
 * Contact: Jihye Song <jihye3.song@samsung.com>
 *
 */

#ifndef HISTORY_VIEW_H
#define HISTORY_VIEW_H

#include <Evas.h>
#include <Elementary.h>
#include <vector>
#include <system_settings.h>

#include "browser-object.h"
#include "common-view.h"
#include "history-item.h"
#include "history-listener.h"

class history_view : public common_view, public history_listener {
public:
	history_view(void);
	~history_view(void);

	void show(void);
	void hide_history_popups(void);
	int get_list_size(void) { return m_history_list.size(); }
	virtual void __history_cleared(Eina_Bool is_cancelled);
	std::vector<history_item *> get_today_list (void) { return m_today_history_list; }
	std::vector<history_item *> get_yesterday_list (void) { return m_yesterday_history_list; }
	std::vector<history_item *> get_lastweek_list (void) { return m_lastweek_history_list; }
	std::vector<history_item *> get_lastmonth_list (void) { return m_lastmonth_history_list; }
	std::vector<history_item *> get_older_list (void) { return m_older_history_list; }
	std::vector<history_item *> get_ahead_list (void) { return m_ahead_history_list; }
	void _set_selected_title(void);
	void _unset_selected_title(void);
	void set_edit_mode (Eina_Bool mode ) { m_edit_mode = mode; }
	void select_all_items(void);
	void unselect_all_items(void);
private:

	typedef struct {
		Time_stamp_type timestamp;
		Eina_Bool expanded;
	} ItemGroupData;

	Evas_Object *_create_genlist(Evas_Object *parent);
	void _create_show_no_contents();
	void _show_more_context_popup(void);
	void _show_processing_popup(void);
	void _close_processing_popup(void);
	void _close_more_context_popup(void);
	void _populate_genlist(Evas_Object *genlist, history_item *item, Time_stamp_type time_stamp);
	void _set_select_all_layout(void);
	static void __select_all_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info, Eina_Bool checkbox_flow_switch=EINA_FALSE);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static char *__genlist_date_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static char *__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static char *__long_press_popup_genlist_text_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_date_icon_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_date_del_cb(void *data, Evas_Object *obj);
	static void __popup_clear_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_headitem_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_realized_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_share_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_add_bookmark_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_delete_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __toolbar_delete_cb(void *data, Evas_Object *obj, void *event_info);
	static void __toolbar_delete_confirm_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __longpress_popup_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
	static void __popup_set_homepage_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_confirm_response_by_popup_button_cb(void *data, Evas_Object *obj, void *event_info);
	void _delete_history_item(history_item *item);
	void _delete_date_only_label_genlist_item(void);
	void _rebuild_list(void);
	static Eina_Bool __items_append_timer_cb(void *data);
	static void __clear_history_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __more_delete_cb(void *data, Evas_Object *obj, void *event_info);
	void _set_edit_mode(Eina_Bool edit_mode);
	static void __genlist_item_longpressed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_cb(void *data, Evas_Object *obj, void *event_info);
	static void __more_cb(void *data, Evas_Object *obj, void *event_info);
	static void __select_all_key_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
	static void __select_all_key_down_checkbox_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
	static void __toolbar_delete_cancel_cb(void *data, Evas_Object *obj, void *event_info);
	static void __context_popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __rotate_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __resize_history_ctxpopup_cb(void *data);
	static void _close_clear_history_popup(void *data, Evas_Object *obj, void *event_info);
	static void _thread_start_clear_items_from_history_db(void* data, Ecore_Thread *th);
	static void __thread_cancel_cb(void* data, Ecore_Thread *th);
	static void __thread_end_cb(void* data, Ecore_Thread *th);
	void _update_genlist_after_deletion(void* data);
	static void __system_time_changed_cb(system_settings_key_e key, void* data);
	void _update_genlist_item_fields();
	static void __bookmark_clicked_cb(void* data, Evas_Object* obj, void* event_info);
	void _add_tabbar();

	Elm_Object_Item *m_naviframe_item;
	Evas_Object *m_main_layout;
	Evas_Object *m_no_contents_scroller;
	Evas_Object *m_genlist;
	Evas_Object *m_popup_processing;
	Evas_Object *m_progressbar;
	Elm_Genlist_Item_Class *m_item_ic;
	Elm_Genlist_Item_Class *m_date_ic;
	Elm_Genlist_Item_Class *m_long_pressed_popup_genlist_item;
	Evas_Object *m_btn_delete;
	Evas_Object *m_select_all_layout;
	Eina_Bool m_show_toast_message;
	Evas_Object *m_delete_popup;
	Evas_Object *m_title_select_all_button;
	Elm_Object_Item *m_toolbar_item;
	Elm_Object_Item *m_current_selected_item;
	std::vector<history_item *> m_history_list;
	Evas_Object *m_popup;
	history_item* m_selected_item;
	Evas_Object *m_more_popup;
	Eina_Bool m_edit_mode;
	Evas_Object* m_clear_history_popup;
	Evas_Object* m_tabbar;
	Evas_Object *m_history;
	Evas_Object *m_bookmarks;
	std::vector<history_item *> m_today_history_list;
	std::vector<history_item *> m_yesterday_history_list;
	std::vector<history_item *> m_lastweek_history_list;
	std::vector<history_item *> m_lastmonth_history_list;
	std::vector<history_item *> m_older_history_list;
	std::vector<history_item *> m_current_history_list;
	std::vector<history_item *> m_ahead_history_list;
	Elm_Object_Item *m_today_parent;
	Elm_Object_Item *m_yesterday_parent;
	Elm_Object_Item *m_lastweek_parent;
	Elm_Object_Item *m_lastmonth_parent ;
	Elm_Object_Item *m_older_parent;
	Elm_Object_Item *m_parent;
	Elm_Object_Item *m_ahead_parent;
	Ecore_Timer *m_append_timer;
	unsigned int m_count_checked_item;
	unsigned int m_list_item_count;
        static Ecore_Thread *m_current_thread;
        static std::vector<history_item*> m_item_vector;
	bool m_first_tab_selected;
};

#endif /* HISTORY_VIEW_H */

