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

using namespace std;

#include "browser-notification-db.h"

sqlite3* Browser_Notification_DB::m_db_descriptor = NULL;

Browser_Notification_DB::Browser_Notification_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Notification_DB::~Browser_Notification_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Eina_Bool Browser_Notification_DB::_open_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	int error = db_util_open(BROWSER_NOTIFICATION_DB_PATH, &m_db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		BROWSER_LOGE("db_util_open error");
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool Browser_Notification_DB::_close_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_db_descriptor)
	{
		int error = db_util_close(m_db_descriptor);
		if (error != SQLITE_OK) {
			BROWSER_LOGE("db_util_close error");
			m_db_descriptor = NULL;
			return EINA_FALSE;
		}
		m_db_descriptor = NULL;
	}

	return EINA_TRUE;
}

Eina_Bool Browser_Notification_DB::save_domain(const char *domain)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!domain || strlen(domain) == 0)
		return EINA_FALSE;

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor,
			"insert into notification_permitted_domains (domain) values (?)", -1, &stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_text(stmt, 1, domain, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");
	error = sqlite3_step(stmt);

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	_close_db();

	return (error == SQLITE_DONE);
}

Eina_Bool Browser_Notification_DB::has_domain(const char *domain)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!domain || strlen(domain) == 0)
		return EINA_FALSE;

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor,
			"select count(*) from notification_permitted_domains where domain=?", -1, &stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_text(stmt, 1, domain, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");
	error = sqlite3_step(stmt);
	if (error != SQLITE_ROW) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}
	if (0 < (sqlite3_column_int(stmt, 0))) {
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_TRUE;
	}

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	_close_db();

	return EINA_FALSE;
}

Eina_Bool Browser_Notification_DB::save_notification(Ewk_Notification *ewk_notification, int &noti_id)
{
	if (!ewk_notification) {
		BROWSER_LOGE("ewk_notification null");
		return EINA_FALSE;
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor,
			"insert into notification_table (notification, title, body, url, iconURL, iconValidity) values (?, ?, ?, ?, ?, ?)",
			-1, &stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_int(stmt, 1, (int)ewk_notification->notification) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");
	if (sqlite3_bind_text(stmt, 2, ewk_notification->title, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");
	if (sqlite3_bind_text(stmt, 3, ewk_notification->body, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");
	if (sqlite3_bind_text(stmt, 4, ewk_notification->url, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");
	if (sqlite3_bind_text(stmt, 5, ewk_notification->iconURL, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");
	if (sqlite3_bind_int(stmt, 6, (int)0) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");

	error = sqlite3_step(stmt);
	noti_id = sqlite3_last_insert_rowid(m_db_descriptor);
	if (sqlite3_finalize(stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	_close_db();

	if (error != SQLITE_DONE)
		return EINA_FALSE;

	return EINA_TRUE;
}

Eina_Bool Browser_Notification_DB::get_title_by_id(int id, std::string &title)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor,
			"select title from notification_table where id=?", -1, &stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");
	error = sqlite3_step(stmt);
	if (error != SQLITE_ROW) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	title = std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));


	if (sqlite3_finalize(stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	_close_db();

	return EINA_TRUE;
}

Eina_Bool Browser_Notification_DB::get_body_by_id(int id, std::string &body)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor,
			"select body from notification_table where id=?", -1, &stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");
	error = sqlite3_step(stmt);
	if (error != SQLITE_ROW) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	body = std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
	if (sqlite3_finalize(stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	_close_db();

	return EINA_TRUE;
}

Eina_Bool Browser_Notification_DB::update_icon_validity(int noti_id)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor,
				"update notification_table set iconValidity=? where id=?",
				-1, &stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_int(stmt, 1, (int)1) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");
	if (sqlite3_bind_int(stmt, 2, noti_id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");

	error = sqlite3_step(stmt);

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	_close_db();

	return EINA_TRUE;
}

Eina_Bool Browser_Notification_DB::delete_notifications(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "delete from notification_table",
					-1, &stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("\nSQL error: %d", error);
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}
	error = sqlite3_step(stmt);

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	_close_db();

	return EINA_TRUE;
}
