/*
  * Copyright 2012  Samsung Electronics Co., Ltd
  *
  * Licensed under the Flora License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *    http://www.tizenopensource.org/license
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */

#ifndef BROWSER_PREDICTIVE_HISTORY_H
#define BROWSER_PREDICTIVE_HISTORY_H

#include "browser-config.h"
#include "browser-common-view.h"

class Browser_History_DB;
class Browser_View;

class Browser_Predictive_History : public Browser_Common_View {
public:
	Browser_Predictive_History(Evas_Object *navi_bar, Browser_History_DB *history_db,
								Browser_View *browser_view);
	~Browser_Predictive_History(void);

	Evas_Object *create_predictive_history_layout(void);
	void url_changed(const char *url);

	Eina_Bool init(void) {}
private:
	typedef struct _genlist_callback_param{
		Browser_Predictive_History *predictive_history;
		int index;
	} genlist_callback_param;

	static Eina_Bool __load_uri_idler_cb(void *data);
	/* genlist callback functions */
	static char *__genlist_predictive_history_get(void *data,
						Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_history_icon_get_cb(void *data,
						Evas_Object *obj, const char *part);

	/* elementary event callback functions */
	static void __predictive_history_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_scrolled_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_navi_bar;
	Browser_History_DB *m_history_db;
	Browser_View *m_browser_view;

	Evas_Object *m_main_layout;
	Evas_Object *m_genlist;
	Elm_Genlist_Item_Class m_item_class;

	std::vector<std::string> m_history_list;
	genlist_callback_param m_param[BROWSER_PREDICTIVE_HISTORY_COUNT];

	std::string m_uri_to_load;
};
#endif /* BROWSER_PREDICTIVE_HISTORY_H */

