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

#ifndef CLEAR_PRIVATE_DATA_VIEW_H
#define CLEAR_PRIVATE_DATA_VIEW_H


#include "browser-object.h"
#include "common-view.h"

class clear_private_data_view : public common_view {
public:
	typedef enum _menu_type
	{
		CELAR_PRIVATE_DATA_VIEW_SELECT_ALL = 0,
		CELAR_PRIVATE_DATA_VIEW_CLEAR_HISTORY,
		CELAR_PRIVATE_DATA_VIEW_CLEAR_CACHE,
		CELAR_PRIVATE_DATA_VIEW_CLEAR_COOKIE,
		CELAR_PRIVATE_DATA_VIEW_CLEAR_SAVED_PASSWORD,
		CELAR_PRIVATE_DATA_VIEW_MENU_UNKNOWN
	} menu_type;
	clear_private_data_view(void);
	~clear_private_data_view(void);
	Eina_Bool show(void);
private:
	typedef struct _genlist_callback_data {
		menu_type type;
		void *user_data;
		Evas_Object *checkbox;
		Elm_Object_Item *it;
	} genlist_callback_data;
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_genlist(Evas_Object *parent);
	Evas_Object *_create_select_all_layout(Evas_Object *parent);
	void _delete_private_data(void);

	static char *__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __select_all_chceckbox_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __select_all_layout_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
	static void __checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_ok_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	genlist_callback_data m_clear_history_data;
	genlist_callback_data m_clear_cache_data;
	genlist_callback_data m_clear_cookie_data;
	genlist_callback_data m_clear_saved_password_data;

	Evas_Object *m_content_box;
	Evas_Object *m_select_all_layout;
	Evas_Object *m_genlist;
	Evas_Object *m_delete_button;
	Elm_Genlist_Item_Class *m_item_ic;
	Elm_Genlist_Item_Class *m_select_all_item_ic;
protected:
};

#endif /* CLEAR_PRIVATE_DATA_VIEW_H */
