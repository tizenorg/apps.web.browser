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

#ifndef BROWSER_USER_AGENT_DB_H
#define BROWSER_USER_AGENT_DB_H

#include "browser-config.h"

extern "C" {
#include "db-util.h"
}

#include <iostream>
#include <string>
#include <vector>

class Browser_User_Agent_DB {
public:
	Browser_User_Agent_DB();
	~Browser_User_Agent_DB();

	Eina_Bool get_user_agent(const char *title, char *&user_agent);
private:
	Eina_Bool _open_db(void);
	Eina_Bool _close_db(void);

	static sqlite3* m_db_descriptor;
};

#endif	/* BROWSER_USER_AGENT_DB_H */

