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

#ifndef MOST_VISITED_ITEM_H
#define MOST_VISITED_ITEM_H

#include <Evas.h>
#include "browser-object.h"

class most_visited_item : public browser_object {
public:
	most_visited_item(Evas_Object *snapshot, const char *title, const char *uri, int visit_count = 0);
	~most_visited_item(void);

	const char *get_title(void) { return m_title; }
	const char *get_uri(void) { return m_uri; }
	int get_visit_count(void) { return m_visit_count; }

	Evas_Object *copy_layout(void);
private:
	const char *m_title;
	const char *m_uri;
	Evas_Object *m_snapshot;
	int m_visit_count;
};

#endif /* MOST_VISITED_ITEM_H */

