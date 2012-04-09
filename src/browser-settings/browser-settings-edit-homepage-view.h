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

#ifndef BROWSER_SETTINGS_EDIT_HOMEPAGE_H
#define BROWSER_SETTINGS_EDIT_HOMEPAGE_H

#include "browser-common-view.h"
#include "browser-config.h"
#include "browser-settings-main-view.h"

class Browser_Settings_Edit_Homepage_View : public Browser_Common_View {
public:
	Browser_Settings_Edit_Homepage_View(Browser_Settings_Main_View *main_view);
	~Browser_Settings_Edit_Homepage_View(void);

	Eina_Bool init(void);
private:
	Eina_Bool _create_main_layout(void);

	/* elementary event callback functions */
	static void __back_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __done_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_field_changed_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_content_layout;
	Evas_Object *m_edit_field;
	Evas_Object *m_done_button;
	Evas_Object *m_cancel_button;

	Browser_Settings_Main_View *m_main_view;
};

#endif /* BROWSER_SETTINGS_EDIT_HOMEPAGE_H */

