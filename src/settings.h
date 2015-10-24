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
 * Contact: Junghwan Kang <junghwan.kang@samsung.com>
 *
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <vector>

#include "browser.h"
#include "common-view.h"
#include "settings-advanced.h"
#include "settings-basic.h"

typedef enum _settings_menu_index
{
	br_settings_main_menu_start = 0,

	br_settings_main_menu_basic_item_start = br_settings_main_menu_start,
	br_settings_main_menu_basic_item_title,
	br_settings_main_menu_basic_item_set_homepage,
	br_settings_main_menu_basic_item_set_homepage_sub_item_default_page,
	br_settings_main_menu_basic_item_set_homepage_sub_item_current_page,
	br_settings_main_menu_basic_item_set_homepage_sub_item_set_homepage_url,
	br_settings_main_menu_basic_item_auto_fill_forms,
	br_settings_main_menu_basic_item_end,

	br_settings_main_menu_advanced_item_start = br_settings_main_menu_basic_item_end,
	br_settings_main_menu_advanced_item_title,
	br_settings_main_menu_advanced_item_privacy,
	br_settings_main_menu_advanced_item_contents_settings,
	br_settings_main_menu_advanced_item_bandwidth_management,
	br_settings_main_menu_advanced_item_developer_mode,
	br_settings_main_menu_advanced_item_bottom_margin,
	br_settings_main_menu_advanced_item_end,

	br_settings_main_menu_end = br_settings_main_menu_advanced_item_end,

} settings_menu_index;

typedef struct _settings_genlist_callback_data {
	settings_menu_index type;
	void *user_data;
	Elm_Object_Item *it;
} settings_genlist_callback_data;


class homepage_edit;
class privacy_edit;
class screen_and_text_edit;
class contents_settings_edit;
class bandwidth_management_edit;
class developer_mode_edit;
class settings : public common_view {
public:
	settings(void);
	~settings(void);

	Eina_Bool show(void);
	virtual void clear_popups(void);
	void hide_setting_popups(void);

	homepage_edit *get_homepage_edit(void);
	void delete_homepage_edit(void);

	set_homepage_view *get_homepage_view(void);
	void delete_homepage_view(void);

	auto_fill_form_manager *get_auto_fill_form_manager(void);
	void delete_auto_fill_form_manager(void);

	privacy_edit *get_privacy_edit(void);
	Eina_Bool is_privacy_edit_exist(void);
	Eina_Bool is_content_settings_edit_exist(void);
	void delete_privacy_edit(void);

	contents_settings_edit *get_contents_settings_edit(void);
	void delete_contents_settings_edit(void);

	bandwidth_management_edit *get_bandwidth_management_edit(void);
	void delete_bandwidth_management_edit(void);
	developer_mode_edit *get_developer_mode_edit(void);
	void delete_developer_mode_edit(void);
	Eina_Bool is_shown(void);
	void set_homepage_type_in_genlist(homepage_type type);
	void hide_homepage_sub_items_in_genlist(void);
	void realize_genlist(void) { elm_genlist_realized_items_update(m_genlist); }
	Evas_Object * get_genlist(void) {return m_genlist;}

private:
	Eina_Bool _create_main_layout(Evas_Object *parent);
	Elm_Genlist_Item_Class *_create_genlist_item_class(settings_menu_index index);
	Elm_Genlist_Item_Class *_get_genlist_item_class(settings_menu_index index);
	Elm_Object_Item *_get_genlist_item(settings_menu_index index);

	void _destroy_genlist_styles(void);
	void _destroy_genlist_callback_datas(void);
	void _change_homepage_type_current_to_other(void);
	const char *_get_current_page_uri_to_show(void);

	static char *__genlist_text_get(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_contents_get(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info);
	static void __homepage_sub_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __homepage_sub_radio_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __user_homepage_reveal_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __auto_fill_form_check_changed_cb(void *data, Evas_Object *obj, void *event_info);

	static Eina_Bool __naviframe_pop_cb(void *data, Elm_Object_Item *it);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);

	homepage_edit *m_homepage_edit;
	auto_fill_form_manager *m_auto_fill_form_manager;
	privacy_edit *m_privacy_edit;
	set_homepage_view *m_set_homepage_view;
	contents_settings_edit *m_contents_settings_edit;
	bandwidth_management_edit *m_bandwidth_management_edit;

	Evas_Object *m_genlist;
	Evas_Object *m_homepage_radio_group;
	Evas_Object *m_auto_fill_form_check;
	Evas_Object *m_main_layout;
	Elm_Object_Item *m_navi_item;
	developer_mode_edit *m_developer_mode_edit;

	std::vector<Elm_Genlist_Item_Class *> m_genlist_item_class_list;
	std::vector<settings_genlist_callback_data *> m_genlist_callback_data_list;
};
#endif /* SETTINGS_H */

