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

#ifndef BROWSER_SELECT_FOLDER_VIEW_H
#define BROWSER_SELECT_FOLDER_VIEW_H

#include "browser-common-view.h"
#include "browser-config.h"

class Browser_Select_Folder_View : public Browser_Common_View {
public:
	Browser_Select_Folder_View(int current_folder_id);
	~Browser_Select_Folder_View(void);

	Eina_Bool init(void);
	void return_to_select_folder_view(void);
private:
	Eina_Bool _create_main_layout(void);
	void _fill_folder_list(void);

	/* Elementary event callback functions */
	static void __folder_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __new_folder_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __done_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info);

	Evas_Object *m_list;
	Evas_Object *m_content_layout;
	Evas_Object *m_conformant;
	Evas_Object *m_bottom_control_bar;
	Evas_Object *m_cancel_button;
	Evas_Object *m_done_button;
	Elm_Object_Item *m_navi_it;

	int m_current_folder_id;
	vector<Browser_Bookmark_DB::bookmark_item *> m_folder_list;
	Browser_Bookmark_DB::bookmark_item *m_main_folder_item;
};

#endif /* BROWSER_SELECT_FOLDER_VIEW_H */

