/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.0 (the License);
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
#include <db-util.h>

#include "bmsvc-define.h"
#include "bmsvc-dlog.h"
#include "bmsvc-private.h"
#include "bookmark-service.h"

extern sqlite3 *m_internet_bookmark_db;

bookmark_entry * bmsvc_get_bookmark_by_id(int id)
{
	TRACE_BEGIN;
	DBG_LOGD("id :%d", id);

	bookmark_entry *entry = NULL;
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		return NULL;
	}
#if defined(BROWSER_TAG)
	nError = sqlite3_prepare_v2(m_internet_bookmark_db,
					"SELECT\
					type, parent, address, title, sequence,\
					editable, strftime('%s',creationdate), updatedate, tag1, tag2,\
					tag3, tag4\
					FROM bookmarks\
					WHERE id=?",
					-1, &stmt, NULL);
#else
	nError = sqlite3_prepare_v2(m_internet_bookmark_db,
					"SELECT\
					type, parent, address, title, sequence,\
					editable, strftime('%s',creationdate), updatedate\
					FROM bookmarks\
					WHERE id=?",
					-1, &stmt, NULL);
#endif
	if (nError != SQLITE_OK) {
		DBG_LOGE("SQL error=%d", nError);
		_internet_bookmark_db_close(stmt);
		return NULL;
	}

	if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		return NULL;
	}

	if ((nError = sqlite3_step(stmt)) == SQLITE_ROW) {
		entry = (bookmark_entry *)calloc(1, sizeof(bookmark_entry));
		if (!entry) {
			DBG_LOGE("bookmark_item new is failed.\n");
			return NULL;
		}

		entry->id = id;
		entry->is_folder = sqlite3_column_int(stmt, 0);

		if (!entry->is_folder) {
			const char *url =
			    (const char *)(sqlite3_column_text(stmt, 2));
			entry->address = NULL;
			if (url) {
				int length = strlen(url);
				if (length > 0) {
					entry->address =
					    (char *)calloc(length + 1,
							   sizeof(char));
					memcpy(entry->address, url,
					       length);
				}
			}
		}
		entry->folder_id = sqlite3_column_int(stmt, 1);

		const char *title =
		    (const char *)(sqlite3_column_text(stmt, 3));
		entry->title = NULL;
		if (title) {
			int length = strlen(title);
			if (length > 0) {
				entry->title =
				    (char *)calloc(length + 1, sizeof(char));
				memcpy(entry->title, title, length);
			}
		}
		entry->orderIndex = sqlite3_column_int(stmt, 4);
		entry->editable = sqlite3_column_int(stmt, 5);

		const char *creationdate =
		    (const char *)(sqlite3_column_text(stmt, 6));
		entry->creationdate = NULL;
		if (creationdate) {
			int length = strlen(creationdate);
			if (length > 0) {
				entry->creationdate =
				    (char *)calloc(length + 1, sizeof(char));
				memcpy(entry->creationdate,
				       creationdate, length);
			}
		}
		const char *updatedate =
		    (const char *)(sqlite3_column_text(stmt, 7));
		entry->updatedate = NULL;
		if (updatedate) {
			int length = strlen(updatedate);
			if (length > 0) {
				entry->updatedate =
				    (char *)calloc(length + 1, sizeof(char));
				memcpy(entry->updatedate, updatedate,
				       length);
			}
		}
#if defined(BROWSER_TAG)
		const char *tag1 =
		    (const char *)(sqlite3_column_text(stmt, 8));
		entry->tag1 = NULL;
		if (tag1) {
			int length = strlen(tag1);
			if (length > 0) {
				entry->tag1 =
				    (char *)calloc(length + 1, sizeof(char));
				memcpy(entry->tag1, tag1, length);
			}
		}

		const char *tag2 =
		    (const char *)(sqlite3_column_text(stmt, 9));
		entry->tag2 = NULL;
		if (tag2) {
			int length = strlen(tag2);
			if (length > 0) {
				entry->tag2 =
				    (char *)calloc(length + 1, sizeof(char));
				memcpy(entry->tag2, tag2, length);
			}
		}

		const char *tag3 =
		    (const char *)(sqlite3_column_text(stmt, 10));
		entry->tag3 = NULL;
		if (tag3) {
			int length = strlen(tag3);
			if (length > 0) {
				entry->tag3 =
				    (char *)calloc(length + 1, sizeof(char));
				memcpy(entry->tag3, tag3, length);
			}
		}

		const char *tag4 =
		    (const char *)(sqlite3_column_text(stmt, 11));
		entry->tag4 = NULL;
		if (tag4) {
			int length = strlen(tag4);
			if (length > 0) {
				entry->tag4 =
				    (char *)calloc(length + 1, sizeof(char));
				memcpy(entry->tag4, tag4, length);
			}
		}
#endif
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return entry;
	}

	__bookmark_db_close();
	TRACE_END;
	return NULL;
}

void bmsvc_free_bookmark_entry(bookmark_entry *entry)
{
	TRACE_BEGIN;
	_bmsvc_destroy_bookmark_entry((void *)entry);
	TRACE_END;
}

tag_list *bmsvc_get_tag_list()
{
	TRACE_BEGIN;
	int nError;
	sqlite3_stmt *stmt;
	tag_list *pList;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed");
		return NULL;
	}
	/* same title  */
	nError = sqlite3_prepare_v2(m_internet_bookmark_db,
				"SELECT tag FROM tags ORDER BY tag", -1,
				&stmt, NULL);

	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed(%s).\n",
			sqlite3_errmsg(m_internet_bookmark_db));
		_internet_bookmark_db_close(stmt);
		return NULL;
	}

	/*  allocation .... Array for Items */
	pList = (tag_list *)calloc(1, sizeof(tag_list));
	pList->count = bmsvc_get_tag_count();
	if ((pList->count) < 0) {
		bmsvc_free_tag_list(pList);
		return NULL;
	}
	DBG_LOGD("count: %d", pList->count);
	pList->tag = (char **)calloc(pList->count, sizeof(char*));
	int i = 0;
	while ((nError = sqlite3_step(stmt)) == SQLITE_ROW) {
		const char *tag = (const char *)(sqlite3_column_text(stmt, 0));
		*((pList->tag)+i) = NULL;
		if (tag) {
			int length = strlen(tag);
			if (length > 0) {
				*((pList->tag)+i) = (char *)strdup(tag);
				DBG_LOGD("TAG[%d]: %s", i, *((pList->tag)+i));
			} else {
				DBG_LOGE("Empty tag is found");
			}
		}
		i++;
	}
	_internet_bookmark_db_close(stmt);
	TRACE_END;
	return pList;
}

int bmsvc_get_tag_count()
{
	TRACE_BEGIN;
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		return -1;
	}

	nError = sqlite3_prepare_v2(m_internet_bookmark_db,
			"SELECT\
			count(*)\
			FROM tags",
			-1, &stmt, NULL);

	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_ROW || nError == SQLITE_DONE) {
		int count = sqlite3_column_int(stmt, 0);
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return count;
	}
	__bookmark_db_close();
	TRACE_END;
	return 0;
}

int bmsvc_get_max_tag_count_for_a_bookmark_entry(int *max_count)
{
	*max_count = MAX_TAG_COUNT_FOR_1_BOOKMARK_ENTRY;
	return BMSVC_ERROR_NONE;
}

void bmsvc_free_tag_list(tag_list *list)
{
	TRACE_BEGIN;
	int i = 0;
	RET_MSG_IF(list == NULL, "Tag list is null");

	if (list->tag != NULL) {
		for (i = 0; i < list->count; i++) {
			if (*((list->tag)+i) != NULL)
				free(*((list->tag)+i));
		}
		free(list->tag);
	}
	free(list);
	list = NULL;
	TRACE_END;
}

bookmark_list *bmsvc_get_bookmark_list_by_tag(const char *tag)
{
	TRACE_BEGIN;
	DBG_LOGD("TAG :%s", tag);

	bookmark_list *m_list = NULL;
	int nError;
	sqlite3_stmt *stmt;

	/* check the total count of items */
	int item_count = 0;
	item_count = bmsvc_get_bookmark_count_by_tag(tag);
	if (item_count <= 0)
		return NULL;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		return NULL;
	}

	if (!tag || (strlen(tag) <= 0)) {
		DBG_LOGE("Untagged items \n");
		nError = sqlite3_prepare_v2(m_internet_bookmark_db,
			"SELECT\
			id, type, parent, address, title, editable, creationdate, updatedate, sequence,\
			tag1, tag2, tag3, tag4\
			FROM bookmarks\
			WHERE type=0 AND tag1=? AND tag2=? AND tag3=? AND tag4=?\
			ORDER BY title",
			-1, &stmt, NULL);
	} else {
		nError = sqlite3_prepare_v2(m_internet_bookmark_db,
			"SELECT\
			id, type, parent, address, title, editable, creationdate, updatedate, sequence,\
			tag1, tag2, tag3, tag4\
			FROM bookmarks\
			WHERE type=0 AND tag1=? OR tag2=? OR tag3=? OR tag4=?\
			ORDER BY title",
			-1, &stmt, NULL);
	}

	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed(%s).\n",
			sqlite3_errmsg(m_internet_bookmark_db));
		_internet_bookmark_db_close(stmt);
		return NULL;
	}

	if (sqlite3_bind_text(stmt, 1, tag, -1, NULL) != SQLITE_OK)
		DBG_LOGE("sqlite3_bind_text is failed.\n");
	if (sqlite3_bind_text(stmt, 2, tag, -1, NULL) != SQLITE_OK)
		DBG_LOGE("sqlite3_bind_text is failed.\n");
	if (sqlite3_bind_text(stmt, 3, tag, -1, NULL) != SQLITE_OK)
		DBG_LOGE("sqlite3_bind_text is failed.\n");
	if (sqlite3_bind_text(stmt, 4, tag, -1, NULL) != SQLITE_OK)
		DBG_LOGE("sqlite3_bind_text is failed.\n");

	/*  allocation .... Array for Items */
	m_list = (bookmark_list *) calloc(1, sizeof(bookmark_list));
	m_list->item =
	    (bookmark_entry *) calloc(item_count, sizeof(bookmark_entry));
	m_list->count = item_count;
	int i = 0;
	while ((nError = sqlite3_step(stmt)) == SQLITE_ROW
		&& (i < item_count)) {
		m_list->item[i].id = sqlite3_column_int(stmt, 0);
		m_list->item[i].is_folder = sqlite3_column_int(stmt, 1);
		m_list->item[i].folder_id = sqlite3_column_int(stmt, 2);

		if (!m_list->item[i].is_folder) {
			const char *url =
			    (const char *)(sqlite3_column_text(stmt, 3));
			m_list->item[i].address = NULL;
			if (url) {
				int length = strlen(url);
				if (length > 0) {
					m_list->item[i].address =
					    (char *)calloc(length + 1,
							   sizeof(char));
					memcpy(m_list->item[i].address, url,
					       length);
				}
			}
		}

		const char *title =
		    (const char *)(sqlite3_column_text(stmt, 4));
		DBG_LOGD("Title: %s", title);
		m_list->item[i].title = NULL;
		if (title) {
			int length = strlen(title);
			if (length > 0) {
				m_list->item[i].title =
				    (char *)calloc(length + 1, sizeof(char));
				memcpy(m_list->item[i].title, title, length);
			}
		}
		m_list->item[i].editable = sqlite3_column_int(stmt, 5);

		const char *creationdate =
		    (const char *)(sqlite3_column_text(stmt, 6));
		m_list->item[i].creationdate = NULL;
		if (creationdate) {
			int length = strlen(creationdate);
			if (length > 0) {
				m_list->item[i].creationdate =
				    (char *)calloc(length + 1, sizeof(char));
				memcpy(m_list->item[i].creationdate, creationdate, length);
			}
		}
		const char *updatedate =
		    (const char *)(sqlite3_column_text(stmt, 7));
		m_list->item[i].updatedate = NULL;
		if (updatedate) {
			int length = strlen(updatedate);
			if (length > 0) {
				m_list->item[i].updatedate =
				    (char *)calloc(length + 1, sizeof(char));
				memcpy(m_list->item[i].updatedate, updatedate, length);
			}
		}

		m_list->item[i].orderIndex = sqlite3_column_int(stmt, 8);

		const char *tag1 =
		    (const char *)(sqlite3_column_text(stmt, 9));
		m_list->item[i].tag1 = NULL;
		if (tag1) {
			int length = strlen(tag1);
			if (length > 0) {
				m_list->item[i].tag1 =
				    (char *)calloc(length + 1, sizeof(char));
				memcpy(m_list->item[i].tag1, tag1, length);
			}
		}

		const char *tag2 =
		    (const char *)(sqlite3_column_text(stmt, 10));
		m_list->item[i].tag2 = NULL;
		if (tag2) {
			int length = strlen(tag2);
			if (length > 0) {
				m_list->item[i].tag2 =
				    (char *)calloc(length + 1, sizeof(char));
				memcpy(m_list->item[i].tag2, tag2, length);
			}
		}

		const char *tag3 =
		    (const char *)(sqlite3_column_text(stmt, 11));
		m_list->item[i].tag3 = NULL;
		if (tag3) {
			int length = strlen(tag3);
			if (length > 0) {
				m_list->item[i].tag3 =
				    (char *)calloc(length + 1, sizeof(char));
				memcpy(m_list->item[i].tag3, tag3, length);
			}
		}

		const char *tag4 =
		    (const char *)(sqlite3_column_text(stmt, 12));
		m_list->item[i].tag4 = NULL;
		if (tag4) {
			int length = strlen(tag4);
			if (length > 0) {
				m_list->item[i].tag4 =
				    (char *)calloc(length + 1, sizeof(char));
				memcpy(m_list->item[i].tag4, tag4, length);
			}
		}

		i++;
	}
	m_list->count = i;
	_internet_bookmark_db_close(stmt);

	TRACE_END;
	return m_list;
}

int bmsvc_get_bookmark_count_by_tag(const char *tag)
{
	TRACE_BEGIN;
	DBG_LOGD("TAG :%s", tag);

	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		return -1;
	}

	if (!tag || (strlen(tag) <= 0)) {
		DBG_LOGD("get UNTAGGED ITEMS");
		nError = sqlite3_prepare_v2(m_internet_bookmark_db,
				"SELECT\
				count(*)\
				FROM bookmarks\
				WHERE type=0 AND tag1=? AND tag2=? AND tag3=? AND tag4=?",
				-1, &stmt, NULL);
	} else {
		nError = sqlite3_prepare_v2(m_internet_bookmark_db,
				"SELECT\
				count(*)\
				FROM bookmarks\
				WHERE type=0 AND tag1=? OR tag2=? OR tag3=? OR tag4=?",
				-1, &stmt, NULL);
	}

	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed(%s).\n",
			sqlite3_errmsg(m_internet_bookmark_db));
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	if (sqlite3_bind_text(stmt, 1, tag, -1, NULL) != SQLITE_OK)
		DBG_LOGE("sqlite3_bind_text is failed.\n");
	if (sqlite3_bind_text(stmt, 2, tag, -1, NULL) != SQLITE_OK)
		DBG_LOGE("sqlite3_bind_text is failed.\n");
	if (sqlite3_bind_text(stmt, 3, tag, -1, NULL) != SQLITE_OK)
		DBG_LOGE("sqlite3_bind_text is failed.\n");
	if (sqlite3_bind_text(stmt, 4, tag, -1, NULL) != SQLITE_OK)
		DBG_LOGE("sqlite3_bind_text is failed.\n");

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_ROW || nError == SQLITE_DONE) {
		int count = sqlite3_column_int(stmt, 0);
		_internet_bookmark_db_close(stmt);
		return count;
	}
	__bookmark_db_close();
	TRACE_END;
	return 0;
}

int bmsvc_add_bookmark(const char *title, const char *address, int parent_id,
						const char *tag1,
						const char *tag2,
						const char *tag3,
						const char *tag4,
						int *saved_bookmark_id)
{
	TRACE_BEGIN;
	RETV_MSG_IF(title == NULL || strlen(title) <= 0,
						BMSVC_ERROR_INVALID_PARAMETER,
						"Invalid param: Title is empty");
	RETV_MSG_IF(address == NULL || strlen(address) <= 0,
						BMSVC_ERROR_INVALID_PARAMETER,
						"Invalid param: Address is empty");
	RETV_MSG_IF(memcmp(address, "file://", 7) == 0,
						BMSVC_ERROR_INVALID_PARAMETER,
						"Not Allow file://");
	if (parent_id > 0) {
		/* check whether folder is exist or not. */
		if (_internet_id_is_exist(parent_id, 1) <= 0) {
			DBG_LOGE("Not Found Folder (%d)\n", parent_id);
			return BMSVC_ERROR_INVALID_PARAMETER;
		}
	} else {
		parent_id = internet_bookmark_get_root_id();
	}
	DBG_LOGD("[%s][%s][%d]", title, address, parent_id);

	int uid = _bmsvc_bookmark_is_exist(address);
	/* ignore this call.. already exist. */
	if (uid > 0) {
		DBG_LOGD("Bookmark is already exist. cancel the add operation.");
		return BMSVC_ERROR_ITEM_ALREADY_EXIST;
	}

	//return _internet_bookmark_new(address, title, folder_id, property_flag);
	int lastIndex = _internet_bookmark_get_lastindex(parent_id);
	if (lastIndex < 0) {
		DBG_LOGE("Database::getLastIndex() is failed.\n");
		return -1;
	}
	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		return BMSVC_ERROR_DB_FAILED;
	}

	int nError;
	sqlite3_stmt *stmt;
#if defined(BROWSER_TAG)
	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
					"INSERT INTO bookmarks (type, parent, address, title, creationdate,\
					updatedate, editable, sequence, tag1, tag2, tag3, tag4)\
					VALUES (0, ?, ?, ?, DATETIME('now'), DATETIME('now'), 1, ?, ?, ?, ?, ?)",
			       -1, &stmt, NULL);
#else
	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
					"INSERT INTO bookmarks (type, parent, address, title, creationdate,\
					updatedate, editable, sequence)\
					VALUES (0, ?, ?, ?, DATETIME('now'), DATETIME('now'), 1, ?)",
			       -1, &stmt, NULL);
#endif
	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	/*parent */
	if (sqlite3_bind_int(stmt, 1, parent_id) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		return BMSVC_ERROR_DB_FAILED;
	}
	/*address */
	if (sqlite3_bind_text(stmt, 2, address, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		return BMSVC_ERROR_DB_FAILED;
	}
	/*title */
	if (sqlite3_bind_text(stmt, 3, title, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		return BMSVC_ERROR_DB_FAILED;
	}
	/* order */
	if (lastIndex >= 0) {
		if (sqlite3_bind_int(stmt, 4, (lastIndex + 1)) != SQLITE_OK) {
			DBG_LOGE("sqlite3_bind_int is failed.\n");
			_internet_bookmark_db_close(stmt);
			return BMSVC_ERROR_DB_FAILED;
		}
	}
#if defined(BROWSER_TAG)
	/*tag1 */
	if (!tag1)
		tag1 = "";
	if (sqlite3_bind_text(stmt, 5, tag1, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		return BMSVC_ERROR_DB_FAILED;
	}
	/*tag2 */
	if (!tag2)
		tag2 = "";
	if (sqlite3_bind_text(stmt, 6, tag2, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		return BMSVC_ERROR_DB_FAILED;
	}
	/*tag3 */
	if (!tag3)
		tag3 = "";
	if (sqlite3_bind_text(stmt, 7, tag3, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		return BMSVC_ERROR_DB_FAILED;
	}
	/*tag4 */
	if (!tag4)
		tag4 = "";
	if (sqlite3_bind_text(stmt, 8, tag4, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		return BMSVC_ERROR_DB_FAILED;
	}
#endif
	nError = sqlite3_step(stmt);
	if (nError == SQLITE_OK || nError == SQLITE_DONE) {
		_internet_bookmark_db_close(stmt);
		int id = _bmsvc_get_bookmark_id(address);
		if (id > 0)
			*saved_bookmark_id = id;
#if defined(BROWSER_TAG)
		/* process tags */
		if (strlen(tag1) > 0 && _bmsvc_tag_exists_at_bookmarks(tag1) > 0)
			_bmsvc_add_tag_to_taglist(tag1);
		if (strlen(tag2) > 0 && _bmsvc_tag_exists_at_bookmarks(tag2) > 0)
			_bmsvc_add_tag_to_taglist(tag2);
		if (strlen(tag3) > 0 && _bmsvc_tag_exists_at_bookmarks(tag3) > 0)
			_bmsvc_add_tag_to_taglist(tag3);
		if (strlen(tag4) > 0 && _bmsvc_tag_exists_at_bookmarks(tag4) > 0)
			_bmsvc_add_tag_to_taglist(tag4);
#endif
		TRACE_END;
		return BMSVC_ERROR_NONE;
	}
	DBG_LOGE("sqlite3_step is failed");
	__bookmark_db_close();
	TRACE_END;
	return BMSVC_ERROR_DB_FAILED;
}

int bmsvc_add_folder(const char *title, int parent_id, int editable, int *saved_folder_id)
{
	TRACE_BEGIN;
	RETV_MSG_IF(title == NULL || strlen(title) <= 0,
						BMSVC_ERROR_INVALID_PARAMETER,
						"Invalid param: Title is empty");
	if (parent_id > 0) {
		/* check whether folder is exist or not. */
		if (_internet_id_is_exist(parent_id, 1) <= 0) {
			DBG_LOGE("Not Found Folder (%d)\n", parent_id);
			return -1;
		}
	} else {
		parent_id = internet_bookmark_get_root_id();
	}
	DBG_LOGD("[%s][%d]", title, parent_id);

	if (editable < 0)
		editable = 1;

	int uid = _bmsvc_folder_is_exist(title, parent_id);
	/* ignore this call.. already exist. */
	if (uid > 0) {
		DBG_LOGD("Bookmark is already exist. cancel the add operation.");
		return BMSVC_ERROR_ITEM_ALREADY_EXIST;
	}

	int lastIndex = _internet_bookmark_get_lastindex(parent_id);
	if (lastIndex < 0) {
		DBG_LOGE("Database::getLastIndex() is failed.\n");
		return -1;
	}
	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		return BMSVC_ERROR_DB_FAILED;
	}

	int nError;
	sqlite3_stmt *stmt;

	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
					"INSERT INTO bookmarks (type, parent, title, creationdate,\
					updatedate, editable, sequence)\
					VALUES (1, ?, ?, DATETIME('now'), DATETIME('now'), ?, ?)",
			       -1, &stmt, NULL);
	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed.\n");
		_internet_bookmark_db_close(stmt);
		return -1;
	}

	/*parent */
	if (sqlite3_bind_int(stmt, 1, parent_id) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		return BMSVC_ERROR_DB_FAILED;
	}
	/*title */
	if (sqlite3_bind_text(stmt, 2, title, -1, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_text is failed.\n");
		_internet_bookmark_db_close(stmt);
		return BMSVC_ERROR_DB_FAILED;
	}
	/*Editable */
	if (sqlite3_bind_int(stmt, 3, editable) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		return BMSVC_ERROR_DB_FAILED;
	}
	/* order */
	if (lastIndex >= 0) {
		if (sqlite3_bind_int(stmt, 4, (lastIndex + 1)) != SQLITE_OK) {
			DBG_LOGE("sqlite3_bind_int is failed.\n");
			_internet_bookmark_db_close(stmt);
			return BMSVC_ERROR_DB_FAILED;
		}
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_OK || nError == SQLITE_DONE) {
		_internet_bookmark_db_close(stmt);
		int id = _bmsvc_get_folder_id(title, parent_id);
		if (id > 0)
			*saved_folder_id = id;
		TRACE_END;
		return BMSVC_ERROR_NONE;
	}
	DBG_LOGE("sqlite3_step is failed");
	__bookmark_db_close();
	TRACE_END;
	return BMSVC_ERROR_DB_FAILED;
}

int bmsvc_get_bookmark_id(const char *address, int *bookmark_id)
{
	TRACE_BEGIN;

	int ret = _bmsvc_bookmark_is_exist(address);
	if (ret == 1) {
		*bookmark_id = _bmsvc_get_bookmark_id(address);
		TRACE_END;
		return BMSVC_ERROR_NONE;
	} else {
		*bookmark_id = -1;
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
}

int bmsvc_get_folder_id(const char *title, int parent_id, int *folder_id)
{
	TRACE_BEGIN;

	int ret = _bmsvc_folder_is_exist(title, parent_id);
	if (ret == 1) {
		*folder_id = _bmsvc_get_folder_id(title, parent_id);
		TRACE_END;
		return BMSVC_ERROR_NONE;
	} else {
		*folder_id = -1;
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
}

int bmsvc_delete_bookmark(int id, int check_editable)
{
	TRACE_BEGIN;

	bookmark_entry *entry = bmsvc_get_bookmark_by_id(id);
	int ret = 0;
	if (!entry)
		return BMSVC_ERROR_DB_FAILED;

	// check wheather this bookmark(or folder) is uneditable or not.
	if (!(entry->editable) && check_editable) {
		_bmsvc_destroy_bookmark_entry((void *)entry);
		TRACE_END;
		return BMSVC_ERROR_ITEM_IS_NOT_EDITABLE;
	}

	if (entry->is_folder) {
		DBG_LOGD("Folder [%s]", entry->title);
		/* Folder */
		/* try to delete all subitems fisrt */
		ret = _bmsvc_delete_folder_subitems(entry->id, check_editable);
		if (ret != BMSVC_ERROR_NONE) {
			_bmsvc_destroy_bookmark_entry((void *)entry);
			TRACE_END;
			return ret;
		}
		/* if the folder is empty, delete the folder itself */
		if (_bmsvc_delete_bookmark_entry(id) < 0) {
			_bmsvc_destroy_bookmark_entry((void *)entry);
			TRACE_END;
			return BMSVC_ERROR_DB_FAILED;
		}
		_bmsvc_destroy_bookmark_entry((void *)entry);
		TRACE_END;
		return BMSVC_ERROR_NONE;
	} else {
		DBG_LOGD("Bookmark [%s]", entry->title);
		/* Bookmark */
		// get tags from this bookmark if there are any tags.
		// erase the bookmark.
		if (_bmsvc_delete_bookmark_entry(id) < 0) {
			_bmsvc_destroy_bookmark_entry((void *)entry);
			TRACE_END;
			return BMSVC_ERROR_DB_FAILED;
		}
		// check if there are any other bookmarks which has same tags
		// if there are same tag at another bookmark, remain tag name from the tags table
		// if there is no same tag at another bookmark, erase the tag name from the tags table.
		_bmsvc_delete_bookmark_tags(entry);

		_bmsvc_destroy_bookmark_entry((void *)entry);
		TRACE_END;
		return BMSVC_ERROR_NONE;
	}
}

int bmsvc_update_bookmark(int id, const char *title, const char *address, int parent_id, int order,
						const char *tag1,
						const char *tag2,
						const char *tag3,
						const char *tag4,
						int check_editable)
{
	TRACE_BEGIN;
	bookmark_entry *entry = NULL;
	bookmark_entry *parent_entry = NULL;

	if (id <= 0)
		return BMSVC_ERROR_INVALID_PARAMETER;
	if (title == NULL || strlen(title) <= 0)
		return BMSVC_ERROR_INVALID_PARAMETER;
	entry =  bmsvc_get_bookmark_by_id(id);
	if (!entry)
		return BMSVC_ERROR_INVALID_PARAMETER;
	if (!(entry->is_folder)) {
		if (address == NULL || strlen(address) <= 0) {
			_bmsvc_destroy_bookmark_entry((void *)entry);
			return BMSVC_ERROR_INVALID_PARAMETER;
		}
	}

	if (parent_id > 0) {
		/*folder */
		parent_entry =  bmsvc_get_bookmark_by_id(id);
		if (!parent_entry) {
			DBG_LOGE("Not Found Folder (%d)\n", parent_entry);
			_bmsvc_destroy_bookmark_entry((void *)parent_entry);
			_bmsvc_destroy_bookmark_entry((void *)entry);
			return BMSVC_ERROR_INVALID_PARAMETER;
		}
		_bmsvc_destroy_bookmark_entry((void *)parent_entry);
	} else {
		parent_id = entry->folder_id;
	}

	if (order < 0)
		order = entry->orderIndex;

	if (check_editable) {
		if (!(entry->editable)) {
			_bmsvc_destroy_bookmark_entry((void *)entry);
			return BMSVC_ERROR_ITEM_IS_NOT_EDITABLE;
		}
	}

	int ret = 0;
	if (entry->is_folder) {
		/* Folder */
		_bmsvc_destroy_bookmark_entry((void *)entry);
		return _bmsvc_update_folder_record(id, title,  parent_id, order);
	} else {
		/* Bookmark */
		ret = _bmsvc_update_bookmark_record(id, title, address, parent_id, order,
						tag1, tag2, tag3, tag4);
		if (ret != BMSVC_ERROR_NONE) {
			_bmsvc_destroy_bookmark_entry((void *)entry);
			TRACE_END;
			return ret;
		}
		/* process tags */
		// check if there are any other bookmarks which has same tags
		// if there are same tag at another bookmark, remain tag name from the tags table
		// if there is no same tag at another bookmark, erase the tag name from the tags table.
		DBG_LOGE("Processing Tags");
		_bmsvc_delete_bookmark_tags(entry);
		if (!tag1)
			tag1 = "";
		if (!tag2)
			tag2 = "";
		if (!tag3)
			tag3 = "";
		if (!tag4)
			tag4 = "";
		if (strlen(tag1) > 0 && _bmsvc_tag_exists_at_bookmarks(tag1) > 0)
			_bmsvc_add_tag_to_taglist(tag1);
		if (strlen(tag2) > 0 && _bmsvc_tag_exists_at_bookmarks(tag2) > 0)
			_bmsvc_add_tag_to_taglist(tag2);
		if (strlen(tag3) > 0 && _bmsvc_tag_exists_at_bookmarks(tag3) > 0)
			_bmsvc_add_tag_to_taglist(tag3);
		if (strlen(tag4) > 0 && _bmsvc_tag_exists_at_bookmarks(tag4) > 0)
			_bmsvc_add_tag_to_taglist(tag4);
		DBG_LOGE("Processing Tags finisied");
		_bmsvc_destroy_bookmark_entry((void *)entry);
		TRACE_END;
		return BMSVC_ERROR_NONE;
	}
}

int bmsvc_set_thumbnail(int id, void *image_data, int w, int h, int len)
{
	TRACE_BEGIN;
	bookmark_entry *entry = NULL;
	int nError;
	sqlite3_stmt *stmt;

	RETV_MSG_IF(id < 0, BMSVC_ERROR_INVALID_PARAMETER,
						"Invalid param: id is negative");
	RETV_MSG_IF(image_data == NULL,
						BMSVC_ERROR_INVALID_PARAMETER,
						"Invalid param: thumbnail_data is empty");
	RETV_MSG_IF(w < 0, BMSVC_ERROR_INVALID_PARAMETER,
						"Invalid param: width is negative");
	RETV_MSG_IF(h < 0, BMSVC_ERROR_INVALID_PARAMETER,
						"Invalid param: height is negative");
	RETV_MSG_IF(len < 0, BMSVC_ERROR_INVALID_PARAMETER,
						"Invalid param: length is negative");
	DBG_LOGD("id: %d", id);
	DBG_LOGD("w:%d, h: %d, len: %d", w, h, len);
	entry =  bmsvc_get_bookmark_by_id(id);
	if (!entry)
		return BMSVC_ERROR_INVALID_PARAMETER;
	_bmsvc_destroy_bookmark_entry((void *)entry);

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		return BMSVC_ERROR_DB_FAILED;
	}

	nError = sqlite3_prepare_v2(m_internet_bookmark_db,
				       "UPDATE bookmarks SET thumbnail=?, thumbnail_length=?\
				       ,thumbnail_w=?, thumbnail_h=? \
				       WHERE id=?",
				       -1, &stmt, NULL);
	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	/* binding values */
	if (sqlite3_bind_blob(stmt, 1, image_data , len, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_blob is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	if (sqlite3_bind_int(stmt, 2, len) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	if (sqlite3_bind_int(stmt, 3, w) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	if (sqlite3_bind_int(stmt, 4, h) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
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
	return BMSVC_ERROR_DB_FAILED;
}

int bmsvc_get_thumbnail(int id, void **image_data, int *w, int *h, int *len)
{
	TRACE_BEGIN;
	DBG_LOGD("id :%d", id);

	int nError;
	sqlite3_stmt *stmt;
	void *thumbnail_data_temp = NULL;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}

	nError = sqlite3_prepare_v2(m_internet_bookmark_db,
					"SELECT\
					thumbnail, thumbnail_length\
					,thumbnail_w, thumbnail_h\
					FROM bookmarks\
					WHERE id=?",
					-1, &stmt, NULL);
	if (nError != SQLITE_OK) {
		DBG_LOGE("SQL error=%d", nError);
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}

	if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_ROW) {
		thumbnail_data_temp = (void *)sqlite3_column_blob(stmt,0);
		*len = sqlite3_column_int(stmt,1);
		*w = sqlite3_column_int(stmt,2);
		*h = sqlite3_column_int(stmt,3);
		DBG_LOGD("len:%d, w:%d. h:%d", *len, *w, *h);
		if (len > 0){
			*image_data = calloc(1, *len);
			memcpy(*image_data, thumbnail_data_temp, *len);
		}
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_NONE;
	}

	__bookmark_db_close();
	TRACE_END;
	return BMSVC_ERROR_DB_FAILED;
}

int bmsvc_set_favicon(int id, void *image_data, int w, int h, int len)
{
	TRACE_BEGIN;
	bookmark_entry *entry = NULL;
	int nError;
	sqlite3_stmt *stmt;

	RETV_MSG_IF(id < 0, BMSVC_ERROR_INVALID_PARAMETER,
						"Invalid param: id is negative");
	RETV_MSG_IF(image_data == NULL,
						BMSVC_ERROR_INVALID_PARAMETER,
						"Invalid param: thumbnail_data is empty");
	RETV_MSG_IF(w < 0, BMSVC_ERROR_INVALID_PARAMETER,
						"Invalid param: width is negative");
	RETV_MSG_IF(h < 0, BMSVC_ERROR_INVALID_PARAMETER,
						"Invalid param: height is negative");
	RETV_MSG_IF(len < 0, BMSVC_ERROR_INVALID_PARAMETER,
						"Invalid param: length is negative");
	DBG_LOGD("id: %d", id);
	DBG_LOGD("w:%d, h: %d, len: %d", w, h, len);
	entry =  bmsvc_get_bookmark_by_id(id);
	if (!entry)
		return BMSVC_ERROR_INVALID_PARAMETER;
	_bmsvc_destroy_bookmark_entry((void *)entry);

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		return BMSVC_ERROR_DB_FAILED;
	}

	nError = sqlite3_prepare_v2(m_internet_bookmark_db,
				       "UPDATE bookmarks SET favicon=?, favicon_length=?\
				       ,favicon_w=?, favicon_h=? \
				       WHERE id=?",
				       -1, &stmt, NULL);
	if (nError != SQLITE_OK) {
		DBG_LOGE("sqlite3_prepare_v2 is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	/* binding values */
	if (sqlite3_bind_blob(stmt, 1, image_data , len, NULL) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_blob is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	if (sqlite3_bind_int(stmt, 2, len) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	if (sqlite3_bind_int(stmt, 3, w) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
	if (sqlite3_bind_int(stmt, 4, h) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}
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
	return BMSVC_ERROR_DB_FAILED;
}

int bmsvc_get_favicon(int id, void **image_data, int *w, int *h, int *len)
{
	TRACE_BEGIN;
	DBG_LOGD("id :%d", id);

	int nError;
	sqlite3_stmt *stmt;
	void *thumbnail_data_temp = NULL;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}

	nError = sqlite3_prepare_v2(m_internet_bookmark_db,
					"SELECT\
					favicon, favicon_length\
					,favicon_w, favicon_h\
					FROM bookmarks\
					WHERE id=?",
					-1, &stmt, NULL);
	if (nError != SQLITE_OK) {
		DBG_LOGE("SQL error=%d", nError);
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}

	if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK) {
		DBG_LOGE("sqlite3_bind_int is failed.\n");
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_DB_FAILED;
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_ROW) {
		thumbnail_data_temp = (void *)sqlite3_column_blob(stmt,0);
		*len = sqlite3_column_int(stmt,1);
		*w = sqlite3_column_int(stmt,2);
		*h = sqlite3_column_int(stmt,3);
		DBG_LOGD("len:%d, w:%d. h:%d", *len, *w, *h);
		if (len > 0){
			*image_data = calloc(1, *len);
			memcpy(*image_data, thumbnail_data_temp, *len);
		}
		_internet_bookmark_db_close(stmt);
		TRACE_END;
		return BMSVC_ERROR_NONE;
	}

	__bookmark_db_close();
	TRACE_END;
	return BMSVC_ERROR_DB_FAILED;
}