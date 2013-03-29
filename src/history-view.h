/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.0 (the License);
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

#ifndef HISTORY_VIEW_H
#define HISTORY_VIEW_H

#include <Evas.h>
#include <Elementary.h>
#include <vector>

#include "browser-object.h"
#include "common-view.h"
#include "history-item.h"

class history_view : public browser_object, public common_view {
public:
	history_view(void);
	~history_view(void);

	void show(void);
private:
	Evas_Object *_create_genlist(Evas_Object *parent);
	Evas_Object *_create_main_layout(Evas_Object *parent);
	static void __naviframe_pop_cb(void *data, Evas_Object *obj, void *event_info);
	static void __clear_history(void *data, Evas_Object *obj, void *event_info);
	static char *__genlist_date_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static char *__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __sweep_right_genlist_cb(void *data, Evas_Object *obj, void *event_info);
	static void __sweep_left_genlist_cb(void *data, Evas_Object *obj, void *event_info);
	static void __sweep_cancel_genlist_cb(void *data, Evas_Object *obj, void *event_info);
	static void __bookmark_on_off_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __slide_add_to_bookmark_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __slide_delete_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_confirm_response_by_slide_button_cb(void *data, Evas_Object *obj, void *event_info);
	void _delete_history_item_by_slide_button(history_item *item);
	void _delete_date_only_label_genlist_item(void);
	static void __clear_history_button_cb(void *data, Evas_Object *obj, void *event_info);

	Elm_Object_Item *m_naviframe_item;
	Evas_Object *m_main_layout;
	Evas_Object *m_genlist;
	Elm_Genlist_Item_Class *m_item_ic;
	Elm_Genlist_Item_Class *m_date_ic;
	Elm_Object_Item *m_current_sweep_item;
	std::vector<history_item *> m_history_list;
	Eina_Bool m_is_bookmark_on_off_icon_clicked;
	Evas_Object *m_clear_button;
};

#endif /* HISTORY_VIEW_H */

