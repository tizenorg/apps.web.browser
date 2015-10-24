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

#ifndef SETTINGS_ADVANCED_H
#define SETTINGS_ADVANCED_H

#include <ui-gadget.h>
#include <vector>

#include "browser-object.h"
#include "clear-private-data-view.h"
#include "common-view.h"
#include "preference.h"
#include "webview.h"

#include "custom-user-agent-view.h"

typedef enum _settings_advanced_menu_index
{
	br_settings_sub_advanced_menu_start = 0,
	br_settings_sub_advanced_menu_screen_and_text_start = br_settings_sub_advanced_menu_start,
	br_settings_sub_advanced_menu_screen_and_text_end,

	br_settings_sub_advanced_menu_contents_start = br_settings_sub_advanced_menu_screen_and_text_end,
	br_settings_sub_advanced_menu_contents_accept_cookies,
	br_settings_sub_advanced_menu_contents_accept_cookies_desc,
	br_settings_sub_advanced_menu_contents_accept_cookies_bottom_margin,
	br_settings_sub_advanced_menu_contents_enable_javascript,
	br_settings_sub_advanced_menu_contents_enable_javascript_desc,
	br_settings_sub_advanced_menu_contents_enable_javascript_bottom_margin,
	br_settings_sub_advanced_menu_contents_default_storage,
	br_settings_sub_advanced_menu_contents_default_storage_bottom_margin,
	br_settings_sub_advanced_menu_contents_reset_to_default_title,
	br_settings_sub_advanced_menu_contents_reset_to_default,
	br_settings_sub_advanced_menu_contents_reset_to_default_bottom_margin,
	br_settings_sub_advanced_menu_contents_end,

	br_settings_sub_advanced_menu_privacy_start = br_settings_sub_advanced_menu_contents_end,
	br_settings_sub_advanced_menu_privacy_remember_form_data,
	br_settings_sub_advanced_menu_privacy_remember_form_data_desc,
	br_settings_sub_advanced_menu_privacy_remember_form_data_bottom_margin,
	br_settings_sub_advanced_menu_privacy_remember_passwords,
	//br_settings_sub_advanced_menu_privacy_remember_passwords_desc,
	br_settings_sub_advanced_menu_privacy_remember_passwords_bottom_margin,
	br_settings_sub_advanced_menu_privacy_delete_personal_data,
	//br_settings_sub_advanced_menu_privacy_delete_personal_data_desc,
	br_settings_sub_advanced_menu_privacy_delete_personal_data_bottom_margin,
	br_settings_sub_advanced_menu_privacy_end,

	br_settings_sub_advanced_menu_bandwidth_management_start = br_settings_sub_advanced_menu_privacy_end,
	br_settings_sub_advanced_menu_bandwidth_management_load_images,
	br_settings_sub_advanced_menu_bandwidth_management_load_images_desc,
	br_settings_sub_advanced_menu_bandwidth_management_load_images_bottom_margin,
	br_settings_sub_advanced_menu_bandwidth_management_open_pages_in_overview,
	br_settings_sub_advanced_menu_bandwidth_management_open_pages_in_overview_desc,
	br_settings_sub_advanced_menu_bandwidth_management_open_pages_in_overview_bottom_margin,
	br_settings_sub_advanced_menu_bandwidth_management_end,

	br_settings_sub_advanced_menu_developer_mode_start = br_settings_sub_advanced_menu_bandwidth_management_end,
	br_settings_sub_advanced_menu_developer_mode_user_agent,
	br_settings_sub_advanced_menu_developer_mode_custom_user_agent,
	br_settings_sub_advanced_menu_developer_mode_custom_user_agent_bottom_margin,
	br_settings_sub_advanced_menu_developer_mode_end,

	br_settings_sub_advanced_menu_end = br_settings_sub_advanced_menu_developer_mode_end
} settings_advanced_menu_index;

typedef struct _settings_advanced_genlist_callback_data {
	settings_advanced_menu_index menu_index;
	void *user_data;
	Elm_Object_Item *it;
} settings_advanced_genlist_callback_data;


class privacy_edit : public common_view {
public:
	privacy_edit(void);
	~privacy_edit(void);

	Eina_Bool show(void);

	clear_private_data_view *get_clear_private_data_view(void);
	Eina_Bool is_clear_private_data_view_exist(void);
	void delete_clear_private_data_view(void);
	Eina_Bool view_is_shown(void) { return (m_genlist) ? EINA_TRUE : EINA_FALSE; }
	Evas_Object * get_genlist(void) {return m_genlist;}
	void mdm_mode_disable_autofill_setting_menu(Eina_Bool disabled);
	void on_rotate(void);

private:
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Elm_Genlist_Item_Class *_create_genlist_item_class(settings_advanced_menu_index index);

	void _destroy_genlist_styles(void);
	void _destroy_genlist_callback_datas(void);

	static char *__genlist_text_get(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_contents_get(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info);

	static void __title_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_button_cb(void *data, Evas_Object *obj, void *event_info);

	static void __remember_form_data_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __remember_passwords_check_changed_cb(void *data, Evas_Object *obj, void *event_info);

	static void __privacy_edit_naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);

	clear_private_data_view *m_clear_private_data_view;

	Evas_Object *m_genlist;
	Elm_Object_Item *m_navi_item;
	std::vector<Elm_Genlist_Item_Class *> m_genlist_item_class_list;
	std::vector<settings_advanced_genlist_callback_data *> m_genlist_callback_data_list;
protected:
};



class screen_and_text_edit : public common_view {
public:
	screen_and_text_edit(void);
	~screen_and_text_edit(void);

	Eina_Bool show(void);
	void rotate(void);

	void realize_genlist(void) { elm_genlist_realized_items_update(m_genlist); }
	Evas_Object * get_genlist(void) {return m_genlist;}
	Eina_Bool view_is_shown(void) { return (m_genlist) ? EINA_TRUE : EINA_FALSE; }

private:
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Elm_Genlist_Item_Class *_create_genlist_item_class(settings_advanced_menu_index index);

	void _destroy_genlist_styles(void);
	void _destroy_genlist_callback_datas(void);

	static char *__genlist_text_get(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_contents_get(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info);

	std::vector<Elm_Genlist_Item_Class *> m_genlist_item_class_list;
	std::vector<settings_advanced_genlist_callback_data *> m_genlist_callback_data_list;

	Evas_Object *m_genlist;
	Elm_Object_Item *m_navi_item;
	Elm_Theme *m_theme;

protected:
};





class default_storage_ask_popup;
class contents_settings_edit : public common_view {
public:
	contents_settings_edit(void);
	~contents_settings_edit(void);

	Eina_Bool show(void);
	default_storage_ask_popup *get_default_storage_ask_popup(void);
	Elm_Object_Item *get_naviframe_item(void) { return m_navi_item; }
	void delete_default_storage_ask_popup(void);
	void delete_drag_and_drop_desc_popup(void);
	void delete_reset_to_default_ask_popup(void);

	void realize_genlist(void) { elm_genlist_realized_items_update(m_genlist); }
	Eina_Bool view_is_shown(void) { return (m_genlist) ? EINA_TRUE : EINA_FALSE; }
	Evas_Object * get_genlist(void) {return m_genlist;}
	void mdm_mode_disable_accept_cookie_setting_menu(Eina_Bool disabled);
	void mdm_mode_disable_javascript_setting_menu(Eina_Bool disabled);

private:
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Elm_Genlist_Item_Class *_create_genlist_item_class(settings_advanced_menu_index index);

	void _destroy_genlist_styles(void);
	void _destroy_genlist_callback_datas(void);

	void _reset_to_default_setting(void);

	static char *__genlist_text_get(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_contents_get(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info);

	static void __accept_cookies_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __run_javascript_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __default_storage_reveal_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __reset_to_default_reveal_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __reset_confirm_response_cb(void *data, Evas_Object *obj, void *event_info);
	static void __reset_confirm_cancel_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);

	default_storage_ask_popup *m_default_storage_ask_popup;

	std::vector<Elm_Genlist_Item_Class *> m_genlist_item_class_list;
	std::vector<settings_advanced_genlist_callback_data *> m_genlist_callback_data_list;

	Evas_Object *m_genlist;
	Evas_Object *m_enable_drag_and_drop_check;
	Evas_Object *m_drag_and_drop_desc_popup;
	Evas_Object *m_reset_to_default_popup;
	Elm_Object_Item *m_navi_item;
	Elm_Theme *m_theme;

protected:
};





class bandwidth_management_edit : public common_view {
public:
	bandwidth_management_edit(void);
	~bandwidth_management_edit(void);

	Eina_Bool show(void);
	void realize_genlist(void) { elm_genlist_realized_items_update(m_genlist); }
	Eina_Bool view_is_shown(void) { return (m_genlist) ? EINA_TRUE : EINA_FALSE; }
	Evas_Object * get_genlist(void) {return m_genlist;}

private:
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Elm_Genlist_Item_Class *_create_genlist_item_class(settings_advanced_menu_index index);

	void _destroy_genlist_styles(void);
	void _destroy_genlist_callback_datas(void);

	static char *__genlist_text_get(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_contents_get(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info);

	static void __load_images_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __open_pages_in_overview_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	std::vector<Elm_Genlist_Item_Class *> m_genlist_item_class_list;
	std::vector<settings_advanced_genlist_callback_data *> m_genlist_callback_data_list;

	Evas_Object *m_genlist;
	Elm_Object_Item *m_navi_item;
	Elm_Theme *m_theme;

protected:
};





class default_storage_ask_popup : public common_view {
public:
	default_storage_ask_popup(void);
	~default_storage_ask_popup(void);

	Eina_Bool show(void);
private:
	Evas_Object *_create_genlist(Evas_Object *parent);

	void _destroy_genlist_callback_datas(void);
	static char *__genlist_text_get(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_contents_get(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);

	static void __radio_icon_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ok_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_realized_cb(void *data, Evas_Object *obj, void *event_info);

	std::vector<Elm_Genlist_Item_Class *> m_genlist_item_class_list;
	std::vector<settings_advanced_genlist_callback_data *> m_genlist_callback_data_list;

	default_storage_type m_selected_type;

	Evas_Object *m_radio_main;
	Evas_Object *m_popup;
	Elm_Object_Item *m_popup_last_item;

	int m_mmc_mode;
protected:
};

class developer_mode_edit : public common_view {
public:
	developer_mode_edit(void);
	~developer_mode_edit(void);

	Eina_Bool show(void);
	custom_user_agent_view *get_custom_user_agent_view(void);
	Eina_Bool view_is_shown(void) { return (m_genlist) ? EINA_TRUE : EINA_FALSE; }

private:
	Evas_Object *_create_main_layout(Evas_Object *parent);
	Elm_Genlist_Item_Class *_create_genlist_item_class(settings_advanced_menu_index index);
	Eina_Bool _call_user_agent(void);
	void _delete_custom_user_agent_view(void);

	void _destroy_genlist_styles(void);
	void _destroy_genlist_callback_datas(void);

	static char *__genlist_text_get(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_contents_get(void *data, Evas_Object *obj, const char *part);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info);
	static void __title_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_button_cb(void *data, Evas_Object *obj, void *event_info);

	static void __ug_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv);
	static void __ug_destroy_cb(ui_gadget_h ug, void *priv);

	custom_user_agent_view *m_custom_user_agent_view;

	std::vector<Elm_Genlist_Item_Class *> m_genlist_item_class_list;
	std::vector<settings_advanced_genlist_callback_data *> m_genlist_callback_data_list;

	Evas_Object *m_genlist;
	Elm_Object_Item *m_navi_item;
protected:
};

#endif /* SETTINGS_ADVANCED_H */
