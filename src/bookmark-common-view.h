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

#ifndef BOOKMARK_COMMON_VIEW_H
#define BOOKMARK_COMMON_VIEW_H

#include <Elementary.h>

#include "common-view.h"
#include "bookmark-item.h"

class bookmark;

class bookmark_common_view : public common_view {
public:

	bookmark_common_view(void);
	~bookmark_common_view(void);
	virtual void on_pause(void);

protected:
	static bookmark *m_bookmark;

private:
	typedef struct _gl_common_cb_data {
		void *user_data;
		void *cp;
		Elm_Object_Item *it;
	} gl_common_cb_data;

	static char *__genlist_get_text_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_del_cb(void *data, Evas *e, Evas_Object *genlist, void *ei);

	Evas_Object *m_popup_select_folder;
	Elm_Object_Item *m_last_folder_item;
};

#endif /* BOOKMARK_COMMON_VIEW_H */

