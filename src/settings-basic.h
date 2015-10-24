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

#ifndef SETTINGS_BASIC_H
#define SETTINGS_BASIC_H

#include <string>
#include <vector>

#include "browser.h"
#include "common-view.h"
#include "preference.h"
#include "webview.h"

typedef enum _settings_basic_menu_index
{
	br_settings_sub_basic_menu_start = 0,

	br_settings_sub_basic_menu_account_edit_end = br_settings_sub_basic_menu_start,

	br_settings_sub_basic_menu_set_homepage_start = br_settings_sub_basic_menu_account_edit_end,
	br_settings_sub_basic_menu_set_homepage_current_page,
	br_settings_sub_basic_menu_set_homepage_set_homepage_url,
	br_settings_sub_basic_menu_set_homepage_end,

	br_settings_sub_basic_menu_end = br_settings_sub_basic_menu_set_homepage_end,
} settings_basic_menu_index;

typedef struct _settings_basic_genlist_callback_data {
	settings_basic_menu_index menu_index;
	void *user_data;
	Elm_Object_Item *it;
} settings_basic_genlist_callback_data;

class set_homepage_view : public common_view {
public:
	set_homepage_view(void);
	~set_homepage_view(void);

	Eina_Bool show(void);
	void destroy_homepage_edit_popup_show(void);

private:
	Eina_Bool _is_valid_homepage (const char *uri);
	Eina_Bool _create_main_layout(Evas_Object *parent);
	Elm_Genlist_Item_Class *_create_genlist_item_class(settings_basic_menu_index index);
	const char *_get_current_page_uri_to_show(void);

	void _destroy_genlist_styles(void);
	void _destroy_genlist_callback_datas(void);
	void _back_to_previous_view(void);
	void _set_homepage_type_in_genlist(homepage_type type);
	void _change_homepage_type_current_to_other(void);
	Eina_Bool _user_homepage_edit_popup_show(char *homepage);

	static void __set_homepage_ok_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __set_homepage_cancel_button_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __naviframe_pop_cb(void *data, Elm_Object_Item *it);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __homepage_radio_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_realized_cb(void *data, Evas_Object *obj, void *event_info);
	static void __uri_invalid_popup_expired_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_popup_entry_activated_cb(void *data, Evas_Object *obj, void *event_info);
	static char *__genlist_text_get(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__genlist_contents_get(void *data, Evas_Object *obj, const char *part);
	static void __edit_popup_cancel_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_popup_ok_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_popup_entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_popup_editfield_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_popup_clear_button_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_genlist;
	Evas_Object *m_homepage_radio_group;
	Evas_Object *m_popup;
	Evas_Object *m_editfield_entry;
	Evas_Object *m_ok_button;

	Elm_Object_Item *m_navi_item;

	Eina_Bool m_is_closing;
	Eina_Bool m_set_init_homepage;

	Elm_Entry_Filter_Limit_Size m_entry_limit_size;

	int m_previous_radio_state;
	std::string m_input_homepage_uri_str;
	std::vector<Elm_Genlist_Item_Class *> m_genlist_item_class_list;
	std::vector<settings_basic_genlist_callback_data *> m_genlist_callback_data_list;
};

class homepage_edit : public common_view {
public:
	homepage_edit(void);
	~homepage_edit(void);

	Eina_Bool user_homepage_edit_popup_show(char *homepage);
	void destroy_homepage_edit_popup_show(void);

private:
	Eina_Bool _is_valid_homepage (const char *uri);
	void hide_homepage_sub_items(void *data);

	static void __edit_popup_cancel_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_popup_ok_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_popup_entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __edit_popup_entry_activated_cb(void *data, Evas_Object *obj, void *event_info);
	static void __on_shown_clear_btn_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
	static void __uri_invalid_popup_expired_cb(void *data, Evas_Object *obj, void *event_info);

	static void __homepage_edit_keypad_show_cb(void *data, Evas_Object *obj, void *event_info);
	static void __homepage_edit_keypad_hide_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __homepage_edit_keypad_hide_timer_cb(void *data);

	Eina_Bool m_is_closing;

	Evas_Object *m_popup;
	Evas_Object *m_editfield_entry;
	Evas_Object *m_ok_button;
	Elm_Entry_Filter_Limit_Size m_entry_limit_size;

	std::string m_temporary_user_homepage_uri;
	std::vector<Elm_Genlist_Item_Class *> m_genlist_item_class_list;
	std::vector<settings_basic_genlist_callback_data *> m_genlist_callback_data_list;
};

typedef enum _profile_compose_mode
{
	profile_create = 0,
	profile_edit
} profile_compose_mode;

typedef enum _profile_edit_errorcode
{
	profile_edit_failed = 0,
	profile_already_exist,
	update_error_none
} profile_edit_errorcode;

typedef enum _profile_save_errorcode
{
	profile_create_failed = 0,
	duplicate_profile,
	save_error_none
} profile_save_errorcode;

typedef struct _auto_fill_form_item_data {
	unsigned profile_id;
	const char *name;
	const char *company;
	const char *primary_address;
	const char *secondary_address;
	const char *city_town;
	const char *state_province_region;
	const char *post_code;
	const char *country;
	const char *phone_number;
	const char *email_address;
	bool activation;
	profile_compose_mode compose_mode;
} auto_fill_form_item_data;

class auto_fill_form_compose_view;
class auto_fill_form_list_view;
class auto_fill_form_item;
class auto_fill_form_manager {
public:
	auto_fill_form_manager(void);
	~auto_fill_form_manager(void);

	Eina_Bool save_auto_fill_form_item(auto_fill_form_item_data *item_data);
	Eina_Bool delete_auto_fill_form_item(auto_fill_form_item *item);
	Eina_Bool delete_all_auto_fill_form_items(void);
	unsigned int get_auto_fill_form_item_count(void);
	auto_fill_form_item *create_new_auto_fill_form_item(Ewk_Autofill_Profile *profile = NULL);
	auto_fill_form_list_view *show_list_view(void);
	auto_fill_form_list_view *get_list_view(void) { return m_list_view; }
	auto_fill_form_compose_view *show_composer(auto_fill_form_item *item = NULL);
	auto_fill_form_compose_view *get_compose_view(void) { return m_composer; }
	Eina_Bool delete_list_view(void);
	Eina_Bool delete_composer(void);
	std::vector<auto_fill_form_item *> get_item_list(void) { return m_auto_fill_form_item_list; }
	std::vector<auto_fill_form_item *> load_entire_item_list(void);
	Eina_Bool add_item_to_list(auto_fill_form_item *item);
	void refresh_items_view();

	/* test */
	void see_all_data(void);
private:
	static void profiles_updated_cb(void* data);
	std::vector<auto_fill_form_item *> m_auto_fill_form_item_list;
	auto_fill_form_list_view *m_list_view;
	auto_fill_form_compose_view *m_composer;
};

class auto_fill_form_item {
public:
	auto_fill_form_item(auto_fill_form_item_data *item_data);
	~auto_fill_form_item(void);

	friend bool operator==(auto_fill_form_item item1, auto_fill_form_item item2) {
		return ((!item1.get_name() && !item2.get_name()) && !strcmp(item1.get_name(), item2.get_name()));
	}

	unsigned get_profile_id(void) { return m_item_data.profile_id; }
	const char *get_name(void) { return m_item_data.name; }
	const char *get_company(void) {return m_item_data.company; }
	const char *get_primary_address(void) {return m_item_data.primary_address; }
	const char *get_secondary_address2(void) { return m_item_data.secondary_address; }
	const char *get_city_town(void) { return m_item_data.city_town; }
	const char *get_state_province(void) { return m_item_data.state_province_region; }
	const char *get_post_code(void) { return m_item_data.post_code; }
	const char *get_country(void) { return m_item_data.country; }
	const char *get_phone_number(void) { return m_item_data.phone_number; }
	const char *get_email_address(void) { return m_item_data.email_address; }
	Eina_Bool get_activation(void) { return (m_item_data.activation == true) ? EINA_TRUE : EINA_FALSE; }
	profile_compose_mode get_item_compose_mode(void) { return m_item_data.compose_mode; }

	void set_name(const char *name) { m_item_data.name = name; }
	void set_company(const char *company) {m_item_data.company = company; }
	void set_primary_address(const char *primary_address) {m_item_data.primary_address = primary_address; }
	void set_secondary_address2(const char *secondary_address) { m_item_data.secondary_address = secondary_address; }
	void set_city_town(const char *city_town) { m_item_data.city_town = city_town; }
	void set_state_province(const char *state_province_region) { m_item_data.state_province_region = state_province_region; }
	void set_post_code(const char *post_code) { m_item_data.post_code = post_code; }
	void set_country(const char *country) { m_item_data.country = country; }
	void set_phone_number(const char *phone_number) { m_item_data.phone_number = phone_number; }
	void set_email_address(const char *email_address) { m_item_data.email_address = email_address; }
	void set_activation(Eina_Bool activation) { m_item_data.activation = (activation == EINA_TRUE) ? true : false;}

	profile_save_errorcode save_item(void);
	profile_edit_errorcode update_item(void);

private:
	auto_fill_form_item_data m_item_data;
};

class auto_profile_delete_view : public common_view {
public:
	auto_profile_delete_view(void);
	~auto_profile_delete_view(void);
	void show(void);
private:
	typedef enum _menu_type
	{
		DELETE_PROFILE_SELECT_ALL = 0,
		DELETE_PROFILE_ITEM
	} menu_type;
	typedef struct _genlist_callback_data{
		menu_type type;
		unsigned int menu_index;
		void *user_data;
		Eina_Bool is_checked;
		Evas_Object *checkbox;
		Elm_Object_Item *it;
	} genlist_callback_data;
	typedef struct _profile_item_genlist_callback_data {
		genlist_callback_data cd;
		Evas_Object *checkbox;
	} profile_item_genlist_callback_data;

#if defined(DELETE_CONFIRM_POPUP_ENABLE)
	void _set_popup_text();
#endif
	void _set_selected_title(void);
	void _unset_selected_title(void);
	void _back_delete_view(void);
	const char *_get_each_item_full_name(unsigned int index);
	Evas_Object *_create_auto_form_delete_layout(Evas_Object *parent);
	void _remove_each_item_callback_data(void);
	Evas_Object *_create_box(Evas_Object *parent);
#if !defined(DELETE_CONFIRM_POPUP_ENABLE)
	void _item_delete(void);
	void _items_delete_all(void);
#endif
	static char *__text_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__content_get_cb(void *data, Evas_Object *obj, const char *part);
	static Eina_Bool __naviframe_pop_cb(void *data, Elm_Object_Item *it);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
	static void __select_all_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_item_cb(void *data, Evas_Object *obj, void *event_info);
	static void __checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_click_cb(void *data, Evas_Object *obj, void *event_info);
#if defined(DELETE_CONFIRM_POPUP_ENABLE)
	static void __popup_delete_all_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_destroy_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_cancel_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_delete_item_cb(void *data, Evas_Object *obj, void *event_info);
	static void __auto_profile_delete_popup_lang_changed_cb(void *data, Evas_Object *obj, void *event_info);
#endif
	static void __auto_profile_delete_title_lang_changed_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_del_layout;
	Evas_Object *m_del_genlist;
	Evas_Object *m_button_done;
	Evas_Object *m_auto_fill_form_check;
	Evas_Object *m_box;
	Evas_Object *m_select_all_layout;
#if defined(DELETE_CONFIRM_POPUP_ENABLE)
	Evas_Object *m_delete_popup;
#endif
	Elm_Object_Item *m_naviframe_item;
	Eina_Bool m_select_all_flag;
	unsigned int m_count_checked_item;
	genlist_callback_data *m_item_callback_data;
	genlist_callback_data *m_select_all_callback_data;
	std::string m_sub_title;
	std::vector<genlist_callback_data *> m_auto_fill_form_item_callback_data_list;
};

class list_view_item_clicked_popup;
class auto_fill_form_list_view : public common_view {
public:
	auto_fill_form_list_view(auto_fill_form_manager *affm);
	~auto_fill_form_list_view(void);

	Eina_Bool show(void);
	Evas_Object *refresh_view(void);
	list_view_item_clicked_popup *get_list_item_selected_popup(auto_fill_form_item *item);
	void delete_list_item_selected_popup(void);
private:
	typedef enum _menu_type
	{
		aufo_fill_form_description_top = 0,
		aufo_fill_form_description,
		aufo_fill_form_description_desc,
		auto_fill_profile_title,
		auto_fill_profile_each_item,
		aufo_fill_form_bottom
	} menu_type;
	typedef struct _genlist_callback_data {
		menu_type type;
		unsigned int menu_index;
		void *user_data;
		Elm_Object_Item *it;
	} genlist_callback_data;

	typedef struct _profile_item_genlist_callback_data {
		genlist_callback_data cd;
		Evas_Object *checkbox;
	} profile_item_genlist_callback_data;

	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_genlist(Evas_Object *parent);
	Eina_Bool _create_genlist_style_items(void);
	Eina_Bool _destroy_all_genlist_style_items(void);
	const char *_get_each_item_full_name(unsigned int index);
	Eina_Bool _append_genlist(Evas_Object *genlist);
	void _remove_each_item_callback_data(void);
	auto_profile_delete_view *_get_delete_view(void);
	Eina_Bool _is_in_list_view(void);
	void _show_more_context_popup(void);
	static char *__text_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__content_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __check_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_profile_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __back_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_auto_fill_form_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_edit_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_delete_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __naviframe_pop_cb(void *data, Elm_Object_Item *it);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info);
	static void __change_autofillform_check_cb(void *data, Evas_Object *obj, void *event_info);
	static void __naviframe_delete_view_pop_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_item_cb(void *data, Evas_Object *obj, void *event_info);
	static void __add_item_cb(void *data, Evas_Object *obj, void *event_info);
	static void __more_menu_cb(void *data, Evas_Object *obj, void *event_info);
	static void __ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __resize_more_ctxpopup(void *data);
	static void __context_popup_back_cb(void *data, Evas_Object *obj, void 	*event_info);
	static void __back_cb(void *data, Evas_Object *obj, void *event_info);
	static void __rotate_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info);

	auto_fill_form_manager *m_manager;
	Evas_Object *m_main_layout;
	Evas_Object *m_genlist;
	Evas_Object *m_auto_fill_form_check;
	Evas_Object *m_plus_button;
	Evas_Object *m_ctx_popup_more_menu;
	Elm_Object_Item *m_navi_it;
	Elm_Genlist_Item_Class *m_multiline_1text_item_class;
	Elm_Genlist_Item_Class *m_dialogue_1text_item_class;
	Elm_Genlist_Item_Class *m_dialogue_group_title_item_class;
	Elm_Genlist_Item_Class *m_dialogue_bottom_item_class;

	genlist_callback_data m_description_top_callback_data;
	genlist_callback_data m_description_callback_data;
	genlist_callback_data m_sub_text_callback_data;
	genlist_callback_data m_body_pad_item_callback_data;
	genlist_callback_data m_auto_fill_forms_profile_title_item_callback_data;
	genlist_callback_data m_bottom_pad_item_callback_data;
	auto_profile_delete_view *m_delete_view;

	std::vector<profile_item_genlist_callback_data *> m_auto_fill_form_item_callback_data_list;
	list_view_item_clicked_popup *m_select_popup;
};

class auto_fill_form_compose_view : public common_view {
public:
	auto_fill_form_compose_view(auto_fill_form_item *item = NULL);
	~auto_fill_form_compose_view(void);

	void show(void);

private:
	typedef enum _menu_type
	{
		profile_composer_title_full_name = 0,
		profile_composer_title_company_name,
		profile_composer_title_address_line_1,
		profile_composer_title_address_line_2,
		profile_composer_title_city_town,
		profile_composer_title_county_region,
		profile_composer_title_post_code,
		profile_composer_title_country,
		profile_composer_title_phone,
		profile_composer_title_email,

		profile_composer_menu_end
	} menu_type;

	typedef struct _genlist_callback_data {
		menu_type type;
		void *user_data;
		Evas_Object *entry;
		Elm_Object_Item *it;
	} genlist_callback_data;

	Evas_Object *_create_main_layout(Evas_Object *parent);
	Evas_Object *_create_scroller(Evas_Object * parent);
	Evas_Object *_create_box(Evas_Object * parent);
	void _create_list_items(Evas_Object * parent);
	Evas_Object *_create_genlist(Evas_Object *parent);
	Eina_Bool _create_genlist_style_items(void);
	Eina_Bool _destroy_all_genlist_style_items(void);
	Eina_Bool is_entry_has_only_space(const char *);
	Eina_Bool _apply_entry_data(void);
	void _show_error_popup(int error_code);

	static void __genlist_realized_cb(void *data, Evas_Object *obj, void *event_info);
	static char *__text_get_cb(void *data, Evas_Object *obj, const char *part);
	static Evas_Object *__content_get_cb(void *data, Evas_Object *obj, const char *part);
	static void __done_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __naviframe_pop_cb(void *data, Elm_Object_Item *it);
	static void __entry_next_key_cb(void *data, Evas_Object *obj, void *eventInfo);
	static void __entry_clicked_cb(void *data, Evas_Object *obj, void *eventInfo);
	static void __naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info);
	static Eina_Bool __delay_naviframe_pop_idler_cb(void *data);
	static void __timer_popup_expired_cb(void *data, Evas_Object *obj, void *event_info);
	static void __editfield_clear_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
	static void __editfield_changed_cb(void *data, Evas_Object *obj, void *event_info);
	auto_fill_form_item *m_item_for_compose;

	Evas_Object *m_main_layout;
	Evas_Object *m_genlist;
	Evas_Object *m_scroller;
	Evas_Object *m_box;
	Evas_Object *m_contents_layout;
	Evas_Object *m_done_button;
	Evas_Object *m_cancel_button;
	Evas_Object *m_entry_full_name;
	Evas_Object *m_entry_company_name;
	Evas_Object *m_entry_address_line_1;
	Evas_Object *m_entry_address_line_2;
	Evas_Object *m_entry_city_town;
	Evas_Object *m_entry_county;
	Evas_Object *m_entry_post_code;
	Evas_Object *m_entry_country;
	Evas_Object *m_entry_phone;
	Evas_Object *m_entry_email;

	Elm_Object_Item *m_navi_it;
	Ecore_Idler *m_delay_naviframe_pop_idler;
	profile_edit_errorcode m_edit_errorcode;
	profile_save_errorcode m_save_errorcode;
	Elm_Genlist_Item_Class *m_edit_field_item_class;
	Elm_Entry_Filter_Limit_Size m_entry_limit_size;

	genlist_callback_data m_full_name_item_callback_data;
	genlist_callback_data m_company_name_item_callback_data;
	genlist_callback_data m_address_line_1_item_callback_data;
	genlist_callback_data m_address_line_2_item_callback_data;
	genlist_callback_data m_city_town_item_callback_data;
	genlist_callback_data m_country_item_callback_data;
	genlist_callback_data m_post_code_item_callback_data;
	genlist_callback_data m_county_region_item_callback_data;
	genlist_callback_data m_phone_item_callback_data;
	genlist_callback_data m_email_item_callback_data;
};

class list_view_item_clicked_popup : public common_view {
public:
	list_view_item_clicked_popup(auto_fill_form_item *item);
	~list_view_item_clicked_popup(void);

	Eina_Bool show(void);
private:
	typedef enum _menu_type
	{
		selected_item_edit = 0,
		selected_item_delete,
		selected_item_num
	} menu_type;

	Evas_Object *_create_genlist(Evas_Object *parent);

	static char *__genlist_text_get(void *data, Evas_Object *obj, const char *part);
	static void __popup_edit_cb(void *data, Evas_Object *obj, void *event_info);
	static void __popup_delete_cb(void *data, Evas_Object *obj, void *event_info);
	static void __delete_ok_confirm_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __genlist_realized_cb(void *data, Evas_Object *obj, void *event_info);

	Evas_Object *m_popup;
	auto_fill_form_item *m_item;
	Elm_Object_Item *m_popup_last_item;

protected:
};

#endif /* SETTINGS_BASIC_H */
