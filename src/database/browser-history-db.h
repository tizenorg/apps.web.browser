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

#ifndef BROWSER_HISTORY_DB_H
#define BROWSER_HISTORY_DB_H

#include "browser-config.h"

extern "C" {
#include "db-util.h"
}

#include <iostream>
#include <string>
#include <vector>

class Browser_History_DB {
public:
	struct history_item {
		int id;
		std::string url;
		std::string title;
		std::string date;
		Eina_Bool is_delete;

		void *user_data;
	};
	struct most_visited_item {
		std::string url;
		std::string title;
	};

	Browser_History_DB();
	~Browser_History_DB();

	Eina_Bool get_history_list(std::vector<history_item*> &list);
	Eina_Bool get_history_list_by_partial_url(const char *url, int count, std::vector<std::string> &list);
	Eina_Bool get_most_visited_list(std::vector<most_visited_item> &list);
	Eina_Bool save_history(const char *url, const char *title, Eina_Bool *is_full);
	Eina_Bool delete_history(int history_id);
	Eina_Bool delete_history(const char *url);
	Eina_Bool clear_history(void);
private:
	Eina_Bool _open_db(void);
	Eina_Bool _close_db(void);
	
	static sqlite3* m_db_descriptor;
};

#endif	/* BROWSER_HISTORY_DB_H */

