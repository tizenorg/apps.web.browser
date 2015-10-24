/*
 * Copyright 2014  Samsung Electronics Co., Ltd
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
 * Contact: Mahesh Domakuntla <mahesh.d@samsung.com>
 *
 */

#ifndef TAB_VIEW_ITEM_LITE_H
#define TAB_VIEW_ITEM_LITE_H

#include <Evas.h>
#include "browser-object.h"

class webview;
class tab_view_item_lite : public browser_object {
public:
	tab_view_item_lite(webview *wv, Evas_Smart_Cb delete_cb, void *data, Eina_Bool is_current_tab = EINA_FALSE);
	~tab_view_item_lite(void);

	const char *get_title(void) { return m_title; }
	const char *get_uri(void) { return m_uri; }

	Evas_Object *get_layout(void) { return m_layout; }
	void update_snapshot(void);
	void update_title(void);
	webview *get_webview(void) { return m_wv; }
	//void focus_item(Eina_Bool focus);
	Evas_Object *get_item_layout(Evas_Object *parent);
#ifdef ENABLE_INCOGNITO_WINDOW
	void set_current_tab_status(Eina_Bool is_current_tab, Eina_Bool is_incognito = false);
#else
	void set_current_tab_status(Eina_Bool is_current_tab);
#endif
	void set_gengrid_item(Elm_Object_Item *it) { m_gengrid_item = it; }
	void delete_gengrid_item(void);
	Evas_Object *m_container_layout;
	const bool deleteButtonPressed() const {
		return m_delete_button_pressed;
	}

private:
	static void __delete_item_cb(void *data, Evas_Object *obj, void *event_info);
	static void __pressed_delete_cb(void *data, Evas_Object *obj,
			void *event_info);
	static void __unpressed_delete_cb(void *data, Evas_Object *obj,
			void *event_info);

	const char *m_title;
	const char *m_uri;
	webview *m_wv;
	Evas_Object *m_favicon;
	Evas_Smart_Cb m_delete_cb;
	void *m_cb_data;

	Evas_Object *m_layout;

	Eina_Bool m_is_current_tab;
	Elm_Object_Item *m_gengrid_item;

	bool m_delete_button_pressed;
};

#endif /* TAB_VIEW_ITEM_H */

