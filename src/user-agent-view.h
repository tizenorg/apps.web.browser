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

#ifndef USER_AGENT_VIEW_H
#define USER_AGENT_VIEW_H

#include "browser-object.h"
#include "common-view.h"

typedef enum _user_agent_type {
	TIZEN,
	CHROME,
	UNKNOWN
} user_agent_type;

class user_agent_view : public browser_object, public common_view {
public:
	user_agent_view(void);
	~user_agent_view(void);

	Eina_Bool show(void);
private:
	typedef struct _genlist_callback_data {
		user_agent_type type;
		void *user_data;
		Elm_Object_Item *it;
	} genlist_callback_data;

	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_genlist(Evas_Object *parent);

	static Evas_Object *__content_get_cb(void *data, Evas_Object *obj, const char *part);
	static char *__label_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __check_changed_cb( void *data, Evas_Object *obj, void *event_info);
	static void __title_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_tizen_ua_check;
	Evas_Object *m_chrome_ua_check;
	Elm_Genlist_Item_Class *m_item_ic;
	genlist_callback_data m_tizen_ua_item;
	genlist_callback_data m_chrome_ua_item;
};

#endif /* USER_AGENT_VIEW_H */
