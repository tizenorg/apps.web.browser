/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "AutoFillFormManager.h"
#include "AutoFillFormComposeView.h"
#include "AutoFillFormListView.h"
#include "BrowserLogger.h"

#include <Ecore.h>
#include <Elementary.h>
#include <Evas.h>
#include <regex.h>

#define TRIM_SPACE " \t\n\v"

inline std::string _trim(std::string& s, const std::string& drop = TRIM_SPACE)
{
        std::string r = s.erase(s.find_last_not_of(drop) + 1);
        return r.erase(0, r.find_first_not_of(drop));
}

auto_fill_form_compose_view::auto_fill_form_compose_view(auto_fill_form_manager* manager, auto_fill_form_item *item)
:
	m_item_for_compose(NULL),
	m_manager(manager),
	m_main_layout(NULL),
	m_genlist(NULL),
	m_scroller(NULL),
	m_box(NULL),
	m_done_button(NULL),
	m_cancel_button(NULL),
	m_entry_full_name(NULL),
	m_entry_company_name(NULL),
	m_entry_address_line_1(NULL),
	m_entry_address_line_2(NULL),
	m_entry_city_town(NULL),
	m_entry_county(NULL),
	m_entry_post_code(NULL),
	m_entry_country(NULL),
	m_entry_phone(NULL),
	m_entry_email(NULL),
	m_edit_errorcode(update_error_none),
	m_save_errorcode(save_error_none),
	m_edit_field_item_class(NULL)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	if (item && item->get_item_compose_mode() == profile_edit)
		m_item_for_compose = item;
	else
		m_item_for_compose = new auto_fill_form_item(NULL);

	m_entry_limit_size.max_char_count = 0;
	m_entry_limit_size.max_byte_count = AUTO_FILL_FORM_ENTRY_MAX_COUNT;
        m_edjFilePath = EDJE_DIR;
        m_edjFilePath.append("SettingsUI/AutoFillMobileUI.edj");

}

auto_fill_form_compose_view::~auto_fill_form_compose_view(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	_destroy_all_genlist_style_items();

	if (m_item_for_compose && m_item_for_compose->get_item_compose_mode() == profile_create)
		delete m_item_for_compose;
	m_item_for_compose = NULL;

        if (m_genlist) {
            elm_genlist_clear(m_genlist);
            evas_object_del(m_genlist);
        }
        if (m_main_layout) {
            evas_object_hide(m_main_layout);
            evas_object_del(m_main_layout);
        }
}

void auto_fill_form_compose_view::show(Evas_Object* parent)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

        elm_theme_extension_add(nullptr, m_edjFilePath.c_str());

	m_main_layout = elm_layout_add(parent);
        if (!m_main_layout) {
                BROWSER_LOGE("_create_main_layout failed");
                return;
        }
        elm_layout_file_set(m_main_layout, m_edjFilePath.c_str(), "affcv-layout");
        evas_object_size_hint_weight_set(m_main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(m_main_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

        m_genlist = _create_genlist(m_main_layout);
        evas_object_show(m_genlist);
	elm_object_part_content_set(m_main_layout, "affcv_genlist", m_genlist);

        evas_object_show(m_main_layout);

	if (m_item_for_compose->get_item_compose_mode() == profile_edit)
                elm_object_part_text_set(m_main_layout, "title_text", "Auto Fill Edit");
	else
                elm_object_part_text_set(m_main_layout, "title_text", "Auto Fill Compose");

	m_cancel_button = elm_button_add(m_main_layout);
	if (!m_cancel_button) {
		BROWSER_LOGE("Failed to create m_cancel_button");
		return;
	}
	elm_object_style_set(m_cancel_button, "basic_button");
	evas_object_smart_callback_add(m_cancel_button, "clicked", __cancel_button_cb, this);
	elm_object_part_content_set(m_main_layout, "cancel_button", m_cancel_button);

	m_done_button = elm_button_add(m_main_layout);
	if (!m_done_button) {
		BROWSER_LOGE("Failed to create m_done_button");
		return;
	}
	elm_object_style_set(m_done_button, "basic_button");
	evas_object_smart_callback_add(m_done_button, "clicked", __done_button_cb, this);
	elm_object_part_content_set(m_main_layout, "done_button", m_done_button);
        elm_object_signal_emit(m_main_layout, "dim,done,button,signal", "");

	if (m_item_for_compose->get_item_compose_mode() == profile_create)
		elm_object_disabled_set(m_done_button, EINA_TRUE);
}

Evas_Object *auto_fill_form_compose_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}

	if (_create_genlist_style_items() == EINA_FALSE) {
		BROWSER_LOGE("_create_genlist_style_items failed");
		_destroy_all_genlist_style_items();
		return NULL;
	}

	m_full_name_item_callback_data.type = profile_composer_title_full_name;
	m_full_name_item_callback_data.user_data = this;
	m_full_name_item_callback_data.it = elm_genlist_item_append(genlist, m_edit_field_item_class,
						&m_full_name_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	m_company_name_item_callback_data.type = profile_composer_title_company_name;
	m_company_name_item_callback_data.user_data = this;
	m_company_name_item_callback_data.it = elm_genlist_item_append(genlist, m_edit_field_item_class,
						&m_company_name_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	m_address_line_1_item_callback_data.type = profile_composer_title_address_line_1;
	m_address_line_1_item_callback_data.user_data = this;
	m_address_line_1_item_callback_data.it = elm_genlist_item_append(genlist, m_edit_field_item_class,
						&m_address_line_1_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	m_address_line_2_item_callback_data.type = profile_composer_title_address_line_2;
	m_address_line_2_item_callback_data.user_data = this;
	m_address_line_2_item_callback_data.it = elm_genlist_item_append(genlist, m_edit_field_item_class,
						&m_address_line_2_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	m_city_town_item_callback_data.type = profile_composer_title_city_town;
	m_city_town_item_callback_data.user_data = this;
	m_city_town_item_callback_data.it = elm_genlist_item_append(genlist, m_edit_field_item_class,
						&m_city_town_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	m_country_item_callback_data.type = profile_composer_title_country;
	m_country_item_callback_data.user_data = this;
	m_country_item_callback_data.it = elm_genlist_item_append(genlist, m_edit_field_item_class,
						&m_country_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	m_post_code_item_callback_data.type = profile_composer_title_post_code;
	m_post_code_item_callback_data.user_data = this;
	m_post_code_item_callback_data.it = elm_genlist_item_append(genlist, m_edit_field_item_class,
						&m_post_code_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	m_phone_item_callback_data.type = profile_composer_title_phone;
	m_phone_item_callback_data.user_data = this;
	m_phone_item_callback_data.it = elm_genlist_item_append(genlist, m_edit_field_item_class,
						&m_phone_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	m_email_item_callback_data.type = profile_composer_title_email;
	m_email_item_callback_data.user_data = this;
	m_email_item_callback_data.it = elm_genlist_item_append(genlist, m_edit_field_item_class,
						&m_email_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	return genlist;
}

Eina_Bool auto_fill_form_compose_view::_create_genlist_style_items(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	Elm_Genlist_Item_Class *edit_field_item_class = elm_genlist_item_class_new();
	if (!edit_field_item_class) {
		BROWSER_LOGE("elm_genlist_item_class_new for edit_field_item_class failed");
		return EINA_FALSE;
	}
	edit_field_item_class->item_style = "affcv_item";
	edit_field_item_class->func.text_get = __text_get_cb;
	edit_field_item_class->func.content_get = __content_get_cb;
	m_edit_field_item_class = edit_field_item_class;

	return EINA_TRUE;
}

Eina_Bool auto_fill_form_compose_view::_destroy_all_genlist_style_items(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	if (m_edit_field_item_class)
		elm_genlist_item_class_free(m_edit_field_item_class);

	return EINA_TRUE;
}

Eina_Bool auto_fill_form_compose_view::is_entry_has_only_space(const char* field)
{
	Eina_Bool only_has_space = EINA_FALSE;
	unsigned int space_count = 0;
	unsigned int str_len = strlen(field);

	for (unsigned int i = 0 ; i < str_len ; i++) {
		if (field[i] == ' ')
			space_count++;
	}
	if (space_count == str_len)
		only_has_space = EINA_TRUE;

	return only_has_space;
}

Eina_Bool auto_fill_form_compose_view::_apply_entry_data(void)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);

	const char *full_name = elm_entry_entry_get(m_entry_full_name);

	if (!full_name)
		return EINA_FALSE;

	std::string full_name_str = std::string(full_name);
	full_name_str = _trim(full_name_str);
	full_name = full_name_str.c_str();

	if (full_name && strlen(full_name) && !is_entry_has_only_space(full_name))
		m_item_for_compose->set_name(full_name);
	else {
		elm_object_focus_set(m_cancel_button, EINA_TRUE); // Closing virtual keyboard by changing the focus*/
		return EINA_FALSE;
	}
	const char *company_name = elm_entry_entry_get(m_entry_company_name);
	m_item_for_compose->set_company(company_name);
	const char *primary_address = elm_entry_entry_get(m_entry_address_line_1);
	m_item_for_compose->set_primary_address(primary_address);
	const char *secondary_address = elm_entry_entry_get(m_entry_address_line_2);
	m_item_for_compose->set_secondary_address2(secondary_address);
	const char *city_town = elm_entry_entry_get(m_entry_city_town);
	m_item_for_compose->set_city_town(city_town);
	const char *county = elm_entry_entry_get(m_entry_county);
	m_item_for_compose->set_state_province(county);
	const char *post_code = elm_entry_entry_get(m_entry_post_code);
	m_item_for_compose->set_post_code(post_code);
	const char *country = elm_entry_entry_get(m_entry_country);
	m_item_for_compose->set_country(country);
	const char *phone = elm_entry_entry_get(m_entry_phone);
	m_item_for_compose->set_phone_number(phone);
	const char *email = elm_entry_entry_get(m_entry_email);
	m_item_for_compose->set_email_address(email);

	if (m_item_for_compose->get_item_compose_mode() == profile_edit) {

		m_edit_errorcode = m_item_for_compose->update_item();
		/* else case is needed */
	} else {
		m_save_errorcode = m_item_for_compose->save_item();
		if (m_save_errorcode != profile_create_failed && m_save_errorcode != duplicate_profile)
			m_manager->add_item_to_list(m_item_for_compose);
	}

	return EINA_TRUE;
}

char *auto_fill_form_compose_view::__text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	genlist_callback_data *cb_data = (genlist_callback_data *)data;
	auto_fill_form_compose_view::menu_type type = cb_data->type;

	if (!strcmp(part, "field_name")) {
		if (type == profile_composer_title_full_name)
			return strdup("Full Name" /*BR_STRING_AUTO_FILL_DATA_FULL_NAME*/);
		else if (type == profile_composer_title_company_name)
			return strdup("Company name" /*BR_STRING_AUTO_FILL_DATA_COMPANY_NAME*/);
		else if (type == profile_composer_title_address_line_1)
			return strdup("Address line 1" /*BR_STRING_AUTO_FILL_DATA_ADDRESS_LINE_1*/);
		else if (type == profile_composer_title_address_line_2)
			return strdup("Address line 2" /*BR_STRING_AUTO_FILL_DATA_ADDRESS_LINE_2*/);
		else if (type == profile_composer_title_city_town)
			return strdup("City town" /*BR_STRING_TOWN_AUTO_FILL_CITY_COUNTY*/);
		else if (type == profile_composer_title_county_region)
			return strdup("Region" /*BR_STRING_AUTO_FILL_DATA_COUNTRY_REGION*/);
		else if (type == profile_composer_title_post_code)
			return strdup("Post code"/*BR_STRING_AUTO_FILL_DATA_POST_CODE*/);
		else if (type == profile_composer_title_country)
			return strdup("Country" /*BR_STRING_AUTO_FILL_DATA_COUNTRY*/);
		else if (type == profile_composer_title_phone)
			return strdup("Phone" /*BR_STRING_AUTO_FILL_DATA_PHONE*/);
		else if (type == profile_composer_title_email)
			return strdup("Email" /*BR_STRING_AUTO_FILL_DATA_EMAIL*/);
	}

	return NULL;
}

Evas_Object *auto_fill_form_compose_view::__content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	genlist_callback_data *cb_data = (genlist_callback_data *)data;
	auto_fill_form_compose_view *view = (auto_fill_form_compose_view *)cb_data->user_data;
	auto_fill_form_compose_view::menu_type type = cb_data->type;

	if (!strcmp(part, "entry_swallow")) {

                Evas_Object *editfield = elm_layout_add(obj);
                elm_layout_file_set(editfield, view->m_edjFilePath.c_str(), "edit-field");
                evas_object_size_hint_align_set(editfield, EVAS_HINT_FILL, 0.0);
                evas_object_size_hint_weight_set(editfield, EVAS_HINT_EXPAND, 0.0);

		Evas_Object *entry = elm_entry_add(editfield);
                elm_object_style_set(entry, "entry_style");
		elm_entry_single_line_set(entry, EINA_TRUE);
		elm_entry_scrollable_set(entry, EINA_TRUE);
		elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);
                elm_entry_text_style_user_push(entry, "DEFAULT='font_size=30 color=#404040 ellipsis=1'");

#if defined(HW_MORE_BACK_KEY)
		eext_entry_selection_back_event_allow_set(entry, EINA_TRUE);
#endif
		evas_object_smart_callback_add(entry, "preedit,changed", __entry_changed_cb, cb_data);
		evas_object_smart_callback_add(entry, "changed", __entry_changed_cb, cb_data);
                evas_object_smart_callback_add(entry, "changed", __editfield_changed_cb, editfield);
		evas_object_smart_callback_add(entry, "activated", __entry_next_key_cb, cb_data);
		evas_object_smart_callback_add(entry, "clicked", __entry_clicked_cb, cb_data);
		elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_NEXT);
		elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &(view->m_entry_limit_size));

		if (type == profile_composer_title_full_name) {
			if (view->m_item_for_compose->get_name() && strlen(view->m_item_for_compose->get_name()))
				elm_object_part_text_set(entry, "elm.text", view->m_item_for_compose->get_name());
			view->m_entry_full_name = cb_data->entry = entry;
                        elm_object_focus_set(entry, true);
                        elm_entry_cursor_end_set(entry);
		} else if (type == profile_composer_title_company_name) {
			if (view->m_item_for_compose->get_company() && strlen(view->m_item_for_compose->get_company()))
				elm_object_part_text_set(entry, "elm.text", view->m_item_for_compose->get_company());
			view->m_entry_company_name = cb_data->entry = entry;
		} else if (type == profile_composer_title_address_line_1) {
			if (view->m_item_for_compose->get_primary_address() && strlen(view->m_item_for_compose->get_primary_address()))
				elm_object_part_text_set(entry, "elm.text", view->m_item_for_compose->get_primary_address());
			view->m_entry_address_line_1 = cb_data->entry = entry;
		} else if (type == profile_composer_title_address_line_2) {
			if (view->m_item_for_compose->get_secondary_address2() && strlen(view->m_item_for_compose->get_secondary_address2()))
				elm_object_part_text_set(entry, "elm.text", view->m_item_for_compose->get_secondary_address2());
			view->m_entry_address_line_2 = cb_data->entry = entry;
		} else if (type == profile_composer_title_city_town) {
			if (view->m_item_for_compose->get_city_town() && strlen(view->m_item_for_compose->get_city_town()))
				elm_object_part_text_set(entry, "elm.text", view->m_item_for_compose->get_city_town());
			view->m_entry_city_town = cb_data->entry = entry;
		} else if (type == profile_composer_title_county_region) {
			if (view->m_item_for_compose->get_state_province() && strlen(view->m_item_for_compose->get_state_province()))
				elm_object_part_text_set(entry, "elm.text", view->m_item_for_compose->get_state_province());
			view->m_entry_county = cb_data->entry = entry;
		} else if (type == profile_composer_title_post_code) {
			Elm_Entry_Filter_Limit_Size m_entry_limit_size;
			Elm_Entry_Filter_Accept_Set m_entry_accept_set;
			m_entry_limit_size.max_char_count = 10;
			m_entry_accept_set.accepted = "0123456789";
			m_entry_accept_set.rejected = NULL;
			elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &m_entry_limit_size);
			elm_entry_markup_filter_append(entry, elm_entry_filter_accept_set, &m_entry_accept_set);

			if (view->m_item_for_compose->get_post_code() && strlen(view->m_item_for_compose->get_post_code()))
				elm_object_part_text_set(entry, "elm.text", view->m_item_for_compose->get_post_code());
			elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_NUMBERONLY);
			elm_entry_prediction_allow_set(entry, EINA_FALSE);
			view->m_entry_post_code = cb_data->entry = entry;
		} else if (type == profile_composer_title_country) {
			if (view->m_item_for_compose->get_country() && strlen(view->m_item_for_compose->get_country()))
				elm_object_part_text_set(entry, "elm.text", view->m_item_for_compose->get_country());
			view->m_entry_country = cb_data->entry = entry;
		} else if (type == profile_composer_title_phone) {
			if (view->m_item_for_compose->get_phone_number() && strlen(view->m_item_for_compose->get_phone_number()))
				elm_object_part_text_set(entry, "elm.text", view->m_item_for_compose->get_phone_number());
			Elm_Entry_Filter_Accept_Set entry_accept_set;
			entry_accept_set.accepted = PHONE_FIELD_VALID_ENTRIES;
			entry_accept_set.rejected = NULL;
			elm_entry_markup_filter_append(entry, elm_entry_filter_accept_set, &entry_accept_set);
			elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_PHONENUMBER);
			elm_entry_prediction_allow_set(entry, EINA_FALSE);
			view->m_entry_phone = cb_data->entry = entry;
		} else if (type == profile_composer_title_email) {
			if (view->m_item_for_compose->get_email_address() && strlen(view->m_item_for_compose->get_email_address()))
				elm_object_part_text_set(entry, "elm.text", view->m_item_for_compose->get_email_address());
			elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
			evas_object_smart_callback_add(entry, "activated", __done_button_cb, view);
			elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_EMAIL);
			elm_entry_prediction_allow_set(entry, EINA_FALSE);
			view->m_entry_email = cb_data->entry = entry;
		}

                elm_object_part_content_set(editfield, "editfield_entry", entry);

                Evas_Object *button = elm_button_add(editfield);
                elm_object_style_set(button, "basic_button");
                evas_object_smart_callback_add(button, "clicked", __entry_clear_button_clicked_cb, entry);
                elm_object_part_content_set(editfield, "entry_clear_button", button);

                if (!elm_entry_is_empty(entry)) {
                        BROWSER_LOGE("entry is empty");
                        elm_object_signal_emit(editfield, "show,clear,button,signal", "");
                }

                return editfield;
	}

	return NULL;
}

void auto_fill_form_compose_view::__editfield_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
        BROWSER_LOGD("");
        Evas_Object *editfield = (Evas_Object *)data;

        if (!elm_entry_is_empty(obj) && elm_object_focus_get(obj))
                elm_object_signal_emit(editfield, "show,clear,button,signal", "");
        else
                elm_object_signal_emit(editfield, "hide,clear,button,signal", "");
}

void auto_fill_form_compose_view::__done_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_compose_view *view = (auto_fill_form_compose_view *)data;

	if (elm_entry_is_empty(view->m_entry_full_name)) {
		elm_object_focus_set(view->m_cancel_button, EINA_TRUE); // Closing virtual keyboard by changing the focus
		return;
	}

	if (view->_apply_entry_data() == EINA_FALSE)
		return;

	elm_object_focus_set(view->m_cancel_button, EINA_TRUE);
        evas_object_hide(view->m_main_layout);
	view->m_manager->refresh_list_view();
}

void auto_fill_form_compose_view::__cancel_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");

        auto_fill_form_compose_view *view = (auto_fill_form_compose_view *)data;
        view->hide();
}

void auto_fill_form_compose_view::__entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");

	genlist_callback_data *cb_data = (genlist_callback_data *)data;
	auto_fill_form_compose_view *view = (auto_fill_form_compose_view *)cb_data->user_data;
	RET_MSG_IF(!view, "view is NULL");
	const char* text = elm_entry_entry_get(obj);
	//view->show_notification(text, obj, AUTO_FILL_FORM_ENTRY_MAX_COUNT);
        if (text && strlen(text) > 0) {
            elm_object_signal_emit(cb_data->it, "show,clear,button,signal", "");
        }
        else {
            elm_object_signal_emit(cb_data->it, "hide,clear,button,signal", "");
        }

	if (!elm_entry_is_empty(view->m_entry_full_name)) {
                elm_object_signal_emit(view->m_main_layout, "show,done,button,signal", "");
		elm_object_disabled_set(view->m_done_button, EINA_FALSE);
	} else {
                elm_object_signal_emit(view->m_main_layout, "dim,done,button,signal", "");
		elm_object_disabled_set(view->m_done_button, EINA_TRUE);
	}
	return;
}

void auto_fill_form_compose_view::__entry_clicked_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Elm_Object_Item *item = callback_data->it;
	Evas_Object *entry = elm_object_item_part_content_get(item, "entry_swallow");
	elm_object_focus_set(entry, EINA_TRUE);
	//elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_IN);
}

void auto_fill_form_compose_view::__entry_clear_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	Evas_Object *entry = (Evas_Object *)data;

	elm_entry_entry_set(entry, "");
}

void auto_fill_form_compose_view::__entry_next_key_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("-----------[%s : %d]---------", __PRETTY_FUNCTION__, __LINE__);
	RET_MSG_IF(!data, "data is NULL");

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	auto_fill_form_compose_view *view_this = (auto_fill_form_compose_view *)callback_data->user_data;
	auto_fill_form_compose_view::menu_type type = callback_data->type;
	Evas_Object *entry = NULL;
	Elm_Object_Item *item = NULL;

	if (type == profile_composer_title_full_name) {
		entry = elm_object_item_part_content_get(
					view_this->m_company_name_item_callback_data.it, "entry_swallow");
		item = view_this->m_company_name_item_callback_data.it;
	} else if (type == profile_composer_title_company_name) {
		entry = elm_object_item_part_content_get(
					view_this->m_address_line_1_item_callback_data.it, "entry_swallow");
		item = view_this->m_address_line_1_item_callback_data.it;
	} else if (type == profile_composer_title_address_line_1) {
		entry = elm_object_item_part_content_get(
					view_this->m_address_line_2_item_callback_data.it, "entry_swallow");
		item = view_this->m_address_line_2_item_callback_data.it;
	} else if (type == profile_composer_title_address_line_2) {
		entry = elm_object_item_part_content_get(
					view_this->m_city_town_item_callback_data.it, "entry_swallow");
		item = view_this->m_city_town_item_callback_data.it;
	} else if (type == profile_composer_title_city_town) {
		entry = elm_object_item_part_content_get(
					view_this->m_country_item_callback_data.it, "entry_swallow");
		item = view_this->m_country_item_callback_data.it;
	} else if (type == profile_composer_title_country) {
		entry = elm_object_item_part_content_get(
					view_this->m_post_code_item_callback_data.it, "entry_swallow");
		item = view_this->m_post_code_item_callback_data.it;
	} else if (type == profile_composer_title_post_code) {
		entry = elm_object_item_part_content_get(
					view_this->m_phone_item_callback_data.it, "entry_swallow");
		item = view_this->m_phone_item_callback_data.it;
	}
	else if (type == profile_composer_title_county_region) {
		entry = elm_object_item_part_content_get(
					view_this->m_post_code_item_callback_data.it, "entry_swallow");
	}
	else if (type == profile_composer_title_phone) {
		entry = elm_object_item_part_content_get(
					view_this->m_email_item_callback_data.it, "entry_swallow");
		item = view_this->m_email_item_callback_data.it;
	} else if (type == profile_composer_title_email) {
		BROWSER_LOGD("It's last item to go");
		return;
	}
	elm_object_focus_set(entry, EINA_TRUE);
	elm_entry_cursor_end_set(entry);
	//elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_IN);
}

void auto_fill_form_compose_view::hide()
{
        evas_object_hide(m_main_layout);
}
