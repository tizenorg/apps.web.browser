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
 * Contact: Sangpyo Kim <sangpyo7.kim@samsung.com>
 *
 */

#include "bookmark-item.h"
#include "browser-dlog.h"

bookmark_item::bookmark_item(const char *title, const char *uri, int id, int parent_id, int order, Eina_Bool is_folder)
:
	m_title(NULL)
	,m_uri(NULL)
	,m_is_folder(is_folder)
	,m_is_editable(EINA_TRUE)
	,m_id(id)
	,m_parent_id(parent_id)
	,m_order(order)
	,m_initial_order(order)
	,m_is_modified(EINA_FALSE)
{
	if (title)
		m_title = eina_stringshare_add(title);
	if (uri)
		m_uri = eina_stringshare_add(uri);
}

bookmark_item::~bookmark_item(void)
{
	eina_stringshare_del(m_title);
	eina_stringshare_del(m_uri);
}

bookmark_item& bookmark_item::operator=(const bookmark_item &item_src)
{
	set_title(item_src.get_title());
	set_uri(item_src.get_uri());
	set_folder_flag(item_src.is_folder());
	set_editable_flag(item_src.is_editable());
	set_id(item_src.get_id());
	set_parent_id(item_src.get_parent_id());
	set_order(item_src.get_order());
	set_initial_order(item_src.get_order());

	return *this;
}

void bookmark_item::set_title(const char *title)
{
	eina_stringshare_replace(&m_title, title);
}

void bookmark_item::set_uri(const char *uri)
{
	eina_stringshare_replace(&m_uri, uri);
}
