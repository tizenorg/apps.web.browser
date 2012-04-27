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

#ifndef BROWSER_NEW_FOLDER_VIEW_H
#define BROWSER_NEW_FOLDER_VIEW_H

#include "browser-common-view.h"
#include "browser-config.h"

class Browser_New_Folder_View : public Browser_Common_View {
public:
	Browser_New_Folder_View(void);
	~Browser_New_Folder_View(void);

	Eina_Bool init(void);
private:
	Eina_Bool _create_main_layout(void);
	Eina_Bool _create_new_folder(const char *folder_name);
	string _get_default_new_folder_name(void);

	/* Elementary event callback functions */
	static void __cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __save_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data , Evas_Object *obj, void *event_info);

	/* genlist event callback functions */
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);

	Evas_Object *m_genlist;
	Evas_Object *m_conformant;
	Evas_Object *m_save_button;
	Evas_Object *m_cancel_button;
	Evas_Object *m_folder_name_edit_field;

	Elm_Genlist_Item_Class m_item_class;
	Elm_Object_Item *m_navi_it;

	string m_folder_name;
};

#endif /* BROWSER_NEW_FOLDER_VIEW_H */

