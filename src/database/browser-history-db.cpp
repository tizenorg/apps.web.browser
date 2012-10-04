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
 *
 */


using namespace std;

#include "browser-history-db.h"

sqlite3* Browser_History_DB::m_db_descriptor = NULL;

Browser_History_DB::Browser_History_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_History_DB::~Browser_History_DB(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Eina_Bool Browser_History_DB::_open_db(void)
{
	BROWSER_LOGD("[%s]", __func__);
	int error = db_util_open(BROWSER_HISTORY_DB_PATH, &m_db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool Browser_History_DB::_close_db(void)
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

Eina_Bool Browser_History_DB::save_history(const char *url, const char *title, Eina_Bool *is_full)
{
	if (url == NULL || title == NULL || strlen(url) == 0 || strlen(title) == 0)
		return EINA_FALSE;

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select counter from history where address=?",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	error = sqlite3_bind_text(sqlite3_stmt, 1, url, -1, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	error = sqlite3_step(sqlite3_stmt);
	if (error == SQLITE_ROW) {
		int count = sqlite3_column_int(sqlite3_stmt, 0);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			_close_db();
			return EINA_FALSE;
		}

		error = sqlite3_prepare_v2(m_db_descriptor, "update history set counter=?, title=?, visitdate=DATETIME('now') where address=?",
						-1, &sqlite3_stmt, NULL);
		if (error != SQLITE_OK) {
			BROWSER_LOGD("SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}

		if (sqlite3_bind_int(sqlite3_stmt, 1, count + 1) != SQLITE_OK) {
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}

		if (sqlite3_bind_text(sqlite3_stmt, 2, title, -1, NULL) != SQLITE_OK) {
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}

		if (sqlite3_bind_text(sqlite3_stmt, 3, url, -1, NULL) != SQLITE_OK) {
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}

		error = sqlite3_step(sqlite3_stmt);
	} else {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			_close_db();
			return EINA_FALSE;
		}

		error = sqlite3_prepare_v2(m_db_descriptor, "select count(*) from history", -1, &sqlite3_stmt, NULL);
		if (error != SQLITE_OK) {
			BROWSER_LOGD("SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}

		error = sqlite3_step(sqlite3_stmt);
		if (error != SQLITE_ROW) {
			BROWSER_LOGD("SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}

		unsigned history_count = sqlite3_column_int(sqlite3_stmt, 0);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			_close_db();
			return EINA_FALSE;
		}

		if (history_count >= BROWSER_HISTORY_COUNT_LIMIT) {
			string delete_query = "delete from history where id = (select min(id) from history)";
			if (history_count > BROWSER_HISTORY_COUNT_LIMIT) {
				unsigned delete_count = history_count - BROWSER_HISTORY_COUNT_LIMIT + 1;
				char text[128] = {0, };
				sprintf(text, "delete from history where id in (select id from history limit %d)", delete_count);
				delete_query = text;
			}

			error = sqlite3_prepare_v2(m_db_descriptor, delete_query.c_str(), -1, &sqlite3_stmt, NULL);
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
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
				_close_db();
				return EINA_FALSE;
			}
		}

		const string insert_url(url);
		const string insert_title(title);

		const string statement = "insert into history (address, title, counter, visitdate) values('" + insert_url 
						+ "','" + insert_title + "', 0, DATETIME('now'))";
		error = sqlite3_prepare_v2(m_db_descriptor, statement.c_str(), -1, &sqlite3_stmt, NULL);
		if (error != SQLITE_OK) {
			BROWSER_LOGD("SQL error=%d", error);
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
				BROWSER_LOGE("sqlite3_finalize is failed.\n");
			_close_db();
			return EINA_FALSE;
		}

		error = sqlite3_step(sqlite3_stmt);

		if (error == SQLITE_FULL)
			*is_full = EINA_TRUE;
		else
			*is_full = EINA_FALSE;
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		_close_db();
		return EINA_FALSE;
	}

	_close_db();

	return (error == SQLITE_DONE);
}

Eina_Bool Browser_History_DB::delete_history(const char *url)
{
	if (!url || strlen(url) == 0) {
		BROWSER_LOGE("url is empty");
		return EINA_FALSE;
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "delete from history where address=?",
								-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_text(sqlite3_stmt, 1, url, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.");

	error = sqlite3_step(sqlite3_stmt);
	if (error != SQLITE_OK && error != SQLITE_DONE) {
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

Eina_Bool Browser_History_DB::delete_history(int history_id)
{
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "delete from history where id=?",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_int(sqlite3_stmt, 1, history_id) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_int is failed.\n");
	error = sqlite3_step(sqlite3_stmt);
	if (error != SQLITE_OK && error != SQLITE_DONE) {
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

Eina_Bool Browser_History_DB::get_most_visited_list(vector<most_visited_item> &list)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;

	int error = sqlite3_prepare_v2(m_db_descriptor, "select address, title from history order by counter desc limit "BROWSER_MOST_VISITED_COUNT_TEXT,
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}


	most_visited_item item;
	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		item.url = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 0));
		item.title = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 1));

		list.push_back(item);
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	_close_db();

	return (error == SQLITE_DONE || error == SQLITE_ROW);
}

Eina_Bool Browser_History_DB::get_history_list_by_partial_url(const char *url, int count, std::vector<std::string> &list)
{
	if (!url || strlen(url) == 0) {
		BROWSER_LOGE("partial_url is empty");
		return EINA_FALSE;
	}

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	std::string query = "select address, counter from history where address like '%" + std::string(url) + "%'";

	int error = sqlite3_prepare_v2(m_db_descriptor, query.c_str(), -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	int i;
	for (i = 0; i < count; i++) {
		error = sqlite3_step(sqlite3_stmt);
		if (error == SQLITE_ROW) {
			std::string mark_up = std::string(reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 0)));
			size_t index = mark_up.find(BROWSER_HTTP_SCHEME);
			if(index == 0) {
				index = mark_up.find(url, strlen(BROWSER_HTTP_SCHEME));
				if(index == -1)
					continue;
			}

			index = mark_up.find(BROWSER_HTTPS_SCHEME);
			if(index == 0) {
				index = mark_up.find(url, strlen(BROWSER_HTTPS_SCHEME));
				if(index == -1)
					continue;
			}

			list.push_back(std::string(reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 0))));
		} else
			break;
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	_close_db();

	if (error == SQLITE_DONE || error == SQLITE_ROW)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

Eina_Bool Browser_History_DB::get_history_list(vector<history_item*> &list)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select id, address, title, visitdate from history order by visitdate desc",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	history_item *item;
	while(1) {
		error = sqlite3_step(sqlite3_stmt);
		if (error == SQLITE_ROW) {
			item = new(nothrow) history_item;
			if (!item) {
				BROWSER_LOGD("new history_item is failed");
				return EINA_FALSE;
			}
			const char* id = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 0));
			if (id && strlen(id))
				item->id = atoi(id);
			else
				item->id = 0;

			const char* url = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 1));
			if (url && strlen(url))
				item->url = url;
			else
				item->url = "";
			
			const char* title = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 2));
			if (title && strlen(title))
				item->title = title;
			else
				item->title = "";

			const char* date = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 3));
			if (date && strlen(date))
				item->date = date;
			else
				item->date = "";

			item->user_data = NULL;
			item->is_delete = EINA_FALSE;

			list.push_back(item);
		} else
			break;
	}

	BROWSER_LOGD("SQL error: %d", error);\
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	_close_db();
	if (error == SQLITE_DONE || error == SQLITE_ROW)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

Eina_Bool Browser_History_DB::clear_history(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "delete from history", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
	}

	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	_close_db();
	if (error == SQLITE_DONE || error == SQLITE_ROW)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

Eina_Bool Browser_History_DB::is_in_bookmark(const char* url, int *bookmark_id)
{
	int error = db_util_open(BROWSER_BOOKMARK_DB_PATH, &m_db_descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		m_db_descriptor = NULL;
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(m_db_descriptor, "select id from bookmarks where address=?",
									-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		db_util_close(m_db_descriptor);
		return EINA_FALSE;
	}

	if (sqlite3_bind_text(sqlite3_stmt, 1, url, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.\n");

	error = sqlite3_step(sqlite3_stmt);

	if (bookmark_id)
		*bookmark_id = sqlite3_column_int(sqlite3_stmt, 0);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");

	db_util_close(m_db_descriptor);

	return (error == SQLITE_ROW);
}

#ifdef STORE_FAVICON
Eina_Bool Browser_History_DB::save_history_icon(const char* url, const Evas_Object *icon)
{
	BROWSER_LOGD("[%s]", __func__);

	if (url == NULL || strlen(url) == 0) {
		BROWSER_LOGE("url is NULL");
		return EINA_FALSE;
	}

	if (icon == NULL) {
		BROWSER_LOGE("icon is NULL");
		return EINA_FALSE;
	}

	int icon_w = 0;
	int icon_h = 0;
	int stride = 0;
	int icon_length = 0;
	void *icon_data = (void *)evas_object_image_data_get(icon, EINA_TRUE);
	evas_object_image_size_get(icon, &icon_w, &icon_h);
	stride = evas_object_image_stride_get(icon);
			icon_length = icon_h * stride;
	BROWSER_LOGD("favicon size  w:%d, h:%d, stride: %d", icon_w, icon_h, stride);

	if (_open_db() == EINA_FALSE)
		return EINA_FALSE;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "update history set favicon=?, favicon_length=?, favicon_w=?, favicon_h=? where address=?",
						-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

		/* binding values - favicon */
	if (sqlite3_bind_blob(sqlite3_stmt, 1, icon_data , icon_length, NULL) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_blob is failed.\n");
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	/* binding values - favicon length */
	if (sqlite3_bind_int(sqlite3_stmt, 2, icon_length) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_int (icon_length)is failed.\n");
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	/* binding values - favicon width */
	if (sqlite3_bind_int(sqlite3_stmt, 3, icon_w) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_int(icon_w) is failed.\n");
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	/* binding values - favicon height */
	if (sqlite3_bind_int(sqlite3_stmt, 4, icon_h) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_int(icon_h) is failed.\n");
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	if (sqlite3_bind_text(sqlite3_stmt, 5, url, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_text(url) is failed.\n");
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return EINA_FALSE;
	}

	error = sqlite3_step(sqlite3_stmt);

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		_close_db();
		return EINA_FALSE;
	}

	_close_db();

	return (error == SQLITE_DONE);
}

Evas_Object *Browser_History_DB::get_history_icon(Evas_Object *parent, const char* url)
{
	BROWSER_LOGD("[%s]", __func__);

	if (parent == NULL) {
		BROWSER_LOGE("url is NULL");
		return NULL;
	}

	if (url == NULL || strlen(url) == 0) {
		BROWSER_LOGE("url is NULL");
		return NULL;
	}

	Evas_Object *icon = NULL;
	Evas *e = NULL;
	e = evas_object_evas_get(parent);

	if (!e) {
		BROWSER_LOGE("canvas is NULL");
		return NULL;
	}

	if (_open_db() == EINA_FALSE)
		return NULL;

	sqlite3_stmt *sqlite3_stmt = NULL;
	int error = sqlite3_prepare_v2(m_db_descriptor, "select favicon, favicon_length, favicon_w, favicon_h from history where address=?",
					-1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return NULL;
	}

	if (sqlite3_bind_text(sqlite3_stmt, 1, url, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_text(url) is failed.\n");
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		_close_db();
		return NULL;
	}

	error = sqlite3_step(sqlite3_stmt);

	if (error == SQLITE_ROW) {
		int icon_w = 0;
		int icon_h = 0;
		int stride = 0;
		int icon_length = 0;
		void *icon_data = NULL;
		void *icon_data_temp = NULL;
		icon_data_temp = (void *)sqlite3_column_blob(sqlite3_stmt,0);
		icon_length = sqlite3_column_int(sqlite3_stmt,1);
		icon_w = sqlite3_column_int(sqlite3_stmt,2);
		icon_h = sqlite3_column_int(sqlite3_stmt,3);
		BROWSER_LOGD("icon_length(%d), icon_w(%d), icon_h(%d)", icon_length, icon_w, icon_h);

		if (icon_length > 0){
			icon_data = new(nothrow) char[icon_length];
			memcpy(icon_data, icon_data_temp, icon_length);

			icon = evas_object_image_filled_add(e);
			evas_object_image_colorspace_set(icon, EVAS_COLORSPACE_ARGB8888);
			evas_object_image_size_set(icon, icon_w, icon_h);
			evas_object_image_fill_set(icon, 0, 0, icon_w, icon_h);
			evas_object_image_filled_set(icon, EINA_TRUE);
			evas_object_image_alpha_set(icon,EINA_TRUE);
			evas_object_image_data_set(icon, icon_data);
		}
	}

	BROWSER_LOGD("SQL error: %d, %p", error, icon);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	_close_db();
	return icon;
}
#endif
