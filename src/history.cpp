/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contact: Jihye Song <jihye3.song@samsung.com>
 *
 */

extern "C" {
#include "db-util.h"
}

#include <storage.h>
#include <string>

#include "browser-object.h"

#include "browser.h"
#include "browser-view.h"
#include "history.h"
#include "history-item.h"
#include "browser-dlog.h"
#include "platform-service.h"
#include "webview.h"
#include "web_history.h"

#define HISTORY_COUNT_LIMIT	1000
#define most_visited_item_count 6

Ecore_Thread *history::m_current_thread;
THREAD_CANCEL_CB  history::m_thread_cancel_cb;
THREAD_END_CB history::m_thread_end_cb;
void *history::m_thread_data;


static int __get_duplicated_ids_p(int **ids, int *count, const int limit, const int offset,
	const bp_history_offset order_column_offset, const int ordering,
	const bp_history_offset check_column_offset,
	const char *keyword, const int is_like)
{
	bp_history_rows_cond_fmt conds;
	memset(&conds, 0x00, sizeof(bp_history_rows_cond_fmt));

	conds.limit = limit;
	conds.offset = offset;
	conds.ordering = ordering;
	conds.order_offset = order_column_offset;
	conds.period_offset = BP_HISTORY_O_DATE_CREATED;
	conds.period_type = BP_HISTORY_DATE_ALL;

	return bp_history_adaptor_get_cond_ids_p
			(ids, count,
			&conds,
			check_column_offset,
			keyword,
			is_like);
}

static int __get_inquired_ids_p(int **ids, int *count, const int limit, const int offset,
	const bp_history_offset order_column_offset, const int ordering,
	const char *keyword, const int is_like)
{
	bp_history_rows_cond_fmt conds;
	memset(&conds, 0x00, sizeof(bp_history_rows_cond_fmt));

	conds.limit = limit;
	conds.offset = offset;
	conds.ordering = ordering;
	conds.order_offset = order_column_offset;
	conds.period_offset = BP_HISTORY_O_DATE_CREATED;
	conds.period_type = BP_HISTORY_DATE_ALL;

	return bp_history_adaptor_get_cond_ids_p
			(ids, count,
			&conds,
			(BP_HISTORY_O_TITLE | BP_HISTORY_O_URL),
			keyword,
			is_like);
}

static int  __get_date_ids_p
	(int **ids, int *count, const int limit, const int offset,
	const bp_history_offset order_column_offset, const int ordering,
	const bp_history_offset date_column_offset,
	const bp_history_date_defs date_type)
{
	bp_history_rows_cond_fmt conds;
	conds.limit = limit;
	conds.offset = offset;
	conds.order_offset = order_column_offset;
	conds.ordering = ordering;
	conds.period_offset = date_column_offset;
	conds.period_type = date_type;
	return bp_history_adaptor_get_cond_ids_p(ids, count, &conds,
		0, NULL, 0);
}

history::history(void)
	: m_memory_full(EINA_FALSE)
	, m_history_adaptor_initialize(EINA_TRUE)
{
	BROWSER_LOGD("");
	if (bp_history_adaptor_initialize() < 0) {
		m_history_adaptor_initialize = EINA_FALSE;
		BROWSER_LOGE("bp_history_adaptor_initialize failed");
	} else
		m_history_adaptor_initialize = EINA_TRUE;
}

history::~history(void)
{
	BROWSER_LOGD("");
	bp_history_adaptor_deinitialize();
}

Eina_Bool history::save_history(const char *title, const char *uri, Evas_Object *snapshot, Evas_Object *favicon, int *visit)
{
	BROWSER_SECURE_LOGD("title=[%s], uri=[%s]", title, uri);
	RETV_MSG_IF(!title, EINA_FALSE, "title is EINA_FALSE");
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is EINA_FALSE");

	if (!m_history_adaptor_initialize) {
		if (bp_history_adaptor_initialize() < 0) {
			if (bp_history_adaptor_get_errorcode() == BP_HISTORY_ERROR_DISK_FULL)
				m_memory_full = EINA_TRUE;
			else
				m_memory_full = EINA_FALSE;
			return EINA_FALSE;
		} else
			m_history_adaptor_initialize = EINA_TRUE;
	}

	if (!strcmp(uri,blank_page))
		return EINA_FALSE;

	// return if file:///
	std::string url = uri;
	std::size_t found = url.find("file://");
	if (found!=std::string::npos) {
		BROWSER_SECURE_LOGD("file:// format - will not save to history");
		return EINA_FALSE;
	}

	int id = -1;
	int *ids = NULL;
	int ids_count = -1;
	int visit_number = 0;

	struct statvfs s;
	unsigned long content_size;
	int result;
	result = storage_get_internal_memory_size(&s);
	double available_size = (double)(s.f_bsize) * s.f_bavail;
	m_memory_full = EINA_FALSE;

	if (__get_duplicated_ids_p(&ids, &ids_count, 1, 0, BP_HISTORY_O_DATE_VISITED, 0, BP_HISTORY_O_URL, uri, 0) < 0) {
		BROWSER_LOGE("Failed to check duplicate ids for Uri");
		return EINA_FALSE;
	}

	Eina_Bool ret = EINA_TRUE;
	if (ids_count > 0 && ids != NULL) {

		id = ids[0];

		// search keyword in check_column_offset
		//Check whether page that you loaded already exist in history or not.
		//Check with offset. here offset is url.
		bp_history_adaptor_get_frequency(id, &visit_number);

		BROWSER_LOGD("same URI already exist");

		if (bp_history_adaptor_visit(id) < 0) {
			BROWSER_LOGE("Failed to Update visited Date");
			ret = EINA_FALSE;
		}
	} else {

		content_size = (unsigned long)sizeof(bp_history_info_fmt);

		if (result < 0) {
			BROWSER_LOGE(" Could not get available memory ");
			if (ids)
				free(ids);
			return EINA_FALSE;
		} else if (available_size < content_size) {
			BROWSER_LOGD(" Not enough Memory ");
			m_memory_full = EINA_TRUE;
			if (ids)
				free(ids);
			return EINA_FALSE;
		}

		bp_history_info_fmt info;
		memset(&info, 0x00, sizeof(bp_history_info_fmt));
		info.title = (char *)title;
		info.url = (char *)uri;
		info.date_visited = -1;
		info.frequency = 1;

		if (snapshot) {
			const char *evas_obj_type = evas_object_type_get(snapshot);

			if (evas_obj_type && !strcmp(evas_obj_type, "image")) {
				void *snapshot_data = evas_object_image_data_get(snapshot, EINA_TRUE);

				if (snapshot_data) {
					int w = 0;
					int h = 0;
					int stride = 0;
					int len = 0;

					platform_service ps;
					ps.evas_image_size_get(snapshot, &w, &h, &stride);
					len = stride * h;

					info.thumbnail_height = h;
					info.thumbnail_width = w;
					info.thumbnail_length = len;
					info.thumbnail = (unsigned char *)snapshot_data;
				}
			}
		}

		if (favicon) {
			int w = 0;
			int h = 0;
			int stride = 0;
			int len = 0;

			void *icon_data = evas_object_image_data_get(favicon, EINA_TRUE);

			if (icon_data) {

				platform_service ps;
				ps.evas_image_size_get(favicon, &w, &h, &stride);
				len = stride * h;

				info.favicon_height = h;
				info.favicon_width = w;
				info.favicon_length = len;
				info.favicon = (unsigned char *)icon_data;
			}
		}

		if (bp_history_adaptor_easy_create(&id, &info) < 0) {
			BROWSER_SECURE_LOGD("Failed to Create id for Uri = %s" ,uri);
			ret = EINA_FALSE;
		}
	}
	BROWSER_LOGD(" history => visit_number:[%d]", visit_number);
	if (visit)
		*visit = visit_number;

	if (ret)
		m_browser->notify_history_added(id, title, uri, snapshot, favicon, visit_number);

	free(ids);
	return ret;
}

std::vector<history_item *> history::get_history_list_like(const char *part_text)
{
	BROWSER_LOGD("");
	std::vector<history_item *> history_list;

	int *ids = NULL;
	int ids_count = 0;
	bp_history_info_fmt info;
	char *text = NULL ;

	text = new char [2 + strlen(part_text) + 1];
	strcpy(text, "%");
	strcat(text, part_text);
	strcat(text, "%");
	BROWSER_SECURE_LOGD("PART TEXT = %s", text);

	if (__get_inquired_ids_p(&ids, &ids_count, -1, 0, BP_HISTORY_O_DATE_VISITED, 0, text, 1) < 0) {
		BROWSER_LOGE("Failed to retrive list matching entered text");
	} else {

		BROWSER_LOGE("Total Count matching entered text = %d", ids_count);

		bp_history_offset offset = (BP_HISTORY_O_URL |
				BP_HISTORY_O_TITLE | BP_HISTORY_O_DATE_CREATED);
		for (int i = 0;i < ids_count; i++) {
			if (bp_history_adaptor_get_info(ids[i], offset, &info ) < 0) {
				BROWSER_LOGE("Failed to get Uri info for id = %d", i);
				continue;
			}

			history_item *item = new history_item(ids[i], info.title, info.url, NULL, 0, info.date_created);
			history_list.push_back(item);
			bp_history_adaptor_easy_free(&info);
		}
		free(ids);
	}

	delete [] text;

	return history_list;
}

std::vector<history_item *> history::get_history_list(int max_count, Eina_Bool only_today)
{
	std::vector<history_item *> history_list;

	int *ids = NULL;
	int ids_count = -1;
	Evas_Object *image = NULL;

	if (__get_date_ids_p(&ids, &ids_count, max_count, -1, BP_HISTORY_O_DATE_VISITED, 1, BP_HISTORY_O_DATE_VISITED, (only_today) ? BP_HISTORY_DATE_TODAY : BP_HISTORY_DATE_ALL) < 0) {
		BROWSER_LOGE("Failed to get history list ids info");
		return history_list;
	}

	bp_history_info_fmt info;
	bp_history_offset offset = (BP_HISTORY_O_URL | BP_HISTORY_O_TITLE |
			BP_HISTORY_O_DATE_CREATED | BP_HISTORY_O_DATE_VISITED | BP_HISTORY_O_FAVICON);
	for (int i = 0;i < ids_count; i++) {
		if (bp_history_adaptor_get_info(ids[i], offset, &info ) < 0) {
			BROWSER_LOGE("Failed to get Uri info for id = %d", i);
			continue;
		}

		int date;

		if (info.date_visited)
			date = info.date_visited;
		else
			date = info.date_created;

		if (info.favicon_length && info.favicon) {
			void *snapshot_data = info.favicon;
			if (snapshot_data != NULL) {
				image = evas_object_image_filled_add(evas_object_evas_get(m_window));
				evas_object_image_colorspace_set(image, EVAS_COLORSPACE_ARGB8888);
				evas_object_image_size_set(image, info.favicon_width, info.favicon_height);
				evas_object_image_fill_set(image, 0, 0,  info.favicon_width, info.favicon_height);
				evas_object_image_filled_set(image, EINA_TRUE);
				evas_object_image_alpha_set(image, EINA_TRUE);
				void *pixels = evas_object_image_data_get(image, EINA_TRUE);
				if (pixels) {
					memcpy(pixels, (void *)snapshot_data, info.favicon_length);
					evas_object_image_data_set(image, pixels);
				}
			}
		}

		history_item *item = new history_item(ids[i], info.title, info.url, NULL, 0, date, image);
		history_list.push_back(item);
		bp_history_adaptor_easy_free(&info);
		SAFE_FREE_OBJ(image);
	}

	free(ids);
	ids = NULL;

	return history_list;
}

Evas_Object *history::get_snapshot(const char *uri)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RETV_MSG_IF(!uri, NULL, "uri is NULL");
	Evas_Object *image = NULL;

	int id = -1;
	int *ids = NULL;
	int ids_count = -1;
	if (__get_duplicated_ids_p(&ids, &ids_count, 1, 0,  BP_HISTORY_O_DATE_VISITED , 0, BP_HISTORY_O_URL, uri, 0) < 0) {
		BROWSER_LOGE("Failed to retrieve id for Url = %s", uri);
		return NULL;
	}
	if (ids_count > 0 && ids != NULL) {
		id = ids[0];

		void *snapshot_data = NULL;
		int length = 0;
		int width = 0;
		int height = 0;
		int ret = bp_history_adaptor_get_snapshot(id, &width, &height,
				(unsigned char **)&snapshot_data, &length);
		if (ret < 0) {
			BROWSER_LOGE("bp_history_adaptor_get_snapshot is failed");
		} else {

			if (length > 0 && snapshot_data != NULL) {
				image = evas_object_image_filled_add(evas_object_evas_get(m_window));
				evas_object_image_colorspace_set(image, EVAS_COLORSPACE_ARGB8888);
				evas_object_image_size_set(image, width, height);
				evas_object_image_fill_set(image, 0, 0, width, height);
				evas_object_image_filled_set(image, EINA_TRUE);
				evas_object_image_alpha_set(image,EINA_TRUE);
				BROWSER_LOGD("image=[%d]", image);

				void *pixels = evas_object_image_data_get(image, EINA_TRUE);
				if (pixels) {
					memcpy(pixels, (void *)snapshot_data, length);
					evas_object_image_data_set(image, pixels);
				}
			}
		}
	}

	free(ids);

	if (!image) {
		BROWSER_LOGE("snapshot doesn't exist");
		return NULL;
	}
	return image;
}

void history::set_snapshot(int id, Evas_Object *snapshot)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!snapshot, "snapshot is NULL");

	unsigned char *snapshot_data = (unsigned char *)evas_object_image_data_get(snapshot, EINA_TRUE);
	RET_MSG_IF(!snapshot_data, "snapshot data is NULL");

	int w = 0;
	int h = 0;
	int stride = 0;
	int len = 0;

	platform_service ps;
	ps.evas_image_size_get(snapshot, &w, &h, &stride);
	len = stride * h;

	int ret = bp_history_adaptor_set_snapshot(id, w, h, snapshot_data, len);
	if (ret < 0)
		BROWSER_LOGE("bp_history_adaptor_set_snapshot failed. %d - %d, %d, %d", id, w, h, len);
}

Eina_Bool history::is_snapshot_exist(int id)
{
	void *snapshot_data = NULL;
	int length = 0;
	int width = 0;
	int height = 0;
	int ret = bp_history_adaptor_get_snapshot(id, &width, &height, (unsigned char **)&snapshot_data, &length);
	if (ret < 0)
		return EINA_FALSE;

	if (length > 0)
		return EINA_TRUE;

	return EINA_FALSE;
}

std::vector<history_item *> history::get_histories_order_by_visit_count(unsigned int count)
{
	std::vector<history_item *> history_list;

	if (count == 0)
		return history_list;

	int *ids = NULL;
	int ids_count = -1;

	if (__get_date_ids_p(&ids, &ids_count, count, 0, BP_HISTORY_O_FREQUENCY, 1,
		BP_HISTORY_O_DATE_VISITED, BP_HISTORY_DATE_ALL) < 0) {
		BROWSER_LOGE("Failed to get Todays history list" );
		return history_list;
	}

	bp_history_info_fmt info;
	bp_history_offset offset = (BP_HISTORY_O_URL | BP_HISTORY_O_TITLE |
			BP_HISTORY_O_FREQUENCY);

	for (int i = 0; i < ids_count; i++) {
		Evas_Object *image = NULL;
		if (bp_history_adaptor_get_info(ids[i], offset, &info ) < 0) {
			BROWSER_LOGE("Failed to get Uri info for id = %d", i);
			continue;
		}

		if (info.frequency < 1) {
			bp_history_adaptor_easy_free(&info);
			break;
		}

		unsigned char *snapshot = NULL;
		int length = 0;
		int width = 0;
		int height = 0;
		if (bp_history_adaptor_get_snapshot(ids[i], &width, &height,
				(unsigned char **)&snapshot, &length) == 0) {
			if (snapshot != NULL && length > 0) {
				image = evas_object_image_filled_add(evas_object_evas_get(m_window));
				evas_object_image_colorspace_set(image, EVAS_COLORSPACE_ARGB8888);
				evas_object_image_size_set(image, width, height);
				evas_object_image_fill_set(image, 0, 0, width, height);
				evas_object_image_filled_set(image, EINA_TRUE);
				evas_object_image_alpha_set(image,EINA_TRUE);
				BROWSER_LOGD("image=[%d]", image);

				void *pixels = evas_object_image_data_get(image, EINA_TRUE);
				if (pixels) {
					memcpy(pixels, (void *)snapshot, length);
					evas_object_image_data_set(image, pixels);
				}
			}
		}

		history_item *item = new history_item(ids[i], info.title, info.url, image, info.frequency);
		SAFE_FREE_OBJ(image);
		history_list.push_back(item);
		bp_history_adaptor_easy_free(&info);
		if (history_list.size() >= count)
			break;
	}

	free(ids);

	return history_list;
}

void history::reset_history_visit_count(history_item *item)
{
	RET_MSG_IF(!item, "item is NULL");

	int *ids = NULL;
	int ids_count = -1;
	BROWSER_SECURE_LOGD("URI SELECTED TO DELETE = %s",item->get_uri());
	if (__get_duplicated_ids_p(&ids, &ids_count, 1, 0,  BP_HISTORY_O_DATE_VISITED , 0, BP_HISTORY_O_URL, item->get_uri(), 0) < 0) {
		BROWSER_LOGE("Failed to get id for Uri");
		return;
	}

	if (bp_history_adaptor_set_frequency(ids[0], 0) < 0)
		BROWSER_LOGE("Failed to Set frequency");

	free(ids);
}

history_item *history::get_next_history_order_by_visit_count(history_item *item)
{
	RETV_MSG_IF(!item, NULL, "item is NULL");

	int *ids = NULL;
	int ids_count = -1;

	if (__get_date_ids_p(&ids, &ids_count, most_visited_item_count + 1, 0, BP_HISTORY_O_FREQUENCY, 1,
		BP_HISTORY_O_DATE_VISITED, BP_HISTORY_DATE_ALL) < 0) {
		BROWSER_LOGE("failured to get Todays history list" );
		return NULL;
	}

	BROWSER_SECURE_LOGD("IDS_COUNT = %d" , ids_count);

	if (ids_count < most_visited_item_count) {
		free(ids);
		return NULL;
	} else {
		bp_history_info_fmt info;
		if (bp_history_adaptor_get_info(ids[most_visited_item_count - 1],
				(BP_HISTORY_O_URL | BP_HISTORY_O_TITLE |BP_HISTORY_O_FREQUENCY),
				&info ) < 0) {
			BROWSER_LOGE("Failed to get Uri info for id = %d", most_visited_item_count);
			free(ids);
			return NULL;
		}

		if (info.frequency == 0) {
			bp_history_adaptor_easy_free(&info);
			free(ids);
			return NULL;
		}

		Evas_Object *image = NULL;
		unsigned char *snapshot = NULL;
		int length = 0;
		int width = 0;
		int height = 0;
		if (bp_history_adaptor_get_snapshot(ids[most_visited_item_count],
				&width, &height,
				(unsigned char **)&snapshot, &length) == 0) {
			if (snapshot != NULL && length > 0) {
				image = evas_object_image_filled_add(evas_object_evas_get(m_window));
				evas_object_image_colorspace_set(image, EVAS_COLORSPACE_ARGB8888);
				evas_object_image_size_set(image, width, height);
				evas_object_image_fill_set(image, 0, 0, width, height);
				evas_object_image_filled_set(image, EINA_TRUE);
				evas_object_image_alpha_set(image,EINA_TRUE);
				BROWSER_LOGD("image=[%d]", image);

				void *pixels = evas_object_image_data_get(image, EINA_TRUE);
				if (pixels) {
					memcpy(pixels, (void *)snapshot, length);
					evas_object_image_data_set(image, pixels);
				}
			}
		}

		if (!image) {
			Evas_Object *current_ewk_view = NULL;
			if (m_browser->get_browser_view()->get_current_webview())
				current_ewk_view = m_browser->get_browser_view()->get_current_webview()->get_ewk_view();

			image = evas_object_rectangle_add(evas_object_evas_get(m_window));
			int w = 0;

			if (current_ewk_view)
				evas_object_geometry_get(current_ewk_view, NULL, NULL, &w, NULL);
			else
				evas_object_geometry_get(m_window, NULL, NULL, &w, NULL);

			evas_object_resize(image, w, w * 0.3);
			evas_object_color_set(image, 255, 255, 255, 255);
		}

		history_item *item = new history_item(ids[most_visited_item_count - 1], info.title, info.url, image, info.frequency);

		SAFE_FREE_OBJ(image);
		free(ids);
		bp_history_adaptor_easy_free(&info);

		return item;
	}

}

Eina_Bool history::__clear_history_data(void *data)
{
	BROWSER_LOGD("");
	m_browser->notify_history_cleared(EINA_FALSE);
	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool history::delete_all(void)
{
	if (bp_history_adaptor_reset() < 0) {
		BROWSER_LOGE("Failed to clear History");
		return EINA_FALSE;
	}
	ecore_idler_add(__clear_history_data , this);

	return EINA_TRUE;
}
Eina_Bool history::delete_all(void* data, THREAD_CANCEL_CB cancel_cb, THREAD_END_CB end_cb)
{
	m_thread_cancel_cb = cancel_cb;
	m_thread_end_cb = end_cb;
	m_thread_data  = data;
	m_current_thread = ecore_thread_run(_thread_start_clear_history, __thread_end_cb, __thread_cancel_cb, data);

	if (!m_current_thread)
		return EINA_FALSE;
	return EINA_TRUE;
}

void history::_thread_start_clear_history(void* data, Ecore_Thread *th)
{
	BROWSER_LOGD("");
	int *ids = NULL;
	int ids_count = -1;

	if (__get_date_ids_p(&ids, &ids_count,-1, -1, BP_HISTORY_O_DATE_VISITED, 1, BP_HISTORY_O_DATE_VISITED, BP_HISTORY_DATE_ALL) < 0) {
		BROWSER_LOGE("Failed to get history list ids info");
		return;
	}

	for (int i = 0;i < ids_count; i++) {
		if (ecore_thread_check(m_current_thread)) {
			BROWSER_LOGE("Cancelling the thread");
			m_current_thread = NULL;
			free(ids);
			return;
		}
		if (bp_history_adaptor_delete(ids[i]) < 0) {
			BROWSER_LOGE("Failed to delete ids[%d]= %d ", i,ids[i]);
			continue;
		}
		BROWSER_LOGE("Deleted item  = %d",i);
	}
	BROWSER_LOGE("cleared History");
	free(ids);
}

void history::__thread_cancel_cb(void* data, Ecore_Thread *th)
{
	BROWSER_LOGD("");
	m_current_thread = NULL;
	if (m_thread_cancel_cb)
		m_thread_cancel_cb(data);
	m_browser->notify_history_cleared(EINA_TRUE);
}

void history::__thread_end_cb(void* data, Ecore_Thread *th)
{
	m_current_thread = NULL;
	if (m_thread_cancel_cb)
		m_thread_end_cb(data);
	m_browser->notify_history_cleared(EINA_FALSE);
}

Eina_Bool history::delete_history(const char *uri)
{
	BROWSER_SECURE_LOGD("uri=[%s]", uri);
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is EINA_FALSE");

	int *ids = NULL;
	int ids_count = -1;
	if ( __get_duplicated_ids_p(&ids, &ids_count, 1, 0,  BP_HISTORY_O_DATE_CREATED, 0, BP_HISTORY_O_URL, uri, 0) < 0) {
		BROWSER_LOGE("Failed to get Url Information");
		return EINA_FALSE;
	}

	Eina_Bool ret = EINA_TRUE;
	if (ids_count > 0 && ids != NULL) {
		int id = ids[0];
		if (bp_history_adaptor_delete(id) < 0) {
			BROWSER_LOGE("Failed to delete Uri = %s ", uri);
			ret = EINA_FALSE;
		}
	}

	free(ids);
	return ret;
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

	BROWSER_SECURE_LOGD("favicon w=[%d], h=[%d], stride=[%d]", w, h, stride);

	len = stride * h;

	if (len == 0)
		return EINA_FALSE;

	int *ids = NULL;
	int ids_count = -1;
	if (__get_duplicated_ids_p(&ids, &ids_count, 1, 0,  BP_HISTORY_O_DATE_VISITED , 0, BP_HISTORY_O_URL, uri, 0) < 0) {
		BROWSER_LOGE("Failed to check duplicate ids for Uri = %s", uri);
		return EINA_FALSE;
	}

	Eina_Bool ret = EINA_TRUE;
	if (ids_count > 0 && ids != NULL) {
		int id = ids[0];
		void *icon_data = evas_object_image_data_get(icon, EINA_TRUE);
		if (bp_history_adaptor_set_icon( id, w, h, (const unsigned char *)icon_data, len) < 0) {
			BROWSER_LOGE("Failed to set Favicon info for URL= %s" , uri);
			ret = EINA_FALSE;
		}
	}
	free(ids);
	return ret;
}

Evas_Object *history::get_history_favicon(const char *uri)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RETV_MSG_IF(!uri, NULL, "uri is NULL");
	Evas_Object *icon = NULL;
	Evas *e = NULL;
	e = evas_object_evas_get(m_window);

	if (!e) {
		BROWSER_LOGE("canvas is NULL");
		return NULL;
	}

	int id = -1;
	int *ids = NULL;
	int ids_count = -1;
	if (__get_duplicated_ids_p(&ids, &ids_count, 1, 0,  BP_HISTORY_O_DATE_VISITED , 0, BP_HISTORY_O_URL, uri, 0) < 0) {
		BROWSER_LOGE("Failed to check duplicate ids for Url = %s" , uri);
		return icon;
	}

	if (ids_count > 0 && ids != NULL) {
		id = ids[0];

		void *favicon_data = NULL;
		int icon_w = 0;
		int icon_h = 0;
		int icon_length = 0;
		int ret = bp_history_adaptor_get_icon(id, &icon_w, &icon_h, (unsigned char **)&favicon_data, &icon_length);
		if (ret < 0) {
			BROWSER_LOGE("bp_history_adaptor_get_icon is failed");
		} else {

			BROWSER_LOGE("icon_length(%d), icon_w(%d), icon_h(%d)", icon_length, icon_w, icon_h);

			if (favicon_data != NULL && icon_length > 0){
				icon = evas_object_image_filled_add(e);
				evas_object_image_colorspace_set(icon, EVAS_COLORSPACE_ARGB8888);
				evas_object_image_size_set(icon, icon_w, icon_h);
				evas_object_image_fill_set(icon, 0, 0, icon_w, icon_h);
				evas_object_image_filled_set(icon, EINA_TRUE);
				evas_object_image_alpha_set(icon,EINA_TRUE);

				void *pixels = evas_object_image_data_get(icon, EINA_TRUE);
				if (pixels) {
					memcpy(pixels, (void *)favicon_data, icon_length);
					evas_object_image_data_set(icon, pixels);
				}
			}
		}
	}
	free(ids);
	return icon;
}

Eina_Bool history::set_history_webicon(const char *uri, Evas_Object *icon)
{
	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");
	RETV_MSG_IF(!icon, EINA_FALSE, "icon is NULL");

	int w = 0;
	int h = 0;
	int stride = 0;
	int len = 0;

	platform_service ps;
	ps.evas_image_size_get(icon, &w, &h, &stride);

	BROWSER_SECURE_LOGD("webicon w=[%d], h=[%d], stride=[%d]", w, h, stride);

	len = stride * h;

	if (len == 0)
		return EINA_FALSE;

	void *icon_data = evas_object_image_data_get(icon, EINA_TRUE);
	int id = -1;
	int *ids = NULL;
	int ids_count = -1;
	if (__get_duplicated_ids_p(&ids, &ids_count, 1, 0,  BP_HISTORY_O_DATE_VISITED , 0, BP_HISTORY_O_URL, uri, 0) < 0) {
		BROWSER_LOGE("Failed to check duplicate ids for Uri = %s", uri);
		return EINA_FALSE;
	}

	if (ids_count > 0) {
		id = ids[0];
		free(ids);
		ids = NULL;

		if (bp_history_adaptor_set_webicon( id, w, h, (const unsigned char *)icon_data, len) < 0) {
			BROWSER_LOGE("Failed to set Favicon (info/width/height) for URL= %s" , uri);
			return EINA_FALSE;
		}
	}
	if (ids)
		free(ids);

	return EINA_TRUE;
}

Evas_Object *history::get_history_webicon(const char *uri)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);
	RETV_MSG_IF(!uri, NULL, "uri is NULL");
	Evas_Object *icon = NULL;
	Evas *e = NULL;
	e = evas_object_evas_get(m_window);

	if (!e) {
		BROWSER_LOGE("canvas is NULL");
		return NULL;
	}

	int id = -1;
	int *ids = NULL;
	int ids_count = -1;
	if (__get_duplicated_ids_p(&ids, &ids_count, 1, 0,  BP_HISTORY_O_DATE_VISITED , 0, BP_HISTORY_O_URL, uri, 0) < 0) {
		BROWSER_LOGE("Failed to check duplicate ids for Url = %s" , uri);
		return icon;
	}
	if (ids_count) {
		id = ids[0];
		bp_history_info_fmt info;

		if (bp_history_adaptor_get_info(id, BP_HISTORY_O_WEBICON, &info ) < 0) {
			BROWSER_LOGE("Failed to get Info for Uri = %s" , uri);
			free(ids);
			bp_history_adaptor_easy_free(&info);
			return icon;
		}

		int icon_w = 0;
		int icon_h = 0;
		int icon_length = 0;

		icon_length = info.webicon_length;
		icon_w = info.webicon_width;
		icon_h = info.webicon_height;

		BROWSER_LOGE("icon_length(%d), icon_w(%d), icon_h(%d)", icon_length, icon_w, icon_h);

		if (icon_length > 0){
			icon = evas_object_image_filled_add(e);
			evas_object_image_colorspace_set(icon, EVAS_COLORSPACE_ARGB8888);
			evas_object_image_size_set(icon, icon_w, icon_h);
			evas_object_image_fill_set(icon, 0, 0, icon_w, icon_h);
			evas_object_image_filled_set(icon, EINA_TRUE);
			evas_object_image_alpha_set(icon,EINA_TRUE);

			void *pixels = evas_object_image_data_get(icon, EINA_TRUE);
			if (pixels) {
				memcpy(pixels, (void *)info.webicon, icon_length);
				evas_object_image_data_set(icon, pixels);
			}
		}

		bp_history_adaptor_easy_free(&info);
	}
	free(ids);

	return icon;
}

history_item *history::get_top_nth_history_item(int index)
{
	BROWSER_SECURE_LOGD("index = [%d]", index);
	RETV_MSG_IF(index <= 0, NULL, "index is invalid");

	int *ids = NULL;
	int ids_count = -1;

	if (__get_date_ids_p(&ids, &ids_count, index, -1, BP_HISTORY_O_DATE_VISITED, 1, BP_HISTORY_O_DATE_VISITED, BP_HISTORY_DATE_ALL) < 0) {
		BROWSER_LOGE("Failed to get history list ids info");
		return NULL;
	}

	bp_history_info_fmt info;
	bp_history_offset offset = (BP_HISTORY_O_URL | BP_HISTORY_O_TITLE |
			BP_HISTORY_O_DATE_CREATED | BP_HISTORY_O_DATE_VISITED);

	BROWSER_LOGD("size of history list received is %d", ids_count);

	if (ids_count < index) {
		free(ids);
		ids = NULL;
		return NULL;
	}

	if (bp_history_adaptor_get_info(ids[index - 1], offset, &info ) < 0) {
		BROWSER_LOGE("Failed to get Uri info for id = %d", index);
		free(ids);
		ids = NULL;
		return NULL;
	}

	int date;

	if (info.date_visited)
		date = info.date_visited;
	else
		date = info.date_created;

	history_item *item = new history_item(ids[index - 1], info.title, info.url, NULL, 0, date);

	bp_history_adaptor_easy_free(&info);

	free(ids);
	ids = NULL;

	return item;
}
