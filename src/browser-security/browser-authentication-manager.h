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

