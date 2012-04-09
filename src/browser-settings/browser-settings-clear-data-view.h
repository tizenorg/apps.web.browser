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

#ifndef BROWSER_SETTINGS_CLEAR_DATA_VIEW_H
#define BROWSER_SETTINGS_CLEAR_DATA_VIEW_H

#include "browser-common-view.h"
#include "browser-config.h"
#include "browser-settings-main-view.h"

class Browser_Settings_Clear_Data_View : public Browser_Common_View {
public:
	typedef enum _menu_type
	{
		BR_SELECT_ALL = 0,
		BR_CLEAR_HISTORY,
		BR_CLEAR_CACHE,
		BR_CLEAR_COOKIE,
		BR_CLEAR_SAVED_ID_PASSWORD,
		BR_MENU_UNKNOWN
	}menu_type;

	Browser_Settings_Clear_Data_View(Browser_Settings_Main_View *main_view);
	~Browser_Settings_Clear_Data_View(void);

	Eina_Bool init(void);
private:
	typedef struct _genlist_callback_data {
		menu_type type;
		void *user_data;
		Elm_Object_Item *it;
	} genlist_callback_data;

	Eina_Bool _create_main_layout(void);
	Evas_Object *_create_genlist(void);
	Evas_Object *_show_delete_confirm_popup(void);
	void _delete_private_data(void);

	/* Elementary event callback functions */
	static void __cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __select_all_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __item_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);

	/* genlist event callback functions */
	static char *__genlist_label_get(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_icon_get(void *data, Evas_Object *obj, const char *part);

	/* Evas object event callback functions */
	static void __select_all_layout_mouse_down_cb(void *data, Evas *evas,
							Evas_Object *obj, void *event_info);

	Evas_Object *m_genlist;
	Evas_Object *m_back_button;
	Evas_Object *m_delete_button;
	Evas_Object *m_bottom_control_bar;
	Elm_Object_Item *m_delete_controlbar_item;

	Evas_Object *m_select_all_check_box;
	Evas_Object *m_history_check_box;
	Evas_Object *m_cache_check_box;
	Evas_Object *m_cookie_check_box;
	Evas_Object *m_saved_id_password_check_box;

	Evas_Object *m_select_all_layout;
	Evas_Object *m_content_box;

	Evas_Object *m_delete_confirm_popup;

	Elm_Genlist_Item_Class m_1text_item_class;
	genlist_callback_data m_clear_history_item_callback_data;
	genlist_callback_data m_clear_cache_item_callback_data;
	genlist_callback_data m_clear_cookie_item_callback_data;
	genlist_callback_data m_clear_saved_id_password_item_callback_data;

	Browser_Settings_Main_View *m_main_view;
};

#endif /* BROWSER_SETTINGS_CLEAR_DATA_VIEW_H */

