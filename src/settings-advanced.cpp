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

#include "settings-advanced.h"

#include <app_control.h>
#include <vconf.h>
#if defined(HW_MORE_BACK_KEY)
#include <efl_extension.h>
#endif

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "settings.h"
#include "platform-service.h"
#include "url-input-bar.h"
#include "search-engine-manager.h"

#define browser_popup_edj_path browser_edj_dir"/browser-popup.edj"
#define settings_edj_path browser_edj_dir"/settings.edj"

typedef struct _settings_advanced_menu_item {
	settings_advanced_menu_index menu_index;
	Eina_Bool is_title;
	const char *item_style;
} settings_advanced_menu_item;

settings_advanced_menu_item settings_advanced_menus[] = {
	{br_settings_sub_advanced_menu_screen_and_text_start, EINA_TRUE, "separator"},
	{br_settings_sub_advanced_menu_contents_start, EINA_TRUE, "separator"},
	{br_settings_sub_advanced_menu_contents_accept_cookies, EINA_FALSE, "multiline_sub.main.1icon"},
	{br_settings_sub_advanced_menu_contents_accept_cookies_desc, EINA_TRUE, "multiline/1text"},
	{br_settings_sub_advanced_menu_contents_accept_cookies_bottom_margin, EINA_TRUE, "separator"},
	{br_settings_sub_advanced_menu_contents_enable_javascript, EINA_FALSE, "multiline_sub.main.1icon"},
	{br_settings_sub_advanced_menu_contents_enable_javascript_desc, EINA_TRUE, "multiline/1text"},
	{br_settings_sub_advanced_menu_contents_enable_javascript_bottom_margin, EINA_TRUE, "separator"},
	{br_settings_sub_advanced_menu_contents_default_storage, EINA_FALSE, "multiline_sub.main.1icon"},
	{br_settings_sub_advanced_menu_contents_default_storage_bottom_margin, EINA_TRUE, "separator"},
	{br_settings_sub_advanced_menu_contents_reset_to_default_title, EINA_TRUE, "groupindex"},
	{br_settings_sub_advanced_menu_contents_reset_to_default, EINA_FALSE, "multiline_sub.main.1icon"},
	{br_settings_sub_advanced_menu_contents_reset_to_default_bottom_margin, EINA_TRUE, "separator"},

	{br_settings_sub_advanced_menu_privacy_start, EINA_TRUE, "separator"},
	{br_settings_sub_advanced_menu_privacy_remember_form_data, EINA_FALSE, "multiline_sub.main.1icon"},
	{br_settings_sub_advanced_menu_privacy_remember_form_data_desc, EINA_TRUE, "multiline/1text"},
	{br_settings_sub_advanced_menu_privacy_remember_form_data_bottom_margin, EINA_TRUE, "separator"},
	{br_settings_sub_advanced_menu_privacy_remember_passwords, EINA_FALSE, "multiline_sub.main.1icon"},
	{br_settings_sub_advanced_menu_privacy_remember_passwords_bottom_margin, EINA_TRUE, "separator"},
	{br_settings_sub_advanced_menu_privacy_delete_personal_data, EINA_FALSE, "1line"},
	{br_settings_sub_advanced_menu_privacy_delete_personal_data_bottom_margin, EINA_TRUE, "separator"},

	{br_settings_sub_advanced_menu_bandwidth_management_start, EINA_TRUE, "separator"},
	{br_settings_sub_advanced_menu_bandwidth_management_load_images, EINA_FALSE, "multiline_sub.main.1icon"},
	{br_settings_sub_advanced_menu_bandwidth_management_load_images_desc, EINA_TRUE, "multiline/1text"},
	{br_settings_sub_advanced_menu_bandwidth_management_load_images_bottom_margin, EINA_TRUE, "separator"},
	{br_settings_sub_advanced_menu_bandwidth_management_open_pages_in_overview, EINA_FALSE, "multiline_sub.main.1icon"},
	{br_settings_sub_advanced_menu_bandwidth_management_open_pages_in_overview_desc, EINA_TRUE, "multiline/1text"},
	{br_settings_sub_advanced_menu_bandwidth_management_open_pages_in_overview_bottom_margin, EINA_TRUE, "separator"},

	{br_settings_sub_advanced_menu_developer_mode_start, EINA_TRUE, "separator"},
	{br_settings_sub_advanced_menu_developer_mode_user_agent, EINA_FALSE, "1line"},
	{br_settings_sub_advanced_menu_developer_mode_custom_user_agent, EINA_FALSE, "1line"},
	{br_settings_sub_advanced_menu_developer_mode_custom_user_agent_bottom_margin, EINA_TRUE, "separator"},

	{br_settings_sub_advanced_menu_end, EINA_FALSE, NULL}
};

privacy_edit::privacy_edit(void)
:
	m_clear_private_data_view(NULL)
	, m_genlist(NULL)
	, m_navi_item(NULL)
{
	BROWSER_LOGD("");
}

privacy_edit::~privacy_edit(void)
{
	BROWSER_LOGD("");

	_destroy_genlist_styles();
	_destroy_genlist_callback_datas();

	delete_clear_private_data_view();
	evas_object_smart_callback_del(m_naviframe, "transition,finished", __privacy_edit_naviframe_pop_finished_cb);
}

Eina_Bool privacy_edit::show(void)
{
	BROWSER_LOGD("");

	if (_create_main_layout(m_naviframe) == NULL) {
		BROWSER_LOGE("_create_main_layout failed");
		return EINA_FALSE;
	}

	m_navi_item = elm_naviframe_item_push(m_naviframe, "IDS_BR_BODY_PRIVACY", NULL, NULL, m_genlist, NULL);//BR_STRING_SETTINGS_PRIVACY
	elm_object_item_domain_text_translatable_set(m_navi_item, BROWSER_DOMAIN_NAME, EINA_TRUE);

//	elm_naviframe_item_pop_cb_set(m_navi_item, __naviframe_pop_cb, this);
	evas_object_smart_callback_add(m_naviframe, "transition,finished", __privacy_edit_naviframe_pop_finished_cb, this);

	return EINA_TRUE;
}

clear_private_data_view *privacy_edit::get_clear_private_data_view(void)
{
	BROWSER_LOGD("");
	if (!m_clear_private_data_view)
		m_clear_private_data_view = new clear_private_data_view();

	return m_clear_private_data_view;
}

Eina_Bool privacy_edit::is_clear_private_data_view_exist(void)
{
	BROWSER_LOGD("");
	if (m_clear_private_data_view)
		return EINA_TRUE;
	return EINA_FALSE;
}

void privacy_edit::delete_clear_private_data_view(void)
{
	if (m_clear_private_data_view)
		delete m_clear_private_data_view;
	m_clear_private_data_view = NULL;
}

void privacy_edit::mdm_mode_disable_autofill_setting_menu(Eina_Bool disabled)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_genlist_callback_data_list.size(); i++) {
		if (m_genlist_callback_data_list[i]->it) {
			if (m_genlist_callback_data_list[i]->menu_index == br_settings_sub_advanced_menu_privacy_remember_form_data) {
				BROWSER_LOGD("MDM restricted remember form data");
				elm_object_item_disabled_set(m_genlist_callback_data_list[i]->it, disabled);
			} else if (m_genlist_callback_data_list[i]->menu_index == br_settings_sub_advanced_menu_privacy_remember_passwords) {
				BROWSER_LOGD("MDM restricted remember form data");
				elm_object_item_disabled_set(m_genlist_callback_data_list[i]->it, disabled);
			}
		}
	}
}

void privacy_edit::on_rotate(void)
{
	if (m_clear_private_data_view)
		m_clear_private_data_view->on_rotate();
}

Evas_Object *privacy_edit::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");

	m_genlist = elm_genlist_add(parent);
	if (!m_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	elm_genlist_mode_set(m_genlist, ELM_LIST_COMPRESS);

	evas_object_smart_callback_add(m_genlist, "realized", __genlist_item_realized_cb, this);
	evas_object_smart_callback_add(m_genlist, "language,changed", __genlist_lang_changed, NULL);

	_destroy_genlist_styles();
	_destroy_genlist_callback_datas();

	for (unsigned short i = br_settings_sub_advanced_menu_privacy_start; i < br_settings_sub_advanced_menu_privacy_end; i++) {
		Elm_Genlist_Item_Class *item_class = _create_genlist_item_class((settings_advanced_menu_index)i);
		if (item_class) {
			settings_advanced_genlist_callback_data *cb_data = new settings_advanced_genlist_callback_data;
			memset(cb_data, 0x00, sizeof(settings_advanced_genlist_callback_data));

			cb_data->menu_index = (settings_advanced_menu_index)i;
			cb_data->user_data = this;

			if (settings_advanced_menus[i].is_title == EINA_TRUE) {
				cb_data->it = elm_genlist_item_append(m_genlist, item_class, (void *)cb_data, NULL,
													ELM_GENLIST_ITEM_NONE, NULL, NULL);
				elm_genlist_item_select_mode_set(cb_data->it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			} else {
				cb_data->it = elm_genlist_item_append(m_genlist, item_class, (void *)cb_data, NULL,
													ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, (void *)cb_data);
			}
			m_genlist_callback_data_list.push_back(cb_data);
		} else {
			BROWSER_LOGE("item_class is NULL");
			break;
		}
	}

	return m_genlist;
}

Elm_Genlist_Item_Class *privacy_edit::_create_genlist_item_class(settings_advanced_menu_index index)
{
	BROWSER_LOGD("settings_advanced_menu_index[%d]", index);

	if ((index < br_settings_sub_advanced_menu_privacy_start) || (index > br_settings_sub_advanced_menu_privacy_end)) {
		BROWSER_LOGE("index is out of allowed");
		return NULL;
	}

	if (!settings_advanced_menus[index].item_style) {
		BROWSER_LOGE("unable to get privacy_edit : settings_advanced_menus[i].item_style", index);
		return NULL;
	}

	Elm_Genlist_Item_Class *item_class = NULL;
	/* check duplicated item style */
	for (unsigned short i = 0; i < m_genlist_item_class_list.size(); i++) {
		if (strlen(m_genlist_item_class_list[i]->item_style) == strlen(settings_advanced_menus[index].item_style)
			&& !strncmp(m_genlist_item_class_list[i]->item_style, settings_advanced_menus[index].item_style, strlen(m_genlist_item_class_list[i]->item_style))) {
			BROWSER_LOGE("[%s] style is already created", settings_advanced_menus[index].item_style);
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
		item_class->item_style = settings_advanced_menus[index].item_style;
		item_class->func.text_get = __genlist_text_get;
		item_class->func.content_get = __genlist_contents_get;
		item_class->func.state_get = NULL;
		item_class->func.del = NULL;
		m_genlist_item_class_list.push_back(item_class);
	}

	return item_class;
}

void privacy_edit::_destroy_genlist_styles(void)
{
	for (unsigned int i = 0; i < m_genlist_item_class_list.size(); i++) {
		if (m_genlist_item_class_list[i])
			elm_genlist_item_class_free(m_genlist_item_class_list[i]);
		m_genlist_item_class_list[i] = NULL;
	}

	m_genlist_item_class_list.clear();

}

void privacy_edit::_destroy_genlist_callback_datas(void)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_genlist_callback_data_list.size(); i++) {
		if (m_genlist_callback_data_list[i])
			free(m_genlist_callback_data_list[i]);
		m_genlist_callback_data_list[i] = NULL;
	}

	m_genlist_callback_data_list.clear();
}

char *privacy_edit::__genlist_text_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	settings_advanced_genlist_callback_data *cb_data = (settings_advanced_genlist_callback_data *)data;
//	privacy_edit *edit = (privacy_edit *)cb_data->user_data;
	settings_advanced_menu_index menu_index = cb_data->menu_index;

	char *menu_text = NULL;
	BROWSER_LOGD("menu_index[%d]", menu_index);
	switch (menu_index) {
		case br_settings_sub_advanced_menu_privacy_remember_form_data:
			BROWSER_LOGD("here1");
			if (!strcmp(part, "elm.text.main"))
				menu_text = strdup(BR_STRING_SETTINGS_REMEMBER_FORM_DATA);
			else if (!strcmp(part, "elm.text.multiline"))
				menu_text = strdup(BR_STRING_SETTINGS_REMEMBER_FORM_DATA_DESC);
		break;

		case br_settings_sub_advanced_menu_privacy_remember_form_data_desc:
			BROWSER_LOGD("here2");
			if (!strcmp(part, "elm.text"))
				menu_text = strdup(BR_STRING_SETTINGS_REMEMBER_FORM_DATA_DESC);
		break;

		case br_settings_sub_advanced_menu_privacy_remember_passwords:
			BROWSER_LOGD("here3");
			if (!strcmp(part, "elm.text.main"))
				menu_text = strdup(BR_STRING_SETTINGS_REMEMBER_PASSWORDS);
			else if (!strcmp(part, "elm.text.multiline"))
				menu_text = strdup(BR_STRING_SETTINGS_REMEMBER_PASSWORDS_DESC);
		break;

		case br_settings_sub_advanced_menu_privacy_delete_personal_data:
			BROWSER_LOGD("here4");
			if (!strcmp(part, "elm.text.main.left"))
				menu_text = strdup(BR_STRING_SETTINGS_DELETE_PERSONAL_DATA);
			else if (!strcmp(part, "elm.text.multiline"))
				menu_text = strdup(BR_STRING_SETTINGS_DELETE_PERSONAL_DATA_DESC);
		break;

		default:
			BROWSER_LOGD("menu_index[%d] is out of allowed", menu_index);
		break;
	}

	return menu_text;
}

Evas_Object *privacy_edit::__genlist_contents_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	Evas_Object *content = NULL;
	settings_advanced_genlist_callback_data *callback_data = (settings_advanced_genlist_callback_data *)data;
	settings_advanced_menu_index menu_index = callback_data->menu_index;
	privacy_edit *edit = (privacy_edit *)(callback_data->user_data);

	switch (menu_index) {

		case br_settings_sub_advanced_menu_privacy_remember_form_data:
			if (!strcmp(part, "elm.icon")) {
				Evas_Object *check_box = elm_check_add(obj);
				if (check_box) {
					elm_object_style_set(check_box, "on&off");
					evas_object_smart_callback_add(check_box, "changed", __remember_form_data_check_changed_cb, edit);

					Eina_Bool remember_form = m_preference->get_remember_form_data_enabled();
					elm_check_state_set(check_box, remember_form);
					evas_object_propagate_events_set(check_box, EINA_FALSE);
				}
				content = check_box;
			}
		break;

		case br_settings_sub_advanced_menu_privacy_remember_passwords:
			if (!strcmp(part, "elm.icon")) {
				Evas_Object *check_box = elm_check_add(obj);
				if (check_box) {
					elm_object_style_set(check_box, "on&off");
					evas_object_smart_callback_add(check_box, "changed", __remember_passwords_check_changed_cb, edit);

					Eina_Bool remember_passwords = m_preference->get_remember_passwords_enabled();
					elm_check_state_set(check_box, remember_passwords);
					evas_object_propagate_events_set(check_box, EINA_FALSE);
				}
				content = check_box;
			}
		break;

		case br_settings_sub_advanced_menu_privacy_delete_personal_data:
		break;

		default:
			BROWSER_LOGD("menu_index[%d]is out of allowed", menu_index);
		break;
	}


	return content;
}

void privacy_edit::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_object_item_signal_emit(it, "play,touch_sound,signal", "");
	settings_advanced_genlist_callback_data *callback_data = (settings_advanced_genlist_callback_data *)data;
	settings_advanced_menu_index menu_index = callback_data->menu_index;
	privacy_edit *edit = (privacy_edit *)(callback_data->user_data);

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);

	switch (menu_index) {
		case br_settings_sub_advanced_menu_privacy_remember_form_data: {
			RET_MSG_IF(!event_info, "event_info is NULL");

			Evas_Object *check_box = elm_object_item_part_content_get((const Elm_Object_Item *)event_info, "elm.icon");
			Eina_Bool state = elm_check_state_get(check_box);
			elm_check_state_set(check_box, !state);

			edit->__remember_form_data_check_changed_cb(data, check_box, NULL);
		}
		break;

		case br_settings_sub_advanced_menu_privacy_remember_passwords: {
			RET_MSG_IF(!event_info, "event_info is NULL");

			Evas_Object *check_box = elm_object_item_part_content_get((const Elm_Object_Item *)event_info, "elm.icon");
			Eina_Bool state = elm_check_state_get(check_box);
			elm_check_state_set(check_box, !state);

			edit->__remember_passwords_check_changed_cb(data, check_box, NULL);
		}
		break;
		case br_settings_sub_advanced_menu_privacy_delete_personal_data:
			evas_object_freeze_events_set(edit->m_genlist, EINA_TRUE);
			edit->get_clear_private_data_view()->show();
		break;

		default:
		break;
	}
}

void privacy_edit::__genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *selected_item = (Elm_Object_Item *)(event_info);

	if (elm_genlist_item_select_mode_get(selected_item) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
		Elm_Object_Item *next_it = NULL;
		Elm_Object_Item *prev_it = NULL;
		next_it = elm_genlist_item_next_get(selected_item);
		prev_it = elm_genlist_item_prev_get(selected_item);

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


void privacy_edit::__title_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
}

void privacy_edit::__back_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	elm_naviframe_item_pop(m_naviframe);
}

void privacy_edit::__remember_form_data_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	Eina_Bool state = elm_check_state_get(obj);
	m_preference->set_remember_form_data_enabled(state);
}

void privacy_edit::__remember_passwords_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	Eina_Bool state = elm_check_state_get(obj);
	m_preference->set_remember_passwords_enabled(state);
}

void privacy_edit::__privacy_edit_naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	privacy_edit *edit = (privacy_edit *)data;
	if (edit->m_navi_item != elm_naviframe_top_item_get(m_naviframe))
		return;
}

screen_and_text_edit::screen_and_text_edit(void)
:
	m_genlist(NULL)
	, m_navi_item(NULL)
	, m_theme(NULL)
{
	BROWSER_LOGD("");

	m_theme = elm_theme_new();
	elm_theme_ref_set(m_theme, NULL);
	elm_theme_extension_add(m_theme, settings_edj_path);
}

screen_and_text_edit::~screen_and_text_edit(void)
{
	BROWSER_LOGD("");

	_destroy_genlist_styles();
	_destroy_genlist_callback_datas();

	elm_theme_extension_del(m_theme, settings_edj_path);
	elm_theme_free(m_theme);
}

Eina_Bool screen_and_text_edit::show(void)
{
	BROWSER_LOGD("");

	if (_create_main_layout(m_naviframe) == NULL) {
		BROWSER_LOGE("_create_main_layout failed");
		return EINA_FALSE;
	}

	m_navi_item = elm_naviframe_item_push(m_naviframe, "IDS_BR_BODY_SCREEN_AND_TEXT", NULL, NULL, m_genlist, NULL);//BR_STRING_SETTINGS_SCREEN_AND_TEXT
	elm_object_item_domain_text_translatable_set(m_navi_item, BROWSER_DOMAIN_NAME, EINA_TRUE);

//	elm_naviframe_item_pop_cb_set(m_navi_item, __naviframe_pop_cb, this);
//	evas_object_smart_callback_add(m_naviframe, "transition,finished", __naviframe_pop_finished_cb, this);

	return EINA_TRUE;
}

void screen_and_text_edit::rotate(void)
{
	BROWSER_LOGD("");
}

Evas_Object *screen_and_text_edit::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");

	m_genlist = elm_genlist_add(parent);
	if (!m_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	elm_genlist_mode_set(m_genlist, ELM_LIST_COMPRESS);

	evas_object_smart_callback_add(m_genlist, "realized", __genlist_item_realized_cb, this);
	evas_object_smart_callback_add(m_genlist, "language,changed", __genlist_lang_changed, NULL);

	_destroy_genlist_styles();
	_destroy_genlist_callback_datas();
	elm_object_theme_set(m_genlist, m_theme);

	for (unsigned short i = br_settings_sub_advanced_menu_screen_and_text_start; i < br_settings_sub_advanced_menu_screen_and_text_end; i++) {
		Elm_Genlist_Item_Class *item_class = _create_genlist_item_class((settings_advanced_menu_index)i);
		if (item_class) {
			settings_advanced_genlist_callback_data *cb_data = new settings_advanced_genlist_callback_data;
			memset(cb_data, 0x00, sizeof(settings_advanced_genlist_callback_data));

			cb_data->menu_index = (settings_advanced_menu_index)i;
			cb_data->user_data = this;

			if (settings_advanced_menus[i].is_title == EINA_TRUE) {
				cb_data->it = elm_genlist_item_append(m_genlist, item_class, (void *)cb_data, NULL,
													ELM_GENLIST_ITEM_NONE, NULL, NULL);
				elm_genlist_item_select_mode_set(cb_data->it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			} else {
				cb_data->it = elm_genlist_item_append(m_genlist, item_class, (void *)cb_data, NULL,
													ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, (void *)cb_data);
			}
			m_genlist_callback_data_list.push_back(cb_data);
		} else {
			BROWSER_LOGE("item_class is NULL");
			break;
		}
	}

	return m_genlist;
}

Elm_Genlist_Item_Class *screen_and_text_edit::_create_genlist_item_class(settings_advanced_menu_index index)
{
	BROWSER_LOGD("settings_advanced_menu_index[%d]", index);

	if ((index < br_settings_sub_advanced_menu_screen_and_text_start) || (index > br_settings_sub_advanced_menu_screen_and_text_end)) {
		BROWSER_LOGE("index is out of allowed");
		return NULL;
	}

	if (!settings_advanced_menus[index].item_style) {
		BROWSER_LOGE("unable to get screen_and_text_edit : settings_advanced_menus[i].item_style", index);
		return NULL;
	}

	Elm_Genlist_Item_Class *item_class = NULL;
	/* check duplicated item style */
	for (unsigned short i = 0; i < m_genlist_item_class_list.size(); i++) {
		if (strlen(m_genlist_item_class_list[i]->item_style) == strlen(settings_advanced_menus[index].item_style)
			&& !strncmp(m_genlist_item_class_list[i]->item_style, settings_advanced_menus[index].item_style, strlen(m_genlist_item_class_list[i]->item_style))) {
			BROWSER_LOGE("[%s] style is already created", settings_advanced_menus[index].item_style);
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
		item_class->item_style = settings_advanced_menus[index].item_style;
		item_class->func.text_get = __genlist_text_get;
		item_class->func.content_get = __genlist_contents_get;
		item_class->func.state_get = NULL;
		item_class->func.del = NULL;
		m_genlist_item_class_list.push_back(item_class);
	}

	return item_class;
}

void screen_and_text_edit::_destroy_genlist_styles(void)
{
	for (unsigned int i = 0; i < m_genlist_item_class_list.size(); i++) {
		if (m_genlist_item_class_list[i])
			elm_genlist_item_class_free(m_genlist_item_class_list[i]);
		m_genlist_item_class_list[i] = NULL;
	}

	m_genlist_item_class_list.clear();

}

void screen_and_text_edit::_destroy_genlist_callback_datas(void)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_genlist_callback_data_list.size(); i++) {
		if (m_genlist_callback_data_list[i])
			free(m_genlist_callback_data_list[i]);
		m_genlist_callback_data_list[i] = NULL;
	}

	m_genlist_callback_data_list.clear();
}

char *screen_and_text_edit::__genlist_text_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	settings_advanced_genlist_callback_data *cb_data = (settings_advanced_genlist_callback_data *)data;
	settings_advanced_menu_index menu_index = cb_data->menu_index;

	char *menu_text = NULL;
	BROWSER_LOGD("menu_index[%d]", menu_index);
	switch (menu_index) {
		default:
			BROWSER_LOGD("menu_index[%d]is out of allowed", menu_index);
		break;
	}
	return menu_text;
}

Evas_Object *screen_and_text_edit::__genlist_contents_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	Evas_Object *content = NULL;
	settings_advanced_genlist_callback_data *callback_data = (settings_advanced_genlist_callback_data *)data;
	settings_advanced_menu_index menu_index = callback_data->menu_index;

	switch (menu_index) {
		default:
			BROWSER_LOGD("menu_index[%d]is out of allowed", menu_index);
		break;
	}

	return content;
}

void screen_and_text_edit::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_object_item_signal_emit(it, "play,touch_sound,signal", "");
	settings_advanced_genlist_callback_data *callback_data = (settings_advanced_genlist_callback_data *)data;
	settings_advanced_menu_index menu_index = callback_data->menu_index;

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);

	switch (menu_index) {
		default:
		break;
	}
}

void screen_and_text_edit::__genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *selected_item = (Elm_Object_Item *)(event_info);

	if (elm_genlist_item_select_mode_get(selected_item) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
		Elm_Object_Item *next_it = NULL;
		Elm_Object_Item *prev_it = NULL;
		next_it = elm_genlist_item_next_get(selected_item);
		prev_it = elm_genlist_item_prev_get(selected_item);

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

contents_settings_edit::contents_settings_edit(void)
:
	m_default_storage_ask_popup(NULL)
	, m_genlist(NULL)
	, m_enable_drag_and_drop_check(NULL)
	, m_drag_and_drop_desc_popup(NULL)
	, m_reset_to_default_popup(NULL)
	, m_navi_item(NULL)
	, m_theme(NULL)
{
	BROWSER_LOGD("");

	/*m_theme = elm_theme_new();
	elm_theme_ref_set(m_theme, NULL);
	elm_theme_extension_add(m_theme, settings_edj_path);*/

	int mmc_mode = VCONFKEY_SYSMAN_MMC_REMOVED;
	if (vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &mmc_mode) != 0)
		BROWSER_LOGE("Fail to get vconf_get_int : VCONFKEY_SYSMAN_MMC_STATUS");

	if (mmc_mode == -1) /* This values also means unmounted mmc */
		mmc_mode = VCONFKEY_SYSMAN_MMC_REMOVED;

	if (mmc_mode == VCONFKEY_SYSMAN_MMC_REMOVED)
		m_preference->set_default_storage_type(DEFAULT_STORAGE_TYPE_PHONE);
}

contents_settings_edit::~contents_settings_edit(void)
{
	BROWSER_LOGD("");

	evas_object_smart_callback_del(m_naviframe, "transition,finished", __naviframe_pop_finished_cb);
	delete_drag_and_drop_desc_popup();
	delete_default_storage_ask_popup();
	delete_reset_to_default_ask_popup();

	_destroy_genlist_styles();
	_destroy_genlist_callback_datas();

	//elm_theme_extension_del(m_theme, settings_edj_path);
	//elm_theme_free(m_theme);
}

Eina_Bool contents_settings_edit::show(void)
{
	BROWSER_LOGD("");

	if (_create_main_layout(m_naviframe) == NULL) {
		BROWSER_LOGE("_create_main_layout failed");
		return EINA_FALSE;
	}

	m_navi_item = elm_naviframe_item_push(m_naviframe, "IDS_BR_BODY_CONTENT_SETTINGS", NULL, NULL, m_genlist, NULL);//BR_STRING_SETTINGS_CONTENT_SETTINGS
	elm_object_item_domain_text_translatable_set(m_navi_item, BROWSER_DOMAIN_NAME, EINA_TRUE);
	evas_object_smart_callback_add(m_naviframe, "transition,finished", __naviframe_pop_finished_cb, this);

	return EINA_TRUE;
}

default_storage_ask_popup *contents_settings_edit::get_default_storage_ask_popup(void)
{
	BROWSER_LOGD("");
	if (!m_default_storage_ask_popup)
		m_default_storage_ask_popup = new default_storage_ask_popup();

	return m_default_storage_ask_popup;
}

void contents_settings_edit::delete_default_storage_ask_popup(void)
{
	BROWSER_LOGD("");

	if (m_default_storage_ask_popup)
		delete m_default_storage_ask_popup;
	m_default_storage_ask_popup = NULL;
}

void contents_settings_edit::delete_drag_and_drop_desc_popup(void)
{
	BROWSER_LOGD("");

	if (m_drag_and_drop_desc_popup != NULL)
		evas_object_del(m_drag_and_drop_desc_popup);
	m_drag_and_drop_desc_popup = NULL;
}

void contents_settings_edit::delete_reset_to_default_ask_popup(void)
{
	BROWSER_LOGD("");

	if (m_reset_to_default_popup != NULL)
		evas_object_del(m_reset_to_default_popup);
	m_reset_to_default_popup = NULL;
}

void contents_settings_edit::mdm_mode_disable_accept_cookie_setting_menu(Eina_Bool disabled)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_genlist_callback_data_list.size(); i++) {
		if (m_genlist_callback_data_list[i]->it) {
			if (m_genlist_callback_data_list[i]->menu_index == br_settings_sub_advanced_menu_contents_accept_cookies) {
				BROWSER_LOGD("MDM restricted accept cookie");
				elm_object_item_disabled_set(m_genlist_callback_data_list[i]->it, disabled);
				break;
			}
		}
	}
}

void contents_settings_edit::mdm_mode_disable_javascript_setting_menu(Eina_Bool disabled)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_genlist_callback_data_list.size(); i++) {
		if (m_genlist_callback_data_list[i]->it) {
			if (m_genlist_callback_data_list[i]->menu_index == br_settings_sub_advanced_menu_contents_enable_javascript) {
				BROWSER_LOGD("MDM restricted javscript");
				elm_object_item_disabled_set(m_genlist_callback_data_list[i]->it, disabled);
				break;
			}
		}
	}
}

Evas_Object *contents_settings_edit::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");

	m_genlist = elm_genlist_add(parent);
	if (!m_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	elm_genlist_mode_set(m_genlist, ELM_LIST_COMPRESS);

	evas_object_smart_callback_add(m_genlist, "realized", __genlist_item_realized_cb, this);
	evas_object_smart_callback_add(m_genlist, "language,changed", __genlist_lang_changed, NULL);

	_destroy_genlist_styles();
	_destroy_genlist_callback_datas();
	//elm_object_theme_set(m_genlist, m_theme);

	for (unsigned short i = br_settings_sub_advanced_menu_contents_start; i < br_settings_sub_advanced_menu_contents_end; i++) {
		Elm_Genlist_Item_Class *item_class = _create_genlist_item_class((settings_advanced_menu_index)i);
		if (item_class) {
			settings_advanced_genlist_callback_data *cb_data = new settings_advanced_genlist_callback_data;
			memset(cb_data, 0x00, sizeof(settings_advanced_genlist_callback_data));

			cb_data->menu_index = (settings_advanced_menu_index)i;
			cb_data->user_data = this;

			if (settings_advanced_menus[i].is_title == EINA_TRUE) {
				cb_data->it = elm_genlist_item_append(m_genlist, item_class, (void *)cb_data, NULL,
													ELM_GENLIST_ITEM_NONE, NULL, NULL);
				elm_genlist_item_select_mode_set(cb_data->it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			} else {
				cb_data->it = elm_genlist_item_append(m_genlist, item_class, (void *)cb_data, NULL,
													ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, (void *)cb_data);
			}

			m_genlist_callback_data_list.push_back(cb_data);
		} else {
			BROWSER_LOGE("item_class is NULL");
			break;
		}
	}
	return m_genlist;
}

Elm_Genlist_Item_Class *contents_settings_edit::_create_genlist_item_class(settings_advanced_menu_index index)
{
	BROWSER_LOGD("settings_advanced_menu_index[%d]", index);

	if ((index < br_settings_sub_advanced_menu_contents_start) || (index > br_settings_sub_advanced_menu_contents_end)) {
		BROWSER_LOGE("index is out of allowed");
		return NULL;
	}

	if (!settings_advanced_menus[index].item_style) {
		BROWSER_LOGE("unable to get contents_settings_edit : settings_advanced_menus[i].item_style", index);
		return NULL;
	}

	Elm_Genlist_Item_Class *item_class = NULL;
	/* check duplicated item style */
	for (unsigned short i = 0; i < m_genlist_item_class_list.size(); i++) {
		if (strlen(m_genlist_item_class_list[i]->item_style) == strlen(settings_advanced_menus[index].item_style)
			&& !strncmp(m_genlist_item_class_list[i]->item_style, settings_advanced_menus[index].item_style, strlen(m_genlist_item_class_list[i]->item_style))) {
			BROWSER_LOGE("[%s] style is already created", settings_advanced_menus[index].item_style);
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
		item_class->item_style = settings_advanced_menus[index].item_style;
		item_class->func.text_get = __genlist_text_get;
		item_class->func.content_get = __genlist_contents_get;
		item_class->func.state_get = NULL;
		item_class->func.del = NULL;
		m_genlist_item_class_list.push_back(item_class);
	}

	return item_class;
}

void contents_settings_edit::_destroy_genlist_styles(void)
{
	for (unsigned int i = 0; i < m_genlist_item_class_list.size(); i++) {
		if (m_genlist_item_class_list[i])
			elm_genlist_item_class_free(m_genlist_item_class_list[i]);
		m_genlist_item_class_list[i] = NULL;
	}

	m_genlist_item_class_list.clear();

}

void contents_settings_edit::_destroy_genlist_callback_datas(void)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_genlist_callback_data_list.size(); i++) {
		if (m_genlist_callback_data_list[i])
			free(m_genlist_callback_data_list[i]);
		m_genlist_callback_data_list[i] = NULL;
	}

	m_genlist_callback_data_list.clear();
}

void contents_settings_edit::_reset_to_default_setting(void)
{
	BROWSER_LOGD("");

	m_preference->reset();

	if (vconf_set_str(VCONFKEY_BROWSER_BROWSER_USER_AGENT, "System user agent") < 0)
		BROWSER_LOGE("vconf_set_str(VCONFKEY_BROWSER_BROWSER_USER_AGENT, [System user agent]) failed");

	realize_genlist();
	if (m_browser->get_settings()->is_shown())
		m_browser->get_settings()->realize_genlist();
	m_browser->get_browser_view()->get_url_input_bar()->update_search_engine(SEARCH_ENGINE_NOT_SELECTED);
}

char *contents_settings_edit::__genlist_text_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	settings_advanced_genlist_callback_data *cb_data = (settings_advanced_genlist_callback_data *)data;
	settings_advanced_menu_index menu_index = cb_data->menu_index;

	char *menu_text = NULL;
	BROWSER_LOGD("menu_index[%d]", menu_index);
	switch (menu_index) {
		case br_settings_sub_advanced_menu_contents_accept_cookies:
			if (!strcmp(part, "elm.text.main"))
				menu_text = strdup(BR_STRING_SETTINGS_ACCEPT_COOKIES);
			else if (!strcmp(part, "elm.text.multiline"))
				menu_text = strdup(BR_STRING_ACCEPT_COOKIES_DESC);
		break;

		case br_settings_sub_advanced_menu_contents_enable_javascript:
			if (!strcmp(part, "elm.text.main"))
				menu_text = strdup(BR_STRING_SETTINGS_ENABLE_JAVASCRIPT);
			else if (!strcmp(part, "elm.text.multiline"))
				menu_text = strdup(BR_STRING_SETTINGS_ENABLE_JAVASCRIPT_DESC);
		break;

		case br_settings_sub_advanced_menu_contents_default_storage:
			if (!strcmp(part, "elm.text.main"))
				menu_text = strdup(BR_STRING_DEFAULT_STORAGE);
			else if (!strcmp(part, "elm.text.multiline")) {
				if (m_preference->get_default_storage_type() == DEFAULT_STORAGE_TYPE_MEMORY_CARD)
					menu_text = strdup(BR_STRING_SETTINGS_MEMORY_CARD);
				else
					menu_text = strdup(BR_STRING_SETTINGS_PHONE);
			}
		break;

		case br_settings_sub_advanced_menu_contents_reset_to_default_title:
			if (!strcmp(part, "elm.text.main"))
				menu_text = strdup(BR_STRING_SETTINGS_RESET_SETTINGS);
		break;

		case br_settings_sub_advanced_menu_contents_reset_to_default:
			if (!strcmp(part, "elm.text.main"))
				menu_text = strdup(BR_STRING_SETTINGS_RESET_SETTINGS);
			else if (!strcmp(part, "elm.text.main.left"))
				menu_text = strdup(BR_STRING_SETTINGS_RESET_SETTINGS);
			else if (!strcmp(part, "elm.text.multiline"))
					menu_text = strdup(BR_STRING_RESTORE_DEFAULT_DESC);
		break;

		default:
			BROWSER_LOGD("menu_index[%d]is out of allowed", menu_index);
		break;
	}

	return menu_text;
}

Evas_Object *contents_settings_edit::__genlist_contents_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	Evas_Object *content = NULL;
	settings_advanced_genlist_callback_data *callback_data = (settings_advanced_genlist_callback_data *)data;
	settings_advanced_menu_index menu_index = callback_data->menu_index;
	contents_settings_edit *edit = (contents_settings_edit *)(callback_data->user_data);

	switch (menu_index) {
		case br_settings_sub_advanced_menu_contents_accept_cookies:
			if (!strcmp(part, "elm.icon")) {
				Evas_Object *check_box = elm_check_add(obj);
				if (check_box) {
					elm_object_style_set(check_box, "on&off");
					evas_object_smart_callback_add(check_box, "changed", __accept_cookies_check_changed_cb, edit);

					Eina_Bool accept_cookie = m_preference->get_accept_cookies_enabled();
					elm_check_state_set(check_box, accept_cookie);
					evas_object_propagate_events_set(check_box, EINA_FALSE);
				}
				content = check_box;
			}
		break;

		case br_settings_sub_advanced_menu_contents_enable_javascript:
			if (!strcmp(part, "elm.icon")) {
				Evas_Object *check_box = elm_check_add(obj);
				if (check_box) {
					elm_object_style_set(check_box, "on&off");
					evas_object_smart_callback_add(check_box, "changed", __run_javascript_check_changed_cb, edit);

					Eina_Bool javascript_enabled = m_preference->get_javascript_enabled();
					elm_check_state_set(check_box, javascript_enabled);
					evas_object_propagate_events_set(check_box, EINA_FALSE);
				}
				content = check_box;
			}
		break;

		case br_settings_sub_advanced_menu_contents_default_storage:
		break;

		default:
			BROWSER_LOGD("menu_index[%d]is out of allowed", menu_index);
		break;
	}

	return content;
}

void contents_settings_edit::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_object_item_signal_emit(it, "play,touch_sound,signal", "");
	settings_advanced_genlist_callback_data *callback_data = (settings_advanced_genlist_callback_data *)data;
	settings_advanced_menu_index menu_index = callback_data->menu_index;
	contents_settings_edit *edit = (contents_settings_edit *)(callback_data->user_data);

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);

	switch (menu_index) {
		case br_settings_sub_advanced_menu_contents_accept_cookies: {
			RET_MSG_IF(!event_info, "event_info is NULL");
			Evas_Object *check_box = elm_object_item_part_content_get((const Elm_Object_Item *)event_info, "elm.icon");
			Eina_Bool state = elm_check_state_get(check_box);
			elm_check_state_set(check_box, !state);

			edit->__accept_cookies_check_changed_cb(edit, check_box, NULL);
		}
		break;

		case br_settings_sub_advanced_menu_contents_enable_javascript: {
			RET_MSG_IF(!event_info, "event_info is NULL");
			Evas_Object *check_box = elm_object_item_part_content_get((const Elm_Object_Item *)event_info, "elm.icon");
			Eina_Bool state = elm_check_state_get(check_box);
			elm_check_state_set(check_box, !state);

			edit->__run_javascript_check_changed_cb(edit, check_box, NULL);
		}
		break;

		case br_settings_sub_advanced_menu_contents_default_storage:
			evas_object_freeze_events_set(edit->m_genlist, EINA_TRUE);
			edit->get_default_storage_ask_popup()->show();
		break;

		case br_settings_sub_advanced_menu_contents_reset_to_default:
			evas_object_freeze_events_set(edit->m_genlist, EINA_TRUE);
			edit->m_reset_to_default_popup = edit->show_msg_popup(BR_STRING_SETTINGS_RESET_SETTINGS, BR_STRING_SETTINGS_RESET_MSG, __reset_confirm_cancel_cb,
																"IDS_BR_SK_CANCEL", __reset_confirm_cancel_cb,
																"IDS_BR_BUTTON_RESET_ABB", __reset_confirm_response_cb,
																edit);
		break;

		default:
		break;
	}
}

void contents_settings_edit::__genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *selected_item = (Elm_Object_Item *)(event_info);

	if (elm_genlist_item_select_mode_get(selected_item) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
		Elm_Object_Item *next_it = NULL;
		Elm_Object_Item *prev_it = NULL;
		next_it = elm_genlist_item_next_get(selected_item);
		prev_it = elm_genlist_item_prev_get(selected_item);

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

void contents_settings_edit::__accept_cookies_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Eina_Bool state = elm_check_state_get(obj);
	m_preference->set_accept_cookies_enabled(state);
}

void contents_settings_edit::__run_javascript_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Eina_Bool state = elm_check_state_get(obj);
	m_preference->set_javascript_enabled(state);
}

void contents_settings_edit::__default_storage_reveal_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	contents_settings_edit *edit = (contents_settings_edit *)data;
	edit->get_default_storage_ask_popup()->show();
}

void contents_settings_edit::__reset_to_default_reveal_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	contents_settings_edit *edit = (contents_settings_edit *)data;

	edit->m_reset_to_default_popup = edit->show_msg_popup("IDS_BR_BODY_RESET_TO_DEFAULT", "IDS_BR_BODY_RESTORE_DEFAULT_SETTINGS_Q", __reset_confirm_cancel_cb,
														"IDS_BR_SK_CANCEL", __reset_confirm_cancel_cb,
														"IDS_BR_BUTTON_RESET_ABB", __reset_confirm_response_cb,
														data);
}

void contents_settings_edit::__reset_confirm_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	contents_settings_edit *edit = (contents_settings_edit *)data;
	edit->_reset_to_default_setting();
	edit->m_reset_to_default_popup = NULL;
	evas_object_freeze_events_set(edit->m_genlist, EINA_FALSE);
}

void contents_settings_edit::__reset_confirm_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	contents_settings_edit *edit = (contents_settings_edit *)data;
	edit->m_reset_to_default_popup = NULL;
	evas_object_freeze_events_set(edit->m_genlist, EINA_FALSE);
}

void contents_settings_edit::__naviframe_pop_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
}

bandwidth_management_edit::bandwidth_management_edit(void)
:
	m_genlist(NULL)
	, m_navi_item(NULL)
	, m_theme(NULL)
{
	BROWSER_LOGD("");

	/*m_theme = elm_theme_new();
	elm_theme_ref_set(m_theme, NULL);
	elm_theme_extension_add(m_theme, settings_edj_path);*/
}

bandwidth_management_edit::~bandwidth_management_edit(void)
{
	BROWSER_LOGD("");

	_destroy_genlist_styles();
	_destroy_genlist_callback_datas();

	//elm_theme_extension_del(m_theme, settings_edj_path);
	//elm_theme_free(m_theme);
}

Eina_Bool bandwidth_management_edit::show(void)
{
	BROWSER_LOGD("");

	if (_create_main_layout(m_naviframe) == NULL) {
		BROWSER_LOGE("_create_main_layout failed");
		return EINA_FALSE;
	}

	m_navi_item = elm_naviframe_item_push(m_naviframe, "IDS_BR_HEADER_BANDWIDTH_MANAGEMENT_ABB", NULL, NULL, m_genlist, NULL);//BR_STRING_SETTINGS_BANDWIDTH_MANAGEMENT
	elm_object_item_domain_text_translatable_set(m_navi_item, BROWSER_DOMAIN_NAME, EINA_TRUE);

	return EINA_TRUE;
}

Evas_Object *bandwidth_management_edit::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");

	m_genlist = elm_genlist_add(parent);
	if (!m_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}
	elm_genlist_mode_set(m_genlist, ELM_LIST_COMPRESS);

	evas_object_smart_callback_add(m_genlist, "realized", __genlist_item_realized_cb, this);
	evas_object_smart_callback_add(m_genlist, "language,changed", __genlist_lang_changed, NULL);

	_destroy_genlist_styles();
	_destroy_genlist_callback_datas();
	//elm_object_theme_set(m_genlist, m_theme);

	for (unsigned short i = br_settings_sub_advanced_menu_bandwidth_management_start; i < br_settings_sub_advanced_menu_bandwidth_management_end; i++) {
		Elm_Genlist_Item_Class *item_class = _create_genlist_item_class((settings_advanced_menu_index)i);
		if (item_class) {
			settings_advanced_genlist_callback_data *cb_data = new settings_advanced_genlist_callback_data;
			memset(cb_data, 0x00, sizeof(settings_advanced_genlist_callback_data));

			cb_data->menu_index = (settings_advanced_menu_index)i;
			cb_data->user_data = this;

			if (settings_advanced_menus[i].is_title == EINA_TRUE) {
				cb_data->it = elm_genlist_item_append(m_genlist, item_class, (void *)cb_data, NULL,
													ELM_GENLIST_ITEM_NONE, NULL, NULL);
				elm_genlist_item_select_mode_set(cb_data->it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			} else {
				cb_data->it = elm_genlist_item_append(m_genlist, item_class, (void *)cb_data, NULL,
													ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, (void *)cb_data);
			}
			m_genlist_callback_data_list.push_back(cb_data);
		} else {
			BROWSER_LOGE("item_class is NULL");
			break;
		}
	}

	return m_genlist;
}

Elm_Genlist_Item_Class *bandwidth_management_edit::_create_genlist_item_class(settings_advanced_menu_index index)
{
	BROWSER_LOGD("settings_advanced_menu_index[%d]", index);

	if ((index < br_settings_sub_advanced_menu_bandwidth_management_start) || (index > br_settings_sub_advanced_menu_bandwidth_management_end)) {
		BROWSER_LOGE("index is out of allowed");
		return NULL;
	}

	if (!settings_advanced_menus[index].item_style) {
		BROWSER_LOGE("unable to get bandwidth_management_edit : settings_advanced_menus[i].item_style", index);
		return NULL;
	}

	Elm_Genlist_Item_Class *item_class = NULL;
	/* check duplicated item style */
	for (unsigned short i = 0; i < m_genlist_item_class_list.size(); i++) {
		if (strlen(m_genlist_item_class_list[i]->item_style) == strlen(settings_advanced_menus[index].item_style)
			&& !strncmp(m_genlist_item_class_list[i]->item_style, settings_advanced_menus[index].item_style, strlen(m_genlist_item_class_list[i]->item_style))) {
			BROWSER_LOGE("[%s] style is already created", settings_advanced_menus[index].item_style);
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
		item_class->item_style = settings_advanced_menus[index].item_style;
		item_class->func.text_get = __genlist_text_get;
		item_class->func.content_get = __genlist_contents_get;
		item_class->func.state_get = NULL;
		item_class->func.del = NULL;
		m_genlist_item_class_list.push_back(item_class);
	}

	return item_class;
}

void bandwidth_management_edit::_destroy_genlist_styles(void)
{
	for (unsigned int i = 0; i < m_genlist_item_class_list.size(); i++) {
		if (m_genlist_item_class_list[i])
			elm_genlist_item_class_free(m_genlist_item_class_list[i]);
		m_genlist_item_class_list[i] = NULL;
	}

	m_genlist_item_class_list.clear();

}

void bandwidth_management_edit::_destroy_genlist_callback_datas(void)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_genlist_callback_data_list.size(); i++) {
		if (m_genlist_callback_data_list[i])
			free(m_genlist_callback_data_list[i]);
		m_genlist_callback_data_list[i] = NULL;
	}

	m_genlist_callback_data_list.clear();
}

char *bandwidth_management_edit::__genlist_text_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	settings_advanced_genlist_callback_data *cb_data = (settings_advanced_genlist_callback_data *)data;
	settings_advanced_menu_index menu_index = cb_data->menu_index;

	char *menu_text = NULL;
	BROWSER_LOGD("menu_index[%d]", menu_index);
	switch (menu_index) {
		case br_settings_sub_advanced_menu_bandwidth_management_load_images:
			if (!strcmp(part, "elm.text.main"))
				menu_text = strdup(BR_STRING_SETTINGS_LOAD_IMAGES);
			else if (!strcmp(part, "elm.text.multiline"))
				menu_text = strdup(BR_STRING_SETTINGS_LOAD_IMAGES_DESC);
		break;

		case br_settings_sub_advanced_menu_bandwidth_management_load_images_desc:
			if (!strcmp(part, "elm.text.multiline"))
				menu_text = strdup(BR_STRING_SETTINGS_LOAD_IMAGES_DESC);
		break;

		case br_settings_sub_advanced_menu_bandwidth_management_open_pages_in_overview:
			if (!strcmp(part, "elm.text.main"))
				menu_text = strdup(BR_STRING_SETTINGS_OPEN_PAGES_IN_OVERVIEW);
			else if (!strcmp(part, "elm.text.multiline"))
				menu_text = strdup(BR_STRING_SETTINGS_OPEN_PAGES_IN_OVERVIEW_DESC);
		break;

		case br_settings_sub_advanced_menu_bandwidth_management_open_pages_in_overview_desc:
			if (!strcmp(part, "elm.text.multiline"))
				menu_text = strdup(BR_STRING_SETTINGS_OPEN_PAGES_IN_OVERVIEW_DESC);
		break;

		default:
			BROWSER_LOGD("menu_index[%d]is out of allowed", menu_index);
		break;
	}

	return menu_text;
}

Evas_Object *bandwidth_management_edit::__genlist_contents_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	Evas_Object *content = NULL;
	settings_advanced_genlist_callback_data *callback_data = (settings_advanced_genlist_callback_data *)data;
	settings_advanced_menu_index menu_index = callback_data->menu_index;
	bandwidth_management_edit *edit = (bandwidth_management_edit *)(callback_data->user_data);

	switch (menu_index) {
		case br_settings_sub_advanced_menu_bandwidth_management_load_images:
			if (!strcmp(part, "elm.icon")) {
				Evas_Object *check_box = elm_check_add(obj);
				if (check_box) {
					elm_object_style_set(check_box, "on&off");
					evas_object_smart_callback_add(check_box, "changed", __load_images_check_changed_cb, edit);

					Eina_Bool load_image = m_preference->get_display_images_enabled();
					elm_check_state_set(check_box, load_image);
					evas_object_propagate_events_set(check_box, EINA_FALSE);
				}
				content = check_box;
			} else if (!strcmp(part, "elm.icon.1")) {

			} else if (!strcmp(part, "elm.icon.2")) {

			}
		break;

		case br_settings_sub_advanced_menu_bandwidth_management_open_pages_in_overview:
			if (!strcmp(part, "elm.icon")) {
				Evas_Object *check_box = elm_check_add(obj);
				if (check_box) {
					elm_object_style_set(check_box, "on&off");
					evas_object_smart_callback_add(check_box, "changed", __open_pages_in_overview_check_changed_cb, edit);

					Eina_Bool open_pages_in_overview = m_preference->get_open_pages_in_overview_enabled();
					elm_check_state_set(check_box, open_pages_in_overview);
					evas_object_propagate_events_set(check_box, EINA_FALSE);
				}
				content = check_box;
			} else if (!strcmp(part, "elm.icon.1")) {

			} else if (!strcmp(part, "elm.icon.2")) {

			}
		break;

		default:
			BROWSER_LOGD("menu_index[%d]is out of allowed", menu_index);
		break;
	}

	return content;
}

void bandwidth_management_edit::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_object_item_signal_emit(it, "play,touch_sound,signal", "");
	settings_advanced_genlist_callback_data *callback_data = (settings_advanced_genlist_callback_data *)data;
	settings_advanced_menu_index menu_index = callback_data->menu_index;
	bandwidth_management_edit *edit = (bandwidth_management_edit *)(callback_data->user_data);

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);

	switch (menu_index) {
		case br_settings_sub_advanced_menu_bandwidth_management_load_images: {
			RET_MSG_IF(!event_info, "event_info is NULL");
			Evas_Object *check_box = elm_object_item_part_content_get((const Elm_Object_Item *)event_info, "elm.icon");
			Eina_Bool state = elm_check_state_get(check_box);
			elm_check_state_set(check_box, !state);

			edit->__load_images_check_changed_cb(data, check_box, NULL);
		}
		break;
		case br_settings_sub_advanced_menu_bandwidth_management_open_pages_in_overview: {
			RET_MSG_IF(!event_info, "event_info is NULL");
			Evas_Object *check_box = elm_object_item_part_content_get((const Elm_Object_Item *)event_info, "elm.icon");
			Eina_Bool state = elm_check_state_get(check_box);
			elm_check_state_set(check_box, !state);
			edit->__open_pages_in_overview_check_changed_cb(data, check_box, NULL);
		}
		break;
		default:
		break;
	}
}

void bandwidth_management_edit::__genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *selected_item = (Elm_Object_Item *)(event_info);

	if (elm_genlist_item_select_mode_get(selected_item) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
		Elm_Object_Item *next_it = NULL;
		Elm_Object_Item *prev_it = NULL;
		next_it = elm_genlist_item_next_get(selected_item);
		prev_it = elm_genlist_item_prev_get(selected_item);

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

void bandwidth_management_edit::__load_images_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Eina_Bool state = elm_check_state_get(obj);
	m_preference->set_display_images_enabled(state);
}

void bandwidth_management_edit::__open_pages_in_overview_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Eina_Bool state = elm_check_state_get(obj);
	m_preference->set_open_pages_in_overview_enabled(state);
}

default_storage_ask_popup::default_storage_ask_popup(void)
:
	m_selected_type(DEFAULT_STORAGE_TYPE_PHONE),
	m_radio_main(NULL),
	m_popup(NULL),
	m_popup_last_item(NULL),
	m_mmc_mode(VCONFKEY_SYSMAN_MMC_REMOVED)
{
	BROWSER_LOGD("");

	if (vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &m_mmc_mode) != 0)
		BROWSER_LOGE("Fail to get vconf_get_int : VCONFKEY_SYSMAN_MMC_STATUS");

	if (m_mmc_mode == -1) /* This values also means unmounted mmc */
		m_mmc_mode = VCONFKEY_SYSMAN_MMC_REMOVED;
}

default_storage_ask_popup::~default_storage_ask_popup(void)
{
	BROWSER_LOGD("");

	_destroy_genlist_callback_datas();

	if (m_popup)
		evas_object_del(m_popup);
	m_popup = NULL;
}

Eina_Bool default_storage_ask_popup::show(void)
{
	BROWSER_LOGD("");

	Evas_Object *popup = brui_popup_add(m_window);
	RETV_MSG_IF(!popup, EINA_FALSE, "popup is NULL");
	m_popup = popup;

#if defined(HW_MORE_BACK_KEY)
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __cancel_button_cb, this);
#endif

	elm_object_style_set(popup,"default");
	elm_object_part_text_set(popup, "title,text", BR_STRING_DEFAULT_STORAGE);// FIXME: temporary fix waiting for string
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(popup, "block,clicked", __cancel_button_cb, this);

	Evas_Object *genlist = _create_genlist(popup);
	Evas_Object *box = elm_box_add(popup);
	evas_object_size_hint_min_set(box, 0, genlist_popup_calculate_height_with_margin(genlist, popup_with_only_top));
	evas_object_show(genlist);
	elm_box_pack_end(box, genlist);
	elm_object_content_set(popup, box);
	evas_object_show(popup);

	return EINA_TRUE;
}

void default_storage_ask_popup::__genlist_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!(data), "data is NULL");
	RET_MSG_IF(!(event_info), "event_info is NULL");

	default_storage_ask_popup *popup_class = (default_storage_ask_popup *)data;
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	if (it == popup_class->m_popup_last_item)
		elm_object_item_signal_emit(it, "elm,state,bottomline,hide", "");
}

Evas_Object *default_storage_ask_popup::_create_genlist(Evas_Object *parent)
{
	BROWSER_LOGD("");

	Evas_Object *genlist = elm_genlist_add(parent);
	if (!genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}

	evas_object_smart_callback_add(genlist, "realized", __genlist_realized_cb, this);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	m_radio_main = elm_radio_add(genlist);
	if (!m_radio_main) {
		BROWSER_LOGE("elm_radio_add failed");
		return NULL;
	}
	elm_radio_state_value_set(m_radio_main, 0);
	elm_radio_value_set(m_radio_main, 0);
	elm_access_object_unregister(m_radio_main);

	Elm_Genlist_Item_Class *item_ic = elm_genlist_item_class_new();

	if (item_ic) {
		item_ic->item_style = "1line";
		item_ic->func.text_get = __genlist_text_get;
		item_ic->func.content_get = __genlist_contents_get;
		item_ic->func.state_get = NULL;
		item_ic->func.del = NULL;
	}

	for (int i = 0; i < DEFAULT_STORAGE_TYPE_NUM; i++) {
		settings_advanced_genlist_callback_data *cb_data = new settings_advanced_genlist_callback_data;
		memset(cb_data, 0x00, sizeof(settings_advanced_genlist_callback_data));

		cb_data->menu_index = (settings_advanced_menu_index)i;
		cb_data->user_data = this;
		cb_data->it = elm_genlist_item_append(genlist, item_ic, cb_data, NULL, ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, cb_data);
		m_popup_last_item = cb_data->it;
		m_genlist_callback_data_list.push_back(cb_data);

		if (i == DEFAULT_STORAGE_TYPE_MEMORY_CARD) {
			if (m_mmc_mode == VCONFKEY_SYSMAN_MMC_REMOVED)
				elm_object_item_disabled_set(cb_data->it, EINA_TRUE);
		}
	}
	elm_genlist_item_class_free(item_ic);

	return genlist;
}

void default_storage_ask_popup::_destroy_genlist_callback_datas(void)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_genlist_callback_data_list.size(); i++) {
		if (m_genlist_callback_data_list[i])
			free(m_genlist_callback_data_list[i]);
		m_genlist_callback_data_list[i] = NULL;
	}

	m_genlist_callback_data_list.clear();
}

char *default_storage_ask_popup::__genlist_text_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, NULL, "data is NULL");

	settings_advanced_genlist_callback_data *callback_data = (settings_advanced_genlist_callback_data *)data;
	default_storage_type type = (default_storage_type)callback_data->menu_index;

	char *label = NULL;
	if (!strcmp(part, "elm.text.main.left")) {
		if (type == DEFAULT_STORAGE_TYPE_MEMORY_CARD)
			label = strdup(BR_STRING_SETTINGS_MEMORY_CARD);
		else if (type == DEFAULT_STORAGE_TYPE_PHONE)
			label = strdup(BR_STRING_SETTINGS_PHONE);
	}

	return label;
}

Evas_Object *default_storage_ask_popup::__genlist_contents_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!data, NULL, "data is NULL");

	settings_advanced_genlist_callback_data *callback_data = (settings_advanced_genlist_callback_data *)data;
	default_storage_ask_popup *popup = (default_storage_ask_popup *)(callback_data->user_data);

	Evas_Object *content = NULL;
	if (!strcmp(part, "elm.icon.right")) {
		Evas_Object *radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, callback_data->menu_index);
		elm_radio_group_add(radio, popup->m_radio_main);

		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_propagate_events_set(radio, EINA_FALSE);

		elm_radio_value_set(popup->m_radio_main, m_preference->get_default_storage_type());

		evas_object_smart_callback_add(radio, "changed", __radio_icon_changed_cb, (void *)data);

		content = radio;
	}

	return content;
}

void default_storage_ask_popup::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	settings_advanced_genlist_callback_data *callback_data = (settings_advanced_genlist_callback_data *)data;
	default_storage_type type = (default_storage_type)callback_data->menu_index;
	default_storage_ask_popup *popup_class = (default_storage_ask_popup *)(callback_data->user_data);
	Elm_Object_Item *item = callback_data->it;

	if (item) {
		elm_genlist_item_selected_set(item, EINA_FALSE);

		Evas_Object *radio = elm_object_item_part_content_get(item, "elm.icon");
		if (radio) {
			elm_radio_state_value_set(radio, type);
			elm_radio_value_set(popup_class->m_radio_main, type);
		}
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}

	popup_class->m_selected_type = type;
	m_preference->set_default_storage_type(popup_class->m_selected_type);
	m_browser->get_settings()->get_contents_settings_edit()->realize_genlist();

	if (popup_class->m_popup) {
		evas_object_del(popup_class->m_popup);
		popup_class->m_popup = NULL;
	}
	evas_object_freeze_events_set(m_browser->get_settings()->get_contents_settings_edit()->get_genlist(), EINA_FALSE);
}

void default_storage_ask_popup::__radio_icon_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	settings_advanced_genlist_callback_data *callback_data = (settings_advanced_genlist_callback_data *)data;
	default_storage_type type = (default_storage_type)callback_data->menu_index;
	default_storage_ask_popup *popup_class = (default_storage_ask_popup *)(callback_data->user_data);
	Elm_Object_Item *item = callback_data->it;

	if (item) {
		elm_genlist_item_selected_set(item, EINA_FALSE);

		Evas_Object *radio = elm_object_item_part_content_get(item, "elm.icon");
		if (radio) {
			elm_radio_state_value_set(radio, type);
			elm_radio_value_set(popup_class->m_radio_main, type);
		}
		elm_genlist_item_selected_set(item, EINA_FALSE);
	}

	popup_class->m_selected_type = type;
	m_preference->set_default_storage_type(popup_class->m_selected_type);
	m_browser->get_settings()->get_contents_settings_edit()->realize_genlist();

	if (popup_class->m_popup) {
		evas_object_del(popup_class->m_popup);
		popup_class->m_popup = NULL;
	}
	evas_object_freeze_events_set(m_browser->get_settings()->get_contents_settings_edit()->get_genlist(), EINA_FALSE);
}

void default_storage_ask_popup::__cancel_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	default_storage_ask_popup *popup_class = (default_storage_ask_popup*)data;
	if (popup_class->m_popup)
		evas_object_del(popup_class->m_popup);
	popup_class->m_popup = NULL;
	evas_object_freeze_events_set(m_browser->get_settings()->get_contents_settings_edit()->get_genlist(), EINA_FALSE);
}

void default_storage_ask_popup::__ok_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	default_storage_ask_popup *popup_class = (default_storage_ask_popup *)data;
	m_preference->set_default_storage_type(popup_class->m_selected_type);

	m_browser->get_settings()->get_contents_settings_edit()->realize_genlist();

	if (popup_class->m_popup)
		evas_object_del(popup_class->m_popup);
	popup_class->m_popup = NULL;
	evas_object_freeze_events_set(m_browser->get_settings()->get_contents_settings_edit()->get_genlist(), EINA_FALSE);
}

developer_mode_edit::developer_mode_edit(void)
:
	m_custom_user_agent_view(NULL),
	m_genlist(NULL),
	m_navi_item(NULL)
{
	BROWSER_LOGD("");

}

developer_mode_edit::~developer_mode_edit(void)
{
	BROWSER_LOGD("");

	_destroy_genlist_styles();
	_destroy_genlist_callback_datas();
	_delete_custom_user_agent_view();
}

Eina_Bool developer_mode_edit::show(void)
{
	BROWSER_LOGD("");

	if (_create_main_layout(m_naviframe) == NULL) {
		BROWSER_LOGE("_create_main_layout failed");
		return EINA_FALSE;
	}

	m_navi_item = elm_naviframe_item_push(m_naviframe, "IDS_BR_BODY_DEVELOPER_MODE", NULL, NULL, m_genlist, NULL);//BR_STRING_SETTINGS_DEVELOPER_MODE
	elm_object_item_domain_text_translatable_set(m_navi_item, BROWSER_DOMAIN_NAME, EINA_TRUE);

#if 0
	Evas_Object *title_icon = elm_image_add(m_naviframe);
	elm_image_file_set(title_icon, settings_edj_path, settings_title_icon);
	evas_object_size_hint_aspect_set(title_icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(title_icon, EINA_TRUE, EINA_TRUE);
	evas_object_smart_callback_add(title_icon, "clicked", __title_icon_clicked_cb, this);
	elm_object_item_part_content_set(m_navi_item, "icon", title_icon);

	Evas_Object *back_button = _create_title_icon_btn(m_naviframe, __back_button_cb, settings_edj_path, settings_title_back_button, this);
	if (!back_button) {
		BROWSER_LOGE("Failed to create back_button");
		return EINA_FALSE;
	}
	elm_object_item_part_content_set(m_navi_item, "title_right_btn", back_button);
	elm_access_info_set(back_button, ELM_ACCESS_INFO, BR_STRING_BACK);
#endif
//	elm_naviframe_item_pop_cb_set(m_navi_item, __naviframe_pop_cb, this);
//	evas_object_smart_callback_add(m_naviframe, "transition,finished", __naviframe_pop_finished_cb, this);

	return EINA_TRUE;
}

custom_user_agent_view *developer_mode_edit::get_custom_user_agent_view(void)
{
	BROWSER_LOGD("");

	if (!m_custom_user_agent_view)
		m_custom_user_agent_view = new custom_user_agent_view();

	return m_custom_user_agent_view;
}

Evas_Object *developer_mode_edit::_create_main_layout(Evas_Object *parent)
{
	BROWSER_LOGD("");

	m_genlist = elm_genlist_add(parent);
	if (!m_genlist) {
		BROWSER_LOGE("elm_genlist_add failed");
		return NULL;
	}

	evas_object_smart_callback_add(m_genlist, "realized", __genlist_item_realized_cb, this);
	evas_object_smart_callback_add(m_genlist, "language,changed", __genlist_lang_changed, NULL);

	_destroy_genlist_styles();
	_destroy_genlist_callback_datas();

	for (unsigned short i = br_settings_sub_advanced_menu_developer_mode_start; i < br_settings_sub_advanced_menu_developer_mode_end; i++) {
		Elm_Genlist_Item_Class *item_class = _create_genlist_item_class((settings_advanced_menu_index)i);
		if (item_class) {
			settings_advanced_genlist_callback_data *cb_data = new settings_advanced_genlist_callback_data;
			memset(cb_data, 0x00, sizeof(settings_advanced_genlist_callback_data));

			cb_data->menu_index = (settings_advanced_menu_index)i;
			cb_data->user_data = this;

			if (settings_advanced_menus[i].is_title == EINA_TRUE) {
				cb_data->it = elm_genlist_item_append(m_genlist, item_class, (void *)cb_data, NULL,
													ELM_GENLIST_ITEM_NONE, NULL, NULL);
				elm_genlist_item_select_mode_set(cb_data->it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			} else {
				cb_data->it = elm_genlist_item_append(m_genlist, item_class, (void *)cb_data, NULL,
													ELM_GENLIST_ITEM_NONE, __genlist_item_clicked_cb, (void *)cb_data);
			}
			m_genlist_callback_data_list.push_back(cb_data);
		} else {
			BROWSER_LOGE("item_class is NULL");
			break;
		}
	}

	return m_genlist;
}

Elm_Genlist_Item_Class *developer_mode_edit::_create_genlist_item_class(settings_advanced_menu_index index)
{
	BROWSER_LOGD("settings_advanced_menu_index[%d]", index);

	if ((index < br_settings_sub_advanced_menu_developer_mode_start) || (index > br_settings_sub_advanced_menu_developer_mode_end)) {
		BROWSER_LOGE("index is out of allowed");
		return NULL;
	}

	if (!settings_advanced_menus[index].item_style) {
		BROWSER_LOGE("unable to get privacy_edit : settings_advanced_menus[i].item_style", index);
		return NULL;
	}

	Elm_Genlist_Item_Class *item_class = NULL;
	/* check duplicated item style */
	for (unsigned short i = 0; i < m_genlist_item_class_list.size(); i++) {
		if (strlen(m_genlist_item_class_list[i]->item_style) == strlen(settings_advanced_menus[index].item_style)
			&& !strncmp(m_genlist_item_class_list[i]->item_style, settings_advanced_menus[index].item_style, strlen(m_genlist_item_class_list[i]->item_style))) {
			BROWSER_LOGE("[%s] style is already created", settings_advanced_menus[index].item_style);
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
		item_class->item_style = settings_advanced_menus[index].item_style;
		item_class->func.text_get = __genlist_text_get;
		item_class->func.content_get = __genlist_contents_get;
		item_class->func.state_get = NULL;
		item_class->func.del = NULL;
		m_genlist_item_class_list.push_back(item_class);
	}

	return item_class;
}

Eina_Bool developer_mode_edit::_call_user_agent(void)
{
	BROWSER_LOGD("");

	struct ug_cbs cbs = {0, };
	cbs.layout_cb = __ug_layout_cb;
	cbs.result_cb = NULL; //__ug_result_cb;
	cbs.destroy_cb = __ug_destroy_cb;
	cbs.priv = (void *)this;

	app_control_h data = NULL;
	if (app_control_create(&data) < 0) {
		BROWSER_LOGE("fail to app_control_create.");
		return EINA_FALSE;
	}
	if (!ug_create(NULL, "browser-user-agent-efl", UG_MODE_FULLVIEW, data, &cbs))
		BROWSER_LOGE("ug_create is failed.");

	if (app_control_destroy(data))
		BROWSER_LOGE("app_control_destroy is failed.");

	return EINA_TRUE;
}

void developer_mode_edit::_delete_custom_user_agent_view(void)
{
	if (m_custom_user_agent_view)
		delete m_custom_user_agent_view;
	m_custom_user_agent_view = NULL;
}

void developer_mode_edit::_destroy_genlist_styles(void)
{
	for (unsigned int i = 0; i < m_genlist_item_class_list.size(); i++) {
		if (m_genlist_item_class_list[i])
			elm_genlist_item_class_free(m_genlist_item_class_list[i]);
		m_genlist_item_class_list[i] = NULL;
	}

	m_genlist_item_class_list.clear();

}

void developer_mode_edit::_destroy_genlist_callback_datas(void)
{
	BROWSER_LOGD("");

	for (unsigned int i = 0; i < m_genlist_callback_data_list.size(); i++) {
		if (m_genlist_callback_data_list[i])
			free(m_genlist_callback_data_list[i]);
		m_genlist_callback_data_list[i] = NULL;
	}

	m_genlist_callback_data_list.clear();
}

char *developer_mode_edit::__genlist_text_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	settings_advanced_genlist_callback_data *cb_data = (settings_advanced_genlist_callback_data *)data;
//	developer_mode_edit *edit = (developer_mode_edit *)cb_data->user_data;
	settings_advanced_menu_index menu_index = cb_data->menu_index;

	char *menu_text = NULL;
	BROWSER_LOGD("menu_index[%d]", menu_index);
	switch (menu_index) {
		case br_settings_sub_advanced_menu_developer_mode_user_agent:
			if (!strcmp(part, "elm.text"))
				menu_text = strdup(BR_STRING_SETTINGS_USER_AGENT);
			else if (!strcmp(part, "elm.text.main.left"))
				menu_text = strdup(BR_STRING_SETTINGS_USER_AGENT);
		break;

		case br_settings_sub_advanced_menu_developer_mode_custom_user_agent:
			if (!strcmp(part, "elm.text"))
				menu_text = strdup(BR_STRING_SETTINGS_CUSTOM_USER_AGENT);
			else if (!strcmp(part, "elm.text.main.left"))
				menu_text = strdup(BR_STRING_SETTINGS_CUSTOM_USER_AGENT);
		break;

		default:
			BROWSER_LOGD("menu_index[%d]is out of allowed", menu_index);
		break;
	}

	return menu_text;
}

Evas_Object *developer_mode_edit::__genlist_contents_get(void *data, Evas_Object *obj, const char *part)
{
	BROWSER_LOGD("part[%s]", part);
	RETV_MSG_IF(!data, NULL, "data is NULL");

	Evas_Object *content = NULL;
	settings_advanced_genlist_callback_data *callback_data = (settings_advanced_genlist_callback_data *)data;
	settings_advanced_menu_index menu_index = callback_data->menu_index;
	developer_mode_edit *edit = (developer_mode_edit *)(callback_data->user_data);

	switch (menu_index) {
		case br_settings_sub_advanced_menu_developer_mode_user_agent:
			if (!strcmp(part, "elm.icon")) {

			} else if (!strcmp(part, "elm.icon.1")) {

			} else if (!strcmp(part, "elm.icon.2")) {

			}
		break;

		case br_settings_sub_advanced_menu_developer_mode_custom_user_agent:
			if (!strcmp(part, "elm.icon")) {

			} else if (!strcmp(part, "elm.icon.1")) {

			} else if (!strcmp(part, "elm.icon.2")) {

			}
		break;

		default:
			BROWSER_LOGD("menu_index[%d]is out of allowed", menu_index);
		break;
	}


	return content;
}

void developer_mode_edit::__genlist_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	settings_advanced_genlist_callback_data *callback_data = (settings_advanced_genlist_callback_data *)data;
	settings_advanced_menu_index menu_index = callback_data->menu_index;
	developer_mode_edit *edit = (developer_mode_edit *)(callback_data->user_data);

	elm_genlist_item_selected_set(callback_data->it, EINA_FALSE);

	switch (menu_index) {
		case br_settings_sub_advanced_menu_developer_mode_user_agent:
			edit->_call_user_agent();
		break;

		case br_settings_sub_advanced_menu_developer_mode_custom_user_agent:
			edit->get_custom_user_agent_view()->popup_edit_view_show();
		break;

		default:
		break;
	}
}

void developer_mode_edit::__genlist_item_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!event_info, "event_info is NULL");

	Elm_Object_Item *selected_item = (Elm_Object_Item *)(event_info);

	if (elm_genlist_item_select_mode_get(selected_item) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
		Elm_Object_Item *next_it = NULL;
		Elm_Object_Item *prev_it = NULL;
		next_it = elm_genlist_item_next_get(selected_item);
		prev_it = elm_genlist_item_prev_get(selected_item);

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

void developer_mode_edit::__title_icon_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
}

void developer_mode_edit::__back_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	elm_naviframe_item_pop(m_naviframe);
}

void developer_mode_edit::__ug_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!priv, "priv is NULL");
	RET_MSG_IF(!ug, "ug is NULL");

	Evas_Object *base = (Evas_Object*)ug_get_layout(ug);
	if (!base)
		return;

	switch (mode) {
		case UG_MODE_FULLVIEW:
			evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_show(base);
			break;

		default:
			break;
	}
}

void developer_mode_edit::__ug_destroy_cb(ui_gadget_h ug, void *priv)
{
	RET_MSG_IF(!priv, "priv is NULL");
	RET_MSG_IF(!ug, "ug is NULL");

	if (ug_destroy(ug))
		BROWSER_LOGE("ug_destroy is failed.\n");

	developer_mode_edit *edit = (developer_mode_edit *)priv;
	Evas_Object *navi_it_access_obj = elm_object_item_access_object_get(elm_naviframe_top_item_get(edit->m_naviframe));
	if (navi_it_access_obj)
		elm_access_highlight_set(navi_it_access_obj);
}
