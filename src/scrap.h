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

#ifndef SCRAP_H
#define SCRAP_H

#include <Evas.h>
#include <Elementary.h>
#include <vector>

#include "browser-object.h"

class webview;

typedef enum _time_stamp_type {
	SCRAP_TODAY,
	SCRAP_YESTERDAY,
	SCRAP_LAST_7_DAYS,
	SCRAP_LAST_MONTH,
	SCRAP_OLDER
} time_stamp_type;

class scrap_item : public browser_object {
public:
	scrap_item(const char *title, const char *uri, const char *file_path, const char *date, const char *tag);
	~scrap_item(void);

	const char *get_title(void) { return m_title; }
	const char *get_uri(void) { return m_uri; }
	const char *get_saved_file_path(void) { return m_file_path; }
	const char *get_date(void) { return m_date; }
	const char *get_tag(void) { return m_tag; }
	void set_tag(const char *tag);
	int get_year(void) { return m_year; }
	int get_month(void) { return m_month; }
	int get_day(void) { return m_day; }
	Eina_Bool compare_date(const char *date);
	time_stamp_type get_time_stamp_type(void);
private:
	const char *m_title;
	const char *m_uri;
	const char *m_file_path;
	const char *m_date;
	const char *m_tag;
	int m_year;
	int m_month;
	int m_day;
};

class scrap : public browser_object {
public:
	scrap(void);
	~scrap(void);

	char *save(const char *title, const char *uri, const char *mht_content, const char *tag = NULL, Eina_Bool keep_it = EINA_FALSE);
	std::vector<scrap_item *> get_scrap_list(void);
	std::vector<char *> get_tag_list(scrap_item *item = NULL);
	void delete_tag(const char *tag);
	void delete_scrap(const char *file_path);
	std::vector<char *> extract_tag_list(const char *tag);
	void update_tag(scrap_item *item, const char *tag);
private:
	void _save_scrap(const char *title, const char *uri, const char *file_path, const char *tag = NULL);
};

#endif /* SCRAP_H */

