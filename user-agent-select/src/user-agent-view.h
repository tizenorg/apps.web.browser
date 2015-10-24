/*
 *  ug-browser-user-agent-efl
 *
 * Copyright (c) 2012 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *              Sangpyo Kim <sangpyo7.kim@samsung.com>
 *              Junghwan Kang <junghwan.kang@samsung.com>
 *              Inbum Chang <ibchang@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef USER_AGENT_VIEW_H
#define USER_AGENT_VIEW_H

#include <string>
#include <map>

#include <Evas.h>
#include <ui-gadget-module.h>
#include <vector>
#include "user-agent-gadget.h"

using namespace std;

class User_Agent_DB;

class User_Agent_View
{
public:
	User_Agent_View(Evas_Object *win, ui_gadget_h gadget);
	~User_Agent_View(void);

	Eina_Bool init(void);
	Evas_Object *get_layout(void) { return m_navi; }
private:
	Eina_Bool _create_layout(void);
	void _set_user_agent(void);

	static void __set_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __checkbox_clicked_cb( void *data, Evas_Object *obj, void *event_info);

	static Evas_Object *_gl_icon_get(void *data, Evas_Object *obj, const char *part);
	static char *_gl_label_get(void *data, Evas_Object *obj, const char *part);
	static void __item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __back_button_cb(void *data, Elm_Object_Item *it);

	typedef struct _genlist_callback_data {
		void *user_data;
		Elm_Object_Item *it;
		std::string user_agent;
		int self_index;
	} genlist_callback_data;

	Evas_Object *m_main_layout;
	Evas_Object *m_win;
	Evas_Object *m_navi;
	Evas_Object *m_genlist;
	Evas_Object *m_radio_main;
	int m_selected_user_agent_index;
	std::map<std::string, std::string> m_user_agent_list;
	std::vector<genlist_callback_data *> m_item_callback_data_list;

	Elm_Genlist_Item_Class *m_item_class;

	User_Agent_DB *m_user_agent_db;
	ui_gadget_h m_gadget;
};

#endif // USER_AGENT_VIEW_H
