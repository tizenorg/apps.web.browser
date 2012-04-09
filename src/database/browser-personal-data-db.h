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
