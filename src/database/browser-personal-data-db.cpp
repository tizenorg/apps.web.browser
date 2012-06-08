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

using namespace std;

#include "browser-personal-data-db.h"

sqlite3* Browser_Personal_Data_DB::m_db_descriptor = NULL;

Browser_Personal_Data_DB::Browser_Personal_Data_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Personal_Data_DB::~Browser_Personal_Data_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Eina_Bool Browser_Personal_Data_DB::_open_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	int error = db_util_open(BROWSER_PERSONAL_DATA_DB_PATH, &m_db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool Browser_Personal_Data_DB::_close_db(void)
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

Eina_Bool Browser_Personal_Data_DB::save_personal_data(std::string url, std::string login, std::string password)
{
	if (url.empty() || login.empty()) {
		BROWSER_LOGE("url or value is empty");
		return EINA_FALSE;
	}

	BROWSER_LOGD("url=[%s], login=[%s], password=[%s]", url.c_str(), login.c_str(), password.c_str());

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select id, login, password from passwords where address=?",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 1, url.c_str(), -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");

	std::string current_login;
	std::string current_password;

	int id = 0;
	error = sqlite3_step(sqlite3_stmt);
	if (error == SQLITE_ROW) {
		id = sqlite3_column_int(sqlite3_stmt, 0);
		current_login = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 1));
		current_password = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 2));
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	if (id == 0) {
		/* url doesn't exist */
		error = sqlite3_prepare_v2(m_db_descriptor, "insert into passwords(login, password, address) values(?,?,?)",
						-1, &sqlite3_stmt, NULL);
		if (error != SQLITE_OK) {
			BROWSER_LOGD("SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}

		if (sqlite3_bind_text(sqlite3_stmt, 1, login.c_str(), -1, NULL) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_bind_text is failed.\n");
		if (sqlite3_bind_text(sqlite3_stmt, 2, password.c_str(), -1, NULL) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_bind_text is failed.\n");
		if (sqlite3_bind_text(sqlite3_stmt, 3, url.c_str(), -1, NULL) != SQLITE_OK)

		error = sqlite3_step(sqlite3_stmt);
		if (error != SQLITE_DONE) {
			BROWSER_LOGD("SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}
	} else {
		/* url exists, update username and password */
		if (strcmp(login.c_str(), current_login.c_str()) || strcmp(password.c_str(), current_password.c_str())) {
			error = sqlite3_prepare_v2(m_db_descriptor, "update passwords set login=?, password=? where id=?",
							-1, &sqlite3_stmt, NULL);
			if (error != SQLITE_OK) {
				BROWSER_LOGD("SQL error=%d", error);
				if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
					BROWSER_LOGE("sqlite3_finalize is failed.\n");
				_close_db();
				return EINA_FALSE;
			}

			if (sqlite3_bind_text(sqlite3_stmt, 1, login.c_str(), -1, NULL) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_bind_text is failed.\n");
			if (sqlite3_bind_text(sqlite3_stmt, 2, password.c_str(), -1, NULL) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_bind_text is failed.\n");
			if (sqlite3_bind_int(sqlite3_stmt, 3, id) != SQLITE_OK)
			
			error = sqlite3_step(sqlite3_stmt);
			if (error != SQLITE_DONE) {
				BROWSER_LOGD("SQL error=%d", error);
				if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
					BROWSER_LOGE("sqlite3_finalize is failed.\n");
				_close_db();
				return EINA_FALSE;
			}
		}
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		_close_db();
		return EINA_FALSE;
	}

	_close_db();

	return EINA_TRUE;
}

Eina_Bool Browser_Personal_Data_DB::get_personal_data(std::string &login, std::string &password, const std::string &url)
{
	BROWSER_LOGD("url=[%s]", url.c_str());
	if (url.empty()) {
		BROWSER_LOGE("url is empty");
		return EINA_FALSE;
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select login, password from passwords where address=?",
					-1, &stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(stmt, 1, url.c_str(), -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");

	Eina_Bool personal_data_found = EINA_FALSE;
	error = sqlite3_step(stmt);
	if (error == SQLITE_ROW) {
		login = std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
		password = std::string(reinterpret_cast<const char *> (sqlite3_column_text(stmt, 1)));
		personal_data_found = EINA_TRUE;
		BROWSER_LOGD("Personal data: %s : %s\n", login.c_str(), password.c_str());
	}


	error = sqlite3_finalize(stmt);

	_close_db();

	return personal_data_found;
}

Eina_Bool Browser_Personal_Data_DB::clear_personal_data(void)
{
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "delete from passwords", -1, &sqlite3_stmt ,NULL);

	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (error != SQLITE_DONE) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	_close_db();

	return EINA_TRUE;
}

