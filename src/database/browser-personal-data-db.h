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

#ifndef BROWSER_PERSONAL_DATA_DB
#define BROWSER_PERSONAL_DATA_DB

#include "browser-config.h"

extern "C" {
#include "db-util.h"
}

#include <string>
#include <vector>

using namespace std;

class Browser_Personal_Data_DB {
public:
	struct personal_data {
		std::string identifier;
		std::string name;
		std::string type;
		std::string value;
		std::string form_id;
		std::string form_name;
	};

	Browser_Personal_Data_DB(void);
	~Browser_Personal_Data_DB(void);

	Eina_Bool save_personal_data(std::string url, std::string login, std::string password);
	Eina_Bool get_personal_data(std::string &login, std::string &password, const std::string &url);
	Eina_Bool clear_personal_data(void);

private:
	Eina_Bool _open_db(void);
	Eina_Bool _close_db(void);

	static sqlite3* m_db_descriptor;
};

#endif	/* BROWSER_PERSONAL_DATA_DB */
