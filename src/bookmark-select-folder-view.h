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

#ifndef BOOKMARK_SELECT_FOLDER_VIEW_H
#define BOOKMARK_SELECT_FOLDER_VIEW_H

#include <Elementary.h>
#include <iostream>

#include "browser-object.h"
#include "common-view.h"
#include "bookmark-listener.h"

class bookmark_select_folder_view : public common_view, public bookmark_listener {
public:

	bookmark_select_folder_view(Evas_Smart_Cb cb_func = NULL, void *cb_data = NULL);
	~bookmark_select_folder_view(void);

	void show();
	void refresh();
private:
	typedef struct _gl_cb_data {
		void *user_data;
		void *cp;
		Elm_Object_Item *it;
	} gl_cb_data;
	Evas_Object *_create_genlist(Evas_Object *parent);
	Eina_Bool _set_genlist_folder_tree(Evas_Object *genlist);
	Eina_Bool _set_genlist_item_by_folder(Evas_Object *genlist, int folder_id, Elm_Object_Item *parent_it);
	void _back_to_previous_view(void);
	void _clear_genlist_item_data(Evas_Object *genlist);
	static char *__genlist_get_text_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_get_content_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __resize_more_ctxpopup_cb(void *data);
	static void __ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __more_cb(void *data, Evas_Object *obj, void *event_info);
	void _show_more_context_popup(void);
	static void __ctxpopup_create_folder_cb(void *data, Evas_Object *obj, void *event_info);
	static void __launch_create_folder_cb(void *data, Evas_Object *obj, void *event_info);
	virtual void __bookmark_added(const char *uri,	int bookmark_id, int parent_id);
	virtual void __bookmark_removed(const char *uri, int bookmark_id, int parent_id) ;
	virtual void __bookmark_updated(const char *uri, const char *title, int bookmark_id, int parent_id);
	static void rotate_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info);
	static void __create_folder_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_folder_genlist;
	bookmark *m_bookmark;
	Evas_Object *m_ctx_popup_more_menu;
	Elm_Object_Item *m_naviframe_item;
	Elm_Genlist_Item_Class *m_itc_folder;

	Evas_Smart_Cb m_cb_func;
	void *m_cb_data;
};

#endif /* BOOKMARK_SELECT_FOLDER_VIEW_H */

