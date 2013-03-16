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

sqlite3 *m_internet_bookmark_db = 0;

/* Open APIs */
/* For SyncML & CSC */

int internet_bookmark_get_root_id()
{
	return ROOT_FOLDER_ID;
}

int internet_bookmark_is_exist(int folder_id, char *title, int property_flag)
{
	__BOOKMARK_DEBUG_TRACE("id :%d title :[%s]flag :%d", folder_id, title,
			       property_flag);
	if (title == NULL || strlen(title) <= 0)
		return -1;
	if (folder_id > 0) {
		/* check whether folder is exist or not. */
		if (_internet_id_is_exist(folder_id, 1) <= 0) {
			__BOOKMARK_DEBUG_TRACE("Not Found Folder (%d)\n",
					       folder_id);
			return -1;
		}
	} else {
		folder_id = internet_bookmark_get_root_id();
	}

	if (property_flag == 1)	/* folder */
		return _internet_bookmark_is_exist(folder_id, title, 1);
	/* bookmark */
	return _internet_bookmark_is_exist(folder_id, title, 0);
}

int internet_bookmark_count(int folder_id, int property_flag)
{
	__BOOKMARK_DEBUG_TRACE("id :%d flag :%d", folder_id, property_flag);

	int nError;
	sqlite3_stmt *stmt;

	if (folder_id > 0) {
		/* check whether folder is exist or not. */
		if (_internet_id_is_exist(folder_id, 1) <= 0) {
			__BOOKMARK_DEBUG_TRACE("Not Found Folder (%d)\n",
					       folder_id);
			return 0;
		}
	}

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed\n");
		return -1;
	}

	if (folder_id < 0) {	/* search entirely */
		/* only bookmarks */
		if (property_flag == 0) {
			nError =
			    sqlite3_prepare_v2(m_internet_bookmark_db,
					       "select count(*) from bookmarks where type=0",
					       -1, &stmt, NULL);
		} else if (property_flag == 1) {
			/* only folders */
			nError =
			    sqlite3_prepare_v2(m_internet_bookmark_db,
					       "select count(*) from bookmarks where type=1",
					       -1, &stmt, NULL);
		} else {
			/* folder + bookmark */
			nError =
			    sqlite3_prepare_v2(m_internet_bookmark_db,
					       "select count(*) from bookmarks",
					       -1, &stmt, NULL);
		}
		if (nError != SQLITE_OK) {
			__BOOKMARK_DEBUG_TRACE
			    ("sqlite3_prepare_v2 is failed.\n");
			_internet_bookmark_db_close(stmt);
			return -1;
		}
	} else {		/* search in a folder */
		/* only bookmarks */
		if (property_flag == 0) {
			nError =
			    sqlite3_prepare_v2(m_internet_bookmark_db,
					       "select count(*) from bookmarks where parent=? and type=0",
					       -1, &stmt, NULL);
		} else if (property_flag == 1) {
			/* only folders */
			nError =
			    sqlite3_prepare_v2(m_internet_bookmark_db,
					       "select count(*) from bookmarks where parent=? and type=1",
					       -1, &stmt, NULL);
		} else {
			/* folder + bookmark */
			nError =
			    sqlite3_prepare_v2(m_internet_bookmark_db,
					       "select count(*) from bookmarks where parent=?",
					       -1, &stmt, NULL);
		}
		if (nError != SQLITE_OK) {
			__BOOKMARK_DEBUG_TRACE
			    ("sqlite3_prepare_v2 is failed.\n");
			_internet_bookmark_db_close(stmt);
			return -1;
		}
		if (sqlite3_bind_int(stmt, 1, folder_id) != SQLITE_OK) {
			__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.\n");
			_internet_bookmark_db_close(stmt);
			return -1;
		}
	}

	nError = sqlite3_step(stmt);
	if (nError == SQLITE_ROW) {
		int count = sqlite3_column_int(stmt, 0);
		_internet_bookmark_db_close(stmt);
		return count;
	}
	__bookmark_db_close();
	return 0;
}

int internet_bookmark_new(char *address, char *title, int folder_id,
			  int property_flag, int overwrite_flag)
{
	__BOOKMARK_DEBUG_TRACE
	    ("[%s][%s] folder_id:%d property_flag :%d overwrite_flag:%d",
	     address, title, folder_id, property_flag, overwrite_flag);

	if (title == NULL || strlen(title) <= 0)
		return -1;
	if (property_flag == 0 && (address == NULL || strlen(address) <= 0))
		return -1;
	if (property_flag == 0 && address != NULL
	    && memcmp(address, "file://", 7) == 0) {
		__BOOKMARK_DEBUG_TRACE("Not Allow file://");
		return -1;
	}
	if (folder_id > 0) {
		/* check whether folder is exist or not. */
		if (_internet_id_is_exist(folder_id, 1) <= 0) {
			__BOOKMARK_DEBUG_TRACE("Not Found Folder (%d)\n",
					       folder_id);
			return -1;
		}
	} else {
		folder_id = internet_bookmark_get_root_id();
	}

	int uid = _internet_bookmark_is_exist(folder_id, title, property_flag);
	/* ignore this call.. already exist. */
	if (uid > 0 && overwrite_flag == 1) {
		__BOOKMARK_DEBUG_TRACE("Ignore this call");
		return 0;
	}
	/* update operation..... */
	if (uid > 0 && overwrite_flag == 0) {
		/* Deny Read Only item */
		if (_internet_id_get_editable(uid) != 1) {
			__BOOKMARK_DEBUG_TRACE("Read Only Item (%s)", title);
			return -1;
		}
		__BOOKMARK_DEBUG_TRACE("Update (%s)", title);
		return _internet_bookmark_update(uid, address, title,
						 folder_id);
	}
	return _internet_bookmark_new(address, title, folder_id, property_flag);
}

int internet_bookmark_update(int uid, char *address, char *title, int folder_id,
			     int property_flag, int create_flag)
{
	__BOOKMARK_DEBUG_TRACE("uid :%d [%s][%s] fid:%d pflag :%d cflag:%d",
			       uid, address, title, folder_id, property_flag,
			       create_flag);

	if (title == NULL || strlen(title) <= 0)
		return -1;
	if (property_flag == 0 && (address == NULL || strlen(address) <= 0))
		return -1;
	if (folder_id > 0) {
		/*folder */
		if (_internet_id_is_exist(folder_id, 1) <= 0) {
			__BOOKMARK_DEBUG_TRACE("Not Found Folder (%d)\n",
					       folder_id);
			return -1;
		}
	} else {
		folder_id = internet_bookmark_get_root_id();
	}

	if (_internet_id_is_exist(uid, property_flag) <= 0) {
		if (create_flag == 0)
			return _internet_bookmark_new(address, title, folder_id,
						      property_flag);
		return 0;	/* ignore this call, not found item. */
	}
	/* Deny Read Only item */
	if (_internet_id_get_editable(uid) != 1) {
		__BOOKMARK_DEBUG_TRACE("Read Only Item (%s)", title);
		return -1;
	}
	return _internet_bookmark_update(uid, address, title, folder_id);
}

int internet_bookmark_remove(int id, int remove_flag)
{
	__BOOKMARK_DEBUG_TRACE("id :%d flag :%d", id, remove_flag);
	int type = 0;
	int editable = 1;

	if ((editable = _internet_id_get_editable(id)) != 1)
		return -1;	/* Deny Read Only item */
	/* If input is folder.... manage sub items. */
	if ((type = _internet_id_get_type(id)) == 1) {
		/* remove all sub items. */
		if (remove_flag == 0) {
			/* get all items ( bookmark & folder ) */
			bookmark_list *m_remove_list =
			    internet_bookmark_list(id, 2);
			if (m_remove_list != NULL && m_remove_list->count > 0) {
				int i = 0;
				for (i = 0; i < m_remove_list->count; i++) {
					/* found folder. */
					if (m_remove_list->item[i].is_folder
					    == 1) {
						/* recursive call */
						internet_bookmark_remove
						    (m_remove_list->item[i].id,
						     remove_flag);
					} else {
						if (m_remove_list->
						    item[i].editable != 1)
						_internet_bookmark_move_to_root
							    (m_remove_list->
							     item[i].id);
						else
						_internet_bookmark_remove
							    (m_remove_list->
							     item[i].id);
					}
				}
			}
			/* free m_remove_list */
			internet_bookmark_free(m_remove_list);
		} else {	/* move all sub items to root folder */

			_internet_sub_bookmarks_move_to_root(id);
		}
	}
	if (type < 0)
		return -1;
	_internet_bookmark_remove(id);
	return 0;
}

bookmark_list *internet_bookmark_list(int folder_id, int property_flag)
{
	DBG_LOGD("id :%d flag :%d", folder_id, property_flag);

	bookmark_list *m_list = NULL;
	int nError;
	sqlite3_stmt *stmt;

	/* check the total count of items */
	int item_count = 0;
	if (property_flag == 0 || property_flag == 2) {
		item_count = internet_bookmark_count(folder_id, 0);
	}
	if (property_flag >= 1) {
		item_count = item_count + internet_bookmark_count(folder_id, 1);
	}
	if (item_count <= 0)
		return NULL;

	if (_internet_bookmark_db_open() < 0) {
		DBG_LOGE("db_util_open is failed\n");
		return NULL;
	}

	if (folder_id < 0) {
		if (property_flag == 0)
			nError =
			    sqlite3_prepare_v2(m_internet_bookmark_db,
					       "select id, type, parent, address, title, editable, creationdate, updatedate, sequence  from bookmarks where type=0 order by sequence",
					       -1, &stmt, NULL);
		else if (property_flag == 1)
			nError =
			    sqlite3_prepare_v2(m_internet_bookmark_db,
					       "select id, type, parent, address, title, editable, creationdate, updatedate, sequence  from bookmarks where type=1 order by sequence",
					       -1, &stmt, NULL);
		else
			nError =
			    sqlite3_prepare_v2(m_internet_bookmark_db,
					       "select id, type, parent, address, title, editable, creationdate, updatedate, sequence  from bookmarks order by sequence",
					       -1, &stmt, NULL);
		if (nError != SQLITE_OK) {
			__BOOKMARK_DEBUG_TRACE
			    ("sqlite3_prepare_v2 is failed.\n");
			_internet_bookmark_db_close(stmt);
			return NULL;
		}
	} else {
		if (property_flag == 0)
			nError =
			    sqlite3_prepare_v2(m_internet_bookmark_db,
					       "select id, type, parent, address, title, editable, creationdate, updatedate, sequence  from bookmarks where parent=? and type=0 order by sequence",
					       -1, &stmt, NULL);
		else if (property_flag == 1)
			nError =
			    sqlite3_prepare_v2(m_internet_bookmark_db,
					       "select id, type, parent, address, title, editable, creationdate, updatedate, sequence  from bookmarks where parent=? and type=1 order by sequence",
					       -1, &stmt, NULL);
		else
			nError =
			    sqlite3_prepare_v2(m_internet_bookmark_db,
					       "select id, type, parent, address, title, editable, creationdate, updatedate, sequence  from bookmarks where parent=? order by sequence",
					       -1, &stmt, NULL);
		if (nError != SQLITE_OK) {
			DBG_LOGE("sqlite3_prepare_v2 is failed.\n");
			_internet_bookmark_db_close(stmt);
			return NULL;
		}
		if (sqlite3_bind_int(stmt, 1, folder_id) != SQLITE_OK) {
			DBG_LOGE("sqlite3_bind_int is failed.\n");
			_internet_bookmark_db_close(stmt);
			return NULL;
		}
	}

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
				memcpy(m_list->item[i].creationdate,
				       creationdate, length);
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
				memcpy(m_list->item[i].updatedate, updatedate,
				       length);
			}
		}

		m_list->item[i].orderIndex = sqlite3_column_int(stmt, 8);
		i++;
	}
	m_list->count = i;

	if (i <= 0) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_step is failed");
		__bookmark_db_close();
		internet_bookmark_free(m_list);
		return NULL;
	}
	_internet_bookmark_db_close(stmt);
	return m_list;

}

int internet_bookmark_new_by_vbm(char *vbmfile, int folder_id,
				 int overwrite_flag)
{
	/* not support */
	return 0;

}

int internet_bookmark_reset()
{
	__BOOKMARK_DEBUG_TRACE(" ");

	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed\n");
		return -1;
	}

	nError =
	    sqlite3_prepare_v2(m_internet_bookmark_db,
			       "delete from bookmarks where editable=1", -1,
			       &stmt, NULL);
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

void internet_bookmark_free(bookmark_list *m_list)
{
	TRACE_BEGIN;
	_bmsvc_destroy_bookmark_list((void *)m_list);
	TRACE_END;
}

/* For KIES backup & restore */
char *internet_bookmark_backup()
{
	__BOOKMARK_DEBUG_TRACE(" ");
	int nError;
	char *restore_path = NULL;
	sqlite3_backup *pBackup;	/* Backup object used to copy data */
	sqlite3 *pTo;		/* Database to copy to (pFile or pInMemory) */

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed\n");
		return NULL;
	}
	/* create new Database. */
	nError = sqlite3_open(INTERNET_BOOKMARK_BACKUP_FILE_NAME, &pTo);
	if (nError == SQLITE_OK) {
		pBackup =
		    sqlite3_backup_init(pTo, "main", m_internet_bookmark_db,
					"main");
		do {
			nError = sqlite3_backup_step(pBackup, 5);
			int nRemain = sqlite3_backup_remaining(pBackup);
			int nPageCount = sqlite3_backup_pagecount(pBackup);
			int nPercent =
			    100 * (nPageCount - nRemain) / nPageCount;
			printf("Backup ...  %d(%%) [%d/%d]\n", nPercent,
			       nRemain, nPageCount);
			if (nError == SQLITE_OK || nError == SQLITE_BUSY
			    || nError == SQLITE_LOCKED) {
				sqlite3_sleep(250);
			}
		} while (nError == SQLITE_OK || nError == SQLITE_BUSY
			 || nError == SQLITE_LOCKED);
		(void)sqlite3_backup_finish(pBackup);
		nError = sqlite3_errcode(pTo);
		(void)sqlite3_close(pTo);
		__bookmark_db_close();
		/* allocation file path and then reture to caller. */
		restore_path =
		    (char *)calloc(strlen(INTERNET_BOOKMARK_BACKUP_FILE_NAME),
				   sizeof(char));
		memcpy(restore_path, INTERNET_BOOKMARK_BACKUP_FILE_NAME,
		       strlen(INTERNET_BOOKMARK_BACKUP_FILE_NAME));
		return restore_path;
	}
	(void)sqlite3_close(pTo);
	__bookmark_db_close();
	return NULL;

}

int internet_bookmark_restore(char *restore_path)
{
	__BOOKMARK_DEBUG_TRACE(" ");
	int nError;
	sqlite3_backup *pBackup;	/* Backup object used to copy data */
	sqlite3 *pFrom;		/* Database to copy to (pFile or pInMemory) */

	if (_file_is_exist(restore_path) <= 0) {
		__BOOKMARK_DEBUG_TRACE("Illegal Param [%s]", restore_path);
		return -1;
	}

	_internet_bookmark_reset();	/* clear all item */

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed\n");
		return -1;
	}
	/* create new Database. */
	nError = sqlite3_open(restore_path, &pFrom);
	if (nError == SQLITE_OK) {
		pBackup =
		    sqlite3_backup_init(m_internet_bookmark_db, "main", pFrom,
					"main");
		do {
			nError = sqlite3_backup_step(pBackup, 5);
			int nRemain = sqlite3_backup_remaining(pBackup);
			int nPageCount = sqlite3_backup_pagecount(pBackup);
			int nPercent =
			    100 * (nPageCount - nRemain) / nPageCount;
			printf("Restore ...  %d(%%) [%d/%d]\n", nPercent,
			       nRemain, nPageCount);
			if (nError == SQLITE_OK || nError == SQLITE_BUSY
			    || nError == SQLITE_LOCKED) {
				sqlite3_sleep(250);
			}
		} while (nError == SQLITE_OK || nError == SQLITE_BUSY
			 || nError == SQLITE_LOCKED);
		(void)sqlite3_backup_finish(pBackup);
		nError = sqlite3_errcode(pFrom);
		(void)sqlite3_close(pFrom);
		__bookmark_db_close();
		return 0;
	}
	(void)sqlite3_close(pFrom);
	__bookmark_db_close();
	return -1;
}

void internet_bookmark_entry_free(bookmark_entry *entry)
{
	TRACE_BEGIN;
	_bmsvc_destroy_bookmark_entry((void *)entry);
	TRACE_END;
}

bookmark_entry *internet_bookmark_get_bookmark_by_id(int id)
{
	__BOOKMARK_DEBUG_TRACE("id :%d", id);

	bookmark_entry *entry = NULL;
	int nError;
	sqlite3_stmt *stmt;

	if (_internet_bookmark_db_open() < 0) {
		__BOOKMARK_DEBUG_TRACE("db_util_open is failed\n");
		return NULL;
	}

	nError = sqlite3_prepare_v2(m_internet_bookmark_db,
					"select type,parent,address,title,sequence,editable,strftime('%s',creationdate),updatedate\
					from bookmarks where id=?",
					-1, &stmt, NULL);
	if (nError != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("SQL error=%d", nError);
		_internet_bookmark_db_close(stmt);
		return NULL;
	}

	if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK) {
		__BOOKMARK_DEBUG_TRACE("sqlite3_bind_int is failed.\n");
		return NULL;
	}

	if ((nError = sqlite3_step(stmt)) == SQLITE_ROW) {
		entry = (bookmark_entry *)calloc(1, sizeof(bookmark_entry));
		if (!entry) {
			__BOOKMARK_DEBUG_TRACE("bookmark_item new is failed.\n");
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

		entry->orderIndex = sqlite3_column_int(stmt, 4);
		entry->editable = sqlite3_column_int(stmt, 5);
		_internet_bookmark_db_close(stmt);
		return entry;
	}

	__bookmark_db_close();
	return NULL;
}
