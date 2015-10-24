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
 * Contact: Karthick R <karthick.r@samsung.com>
 *
 */

#ifndef TAB_MANAGER_LITE_H
#define TAB_MANAGER_LITE_H

#include <Evas.h>
#include <Elementary.h>
#include <vector>

#include "browser-object.h"
#include "common-view.h"
#include "tab-view-item-lite.h"

class tab_manager_view_lite: public common_view {
public:
	tab_manager_view_lite(void);
	~tab_manager_view_lite(void);
	void show();
	void on_rotate(Eina_Bool);
	void delete_tab(webview *deleted_wv);
	void gengrid_item_access_disable(void);
private:
	static Eina_Bool __resize_more_ctxpopup_cb(void *data);
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_gengrid(Evas_Object *parent);
	static Evas_Object *_create_no_contents_layout(Evas_Object *parent);
	void _create_new_tab_button(void);
	void _show_more_context_popup(void);
	Eina_Bool _add_new_webview(void);
	void _delete_webview(webview *deleted_wv);
	void _close_all_webviews();
	void _exit_tab_manager_view_lite();
	void _close_more_context_popup(void);
	static char *__get_text_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__get_content_cb(void *data, Evas_Object *obj, const char *part);
	static void __back_cb(void *data, Evas_Object *obj, void *event_info);
	static void __more_cb(void *data, Evas_Object *obj, void *event_info);
	static void __context_popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __rotate_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __resize_ctxpopup_cb(void *data);
	static void __item_delete_cb(void *data, Evas_Object *obj, void *event_info);
	static void __item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __close_all_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_new_tab_cb(void *data, Evas_Object *obj, void *event_info);
	static void __screenshot_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source);
	static void __access_item_selected_cb(void *data, Evas_Object *obj, void *event_info);

	Elm_Object_Item *m_naviframe_item;
	Evas_Object *m_main_layout;
	Evas_Object *m_grid_layout;
	Evas_Object *m_more_popup;
	Elm_Theme *m_theme;
	Evas_Object *m_gengrid;
	Elm_Gengrid_Item_Class *m_multiwindow_gengrid_ic;
	std::vector<tab_view_item_lite *> m_tabview_item_list;
};

#endif


