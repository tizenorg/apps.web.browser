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

#ifndef BROWSER_BOOKMARK_DB_H
#define BROWSER_BOOKMARK_DB_H

#include "browser-config.h"

extern "C" {
#include "db-util.h"
}

#include <iostream>
#include <string>
#include <vector>

class Browser_Bookmark_DB {
public:
	struct bookmark_item {
		int id;
		std::string url;
		std::string title;
		Eina_Bool is_folder;
		int parent;
		int order_index;
		Eina_Bool is_editable;
		Eina_Bool is_delete;
		void *user_data_1;
		void *user_data_2;
		int user_data_3;
	};

	struct bookmark_item_type {
		int id;
		bool is_folder;
	};

	Browser_Bookmark_DB();
	~Browser_Bookmark_DB();

	Eina_Bool get_bookmark_list(std::vector<bookmark_item *> &list, int folder_id);
	Eina_Bool get_folder_list(std::vector<bookmark_item *> &list);
	Eina_Bool get_folder_id(int current_folder_id, const char *folder_name, int *folder_id);
	std::string get_folder_name_by_id(const int folder_id);
	Eina_Bool get_bookmark_id_by_title_url(int folder_id, const char *title, const char *url, int *bookmark_id);
	bookmark_item *get_bookmark_item_by_id(int bookmark_id);
	Eina_Bool get_untitled_folder_count(int *count);
	Eina_Bool is_duplicated(int folder_id, const char *title, const char *url);
	Eina_Bool is_duplicated(const char *folder_name);
	Eina_Bool is_full(void);
	Eina_Bool save_bookmark(int folder_id, const char *title, const char *url);
	Eina_Bool save_folder(const char *folder_name);
	Eina_Bool delete_bookmark(int bookmark_id);
	Eina_Bool delete_folder(int folder_id);
	Eina_Bool modify_bookmark_title(int bookmark_id, const char *title);
	Eina_Bool modify_bookmark(int bookmark_id, int folder_id, const char *title, const char *url);
	Eina_Bool modify_bookmark_order_index(int bookmark_id, int folder_id, int order_index);	
private:
	Eina_Bool _open_db(void);
	Eina_Bool _close_db(void);
	Eina_Bool _get_last_index(int folder_id, int *last_index);
	
	static sqlite3* m_db_descriptor;
};

#endif	/* BROWSER_BOOKMARK_DB_H */

