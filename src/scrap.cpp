/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.1 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "scrap.h"

#include <Elementary.h>
#include <fcntl.h>
#include <string>
#include <time.h>

#include "browser-dlog.h"

extern "C" {
#include "db-util.h"
}

#define scrap_db_path	"/opt/usr/apps/org.tizen.browser/data/db/.scrap.db"
#define mht_folder_path browser_data_dir"/mht/"

scrap_item::scrap_item(const char *title, const char *uri, const char *file_path, const char *date, const char *tag)
:
	m_title(NULL)
	,m_uri(NULL)
	,m_file_path(NULL)
	,m_date(NULL)
	,m_tag(NULL)
	,m_year(0)
	,m_month(0)
	,m_day(0)
{
	if (title)
		m_title = eina_stringshare_add(title);
	if (uri)
		m_uri = eina_stringshare_add(uri);
	if (file_path)
		m_file_path = eina_stringshare_add(file_path);
	if (date) {
		m_date = eina_stringshare_add(date);
		sscanf(date, "%d-%d-%d", &m_year, &m_month, &m_day);
	}
	if (tag)
		m_tag = eina_stringshare_add(tag);

}

scrap_item::~scrap_item(void)
{
	eina_stringshare_del(m_title);
	eina_stringshare_del(m_uri);
	eina_stringshare_del(m_file_path);
	eina_stringshare_del(m_date);
	eina_stringshare_del(m_tag);
}

Eina_Bool scrap_item::compare_date(const char *date)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(date, EINA_TRUE);
	int year, month, day;
	sscanf(date, "%d-%d-%d", &year, &month, &day);

	if (year != m_year || month != m_month || day != m_day)
		return EINA_TRUE;

	return EINA_FALSE;
}

time_stamp_type scrap_item::get_time_stamp_type(void)
{
	time_t current_time;
	struct tm *current_time_info;
	time(&current_time);
	current_time_info = localtime(&current_time);

	int current_year = current_time_info->tm_year;
	int current_yday = current_time_info->tm_yday;

	struct tm item_time_info;
	memset(&item_time_info, 0x00, sizeof(struct tm));
	item_time_info.tm_year = m_year - 1900;
	item_time_info.tm_mon = m_month - 1;
	item_time_info.tm_mday = m_day;
	time_t item_time = mktime(&item_time_info);
	struct tm *ptr_item_time = localtime(&item_time);

	int item_year = ptr_item_time->tm_year;
	int item_yday = ptr_item_time->tm_yday;

	BROWSER_LOGD("");

	if (current_year == item_year) {
		int day_gap = current_yday - item_yday;
		if (day_gap == 0)
			return SCRAP_TODAY;
		else if (day_gap == 1)
			return SCRAP_YESTERDAY;
		else if (day_gap <= 7)
			return SCRAP_LAST_7_DAYS;
		else if (day_gap <= 30)
			return SCRAP_LAST_MONTH;
		else
			return SCRAP_OLDER;
	}
	return SCRAP_OLDER;
}

void scrap_item::set_tag(const char *tag)
{
	eina_stringshare_replace(&m_tag, tag);
}

scrap::scrap(void)
{
	BROWSER_LOGD("");
}

scrap::~scrap(void)
{
	BROWSER_LOGD("");
}

char *scrap::save(const char *title, const char *uri, const char *mht_content, const char *tag, Eina_Bool keep_it)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(title, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, NULL);
	EINA_SAFETY_ON_NULL_RETURN_VAL(mht_content, NULL);

	time_t raw_time;
	struct tm *time_info;
	time(&raw_time);
	time_info = localtime(&raw_time);

	char file_name[30] = {0, };
	sprintf(file_name, "%04d%02d%02d%02d%02d%02d.mht", time_info->tm_year + 1900, time_info->tm_mon + 1, time_info->tm_mday, time_info->tm_hour, time_info->tm_min, time_info->tm_sec);

	std::string mht_path = std::string(mht_folder_path) + std::string(file_name);
	BROWSER_LOGD("mht_path=[%s]", mht_path.c_str());

	FILE *fd = fopen(mht_path.c_str(), "w");
	if (!fd) {
		BROWSER_LOGE("fopen failed");
		return NULL;
	}
	fwrite(mht_content, 1, strlen(mht_content), fd);
	fclose(fd);

	std::string return_file_path = mht_path;
	mht_path = std::string(file_scheme) + mht_path;

	_save_scrap(title, uri, mht_path.c_str(), tag);
	return strdup(return_file_path.c_str());
}

void scrap::delete_scrap(const char *file_path)
{
	BROWSER_LOGD("file_path=[%s]", file_path);

	EINA_SAFETY_ON_NULL_RETURN(file_path);
	if (!strncmp(file_path, file_scheme, strlen(file_scheme))) {
		unlink(file_path + strlen(file_scheme));
	}
	sqlite3 *descriptor = NULL;
	int error = db_util_open(scrap_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "delete from scrap where file_path=?", -1, &sqlite3_stmt, NULL);
	if (sqlite3_bind_text(sqlite3_stmt, 1, file_path, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.");

	if (sqlite3_step(sqlite3_stmt) != SQLITE_ROW)
		BROWSER_LOGE("sqlite3_step failed");

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		BROWSER_LOGE("sqlite3_finalize failed");
		return;
	}

	db_util_close(descriptor);
}

void scrap::_save_scrap(const char *title, const char *uri, const char *file_path, const char *tag)
{
	EINA_SAFETY_ON_NULL_RETURN(title);
	EINA_SAFETY_ON_NULL_RETURN(uri);
	EINA_SAFETY_ON_NULL_RETURN(file_path);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(scrap_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;

	if (!tag)
		error = sqlite3_prepare_v2(descriptor, "insert into scrap (title, uri, file_path, updatedate) values(?, ?, ?, DATETIME('now'))", -1, &sqlite3_stmt, NULL);
	else
		error = sqlite3_prepare_v2(descriptor, "insert into scrap (title, uri, file_path, updatedate, tag) values(?, ?, ?, DATETIME('now'), ?)", -1, &sqlite3_stmt, NULL);

	if (sqlite3_bind_text(sqlite3_stmt, 1, title, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 2, uri, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 3, file_path, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return;
	}

	if (tag) {
		if (sqlite3_bind_text(sqlite3_stmt, 4, tag, -1, NULL) != SQLITE_OK) {
			BROWSER_LOGD("SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			db_util_close(descriptor);
			return;
		}
	}

	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		return;
	}
	db_util_close(descriptor);
}

std::vector<scrap_item *> scrap::get_scrap_list(void)
{
	std::vector<scrap_item *> scrap_list;
	sqlite3 *descriptor = NULL;
	int error = db_util_open(scrap_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return scrap_list;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select title, uri, file_path, updatedate, tag from scrap order by updatedate desc", -1, &sqlite3_stmt, NULL);
	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		const char *title = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 0));
		const char *uri = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 1));
		const char *file_path = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 2));
		const char *date = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 3));
		const char *tag = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 4));
		BROWSER_LOGD("title=[%s], uri=[%s], file_path=[%s], date=[%s], tag=[%s]", title, uri, file_path, date, tag);

		scrap_item *item = new scrap_item(title, uri, file_path, date, tag);
		scrap_list.push_back(item);
	}
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	db_util_close(descriptor);

	return scrap_list;
}

std::vector<char *> scrap::get_tag_list(scrap_item *item)
{
	std::vector<char *> tag_list;
	sqlite3 *descriptor = NULL;
	int error = db_util_open(scrap_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return tag_list;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;

	if (!item) {
		error = sqlite3_prepare_v2(descriptor, "select tag from scrap order by tag desc", -1, &sqlite3_stmt, NULL);
	} else {
		error = sqlite3_prepare_v2(descriptor, "select tag from scrap where file_path=?", -1, &sqlite3_stmt, NULL);
		error = sqlite3_bind_text(sqlite3_stmt, 1, item->get_saved_file_path(), -1, NULL);
		if (error != SQLITE_OK) {
			BROWSER_LOGD("SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			db_util_close(descriptor);
			return tag_list;
		}
	}
	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		const char *tag = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 0));
		BROWSER_LOGD("tag=[%s]", tag);
		if (tag && strlen(tag)) {
			char *tmp_tag = strdup(tag);
			char *delimeter = strtok(tmp_tag, ",");

			while (delimeter) {
				BROWSER_LOGD("delimeter=[%s]", delimeter);
				if (tag_list.size() == 0) {
					tag_list.push_back(strdup(delimeter));
				} else {
					Eina_Bool dupliated = EINA_FALSE;
					for (int i = 0 ; i < tag_list.size() ; i++) {
						if (!strcmp(tag_list[i], delimeter)) {
							dupliated = EINA_TRUE;
							break;
						}
					}

					if (!dupliated)
						tag_list.push_back(strdup(delimeter));
				}

				delimeter = strtok(NULL, ",");
			}

			if (tmp_tag)
				free(tmp_tag);
		}
	}
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	db_util_close(descriptor);

	return tag_list;

}

std::vector<char *> scrap::extract_tag_list(const char *tag)
{
	BROWSER_LOGD("tag=[%s]", tag);

	std::vector<char *> tag_list;

	if (!tag)
		return tag_list;

	char *tmp_tag = strdup(tag);
	char *delimeter = strtok(tmp_tag, ",");

	while (delimeter) {
		BROWSER_LOGD("delimeter=[%s]", delimeter);
		if (tag_list.size() == 0) {
			tag_list.push_back(strdup(delimeter));
		} else {
			Eina_Bool dupliated = EINA_FALSE;
			for (int i = 0 ; i < tag_list.size() ; i++) {
				if (!strcmp(tag_list[i], delimeter)) {
					dupliated = EINA_TRUE;
					break;
				}
			}

			if (!dupliated)
				tag_list.push_back(strdup(delimeter));
		}

		delimeter = strtok(NULL, ",");
	}

	if (tmp_tag)
		free(tmp_tag);

	return tag_list;
}

static void _update_tag(int scrap_id, const char *tag)
{
	BROWSER_LOGD("scrap_id=[%d], tag=[%s]", scrap_id, tag);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(scrap_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "update scrap set tag=? where id=?", -1, &sqlite3_stmt, NULL);
	if (sqlite3_bind_text(sqlite3_stmt, 1, tag, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 2, scrap_id) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		return;
	}
	db_util_close(descriptor);
}


void scrap::update_tag(scrap_item *item, const char *tag)
{
	BROWSER_LOGD("tag=[%s]", tag);
	EINA_SAFETY_ON_NULL_RETURN(item);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(scrap_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "update scrap set tag=? where file_path=?", -1, &sqlite3_stmt, NULL);
	if (sqlite3_bind_text(sqlite3_stmt, 1, tag, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return;
	}
	BROWSER_LOGD("item->get_saved_file_path()=[%s]", item->get_saved_file_path());
	if (sqlite3_bind_text(sqlite3_stmt, 2, item->get_saved_file_path(), -1, NULL) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return;
	}
	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		return;
	}
	db_util_close(descriptor);
}

void scrap::delete_tag(const char *tag)
{
	EINA_SAFETY_ON_NULL_RETURN(tag);
	sqlite3 *descriptor = NULL;
	int error = db_util_open(scrap_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select id, tag from scrap", -1, &sqlite3_stmt, NULL);
	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		int id = sqlite3_column_int(sqlite3_stmt, 0);
		const char *tag_str = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 1));
		BROWSER_LOGD("id=[%d], tag_str=[%s]", id, tag_str);
		std::vector<char *> tag_list = extract_tag_list(tag_str);
		for (int i = 0 ; i < tag_list.size() ; i++) {
			if (!strcmp(tag, tag_list[i])) {
				free(tag_list[i]);
				tag_list.erase(tag_list.begin() + i);
				break;
			}
		}

		std::string update_tag;
		if (tag_list.size() != 0)
			update_tag = std::string(tag_list[0]);

		for (int i = 1 ; tag_list.size() ; i++)
			update_tag = update_tag + "," + std::string(tag_list[i]);

		_update_tag(id, update_tag.c_str());

		for (int i = 0 ; i < tag_list.size() ; i++)
			free(tag_list[i]);
	}
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	db_util_close(descriptor);
}

