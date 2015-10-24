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

#include "settings-basic.h"

#include <efl_extension.h>
#include <regex.h>

#include "browser-dlog.h"
#include "browser-object.h"
#include "browser-string.h"
#include "browser-view.h"
#include "settings.h"
#include "webview.h"
#include "platform-service.h"

#define common_edj_path browser_edj_dir"/browser-common.edj"
#define settings_edj_path browser_edj_dir"/settings.edj"
#define USER_HOMEPAGE_ENTRY_MAX_COUNT 4096

#define PHONE_FIELD_VALID_ENTRIES "0123456789*#()/N,.;+ "
#define HOMEPAGE_URLEXPR "((https?|ftp|gopher|telnet|file|notes|ms-help):((//)|(\\\\))[\\w\\d:#@%/;$()~_?+-=\\.&]+)"

typedef struct _settings_basic_menu_item {
	settings_basic_menu_index menu_index;
	Eina_Bool is_title;
	const char *item_style;
} settings_basic_menu_item;

settings_basic_menu_item settings_basic_menus[] = {
	{br_settings_sub_basic_menu_set_homepage_start, EINA_TRUE, "separator"},
	{br_settings_sub_basic_menu_set_homepage_current_page, EINA_FALSE, "2line.top"},
	{br_settings_sub_basic_menu_set_homepage_set_homepage_url, EINA_FALSE, "1line"},
};

#define TRIM_SPACE " \t\n\v"

inline std::string _trim(std::string& s, const std::string& drop = TRIM_SPACE)
{
	std::string r = s.erase(s.find_last_not_of(drop) + 1);
	return r.erase(0, r.find_first_not_of(drop));
}

set_homepage_view::set_homepage_view(void)
:
	m_genlist(NULL),
	m_homepage_radio_group(NULL),
	m_popup(NULL),
	m_editfield_entry(NULL),
	m_ok_button(NULL),
	m_navi_item(NULL),
	m_is_closing(EINA_FALSE),
	m_set_init_homepage(EINA_TRUE)
{
	BROWSER_LOGD("");
}

set_homepage_view::~set_homepage_view(void)
{
	BROWSER_LOGD("");

	_destroy_genlist_styles();
	_destroy_genlist_callback_datas();
	m_browser->get_settings()->realize_genlist();
}

Eina_Bool set_homepage_view::show(void)
{
	BROWSER_LOGD("");
	if (_create_main_layout(m_naviframe) == EINA_FALSE) {
		BROWSER_LOGE("_create_main_layout failed");
		return EINA_FALSE;
	}
	m_navi_item = elm_naviframe_item_push(m_naviframe, "IDS_BR_MBODY_SET_HOMEPAGE", NULL, NULL, m_genlist, NULL);
	elm_object_item_domain_text_translatable_set(m_navi_item, BROWSER_DOMAIN_NAME, EINA_TRUE);

	eext_object_event_callback_add(m_genlist, EEXT_CALLBACK_BACK, __set_homepage_cancel_button_cb, this);
	Evas_Object *btn_cancel = elm_button_add(m_naviframe);
	if (!btn_cancel) return EINA_FALSE;
	elm_object_style_set(btn_cancel, "naviframe/title_cancel");
	evas_object_smart_callback_add(btn_cancel, "clicked", __set_homepage_cancel_button_cb, this);
	elm_object_item_part_content_set(m_navi_item, "title_left_btn", btn_cancel);

	Evas_Object *btn_save = elm_button_add(m_naviframe);
	if (!btn_save) return EINA_FALSE;
	elm_object_style_set(btn_save, "naviframe/title_done");
	evas_object_smart_callback_add(btn_save, "clicked", __set_homepage_ok_button_cb, this);
	elm_object_item_part_content_set(m_navi_item, "title_right_btn", btn_save);

	elm_naviframe_item_pop_cb_set(m_navi_item, __naviframe_pop_cb, this);
	evas_object_smart_callback_add(m_naviframe, "transition,finished", __naviframe_pop_finished_cb, this);

	return EINA_TRUE;
}

void set_homepage_view::destroy_homepage_edit_popup_show(void)
{
	BROWSER_LOGD("");

	if (m_popup != NULL)
		evas_object_del(m_popup);
	m_popup = NULL;
}

Eina_Bool set_homepage_view::_is_valid_homepage (const char *uri)
{
	BROWSER_SECURE_LOGD("uri=[%s]", uri);

	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	if (!strcmp(uri, blank_page))
		return EINA_TRUE;

	regex_t regex;
	if (regcomp(&regex, HOMEPAGE_URLEXPR, REG_EXTENDED | REG_ICASE) != 0) {
		BROWSER_LOGD("regcomp failed");
		return EINA_FALSE;
	}

	if (regexec(&regex, uri, 0, NULL, REG_NOTEOL) == 0) {
		BROWSER_LOGD("url expression");
		regfree(&regex);
		if (m_browser->get_browser_view()->is_valid_uri(uri))
			return EINA_TRUE;
		else
			return EINA_FALSE;
	}

	regfree(&regex);
	if (m_browser->get_browser_view()->is_valid_uri(uri))
		return EINA_TRUE;

	return EINA_FALSE;
}

Eina_Bool set_homepage_view::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");

	m_genlist = elm_genlist_add(parent);
	if (!m_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return EINA_FALSE;
	}
	elm_genlist_realization_mode_set(m_genlist, EINA_TRUE);

	_destroy_genlist_styles();
	_destroy_genlist_callback_datas();

	evas_object_smart_callback_add(m_genlist, "realized", __genlist_realized_cb, this);
	evas_object_smart_callback_add(m_genlist, "language,changed", __genlist_lang_changed, NULL);

	Eina_Bool has_error = EINA_FALSE;
	for (unsigned short i = br_settings_sub_basic_menu_set_homepage_start; i < br_settings_sub_basic_menu_set_homepage_end; i++) {
		Elm_Genlist_Item_Class *item_class = _create_genlist_item_class((settings_basic_menu_index)i);
		if (item_class) {
			settings_basic_genlist_callback_data *cb_data = new(settings_basic_genlist_callback_data);
			if (!cb_data) {
				has_error = EINA_TRUE;
				break;
			}
			memset(cb_data, 0x00, sizeof(settings_basic_genlist_callback_data));

			cb_data->menu_index = (settings_basic_menu_index)i;
			cb_data->user_data = this;

			if (settings_basic_menus[i].is_title == EINA_TRUE) {
				cb_data->it = elm_genlist_item_append(m_genlist, item_class, (void *)cb_data, NULL,
													ELM_GENLIST_ITEM_NONE, NULL, NULL);
				elm_genlist_item_select_mode_set(cb_data->it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			} else {
				cb_data->it = elm_genlist_item_append(m_genlist, item_class, (void *)cb_data, NULL,
													ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, (void *)cb_data);
			}
			m_genlist_callback_data_list.push_back(cb_data);
		} else {
			has_error= EINA_TRUE;
			break;
		}
	}

	if (has_error == EINA_TRUE) {
		_destroy_genlist_styles();
		_destroy_genlist_callback_datas();
		return EINA_FALSE;
	}

	m_homepage_radio_group = elm_radio_add(m_genlist);
	if (!m_homepage_radio_group) {
		BROWSER_LOGE("elm_radio_add failed.");
		_destroy_genlist_styles();
		_destroy_genlist_callback_datas();
		return EINA_FALSE;
	}
	elm_radio_state_value_set(m_homepage_radio_group, -1);

	return EINA_TRUE;
}

void set_homepage_view::_set_homepage_type_in_genlist(homepage_type type)
{
	BROWSER_LOGD("type[%d]", type);

	RET_MSG_IF(type < HOMEPAGE_TYPE_DEFAULT_PAGE || type > HOMEPAGE_TYPE_USER_HOMEPAGE, "type is out of allowed");
	elm_radio_value_set(m_homepage_radio_group, type);
}

void set_homepage_view::_back_to_previous_view(void)
{
	BROWSER_LOGD("");

	if (elm_naviframe_bottom_item_get(m_naviframe) != elm_naviframe_top_item_get(m_naviframe))
		elm_naviframe_item_pop(m_naviframe);

	if (m_browser->get_settings()->is_shown())
		m_browser->get_settings()->realize_genlist();

	Evas_Object *genlist = m_browser->get_settings()->get_genlist();
	Elm_Object_Item* it = elm_genlist_first_item_get(genlist);
	int first = elm_genlist_item_index_get(elm_genlist_first_item_get(genlist));
	int last = elm_genlist_item_index_get(elm_genlist_last_item_get(genlist));

	for(int index=first; index<=last; index++) {
		elm_genlist_item_update(it);
		it = elm_genlist_item_next_get(it);
	}
}

Elm_Genlist_Item_Class *set_homepage_view::_create_genlist_item_class(settings_basic_menu_index index)
{
	BROWSER_LOGD("settings_basic_menu_index[%d]", index);

	if ((index < br_settings_sub_basic_menu_set_homepage_start) || (index >= br_settings_sub_basic_menu_end)) {
		BROWSER_LOGE("index is out of allowed");
		return NULL;
	}

	if (!settings_basic_menus[index].item_style) {
		BROWSER_LOGE("unable to get settings_basic_menus[i].item_style", index);
		return NULL;
	}

	Elm_Genlist_Item_Class *item_class = NULL;
	/* check duplicated item style */
	for (unsigned short i = 0; i < m_genlist_item_class_list.size(); i++) {
		if (strlen(m_genlist_item_class_list[i]->item_style) == strlen(settings_basic_menus[index].item_style)
			&& !strncmp(m_genlist_item_class_list[i]->item_style, settings_basic_menus[index].item_style, strlen(m_genlist_item_class_list[i]->item_style))) {
			BROWSER_LOGE("[%s] style is already created", settings_basic_menus[index].item_style);
			item_class = m_genlist_item_class_list[i];
			break;
		}
	}

	if (!item_class) {
		item_class = elm_genlist_item_class_new();
		if (!item_class) {
			BROWSER_LOGE("elm_genlist_item_class_new : item_class failed");
			return NULL;
		}
		item_class->item_style = settings_basic_menus[index].item_style;
		item_class->func.text_get = __genlist_text_get;
		item_class->func.content_get = __genlist_contents_get;
		item_class->func.state_get = NULL;
		item_class->func.del = NULL;
		m_genlist_item_class_list.push_back(item_class);
	}

	return item_class;
}

void set_homepage_view::_destroy_genlist_styles(void)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_genlist_item_class_list.size(); i++) {
		if (m_genlist_item_class_list[i])
			elm_genlist_item_class_free(m_genlist_item_class_list[i]);
		m_genlist_item_class_list[i] = NULL;
	}
	m_genlist_item_class_list.clear();
}

void set_homepage_view::_destroy_genlist_callback_datas(void)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_genlist_callback_data_list.size(); i++) {
		free(m_genlist_callback_data_list[i]);
		m_genlist_callback_data_list[i] = NULL;
	}
	m_genlist_callback_data_list.clear();
}

const char *set_homepage_view::_get_current_page_uri_to_show(void)
{
	const char *current_page_uri = m_browser->get_browser_view()->get_current_webview()->get_uri();
	return current_page_uri;
}

void set_homepage_view::_change_homepage_type_current_to_other(void)
{
	webview *wv = m_browser->get_browser_view()->get_current_webview();
	RET_MSG_IF(!wv, "current webview is NULL");

	char *saved_current_page_uri = NULL;
	const char *web_page_current_uri = NULL;

	saved_current_page_uri = m_preference->get_current_page_uri();
	RET_MSG_IF(saved_current_page_uri == NULL, "unable to compare strings : saved_current_page_uri is NULL");

	web_page_current_uri = wv->get_uri();
	if (web_page_current_uri == NULL) {
		BROWSER_LOGI("web_page_current_uri is NULL");
		free(saved_current_page_uri);
		return;
	}

	unsigned int saved_current_page_uri_len = strlen(saved_current_page_uri);
	unsigned int web_page_current_uri_len = strlen(web_page_current_uri);

	if (strncmp(saved_current_page_uri, web_page_current_uri, saved_current_page_uri_len > web_page_current_uri_len ? saved_current_page_uri_len : web_page_current_uri_len)) {
		/* saved uri of "Current page" and uri of current weboage are different. */
		/* Make current homepage setting as otter */
		m_preference->set_homepage_type(HOMEPAGE_TYPE_USER_HOMEPAGE);
		m_preference->set_user_homagepage(saved_current_page_uri);
	}

	free(saved_current_page_uri);
	return;
}

Eina_Bool set_homepage_view::_user_homepage_edit_popup_show(char *homepage)
{
	BROWSER_LOGD("");
	m_popup = brui_popup_add(m_naviframe);
	if (!m_popup) {
		BROWSER_LOGE("m_popup is NULL");
		if (homepage) free(homepage);
		homepage = NULL;
		return EINA_FALSE;
	}

	evas_object_freeze_events_set(m_genlist, EINA_TRUE);
	elm_object_domain_translatable_part_text_set(m_popup, "title,text", BROWSER_DOMAIN_NAME, "IDS_BR_HEADER_SET_HOMEPAGE_ABB");
	eext_object_event_callback_add(m_popup, EEXT_CALLBACK_BACK, __edit_popup_cancel_button_cb, this);
	evas_object_size_hint_weight_set(m_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/* for margins */
	Evas_Object *layout = elm_layout_add(m_popup);
	elm_layout_file_set(layout, browser_edj_dir"/browser-popup-lite.edj", "popup_input_text");
	elm_layout_theme_set(layout, "layout", "editfield", "singleline");
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, 0.0);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(m_popup, layout);

	m_entry_limit_size.max_char_count = 0;
	m_entry_limit_size.max_byte_count = USER_HOMEPAGE_ENTRY_MAX_COUNT;

	m_editfield_entry = elm_entry_add(layout);
	if (!m_editfield_entry) {
		BROWSER_LOGE("a_editfield_add is failed");
		if (homepage) free(homepage);
		homepage = NULL;
		return EINA_FALSE;
	}

	elm_entry_markup_filter_append(m_editfield_entry, elm_entry_filter_limit_size, &m_entry_limit_size);
	evas_object_size_hint_weight_set(m_editfield_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_editfield_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	eext_entry_selection_back_event_allow_set(m_editfield_entry, EINA_TRUE);
	elm_entry_editable_set(m_editfield_entry, EINA_TRUE);
	elm_entry_cnp_mode_set(m_editfield_entry, ELM_CNP_MODE_PLAINTEXT);
	elm_entry_scrollable_set(m_editfield_entry, EINA_TRUE);
	elm_entry_autocapital_type_set(m_editfield_entry, ELM_AUTOCAPITAL_TYPE_NONE);
	elm_entry_prediction_allow_set(m_editfield_entry, EINA_FALSE);
	elm_entry_single_line_set(m_editfield_entry, EINA_TRUE);

#if defined(HW_MORE_BACK_KEY)
	eext_entry_selection_back_event_allow_set(m_editfield_entry, EINA_TRUE);
#endif
	if (m_input_homepage_uri_str.length()) {
		std::string trim_homepage_uri = _trim(m_input_homepage_uri_str);
		const char *input_homepage_uri = trim_homepage_uri.c_str();
		char *homepage_uri = elm_entry_markup_to_utf8(input_homepage_uri);
		elm_object_text_set(m_editfield_entry, homepage_uri);
	} else if (homepage && strlen(homepage)) {
		char *markup = elm_entry_utf8_to_markup(homepage);
		elm_object_text_set(m_editfield_entry, markup);
		free(markup);
	} else
		elm_object_text_set(m_editfield_entry, blank_page);

	free(homepage);
	homepage = NULL;

	elm_entry_input_panel_layout_set(m_editfield_entry, ELM_INPUT_PANEL_LAYOUT_URL);
	elm_entry_input_panel_return_key_type_set(m_editfield_entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
	elm_entry_context_menu_clear(m_editfield_entry);
	elm_entry_context_menu_item_add(m_editfield_entry, "Clipboard", NULL, ELM_ICON_STANDARD, NULL, NULL);
	elm_object_part_content_set(layout, "elm.swallow.content" , m_editfield_entry);
	elm_entry_cursor_end_set(m_editfield_entry);

	Evas_Object *button = elm_button_add(layout);
	elm_object_style_set(button, "editfield_clear");
	evas_object_smart_callback_add(button, "clicked", __edit_popup_clear_button_cb, m_editfield_entry);
	elm_object_part_content_set(layout, "elm.swallow.button", button);


	if (!m_browser->is_keyboard_active())
		evas_object_smart_callback_add(m_editfield_entry, "activated", __edit_popup_entry_activated_cb, this);

	evas_object_smart_callback_add(m_editfield_entry, "changed", __edit_popup_editfield_changed_cb, layout);
	evas_object_smart_callback_add(m_editfield_entry, "changed", __edit_popup_entry_changed_cb, this);
	evas_object_smart_callback_add(m_editfield_entry, "preedit,changed", __edit_popup_entry_changed_cb, this);

	Evas_Object *cancel_button = elm_button_add(m_popup);
	if (!cancel_button) {
		BROWSER_LOGE("elm_button_add for cancel_button is failed");
		return EINA_FALSE;
	}
	elm_object_domain_translatable_part_text_set(cancel_button, NULL, BROWSER_DOMAIN_NAME, "IDS_BR_SK_CANCEL");
	elm_object_style_set(cancel_button, "popup");
	elm_object_part_content_set(m_popup, "button1", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __edit_popup_cancel_button_cb, this);

	m_ok_button = elm_button_add(m_popup);
	if (!m_ok_button) {
		BROWSER_LOGE("elm_button_add for save_button is failed");
		return EINA_FALSE;
	}
	elm_object_domain_translatable_part_text_set(m_ok_button, NULL, BROWSER_DOMAIN_NAME, "IDS_BR_BUTTON_SET");
	elm_object_style_set(m_ok_button, "popup");
	elm_object_part_content_set(m_popup, "button2", m_ok_button);
	evas_object_smart_callback_add(m_ok_button, "clicked", __edit_popup_ok_button_cb, this);
	evas_object_show(m_popup);

	return EINA_TRUE;
}

char *set_homepage_view::__genlist_text_get(void *data, Evas_Object *obj, const char *part)
{
	RETV_MSG_IF(!data, NULL, "data is NULL");

	settings_basic_genlist_callback_data *cb_data = (settings_basic_genlist_callback_data *)data;
	settings_basic_menu_index menu_index = cb_data->menu_index;
	set_homepage_view *shv = (set_homepage_view *)cb_data->user_data;

	char *menu_text = NULL;
	BROWSER_LOGD("menu_index[%d]", menu_index);
	switch (menu_index) {
		case br_settings_sub_basic_menu_set_homepage_current_page:
			if (!strcmp(part, "elm.text.main.left.top"))
				menu_text = strdup(BR_STRING_SETTINGS_CURRENT_PAGE);
			else if (!strcmp(part, "elm.text.sub.left.bottom")) {
				webview *wv = m_browser->get_browser_view()->get_current_webview();
				if (wv != NULL)
					menu_text = strdup(shv->_get_current_page_uri_to_show());
				else
					menu_text = strdup(blank_page);
			}
		break;

		case br_settings_sub_basic_menu_set_homepage_set_homepage_url:
			if (!strcmp(part, "elm.text.main.left"))
				menu_text = strdup(BR_STRING_SETTINGS_OTHER);
			else if (!strcmp(part, "elm.text.1"))
				menu_text = strdup(BR_STRING_SETTINGS_OTHER);
			else if (!strcmp(part, "elm.text.2")) {
				char *user_homepage_uri = m_preference->get_user_homagepage();
				if (user_homepage_uri) {
					if (strlen(user_homepage_uri) > 0)
						menu_text = strdup(user_homepage_uri);
					else
						menu_text = strdup(blank_page);
					free(user_homepage_uri);
				} else
					menu_text = strdup(blank_page);
			}
		break;
		default:
			BROWSER_LOGD("menu_index[%d] is out of allowed", menu_index);
		break;
	}

	return menu_text;
}

Evas_Object *set_homepage_view::__genlist_contents_get(void *data, Evas_Object *obj, const char *part)
{
	/* for debug */
	BROWSER_LOGD("part[%s]", part);

	settings_basic_genlist_callback_data *cb_data = (settings_basic_genlist_callback_data *)data;
	settings_basic_menu_index menu_index = cb_data->menu_index;
	set_homepage_view *shv = (set_homepage_view *)cb_data->user_data;

	homepage_type homepage;
	if (shv->m_set_init_homepage == EINA_TRUE)
		homepage = m_preference->get_homepage_type();
	else
		homepage =(homepage_type) elm_radio_value_get(shv->m_homepage_radio_group);
	Evas_Object *content = NULL;
	switch(menu_index) {
		case br_settings_sub_basic_menu_set_homepage_current_page:
			if (!strcmp(part, "elm.icon.right")) {
				Evas_Object *radio_button = elm_radio_add(obj);
				if (radio_button) {
					elm_radio_state_value_set(radio_button, HOMEPAGE_TYPE_CURRENT_PAGE);
					shv->m_previous_radio_state = HOMEPAGE_TYPE_CURRENT_PAGE;
					elm_radio_group_add(radio_button, shv->m_homepage_radio_group);
					evas_object_propagate_events_set(radio_button, EINA_FALSE);
					evas_object_smart_callback_add(radio_button, "changed", __homepage_radio_item_clicked_cb, cb_data);
					elm_access_object_unregister(radio_button);
				}
				content = radio_button;
			}
		break;

		case br_settings_sub_basic_menu_set_homepage_set_homepage_url:
			if (!strcmp(part, "elm.icon.right")) {
				Evas_Object *radio_button = elm_radio_add(obj);
				if (radio_button) {
					elm_radio_state_value_set(radio_button, HOMEPAGE_TYPE_USER_HOMEPAGE);
					elm_radio_group_add(radio_button, shv->m_homepage_radio_group);
					evas_object_propagate_events_set(radio_button, EINA_FALSE);
					evas_object_smart_callback_add(radio_button, "changed", __homepage_radio_item_clicked_cb, cb_data);
				}
				content = radio_button;
			}
		break;

		default:
			BROWSER_LOGD("menu_index[%d] is out of allowed", menu_index);
			return content;
	}

	if (homepage != elm_radio_value_get(shv->m_homepage_radio_group))
		elm_radio_value_set(shv->m_homepage_radio_group, homepage);

	/* change homepage setting from current page to other page if current webpage is different from setting's */
	if (m_preference->get_homepage_type() == HOMEPAGE_TYPE_CURRENT_PAGE)
		shv->_change_homepage_type_current_to_other();

	return content;
}

void set_homepage_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	settings_basic_genlist_callback_data *cb_data = (settings_basic_genlist_callback_data *)data;
	settings_basic_menu_index menu_index = cb_data->menu_index;
	set_homepage_view *shv = (set_homepage_view *)cb_data->user_data;
	shv->m_set_init_homepage = EINA_FALSE;

	Elm_Object_Item *item = (Elm_Object_Item *)cb_data->it;
	elm_genlist_item_selected_set(item, EINA_FALSE);
	elm_genlist_item_update(item);

	BROWSER_LOGD("selected menu_index[%d]", menu_index);

	switch(menu_index) {
		case br_settings_sub_basic_menu_set_homepage_current_page:
			shv->_set_homepage_type_in_genlist(HOMEPAGE_TYPE_CURRENT_PAGE);
			shv->m_previous_radio_state = HOMEPAGE_TYPE_CURRENT_PAGE;
		break;

		case br_settings_sub_basic_menu_set_homepage_set_homepage_url:
			evas_object_freeze_events_set(shv->m_genlist, EINA_TRUE);
			m_browser->get_settings()->get_homepage_view()->_user_homepage_edit_popup_show(m_preference->get_user_homagepage());
		break;

		default:
			BROWSER_LOGD("menu_index[%d] is out of allowed for homepage setting", menu_index);
		break;
	}
}

void set_homepage_view::__homepage_radio_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	settings_basic_genlist_callback_data *cb_data = (settings_basic_genlist_callback_data *)data;
	settings_basic_menu_index menu_index = cb_data->menu_index;
	set_homepage_view *shv = (set_homepage_view *)cb_data->user_data;
	shv->m_set_init_homepage = EINA_FALSE;

	Elm_Object_Item *item = (Elm_Object_Item *)cb_data->it;
	elm_genlist_item_selected_set(item, EINA_FALSE);
	elm_genlist_item_update(item);

	BROWSER_LOGD("selected menu_index[%d]", menu_index);

	switch(menu_index) {
		case br_settings_sub_basic_menu_set_homepage_current_page:
			shv->_set_homepage_type_in_genlist(HOMEPAGE_TYPE_CURRENT_PAGE);
			shv->m_previous_radio_state = HOMEPAGE_TYPE_CURRENT_PAGE;
		break;

		case br_settings_sub_basic_menu_set_homepage_set_homepage_url:
			evas_object_freeze_events_set(shv->m_genlist, EINA_TRUE);
			m_browser->get_settings()->get_homepage_view()->_user_homepage_edit_popup_show(m_preference->get_user_homagepage());
		break;

		default:
			BROWSER_LOGD("menu_index[%d] is out of allowed for homepage setting", menu_index);
		break;
	}
}

void set_homepage_view::__set_homepage_ok_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	set_homepage_view *shv = (set_homepage_view *)data;
	webview *wv = m_browser->get_browser_view()->get_current_webview();
	homepage_type homepage =(homepage_type) elm_radio_value_get(shv->m_homepage_radio_group);

	BROWSER_LOGD("selected homepage[%d]", homepage);

	switch(homepage) {
		case HOMEPAGE_TYPE_DEFAULT_PAGE:
			m_preference->set_homepage_type(HOMEPAGE_TYPE_DEFAULT_PAGE);
			shv->m_previous_radio_state = HOMEPAGE_TYPE_DEFAULT_PAGE;
		break;

		case HOMEPAGE_TYPE_CURRENT_PAGE:
			m_preference->set_homepage_type(HOMEPAGE_TYPE_CURRENT_PAGE);
			shv->m_previous_radio_state = HOMEPAGE_TYPE_CURRENT_PAGE;
			if (wv && strlen(wv->get_uri()) > 0)
				m_preference->set_current_page_uri(wv->get_uri());
			else
				m_preference->set_current_page_uri("");
		break;

		case HOMEPAGE_TYPE_USER_HOMEPAGE:
			if (shv->m_input_homepage_uri_str.length()) {
				std::string trim_homepage_uri = _trim(shv->m_input_homepage_uri_str);
				const char *input_homepage_uri = trim_homepage_uri.c_str();
				char *homepage_uri = elm_entry_markup_to_utf8(input_homepage_uri);

				if (!homepage_uri) {
					BROWSER_LOGE("homepage is NULL");
					return;
				} else if (!strlen(homepage_uri)) {
					shv->m_is_closing = EINA_TRUE;
					m_preference->set_homepage_type(HOMEPAGE_TYPE_USER_HOMEPAGE);
					shv->m_previous_radio_state = HOMEPAGE_TYPE_USER_HOMEPAGE;
					m_preference->set_user_homagepage((const char *)blank_page);
					m_browser->get_settings()->set_homepage_type_in_genlist(HOMEPAGE_TYPE_USER_HOMEPAGE);
				} else if (shv->_is_valid_homepage(homepage_uri)) {
					shv->m_is_closing = EINA_TRUE;
					m_preference->set_homepage_type(HOMEPAGE_TYPE_USER_HOMEPAGE);
					shv->m_previous_radio_state = HOMEPAGE_TYPE_USER_HOMEPAGE;
					m_preference->set_user_homagepage((const char *)homepage_uri);
					m_browser->get_settings()->set_homepage_type_in_genlist(HOMEPAGE_TYPE_USER_HOMEPAGE);
				}

				free(homepage_uri);
			}
		break;

		default:
			BROWSER_LOGD("homepage[%d] is out of allowed for homepage setting", homepage);
		break;
	}
	shv->_back_to_previous_view();
}

void set_homepage_view::__set_homepage_cancel_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	elm_naviframe_item_pop(m_naviframe);
}

Eina_Bool set_homepage_view::__naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, EINA_FALSE, "data is NULL");

	return EINA_TRUE;
}

void set_homepage_view::__naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	set_homepage_view *shv = (set_homepage_view *)data;

	if (shv->m_navi_item == elm_naviframe_top_item_get(m_naviframe)) {

	}
}

void set_homepage_view::__genlist_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	RET_MSG_IF(!event_info, "event_info is NULL");
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	settings_basic_genlist_callback_data *item_cb_data = (settings_basic_genlist_callback_data *)elm_object_item_data_get(it);
	if (item_cb_data->menu_index == br_settings_sub_basic_menu_set_homepage_set_homepage_url) {
		elm_object_item_signal_emit(it, "elm,state,bottom", "");
		elm_object_item_access_unregister(it);
	} else {
		elm_object_item_signal_emit(it, "elm,state,top", "");
		elm_object_item_access_unregister(it);
	}
}

void set_homepage_view::__edit_popup_ok_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!obj, "obj is NULL");

	set_homepage_view *edit = (set_homepage_view *)data;
	const char *editfield_entry = elm_entry_entry_get(edit->m_editfield_entry);
	if (editfield_entry) {
		edit->m_input_homepage_uri_str = std::string(editfield_entry);
	}
	std::string trim_homepage_uri = _trim(edit->m_input_homepage_uri_str);
	const char *input_homepage_uri = trim_homepage_uri.c_str();
	char *homepage_uri = elm_entry_markup_to_utf8(input_homepage_uri);

	if ((!edit->m_input_homepage_uri_str.length()) || (!edit->_is_valid_homepage(homepage_uri))) {
		BROWSER_LOGE("homepage is invalid");
		if (edit->m_popup)
			evas_object_hide(edit->m_popup);
		edit->show_msg_popup(BR_STRING_INVALID_URL, 3, __uri_invalid_popup_expired_cb, data);

		free(homepage_uri);
		homepage_uri = NULL;
		return;
}
	elm_radio_value_set(edit->m_homepage_radio_group, HOMEPAGE_TYPE_USER_HOMEPAGE);
	edit->m_previous_radio_state = HOMEPAGE_TYPE_USER_HOMEPAGE;
	evas_object_freeze_events_set(edit->m_genlist, EINA_FALSE);
	edit->destroy_homepage_edit_popup_show();
}

void set_homepage_view::__edit_popup_entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	/* check that url is empty or not */
	set_homepage_view *edit = (set_homepage_view *)data;
	char *homepage = elm_entry_markup_to_utf8(elm_entry_entry_get(edit->m_editfield_entry));

	/* make save button deactivate unless uri is in editfield */
	if (homepage && strlen(homepage)) {
		elm_object_disabled_set(edit->m_ok_button, EINA_FALSE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_FALSE);
		edit->show_notification(homepage, obj, USER_HOMEPAGE_ENTRY_MAX_COUNT);
	} else {
		elm_object_domain_translatable_part_text_set(edit->m_editfield_entry, "elm.guide", BROWSER_DOMAIN_NAME, "IDS_BR_BODY_WEB_ADDRESS");
		elm_object_disabled_set(edit->m_ok_button, EINA_TRUE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_TRUE);
	}
	free(homepage);
}

void set_homepage_view::__edit_popup_cancel_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	set_homepage_view *edit = (set_homepage_view *)data;
	edit->destroy_homepage_edit_popup_show();
	evas_object_freeze_events_set(edit->m_genlist, EINA_FALSE);
	edit->_set_homepage_type_in_genlist((homepage_type)(edit->m_previous_radio_state));
}

void set_homepage_view::__edit_popup_editfield_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Evas_Object *editfield = (Evas_Object *)data;

	if (!elm_entry_is_empty(obj) && elm_object_focus_get(obj))
		elm_object_signal_emit(editfield, "elm,action,show,button", "");
	else
		elm_object_signal_emit(editfield, "elm,action,hide,button", "");
}

void set_homepage_view::__edit_popup_clear_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Evas_Object *entry = (Evas_Object *)data;

	elm_entry_entry_set(entry, "");
}

void set_homepage_view::__uri_invalid_popup_expired_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	set_homepage_view *edit = (set_homepage_view *)data;
	if (edit->m_popup)
		evas_object_show(edit->m_popup);
	elm_object_focus_set(edit->m_editfield_entry, EINA_TRUE);
}

void set_homepage_view::__edit_popup_entry_activated_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!obj, "obj is NULL");

	set_homepage_view *edit = (set_homepage_view *)data;
	const char *editfield_entry = elm_entry_entry_get(edit->m_editfield_entry);
	if(editfield_entry) {
		edit->m_input_homepage_uri_str = std::string(editfield_entry);
	}
	std::string trim_homepage_uri = _trim(edit->m_input_homepage_uri_str);
	const char *input_homepage_uri = trim_homepage_uri.c_str();
	char *homepage_uri = elm_entry_markup_to_utf8(input_homepage_uri);

	if ((!edit->m_input_homepage_uri_str.length()) || (!edit->_is_valid_homepage(homepage_uri))) {
		BROWSER_LOGE("homepage is invalid");
		if (edit->m_popup)
			evas_object_hide(edit->m_popup);
		edit->show_msg_popup(BR_STRING_INVALID_URL, 3, __uri_invalid_popup_expired_cb, data);

		free(homepage_uri);
		homepage_uri = NULL;
		return;
	}
	free(homepage_uri);
	elm_radio_value_set(edit->m_homepage_radio_group, HOMEPAGE_TYPE_USER_HOMEPAGE);
	evas_object_freeze_events_set(edit->m_genlist, EINA_FALSE);
	edit->destroy_homepage_edit_popup_show();
}






/* 5 new lines for class defined depends on each menu item */
homepage_edit::homepage_edit(void)
:
	m_is_closing(EINA_FALSE),
	m_popup(NULL),
	m_editfield_entry(NULL),
	m_ok_button(NULL)
{
	BROWSER_LOGD("");
	//evas_object_smart_callback_add(m_conformant, "virtualkeypad,state,off", __homepage_edit_keypad_hide_cb, this);
}

homepage_edit::~homepage_edit(void)
{
	BROWSER_LOGD("");
	if (m_popup)
		evas_object_del(m_popup);
	m_popup = NULL;
	//evas_object_smart_callback_del(m_conformant, "virtualkeypad,state,off", __homepage_edit_keypad_hide_cb);
}

Eina_Bool homepage_edit::user_homepage_edit_popup_show(char *homepage)
{
	BROWSER_LOGD("");
	m_popup = brui_popup_add(m_naviframe);
	RETV_MSG_IF(!m_popup, EINA_FALSE, "m_popup is NULL");

	elm_object_style_set(m_popup, "default");
	elm_object_domain_translatable_part_text_set(m_popup, "title,text", BROWSER_DOMAIN_NAME, "IDS_BR_HEADER_SET_HOMEPAGE_ABB");
	eext_object_event_callback_add(m_popup, EEXT_CALLBACK_BACK, __edit_popup_cancel_button_cb, this);

	/* for margins */
	Evas_Object *outer_layout = elm_layout_add(m_popup);
	RETV_MSG_IF(!outer_layout, EINA_FALSE, "elm_layout_add is failed");
	elm_layout_file_set(outer_layout, browser_edj_dir"/browser-popup.edj", "simple_message_popup");
	elm_layout_theme_set(outer_layout, "layout", "editfield", "singleline");
	evas_object_size_hint_weight_set(outer_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(outer_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *layout = elm_layout_add(outer_layout);
	RETV_MSG_IF(!layout, EINA_FALSE, "elm_layout_add is failed");

	elm_layout_file_set(layout, browser_edj_dir"/browser-popup.edj", "entry_text_popup");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_entry_limit_size.max_char_count = 0;
	m_entry_limit_size.max_byte_count = USER_HOMEPAGE_ENTRY_MAX_COUNT;

	m_editfield_entry = elm_entry_add(m_popup);
	RETV_MSG_IF(!m_editfield_entry, EINA_FALSE, "ea_editfield_add is failed");

	elm_entry_markup_filter_append(m_editfield_entry, elm_entry_filter_limit_size, &m_entry_limit_size);

	Evas_Object *clear_button = elm_object_part_content_get(m_editfield_entry, "elm.swallow.clear");
	elm_access_info_set(clear_button, ELM_ACCESS_INFO, BR_STRING_CLEAR_ALL);

	/* To apply common sytle font type. */
	elm_object_style_set(m_editfield_entry, "editfield/popup");
	elm_object_signal_emit(m_editfield_entry, "elm,action,hide,search_icon", "");

	elm_object_part_content_set(layout, "elm.swallow.content", m_editfield_entry);
	elm_entry_cnp_mode_set(m_editfield_entry, ELM_CNP_MODE_PLAINTEXT);
	elm_entry_scrollable_set(m_editfield_entry, EINA_TRUE);
	elm_entry_autocapital_type_set(m_editfield_entry, ELM_AUTOCAPITAL_TYPE_NONE);
	elm_entry_prediction_allow_set(m_editfield_entry, EINA_FALSE);
	elm_entry_single_line_set(m_editfield_entry, EINA_TRUE);

#if defined(HW_MORE_BACK_KEY)
	eext_entry_selection_back_event_allow_set(m_editfield_entry, EINA_TRUE);
#endif
	if (homepage && strlen(homepage)) {
		char *markup = elm_entry_utf8_to_markup(homepage);
		elm_object_text_set(m_editfield_entry, markup);
		free(markup);
	}
	else
		elm_object_text_set(m_editfield_entry, blank_page);

	if (homepage) free(homepage);
	homepage = NULL;

	const char *input_text = elm_entry_entry_get(m_editfield_entry);
	if (input_text && strlen(input_text) != 0) {
		/* There is a timing issue between editfield clear button and text show.
			therefore using callback when EVAS_CALLBACK_SHOW is called for clear button */
		evas_object_event_callback_add(clear_button, EVAS_CALLBACK_SHOW, __on_shown_clear_btn_cb, m_editfield_entry);
	}

	elm_entry_input_panel_layout_set(m_editfield_entry, ELM_INPUT_PANEL_LAYOUT_URL);
	elm_entry_input_panel_return_key_type_set(m_editfield_entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
	elm_entry_context_menu_clear(m_editfield_entry);
	elm_entry_context_menu_item_add(m_editfield_entry, "Clipboard", NULL, ELM_ICON_STANDARD, NULL, NULL);

	elm_object_part_content_set(layout, "elm.swallow.content", m_editfield_entry);
	elm_object_part_content_set(outer_layout, "elm.swallow.content", layout);
	elm_object_content_set(m_popup, outer_layout);

	if (!m_browser->is_keyboard_active())
		evas_object_smart_callback_add(m_editfield_entry, "activated", __edit_popup_entry_activated_cb, this);
	evas_object_smart_callback_add(m_editfield_entry, "changed", __edit_popup_entry_changed_cb, this);
	evas_object_smart_callback_add(m_editfield_entry, "preedit,changed", __edit_popup_entry_changed_cb, this);

	Evas_Object *cancel_button = elm_button_add(m_popup);
	if (!cancel_button) {
		BROWSER_LOGE("elm_button_add for cancel_button is failed");
		return EINA_FALSE;
	}
	elm_object_domain_translatable_part_text_set(cancel_button, NULL, BROWSER_DOMAIN_NAME, "IDS_BR_SK_CANCEL");
	elm_object_style_set(cancel_button, "popup");
	elm_object_part_content_set(m_popup, "button1", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __edit_popup_cancel_button_cb, this);


	m_ok_button = elm_button_add(m_popup);
	if (!m_ok_button) {
		BROWSER_LOGE("elm_button_add for save_button is failed");
		return EINA_FALSE;
	}
	elm_object_domain_translatable_part_text_set(m_ok_button, NULL, BROWSER_DOMAIN_NAME, "IDS_BR_BUTTON_SET");
	elm_object_style_set(m_ok_button, "popup");
	elm_object_part_content_set(m_popup, "button2", m_ok_button);
	evas_object_smart_callback_add(m_ok_button, "clicked", __edit_popup_ok_button_cb, this);
	evas_object_show(m_popup);

	return EINA_TRUE;
}

void homepage_edit::destroy_homepage_edit_popup_show(void)
{
	BROWSER_LOGD("");

	if (m_popup != NULL)
		evas_object_del(m_popup);
	m_popup = NULL;
}

Eina_Bool homepage_edit::_is_valid_homepage (const char *uri)
{
	BROWSER_SECURE_LOGD("uri=[%s]", uri);

	RETV_MSG_IF(!uri, EINA_FALSE, "uri is NULL");

	if (!strcmp(uri, blank_page))
		return EINA_TRUE;

	regex_t regex;
	if (regcomp(&regex, HOMEPAGE_URLEXPR, REG_EXTENDED | REG_ICASE) != 0) {
		BROWSER_LOGD("regcomp failed");
		return EINA_FALSE;
	}

	if (regexec(&regex, uri, 0, NULL, REG_NOTEOL) == 0) {
		BROWSER_LOGD("url expression");
		regfree(&regex);
		if (m_browser->get_browser_view()->is_valid_uri(uri))
			return EINA_TRUE;
		else
			return EINA_FALSE;
	}

	regfree(&regex);
	if (m_browser->get_browser_view()->is_valid_uri(uri))
		return EINA_TRUE;

	return EINA_FALSE;
}

void homepage_edit::__edit_popup_cancel_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	homepage_edit *edit = (homepage_edit *)data;
	edit->destroy_homepage_edit_popup_show();
	evas_object_freeze_events_set(m_browser->get_settings()->get_genlist(), EINA_FALSE);
}

void homepage_edit::__edit_popup_ok_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!obj, "obj is NULL");

	homepage_edit *edit = (homepage_edit *)data;
	const char *editfield_entry = elm_entry_entry_get(edit->m_editfield_entry);
	if (!editfield_entry) return;
	std::string input_homepage_uri_str = std::string(editfield_entry);

	std::string trim_homepage_uri = _trim(input_homepage_uri_str);
	const char *input_homepage_uri = trim_homepage_uri.c_str();
	char *homepage = elm_entry_markup_to_utf8(input_homepage_uri);
	evas_object_freeze_events_set(m_browser->get_settings()->get_genlist(), EINA_FALSE);

	if (!homepage) {
		BROWSER_LOGE("homepage is NULL");
		return;
	} else if(!strlen(homepage)) {
		edit->m_is_closing = EINA_TRUE;
		m_preference->set_homepage_type(HOMEPAGE_TYPE_USER_HOMEPAGE);
		m_preference->set_user_homagepage((const char *)blank_page);
		m_browser->get_settings()->set_homepage_type_in_genlist(HOMEPAGE_TYPE_USER_HOMEPAGE);
		edit->hide_homepage_sub_items(data);
	} else {
		if (edit->_is_valid_homepage(homepage)) {
			edit->m_is_closing = EINA_TRUE;
			m_preference->set_homepage_type(HOMEPAGE_TYPE_USER_HOMEPAGE);
			m_preference->set_user_homagepage((const char *)homepage);
			m_browser->get_settings()->set_homepage_type_in_genlist(HOMEPAGE_TYPE_USER_HOMEPAGE);
			edit->hide_homepage_sub_items(data);
		} else {
			BROWSER_LOGE("homepage is invalid");
			if (edit->m_popup)
				evas_object_hide(edit->m_popup);
			edit->show_msg_popup(BR_STRING_INVALID_URL, 3, __uri_invalid_popup_expired_cb, data);

			free(homepage);
			homepage = NULL;

			return;
		}
	}

	free(homepage);
	edit->destroy_homepage_edit_popup_show();
}

void homepage_edit::__edit_popup_entry_activated_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!obj, "obj is NULL");

	homepage_edit *edit = (homepage_edit *)data;
	const char *editfield_entry = elm_entry_entry_get(edit->m_editfield_entry);
	if (!editfield_entry) return;
	std::string input_homepage_uri_str = std::string(editfield_entry);

	std::string trim_homepage_uri = _trim(input_homepage_uri_str);
	const char *input_homepage_uri = trim_homepage_uri.c_str();
	char *homepage = elm_entry_markup_to_utf8(input_homepage_uri);
	evas_object_freeze_events_set(m_browser->get_settings()->get_genlist(), EINA_FALSE);

	if (!homepage) {
		BROWSER_LOGE("homepage is NULL");
		return;
	} else if(!strlen(homepage)) {
		edit->m_is_closing = EINA_TRUE;
		m_preference->set_homepage_type(HOMEPAGE_TYPE_USER_HOMEPAGE);
		m_preference->set_user_homagepage((const char *)blank_page);
		m_browser->get_settings()->set_homepage_type_in_genlist(HOMEPAGE_TYPE_USER_HOMEPAGE);
		edit->hide_homepage_sub_items(data);
	} else {
		if (edit->_is_valid_homepage(homepage)) {
			edit->m_is_closing = EINA_TRUE;
			m_preference->set_homepage_type(HOMEPAGE_TYPE_USER_HOMEPAGE);
			m_preference->set_user_homagepage((const char *)homepage);
			m_browser->get_settings()->set_homepage_type_in_genlist(HOMEPAGE_TYPE_USER_HOMEPAGE);
			edit->hide_homepage_sub_items(data);
		} else {
			BROWSER_LOGE("homepage is invalid");
			if (edit->m_popup)
				evas_object_hide(edit->m_popup);
			edit->show_msg_popup(BR_STRING_INVALID_URL, 3, __uri_invalid_popup_expired_cb, data);

			free(homepage);
			homepage = NULL;

			return;
		}
	}

	free(homepage);
	edit->destroy_homepage_edit_popup_show();
}

void homepage_edit::__edit_popup_entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	/* check that url is empty or not */
	homepage_edit *edit = (homepage_edit *)data;
	char *homepage = elm_entry_markup_to_utf8(elm_entry_entry_get(edit->m_editfield_entry));

	/* make save button deactivate unless uri is in editfield */
	if (homepage && strlen(homepage)) {
		elm_object_disabled_set(edit->m_ok_button, EINA_FALSE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_FALSE);
		edit->show_notification(homepage, obj, USER_HOMEPAGE_ENTRY_MAX_COUNT);
	} else {
		elm_object_domain_translatable_part_text_set(edit->m_editfield_entry, "elm.guide", BROWSER_DOMAIN_NAME, "IDS_BR_POP_ENTER_URL");
		elm_object_disabled_set(edit->m_ok_button, EINA_TRUE);
		elm_entry_input_panel_return_key_disabled_set(obj, EINA_TRUE);
	}
	free(homepage);
}

void homepage_edit::__on_shown_clear_btn_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is Null");
	Evas_Object *entry =(Evas_Object *)data;

	elm_entry_cursor_end_set(entry);
	Evas_Object *clear_button = elm_object_part_content_get(entry, "elm.swallow.clear");
	evas_object_event_callback_del(clear_button, EVAS_CALLBACK_SHOW, __on_shown_clear_btn_cb);
}

void homepage_edit::__uri_invalid_popup_expired_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	homepage_edit *edit = (homepage_edit *)data;
	if (edit->m_popup)
		evas_object_show(edit->m_popup);
	elm_object_focus_set(edit->m_editfield_entry, EINA_TRUE);
}

void homepage_edit::__homepage_edit_keypad_show_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
}

void homepage_edit::__homepage_edit_keypad_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	homepage_edit *edit = (homepage_edit *)data;

	if (edit->m_is_closing == EINA_TRUE)
		ecore_timer_add(0.4, __homepage_edit_keypad_hide_timer_cb, data);
}

void homepage_edit::hide_homepage_sub_items(void *data)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	if (m_is_closing == EINA_TRUE)
		ecore_timer_add(0.4, __homepage_edit_keypad_hide_timer_cb, data);
}

Eina_Bool homepage_edit::__homepage_edit_keypad_hide_timer_cb(void *data)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, ECORE_CALLBACK_CANCEL, "data is NULL");

	homepage_edit *edit = (homepage_edit *)data;
	if (edit->m_is_closing == EINA_TRUE)
		m_browser->get_settings()->hide_homepage_sub_items_in_genlist();

	edit->m_is_closing = EINA_FALSE;

	return ECORE_CALLBACK_CANCEL;
}


auto_fill_form_manager::auto_fill_form_manager(void)
:
	m_list_view(NULL),
	m_composer(NULL)
{
	BROWSER_LOGD("");

	m_auto_fill_form_item_list = load_entire_item_list();
#ifndef WEBKIT_EFL
	ewk_context_form_autofill_profile_changed_callback_set(profiles_updated_cb, this);
#endif
}

auto_fill_form_manager::~auto_fill_form_manager(void)
{
	BROWSER_LOGD("");

	if (m_list_view)
		delete m_list_view;
	m_list_view = NULL;

	if(m_composer)
		delete m_composer;
	m_composer = NULL;
#ifndef WEBKIT_EFL
	ewk_context_form_autofill_profile_changed_callback_set(NULL, this);
#endif
}

void auto_fill_form_manager::profiles_updated_cb(void* data)
{
	auto_fill_form_manager *formManager = (auto_fill_form_manager*)data;
	formManager->refresh_items_view();
}

void auto_fill_form_manager::refresh_items_view()
{
	load_entire_item_list();
	if (m_list_view)
		m_list_view->refresh_view();
}

Eina_Bool auto_fill_form_manager::save_auto_fill_form_item(auto_fill_form_item_data *item_data)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!item_data, EINA_FALSE, "key is NULL");
	RETV_MSG_IF(!item_data->name, EINA_FALSE, "key is NULL");

	return EINA_TRUE;
}

Eina_Bool auto_fill_form_manager::delete_auto_fill_form_item(auto_fill_form_item *item)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!item, EINA_FALSE, "item is NULL");

	for (unsigned int i = 0; i < m_auto_fill_form_item_list.size(); i++) {
		if (m_auto_fill_form_item_list[i]->get_profile_id() == item->get_profile_id()) {

			Ewk_Context *ewk_context = ewk_context_default_get();
			if (ewk_context_form_autofill_profile_remove(ewk_context, item->get_profile_id()) == EINA_FALSE)
				return EINA_FALSE;

			m_auto_fill_form_item_list.erase(m_auto_fill_form_item_list.begin() + i);
		}
	}

	return EINA_TRUE;
}

Eina_Bool auto_fill_form_manager::delete_all_auto_fill_form_items(void)
{
	BROWSER_LOGD("");

	m_auto_fill_form_item_list.clear();

	Ewk_Context *ewk_context = ewk_context_default_get();
	Eina_List *entire_item_list = ewk_context_form_autofill_profile_get_all(ewk_context);

	Eina_List *list = NULL;
	void *item_data = NULL;

	EINA_LIST_FOREACH(entire_item_list, list, item_data) {
		if (item_data) {
			Ewk_Autofill_Profile *profile = (Ewk_Autofill_Profile *)item_data;
			ewk_context_form_autofill_profile_remove(ewk_context, ewk_autofill_profile_id_get(profile));
		}
	}

	return EINA_TRUE;
}

unsigned int auto_fill_form_manager::get_auto_fill_form_item_count(void)
{
	BROWSER_LOGD("");

	return m_auto_fill_form_item_list.size();
}

auto_fill_form_item *auto_fill_form_manager::create_new_auto_fill_form_item(Ewk_Autofill_Profile *profile)
{
	BROWSER_LOGD("");
	auto_fill_form_item *item = NULL;
	if (!profile)
		item = new auto_fill_form_item(NULL);
	else {
		auto_fill_form_item_data *item_data = (auto_fill_form_item_data *)malloc(sizeof(auto_fill_form_item_data));
		if (!item_data) {
			BROWSER_LOGE("Malloc failed to get item_data");
			return NULL;
		}
		memset(item_data, 0x00, sizeof(auto_fill_form_item_data));
		item_data->profile_id = ewk_autofill_profile_id_get(profile);
		item_data->name = ewk_autofill_profile_data_get(profile, EWK_PROFILE_NAME);
		item_data->company = ewk_autofill_profile_data_get(profile, EWK_PROFILE_COMPANY);
		item_data->primary_address = ewk_autofill_profile_data_get(profile, EWK_PROFILE_ADDRESS1);
		item_data->secondary_address = ewk_autofill_profile_data_get(profile, EWK_PROFILE_ADDRESS2);
		item_data->city_town = ewk_autofill_profile_data_get(profile, EWK_PROFILE_CITY_TOWN);
		item_data->state_province_region = ewk_autofill_profile_data_get(profile, EWK_PROFILE_STATE_PROVINCE_REGION);
		item_data->post_code = ewk_autofill_profile_data_get(profile, EWK_PROFILE_ZIPCODE);
		item_data->country = ewk_autofill_profile_data_get(profile, EWK_PROFILE_COUNTRY);
		item_data->phone_number = ewk_autofill_profile_data_get(profile, EWK_PROFILE_PHONE);
		item_data->email_address = ewk_autofill_profile_data_get(profile, EWK_PROFILE_EMAIL);
		item_data->activation = false;
		item_data->compose_mode = profile_edit;

		item = new auto_fill_form_item(item_data);
		free(item_data);
	}

	return item;
}

auto_fill_form_list_view *auto_fill_form_manager::show_list_view(void)
{
	BROWSER_LOGD("");

	if (m_list_view)
		delete m_list_view;

	m_list_view = new auto_fill_form_list_view(this);
	m_list_view->show();

	return m_list_view;
}

auto_fill_form_compose_view *auto_fill_form_manager::show_composer(auto_fill_form_item *item)
{
	BROWSER_LOGD("");

	if (m_composer)
		delete m_composer;

	m_composer = new auto_fill_form_compose_view(item);
	m_composer->show();

	return m_composer;
}

Eina_Bool auto_fill_form_manager::delete_list_view(void)
{
	BROWSER_LOGD("");
	if (m_list_view)
		delete m_list_view;
	m_list_view = NULL;

	return EINA_TRUE;
}


Eina_Bool auto_fill_form_manager::delete_composer(void)
{
	BROWSER_LOGD("");

	if (m_composer)
		delete m_composer;
	m_composer = NULL;

	return EINA_TRUE;
}


void auto_fill_form_manager::see_all_data(void)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_auto_fill_form_item_list.size(); i++) {
		BROWSER_SECURE_LOGD("m_auto_fill_form_item_list[%d] item - start", i);
		BROWSER_SECURE_LOGD("************************************************************************************");
		BROWSER_SECURE_LOGD("m_auto_fill_form_item_list[%d].id[%d]", i, m_auto_fill_form_item_list[i]->get_profile_id());
		BROWSER_SECURE_LOGD("m_auto_fill_form_item_list[%d].m_name[%s]", i, m_auto_fill_form_item_list[i]->get_name());
		BROWSER_SECURE_LOGD("m_auto_fill_form_item_list[%d].m_company[%s]", i, m_auto_fill_form_item_list[i]->get_company());
		BROWSER_SECURE_LOGD("m_auto_fill_form_item_list[%d].m_primary_address[%s]", i, m_auto_fill_form_item_list[i]->get_primary_address());
		BROWSER_SECURE_LOGD("m_auto_fill_form_item_list[%d].m_secondary_address[%s]", i, m_auto_fill_form_item_list[i]->get_secondary_address2());
		BROWSER_SECURE_LOGD("m_auto_fill_form_item_list[%d].m_city_town[%s]", i, m_auto_fill_form_item_list[i]->get_city_town());
		BROWSER_SECURE_LOGD("m_auto_fill_form_item_list[%d].m_state_province_region[%s]", i, m_auto_fill_form_item_list[i]->get_state_province());
		BROWSER_SECURE_LOGD("m_auto_fill_form_item_list[%d].m_post_code[%s]", i, m_auto_fill_form_item_list[i]->get_post_code());
		BROWSER_SECURE_LOGD("m_auto_fill_form_item_list[%d].m_country_region[%s]", i, m_auto_fill_form_item_list[i]->get_country());
		BROWSER_SECURE_LOGD("m_auto_fill_form_item_list[%d].m_phone_number[%s]", i, m_auto_fill_form_item_list[i]->get_phone_number());
		BROWSER_SECURE_LOGD("m_auto_fill_form_item_list[%d].m_email_address[%s]", i, m_auto_fill_form_item_list[i]->get_email_address());
		BROWSER_SECURE_LOGD("m_auto_fill_form_item_list[%d].m_activation[%d]", i, m_auto_fill_form_item_list[i]->get_activation());
		BROWSER_SECURE_LOGD("************************************************************************************");
		BROWSER_SECURE_LOGD("m_auto_fill_form_item_list[%d] item - end", i);
	}

	return;
}

std::vector<auto_fill_form_item *> auto_fill_form_manager::load_entire_item_list(void)
{
	BROWSER_LOGD("");

	m_auto_fill_form_item_list.clear();

	Ewk_Context *ewk_context = ewk_context_default_get();
	Eina_List *entire_item_list = ewk_context_form_autofill_profile_get_all(ewk_context);

	Eina_List *list = NULL;
	void *item_data = NULL;

	EINA_LIST_FOREACH(entire_item_list, list, item_data) {
		if (item_data) {
			Ewk_Autofill_Profile *profile = (Ewk_Autofill_Profile *)item_data;
			auto_fill_form_item *item = create_new_auto_fill_form_item(profile);
			if (item)
				m_auto_fill_form_item_list.push_back(item);
		}
	}

	return m_auto_fill_form_item_list;
}

Eina_Bool auto_fill_form_manager::add_item_to_list(auto_fill_form_item *item)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!item, EINA_FALSE, "item is NULL");

	m_auto_fill_form_item_list.push_back(item);
	//m_list_view->refresh_view();

	return EINA_TRUE;;
}

auto_fill_form_item::auto_fill_form_item(auto_fill_form_item_data *item_data)
{
	BROWSER_LOGD("");

	memset(&m_item_data, 0x00, sizeof(m_item_data));
	m_item_data.profile_id = -1; //The id starts with 0 from webkit

	if (item_data) {
		m_item_data.profile_id= item_data->profile_id;
		m_item_data.name = item_data->name;
		m_item_data.company = item_data->company;
		m_item_data.primary_address = item_data->primary_address;
		m_item_data.secondary_address = item_data->secondary_address;
		m_item_data.city_town = item_data->city_town;
		m_item_data.state_province_region = item_data->state_province_region;
		m_item_data.post_code = item_data->post_code;
		m_item_data.country = item_data->country;
		m_item_data.phone_number = item_data->phone_number;
		m_item_data.email_address = item_data->email_address;
		m_item_data.activation = item_data->activation;
		m_item_data.compose_mode = item_data->compose_mode;
	}
}

auto_fill_form_item::~auto_fill_form_item(void)
{
	BROWSER_LOGD("");
}

profile_save_errorcode auto_fill_form_item::save_item(void)
{
	BROWSER_LOGD("");

	Ewk_Autofill_Profile *profile = ewk_autofill_profile_new();
	if (!profile) {
		BROWSER_LOGE("Failed to ewk_autofill_profile_new");
		return profile_create_failed;
	}

	if (m_item_data.name)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_NAME, m_item_data.name);

	if (m_item_data.company)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_COMPANY, m_item_data.company);

	if (m_item_data.primary_address)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_ADDRESS1, m_item_data.primary_address);

	if (m_item_data.secondary_address)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_ADDRESS2, m_item_data.secondary_address);

	if (m_item_data.city_town)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_CITY_TOWN, m_item_data.city_town);

	if (m_item_data.state_province_region)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_STATE_PROVINCE_REGION, m_item_data.state_province_region);

	if (m_item_data.post_code)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_ZIPCODE, m_item_data.post_code);

	if (m_item_data.country)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_COUNTRY, m_item_data.country);

	if (m_item_data.phone_number)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_PHONE, m_item_data.phone_number);

	if (m_item_data.email_address)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_EMAIL, m_item_data.email_address);

	Ewk_Context *ewk_context = ewk_context_default_get();
	if (ewk_context_form_autofill_profile_add(ewk_context, profile) == EINA_FALSE) {
		BROWSER_LOGE("Failed to ewk_context_form_autofill_profile_add");
		ewk_autofill_profile_delete(profile);
		return duplicate_profile;
	}
	m_item_data.profile_id = ewk_autofill_profile_id_get(profile);
	ewk_autofill_profile_delete(profile);

	return save_error_none;
}

profile_edit_errorcode auto_fill_form_item::update_item(void)
{
	BROWSER_LOGD("");

	/* Find profile with id */
	Ewk_Context *ewk_context = ewk_context_default_get();
	Ewk_Autofill_Profile *profile = ewk_context_form_autofill_profile_get(ewk_context, m_item_data.profile_id);
	if (!profile) {
		BROWSER_LOGE("Failed to ewk_context_form_autofill_profile_get with ID [%d]", m_item_data.profile_id);
		return profile_edit_failed;
	}

	if (m_item_data.name)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_NAME, m_item_data.name);
	if (m_item_data.company)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_COMPANY, m_item_data.company);
	if (m_item_data.primary_address)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_ADDRESS1, m_item_data.primary_address);
	if (m_item_data.secondary_address)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_ADDRESS2, m_item_data.secondary_address);
	if (m_item_data.city_town)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_CITY_TOWN, m_item_data.city_town);
	if (m_item_data.state_province_region)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_STATE_PROVINCE_REGION, m_item_data.state_province_region);
	if (m_item_data.post_code)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_ZIPCODE, m_item_data.post_code);
	if (m_item_data.country)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_COUNTRY, m_item_data.country);
	if (m_item_data.phone_number)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_PHONE, m_item_data.phone_number);
	if (m_item_data.email_address)
		ewk_autofill_profile_data_set(profile, EWK_PROFILE_EMAIL, m_item_data.email_address);

	if (ewk_context_form_autofill_profile_set(ewk_context, m_item_data.profile_id, profile) == EINA_FALSE) {
		BROWSER_LOGE("Failed to ewk_context_form_autofill_profile_set with ID [%d]", m_item_data.profile_id);
		return profile_already_exist;
	}

	return update_error_none;
}

auto_fill_form_list_view::auto_fill_form_list_view(auto_fill_form_manager *affm)
:
	m_manager(affm),
	m_main_layout(NULL),
	m_genlist(NULL),
	m_auto_fill_form_check(NULL),
	m_plus_button(NULL),
	m_ctx_popup_more_menu(NULL),
	m_navi_it(NULL),
	m_multiline_1text_item_class(NULL),
	m_dialogue_1text_item_class(NULL),
	m_dialogue_group_title_item_class(NULL),
	m_dialogue_bottom_item_class(NULL),
	m_delete_view(NULL),
	m_select_popup(NULL)
{
	BROWSER_LOGD("");
}

auto_fill_form_list_view::~auto_fill_form_list_view(void)
{
	BROWSER_LOGD("");

	delete_list_item_selected_popup();
	_destroy_all_genlist_style_items();
	_remove_each_item_callback_data();
	if (m_delete_view)
		delete m_delete_view;
	m_delete_view = NULL;

	if (m_ctx_popup_more_menu) {
#if !defined(SPLIT_WINDOW)
		evas_object_smart_callback_del(elm_object_top_widget_get(m_ctx_popup_more_menu), "rotation,changed", __rotate_ctxpopup_cb);
#endif
		evas_object_del(m_ctx_popup_more_menu);
		m_ctx_popup_more_menu = NULL;
	}

	evas_object_smart_callback_del(m_naviframe, "transition,finished", __naviframe_pop_finished_cb);
}

auto_profile_delete_view *auto_fill_form_list_view::_get_delete_view(void)
{
	BROWSER_LOGD("");
	if (!m_delete_view)
		m_delete_view = new auto_profile_delete_view();

	return m_delete_view;
}

auto_profile_delete_view::auto_profile_delete_view(void)
:
	m_del_layout(NULL),
	m_del_genlist(NULL),
	m_button_done(NULL),
	m_auto_fill_form_check(NULL),
#if defined(DELETE_CONFIRM_POPUP_ENABLE)
	m_delete_popup(NULL),
#endif
	m_naviframe_item(NULL),
	m_select_all_flag(EINA_FALSE),
	m_count_checked_item(0),
	m_item_callback_data(NULL),
	m_select_all_callback_data(NULL)
{
	BROWSER_LOGD("");
}

auto_profile_delete_view::~auto_profile_delete_view(void)
{
	BROWSER_LOGD("");
	m_select_all_flag = EINA_FALSE;
#if defined(DELETE_CONFIRM_POPUP_ENABLE)
	if (m_delete_popup)
		evas_object_smart_callback_del(m_delete_popup, "language,changed", __auto_profile_delete_popup_lang_changed_cb);
	m_delete_popup = NULL;
#endif
	_remove_each_item_callback_data();
}

void auto_profile_delete_view::__checkbox_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_profile_delete_view *apdv = (auto_profile_delete_view *)data;
	Eina_Bool all_selected = EINA_TRUE;
	Elm_Object_Item *it = elm_genlist_first_item_get(apdv->m_del_genlist);
	Elm_Object_Item *it_select_all = it;
	it = elm_genlist_item_next_get(it);//To avoid the first "select All" option.
	while (it) {
		genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
		if (cb_data) {
			if (cb_data->is_checked == EINA_FALSE) {
				all_selected = EINA_FALSE;
				break;
			}
		}
		it = elm_genlist_item_next_get(it);
	}
	genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it_select_all);
	if (cb_data) {
		cb_data->is_checked = all_selected;
		elm_genlist_item_update(it_select_all);
		apdv->m_select_all_flag = all_selected;
	}
	it = elm_genlist_first_item_get(apdv->m_del_genlist);
	it = elm_genlist_item_next_get(it);//To avoid the first "select All" option.
	apdv->m_count_checked_item = 0;
	while (it) {
		genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
		if (cb_data) {
			if (cb_data->is_checked == EINA_TRUE)
				apdv->m_count_checked_item++;
		}
		it = elm_genlist_item_next_get(it);
	}
	apdv->_set_selected_title();
	/* Set delete button status */
	it = elm_genlist_first_item_get(apdv->m_del_genlist);
	while (it) {
		genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
		if (cb_data) {
			if (cb_data->is_checked == EINA_TRUE) {
				elm_object_disabled_set(apdv->m_button_done, EINA_FALSE);
				return;
			}
		}
		it = elm_genlist_item_next_get(it);
	}
	elm_object_disabled_set(apdv->m_button_done, EINA_TRUE);
}

void auto_profile_delete_view::show(void )
{
	BROWSER_LOGD("");

	m_del_layout = _create_auto_form_delete_layout(m_naviframe);

	if (!m_naviframe) {
		BROWSER_LOGE("create delete layout failed");
		return ;
	}
	m_naviframe_item = elm_naviframe_item_push(m_naviframe, NULL, NULL, NULL, m_del_layout, NULL);

	Evas_Object *btn_cancel = elm_button_add(m_naviframe);
	if (!btn_cancel) return;
	elm_object_style_set(btn_cancel, "naviframe/title_cancel");
	evas_object_smart_callback_add(btn_cancel, "clicked", __cancel_button_click_cb, this);
	elm_object_item_part_content_set(m_naviframe_item, "title_left_btn", btn_cancel);

	Evas_Object *btn_save = elm_button_add(m_naviframe);
	if (!btn_save) return;
	elm_object_style_set(btn_save, "naviframe/title_done");
	evas_object_smart_callback_add(btn_save, "clicked", __delete_item_cb, this);
	elm_object_item_part_content_set(m_naviframe_item, "title_right_btn", btn_save);
	m_button_done = btn_save;
	elm_object_disabled_set(m_button_done, EINA_TRUE);

	elm_naviframe_item_pop_cb_set(m_naviframe_item, __naviframe_pop_cb, this);
	evas_object_smart_callback_del(m_naviframe, "transition,finished", __naviframe_pop_finished_cb);

	if (m_del_genlist)
		evas_object_smart_callback_add(m_del_genlist, "language,changed", __auto_profile_delete_title_lang_changed_cb, this);
	m_count_checked_item = 0;
	_set_selected_title();
}

Evas_Object *auto_profile_delete_view::_create_auto_form_delete_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	if (m_auto_fill_form_item_callback_data_list.size())
		_remove_each_item_callback_data();

	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	m_select_all_callback_data = (genlist_callback_data *)malloc(sizeof(genlist_callback_data));
	if (!m_select_all_callback_data){
			if (item_ic)
				elm_genlist_item_class_free(item_ic);
			return NULL;
	}
	m_select_all_callback_data->user_data = this;
	m_select_all_callback_data->type = DELETE_PROFILE_SELECT_ALL;
	m_select_all_callback_data->is_checked = EINA_FALSE;

	m_item_callback_data = (genlist_callback_data *)malloc(sizeof(genlist_callback_data));
	if (!m_item_callback_data){
			if (item_ic)
				elm_genlist_item_class_free(item_ic);
			return NULL;
	}
	m_item_callback_data->user_data = this;
	m_item_callback_data->type = DELETE_PROFILE_ITEM;
	m_item_callback_data->is_checked = EINA_FALSE;

	Elm_Genlist_Item_Class *selectall_item_ic = elm_genlist_item_class_new();
	if (!selectall_item_ic) {
		BROWSER_LOGE("elm_genlist_item_class_new failed");
		if (item_ic)
			elm_genlist_item_class_free(item_ic);

		return NULL;
	}
	selectall_item_ic->item_style = "select_all";
	selectall_item_ic->func.text_get = __text_get_cb;
	selectall_item_ic->func.content_get = __content_get_cb;
	selectall_item_ic->func.state_get = NULL;
	selectall_item_ic->func.del = NULL;

	if (!item_ic) {
		BROWSER_LOGE("elm_genlist_item_class_new failed");
		if (selectall_item_ic)
			elm_genlist_item_class_free(selectall_item_ic);

		return NULL;
	}
	item_ic->item_style = "1line";
	item_ic->func.text_get = __text_get_cb;
	item_ic->func.content_get = __content_get_cb;
	item_ic->func.state_get = NULL;
	item_ic->func.del = NULL;

	unsigned int item_count = m_browser->get_settings()->get_auto_fill_form_manager()->get_auto_fill_form_item_count();
	BROWSER_LOGD("%d ",item_count);

	if (item_count >0) {
		genlist_callback_data *each_item_callback_data = (genlist_callback_data *)malloc(sizeof(genlist_callback_data));
		if (!each_item_callback_data){
			if (selectall_item_ic) elm_genlist_item_class_free(selectall_item_ic);
			if (item_ic) elm_genlist_item_class_free(item_ic);
			return NULL;
		}
		memset(each_item_callback_data, 0x00, sizeof(genlist_callback_data));
		each_item_callback_data->type = DELETE_PROFILE_SELECT_ALL;
		each_item_callback_data->user_data = this;
		each_item_callback_data->it = elm_genlist_item_append(genlist, selectall_item_ic,
									each_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_selected_cb, m_select_all_callback_data);
		m_auto_fill_form_item_callback_data_list.push_back(each_item_callback_data);
	}
	for (unsigned int i = 0; i < item_count; i++) {
		genlist_callback_data *each_item_callback_data = (genlist_callback_data *)malloc(sizeof(genlist_callback_data));
		if (!each_item_callback_data)
			continue;

		memset(each_item_callback_data, 0x00, sizeof(genlist_callback_data));
		each_item_callback_data->type = DELETE_PROFILE_ITEM;
		each_item_callback_data->menu_index = i;
		each_item_callback_data->user_data = this;
		each_item_callback_data->it = elm_genlist_item_append(genlist, item_ic,
									each_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_selected_cb, m_item_callback_data);
		m_auto_fill_form_item_callback_data_list.push_back(each_item_callback_data);
	}

	if (selectall_item_ic) elm_genlist_item_class_free(selectall_item_ic);
	if (item_ic) elm_genlist_item_class_free(item_ic);
	m_del_genlist = genlist;
	evas_object_show(m_del_genlist);
	return m_del_genlist;
}

void auto_profile_delete_view::__genlist_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(item, EINA_FALSE);
	Evas_Object *checkbox = elm_object_item_part_content_get(item, "elm.icon.right");
	Eina_Bool state = elm_check_state_get(checkbox);
	elm_check_state_set(checkbox, !state);
	genlist_callback_data *gcd = (genlist_callback_data *)data;
	auto_profile_delete_view *apdv = (auto_profile_delete_view *)(gcd->user_data);
	if (gcd->type == DELETE_PROFILE_SELECT_ALL)
		__select_all_icon_clicked_cb((void *)apdv, NULL, NULL);
	else
		__checkbox_changed_cb((void *)apdv, NULL, NULL);
}

void auto_profile_delete_view::__delete_item_cb(void *data,Evas_Object *obj,void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	auto_profile_delete_view *apdv = (auto_profile_delete_view *)data;

	if (apdv->m_select_all_flag)
		apdv->_items_delete_all();
	else
		apdv->_item_delete();
}
void auto_profile_delete_view::_items_delete_all(void)
{
	BROWSER_LOGD("");

	m_browser->get_settings()->get_auto_fill_form_manager()->delete_all_auto_fill_form_items();
	_back_delete_view();
	m_browser->get_settings()->get_auto_fill_form_manager()->get_list_view()->refresh_view();
}

void auto_profile_delete_view::_item_delete(void)
{

	BROWSER_LOGD("");

	genlist_callback_data *cb_data = NULL;
	Elm_Object_Item *it = elm_genlist_first_item_get(m_del_genlist);
	it = elm_genlist_item_next_get(it); //To avoid the first "select All" option.
	Evas_Object *checkbox;
	int del_count =0;

	while (it) {
		checkbox = elm_object_item_part_content_get(it, "elm.icon.right");
		cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
		if (cb_data && cb_data->is_checked ) {
			auto_fill_form_item *item = m_browser->get_settings()->get_auto_fill_form_manager()->get_item_list()[cb_data->menu_index - del_count];
			m_browser->get_settings()->get_auto_fill_form_manager()->delete_auto_fill_form_item(item);
			del_count++;
		}
		it = elm_genlist_item_next_get(it);
	}
	BROWSER_LOGD("Total items deleted %d",del_count);
	_back_delete_view();
	m_browser->get_settings()->get_auto_fill_form_manager()->get_list_view()->refresh_view();
}

void auto_profile_delete_view::__auto_profile_delete_title_lang_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_profile_delete_view *apdv = (auto_profile_delete_view *)data;
	apdv->_set_selected_title();
}

#if defined(DELETE_CONFIRM_POPUP_ENABLE)
void auto_profile_delete_view::__popup_delete_item_cb(void *data, Evas_Object *obj, void *event_info)
{

	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_profile_delete_view *apdv = (auto_profile_delete_view *)data;
	if (apdv->m_delete_popup)
		evas_object_smart_callback_del(apdv->m_delete_popup, "language,changed", __auto_profile_delete_popup_lang_changed_cb);
	apdv->m_delete_popup = NULL;
	genlist_callback_data *cb_data = NULL;
	Elm_Object_Item *it = elm_genlist_first_item_get(apdv->m_del_genlist);
	Evas_Object *checkbox;
	int del_count =0;

	while (it) {
		checkbox = elm_object_item_part_content_get(it, "elm.icon.right");
		cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
		if (cb_data && elm_check_state_get(checkbox)) {
			auto_fill_form_item *item = m_browser->get_settings()->get_auto_fill_form_manager()->get_item_list()[cb_data->menu_index - del_count];
			m_browser->get_settings()->get_auto_fill_form_manager()->delete_auto_fill_form_item(item);
			del_count++;
		}
		it = elm_genlist_item_next_get(it);
	}
	BROWSER_LOGD("Total items deleted %d",del_count);
	apdv->_back_delete_view();
	m_browser->get_settings()->get_auto_fill_form_manager()->get_list_view()->refresh_view();
}

void auto_profile_delete_view::_set_popup_text()
{
	BROWSER_LOGD("");
	if ((m_count_checked_item > 1) && m_delete_popup) {
		char label_count[256] = {'\0', };
		char *text = NULL;
		int len ;
		snprintf(label_count, sizeof(label_count), "%d", m_count_checked_item);
		len = strlen(label_count) + strlen(BR_STRING_SETTINGS_PD_PROFILES_DELETED) - 1;
		text = (char *)malloc(len * sizeof(char));
		RET_MSG_IF(!text, "text is NULL");
		snprintf(text, len, BR_STRING_SETTINGS_PD_PROFILES_DELETED, m_count_checked_item);
		elm_object_text_set(m_delete_popup, text);

		free(text);
	}
}
#endif

void auto_profile_delete_view::_set_selected_title()
{
	BROWSER_LOGD("");
	m_sub_title.clear();
	char label_count[1024] = {'\0', };
	char *text = NULL;
	int len ;

	snprintf(label_count, sizeof(label_count), "%d", (m_count_checked_item));
	len = strlen(label_count) + strlen(BR_STRING_SELECTED) + 1;
	text = (char *)malloc(len * sizeof(char));

	if (!text)
		return ;

	memset(text, 0x00, len);
	snprintf(text, len-1, BR_STRING_SELECTED, m_count_checked_item);
	m_sub_title.append(text);
	elm_object_item_part_text_set(m_naviframe_item, "elm.text.title", m_sub_title.c_str());
	free(text);
}

void auto_profile_delete_view::_unset_selected_title()
{
	BROWSER_LOGD(" ");
	elm_object_item_part_text_set(m_naviframe_item, "elm.text.title", BR_STRING_DELETE);
}

void auto_profile_delete_view::__select_all_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_profile_delete_view *apdv = (auto_profile_delete_view *)data;
	Eina_Bool state = apdv->m_select_all_flag = !apdv->m_select_all_flag;

	Elm_Object_Item *it = elm_genlist_first_item_get(apdv->m_del_genlist);
	genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
	if (cb_data && cb_data->is_checked != state) {
		cb_data->is_checked = state;
		elm_genlist_item_update(it);
	}

	apdv->m_count_checked_item = 0;
	it = elm_genlist_item_next_get(it); //To avoid select all
	while (it) {
		genlist_callback_data *cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
		if (cb_data && cb_data->is_checked != state) {
			cb_data->is_checked = state;
			elm_genlist_item_update(it);
		}
		if (cb_data && cb_data->is_checked)
			apdv->m_count_checked_item++;
		it = elm_genlist_item_next_get(it);
	}
	apdv->_set_selected_title();
	if (apdv->m_select_all_flag)
		elm_object_disabled_set(apdv->m_button_done, EINA_FALSE);
	else
		elm_object_disabled_set(apdv->m_button_done, EINA_TRUE);

}

char *auto_profile_delete_view::__text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	auto_profile_delete_view *view = (auto_profile_delete_view *)callback_data->user_data;
	menu_type type = callback_data->type;

	if (type == DELETE_PROFILE_SELECT_ALL) {
		if (!strcmp(part, "elm.text.main"))
			return strdup(BR_STRING_SELECT_ALL);
	}
	if (type == DELETE_PROFILE_ITEM) {
		if (!strcmp(part, "elm.text.main.left")) {
			const char *item_full_name = view->_get_each_item_full_name((unsigned int)callback_data->menu_index);
			if (item_full_name)
				return strdup(item_full_name);
			else
				return NULL;
		}
	}
	return NULL;
}

const char *auto_profile_delete_view::_get_each_item_full_name(unsigned int index)
{
	BROWSER_LOGD("");
	if (m_browser->get_settings()->get_auto_fill_form_manager()->get_auto_fill_form_item_count() == 0)
		return NULL;
	return (m_browser->get_settings()->get_auto_fill_form_manager()->get_item_list())[index]->get_name();
}

Evas_Object *auto_profile_delete_view::__content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	Evas_Object *checkbox = NULL;
	genlist_callback_data *callback_data = (genlist_callback_data *)data;

	if (!strcmp(part, "elm.icon")) {
		checkbox = elm_check_add(obj);
		if (!checkbox) {
			BROWSER_LOGE("Failed to add check");
			return NULL;
		}
		elm_check_state_pointer_set(checkbox, &(callback_data->is_checked));
		if (callback_data->type == DELETE_PROFILE_SELECT_ALL)
			evas_object_smart_callback_add(checkbox, "changed", __select_all_icon_clicked_cb, callback_data->user_data);
		evas_object_propagate_events_set(checkbox, EINA_FALSE);
		return checkbox;
	}
	if (!strcmp(part, "elm.icon.right")) {
		checkbox = elm_check_add(obj);
		if (!checkbox) {
			BROWSER_LOGE("Failed to add check");
			return NULL;
		}
		elm_check_state_pointer_set(checkbox, &(callback_data->is_checked));
		evas_object_smart_callback_add(checkbox, "changed", __checkbox_changed_cb, callback_data->user_data);
		evas_object_propagate_events_set(checkbox, EINA_FALSE);
	}
	return checkbox;
}

void auto_profile_delete_view::_remove_each_item_callback_data(void)
{
	BROWSER_LOGD("");

	free(m_select_all_callback_data);
	m_select_all_callback_data = NULL;
	free(m_item_callback_data);
	m_item_callback_data = NULL;

	for (unsigned int i = 0; i < m_auto_fill_form_item_callback_data_list.size(); i++) {
		free(m_auto_fill_form_item_callback_data_list[i]);
		m_auto_fill_form_item_callback_data_list[i] = NULL;
	}
}

Eina_Bool auto_profile_delete_view::__naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, EINA_FALSE, "data is NULL");
	auto_profile_delete_view *view = (auto_profile_delete_view *)data;
	view->m_select_all_flag = EINA_FALSE;

	return EINA_TRUE;
}

void auto_profile_delete_view::_back_delete_view(void)
{
	BROWSER_LOGD("");

	if (m_naviframe_item == elm_naviframe_top_item_get(m_naviframe)) {
		elm_naviframe_item_pop(m_naviframe);
		m_select_all_layout = NULL;
	}
}

void auto_profile_delete_view::__cancel_button_click_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	__naviframe_pop_finished_cb(data, obj, event_info);

}

void auto_fill_form_list_view::__change_autofillform_check_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_object_item_signal_emit(it, "play,touch_sound,signal", "");

	Eina_Bool auto_fill_form_enalbed;
	genlist_callback_data *cb = (genlist_callback_data *)data;
	auto_fill_form_list_view *view = (auto_fill_form_list_view *)cb->user_data;

	elm_genlist_item_selected_set(cb->it, EINA_FALSE);
	auto_fill_form_enalbed = m_preference->get_auto_fill_forms_enabled();
	elm_check_state_set(view->m_auto_fill_form_check, !auto_fill_form_enalbed);
	__check_changed_cb(view, obj, event_info);
}

void auto_profile_delete_view::__naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	auto_profile_delete_view *view = (auto_profile_delete_view *)data;

	if (view->m_naviframe_item == elm_naviframe_top_item_get(m_naviframe)) {
		if (view->m_del_genlist)
			evas_object_smart_callback_del(view->m_del_genlist, "language,changed", view->__auto_profile_delete_title_lang_changed_cb);
		elm_naviframe_item_pop(m_naviframe);
	}
	m_browser->get_settings()->get_auto_fill_form_manager()->get_list_view()->refresh_view();
}

Eina_Bool auto_fill_form_list_view::_is_in_list_view(void)
{
	BROWSER_LOGD("");
	if (m_navi_it == elm_naviframe_top_item_get(m_naviframe))
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

void auto_fill_form_list_view::__delete_item_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	auto_fill_form_list_view *afflv = (auto_fill_form_list_view*)data;
	afflv->__ctxpopup_dismissed_cb(data, obj, event_info);
	afflv->_get_delete_view()->show();
}

void auto_fill_form_list_view::__add_item_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	auto_fill_form_list_view *afflv = (auto_fill_form_list_view*)data;
	afflv->__ctxpopup_dismissed_cb(data, obj, event_info);
	m_browser->get_settings()->get_auto_fill_form_manager()->show_composer();
}

void auto_fill_form_list_view::__more_menu_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	auto_fill_form_list_view *afflv = (auto_fill_form_list_view *)data;
	afflv->_show_more_context_popup();
}

void auto_fill_form_list_view::__back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	auto_fill_form_list_view *afflv = (auto_fill_form_list_view *)data;
	if (afflv->m_navi_it != elm_naviframe_top_item_get(m_naviframe))
		return;

	elm_naviframe_item_pop(m_naviframe);
}

void auto_fill_form_list_view::__context_popup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	eext_ctxpopup_back_cb(data, obj, event_info);
}

#if !defined(SPLIT_WINDOW)
Eina_Bool auto_fill_form_list_view::__resize_more_ctxpopup(void *data)
{
	auto_fill_form_list_view *afflv = (auto_fill_form_list_view *)data;
	if (!afflv->m_ctx_popup_more_menu)
		return ECORE_CALLBACK_CANCEL;

	// Move ctxpopup to proper position.
	int x = 0;
	int y = 0;
	platform_service ps;
	ps.get_more_ctxpopup_position(&x, &y);
	evas_object_move(afflv->m_ctx_popup_more_menu, x, y);

	return ECORE_CALLBACK_CANCEL;
}

void auto_fill_form_list_view::__rotate_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	__resize_more_ctxpopup(data);
}
#endif

void auto_fill_form_list_view::__ctxpopup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	auto_fill_form_list_view *cp = (auto_fill_form_list_view *)data;

#if !defined(SPLIT_WINDOW)
	evas_object_smart_callback_del(elm_object_top_widget_get(cp->m_ctx_popup_more_menu), "rotation,changed", __rotate_ctxpopup_cb);
#endif
	if (obj)
		evas_object_del(obj);
	cp->m_ctx_popup_more_menu = NULL;
}

void auto_fill_form_list_view::_show_more_context_popup(void)
{
	BROWSER_LOGD("");

	if (!_is_in_list_view())
		return;
#if defined(SPLIT_WINDOW)
	Evas_Object *more_popup = ea_menu_popup_add(m_window);
#else
	Evas_Object *more_popup = elm_ctxpopup_add(m_window);
#endif
	if (!more_popup) {
		BROWSER_LOGE("elm_ctxpopup_add failed");
		return;
	}

	elm_object_style_set(more_popup, "more/default");
	elm_ctxpopup_direction_priority_set(more_popup, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);
	evas_object_size_hint_weight_set(more_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(more_popup, "dismissed", __ctxpopup_dismissed_cb, this);
	elm_ctxpopup_auto_hide_disabled_set(more_popup, EINA_TRUE);
	m_ctx_popup_more_menu = more_popup;
#if defined(HW_MORE_BACK_KEY)
	// Add HW key callback.
	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_BACK, __context_popup_back_cb, this);
	eext_object_event_callback_add(more_popup, EEXT_CALLBACK_MORE, __context_popup_back_cb, this);
#endif
#if !defined(SPLIT_WINDOW)
	evas_object_smart_callback_add(elm_object_top_widget_get(more_popup), "rotation,changed", __rotate_ctxpopup_cb, this);
#endif

	brui_ctxpopup_item_append(more_popup, BR_STRING_ADD,
			__add_item_cb, NULL, NULL, this);
	if (m_browser->get_settings()->get_auto_fill_form_manager()->get_auto_fill_form_item_count() >= 1)
		brui_ctxpopup_item_append(more_popup, BR_STRING_DELETE, __delete_item_cb, NULL, NULL, this);

#if defined(SPLIT_WINDOW)
	ea_menu_popup_move(more_popup);
#else
	__resize_more_ctxpopup(this);
#endif

	evas_object_show(more_popup);
}

Eina_Bool auto_fill_form_list_view::show(void)
{
	BROWSER_LOGD("");

	m_main_layout = _create_main_layout(m_naviframe);
	if (!m_main_layout) {
		BROWSER_LOGE("_create_main_layout failed");
		return EINA_FALSE;
	}

	m_navi_it = elm_naviframe_item_push(m_naviframe, "IDS_BR_MBODY_AUTO_FILL_FORMS", NULL, NULL, m_main_layout, NULL);//BR_STRING_SETTINGS_AUTO_FILL_FORMS
	elm_object_item_domain_text_translatable_set(m_navi_it, BROWSER_DOMAIN_NAME, EINA_TRUE);
	Evas_Object *check_box = elm_check_add(m_naviframe);
	RETV_MSG_IF(!check_box, EINA_FALSE, "Failed to add checkbox");

	elm_object_style_set(check_box, "naviframe/title_on&off");
	evas_object_propagate_events_set(check_box, EINA_FALSE);
	evas_object_smart_callback_add(check_box, "changed", __check_changed_cb, this);
	elm_object_item_part_content_set(m_navi_it, "title_right_btn", check_box);
	elm_access_info_set(check_box, ELM_ACCESS_INFO, BR_STRING_SETTINGS_AUTO_FILL_FORMS);

#if defined(HW_MORE_BACK_KEY)
		// Add HW key callback.
	eext_object_event_callback_add(m_main_layout, EEXT_CALLBACK_MORE, __more_menu_cb, this);
	eext_object_event_callback_add(m_main_layout, EEXT_CALLBACK_BACK, __back_cb, this);
#endif
	Eina_Bool auto_fill_form_enalbed = m_preference->get_auto_fill_forms_enabled();
	elm_check_state_set(check_box, auto_fill_form_enalbed);
	m_auto_fill_form_check = check_box;

	Evas_Object *btn = elm_button_add(m_naviframe);
	elm_object_style_set(btn, "bottom");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(btn, "clicked", __add_profile_button_cb, this);
	elm_object_item_part_content_set(m_navi_it, "toolbar", btn);
	elm_object_domain_translatable_part_text_set(btn, NULL, BROWSER_DOMAIN_NAME, "IDS_BR_MBODY_ADD_PROFILE");

	elm_naviframe_item_pop_cb_set(m_navi_it, __naviframe_pop_cb, this);
	evas_object_smart_callback_add(m_naviframe, "transition,finished", __naviframe_pop_finished_cb, this);

	return EINA_TRUE;
}

Evas_Object *auto_fill_form_list_view::refresh_view(void)
{
	BROWSER_LOGD("");
	elm_genlist_clear(m_genlist);

	if (_append_genlist(m_genlist) == EINA_FALSE) {
		_destroy_all_genlist_style_items();
		_remove_each_item_callback_data();
		return NULL;
	}

	return m_genlist;
}

list_view_item_clicked_popup *auto_fill_form_list_view::get_list_item_selected_popup(auto_fill_form_item *item)
{
	delete_list_item_selected_popup();

	if (item)
		m_select_popup = new list_view_item_clicked_popup(item);

	return m_select_popup;
}

void auto_fill_form_list_view::delete_list_item_selected_popup(void)
{
	BROWSER_LOGD("");
	if (m_select_popup)
		delete m_select_popup;
	m_select_popup = NULL;

	if (m_ctx_popup_more_menu)
		evas_object_del(m_ctx_popup_more_menu);
	m_ctx_popup_more_menu = NULL;
}

Evas_Object *auto_fill_form_list_view::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	m_genlist = _create_genlist(parent);
	if (!m_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}

	return m_genlist;
}

Evas_Object *auto_fill_form_list_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}

	if (_create_genlist_style_items() == EINA_FALSE) {
		BROWSER_LOGE("_create_genlist_style_items failed");
		return NULL;
	}

	if (_append_genlist(genlist) == EINA_FALSE) {
		_destroy_all_genlist_style_items();
		_remove_each_item_callback_data();
		return NULL;
	}
	evas_object_smart_callback_add(genlist, "realized", __genlist_item_realized_cb, this);
	evas_object_smart_callback_add(genlist, "language,changed", common_view::__genlist_lang_changed, NULL);

	return genlist;
}

Eina_Bool auto_fill_form_list_view::_create_genlist_style_items(void)
{
	BROWSER_LOGD("");
	Elm_Genlist_Item_Class *description_item_class = elm_genlist_item_class_new();
	if (!description_item_class) {
		BROWSER_LOGE("elm_genlist_item_class_new for description_item_class failed");
		return EINA_FALSE;
	}
	description_item_class->item_style = "multiline/1text";
	description_item_class->func.content_get = __content_get_cb;

	description_item_class->func.text_get = __text_get_cb;
	description_item_class->func.state_get = NULL;
	description_item_class->func.del = NULL;
	m_multiline_1text_item_class = description_item_class;

	Elm_Genlist_Item_Class *title_item_class = elm_genlist_item_class_new();
	if (!title_item_class) {
		BROWSER_LOGE("elm_genlist_item_class_new for title_item_class failed");
		return EINA_FALSE;
	}
	title_item_class->item_style = "groupindex";

	title_item_class->func.text_get = __text_get_cb;
	title_item_class->func.content_get = NULL;
	title_item_class->func.state_get = NULL;
	title_item_class->func.del = NULL;
	m_dialogue_group_title_item_class = title_item_class;

	Elm_Genlist_Item_Class *each_form_data_menu_item_class = elm_genlist_item_class_new();
	if (!each_form_data_menu_item_class) {
		BROWSER_LOGE("elm_genlist_item_class_new for each_form_data_menu_item_class failed");
		return EINA_FALSE;
	}

	each_form_data_menu_item_class->item_style = "1line";
	each_form_data_menu_item_class->func.content_get = NULL;

	each_form_data_menu_item_class->decorate_all_item_style = "edit_default";
	each_form_data_menu_item_class->func.text_get = __text_get_cb;
	each_form_data_menu_item_class->func.state_get = NULL;
	each_form_data_menu_item_class->func.del = NULL;
	m_dialogue_1text_item_class = each_form_data_menu_item_class;

	Elm_Genlist_Item_Class *dialogue_bottom_item_class = elm_genlist_item_class_new();
	if (!dialogue_bottom_item_class) {
		BROWSER_LOGE("elm_genlist_item_class_new for dialogue_bottom_item_class failed");
		return EINA_FALSE;
	}
	dialogue_bottom_item_class->item_style = "separator";
	dialogue_bottom_item_class->func.text_get = NULL;
	dialogue_bottom_item_class->func.content_get = NULL;
	dialogue_bottom_item_class->func.state_get = NULL;
	dialogue_bottom_item_class->func.del = NULL;
	m_dialogue_bottom_item_class = dialogue_bottom_item_class;

	return EINA_TRUE;
}

Eina_Bool auto_fill_form_list_view::_destroy_all_genlist_style_items(void)
{
	BROWSER_LOGD("");

	if (m_multiline_1text_item_class)
		elm_genlist_item_class_free(m_multiline_1text_item_class);
	m_multiline_1text_item_class = NULL;

	if (m_dialogue_group_title_item_class)
		elm_genlist_item_class_free(m_dialogue_group_title_item_class);
	m_dialogue_group_title_item_class = NULL;

	if (m_dialogue_1text_item_class)
		elm_genlist_item_class_free(m_dialogue_1text_item_class);
	m_dialogue_1text_item_class = NULL;

	if (m_dialogue_bottom_item_class)
		elm_genlist_item_class_free(m_dialogue_bottom_item_class);
	m_dialogue_bottom_item_class = NULL;

	return EINA_TRUE;
}

const char *auto_fill_form_list_view::_get_each_item_full_name(unsigned int index)
{
	if (m_manager->get_auto_fill_form_item_count() == 0)
		return NULL;
	return (m_manager->get_item_list())[index]->get_name();
}

Eina_Bool auto_fill_form_list_view::_append_genlist(Evas_Object *genlist)
{
	BROWSER_LOGD("");

	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_realization_mode_set(genlist, EINA_TRUE);

	m_description_top_callback_data.type = aufo_fill_form_description;
	m_description_top_callback_data.user_data = this;
	m_description_top_callback_data.it = elm_genlist_item_append(genlist, m_dialogue_bottom_item_class,
						&m_description_top_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(m_description_top_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);


	m_description_callback_data.it = elm_genlist_item_append(genlist, m_multiline_1text_item_class,
						&m_description_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(m_description_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	unsigned int item_count = m_manager->get_auto_fill_form_item_count();
	if (item_count > 0) {
		m_bottom_pad_item_callback_data.type = aufo_fill_form_bottom;
		m_bottom_pad_item_callback_data.user_data = this;
		m_bottom_pad_item_callback_data.it = elm_genlist_item_append(genlist, m_dialogue_bottom_item_class,
							&m_bottom_pad_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							NULL, &m_bottom_pad_item_callback_data);
		elm_genlist_item_select_mode_set(m_bottom_pad_item_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

		m_auto_fill_forms_profile_title_item_callback_data.type = auto_fill_profile_title;
		m_auto_fill_forms_profile_title_item_callback_data.user_data = this;
		m_auto_fill_forms_profile_title_item_callback_data.it = elm_genlist_item_append(genlist, m_dialogue_group_title_item_class,
							&m_auto_fill_forms_profile_title_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(m_auto_fill_forms_profile_title_item_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

		if (m_auto_fill_form_item_callback_data_list.size())
			_remove_each_item_callback_data();

		for (unsigned int i = 0; i < item_count; i++) {
			profile_item_genlist_callback_data *each_item_callback_data = (profile_item_genlist_callback_data *)malloc(sizeof(profile_item_genlist_callback_data));
			if (!each_item_callback_data)
				continue;

			memset(each_item_callback_data, 0x00, sizeof(profile_item_genlist_callback_data));
			each_item_callback_data->cd.type = auto_fill_profile_each_item;
			each_item_callback_data->cd.menu_index = i;
			each_item_callback_data->cd.user_data = this;

			each_item_callback_data->cd.it = elm_genlist_item_append(genlist, m_dialogue_1text_item_class,
								each_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
								__genlist_item_clicked_cb, each_item_callback_data);

			m_auto_fill_form_item_callback_data_list.push_back(each_item_callback_data);
		}

		m_bottom_pad_item_callback_data.type = aufo_fill_form_bottom;
		m_bottom_pad_item_callback_data.user_data = this;
		m_bottom_pad_item_callback_data.it = elm_genlist_item_append(genlist, m_dialogue_bottom_item_class,
							&m_bottom_pad_item_callback_data, NULL, ELM_GENLIST_ITEM_NONE,
							NULL, &m_bottom_pad_item_callback_data);

		elm_genlist_item_select_mode_set(m_bottom_pad_item_callback_data.it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	return EINA_TRUE;
}

void auto_fill_form_list_view::_remove_each_item_callback_data(void)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_auto_fill_form_item_callback_data_list.size(); i++) {
		free(m_auto_fill_form_item_callback_data_list[i]);
		m_auto_fill_form_item_callback_data_list[i] = NULL;
	}
}

char *auto_fill_form_list_view::__text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	auto_fill_form_list_view *view = (auto_fill_form_list_view *)callback_data->user_data;
	auto_fill_form_list_view::menu_type type = callback_data->type;

	if (type == aufo_fill_form_description_top) {
		if (!strcmp(part, "elm.text.main.left"))
			return strdup("");
	} else if (type == aufo_fill_form_description) {

		if (!strcmp(part, "elm.text.main"))
			return strdup(BR_STRING_SETTINGS_AUTO_FILL_FORMS);
		else if (!strcmp(part, "elm.text.multiline"))
			return strdup(BR_STRING_AUTO_FILL_DESC);
	}
	else if (type == auto_fill_profile_title) {
		if (!strcmp(part, "elm.text.main")) {
			if (view->m_manager->get_auto_fill_form_item_count() == 0)
				return NULL;
			else if (view->m_manager->get_auto_fill_form_item_count() == 1)
				return strdup(BR_STRING_PROFILE);
			else
				return strdup(BR_STRING_PROFILES);
		}
	} else if (type == auto_fill_profile_each_item) {
		if (!strcmp(part, "elm.text.main.left")) {
			const char *item_full_name = view->_get_each_item_full_name((unsigned int)callback_data->menu_index);
			if (item_full_name)
				return strdup(item_full_name);
			else
				return NULL;
		}
	} else
		return NULL;

	return NULL;
}

Evas_Object *auto_fill_form_list_view::__content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");
	Evas_Object *checkbox = NULL;
	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	auto_fill_form_list_view *view = (auto_fill_form_list_view *)callback_data->user_data;
	if (!strcmp(part, "elm.icon")) {
		checkbox = elm_check_add(obj);
		if (!checkbox) {
				BROWSER_LOGE("Failed to add clear_history_check");
				return NULL;
		}
		elm_object_style_set(checkbox, "on&off");
		evas_object_propagate_events_set(checkbox, EINA_FALSE);
		evas_object_smart_callback_add(checkbox, "changed", __check_changed_cb, callback_data->user_data);
		Eina_Bool auto_fill_form_enalbed = m_preference->get_auto_fill_forms_enabled();
		elm_check_state_set(checkbox, auto_fill_form_enalbed);
		view->m_auto_fill_form_check = checkbox;
	}
	return checkbox;
}

void auto_fill_form_list_view::__check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_list_view *view = (auto_fill_form_list_view *)data;

	Eina_Bool state = elm_check_state_get(view->m_auto_fill_form_check);
	m_preference->set_auto_fill_forms_enabled(state);

	if (m_browser->get_settings()->is_shown())
		m_browser->get_settings()->realize_genlist();
}

void auto_fill_form_list_view::__add_profile_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	/* create new profile */
	auto_fill_form_list_view *list_view = (auto_fill_form_list_view *)data;
	list_view->m_manager->show_composer();
}

void auto_fill_form_list_view::__delete_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

//	auto_fill_form_list_view *view = (auto_fill_form_list_view *)data;
}

void auto_fill_form_list_view::__back_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	elm_naviframe_item_pop(m_naviframe);
}

void auto_fill_form_list_view::__genlist_auto_fill_form_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	auto_fill_form_list_view *view = (auto_fill_form_list_view *)(callback_data->user_data);

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);

	Eina_Bool state = elm_check_state_get(view->m_auto_fill_form_check);
	elm_check_state_set(view->m_auto_fill_form_check, !state);
	m_preference->set_auto_fill_forms_enabled(!state);

	if (m_browser->get_settings()->is_shown())
		m_browser->get_settings()->realize_genlist();
}

void auto_fill_form_list_view::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	profile_item_genlist_callback_data *callback_data = (profile_item_genlist_callback_data *)data;
	auto_fill_form_list_view *view = (auto_fill_form_list_view *)(callback_data->cd.user_data);

	elm_genlist_item_selected_set(callback_data->cd.it, EINA_FALSE);
	auto_fill_form_manager *manager = m_browser->get_settings()->get_auto_fill_form_manager();
	RET_MSG_IF(!manager, "auto_fill_form_manager is NULL");

	manager->show_composer((view->m_manager->get_item_list())[callback_data->cd.menu_index]);
}

void auto_fill_form_list_view::__popup_edit_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_item *item = (auto_fill_form_item *)data;

	m_browser->get_browser_view()->destroy_popup(obj);
	m_browser->get_settings()->get_auto_fill_form_manager()->show_composer(item);
}

void auto_fill_form_list_view::__popup_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_item *item = (auto_fill_form_item *)data;

	m_browser->get_browser_view()->destroy_popup(obj);
	m_browser->get_settings()->get_auto_fill_form_manager()->delete_auto_fill_form_item(item);
}

Eina_Bool auto_fill_form_list_view::__naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, EINA_FALSE, "data is NULL");

//	auto_fill_form_list_view *view = (auto_fill_form_list_view *)data;

	return EINA_TRUE;
}

void auto_fill_form_list_view::__naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	auto_fill_form_list_view *view = (auto_fill_form_list_view *)data;

	if (view->m_navi_it == elm_naviframe_top_item_get(m_naviframe))
		m_browser->get_settings()->get_auto_fill_form_manager()->delete_composer();
}

void auto_fill_form_list_view::__genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	RET_MSG_IF(!event_info, "event_info is NULL");

	auto_fill_form_list_view *psView = (auto_fill_form_list_view *)(data);
	Elm_Object_Item *selected_item = (Elm_Object_Item *)(event_info);
	profile_item_genlist_callback_data *callback_data = (profile_item_genlist_callback_data *)elm_object_item_data_get(selected_item);
	menu_type type = callback_data->cd.type;
	BROWSER_LOGD("type[%d]", type);

	profile_item_genlist_callback_data *cb_data = (profile_item_genlist_callback_data *)elm_object_item_data_get(selected_item);
	RET_MSG_IF(!cb_data, "cb_data is NULL");
	unsigned int iItemCount = psView->m_manager->get_auto_fill_form_item_count();

	if (elm_genlist_item_select_mode_get(selected_item) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
		Elm_Object_Item *next_it = NULL;
		Elm_Object_Item *prev_it = NULL;
		next_it = elm_genlist_item_next_get(selected_item);
		prev_it = elm_genlist_item_prev_get(selected_item);

		if ((prev_it && elm_genlist_item_select_mode_get(prev_it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
		    && (!next_it && (iItemCount > 1))) {
			elm_object_item_signal_emit(selected_item, "elm,state,top", "");
			return;
		}
		if ((prev_it && elm_genlist_item_select_mode_get(prev_it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
		    && (next_it && elm_genlist_item_select_mode_get(next_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)) {
			elm_object_item_signal_emit(selected_item, "elm,state,top", "");
			return;
		}
		if (next_it && prev_it) {
			if ((elm_genlist_item_select_mode_get(next_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
			    && (elm_genlist_item_select_mode_get(prev_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)) {
				elm_object_item_signal_emit(selected_item, "elm,state,center", "");
				return;
			}
		}
		if ((prev_it && elm_genlist_item_select_mode_get(prev_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
		    && (next_it && elm_genlist_item_select_mode_get(next_it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)) {
			elm_object_item_signal_emit(selected_item, "elm,state,bottom", "");
			return;
		}
		if ((prev_it && elm_genlist_item_select_mode_get(prev_it) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
		    && !next_it) {
			elm_object_item_signal_emit(selected_item, "elm,state,bottom", "");
			return;
		}
	}

	elm_object_item_signal_emit(selected_item, "elm,state,normal", "");
}



auto_fill_form_compose_view::auto_fill_form_compose_view(auto_fill_form_item *item)
:
	m_item_for_compose(NULL),
	m_main_layout(NULL),
	m_genlist(NULL),
	m_contents_layout(NULL),
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
	m_navi_it(NULL),
	m_delay_naviframe_pop_idler(NULL),
	m_edit_errorcode(update_error_none),
	m_save_errorcode(save_error_none),
	m_edit_field_item_class(NULL)
{
	BROWSER_LOGD("");

	if (item && item->get_item_compose_mode() == profile_edit)
		m_item_for_compose = item;
	else
		m_item_for_compose = new auto_fill_form_item(NULL);

	m_entry_limit_size.max_char_count = 0;
	m_entry_limit_size.max_byte_count = AUTO_FILL_FORM_ENTRY_MAX_COUNT;
}

auto_fill_form_compose_view::~auto_fill_form_compose_view(void)
{
	BROWSER_LOGD("");

	_destroy_all_genlist_style_items();

	if (m_item_for_compose && m_item_for_compose->get_item_compose_mode() == profile_create)
		delete m_item_for_compose;
	m_item_for_compose = NULL;
	if (m_delay_naviframe_pop_idler)
		ecore_idler_del(m_delay_naviframe_pop_idler);
	evas_object_smart_callback_del(m_naviframe, "transition,finished", __naviframe_pop_finished_cb);
}

Evas_Object *auto_fill_form_compose_view::_create_scroller(Evas_Object * parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");
	Evas_Object *scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);
	return scroller;
}

Evas_Object *auto_fill_form_compose_view::_create_box(Evas_Object * parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");
	Evas_Object *box = NULL;
	box = elm_box_add(parent);
	RETV_MSG_IF(!box, NULL, "box is NULL");
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(box);
	return box;
}

void auto_fill_form_compose_view::_create_list_items(Evas_Object * parent)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!parent, "parent is NULL");

	m_contents_layout = elm_layout_add(parent);
	elm_layout_file_set(m_contents_layout, settings_edj_path, "contents");
	elm_object_focus_set(m_contents_layout, EINA_FALSE);

	evas_object_size_hint_weight_set(m_contents_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_contents_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(m_contents_layout);

	elm_box_pack_end(m_box, m_contents_layout);

	m_genlist = _create_genlist(parent);
	evas_object_show(m_genlist);
	elm_layout_content_set(m_contents_layout, "elm.swallow.content", m_genlist);
	evas_object_show(m_contents_layout);
}

void auto_fill_form_compose_view::show(void)
{
	BROWSER_LOGD("");

	m_main_layout = elm_layout_add(m_naviframe);
	RET_MSG_IF(!m_main_layout, "m_main_layout is NULL");
	elm_layout_theme_set(m_main_layout, "layout", "application", "noindicator");
	evas_object_size_hint_weight_set(m_main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_scroller = _create_scroller(m_main_layout);
	RET_MSG_IF(!m_scroller, "m_scroller is NULL");
	elm_object_part_content_set(m_main_layout, "elm.swallow.content", m_scroller);

	m_box = _create_box(m_scroller);
	RET_MSG_IF(!m_box, "m_box is NULL");
	elm_object_content_set(m_scroller, m_box);
	_create_list_items(m_box);

	if (m_item_for_compose->get_item_compose_mode() == profile_edit)
		m_navi_it = elm_naviframe_item_push(m_naviframe, "IDS_BR_HEADER_EDIT_INFO_ABB", NULL, NULL, m_main_layout, NULL);
	else
		m_navi_it = elm_naviframe_item_push(m_naviframe, "IDS_BR_HEADER_ADD_INFO_ABB", NULL, NULL, m_main_layout, NULL);

	elm_object_item_domain_text_translatable_set(m_navi_it, BROWSER_DOMAIN_NAME, EINA_TRUE);

	m_cancel_button = elm_button_add(m_naviframe);
	if (!m_cancel_button) {
		BROWSER_LOGE("Failed to create m_cancel_button");
		return;
	}
	elm_object_style_set(m_cancel_button, "naviframe/title_cancel");
	evas_object_smart_callback_add(m_cancel_button, "clicked", __cancel_button_cb, this);
	elm_object_item_part_content_set(m_navi_it, "title_left_btn", m_cancel_button);

	m_done_button = elm_button_add(m_naviframe);
	if (!m_done_button) {
		BROWSER_LOGE("Failed to create m_done_button");
		return;
	}
	elm_object_style_set(m_done_button, "naviframe/title_done");
	evas_object_smart_callback_add(m_done_button, "clicked", __done_button_cb, this);
	elm_object_item_part_content_set(m_navi_it, "title_right_btn", m_done_button);
	elm_naviframe_item_pop_cb_set(m_navi_it, __naviframe_pop_cb, this);

	if (m_item_for_compose->get_item_compose_mode() == profile_create)
		elm_object_disabled_set(m_done_button, EINA_TRUE);
	evas_object_smart_callback_add(m_naviframe, "transition,finished", __naviframe_pop_finished_cb, this);
}

void auto_fill_form_compose_view::__genlist_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	RET_MSG_IF(!event_info, "event_info is NULL");
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;

	genlist_callback_data *item_cb_data = (genlist_callback_data *)elm_object_item_data_get(it);
	if (item_cb_data->type == profile_composer_title_email) {
		elm_object_item_signal_emit(it, "elm,state,bottom", "");
		elm_object_item_access_unregister(it);
	} else {
		elm_object_item_signal_emit(it, "elm,state,top", "");
		elm_object_item_access_unregister(it);
	}
}

Evas_Object *auto_fill_form_compose_view::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!parent, NULL, "parent is NULL");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	evas_object_smart_callback_add(genlist, "realized", __genlist_realized_cb, NULL);
	evas_object_smart_callback_add(genlist, "language,changed", __genlist_lang_changed, NULL);
	elm_genlist_realization_mode_set(genlist, EINA_TRUE);

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
	BROWSER_LOGD("");

	Elm_Genlist_Item_Class *edit_field_item_class = elm_genlist_item_class_new();
	if (!edit_field_item_class) {
		BROWSER_LOGE("elm_genlist_item_class_new for edit_field_item_class failed");
		return EINA_FALSE;
	}
	edit_field_item_class->item_style = "entry.main";
	edit_field_item_class->func.text_get = __text_get_cb;
	edit_field_item_class->func.content_get = __content_get_cb;
	m_edit_field_item_class = edit_field_item_class;

	return EINA_TRUE;
}

Eina_Bool auto_fill_form_compose_view::_destroy_all_genlist_style_items(void)
{
	BROWSER_LOGD("");

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
	BROWSER_LOGD("");

	const char *full_name = elm_entry_entry_get(m_entry_full_name);

	if (!full_name)
		return EINA_FALSE;

	std::string full_name_str = std::string(full_name);
	full_name_str = _trim(full_name_str);
	full_name = full_name_str.c_str();

	if (full_name && strlen(full_name) && !is_entry_has_only_space(full_name))
		m_item_for_compose->set_name(full_name);
	else {
		show_msg_popup(BR_STRING_ENTER_NAME, 2, __timer_popup_expired_cb, this);
		elm_object_focus_set(m_cancel_button, EINA_TRUE); // Closing virtual keyboard by changing the focus
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
			m_browser->get_settings()->get_auto_fill_form_manager()->add_item_to_list(m_item_for_compose);
	}
	m_browser->get_settings()->get_auto_fill_form_manager()->load_entire_item_list();

	return EINA_TRUE;
}

char *auto_fill_form_compose_view::__text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	genlist_callback_data *cb_data = (genlist_callback_data *)data;
	auto_fill_form_compose_view::menu_type type = cb_data->type;

	if (!strcmp(part, "elm.text.main")) {
		if (type == profile_composer_title_full_name)
			return strdup(BR_STRING_AUTO_FILL_DATA_FULL_NAME);
		else if (type == profile_composer_title_company_name)
			return strdup(BR_STRING_AUTO_FILL_DATA_COMPANY_NAME);
		else if (type == profile_composer_title_address_line_1)
			return strdup(BR_STRING_AUTO_FILL_DATA_ADDRESS_LINE_1);
		else if (type == profile_composer_title_address_line_2)
			return strdup(BR_STRING_AUTO_FILL_DATA_ADDRESS_LINE_2);
		else if (type == profile_composer_title_city_town)
			return strdup(BR_STRING_TOWN_AUTO_FILL_CITY_COUNTY);
		else if (type == profile_composer_title_county_region)
			return strdup(BR_STRING_AUTO_FILL_DATA_COUNTRY_REGION);
		else if (type == profile_composer_title_post_code)
			return strdup(BR_STRING_AUTO_FILL_DATA_POST_CODE);
		else if (type == profile_composer_title_country)
			return strdup(BR_STRING_AUTO_FILL_DATA_COUNTRY);
		else if (type == profile_composer_title_phone)
			return strdup(BR_STRING_AUTO_FILL_DATA_PHONE);
		else if (type == profile_composer_title_email)
			return strdup(BR_STRING_AUTO_FILL_DATA_EMAIL);
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

	if (!strcmp(part, "elm.icon.entry")) {
		Evas_Object *editfield = elm_layout_add(obj);
		elm_layout_theme_set(editfield, "layout", "editfield", "singleline");
		evas_object_size_hint_align_set(editfield, EVAS_HINT_FILL, 0.0);
		evas_object_size_hint_weight_set(editfield, EVAS_HINT_EXPAND, 0.0);

		Evas_Object *entry = elm_entry_add(editfield);
		elm_entry_single_line_set(entry, EINA_TRUE);
		elm_entry_scrollable_set(entry, EINA_TRUE);
		elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);

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

		Evas_Object *bt_clear = elm_object_part_content_get(entry, "elm.swallow.clear");
		elm_access_info_set(bt_clear, ELM_ACCESS_INFO, BR_STRING_CLEAR_ALL);

		if (type == profile_composer_title_full_name) {
			if (view->m_item_for_compose->get_name() && strlen(view->m_item_for_compose->get_name()))
				elm_object_part_text_set(entry, "elm.text", view->m_item_for_compose->get_name());
			view->m_entry_full_name = cb_data->entry = entry;
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

		elm_object_part_content_set(editfield, "elm.swallow.content", entry);

		Evas_Object *button = elm_button_add(editfield);
		elm_object_style_set(button, "editfield_clear");
		evas_object_smart_callback_add(button, "clicked", __editfield_clear_button_clicked_cb, entry);
		elm_object_part_content_set(editfield, "elm.swallow.button", button);

		if (!elm_entry_is_empty(entry)) {
			BROWSER_LOGE("entry is empty");
			elm_object_signal_emit(editfield, "elm,action,show,button", "");
		}

		return editfield;
	}

	return NULL;
}

void auto_fill_form_compose_view::_show_error_popup(int error_code)
{
	BROWSER_LOGD("");
	browser_view *bv = m_browser->get_browser_view();
	int timer_val = 2;
	if (!bv)
		return;

	switch(error_code) {
		case profile_create_failed:
			bv->show_msg_popup(BR_STRING_FAILED, timer_val);
		break;
		case duplicate_profile:
			bv->show_msg_popup(BR_STRING_ALREADY_EXISTS, timer_val);
		break;
		default:
			BROWSER_LOGD("NO ERROR CODE MATCHED");
	}
}

void auto_fill_form_compose_view::__timer_popup_expired_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	auto_fill_form_compose_view *view = (auto_fill_form_compose_view *)data;
	if (view->m_navi_it != elm_naviframe_top_item_get(m_naviframe))
		return;

	elm_object_focus_set(view->m_entry_full_name, EINA_TRUE);
}

void auto_fill_form_compose_view::__done_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_compose_view *view = (auto_fill_form_compose_view *)data;

	if (elm_entry_is_empty(view->m_entry_full_name)) {
		view->show_msg_popup(BR_STRING_ENTER_NAME, 2, __timer_popup_expired_cb, view);
		elm_object_focus_set(view->m_cancel_button, EINA_TRUE); // Closing virtual keyboard by changing the focus
		return;
	}

	if (view->_apply_entry_data() == EINA_FALSE)
		return;

	auto_fill_form_manager *manager = m_browser->get_settings()->get_auto_fill_form_manager();
	manager->get_list_view()->refresh_view();
	elm_object_focus_set(view->m_cancel_button, EINA_TRUE);
	if (view->m_delay_naviframe_pop_idler)
		ecore_idler_del(view->m_delay_naviframe_pop_idler);
	view->m_delay_naviframe_pop_idler = ecore_idler_add(__delay_naviframe_pop_idler_cb, data);
}

void auto_fill_form_compose_view::__cancel_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	if (elm_naviframe_bottom_item_get(m_naviframe) != elm_naviframe_top_item_get(m_naviframe))
		elm_naviframe_item_pop(m_naviframe);
}

Eina_Bool auto_fill_form_compose_view::__delay_naviframe_pop_idler_cb(void *data)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(data == NULL, ECORE_CALLBACK_CANCEL, "data is NULL");

	auto_fill_form_compose_view *view = (auto_fill_form_compose_view *)data;
	browser_view *bv = m_browser->get_browser_view();
	if (bv->is_ime_on())
		return ECORE_CALLBACK_RENEW;
	else {
		if (elm_naviframe_bottom_item_get(m_naviframe) != elm_naviframe_top_item_get(m_naviframe))
			elm_naviframe_item_pop(m_naviframe);
	}

	if (view->m_edit_errorcode != update_error_none)
		view->_show_error_popup(view->m_edit_errorcode);
	if (view->m_save_errorcode != save_error_none)
		view->_show_error_popup(view->m_save_errorcode);

	view->m_delay_naviframe_pop_idler = NULL;
	return ECORE_CALLBACK_CANCEL;
}

void auto_fill_form_compose_view::__editfield_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Evas_Object *editfield = (Evas_Object *)data;

	if (!elm_entry_is_empty(obj) && elm_object_focus_get(obj))
		elm_object_signal_emit(editfield, "elm,action,show,button", "");
	else
		elm_object_signal_emit(editfield, "elm,action,hide,button", "");
}

void auto_fill_form_compose_view::__entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	genlist_callback_data *cb_data = (genlist_callback_data *)data;
	auto_fill_form_compose_view *view = (auto_fill_form_compose_view *)cb_data->user_data;
	RET_MSG_IF(!view, "view is NULL");
	const char* text = elm_entry_entry_get(obj);
	view->show_notification(text, obj, AUTO_FILL_FORM_ENTRY_MAX_COUNT);

	if (!elm_entry_is_empty(view->m_entry_full_name)) {
		elm_object_disabled_set(view->m_done_button, EINA_FALSE);
	} else {
		elm_object_disabled_set(view->m_done_button, EINA_TRUE);
	}
	return;
}

void auto_fill_form_compose_view::__entry_clicked_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	Elm_Object_Item *item = callback_data->it;
	Evas_Object *entry = elm_object_item_part_content_get(item, "elm.icon.entry");
	elm_object_focus_set(entry, EINA_TRUE);
	elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_IN);
}

void auto_fill_form_compose_view::__editfield_clear_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	Evas_Object *entry = (Evas_Object *)data;

	elm_entry_entry_set(entry, "");
}

void auto_fill_form_compose_view::__entry_next_key_cb(void *data, Evas_Object *obj, void *eventInfo)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	genlist_callback_data *callback_data = (genlist_callback_data *)data;
	auto_fill_form_compose_view *view_this = (auto_fill_form_compose_view *)callback_data->user_data;
	auto_fill_form_compose_view::menu_type type = callback_data->type;
	Evas_Object *entry = NULL;
	Elm_Object_Item *item = NULL;

	if (type == profile_composer_title_full_name) {
		entry = elm_object_item_part_content_get(
					view_this->m_company_name_item_callback_data.it, "elm.icon.entry");
		item = view_this->m_company_name_item_callback_data.it;
	} else if (type == profile_composer_title_company_name) {
		entry = elm_object_item_part_content_get(
					view_this->m_address_line_1_item_callback_data.it, "elm.icon.entry");
		item = view_this->m_address_line_1_item_callback_data.it;
	} else if (type == profile_composer_title_address_line_1) {
		entry = elm_object_item_part_content_get(
					view_this->m_address_line_2_item_callback_data.it, "elm.icon.entry");
		item = view_this->m_address_line_2_item_callback_data.it;
	} else if (type == profile_composer_title_address_line_2) {
		entry = elm_object_item_part_content_get(
					view_this->m_city_town_item_callback_data.it, "elm.icon.entry");
		item = view_this->m_city_town_item_callback_data.it;
	} else if (type == profile_composer_title_city_town) {
		entry = elm_object_item_part_content_get(
					view_this->m_country_item_callback_data.it, "elm.icon.entry");
		item = view_this->m_country_item_callback_data.it;
	} else if (type == profile_composer_title_country) {
		entry = elm_object_item_part_content_get(
					view_this->m_post_code_item_callback_data.it, "elm.icon.entry");
		item = view_this->m_post_code_item_callback_data.it;
	} else if (type == profile_composer_title_post_code) {
		entry = elm_object_item_part_content_get(
					view_this->m_phone_item_callback_data.it, "elm.icon.entry");
		item = view_this->m_phone_item_callback_data.it;
	}
#if 0 /* UX is not allowed */
	else if (type == profile_composer_title_county_region) {
		entry = elm_object_item_part_content_get(
					view_this->m_post_code_item_callback_data.it, "elm.icon.entry");
	}
#endif
	else if (type == profile_composer_title_phone) {
		entry = elm_object_item_part_content_get(
					view_this->m_email_item_callback_data.it, "elm.icon.entry");
		item = view_this->m_email_item_callback_data.it;
	} else if (type == profile_composer_title_email) {
		BROWSER_LOGD("It's last item to go");
		return;
	}
	elm_object_focus_set(entry, EINA_TRUE);
	elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_IN);
	elm_entry_cursor_end_set(entry);
}

Eina_Bool auto_fill_form_compose_view::__naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, EINA_FALSE, "data is NULL");

	return EINA_TRUE;
}

void auto_fill_form_compose_view::__naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	auto_fill_form_compose_view *view = (auto_fill_form_compose_view *)data;
	if (view->m_navi_it != elm_naviframe_top_item_get(m_naviframe))
		return;
	if (!m_browser->is_tts_enabled()) {
		Evas_Object *entry = elm_object_item_part_content_get(
							view->m_full_name_item_callback_data.it, "elm.icon.entry");
		elm_object_focus_set(entry, EINA_TRUE);
		elm_entry_cursor_end_set(entry);
	}
}

list_view_item_clicked_popup::list_view_item_clicked_popup(auto_fill_form_item *item)
:
	m_popup(NULL),
	m_item(item),
	m_popup_last_item(NULL)
{
	BROWSER_LOGD("");

}

list_view_item_clicked_popup::~list_view_item_clicked_popup(void)
{
	BROWSER_LOGD("");

	if (m_popup)
		evas_object_del(m_popup);
	m_popup = NULL;
}

Eina_Bool list_view_item_clicked_popup::show(void)
{
	BROWSER_LOGD("");

	Evas_Object *popup = brui_popup_add(m_window);
	RETV_MSG_IF(!popup, EINA_FALSE, "popup is NULL");
	m_popup = popup;
	evas_object_smart_callback_add(popup, "block,clicked", __cancel_button_cb, this);

#if defined(HW_MORE_BACK_KEY)
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __cancel_button_cb, this);
#endif

	elm_object_style_set(popup,"default");
	elm_object_part_text_set(popup, "title,text", m_item->get_name());
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *genlist = _create_genlist(popup);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return EINA_FALSE;
	}

	Evas_Object *box = elm_box_add(popup);
	if (!box) {
		BROWSER_LOGE("elm_box_add failed");
		return EINA_FALSE;
	}

	evas_object_size_hint_min_set(box, 0, ELM_SCALE_SIZE(VIEWER_ATTACH_LIST_ITEM_HEIGHT * selected_item_num));
	evas_object_show(genlist);
	elm_box_pack_end(box, genlist);
	elm_object_content_set(popup, box);
	evas_object_show(popup);

	return EINA_TRUE;
}

void list_view_item_clicked_popup::__genlist_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");
	RET_MSG_IF(!(event_info), "event_info is NULL");

	list_view_item_clicked_popup *popup_class = (list_view_item_clicked_popup *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	if (it == popup_class->m_popup_last_item)
		elm_object_item_signal_emit(it, "elm,state,bottomline,hide", "");
}

Evas_Object *list_view_item_clicked_popup::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return EINA_FALSE;
	}

	evas_object_smart_callback_add(genlist, "realized", __genlist_realized_cb, this);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();
	if (item_ic) {
		item_ic->item_style = "1text/popup";
		item_ic->func.text_get = __genlist_text_get;
		item_ic->func.content_get = NULL;
		item_ic->func.state_get = NULL;
		item_ic->func.del = NULL;

		elm_genlist_item_append(genlist, item_ic, (void *)selected_item_edit, NULL, ELM_GENLIST_ITEM_NONE, __popup_edit_cb, m_item);
		m_popup_last_item = elm_genlist_item_append(genlist, item_ic, (void *)selected_item_delete, NULL, ELM_GENLIST_ITEM_NONE, __popup_delete_cb, m_item);
		elm_genlist_item_class_free(item_ic);
	}

	return genlist;
}

char *list_view_item_clicked_popup::__genlist_text_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("");

	char *label = NULL;
	int type = (int)data;

	if (!strcmp(part, "elm.text")) {
		if (type == selected_item_edit)
			label = strdup(BR_STRING_EDIT);
		else if (type == selected_item_delete)
			label = strdup(BR_STRING_DELETE);
	}

	return label;
}

void list_view_item_clicked_popup::__popup_edit_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_item *item = (auto_fill_form_item *)data;
	auto_fill_form_manager *manager = m_browser->get_settings()->get_auto_fill_form_manager();
	RET_MSG_IF(!manager, "auto_fill_form_manager is NULL");

	manager->get_list_view()->delete_list_item_selected_popup();
	manager->show_composer(item);
}

void list_view_item_clicked_popup::__popup_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_item *item = (auto_fill_form_item *)data;
	auto_fill_form_manager *manager = m_browser->get_settings()->get_auto_fill_form_manager();
	RET_MSG_IF(!manager, "auto_fill_form_manager is NULL");

	manager->get_list_view()->delete_list_item_selected_popup();
	m_browser->get_browser_view()->show_msg_popup("IDS_BR_HEADER_DELETE_PROFILE",
										"IDS_BR_POP_DELETE_Q",
										NULL,
										"IDS_BR_SK_CANCEL",
										NULL,
										"IDS_BR_SK_DELETE",
										__delete_ok_confirm_cb,
										item);
}

void list_view_item_clicked_popup::__delete_ok_confirm_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	auto_fill_form_item *item = (auto_fill_form_item *)data;
	m_browser->get_settings()->get_auto_fill_form_manager()->delete_auto_fill_form_item(item);
	m_browser->get_browser_view()->show_noti_popup(BR_STRING_DELETED);
}

void list_view_item_clicked_popup::__cancel_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	list_view_item_clicked_popup *popup_class = (list_view_item_clicked_popup*)data;
	if (popup_class->m_popup) {
		evas_object_del(popup_class->m_popup);
		popup_class->m_popup = NULL;
	}
}

