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
#include "browser-personal-data-db.h"

#include <string>

using namespace std;

typedef enum _personal_data_save_mode {
	SAVE_PERSONAL_DATA_ALWAYS_ASK,
	SAVE_PERSONAL_DATA_ON,
	SAVE_PERSONAL_DATA_OFF,
} personal_data_save_mode;

class Browser_Personal_Data_Manager : public Browser_Common_View {
public:
	Browser_Personal_Data_Manager(void);
	~Browser_Personal_Data_Manager(void);

	Eina_Bool init(void) {}

	void init(Evas_Object *webview);
	void deinit(void);

	Eina_Bool clear_personal_data(void);
	Eina_Bool set_personal_data(const char *url);
private:
	personal_data_save_mode _get_save_mode(void);
	Eina_Bool _save_personal_data(std::string user_name,
						std::string password, std::string url);
	int _show_ask_confirm_popup(void);

	/* Elementary event callback functions */
	static void __submit_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_webview;
	Browser_Personal_Data_DB *m_personal_data_db;
	Evas_Object *m_popup;
};

#endif /* BROWSER_PERSONAL_DATA_MANAGER_H */

