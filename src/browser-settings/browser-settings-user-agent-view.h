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

#ifndef BROWSER_SETTINGS_USER_AGENT_VIEW_H
#define BROWSER_SETTINGS_USER_AGENT_VIEW_H

#include "browser-common-view.h"
#include "browser-config.h"

class Browser_Settings_Main_View;
class Browser_Settings_User_Agent_View : public Browser_Common_View {
public:
	Browser_Settings_User_Agent_View(Browser_Settings_Main_View *main_view);
	~Browser_Settings_User_Agent_View(void);

	Eina_Bool init(void);

	typedef enum _user_agent_type {
		TIZEN,
		CHROME,
		FIREFOX,
		UNKNOWN
	} user_agent_type;
private:
	typedef struct _genlist_callback_data {
		user_agent_type type;
		void *user_data;
		Elm_Object_Item *it;
	} genlist_callback_data;

	Eina_Bool _create_main_layout(void);

	/* Elementary event callback functions */
	static void __back_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	static char *__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __check_changed_cb( void *data, Evas_Object *obj, void *event_info);
	static void __item_selected_cb(void *data, Evas_Object *obj, void *event_info);

	Browser_Settings_Main_View *m_main_view;
	Evas_Object *m_back_button;
	Evas_Object *m_genlist;
	Evas_Object *m_tizen_checkbox;
	Evas_Object *m_chrome_checkbox;
	Evas_Object *m_firefox_checkbox;

	genlist_callback_data m_tizen_item_callback_data;
	genlist_callback_data m_chrome_item_callback_data;
	genlist_callback_data m_firefox_item_callback_data;

	Elm_Genlist_Item_Class m_1_text_1_icon_item_class;
};

#endif /* BROWSER_SETTINGS_ABOUT_BROWSER_H */

