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

#include "history-item.h"

#include "browser-dlog.h"
#include "browser-string.h"
#include "platform-service.h"

history_item::history_item(const char *title, const char *uri, Evas_Object *snapshot, int visit_count, const char *date)
:
	m_title(NULL)
	,m_uri(NULL)
	,m_date(NULL)
	,m_snapshot(snapshot)
	,m_visit_count(visit_count)
	,m_year(0)
	,m_month(0)
	,m_day(0)
{
	BROWSER_LOGD("");
	if (title)
		m_title = eina_stringshare_add(title);
	if (uri)
		m_uri = eina_stringshare_add(uri);
	if (date) {
		m_date = eina_stringshare_add(date);
		sscanf(date, "%d-%d-%d", &m_year, &m_month, &m_day);
	}
}

history_item::~history_item(void)
{
	BROWSER_LOGD("");
	if (m_snapshot)
		evas_object_del(m_snapshot);

	eina_stringshare_del(m_title);
	eina_stringshare_del(m_uri);
	eina_stringshare_del(m_date);
}

void history_item::set_title(const char *title)
{
	BROWSER_LOGD("title = [%s]", title);

	eina_stringshare_replace(&m_title, title);
}

void history_item::set_uri(const char *uri)
{
	BROWSER_LOGD("uri = [%s]", uri);

	eina_stringshare_replace(&m_uri, uri);
}

void history_item::set_date(const char *date)
{
	BROWSER_LOGD("date = [%s]", date);

	eina_stringshare_replace(&m_date, date);
}

Evas_Object *history_item::copy_snapshot(void)
{
	EINA_SAFETY_ON_NULL_RETURN_VAL(m_snapshot, NULL);

	platform_service ps;
	return ps.copy_evas_image(m_snapshot);
}

time_stamp_history_type history_item::get_time_stamp_type(void)
{
	time_t current_time;
	struct tm *current_time_info;
	time(&current_time);
	current_time_info = localtime(&current_time);

	int current_year = current_time_info->tm_year;
	int current_yday = current_time_info->tm_yday;

	struct tm item_time_info;
	memset(&item_time_info, 0x00, sizeof(struct tm));
	item_time_info.tm_year = m_year - 1900;
	item_time_info.tm_mon = m_month - 1;
	item_time_info.tm_mday = m_day;
	time_t item_time = mktime(&item_time_info);
	struct tm *ptr_item_time = localtime(&item_time);

	int item_year = ptr_item_time->tm_year;
	int item_yday = ptr_item_time->tm_yday;

	if (current_year == item_year) {
		int day_gap = current_yday - item_yday;
		if (day_gap == 0)
			return HISTORY_TODAY;
		else if (day_gap == 1)
			return HISTORY_YESTERDAY;
		else if (day_gap <= 7)
			return HISTORY_LAST_7_DAYS;
		else if (day_gap <= 30)
			return HISTORY_LAST_MONTH;
		else
			return HISTORY_OLDER;
	}
	return HISTORY_OLDER;
}
