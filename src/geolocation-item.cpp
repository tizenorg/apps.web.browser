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

#include "geolocation-item.h"

#include "browser-dlog.h"

geolocation_item::geolocation_item(const char *uri, bool accept, const char *title, const char *date)
:
	m_accept(false)
	,m_title(NULL)
	,m_date(NULL)
{
	BROWSER_LOGD("");

	if (uri)
		m_uri = eina_stringshare_add(uri);
	if (title)
		m_title = eina_stringshare_add(title);
	if (date)
		m_date = eina_stringshare_add(date);

	m_accept = accept;
}

geolocation_item::~geolocation_item(void)
{
	BROWSER_LOGD("");

	eina_stringshare_del(m_uri);
	eina_stringshare_del(m_title);
	eina_stringshare_del(m_date);
}

void geolocation_item::set_title(const char *title)
{
	BROWSER_LOGD("title = [%s]", title);

	eina_stringshare_replace(&m_title, title);
}

void geolocation_item::set_uri(const char *uri)
{
	BROWSER_LOGD("uri = [%s]", uri);

	eina_stringshare_replace(&m_uri, uri);
}

void geolocation_item::set_date(const char *date)
{
	BROWSER_LOGD("date = [%s]", date);

	eina_stringshare_replace(&m_date, date);
}

