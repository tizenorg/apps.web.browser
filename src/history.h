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

#ifndef HISTORY_H
#define HISTORY_H

#include <Evas.h>
#include <vector>

#include "bookmark.h"
#include "browser-object.h"

class history_item;
class history : public browser_object {
public:
	history(void);
	~history(void);

	Eina_Bool save_history(const char *title, const char *uri, Evas_Object *snapshot = NULL, int *visit = NULL);
	Eina_Bool save_history(history_item item);
	Eina_Bool delete_history(const char *uri);
	Eina_Bool delete_history(history_item item);
	Eina_Bool delete_all(void);
	int get_count(void);
	Eina_Bool set_history_favicon(const char *uri, Evas_Object *icon);
	Evas_Object *get_history_favicon(const char *uri);

	// The return value is malloced list, it should be freed by caller.
	std::vector<history_item *> get_history_list(void);
	// The return value is malloced list, it should be freed by caller.
	std::vector<history_item *> get_histories_order_by_visit_count(int count);
	// The return value is malloced item, it should be freed by caller.
	history_item *get_next_history_order_by_visit_count(history_item *item);
	void reset_history_visit_count(history_item *item);
private:
};

#endif /* HISTORY_H */

