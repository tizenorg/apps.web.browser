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

#ifndef ADD_TAG_VIEW_H
#define ADD_TAG_VIEW_H

#include <Evas.h>
#include <Elementary.h>
#include <vector>

#include "browser-object.h"
#include "common-view.h"

class add_tag_view : public browser_object, public common_view {
public:
	add_tag_view(Evas_Smart_Cb done_cb = NULL, void *done_cb_data = NULL, const char *tag = NULL);
	~add_tag_view(void);

	void show(void);
private:
	typedef struct _tag_item {
		char *tag;
		Eina_Bool is_checked;
		void *data;
	} tag_item;

	Evas_Object *_create_genlist(Evas_Object *parent);
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_tagbar_layout(Evas_Object *parent);
	Evas_Object *_create_multibutton_entry(Evas_Object *parent);
	Eina_Bool _add_tag_to_multibutton_entry(const char *tag);
	void _delete_tag_from_multibutton_entry(const char *tag);
	int _get_multibutton_entry_count(void);
	void _show_tag_clicked_popup(Elm_Object_Item *item, Evas_Object *parent);
	void _delete_multibutton_entry_item(Elm_Object_Item *item);

	static char *__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_content_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __tag_add_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void _tag_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __tag_delete_cb(void *data, Evas_Object *obj, void *event_info);
	static void __tag_edit_cb(void *data, Evas_Object *obj, void *event_info);
	static void __done_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ime_show_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ime_hide_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_scrap_done_cb(void *data, Evas_Object *obj, void *event_info);
	static void __check_changed_cb(void *data, Evas_Object *obj, void *event_info);

	Elm_Genlist_Item_Class *m_item_ic;
	Evas_Object *m_main_layout;
	Evas_Object *m_genlist;
	Evas_Object *m_tagbar_layout;
	Evas_Object *m_tag_entry;
	Evas_Object *m_multibutton_entry;
	Elm_Object_Item *m_multibutton_entry_clicked_item;

	Evas_Smart_Cb m_done_cb;
	void *m_done_cb_data;

	std::vector<char *> m_tag_list;
	std::vector<tag_item *> m_tag_item_list;
	const char *m_init_tag;
};

#endif /* ADD_TAG_VIEW_H */

