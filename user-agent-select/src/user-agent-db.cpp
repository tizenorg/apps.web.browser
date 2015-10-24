/*
 *  ug-browser-user-agent-efl
 *
 * Copyright (c) 2012 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *              Sangpyo Kim <sangpyo7.kim@samsung.com>
 *              Junghwan Kang <junghwan.kang@samsung.com>
 *              Inbum Chang <ibchang@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <dlog.h>
#include "user-agent-db.h"
#include "browser-object.h"

using namespace std;

#define DATABASE_PATH   browser_data_dir"/db/.browser.db"

User_Agent_DB::User_Agent_DB(void)
:	m_db_descriptor(NULL)
{
	SLOGE("[%s]", __func__);
}

User_Agent_DB::~User_Agent_DB(void)
{
	SLOGE("[%s]", __func__);
}

Eina_Bool User_Agent_DB::_open_db(void)
{
	_close_db();
	int error = db_util_open(DATABASE_PATH, &m_db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool User_Agent_DB::_close_db()
{
	if (m_db_descriptor)
	{
		int error = db_util_close(m_db_descriptor);
		if (error != SQLITE_OK) {
			SLOGE("db_util_close error");
			m_db_descriptor = NULL;
			return EINA_FALSE;
		}
		m_db_descriptor = NULL;
	}

	return EINA_TRUE;
}

Eina_Bool User_Agent_DB::get_user_agent_list(std::map<std::string, std::string>& list)
{
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select name, value from user_agents order by name",
										-1, &stmt, NULL);
	if (error != SQLITE_OK) {
		SLOGE("SQL error=%d", error);
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			SLOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	while (1) {
		error = sqlite3_step(stmt);
		if(error == SQLITE_ROW) {
			std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
			std::string value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
			list.insert(pair<std::string, std::string>(title, value));
		}
		else
			break;
	}

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		SLOGE("sqlite3_finalize is failed. (%d)\n", error);

	_close_db();

	return (error == SQLITE_DONE);
}

Eina_Bool User_Agent_DB::get_user_agent_title(std::string value, std::string &title)
{
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select name from user_agents where value=?",
										-1, &stmt, NULL);
	if (error != SQLITE_OK) {
		SLOGE("SQL error=%d", error);
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			SLOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	error = sqlite3_bind_text(stmt, 1, value.c_str(), -1, NULL);
	if (error != SQLITE_OK) {
		SLOGE("SQL error=%d", error);
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			SLOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	error = sqlite3_step(stmt);
	if (error == SQLITE_ROW)
		title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		SLOGE("sqlite3_finalize is failed. (%d)\n", error);

	_close_db();

	return (error == SQLITE_DONE);
}

