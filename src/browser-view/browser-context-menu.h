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

#ifndef BROWSER_CONTEXT_MENU_H
#define BROWSER_CONTEXT_MENU_H

#include "browser-config.h"

class Browser_View;
class Browser_Context_Menu {
public:
	Browser_Context_Menu(Evas_Object *naviframe, Browser_View *browser_view);
	~Browser_Context_Menu(void);

	Eina_Bool init(Evas_Object *webview, Eina_Bool reader_view = EINA_FALSE);
	void deinit(void);

	void destroy_context_popup(void);

	typedef struct _context_popup_item_callback_param {
		Ewk_Context_Menu_Item *menu_item;
		void *user_data;
	} context_popup_item_callback_param;
private:
	/* evas object smart callback functions  */
	static void __context_menu_new_cb(void *data, Evas_Object *obj, void *event_info);
	static void __context_menu_show_cb(void *data, Evas_Object *obj, void *event_info);
	static void __context_menu_move_cb(void *data, Evas_Object *obj, void *event_info);
	static void __context_menu_hide_cb(void *data, Evas_Object *obj, void *event_info);
	static void __context_menu_del_cb(void *data, Evas_Object *obj, void *event_info);
	static void __context_menu_customize_cb(void *data, Evas_Object *obj, void *event_info);
	static void __context_menu_save_as_cb(void *data, Evas_Object *obj, void *event_info);
	static void __magnifier_shown_cb(void *data, Evas_Object *obj, void *event_info);
	static void __magnifier_hidden_cb(void *data, Evas_Object *obj, void *event_info);

	static void __context_menu_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __context_menu_paste_more_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __context_popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);

	std::string _get_context_menu_item_text(Ewk_Context_Menu_Action action);

	Evas_Object *m_naviframe;
	Evas_Object *m_webview;
	Evas_Object *m_context_popup;
	Elm_WebView_Context_Menu_Data *m_context_menu_data;
	Eina_Bool m_for_reader_view;

	Browser_View *m_browser_view;

	std::vector<context_popup_item_callback_param *> m_param_list;

	Ewk_Context_Menu_Item *m_current_context_menu_item;
};
#endif /* BROWSER_CONTEXT_MENU_H */

