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

#include "most-visited-item.h"

#include "browser-dlog.h"
#include "most-visited.h"
#include "platform-service.h"

most_visited_item::most_visited_item(Evas_Object *snapshot, const char *title, const char *uri, int visit_count)
:
	m_snapshot(snapshot)
	,m_visit_count(visit_count)
{
	BROWSER_LOGD("");
	m_title = eina_stringshare_add(title);
	m_uri = eina_stringshare_add(uri);
}


most_visited_item::~most_visited_item(void)
{
	BROWSER_LOGD("");
	if (m_snapshot)
		evas_object_del(m_snapshot);

	eina_stringshare_del(m_title);
	eina_stringshare_del(m_uri);
}

Evas_Object *most_visited_item::copy_layout(void)
{
	Evas_Object *layout = elm_layout_add(m_window);
	if (!layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return NULL;
	}
	elm_layout_file_set(layout, most_visited_edj_path, "most-visited-item");

	platform_service ps;
	Evas_Object *snapshot = ps.copy_evas_image(m_snapshot);
	elm_object_part_content_set(layout, "elm.swallow.snapshot", snapshot);

	return layout;
}
