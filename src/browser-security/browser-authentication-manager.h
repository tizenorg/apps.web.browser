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

#ifndef BROWSER_PERSONAL_DATA_MANAGER_H
#define BROWSER_PERSONAL_DATA_MANAGER_H

#include "browser-common-view.h"
#include "browser-config.h"

#include <string>

using namespace std;

class Browser_Authetication_Manager : public Browser_Common_View {
public:
	Browser_Authetication_Manager(void);
	~Browser_Authetication_Manager(void);

	Eina_Bool init(void);
private:
	static void __show_auth_dialog_cb(const char *msg, const char *uri, void *data);
	static void __popup_reponse_cb(void* data, Evas_Object* obj, void* event_info);
	static void __popup_cancel_cb(void* data, Evas_Object* obj, void* event_info);

	static Evas_Object *m_popup;
	static Evas_Object *m_user_name_edit_field;
	static Evas_Object *m_password_edit_field;
};

#endif /* BROWSER_PERSONAL_DATA_MANAGER_H */

