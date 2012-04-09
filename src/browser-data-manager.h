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

#ifndef BROWSER_DATA_MANAGER_H
#define BROWSER_DATA_MANAGER_H

using namespace std;

#include "browser-config.h"

typedef enum _view_stack_status {
	BR_BROWSER_VIEW = 1,
	BR_BOOKMARK_VIEW = 1 << 1,
	BR_ADD_TO_BOOKMARK_VIEW = 1 << 2,
	BR_EDIT_BOOKMARK_VIEW = 1 << 3,
	BR_NEW_FOLDER_VIEW = 1 << 4,
	BR_SELECT_FOLDER_VIEW = 1 << 5,
	BR_MULTI_WINDOW_VIEW = 1 << 6,
	ADD_TO_MOST_VISITED_SITES_VIEW = 1 << 7
} view_stack_status;

class Browser_Add_To_Bookmark_View;
class Browser_Bookmark_DB;
class Browser_Bookmark_View;
class Browser_History_DB;
class Browser_History_Layout;
class Browser_Multi_Window_View;
class Browser_New_Folder_View;
class Browser_Select_Folder_View;
class Browser_View;

class Browser_Data_Manager {
public:
	Browser_Data_Manager(void);
	~Browser_Data_Manager(void);

	/* Must be.
	  * When create some view, it should be created by create_xxx method of Browser_Data_Manager.
	  * When leave that view (using elm_navigationbar_pop), the destroy_xxx method also called.
	  * The create_xxx, destroy_xxx methods control the view stack.
	  */
	Eina_Bool is_in_view_stack(view_stack_status view);

	Browser_View *get_browser_view(void) { return m_browser_view; }
	void set_browser_view(Browser_View *browser_view);

	Browser_Bookmark_View *get_bookmark_view(void) { return m_bookmark_view; }
	Browser_Bookmark_View *create_bookmark_view(void);
	void destroy_bookmark_view(void);

	Browser_Bookmark_DB *get_bookmark_db(void) { return m_bookmark_db; }
	Browser_Bookmark_DB *create_bookmark_db(void);
	void destroy_bookmark_db(void);

	Browser_Add_To_Bookmark_View *get_add_to_bookmark_view(void) { return m_add_to_bookmark_view; }
	Browser_Add_To_Bookmark_View *create_add_to_bookmark_view(string title,
								string url, int current_folder_id = BROWSER_BOOKMARK_MAIN_FOLDER_ID);
	void destroy_add_to_bookmark_view(void);

	Browser_Add_To_Bookmark_View *get_edit_bookmark_view(void) { return m_edit_bookmark_view; }
	Browser_Add_To_Bookmark_View *create_edit_bookmark_view(string title,
								string url, int current_folder_id = BROWSER_BOOKMARK_MAIN_FOLDER_ID);
	void destroy_edit_bookmark_view(void);

	Browser_New_Folder_View *get_new_folder_view(void) { return m_new_folder_view; }
	Browser_New_Folder_View *create_new_folder_view(void);
	void destroy_new_folder_view(void);

	Browser_Select_Folder_View *get_select_folder_view(void) { return m_select_folder_view; }
	Browser_Select_Folder_View *create_select_folder_view(int current_folder_id);
	void destroy_select_folder_view(void);

	Browser_Multi_Window_View *get_multi_window_view(void) { return m_multi_window_view; }
	Browser_Multi_Window_View *create_multi_window_view(void);
	void destroy_multi_window_view(void);

	Browser_History_Layout *get_history_layout(void) { return m_history_layout; }
	Browser_History_Layout *create_history_layout(void);
	void destroy_history_layout(void);

	Browser_History_DB *get_history_db(void) { return m_history_db; }
	Browser_History_DB *create_history_db(void);
	void destroy_history_db(void);
private:
	Browser_View *m_browser_view;
	Browser_Bookmark_View *m_bookmark_view;
	Browser_Add_To_Bookmark_View *m_add_to_bookmark_view;
	Browser_Add_To_Bookmark_View *m_edit_bookmark_view;
	Browser_New_Folder_View *m_new_folder_view;
	Browser_Select_Folder_View *m_select_folder_view;
	Browser_History_Layout *m_history_layout;
	Browser_Multi_Window_View *m_multi_window_view;

	Browser_Bookmark_DB *m_bookmark_db;
	Browser_History_DB *m_history_db;

	unsigned int m_stack_status;
};

#endif /* BROWSER_DATA_MANAGER_H */

