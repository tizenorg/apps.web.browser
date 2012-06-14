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

#ifndef BROWSER_SETTINGS_PLUGIN_VIEW_H
#define BROWSER_SETTINGS_PLUGIN_VIEW_H

#include "browser-common-view.h"
#include "browser-config.h"
#include "browser-settings-main-view.h"

class Browser_Settings_Plugin_View : public Browser_Common_View {
public:
	Browser_Settings_Plugin_View(Browser_Settings_Main_View *main_view);
	~Browser_Settings_Plugin_View(void);

	Eina_Bool init(void);

	typedef enum _menu_type {
		BR_RUN_PLUGINS,
		BR_UNKOWN
	} menu_type;
private:
	typedef struct _genlist_callback_data {
		menu_type type;
		void *user_data;
		Elm_Object_Item *it;
	} genlist_callback_data;

	Eina_Bool _create_main_layout(void);

	/* genlist callback functions */
	static Evas_Object *__genlist_icon_get(void *data, Evas_Object *obj, const char *part);
	static char *__genlist_label_get(void *data, Evas_Object *obj, const char *part);

	/* Elementary event callback functions */
	static void __item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __run_plugins_check_button_changed_cb(void *data,
							Evas_Object *obj, void *event_info);

	Evas_Object *m_genlist;
	Evas_Object *m_back_button;
	Evas_Object *m_run_plugins_check_button;

	Elm_Genlist_Item_Class m_seperator_item_class;
	Elm_Genlist_Item_Class m_1_text_1_icon_item_class;

	genlist_callback_data m_run_plugins_callback_data;
	Browser_Settings_Main_View *m_main_view;
};

#endif /* BROWSER_SETTINGS_PLUGIN_VIEW_H */

