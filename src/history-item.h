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

#ifndef HISTORY_ITEM_H
#define HISTORY_ITEM_H

#include <Evas.h>
#include "browser-object.h"

typedef enum  {
	HISTORY_PREVIOUS_DAY_CHECK = -364,
	HISTORY_PREVIOUS_WEEK_CHECK = -358,
	HISTORY_PREVIOUS_MONTH_CHECK = -335,
} HISTORY_previous_year_days;

typedef enum {
	HISTORY_TODAY = 0,
	HISTORY_YESTERDAY = 1,
	HISTORY_LAST_7_DAYS = 7,
	HISTORY_LAST_MONTH = 30,
	HISTORY_OLDER = 31,
	HISTORY_NEXT_DAYS = -1,  //timestamp to show whether the page is ahead of time
} Time_stamp_type;

class history_item : public browser_object {
public:
	history_item(int id, const char *title, const char *uri, Evas_Object *snapshot, int visit_count, int date = 0, Evas_Object *favicon = NULL);
	~history_item(void);

	/* If the title & uri is same, they are equal. */
	friend bool operator==(history_item item1, history_item item2) {
		return (((!item1.get_title() && !item2.get_title()) || !strcmp(item1.get_title(), item2.get_title()))
			&& ((!item1.get_uri() && !item2.get_uri()) || !strcmp(item1.get_uri(), item2.get_uri())));
	}

	Time_stamp_type get_time_stamp_type(void);
	int get_id(void) { return m_id; }
	const char *get_title(void) { return m_title; }
	const char *get_uri(void) { return m_uri; }
	Evas_Object *get_snapshot(void) { return m_snapshot; }
	int get_date(void) { return m_date; }
	int get_year(void) { return m_year; }
	int get_month(void) { return m_month; }
	int get_day(void) { return m_day; }
	Evas_Object * get_favicon(void) { return m_favicon; }

	void set_title(const char *title);
	void set_uri(const char *uri);

	int get_visit_count(void) { return m_visit_count; }
	Evas_Object *copy_snapshot(void);
	Evas_Object *copy_favicon(void);
	void set_it_data(Elm_Object_Item *it){ m_genlist_it = it;}
	Elm_Object_Item *get_it_data(void){ return m_genlist_it;}
	int get_month_day(void) { return m_month_day; }
	Eina_Bool get_is_checked_data (void) { return m_is_checked; }
	void set_checked_data (Eina_Bool flag) { m_is_checked = flag; }
private:
	int m_id;
	int m_year;
	int m_month;
	int m_day;
	int m_date;
	int m_visit_count;
	const char *m_title;
	const char *m_uri;

	Evas_Object *m_snapshot;
	Evas_Object *m_favicon;
	Elm_Object_Item *m_genlist_it;
	int m_month_day;
	Eina_Bool m_is_checked;
};

#endif /* HISTORY_ITEM_H */

