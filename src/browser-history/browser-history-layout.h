/*
Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved

This file is part of browser
Written by Hyerim Bae hyerim.bae@samsung.com.

PROPRIETARY/CONFIDENTIAL

This software is the confidential and proprietary information of
SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered
into with SAMSUNG ELECTRONICS.

SAMSUNG make no representations or warranties about the suitability
of the software, either express or implied, including but not limited
to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. SAMSUNG shall not be liable for any
damages suffered by licensee as a result of using, modifying or
distributing this software or its derivatives.
*/

#ifndef BROWSER_HISTORY_LAYOUT_H
#define BROWSER_HISTORY_LAYOUT_H

#include "browser-common-view.h"
#include "browser-config.h"
#include "browser-history-db.h"
#include "browser-bookmark-view.h"

class Date {
public:
	int year;
	int month;
	int day;

	Date();
	Date(Date &date);
	void operator=(Date &date);
	bool operator==(Date &date);
	bool operator!=(Date &date);
};

class Browser_History_Layout : public Browser_Common_View {
	friend class Browser_Bookmark_View;
public:
	Browser_History_Layout(void);
	~Browser_History_Layout(void);

	Eina_Bool init(void);
	Evas_Object *get_main_layout(void) { return m_searchbar_layout; }
protected:
	void _set_edit_mode(Eina_Bool edit_mode);

private:
	typedef struct _history_date_param {
		Date date;
		Browser_History_Layout *history_layout;
	} history_date_param;

	Eina_Bool _create_main_layout(void);
	Evas_Object *_create_history_genlist(void);
	void _reload_history_genlist(void);
	void _show_selection_info(void);
	void _delete_selected_history(void);
	Evas_Object *_show_delete_confirm_popup(void);
	void _show_select_processing_popup(void);
	void _show_delete_processing_popup(void);
	void _enable_searchbar_layout(Eina_Bool enable);
	Eina_Bool _show_searched_history(const char *search_text);
	void _delete_date_only_label_genlist_item(void);

	/* elementary event callback functions. */
	static void __history_item_clicked_cb(void *data, Evas_Object *obj, void *eventInfo);
	static void __edit_mode_item_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_mode_select_all_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_confirm_response_by_edit_mode_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_confirm_response_by_slide_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __select_processing_popup_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_processing_popup_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __search_delay_changed_cb(void *data, Evas_Object *obj, void *event_info);

	/* evas object event callback functions */
	static void __edit_mode_select_all_clicked_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);

	/* genlist callback functions. */
	static char *__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static char *__genlist_date_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);

	/* ecore timer callback functions */
	static Eina_Bool __select_processing_popup_timer_cb(void *data);
	static Eina_Bool __delete_processing_popup_timer_cb(void *data);

	history_date_param m_date_param;
	Elm_Genlist_Item_Class m_history_genlist_item_class;
	Elm_Genlist_Item_Class m_history_group_title_class;

	Evas_Object *m_searchbar_layout;
	Evas_Object *m_searchbar;
	Evas_Object *m_history_genlist;
	Evas_Object *m_searched_history_genlist;
	Evas_Object *m_no_content_search_result;
	Evas_Object *m_content_box;
	Evas_Object *m_edit_mode_select_all_layout;
	Evas_Object *m_edit_mode_select_all_check_button;
	Evas_Object *m_no_history_label;

	vector<Browser_History_DB::history_item *> m_history_list;
	vector<char *> m_history_date_label_list;
	vector<Browser_History_DB::history_item *> m_searched_history_item_list;
	vector<char *> m_searched_history_date_label_list;
	Date m_last_date;

	/* For select all processing popup. */
	Evas_Object *m_processing_progress_bar;
	Ecore_Timer *m_processing_popup_timer;
	Elm_Object_Item *m_processed_it;
	int m_processed_count;
	Evas_Object *m_processing_popup;
	Evas_Object *m_processing_popup_layout;
	Eina_Bool m_select_all_check_value;
	int m_total_item_count;

	Evas_Object *m_delete_confirm_popup;
};
#endif /* BROWSER_HISTORY_LAYOUT_H */

