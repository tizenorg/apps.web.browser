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

#ifndef HISTORY_H
#define HISTORY_H

#include <Evas.h>
#include <vector>

#include "bookmark.h"
#include "browser-object.h"

typedef void (* THREAD_CANCEL_CB)(void* data);
typedef void (* THREAD_END_CB)(void* data);
class history_item;
class history : public browser_object {
public:
	history(void);
	~history(void);

	Eina_Bool save_history(const char *title, const char *uri, Evas_Object *snapshot = NULL, Evas_Object *favicon = NULL, int *visit = NULL);
	Eina_Bool delete_history(const char *uri);
	Eina_Bool delete_all(void);
	static Ecore_Thread *m_get_thread(void) { return m_current_thread; }
	Eina_Bool delete_all(void* data, THREAD_CANCEL_CB cancel_cb, THREAD_END_CB end_cb);
	static void _thread_start_clear_history(void* data, Ecore_Thread *th);
	static void __thread_cancel_cb(void* data, Ecore_Thread *th);
	static void __thread_end_cb(void* data, Ecore_Thread *th);
	static Eina_Bool __clear_history_data(void *data);
	Evas_Object *get_snapshot(const char *uri);
	void set_snapshot(int id, Evas_Object *snapshot);
	Eina_Bool is_snapshot_exist(int id);
	Eina_Bool set_history_favicon(const char *uri, Evas_Object *icon);
	Evas_Object *get_history_favicon(const char *uri);
	Eina_Bool set_history_webicon(const char *uri, Evas_Object *icon);
	Evas_Object *get_history_webicon(const char *uri);

	// The return value is malloced list, it should be freed by caller.
	std::vector<history_item *> get_history_list(int max_count = -1, Eina_Bool only_today = EINA_FALSE);
	// The return value is malloced list, it should be freed by caller.
	std::vector<history_item *> get_histories_order_by_visit_count(unsigned int count);
	// The return value is malloced item, it should be freed by caller.
	history_item *get_next_history_order_by_visit_count(history_item *item);
	void reset_history_visit_count(history_item *item);
	std::vector<history_item *> get_history_list_like(const char *part_text);
	Eina_Bool get_memory_full(void) { return m_memory_full; }
	history_item *get_top_nth_history_item(int index);
private:
	Eina_Bool m_memory_full;
	Eina_Bool m_history_adaptor_initialize;
	static Ecore_Thread *m_current_thread;
	static THREAD_CANCEL_CB m_thread_cancel_cb;
	static THREAD_END_CB m_thread_end_cb;
	static void*  m_thread_data;
};

#endif /* HISTORY_H */

