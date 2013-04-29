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

#ifndef HISTORY_ITEM_H
#define HISTORY_ITEM_H

#include <Evas.h>
#include "browser-object.h"

typedef enum _time_stamp_history_type {
	HISTORY_TODAY,
	HISTORY_YESTERDAY,
	HISTORY_LAST_7_DAYS,
	HISTORY_LAST_MONTH,
	HISTORY_OLDER
} time_stamp_history_type;

class history_item : public browser_object {
public:
	history_item(const char *title, const char *uri, Evas_Object *snapshot, int visit_count, const char *date = NULL);
	~history_item(void);

	/* If the title & uri is same, they are equal. */
	friend bool operator==(history_item item1, history_item item2) {
		return (((!item1.get_title() && !item2.get_title()) || !strcmp(item1.get_title(), item2.get_title()))
			&& ((!item1.get_uri() && !item2.get_uri()) || !strcmp(item1.get_uri(), item2.get_uri())));
	}

	time_stamp_history_type get_time_stamp_type(void);
	const char *get_title(void) { return m_title; }
	const char *get_uri(void) { return m_uri; }
	const char *get_date(void) { return m_date; }
	int get_year(void) { return m_year; }
	int get_month(void) { return m_month; }
	int get_day(void) { return m_day; }

	void set_title(const char *title);
	void set_uri(const char *uri);
	void set_date(const char *date);

	int get_visit_count(void) { return m_visit_count; }
	Evas_Object *copy_snapshot(void);
private:
	int m_year;
	int m_month;
	int m_day;
	int m_visit_count;
	const char *m_title;
	const char *m_uri;
	const char *m_date;
	Evas_Object *m_snapshot;
};

#endif /* HISTORY_ITEM_H */

