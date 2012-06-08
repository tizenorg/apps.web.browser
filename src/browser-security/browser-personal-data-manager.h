/*
  * Copyright 2012  Samsung Electronics Co., Ltd
  *
  * Licensed under the Flora License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *    http://www.tizenopensource.org/license
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
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

