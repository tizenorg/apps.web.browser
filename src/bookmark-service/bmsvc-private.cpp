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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bmsvc-define.h"
#include "bmsvc-dlog.h"
#include "bmsvc-private.h"

extern sqlite3 *m_internet_bookmark_db;

/* Private Functions */
void __bookmark_db_close()
{
	if (m_internet_bookmark_db) {
		/* ASSERT(currentThread() == m_openingThread); */
		db_util_close(m_internet_bookmark_db);
		m_internet_bookmark_db = 0;
	}
}

void _internet_bookmark_db_close(sqlite3_stmt *stmt)
{
	if (sqlite3_finalize(stmt) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_finalize is failed");
	}
	__bookmark_db_close();
}

int _internet_bookmark_db_open()
{
	__bookmark_db_close();
	if (db_util_open
	    (INTERNET_BOOKMARK_DB_NAME, &m_internet_bookmark_db,
	     DB_UTIL_REGISTER_HOOK_METHOD) != SQLITE_OK) {
		db_util_close(m_internet_bookmark_db);
		m_internet_bookmark_db = 0;
		return -1;
	}
	return m_internet_bookmark_db ? 0 : -1;
}

int _internet_id_get_editable(int id)
{
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed");
		return -1;
	}
	/* same title  */
	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "select editable from bookmarks where id=?", -1,
			       &stmt, NULL);

	if (nError != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_prepare_v2 is failed.");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_ROW) {
		int type = sqlite3_column_int(stmt, 0);
		_internet_bookmark_db_close(stmt);
		return type;
	}
	__bookmark_db_close();
	return -1;
}

int _internet_id_get_type(int id)
{
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed");
		return -1;
	}
	/* same title  */
	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "select type from bookmarks where id=?", -1,
			       &stmt, NULL);

	if (nError != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_prepare_v2 is failed.");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_ROW) {
		int type = sqlite3_column_int(stmt, 0);
		_internet_bookmark_db_close(stmt);
		return type;
	}
	__bookmark_db_close();
	return -1;
}

int _internet_id_is_exist(int id, int type)
{
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed");
		return -1;
	}
	/* same title  */
	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "select * from bookmarks where id=? and type=?",
			       -1, &stmt, NULL);

	if (nError != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_prepare_v2 is failed.");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	if (sqlite3_bind_int(stmt, 2, type) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_ROW) {
		_internet_bookmark_db_close(stmt);
		return 1;
	}
	__bookmark_db_close();
	return 0;
}

/* return uniq id of item. */
int _internet_bookmark_is_exist(int folder_id, char *title, int type)
{
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed\n");
		return -1;
	}
	/* same title  */
	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "select id from bookmarks where parent=? and type=? and title=?",
			       -1, &stmt, NULL);

	if (nError != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_prepare_v2 is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	if (sqlite3_bind_int(stmt, 1, folder_id) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	if (sqlite3_bind_int(stmt, 2, type) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	if (sqlite3_bind_text(stmt, 3, title, -1, NULL) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_ROW) {
		int id = sqlite3_column_int(stmt, 0);
		_internet_bookmark_db_close(stmt);
		return id;
	}
	__bookmark_db_close();
	return 0;
}

/* search last number of sequence(order's index) */
int _internet_bookmark_get_lastindex(int locationId)
{
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed\n");
		return -1;
	}

	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "select sequence from bookmarks where parent=? order by sequence desc",
			       -1, &stmt, NULL);
	if (nError != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_prepare_v2 is failed");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	if (sqlite3_bind_int(stmt, 1, locationId) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	if ((nError = sqlite3_step(stmt)) == SQLITE_ROW) {
		int index = sqlite3_column_int(stmt, 0);
		_internet_bookmark_db_close(stmt);
		return index;
	}
	__BOOKMARK_DEBUG_TRACE("Not found items in This Folder");
	__bookmark_db_close();
	return 0;
}

int _internet_bookmark_move_to_root(int uid)
{
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed\n");
		return -1;
	}
	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "update bookmarks set parent=? where id =?", -1,
			       &stmt, NULL);
	if (nError != SQLITE_OK) {
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	if (sqlite3_bind_int(stmt, 1, internet_bookmark_get_root_id()) !=
	    SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
	}
	/* bookmarkId/folderId  */
	if (sqlite3_bind_int(stmt, 2, uid) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
	}
	nError = sqlite3_step(stmt);
	if (nError != SQLITE_OK && nError != SQLITE_DONE) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_step is failed");
		__bookmark_db_close();
		return -1;
	}
	return 0;
}

int _internet_sub_bookmarks_move_to_root(int uid)
{
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed\n");
		return -1;
	}
	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "update bookmarks set parent=? where parent=?",
			       -1, &stmt, NULL);
	if (nError != SQLITE_OK) {
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	if (sqlite3_bind_int(stmt, 1, internet_bookmark_get_root_id()) !=
	    SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
	}
	/* bookmarkId/folderId  */
	if (sqlite3_bind_int(stmt, 2, uid) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
	}
	nError = sqlite3_step(stmt);
	if (nError != SQLITE_OK && nError != SQLITE_DONE) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_step is failed");
		__bookmark_db_close();
		return -1;
	}
	return 0;
}

int _internet_bookmark_new(char *address, char *title, int folder_id,
			   int property_flag)
{
	int nError;
	sqlite3_stmt *stmt;

	int lastIndex = 0;
	if ((lastIndex = _internet_bookmark_get_lastindex(folder_id)) < 0) {
		__BOOKMARK_DEBUG_TRACE("Database::getLastIndex() is failed.\n");
		return -1;
	}

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed\n");
		return -1;
	}

	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "insert into bookmarks (type, parent, address, title, creationdate, updatedate, editable, sequence) values (?, ?, ?, ?, DATETIME('now'), DATETIME('now'), 1, ?)",
			       -1, &stmt, NULL);
	if (nError != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_prepare_v2 is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	/*type */
	if (sqlite3_bind_int(stmt, 1, property_flag) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	/*parent */
	if (sqlite3_bind_int(stmt, 2, folder_id) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	/*address */
	if (sqlite3_bind_text(stmt, 3, address, -1, NULL) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	/*title */
	if (sqlite3_bind_text(stmt, 4, title, -1, NULL) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	/* order */
	if (lastIndex >= 0) {
		if (sqlite3_bind_int(stmt, 5, (lastIndex + 1)) != SQLITE_OK) {
			__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.\n");
			_internet_bookmark_db_close(stmt);
			return -1;
		}
	}
	nError = sqlite3_step(stmt);
	if (nError == SQLITE_OK || nError == SQLITE_DONE) {
		_internet_bookmark_db_close(stmt);
		return 0;
	}
	__BOOKMARK_DEBUG_TRACE("sqlite3_step is failed");
	__bookmark_db_close();
	return -1;

}

int _internet_bookmark_update(int uid, char *address, char *title,
			      int folder_id)
{
	__BOOKMARK_DEBUG_TRACE("[%s][%s] folder_id:%d", address, title,
			       folder_id);
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed\n");
		return -1;
	}

	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "update bookmarks set title=?, address=?, parent=?, updatedate=DATETIME('now') where id=?",
			       -1, &stmt, NULL);
	if (nError != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_prepare_v2 is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	if (sqlite3_bind_text(stmt, 1, title, -1, NULL) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
	}
	if (sqlite3_bind_text(stmt, 2, address, -1, NULL) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
	}
	if (sqlite3_bind_int(stmt, 3, folder_id) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
	}
	/* bookmarkId or folderId */
	if (sqlite3_bind_int(stmt, 4, uid) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_OK || nError == SQLITE_DONE) {
		_internet_bookmark_db_close(stmt);
		return 0;
	}
	__BOOKMARK_DEBUG_TRACE("sqlite3_step is failed");
	__bookmark_db_close();
	return -1;

}

int _internet_bookmark_remove(int id)
{
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed\n");
		return -1;
	}
	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "delete from bookmarks where id=?", -1, &stmt,
			       NULL);
	if (nError != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_prepare_v2 is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_OK || nError == SQLITE_DONE) {
		_internet_bookmark_db_close(stmt);
		return 0;
	}
	__BOOKMARK_DEBUG_TRACE("sqlite3_step is failed");
	__bookmark_db_close();
	return -1;
}

int _internet_bookmark_reset()
{
	__BOOKMARK_DEBUG_TRACE(" ");

	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed\n");
		return -1;
	}

	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db, "delete from bookmarks",
			       -1, &stmt, NULL);
	if (nError != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_prepare_v2 is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	nError = sqlite3_step(stmt);
	if (nError == SQLITE_OK || nError == SQLITE_DONE) {
		_internet_bookmark_db_close(stmt);
		return 0;
	}
	__bookmark_db_close();
	return 0;
}

/* 1 : ok   0 : exist but can not use it    -1 : not found */
int _file_is_exist(char *filename)
{
	struct stat stat_info;
	if (filename == NULL || strlen(filename) <= 0)
		return -1;
	if (stat(filename, &stat_info) < 0)
		return -1;
	if (!S_ISREG(stat_info.st_mode))
		return 0;
	if (stat_info.st_size <= 0)
		return 0;
	return 1;
}

int _bmsvc_bookmark_is_exist(const char *address)
{
	TRACE_BEGIN;
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed");
		return -1;
	}
	/* same title  */
	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "select * from bookmarks where address=? and type=0",
			       -1, &stmt, NULL);

	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed.");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	if (sqlite3_bind_text(stmt, 1, address, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_ROW) {
		_internet_bookmark_db_close(stmt);
		return 1;
	}
	__bookmark_db_close();
	TRACE_END;
	return 0;
}

int _bmsvc_folder_is_exist(const char *title, int parent_id)
{
	TRACE_BEGIN;
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed");
		return -1;
	}
	/* same title  */
	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "select * from bookmarks where title=? and parent=? and type=1",
			       -1, &stmt, NULL);

	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed.");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	if (sqlite3_bind_text(stmt, 1, title, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	if (sqlite3_bind_int(stmt, 2, parent_id) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_ROW) {
		_internet_bookmark_db_close(stmt);
		return 1;
	}
	__bookmark_db_close();
	TRACE_END;
	return 0;
}

int _bmsvc_get_bookmark_id(const char *address)
{

	TRACE_BEGIN;
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed");
		return -1;
	}
	/* same title  */
	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "select id from bookmarks where address=? and type=0",
			       -1, &stmt, NULL);

	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed.");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	if (sqlite3_bind_text(stmt, 1, address, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_ROW) {
		int id = sqlite3_column_int(stmt, 0);
		_internet_bookmark_db_close(stmt);
		return id;
	}
	__bookmark_db_close();
	TRACE_END;
	return 0;
}

int _bmsvc_get_folder_id(const char *title, const int parent)
{

	TRACE_BEGIN;
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed");
		return -1;
	}
	/* same title  */
	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "SELECT id FROM bookmarks WHERE title=? AND parent=? AND type=1",
			       -1, &stmt, NULL);

	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed.");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	if (sqlite3_bind_text(stmt, 1, title, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	if (sqlite3_bind_int(stmt, 2, parent) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_ROW) {
		int id = sqlite3_column_int(stmt, 0);
		_internet_bookmark_db_close(stmt);
		return id;
	}
	__bookmark_db_close();
	TRACE_END;
	return 0;
}

int _bmsvc_tag_exists_at_bookmarks(const char *tag_name)
{
	TRACE_BEGIN;
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		TRACE_END;
		return -1;
	}
	/* same title  */
	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "select id from bookmarks where tag1=? OR tag2=? OR tag3=? OR tag4=?",
			       -1, &stmt, NULL);

	if (sqlite3_bind_text(stmt, 1, tag_name, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return -1;
	}
	if (sqlite3_bind_text(stmt, 2, tag_name, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return -1;
	}
	if (sqlite3_bind_text(stmt, 3, tag_name, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return -1;
	}
	if (sqlite3_bind_text(stmt, 4, tag_name, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return -1;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_ROW) {
		int id = sqlite3_column_int(stmt, 0);
		_internet_bookmark_db_close(stmt);
		DBG_LOGD("Tag name is exist in the bookmakrs(id:%d)\n", id);
		TRACE_END;
		return 1;
	}
	__bookmark_db_close();
	TRACE_END;
	return 0;
}

int _bmsvc_tag_exists_at_taglist(const char *tag_name)
{
	TRACE_BEGIN;
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		TRACE_END;
		return -1;
	}

	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "select tag from tags where tag=?",
			       -1, &stmt, NULL);

	if (sqlite3_bind_text(stmt, 1, tag_name, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return -1;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_ROW) {
		_internet_bookmark_db_close(stmt);
		DBG_LOGD("Tag name is exist in the taglist\n");
		TRACE_END;
		return 1;
	}
	__bookmark_db_close();
	TRACE_END;
	return 0;
}

int _bmsvc_add_tag_to_taglist(const char *tag_name)
{
	TRACE_BEGIN;
	int nError;
	sqlite3_stmt *stmt;

	if(_bmsvc_tag_exists_at_taglist(tag_name) > 0) {
		DBG_LOGE("tag is already exist\n");
		TRACE_END;
		return -1;
	}
	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		TRACE_END;
		return -1;
	}

	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "INSERT INTO tags (tag) VALUES (?)",
			       -1, &stmt, NULL);
	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return -1;
	}

	/*address */
	if (sqlite3_bind_text(stmt, 1, tag_name, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return -1;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_OK || nError == SQLITE_DONE) {
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return 0;
	}
	DBG_LOGE("sqlite3_step is failed");
	__bookmark_db_close();
	TRACE_END;
	return -1;
}

int _bmsvc_delete_tag_from_taglist(const char *tag_name)
{
	TRACE_BEGIN;
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		TRACE_END;
		return -1;
	}

	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "DELETE FROM tags WHERE tag=?",
			       -1, &stmt, NULL);
	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return -1;
	}

	/*address */
	if (sqlite3_bind_text(stmt, 1, tag_name, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return -1;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_OK || nError == SQLITE_DONE) {
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return 0;
	}
	DBG_LOGE("sqlite3_step is failed");
	__bookmark_db_close();
	TRACE_END;
	return -1;
}

void _bmsvc_destroy_bookmark_list(gpointer data)
{
	TRACE_BEGIN;
	bookmark_list *list = (bookmark_list *)data;

	int i = 0;
	if (list == NULL)
		return;

	if (list->item != NULL) {
		for (i = 0; i < list->count; i++) {
			if (list->item[i].address != NULL)
				free(list->item[i].address);
			if (list->item[i].title != NULL)
				free(list->item[i].title);
			if (list->item[i].creationdate != NULL)
				free(list->item[i].creationdate);
			if (list->item[i].updatedate != NULL)
				free(list->item[i].updatedate);
			if (list->item[i].tag1 != NULL)
				free(list->item[i].tag1);
			if (list->item[i].tag2 != NULL)
				free(list->item[i].tag2);
			if (list->item[i].tag3 != NULL)
				free(list->item[i].tag3);
			if (list->item[i].tag4 != NULL)
				free(list->item[i].tag4);
		}
		free(list->item);
	}
	free(list);
	list = NULL;
	TRACE_END;
}

void _bmsvc_destroy_bookmark_entry(gpointer data)
{
	bookmark_entry *entry = (bookmark_entry *)data;
	if (entry == NULL) {
		DBG_LOGD("entry is null");
		return;
	}
	DBG_LOGD("destroy entry : %s", entry->title);

	if (entry->address != NULL)
		free(entry->address);
	if (entry->title != NULL)
		free(entry->title);
	if (entry->creationdate != NULL)
		free(entry->creationdate);
	if (entry->updatedate != NULL)
		free(entry->updatedate);
	if (entry->tag1 != NULL)
		free(entry->tag1);
	if (entry->tag2 != NULL)
		free(entry->tag2);
	if (entry->tag3 != NULL)
		free(entry->tag3);
	if (entry->tag4 != NULL)
		free(entry->tag4);
	free(entry);
	entry = NULL;
}

int _bmsvc_copy_bookmark_entry_deep(bookmark_entry *dest, bookmark_entry *src)
{
	TRACE_BEGIN;
	if (!dest || !src) {
		DBG_LOGD("dest: %p, src:%p", dest, src);
		return BMSVC_ERROR_INVALID_PARAMETER;
	}

	if (src->address)
		dest->address = strdup(src->address);
	if (src->title)
		dest->title = strdup(src->title);
	if (src->creationdate)
		dest->creationdate = strdup(src->creationdate);
	if (src->updatedate)
		dest->updatedate = strdup(src->updatedate);

	dest->id = src->id;
	dest->is_folder = src->is_folder;
	dest->folder_id = src->folder_id;
	dest->orderIndex = src->orderIndex;
	dest->editable = src->editable;

	if (src->tag1)
		dest->tag1 = strdup(src->tag1);
	if (src->tag2)
		dest->tag2 = strdup(src->tag2);
	if (src->tag3)
		dest->tag3 = strdup(src->tag3);
	if (src->tag4)
		dest->tag4 = strdup(src->tag4);

	TRACE_END;
	return BMSVC_ERROR_NONE;
}

int _bmsvc_get_folder_subitems_recursive(int parent_id, GList **list)
{
	DBG_LOGD("parent_id: %d", parent_id);
	int i = 0;
	bookmark_entry *entry = NULL;

	if (list == NULL) {
		DBG_LOGD("list pointer is invalid");
		return BMSVC_ERROR_INVALID_PARAMETER;
	}

	bookmark_list *temp_list = internet_bookmark_list(parent_id, 2);
	if (temp_list && (temp_list->count > 0)){
		for (i = 0 ; i < temp_list->count ; i++) {
			entry = (bookmark_entry *) calloc(1, sizeof(bookmark_entry));
			_bmsvc_copy_bookmark_entry_deep(entry, (temp_list->item)+i);
			DBG_LOGD("Bookmark Title: %s", entry->title);
			*list = g_list_prepend(*list, (gpointer)entry);
			if (entry->is_folder) {
				/* Just in case, when the stack is currupted*/
				if (_bmsvc_get_folder_subitems_recursive(entry->id, list) 
							== BMSVC_ERROR_INVALID_PARAMETER) {
							_bmsvc_destroy_bookmark_list((gpointer)temp_list);
							_bmsvc_destroy_bookmark_entry((gpointer)entry);
							return BMSVC_ERROR_INVALID_PARAMETER;
				}
			}
		}
	}
	_bmsvc_destroy_bookmark_list((gpointer)temp_list);
	return BMSVC_ERROR_NONE;
}

int _bmsvc_delete_folder_subitems(int folder_id, int check_editable)
{
	TRACE_BEGIN;
	DBG_LOGD("folder_id: %d", folder_id);
	GList *list = NULL;
	GList *tmp_list = NULL;
	bookmark_entry *temp_entry = NULL;

	if (_bmsvc_get_folder_subitems_recursive(folder_id, &list)
				== BMSVC_ERROR_INVALID_PARAMETER) {
				TRACE_END;
				return BMSVC_ERROR_UNKNOWN;
	}
	// check if there are any uneditable item under this folder (include the folder itself)
	// if there is no uneditable item, erase this and all sub items.
	/* check the uneditable item */
	if (check_editable) {
		if (g_list_length(list) > 0) {
			tmp_list = list;
			while (tmp_list) {
				if (tmp_list->data) {
					temp_entry = (bookmark_entry *)tmp_list->data;
					if (temp_entry->editable == 0) {
						DBG_LOGD("Uneditable item is found [%s]", temp_entry->title);
						g_list_free_full(list, _bmsvc_destroy_bookmark_entry);
						TRACE_END;
						return BMSVC_ERROR_ITEM_IS_NOT_EDITABLE;
					}
				}
				tmp_list = g_list_next(tmp_list);
			}
		}
	}

	/* Delete all listed items */
	if (g_list_length(list) > 0) {
		tmp_list = list;
		while (tmp_list) {
			if (tmp_list->data) {
				temp_entry = (bookmark_entry *)tmp_list->data;
				DBG_LOGD("Title [%s]", temp_entry->title);
				_bmsvc_delete_bookmark_entry(temp_entry->id);
				_bmsvc_delete_bookmark_tags(temp_entry);
			}
			tmp_list = g_list_next(tmp_list);
		}
	}
	g_list_free_full(list, _bmsvc_destroy_bookmark_entry);
	TRACE_END;
	return BMSVC_ERROR_NONE;
}

int _bmsvc_delete_bookmark_entry(int id)
{
	TRACE_BEGIN;
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		return -1;
	}
	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "delete from bookmarks where id=?", -1, &stmt,
			       NULL);
	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}
	if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_OK || nError == SQLITE_DONE) {
		_internet_bookmark_db_close(stmt);
		return 0;
	}
	DBG_LOGE("sqlite3_step is failed");
	__bookmark_db_close();
	TRACE_END;
	return -1;
}

int _bmsvc_delete_bookmark_tags(bookmark_entry *entry)
{
#if defined(BROWSER_TAG)
	if (entry->tag1 && (strlen(entry->tag1) > 0)) {
		if (_bmsvc_tag_exists_at_bookmarks(entry->tag1) != 1) {
			/* There is no same tag at the bookmark list */
			if (_bmsvc_tag_exists_at_taglist(entry->tag1) == 1) {
				/*tag name is at the tags table. must be removed */
				_bmsvc_delete_tag_from_taglist(entry->tag1);
			}
		}
	}
	if (entry->tag2 && (strlen(entry->tag2) > 0)) {
		if (_bmsvc_tag_exists_at_bookmarks(entry->tag2) != 1) {
			/* There is no same tag at the bookmark list */
			if (_bmsvc_tag_exists_at_taglist(entry->tag2) == 1) {
				/*tag name is at the tags table. must be removed */
				_bmsvc_delete_tag_from_taglist(entry->tag2);
			}
		}
	}
	if (entry->tag3 && (strlen(entry->tag3) > 0)) {
		if (_bmsvc_tag_exists_at_bookmarks(entry->tag3) != 1) {
			/* There is no same tag at the bookmark list */
			if (_bmsvc_tag_exists_at_taglist(entry->tag3) == 1) {
				/*tag name is at the tags table. must be removed */
				_bmsvc_delete_tag_from_taglist(entry->tag3);
			}
		}
	}
	if (entry->tag4 && (strlen(entry->tag4) > 0)) {
		if (_bmsvc_tag_exists_at_bookmarks(entry->tag4) != 1) {
			/* There is no same tag at the bookmark list */
			if (_bmsvc_tag_exists_at_taglist(entry->tag4) == 1) {
				/*tag name is at the tags table. must be removed */
				_bmsvc_delete_tag_from_taglist(entry->tag4);
			}
		}
	}
#endif
	return BMSVC_ERROR_NONE;
}

int _bmsvc_update_bookmark_record(int id, const char *title, const char *address, int parent_id, int order,
						const char *tag1,
						const char *tag2,
						const char *tag3,
						const char *tag4)
{
	DBG_LOGD("id:%d, title: %s, parent_id: %d, order:%d", id, title, parent_id, order);
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		return BMSVC_ERROR_DB_FAILED;
	}
#if defined(BROWSER_TAG)
	nError =
		    sqlite3_prepare_v2(m_internet_bookmark_db,
				       "UPDATE bookmarks SET title=?, address=?, parent=?\
				       ,updatedate=DATETIME('now'), sequence=?\
				       ,tag1=?,tag2=?,tag3=?,tag4=?\
				       WHERE id=?",
				       -1, &stmt, NULL);
#else
	nError =
		    sqlite3_prepare_v2(m_internet_bookmark_db,
				       "UPDATE bookmarks SET title=?, address=?, parent=?\
				       ,updatedate=DATETIME('now'), sequence=?\
				       WHERE id=?",
				       -1, &stmt, NULL);
#endif
	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	if (sqlite3_bind_text(stmt, 1, title, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	if (sqlite3_bind_text(stmt, 2, address, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	if (sqlite3_bind_int(stmt, 3, parent_id) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	if (sqlite3_bind_int(stmt, 4, order) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
#if defined(BROWSER_TAG)
	/*tag1 */
	if (!tag1)
		tag1 = "";
	if (sqlite3_bind_text(stmt, 5, tag1, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	/*tag2 */
	if (!tag2)
		tag2 = "";
	if (sqlite3_bind_text(stmt, 6, tag2, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	/*tag3 */
	if (!tag3)
		tag3 = "";
	if (sqlite3_bind_text(stmt, 7, tag3, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	/*tag4 */
	if (!tag4)
		tag4 = "";
	if (sqlite3_bind_text(stmt, 8, tag4, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
#endif
	/* bookmarkId or folderId */
	if (sqlite3_bind_int(stmt, 5, id) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_OK || nError == SQLITE_DONE) {
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_NONE;
	}

	__bookmark_db_close();
	TRACE_END;
	return BMSVC_ERROR_UNKNOWN;
}

int _bmsvc_update_folder_record(int id, const char *title, int parent_id, int order)
{
	DBG_LOGD("id:%d, title: %s, parent_id: %d, order:%d", id, title, parent_id, order);
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		return BMSVC_ERROR_DB_FAILED;
	}

	nError =
		    sqlite3_prepare_v2(m_internet_bookmark_db,
				       "UPDATE bookmarks SET title=?, parent=?\
				       ,updatedate=DATETIME('now'), sequence=? \
				       WHERE id=?",
				       -1, &stmt, NULL);
	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	if (sqlite3_bind_text(stmt, 1, title, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	if (sqlite3_bind_int(stmt, 2, parent_id) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	/* bookmarkId or folderId */
	if (sqlite3_bind_int(stmt, 3, order) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	/* bookmarkId or folderId */
	if (sqlite3_bind_int(stmt, 4, id) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_OK || nError == SQLITE_DONE) {
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_NONE;
	}

	__bookmark_db_close();
	TRACE_END;
	return BMSVC_ERROR_UNKNOWN;
}