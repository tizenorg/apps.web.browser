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

extern "C" {
#include "db-util.h"
}

#include "browser-object.h"

#include "history.h"
#include "history-item.h"
#include "browser-dlog.h"
#include "platform-service.h"

#define history_db_path browser_data_dir"/db/.browser-history.db"

#define HISTORY_COUNT_LIMIT	1000

history::history(void)
{
	BROWSER_LOGD("");
}

history::~history(void)
{
	BROWSER_LOGD("");
}

static Eina_Bool _is_exist(const char *title, const char *uri, int *counter)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN_VAL(title, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(history_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select counter from history where address=?", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGE("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	error = sqlite3_bind_text(sqlite3_stmt, 1, uri, -1, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	error = sqlite3_step(sqlite3_stmt);
	if (error == SQLITE_ROW) {
		if (counter) {
			*counter = sqlite3_column_int(sqlite3_stmt, 0);
			BROWSER_LOGD("visit counter=[%d]", *counter);
		}
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize failed");
		db_util_close(descriptor);
		return EINA_TRUE;
	} else {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize failed");
		db_util_close(descriptor);
	}

	if (counter)
		*counter = 0;

	return EINA_FALSE;
}

static Eina_Bool _update_history(const char *title, const char *uri, int count)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(title, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(history_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "update history set counter=?, visitdate=DATETIME('now') where address=? and title=?", -1, &sqlite3_stmt, NULL);
	if (sqlite3_bind_int(sqlite3_stmt, 1, count + 1) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 2, uri, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 3, title, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	db_util_close(descriptor);

	return EINA_TRUE;
}

static Eina_Bool _save_history(const char *title, const char *uri)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(title, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(history_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "insert into history (address, title, counter, visitdate) values(?, ?, 1, DATETIME('now'))", -1, &sqlite3_stmt, NULL);
	if (sqlite3_bind_text(sqlite3_stmt, 1, uri, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 2, title, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	db_util_close(descriptor);

	return EINA_TRUE;
}

static Eina_Bool _update_history_with_snapshot(const char *title, const char *uri, int count, Evas_Object *snapshot)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(title, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(snapshot, EINA_FALSE);

	int w = 0;
	int h = 0;
	int stride = 0;
	int len = 0;

	platform_service ps;
	ps.evas_image_size_get(snapshot, &w, &h, &stride);

	BROWSER_LOGD("snapshot w=[%d], h=[%d], stride=[%d]", w, h, stride);

	len = stride * h;

	if (len == 0)
		return EINA_FALSE;

	void *snapshot_data = evas_object_image_data_get(snapshot, EINA_TRUE);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(history_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "update history set counter=?, snapshot=?, snapshot_stride=?, snapshot_w=?, snapshot_h=?, visitdate=DATETIME('now') where address=? and title=?", -1, &sqlite3_stmt, NULL);
	if (sqlite3_bind_int(sqlite3_stmt, 1, count + 1) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_blob(sqlite3_stmt, 2, snapshot_data, len, NULL) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_blob is failed.\n");
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 3, stride) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 4, w) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 5, h) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 6, uri, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 7, title, -1, NULL) != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	db_util_close(descriptor);
	return EINA_TRUE;
}

static Eina_Bool _save_history_with_snapshot(const char *title, const char *uri, Evas_Object *snapshot)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(title, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(snapshot, EINA_FALSE);

	int w = 0;
	int h = 0;
	int stride = 0;
	int len = 0;

	platform_service ps;
	ps.evas_image_size_get(snapshot, &w, &h, &stride);

	BROWSER_LOGD("snapshot w=[%d], h=[%d], stride=[%d]", w, h, stride);

	len = stride * h;

	if (len == 0)
		return EINA_FALSE;

	void *snapshot_data = evas_object_image_data_get(snapshot, EINA_TRUE);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(history_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "insert into history (address, title, counter, visitdate, snapshot, snapshot_stride, snapshot_w, snapshot_h) values(?, ?, 1, DATETIME('now'), ?, ?, ?, ?)", -1, &sqlite3_stmt, NULL);
	if (sqlite3_bind_text(sqlite3_stmt, 1, uri, -1, NULL) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 2, title, -1, NULL) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_blob(sqlite3_stmt, 3, snapshot_data, len, NULL) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_blob is failed.\n");
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 4, stride) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 5, w) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 6, h) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	db_util_close(descriptor);

	return EINA_TRUE;
}

static Eina_Bool _remove_oldest_history_item(void)
{
	sqlite3 *descriptor = NULL;
	int error = db_util_open(history_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	sqlite3_prepare_v2(descriptor, "select count(*) from history", -1, &sqlite3_stmt, NULL);
	sqlite3_step(sqlite3_stmt);
	int count = sqlite3_column_int(sqlite3_stmt, 0);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	if (count > HISTORY_COUNT_LIMIT) {
		sqlite3_prepare_v2(descriptor, "delete from history where id = (select min(id) from history)", -1, &sqlite3_stmt, NULL);
		sqlite3_step(sqlite3_stmt);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			db_util_close(descriptor);
			return EINA_FALSE;
		}
	}

	db_util_close(descriptor);

	return EINA_TRUE;
}

std::vector<history_item *> history::get_history_list(void)
{
	std::vector<history_item *> history_list;
	sqlite3 *descriptor = NULL;
	int error = db_util_open(history_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return history_list;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select id, address, title, visitdate from history order by visitdate desc", -1, &sqlite3_stmt, NULL);
	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		const char *uri = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 1));
		const char *title = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 2));
		BROWSER_LOGD("uri=[%s], title=[%s]", uri, title);
		if (!uri || !strlen(uri) || !title || !strlen(title)) {
			BROWSER_LOGE("invalid history");
			continue;
		}
		const char *date = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 3));
		BROWSER_LOGD("date=[%s]", date);

		history_item *item = new history_item(title, uri, NULL, 0, date);
		history_list.push_back(item);
	}

	return history_list;
}

std::vector<history_item *> history::get_histories_order_by_visit_count(int count)
{
	std::vector<history_item *> history_list;

	if (count == 0)
		return history_list;

	sqlite3 *descriptor = NULL;
	int error = db_util_open(history_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return history_list;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select address, title, snapshot, snapshot_w, snapshot_h, snapshot_stride, counter from history order by counter desc", -1, &sqlite3_stmt, NULL);

	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		Evas_Object *image = NULL;
		const char *uri = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 0));
		const char *title = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 1));

		int visit_count = sqlite3_column_int(sqlite3_stmt, 6);
		if (visit_count < 1)
			break;

		int stride = (int)sqlite3_column_int(sqlite3_stmt, 5);
		if (stride) {
			void *snapshot_data = (void *)sqlite3_column_blob(sqlite3_stmt, 2);
			int w = (int)sqlite3_column_int(sqlite3_stmt, 3);
			int h = (int)sqlite3_column_int(sqlite3_stmt, 4);

			int raw_data_length = h * stride;
			image = evas_object_image_filled_add(evas_object_evas_get(m_window));
			evas_object_image_colorspace_set(image, EVAS_COLORSPACE_ARGB8888);
			evas_object_image_size_set(image, w, h);
			evas_object_image_fill_set(image, 0, 0, w, h);
			evas_object_image_filled_set(image, EINA_TRUE);
			evas_object_image_alpha_set(image,EINA_TRUE);

			void *pixels = evas_object_image_data_get(image, EINA_TRUE);
			memcpy(pixels, snapshot_data, raw_data_length);
			evas_object_image_data_set(image, pixels);
		}

		history_item *item = new history_item(title, uri, image, visit_count);
		history_list.push_back(item);
		if (history_list.size() >= count)
			break;
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		return history_list;
	}

	db_util_close(descriptor);

	return history_list;
}

void history::reset_history_visit_count(history_item *item)
{
	EINA_SAFETY_ON_NULL_RETURN(item);
	if (!_update_history(item->get_title(), item->get_uri(), -1))
		BROWSER_LOGE("_update_history failed");
}

history_item *history::get_next_history_order_by_visit_count(history_item *item)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(item, NULL);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(history_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return NULL;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select address, title, snapshot, snapshot_w, snapshot_h, snapshot_stride, counter from history order by counter desc", -1, &sqlite3_stmt, NULL);

	Eina_Bool is_found = EINA_FALSE;
	while ((error = sqlite3_step(sqlite3_stmt)) == SQLITE_ROW) {
		Evas_Object *image = NULL;
		const char *uri = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 0));
		const char *title = reinterpret_cast<const char *>(sqlite3_column_text(sqlite3_stmt, 1));

		BROWSER_LOGD("uri=[%s], title=[%s], is_found=[%d]", uri, title, is_found);
		if (!strcmp(uri, item->get_uri()) && !strcmp(title, item->get_title())) {
			is_found = EINA_TRUE;
			continue;
		}

		if (!is_found)
			continue;

		int visit_count = (int)sqlite3_column_int(sqlite3_stmt, 6);
		BROWSER_LOGD("visit_count=%d", visit_count);
		if (visit_count < 1) {
			if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
				db_util_close(descriptor);
				return NULL;
			}
			db_util_close(descriptor);
			return NULL;
		}

		int stride = (int)sqlite3_column_int(sqlite3_stmt, 5);
		if (stride) {
			void *snapshot_data = (void *)sqlite3_column_blob(sqlite3_stmt, 2);
			int w = (int)sqlite3_column_int(sqlite3_stmt, 3);
			int h = (int)sqlite3_column_int(sqlite3_stmt, 4);

			int raw_data_length = h * stride;
			image = evas_object_image_filled_add(evas_object_evas_get(m_window));
			evas_object_image_colorspace_set(image, EVAS_COLORSPACE_ARGB8888);
			evas_object_image_size_set(image, w, h);
			evas_object_image_fill_set(image, 0, 0, w, h);
			evas_object_image_filled_set(image, EINA_TRUE);
			evas_object_image_alpha_set(image,EINA_TRUE);

			void *pixels = evas_object_image_data_get(image, EINA_TRUE);
			memcpy(pixels, snapshot_data, raw_data_length);
			evas_object_image_data_set(image, pixels);
		}

		history_item *item = new history_item(title, uri, image, visit_count);

		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
			BROWSER_LOGD("sqlite3_finalize error");
			db_util_close(descriptor);
			return NULL;
		}
		db_util_close(descriptor);

		BROWSER_LOGD("item=%d", item);
		return item;
	}

	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		return NULL;
	}

	db_util_close(descriptor);

	return NULL;
}

Eina_Bool history::delete_all(void)
{
	sqlite3 *descriptor = NULL;
	int error = db_util_open(history_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "delete from history", -1, &sqlite3_stmt, NULL);
	sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_finalize failed");
		return EINA_FALSE;
	}

	db_util_close(descriptor);

	return EINA_TRUE;
}

Eina_Bool history::save_history(const char *title, const char *uri, Evas_Object *snapshot, int *visit)
{
	BROWSER_LOGD("title=[%s], uri=[%s]", title, uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(title, EINA_FALSE);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	int visit_count = 0;
	if (_is_exist(title, uri, &visit_count)) {
		if (snapshot) {
			if (!_update_history_with_snapshot(title, uri, visit_count, snapshot))
				BROWSER_LOGE("_update_history_with_snapshot failed");
		} else {
			if (!_update_history(title, uri, visit_count))
				BROWSER_LOGE("_update_history failed");
		}
	} else {
		if (snapshot) {
			if (!_save_history_with_snapshot(title, uri, snapshot))
				BROWSER_LOGE("_save_history_with_snapshot failed");
		} else {
			if (!_save_history(title, uri))
				BROWSER_LOGE("_save_history failed");
		}

		_remove_oldest_history_item();
	}

	if (visit)
		*visit = visit_count + 1;

	return EINA_TRUE;
}

Eina_Bool history::delete_history(const char *uri)
{
	BROWSER_LOGD("uri=[%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, EINA_FALSE);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(history_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "delete from history where address=?", -1, &sqlite3_stmt, NULL);
	if (sqlite3_bind_text(sqlite3_stmt, 1, uri, -1, NULL) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_bind_text is failed.");

	sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_finalize failed");
		return EINA_FALSE;
	}

	db_util_close(descriptor);

	return EINA_TRUE;
}

Eina_Bool history::set_history_favicon(const char *uri, Evas_Object *icon)
{
	if (uri == NULL) {
		BROWSER_LOGD("uri is null");
		return EINA_FALSE;
	}
	if (icon == NULL) {
		BROWSER_LOGD("icon is null");
		return EINA_FALSE;
	}

	int w = 0;
	int h = 0;
	int stride = 0;
	int len = 0;

	platform_service ps;
	ps.evas_image_size_get(icon, &w, &h, &stride);

	BROWSER_LOGD("favicon w=[%d], h=[%d], stride=[%d]", w, h, stride);

	len = stride * h;

	if (len == 0)
		return EINA_FALSE;

	void *icon_data = evas_object_image_data_get(icon, EINA_TRUE);

	sqlite3 *descriptor = NULL;
	int error = db_util_open(history_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "update history set favicon=?, favicon_length=?, favicon_w=?, favicon_h=? where address=?", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_blob(sqlite3_stmt, 1, icon_data, len, NULL) != SQLITE_OK) {
		BROWSER_LOGE("sqlite3_bind_blob is failed.\n");
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 2, len) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 3, w) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_int(sqlite3_stmt, 4, h) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 5, uri, -1, NULL) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return NULL;
	}
	error = sqlite3_step(sqlite3_stmt);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK) {
		db_util_close(descriptor);
		return EINA_FALSE;
	}

	db_util_close(descriptor);
	return EINA_TRUE;
}

Evas_Object *history::get_history_favicon(const char *uri)
{
	BROWSER_LOGD("uri = [%s]", uri);
	EINA_SAFETY_ON_NULL_RETURN_VAL(uri, NULL);
	sqlite3 *descriptor = NULL;
	Evas_Object *icon = NULL;
	Evas *e = NULL;
	e = evas_object_evas_get(m_window);

	if (!e) {
		BROWSER_LOGE("canvas is NULL");
		return NULL;
	}

	int error = db_util_open(history_db_path, &descriptor, DB_UTIL_REGISTER_HOOK_METHOD);
	if (error != SQLITE_OK) {
		db_util_close(descriptor);
		return NULL;
	}

	sqlite3_stmt *sqlite3_stmt = NULL;
	error = sqlite3_prepare_v2(descriptor, "select favicon, favicon_length, favicon_w, favicon_h from history where address=?", -1, &sqlite3_stmt, NULL);
	if (error != SQLITE_OK) {
		BROWSER_LOGD("SQL error=%d", error);
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.");
		db_util_close(descriptor);
		return EINA_FALSE;
	}
	if (sqlite3_bind_text(sqlite3_stmt, 1, uri, -1, NULL) != SQLITE_OK) {
		if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
			BROWSER_LOGE("sqlite3_finalize is failed.\n");
		db_util_close(descriptor);
		return NULL;
	}
	error = sqlite3_step(sqlite3_stmt);
	if (error == SQLITE_ROW) {
		int icon_w = 0;
		int icon_h = 0;
		int stride = 0;
		int icon_length = 0;
		//void *icon_data = NULL;
		void *icon_data_temp = NULL;
		icon_data_temp = (void *)sqlite3_column_blob(sqlite3_stmt,0);
		icon_length = sqlite3_column_int(sqlite3_stmt,1);
		icon_w = sqlite3_column_int(sqlite3_stmt,2);
		icon_h = sqlite3_column_int(sqlite3_stmt,3);
		BROWSER_LOGD("icon_length(%d), icon_w(%d), icon_h(%d)", icon_length, icon_w, icon_h);

		if (icon_length > 0){
			//icon_data = new(nothrow) char[icon_length];
			//memcpy(icon_data, icon_data_temp, icon_length);

			icon = evas_object_image_filled_add(e);
			evas_object_image_colorspace_set(icon, EVAS_COLORSPACE_ARGB8888);
			evas_object_image_size_set(icon, icon_w, icon_h);
			evas_object_image_fill_set(icon, 0, 0, icon_w, icon_h);
			evas_object_image_filled_set(icon, EINA_TRUE);
			evas_object_image_alpha_set(icon,EINA_TRUE);
			//evas_object_image_data_set(icon, icon_data);

			void *pixels = evas_object_image_data_get(icon, EINA_TRUE);
			memcpy(pixels, icon_data_temp, icon_length);
			evas_object_image_data_set(icon, pixels);
		}
	}

	BROWSER_LOGD("SQL error: %d, %p", error, icon);
	if (sqlite3_finalize(sqlite3_stmt) != SQLITE_OK)
		BROWSER_LOGE("sqlite3_finalize is failed.\n");
	db_util_close(descriptor);
	return icon;
}
