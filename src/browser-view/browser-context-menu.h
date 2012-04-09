/*
Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved

This file is part of browser
Written by Hyerim Bae hyerim.bae@samsung.com.

PROPRIETARY/CONFIDENTIAL

This software is the confidential and proprietary information of
SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered
into with SAMSUNG ELECTRONICS.

SAMSUNG make no representations or warranties about the suitability
of the software, either express or implied, including but not limited
to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. SAMSUNG shall not be liable for any
damages suffered by licensee as a result of using, modifying or
distributing this software or its derivatives.
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

