/*
*  browser
*
* Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
*
* Contact: junghwan Kang <junghwan.kang@samsung.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#ifndef CLEAR_PRIVATE_DATA_VIEW_H
#define CLEAR_PRIVATE_DATA_VIEW_H
#define MAX_CLEAR_DATA_ITEMS 6

#include "browser-object.h"
#include "common-view.h"

class clear_private_data_view : public common_view {
public:
	typedef enum _menu_type
	{
		CELAR_PRIVATE_DATA_VIEW_SELECT_ALL = 0,
		CELAR_PRIVATE_DATA_VIEW_CLEAR_HISTORY,
		CELAR_PRIVATE_DATA_VIEW_CLEAR_CACHE,
		CELAR_PRIVATE_DATA_VIEW_CLEAR_COOKIE,
		CELAR_PRIVATE_DATA_VIEW_CLEAR_SAVED_PASSWORD,
		CELAR_PRIVATE_DATA_VIEW_CLEAR_FORM_DATA,
		CELAR_PRIVATE_DATA_VIEW_MENU_END,
		CELAR_PRIVATE_DATA_VIEW_MENU_UNKNOWN
	} menu_type;
	clear_private_data_view(void);
	~clear_private_data_view(void);
	Eina_Bool show(void);
	void on_rotate(void);
private:
	typedef struct _genlist_callback_data {
		menu_type type;
		void *user_data;
		Eina_Bool is_checked;
		Evas_Object *checkbox;
		Elm_Object_Item *it;
	} genlist_callback_data;
	Evas_Object *_create_genlist(Evas_Object *parent);
	void _delete_private_data(void);
	void _back_to_previous_view(void);

	static char *__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __select_all_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __show_noti_popup_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_ok_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_cancel_cb(void *data, Evas_Object *obj, void *event_info);
	static void __clear_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void _show_processing_popup(void *data);
	static void _close_processing_popup(void);
	static void _thread_start_clear_private_data(void *data, Ecore_Thread *th);
	static void __thread_cancel_cb(void *data, Ecore_Thread *th);
	static void __thread_end_cb(void *data, Ecore_Thread *th);
	static Eina_Bool __timer_expired_cb(void *data);
	static Eina_Bool __clear_selected_idler_cb(void *data);
	static Eina_Bool __resize_idler_cb(void *data);

	genlist_callback_data m_select_all;
	genlist_callback_data m_clear_history_data;
	genlist_callback_data m_clear_cache_data;
	genlist_callback_data m_clear_cookie_data;
	genlist_callback_data m_clear_saved_password_data;
	genlist_callback_data m_clear_form_data;

	Evas_Object *m_genlist;

	Evas_Object *m_clear_button;
	static Evas_Object *m_popup_processing;
	static Evas_Object *m_progressbar;
	Elm_Genlist_Item_Class *m_item_ic;
	Eina_Bool m_select_all_flag;
	Eina_Bool private_data_list[MAX_CLEAR_DATA_ITEMS];
	static Ecore_Thread *m_current_thread;
	Ecore_Timer *m_process_popup_timer;
	Ecore_Idler *m_clear_selected_idler;
	Ecore_Idler *m_rotate_ctxpopup_idler;
};

#endif /* CLEAR_PRIVATE_DATA_VIEW_H */
