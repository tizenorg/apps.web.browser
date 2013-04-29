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

#ifndef WEBSITE_SETTING_VIEW_H
#define WEBSITE_SETTING_VIEW_H

#include <Evas.h>
#include <vector>

#include "browser-object.h"
#include "common-view.h"
#include "website-setting-manager.h"

class website_setting_view : browser_object, public common_view {
public:
	website_setting_view(void);
	~website_setting_view(void);
	void show(void);
	void refresh_view(void);
	void add_website_setting_item_in_genlist(website_setting_item *item);
	website_setting_manager *get_website_setting_manager(void);
	void delete_all_button_disabled_set(Eina_Bool disabled);
private:
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_genlist(Evas_Object *parent);
	void _delete_website_setting_manager(void);

	static char *__genlist_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_icon_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __delete_all_btn_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_all_confirmed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_all_denied_cb(void *data, Evas_Object *obj, void *event_info);
	static void __item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_genlist;
	Evas_Object *m_delete_all_button;
	Elm_Object_Item *m_naviframe_item;
	Elm_Genlist_Item_Class *m_item_ic;
	website_setting_manager *m_website_setting_manager;
	website_setting_item *m_selected_item;
protected:
};
#endif /* WEBSITE_SETTING_VIEW_H */

