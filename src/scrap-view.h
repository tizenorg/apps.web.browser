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

#ifndef SCRAP_VIEW_H
#define SCRAP_VIEW_H

#include <Evas.h>
#include <Elementary.h>
#include <vector>

#include "browser-object.h"
#include "common-view.h"
#include "scrap.h"

class scrap_view : public browser_object, public common_view {
public:
	scrap_view(void);
	~scrap_view(void);

	void show(void);
private:
	typedef struct _scrap_view_genlist_item {
		scrap_item *item;
		Eina_Bool is_checked;
		Eina_Bool is_tag_index;
		Elm_Object_Item *it;
		void *data;
	} scrap_view_genlist_item;

	Evas_Object *_create_genlist(Evas_Object *parent);
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_tag_genlist(Evas_Object *parent);
	Eina_Bool _is_tag_view(void);
	void _change_tag_view(Eina_Bool tag_view);
	void _set_edit_mode(Eina_Bool edit_mode);
	Evas_Object *_create_search_bar(Evas_Object *parent);
	void _search_scrap(const char *keyword);
	Evas_Object *_create_search_genlist(Evas_Object *parent);
	Evas_Object *_create_search_genlist_layout(Evas_Object *parent);

	static char *__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static char *__tag_genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static char *__search_genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__tag_genlist_content_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __tag_genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static Evas_Object *__genlist_content_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_realized_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_tag_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_confirm_cb(void *data, Evas_Object *obj, void *event_info);
	static void __sweep_right_cb(void *data, Evas_Object *obj, void *event_info);
	static void __sweep_left_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_scrap_done_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_cb(void *data, Evas_Object *obj, void *event_info);
	static void __more_cb(void *data, Evas_Object *obj, void *event_info);
	static void __search_cb(void *data, Evas_Object *obj, void *event_info);
//	static void __view_by_tag_cb(void *data, Evas_Object *obj, void *event_info);
//	static void __view_by_date_cb(void *data, Evas_Object *obj, void *event_info);
	static void __more_delete_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __toolbar_delete_cb(void *data, Evas_Object *obj, void *event_info);
	static void __select_all_cb(void *data, Evas_Object *obj, void *event_info);
	static void __toolbar_delete_confirm_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ime_show_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ime_hide_cb(void *data, Evas_Object *obj, void *event_info);
	static void __search_back_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __search_keyword_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __search_genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	Elm_Genlist_Item_Class *m_item_ic;
	Elm_Genlist_Item_Class *m_group_title_ic;
	Elm_Genlist_Item_Class *m_group_title_icon_ic;
	Elm_Object_Item *m_naviframe_item;

	std::vector<scrap_item *> m_scrap_list;
	Evas_Object *m_genlist;
	Evas_Object *m_main_layout;

	Elm_Genlist_Item_Class *m_tag_group_index_ic;
	Elm_Genlist_Item_Class *m_tag_item_ic;

	Evas_Object *m_tag_genlist;
	std::vector<char *> m_tag_list;
	Evas_Object *m_more_button;
	Evas_Object *m_search_button;
	Evas_Object *m_toolbar_delete_button;
	Evas_Object *m_title_select_all_button;
	Evas_Object *m_searchbar_layout;

	Evas_Object *m_search_genlist;
	Evas_Object *m_search_genlist_layout;
	Evas_Object *m_no_contents_layout;
	Evas_Object *m_unset_genlist;

	Elm_Genlist_Item_Class *m_search_item_ic;

	std::vector<scrap_view_genlist_item *> m_genlist_item_list;
	Eina_Bool m_select_all_flag;
};

#endif /* SCRAP_VIEW_H */

