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

#include "settings.h"

#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "preference.h"

#define settings_edj_path browser_edj_dir"/settings.edj"

typedef struct _settings_menu_item {
	settings_menu_index menu_index;
	Eina_Bool is_title;
	Eina_Bool is_expandable;
	Eina_Bool is_sub_item;
	const char *item_style;
} settings_menu_item;

settings_menu_item menu_items[] = {
	{br_settings_main_menu_basic_item_start, EINA_TRUE, EINA_FALSE, EINA_FALSE, "separator"},
	{br_settings_main_menu_basic_item_title, EINA_TRUE, EINA_FALSE, EINA_FALSE, "groupindex"},
	{br_settings_main_menu_basic_item_set_homepage, EINA_FALSE, EINA_TRUE, EINA_FALSE, "2line.top"},
	{br_settings_main_menu_basic_item_set_homepage_sub_item_default_page, EINA_FALSE, EINA_FALSE, EINA_TRUE, "dialogue/2text.1icon.2"},
	{br_settings_main_menu_basic_item_set_homepage_sub_item_current_page, EINA_FALSE, EINA_FALSE, EINA_TRUE, "dialogue/2text.1icon.2"},
	{br_settings_main_menu_basic_item_set_homepage_sub_item_set_homepage_url, EINA_FALSE, EINA_FALSE, EINA_TRUE, "dialogue/1text.1icon.2"},
	{br_settings_main_menu_basic_item_auto_fill_forms, EINA_FALSE, EINA_FALSE, EINA_FALSE, "multiline_sub.main.1icon"},
	{br_settings_main_menu_advanced_item_start, EINA_TRUE, EINA_FALSE, EINA_FALSE, "separator"},
	{br_settings_main_menu_advanced_item_title, EINA_TRUE, EINA_FALSE, EINA_FALSE, "groupindex"},
	{br_settings_main_menu_advanced_item_privacy, EINA_FALSE, EINA_FALSE, EINA_FALSE, "1line"},
	{br_settings_main_menu_advanced_item_contents_settings, EINA_FALSE, EINA_FALSE, EINA_FALSE, "1line"},
	{br_settings_main_menu_advanced_item_bandwidth_management, EINA_FALSE, EINA_FALSE, EINA_FALSE, "1line"},
	{br_settings_main_menu_advanced_item_developer_mode, EINA_FALSE, EINA_FALSE, EINA_FALSE, "1line"},
	{br_settings_main_menu_advanced_item_bottom_margin, EINA_TRUE, EINA_FALSE, EINA_FALSE, "separator"},
	{br_settings_main_menu_end, EINA_FALSE, EINA_FALSE, EINA_FALSE, NULL}
};

settings::settings(void)
	: m_homepage_edit(NULL)
	, m_auto_fill_form_manager(NULL)
	, m_privacy_edit(NULL)
	, m_set_homepage_view(NULL)
	, m_contents_settings_edit(NULL)
	, m_bandwidth_management_edit(NULL)
	, m_genlist(NULL)
	, m_homepage_radio_group(NULL)
	, m_main_layout(NULL)
	, m_navi_item(NULL)
	, m_developer_mode_edit(NULL)
{
	BROWSER_LOGD("");

	/* change homepage setting from current page to other page if current webpage is different from setting's */
	if (m_preference->get_homepage_type() == HOMEPAGE_TYPE_CURRENT_PAGE)
		_change_homepage_type_current_to_other();
}

settings::~settings(void)
{
	BROWSER_LOGD("");

	if (m_homepage_edit)
		delete m_homepage_edit;
	m_homepage_edit = NULL;

	if (m_set_homepage_view)
		delete m_set_homepage_view;
	m_set_homepage_view = NULL;

	if (m_auto_fill_form_manager)
		delete m_auto_fill_form_manager;
	m_auto_fill_form_manager = NULL;

	if (m_privacy_edit)
		delete m_privacy_edit;
	m_privacy_edit = NULL;

	if (m_contents_settings_edit)
		delete m_contents_settings_edit;
	m_contents_settings_edit = NULL;

	if (m_bandwidth_management_edit)
		delete m_bandwidth_management_edit;
	m_bandwidth_management_edit = NULL;

	if (m_developer_mode_edit)
		delete m_developer_mode_edit;
	m_developer_mode_edit = NULL;

	_destroy_genlist_styles();
	_destroy_genlist_callback_datas();
	evas_object_smart_callback_del(m_naviframe, "transition,finished", __naviframe_pop_finished_cb);
}

Eina_Bool settings::show(void)
{
	BROWSER_LOGD("");

	if (_create_main_layout(m_naviframe) == EINA_FALSE) {
		BROWSER_LOGE("_create_main_layout failed");
		return EINA_FALSE;
	}

	m_navi_item = elm_naviframe_item_push(m_naviframe, "IDS_BR_BODY_SETTINGS", NULL, NULL, m_main_layout, NULL);//BR_STRING_SETTINGS
	elm_object_item_domain_text_translatable_set(m_navi_item, BROWSER_DOMAIN_NAME, EINA_TRUE);
	elm_naviframe_item_pop_cb_set(m_navi_item, __naviframe_pop_cb, this);
	evas_object_smart_callback_add(m_naviframe, "transition,finished", __naviframe_pop_finished_cb, this);

	return EINA_TRUE;
}

void settings::clear_popups(void)
{
	BROWSER_LOGD("");

	hide_setting_popups();
}

void settings::hide_setting_popups(void)
{
	BROWSER_LOGD("");

	/* homepage edit */
	if (m_homepage_edit != NULL)
		get_homepage_edit()->destroy_homepage_edit_popup_show();

	/* homepage view */
	if (m_set_homepage_view != NULL)
		get_homepage_view()->destroy_homepage_edit_popup_show();

	if (m_auto_fill_form_manager != NULL) {
		if (get_auto_fill_form_manager()->get_list_view())
			get_auto_fill_form_manager()->get_list_view()->delete_list_item_selected_popup();
	}

	/* delete contents settings popup */
	if (m_contents_settings_edit != NULL) {
		get_contents_settings_edit()->delete_default_storage_ask_popup();
		get_contents_settings_edit()->delete_drag_and_drop_desc_popup();
		get_contents_settings_edit()->delete_reset_to_default_ask_popup();
	}

	/* delete developer mode popup */
	if (m_developer_mode_edit != NULL)
		get_developer_mode_edit()->get_custom_user_agent_view()->destroy_custom_user_agent_edit_popup_show();

}

homepage_edit *settings::get_homepage_edit(void)
{
	BROWSER_LOGD("");

	if (!m_homepage_edit)
		m_homepage_edit = new homepage_edit();

	return m_homepage_edit;
}

void settings::delete_homepage_edit(void)
{
	BROWSER_LOGD("");

	if (m_homepage_edit)
		delete m_homepage_edit;
	m_homepage_edit = NULL;
}

set_homepage_view *settings::get_homepage_view(void)
{
	BROWSER_LOGD("");

	if (!m_set_homepage_view)
		m_set_homepage_view = new set_homepage_view();

	return m_set_homepage_view;
}

void settings::delete_homepage_view(void)
{
	BROWSER_LOGD("");

	if (m_set_homepage_view)
		delete m_set_homepage_view;
	m_set_homepage_view = NULL;
}

auto_fill_form_manager *settings::get_auto_fill_form_manager(void)
{
	BROWSER_LOGD("");

	if (!m_auto_fill_form_manager)
		m_auto_fill_form_manager = new auto_fill_form_manager();

	return m_auto_fill_form_manager;
}

void settings::delete_auto_fill_form_manager(void)
{
	BROWSER_LOGD("");

	if (m_auto_fill_form_manager)
		delete m_auto_fill_form_manager;
	m_auto_fill_form_manager = NULL;
}

privacy_edit *settings::get_privacy_edit(void)
{
	BROWSER_LOGD("");

	if (!m_privacy_edit)
		m_privacy_edit = new privacy_edit();

	return m_privacy_edit;
}

Eina_Bool settings::is_privacy_edit_exist(void)
{
	BROWSER_LOGD("");
	if (m_privacy_edit)
		return EINA_TRUE;

	return EINA_FALSE;
}

void settings::delete_privacy_edit(void)
{
	BROWSER_LOGD("");

	if (m_privacy_edit)
		delete m_privacy_edit;
	m_privacy_edit = NULL;
}

contents_settings_edit *settings::get_contents_settings_edit(void)
{
	BROWSER_LOGD("");

	if (!m_contents_settings_edit)
		m_contents_settings_edit = new contents_settings_edit();

	return m_contents_settings_edit;
}

void settings::delete_contents_settings_edit(void)
{
	BROWSER_LOGD("");

	if (m_contents_settings_edit)
		delete m_contents_settings_edit;
	m_contents_settings_edit = NULL;
}

bandwidth_management_edit *settings::get_bandwidth_management_edit(void)
{
	BROWSER_LOGD("");

	if (!m_bandwidth_management_edit)
		m_bandwidth_management_edit = new bandwidth_management_edit();

	return m_bandwidth_management_edit;
}

void settings::delete_bandwidth_management_edit(void)
{
	BROWSER_LOGD("");

	if (m_bandwidth_management_edit)
		delete m_bandwidth_management_edit;
	m_bandwidth_management_edit = NULL;
}

developer_mode_edit *settings::get_developer_mode_edit(void)
{
	BROWSER_LOGD("");

	if (!m_developer_mode_edit)
		m_developer_mode_edit = new developer_mode_edit();

	return m_developer_mode_edit;
}

void settings::delete_developer_mode_edit(void)
{
	BROWSER_LOGD("");

	if (m_developer_mode_edit)
		delete m_developer_mode_edit;
	m_developer_mode_edit = NULL;
}

Eina_Bool settings::is_shown(void)
{
	if (m_genlist)
		return EINA_TRUE;
	return EINA_FALSE;
}

void settings::set_homepage_type_in_genlist(homepage_type type)
{
	BROWSER_LOGD("type[%d]", type);

	RET_MSG_IF(type < HOMEPAGE_TYPE_DEFAULT_PAGE || type > HOMEPAGE_TYPE_USER_HOMEPAGE, "type is out of allowed");
	elm_radio_value_set(m_homepage_radio_group, type);
	elm_genlist_realized_items_update(m_genlist);
}

void settings::hide_homepage_sub_items_in_genlist(void)
{
	BROWSER_LOGD("");

	Elm_Object_Item *expandable_parent_item = _get_genlist_item(br_settings_main_menu_basic_item_set_homepage);
	RET_MSG_IF(expandable_parent_item == NULL, "expandable_parent_item is NULL");

	elm_genlist_item_expanded_set(expandable_parent_item, EINA_FALSE);
	elm_genlist_item_subitems_clear(expandable_parent_item);
}

Eina_Bool settings::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");

	m_genlist = elm_genlist_add(parent);
	if (!m_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return EINA_FALSE;
	}
	elm_genlist_realization_mode_set(m_genlist, EINA_TRUE);
	elm_genlist_mode_set(m_genlist, ELM_LIST_COMPRESS);

	/* To avoid memory leak, remove allocated memory */
	_destroy_genlist_styles();
	_destroy_genlist_callback_datas();

	evas_object_smart_callback_add(m_genlist, "realized", __genlist_item_realized_cb, this);
	evas_object_smart_callback_add(m_genlist, "language,changed", __genlist_lang_changed, NULL);

	Eina_Bool has_error = EINA_FALSE;
	for (unsigned short i = br_settings_main_menu_start; i < br_settings_main_menu_end; i++) {

		if (i == br_settings_main_menu_advanced_item_developer_mode && m_browser->get_developer_mode() == EINA_FALSE)
			continue;

		Elm_Genlist_Item_Class *item_class = _create_genlist_item_class((settings_menu_index)i);
		if (item_class) {
			settings_genlist_callback_data *cb_data = new(settings_genlist_callback_data);
			if (!cb_data) {
				has_error = EINA_TRUE;
				break;
			}
			memset(cb_data, 0x00, sizeof(settings_genlist_callback_data));

			cb_data->type = (settings_menu_index)i;
			cb_data->user_data = this;

			if (menu_items[i].is_title == EINA_TRUE) {
				cb_data->it = elm_genlist_item_append(m_genlist, item_class, (void *)cb_data, NULL,
													ELM_GENLIST_ITEM_NONE, NULL, NULL);
				elm_genlist_item_select_mode_set(cb_data->it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			} else if (menu_items[i].is_sub_item == EINA_TRUE) {
				/* append item when parent item has been clicked */
			} else if (menu_items[i].is_expandable == EINA_TRUE) {
				cb_data->it = elm_genlist_item_append(m_genlist, item_class, (void *)cb_data, NULL,
													ELM_GENLIST_ITEM_TREE, __genlist_item_clicked_cb, (void *)cb_data);
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

	Evas_Object *layout = elm_layout_add(parent);
	if (!layout) {
		BROWSER_LOGE("elm_layout_add failed");
		_destroy_genlist_styles();
		_destroy_genlist_callback_datas();
		return EINA_FALSE;
	}
	elm_layout_file_set(layout, settings_edj_path, "main-layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_part_content_set(layout, "elm.swallow.content", m_genlist);
	m_main_layout = layout;

	return EINA_TRUE;
}

Elm_Genlist_Item_Class *settings::_create_genlist_item_class(settings_menu_index index)
{
	BROWSER_LOGD("settings_menu_index[%d]", index);

	if ((index < br_settings_main_menu_start) || (index > br_settings_main_menu_end)) {
		BROWSER_LOGE("index is out of allowed");
		return NULL;
	}

	if (!menu_items[index].item_style) {
		BROWSER_LOGE("unable to get menu_items[i].item_style", index);
		return NULL;
	}

	Elm_Genlist_Item_Class *item_class = NULL;
	/* check duplicated item style */
	for (unsigned short i = 0; i < m_genlist_item_class_list.size(); i++) {
		if (strlen(m_genlist_item_class_list[i]->item_style) == strlen(menu_items[index].item_style)
			&& !strncmp(m_genlist_item_class_list[i]->item_style, menu_items[index].item_style, strlen(m_genlist_item_class_list[i]->item_style))) {
			BROWSER_LOGE("[%s] style is already created", menu_items[index].item_style);
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
		item_class->item_style = menu_items[index].item_style;
		item_class->func.text_get = __genlist_text_get;
		item_class->func.content_get = __genlist_contents_get;
		item_class->func.state_get = NULL;
		item_class->func.del = NULL;
		m_genlist_item_class_list.push_back(item_class);
	}

	return item_class;
}

Elm_Genlist_Item_Class *settings::_get_genlist_item_class(settings_menu_index index)
{

	if ((index < br_settings_main_menu_start) || (index > br_settings_main_menu_end)) {
		BROWSER_LOGE("index is out of allowed");
		return NULL;
	}

	if (!(menu_items[index].item_style && strlen(menu_items[index].item_style))) {
		BROWSER_LOGE("[%d]th menu_items has invalid item style", index);
		return NULL;
	}

	Elm_Genlist_Item_Class *item_class = NULL;
	for (unsigned short i = 0; i < m_genlist_item_class_list.size(); i++) {
		if (!(m_genlist_item_class_list[i]->item_style && strlen(m_genlist_item_class_list[i]->item_style) > 0)) {
			BROWSER_LOGE("!(m_genlist_item_class_list[i]->item_style && strlen(m_genlist_item_class_list[i]->item_style) > 0)");
			continue;
		}

		if (strlen(m_genlist_item_class_list[i]->item_style) == strlen(menu_items[index].item_style) &&
			!strncmp(m_genlist_item_class_list[i]->item_style, menu_items[index].item_style, strlen(menu_items[index].item_style))) {
			item_class = m_genlist_item_class_list[i];
			break;
		}
	}

	return item_class;
}

Elm_Object_Item *settings::_get_genlist_item(settings_menu_index index)
{
	BROWSER_LOGD("");
	Elm_Object_Item *item = NULL;

	for (unsigned int i = 0; i < m_genlist_callback_data_list.size(); i++) {
		if (m_genlist_callback_data_list[i]->type == index) {
			item = m_genlist_callback_data_list[i]->it;
			break;
		}
	}

	return item;
}

void settings::_destroy_genlist_styles(void)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_genlist_item_class_list.size(); i++) {
		if (m_genlist_item_class_list[i])
			elm_genlist_item_class_free(m_genlist_item_class_list[i]);
		m_genlist_item_class_list[i] = NULL;
	}
	m_genlist_item_class_list.clear();
}

void settings::_destroy_genlist_callback_datas(void)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_genlist_callback_data_list.size(); i++) {
		if (m_genlist_callback_data_list[i])
			free(m_genlist_callback_data_list[i]);
		m_genlist_callback_data_list[i] = NULL;
	}
	m_genlist_callback_data_list.clear();
}

void settings::_change_homepage_type_current_to_other(void)
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

const char *settings::_get_current_page_uri_to_show(void)
{
	const char *current_page_uri = m_browser->get_browser_view()->get_current_webview()->get_uri();
	return current_page_uri;
}

char *settings::__genlist_text_get(void *data, Evas_Object *obj, const char *part)
{
	RETV_MSG_IF(!data, NULL, "data is NULL");

	settings_genlist_callback_data *cb_data = (settings_genlist_callback_data *)data;
	settings_menu_index menu_index = cb_data->type;
	settings *cp = (settings *)cb_data->user_data;

	char *menu_text = NULL;
	BROWSER_LOGD("menu_index[%d]", menu_index);
	switch (menu_index) {
		case br_settings_main_menu_basic_item_title:
			if (!strcmp(part, "elm.text.main"))
				menu_text = strdup(BR_STRING_BASIC);
		break;

		case br_settings_main_menu_basic_item_set_homepage:
			if (!strcmp(part, "elm.text.main.left.top"))
				menu_text = strdup(BR_STRING_SETTINGS_SET_HOMEPAGE);
			else if (!strcmp(part, "elm.text.sub.left.bottom")) {
				homepage_type type = m_preference->get_homepage_type();
				if (type == HOMEPAGE_TYPE_DEFAULT_PAGE)
					menu_text = strdup(BR_STRING_SETTINGS_DEFAULT_PAGE);
				else if (type == HOMEPAGE_TYPE_CURRENT_PAGE)
					menu_text = strdup(BR_STRING_SETTINGS_CURRENT_PAGE);
				else if (type == HOMEPAGE_TYPE_USER_HOMEPAGE) {
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
			}
		break;

		case br_settings_main_menu_basic_item_set_homepage_sub_item_default_page:
			if (!strcmp(part, "elm.text"))
				menu_text = strdup(BR_STRING_SETTINGS_DEFAULT_PAGE);
			else if (!strcmp(part, "elm.text.1"))
				menu_text = strdup(BR_STRING_SETTINGS_DEFAULT_PAGE);
			else if (!strcmp(part, "elm.text.2")) {
				char *default_homepage_uri = m_preference->get_default_homepage();
				if (default_homepage_uri) {
					if (strlen(default_homepage_uri) > 0)
						menu_text = strdup(default_homepage_uri);
					else
						menu_text = strdup(blank_page);
					free(default_homepage_uri);
				} else
					menu_text = strdup(blank_page);
			}
		break;

		case br_settings_main_menu_basic_item_set_homepage_sub_item_current_page:
			if (!strcmp(part, "elm.text"))
				menu_text = strdup(BR_STRING_SETTINGS_CURRENT_PAGE);
			else if (!strcmp(part, "elm.text.1"))
				menu_text = strdup(BR_STRING_SETTINGS_CURRENT_PAGE);
			else if (!strcmp(part, "elm.text.2")) {
				webview *wv = m_browser->get_browser_view()->get_current_webview();
				if (wv != NULL)
					menu_text = strdup(cp->_get_current_page_uri_to_show());
				else
					menu_text = strdup(blank_page);
			}
		break;

		case br_settings_main_menu_basic_item_set_homepage_sub_item_set_homepage_url:
			if (!strcmp(part, "elm.text"))
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

		case br_settings_main_menu_basic_item_auto_fill_forms:
			if (!strcmp(part, "elm.text.main"))
				menu_text = strdup(BR_STRING_SETTINGS_AUTO_FILL_FORMS);
			else if (!strcmp(part, "elm.text.multiline"))
				menu_text = strdup(BR_STRING_AUTO_FILL_DESC);
		break;

		case br_settings_main_menu_advanced_item_title:
			if (!strcmp(part, "elm.text.main"))
				menu_text = strdup(BR_STRING_SETTINGS_ADVANCED);
		break;

		case br_settings_main_menu_advanced_item_privacy:
			if (!strcmp(part, "elm.text.main.left"))
				menu_text = strdup(BR_STRING_SETTINGS_PRIVACY);
		break;

		case br_settings_main_menu_advanced_item_contents_settings:
			if (!strcmp(part, "elm.text.main.left"))
				menu_text = strdup(BR_STRING_SETTINGS_CONTENT_SETTINGS);
		break;

		case br_settings_main_menu_advanced_item_bandwidth_management:
			if (!strcmp(part, "elm.text.main.left"))
				menu_text = strdup(BR_STRING_SETTINGS_BANDWIDTH_MANAGEMENT);
		break;

		case br_settings_main_menu_advanced_item_developer_mode:
			if (!strcmp(part, "elm.text"))
				menu_text = strdup(BR_STRING_SETTINGS_DEVELOPER_MODE);
			else if (!strcmp(part, "elm.text.main.left"))
				menu_text = strdup(BR_STRING_SETTINGS_DEVELOPER_MODE);
		break;


		default:
			BROWSER_LOGD("menu_index[%d] is out of allowed", menu_index);
		break;
	}

	return menu_text;
}

Evas_Object *settings::__genlist_contents_get(void *data, Evas_Object *obj, const char *part)
{
	/* for debug */
	BROWSER_LOGD("part[%s]", part);

	settings_genlist_callback_data *cb_data = (settings_genlist_callback_data *)data;
	settings *cp = (settings *)cb_data->user_data;
	settings_menu_index menu_index = cb_data->type;

	homepage_type homepage = m_preference->get_homepage_type();
	Evas_Object *content = NULL;

	switch(menu_index) {
		case br_settings_main_menu_basic_item_set_homepage_sub_item_default_page:
			if (!strcmp(part, "elm.icon")) {
				Evas_Object *radio_button = elm_radio_add(obj);
				if (radio_button) {
					elm_radio_state_value_set(radio_button, HOMEPAGE_TYPE_DEFAULT_PAGE);
					elm_radio_group_add(radio_button, cp->m_homepage_radio_group);
					evas_object_propagate_events_set(radio_button, EINA_FALSE);
					evas_object_smart_callback_add(radio_button, "changed", __homepage_sub_radio_item_clicked_cb, cb_data);
					elm_access_object_unregister(radio_button);
				}
				content = radio_button;
			}
		break;

		case br_settings_main_menu_basic_item_set_homepage_sub_item_current_page:
			if (!strcmp(part, "elm.icon")) {
				Evas_Object *radio_button = elm_radio_add(obj);
				if (radio_button) {
					elm_radio_state_value_set(radio_button, HOMEPAGE_TYPE_CURRENT_PAGE);
					elm_radio_group_add(radio_button, cp->m_homepage_radio_group);
					evas_object_propagate_events_set(radio_button, EINA_FALSE);
					evas_object_smart_callback_add(radio_button, "changed", __homepage_sub_radio_item_clicked_cb, cb_data);
					elm_access_object_unregister(radio_button);
				}
				content = radio_button;
			}
		break;

		case br_settings_main_menu_basic_item_set_homepage_sub_item_set_homepage_url:
			if (!strcmp(part, "elm.icon")) {
				Evas_Object *radio_button = elm_radio_add(obj);
				if (radio_button) {
					elm_radio_state_value_set(radio_button, HOMEPAGE_TYPE_USER_HOMEPAGE);
					elm_radio_group_add(radio_button, cp->m_homepage_radio_group);
					evas_object_propagate_events_set(radio_button, EINA_FALSE);
					evas_object_smart_callback_add(radio_button, "changed", __homepage_sub_radio_item_clicked_cb, cb_data);
				}
				content = radio_button;
			} else if (!strcmp(part, "elm.icon.1")) {
				Evas_Object *radio_button = elm_radio_add(obj);
				if (radio_button) {
					elm_radio_state_value_set(radio_button, HOMEPAGE_TYPE_USER_HOMEPAGE);
					elm_radio_group_add(radio_button, cp->m_homepage_radio_group);
					evas_object_propagate_events_set(radio_button, EINA_FALSE);
					evas_object_smart_callback_add(radio_button, "changed", __homepage_sub_radio_item_clicked_cb, cb_data);
					elm_access_object_unregister(radio_button);
				}
				content = radio_button;
			} else if (!strcmp(part, "elm.icon.2")){
				std::string msg = std::string (BR_STRING_SETTINGS_OTHER) + " " + std::string(BR_STRING_HOMEPAGE_ABB);
				Evas_Object *reveal_button = elm_button_add(obj);
				elm_object_style_set(reveal_button, "reveal");
				evas_object_propagate_events_set(reveal_button, EINA_FALSE);
				evas_object_repeat_events_set(reveal_button, EINA_FALSE);
				evas_object_smart_callback_add(reveal_button, "clicked", __user_homepage_reveal_button_clicked_cb, cb_data);
				elm_access_info_set(reveal_button, ELM_ACCESS_INFO, msg.c_str());
				return reveal_button;
			}
		break;

		case br_settings_main_menu_basic_item_auto_fill_forms:
			if (!strcmp(part, "elm.icon")) {
				cp->m_auto_fill_form_check = elm_check_add(obj);
				elm_object_style_set(cp->m_auto_fill_form_check, "on&off");
				if (cp->m_auto_fill_form_check) {
					evas_object_smart_callback_add(cp->m_auto_fill_form_check, "changed", __auto_fill_form_check_changed_cb, data);

					Eina_Bool auto_fill_form_enalbed = m_preference->get_auto_fill_forms_enabled();
					elm_check_state_set(cp->m_auto_fill_form_check, auto_fill_form_enalbed);
					evas_object_propagate_events_set(cp->m_auto_fill_form_check, EINA_FALSE);
				}
				return cp->m_auto_fill_form_check;
			}
		break;


		default:
			BROWSER_LOGD("menu_index[%d] is out of allowed", menu_index);
			return content;
		break;
	}

	if (homepage != elm_radio_value_get(cp->m_homepage_radio_group))
		elm_radio_value_set(cp->m_homepage_radio_group, homepage);

	/* change homepage setting from current page to other page if current webpage is different from setting's */
	if (m_preference->get_homepage_type() == HOMEPAGE_TYPE_CURRENT_PAGE)
		cp->_change_homepage_type_current_to_other();

	return content;
}

void settings::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	settings_genlist_callback_data *cb_data = (settings_genlist_callback_data *)data;
	settings *cp = (settings *)cb_data->user_data;
	settings_menu_index menu_index = cb_data->type;

	Elm_Object_Item *item = cb_data->it;
	elm_genlist_item_selected_set(item, EINA_FALSE);

	switch(menu_index) {
		case br_settings_main_menu_basic_item_set_homepage:
			cp->get_homepage_view()->show();
		break;

		case br_settings_main_menu_basic_item_auto_fill_forms:
			cp->get_auto_fill_form_manager()->show_list_view();
		break;

		case br_settings_main_menu_advanced_item_privacy:
			cp->get_privacy_edit()->show();
		break;

		case br_settings_main_menu_advanced_item_contents_settings:
			cp->get_contents_settings_edit()->show();
		break;

		case br_settings_main_menu_advanced_item_bandwidth_management:
			cp->get_bandwidth_management_edit()->show();
		break;

		case br_settings_main_menu_advanced_item_developer_mode:
			cp->get_developer_mode_edit()->show();
		break;

		default:
			BROWSER_LOGD("menu_index[%d] is out of allowed", menu_index);
		break;
	}

}

void settings::__genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	settings_genlist_callback_data *cb_data = (settings_genlist_callback_data *)elm_object_item_data_get(item);
	RET_MSG_IF(!cb_data, "cb_data is NULL");
	RET_MSG_IF((cb_data->type > br_settings_main_menu_end || cb_data->type < br_settings_main_menu_start), "index is out of allowed");

	switch(cb_data->type) {
		case br_settings_main_menu_advanced_item_bandwidth_management:
			if (m_browser->get_developer_mode() == EINA_TRUE)
				elm_object_item_signal_emit(item, "elm,state,center", "");
			else
				elm_object_item_signal_emit(item, "elm,state,bottom", "");
		break;

		case br_settings_main_menu_advanced_item_developer_mode:
			elm_object_item_signal_emit(item, "elm,state,bottom", "");
		break;

		default:
			elm_object_item_signal_emit(item, "elm,state,center", "");
		break;
	}

}

void settings::__homepage_sub_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	settings_genlist_callback_data *cb_data = (settings_genlist_callback_data *)data;
	settings *cp = (settings *)cb_data->user_data;
	settings_menu_index menu_index = cb_data->type;

	webview *wv = m_browser->get_browser_view()->get_current_webview();

	Elm_Object_Item *item = (Elm_Object_Item *)cb_data->it;
	elm_genlist_item_selected_set(item, EINA_FALSE);
	elm_genlist_item_update(item);

	BROWSER_LOGD("selected menu_index[%d]", menu_index);

	switch(menu_index) {
		case br_settings_main_menu_basic_item_set_homepage_sub_item_default_page:
			m_preference->set_homepage_type(HOMEPAGE_TYPE_DEFAULT_PAGE);
			cp->set_homepage_type_in_genlist(HOMEPAGE_TYPE_DEFAULT_PAGE);
			cp->hide_homepage_sub_items_in_genlist();
		break;

		case br_settings_main_menu_basic_item_set_homepage_sub_item_current_page:
			m_preference->set_homepage_type(HOMEPAGE_TYPE_CURRENT_PAGE);
			if (wv && strlen(wv->get_uri()) > 0)
				m_preference->set_current_page_uri(wv->get_uri());
			else
				m_preference->set_current_page_uri("");
			cp->set_homepage_type_in_genlist(HOMEPAGE_TYPE_CURRENT_PAGE);
			cp->hide_homepage_sub_items_in_genlist();
		break;

		case br_settings_main_menu_basic_item_set_homepage_sub_item_set_homepage_url:
		{
			evas_object_freeze_events_set(cp->m_genlist, EINA_TRUE);
			char *user_homepage_uri = m_preference->get_user_homagepage();
			cp->get_homepage_edit()->user_homepage_edit_popup_show(user_homepage_uri);
			free(user_homepage_uri);
		}
		break;

		default:
			BROWSER_LOGD("menu_index[%d] is out of allowed for homepage setting", menu_index);
		break;
	}
}

void settings::__homepage_sub_radio_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	settings_genlist_callback_data *cb_data = (settings_genlist_callback_data *)data;
	settings *cp = (settings *)cb_data->user_data;
	settings_menu_index menu_index = cb_data->type;

	webview *wv = m_browser->get_browser_view()->get_current_webview();

	Elm_Object_Item *item = (Elm_Object_Item *)cb_data->it;
	elm_genlist_item_selected_set(item, EINA_FALSE);
	elm_genlist_item_update(item);

	BROWSER_LOGD("selected menu_index[%d]", menu_index);

	switch(menu_index) {
		case br_settings_main_menu_basic_item_set_homepage_sub_item_default_page:
			m_preference->set_homepage_type(HOMEPAGE_TYPE_DEFAULT_PAGE);
			cp->set_homepage_type_in_genlist(HOMEPAGE_TYPE_DEFAULT_PAGE);
			cp->hide_homepage_sub_items_in_genlist();
		break;

		case br_settings_main_menu_basic_item_set_homepage_sub_item_current_page:
			m_preference->set_homepage_type(HOMEPAGE_TYPE_CURRENT_PAGE);
			if (wv && strlen(wv->get_uri()) > 0)
				m_preference->set_current_page_uri(wv->get_uri());
			else
				m_preference->set_current_page_uri("");
			cp->set_homepage_type_in_genlist(HOMEPAGE_TYPE_CURRENT_PAGE);
			cp->hide_homepage_sub_items_in_genlist();
		break;

		case br_settings_main_menu_basic_item_set_homepage_sub_item_set_homepage_url:
		{
			evas_object_freeze_events_set(cp->m_genlist, EINA_TRUE);
			char *user_homepage_uri = m_preference->get_user_homagepage();
			cp->get_homepage_edit()->user_homepage_edit_popup_show(user_homepage_uri);
			free(user_homepage_uri);
		}
		break;

		default:
			BROWSER_LOGD("menu_index[%d] is out of allowed for homepage setting", menu_index);
		break;
	}
}

void settings::__user_homepage_reveal_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	settings_genlist_callback_data *cb_data = (settings_genlist_callback_data *)data;
	settings *cp = (settings *)cb_data->user_data;
	char *user_homepage_uri = m_preference->get_user_homagepage();
	cp->get_homepage_edit()->user_homepage_edit_popup_show(user_homepage_uri);
	free(user_homepage_uri);
}

void settings::__auto_fill_form_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	settings_genlist_callback_data *cb_data = (settings_genlist_callback_data *)data;
	settings *cp = (settings *)cb_data->user_data;

	Eina_Bool state = elm_check_state_get(cp->m_auto_fill_form_check);
	m_preference->set_auto_fill_forms_enabled(state);
}

Eina_Bool settings::__naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, EINA_FALSE, "data is NULL");

	return EINA_TRUE;
}

void settings::__naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	settings *cp = (settings *)data;

	if (cp->m_navi_item == elm_naviframe_top_item_get(m_naviframe)) {
		cp->delete_homepage_edit();
		cp->delete_homepage_view();
		cp->delete_privacy_edit();
		cp->delete_contents_settings_edit();
		cp->delete_bandwidth_management_edit();
		cp->delete_developer_mode_edit();
	}
}
