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

