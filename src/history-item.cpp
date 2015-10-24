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

#include "history-item.h"

#include "browser-dlog.h"
#include "browser-string.h"
#include "platform-service.h"

history_item::history_item(int id, const char *title, const char *uri, Evas_Object *snapshot, int visit_count, int date, Evas_Object *favicon)
	: m_year(0)
	, m_month(0)
	, m_day(0)
	, m_date(0)
	, m_visit_count(visit_count)
	, m_title(NULL)
	, m_uri(NULL)
	, m_snapshot(NULL)
	, m_favicon(NULL)
	, m_genlist_it(NULL)
	, m_month_day(0)
	, m_is_checked(EINA_FALSE)
{
	BROWSER_LOGD("");
	m_id = id;
	if (title)
		m_title = eina_stringshare_add(title);
	if (uri)
		m_uri = eina_stringshare_add(uri);
	if (date) {
		struct tm *item_time_info;
		time_t item_time = (time_t)date;
		item_time_info = localtime(&item_time);
		if(item_time_info){
			m_year = item_time_info->tm_year;
			m_month = item_time_info->tm_mon + 1;
			m_day = item_time_info->tm_yday;
			m_month_day = item_time_info->tm_mday;
		}
		m_date = date;
		BROWSER_SECURE_LOGD("m_year = [%d] ......m_month = %d ......m_day = %d.....m_month_day = %d", m_year, m_month, m_day, m_month_day);
	}
	if (snapshot) {
		platform_service ps;
		m_snapshot = ps.copy_evas_image(snapshot);
	}

	if (favicon) {
		platform_service ps;
		m_favicon = ps.copy_evas_image(favicon);
	}
}

history_item::~history_item(void)
{
	BROWSER_LOGD("");

	SAFE_FREE_OBJ(m_snapshot);
	SAFE_FREE_OBJ(m_favicon);

	eina_stringshare_del(m_title);
	eina_stringshare_del(m_uri);
}

void history_item::set_title(const char *title)
{
	BROWSER_SECURE_LOGD("title = [%s]", title);

	eina_stringshare_replace(&m_title, title);
}

void history_item::set_uri(const char *uri)
{
	BROWSER_SECURE_LOGD("uri = [%s]", uri);

	eina_stringshare_replace(&m_uri, uri);
}

Evas_Object *history_item::copy_snapshot(void)
{
	RETV_MSG_IF(!m_snapshot, NULL, "m_snapshot is NULL");

	platform_service ps;
	return ps.copy_evas_image(m_snapshot);
}

Evas_Object *history_item::copy_favicon(void)
{
	RETV_MSG_IF(!m_favicon, NULL, "m_favicon is NULL");

	platform_service ps;
	return ps.copy_evas_image(m_favicon);
}

Time_stamp_type history_item::get_time_stamp_type(void)
{
	BROWSER_LOGD("");
	time_t current_time;
	struct tm *current_time_info;
	time(&current_time);
	current_time_info = localtime(&current_time);

	if(!current_time_info){
		return HISTORY_OLDER;
	}

	int current_year = current_time_info->tm_year;
	int current_yday = current_time_info->tm_yday;

	int year_gap = current_year - this->m_year;
	int day_gap = current_yday - this->m_day;
	if (year_gap == 0) {
		if (day_gap < 0)
			return HISTORY_NEXT_DAYS;
		else if (day_gap == 0)
			return HISTORY_TODAY;
		else if (day_gap == 1)
			return HISTORY_YESTERDAY;
		else if (day_gap <= 7)
			return HISTORY_LAST_7_DAYS;
		else if (day_gap <= 30)
			return HISTORY_LAST_MONTH;
		else
			return HISTORY_OLDER;
	} else if (year_gap == 1) {
		if (day_gap <= HISTORY_PREVIOUS_DAY_CHECK)
			return HISTORY_YESTERDAY;
		else if (day_gap <= HISTORY_PREVIOUS_WEEK_CHECK)
			return HISTORY_LAST_7_DAYS;
		else if (day_gap <= HISTORY_PREVIOUS_MONTH_CHECK)
			return HISTORY_LAST_MONTH;
		else
			return HISTORY_OLDER;
	} else if (year_gap < 0)
		return HISTORY_NEXT_DAYS;
	return HISTORY_OLDER;
}
