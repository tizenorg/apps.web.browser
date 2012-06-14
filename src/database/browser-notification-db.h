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

#ifndef BROWSER_NOTIFICATION_DB_H
#define BROWSER_NOTIFICATION_DB_H

#include "browser-config.h"

extern "C" {
#include "db-util.h"
}

#include <iostream>
#include <string>
#include <vector>

class Browser_Notification_DB {
public:
	Browser_Notification_DB(void);
	~Browser_Notification_DB(void);

	Eina_Bool save_domain(const char *domain);
	Eina_Bool has_domain(const char *domain);
	Eina_Bool save_notification(Ewk_Notification *ewk_notification, int &noti_id);
	Eina_Bool get_title_by_id(int id, std::string &title);
	Eina_Bool get_body_by_id(int id, std::string &body);
	Eina_Bool update_icon_validity(int noti_id);
	Eina_Bool delete_notifications(void);
private:
	Eina_Bool _open_db(void);
	Eina_Bool _close_db(void);

	static sqlite3* m_db_descriptor;
};

#endif	/* BROWSER_NOTIFICATION_DB_H */

