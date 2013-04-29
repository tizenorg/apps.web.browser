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

#include "website-setting-item.h"

#ifndef WEBSITE_SETTING_ITEM_VIEW_H
#define WEBSITE_SETTING_ITEM_VIEW_H

class website_setting_item;
class website_setting_item_view : browser_object, public common_view {
public:
	website_setting_item_view(website_setting_item *item);
	~website_setting_item_view(void);

	typedef enum _menu_type {
		WEBSITE_SETTING_ITEM_VIEW_MENU_GEOLOCATION,
		WEBSITE_SETTING_ITEM_VIEW_MENU_WEB_STORAGE,
		WEBSITE_SETTING_ITEM_VIEW_MENU_UNKNOWN
	} menu_type;

	void show(void);
private:
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_genlist(Evas_Object *parent);
	void _refresh_item_view(void);

	static char *__genlist_geolocation_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_geolocation_icon_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_clear_geolocation_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static char *__genlist_web_storate_label_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_web_storate_icon_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __genlist_clear_web_storage_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	website_setting_item *m_item;
	menu_type m_menu_type;
	Evas_Object *m_genlist;
	Elm_Genlist_Item_Class *m_geolocation_item_ic;
	Elm_Genlist_Item_Class *m_web_storage_item_ic;
	Elm_Object_Item *m_geolocatin_item_it;
	Elm_Object_Item *m_webstorage_item_it;
protected:
};
#endif /* WEBSITE_SETTING_ITEM_VIEW_H */

