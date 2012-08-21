/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.tizenopensource.org/license
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include "browser-settings-class.h"
#include "browser-settings-edit-homepage-view.h"

Browser_Settings_Edit_Homepage_View::Browser_Settings_Edit_Homepage_View(Browser_Settings_Main_View *main_view)
:
	m_main_view(main_view)
	,m_content_layout(NULL)
	,m_edit_field(NULL)
	,m_done_button(NULL)
	,m_cancel_button(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Settings_Edit_Homepage_View::~Browser_Settings_Edit_Homepage_View(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

void Browser_Settings_Edit_Homepage_View::__back_button_clicked_cb(void *data,
						Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	if (elm_naviframe_bottom_item_get(m_navi_bar)
	    != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);
}

void Browser_Settings_Edit_Homepage_View::__done_button_clicked_cb(void *data,
						Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Settings_Edit_Homepage_View *edit_homepage_view = NULL;
	edit_homepage_view = (Browser_Settings_Edit_Homepage_View *)data;

	Evas_Object *entry = br_elm_editfield_entry_get(edit_homepage_view->m_edit_field);
	char *homepage = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));
	if (homepage) {
		br_preference_set_str(USER_HOMEPAGE_KEY, homepage);
		br_preference_set_str(HOMEPAGE_KEY, USER_HOMEPAGE);
		free(homepage);
	}

	if (elm_naviframe_bottom_item_get(m_navi_bar)
	    != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);
}

void Browser_Settings_Edit_Homepage_View::__cancel_button_clicked_cb(void *data,
						Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (elm_naviframe_bottom_item_get(m_navi_bar)
	    != elm_naviframe_top_item_get(m_navi_bar))
		elm_naviframe_item_pop(m_navi_bar);
}

Eina_Bool Browser_Settings_Edit_Homepage_View::init(void)
{
	BROWSER_LOGD("[%s]", __func__);
	return _create_main_layout();
}

void Browser_Settings_Edit_Homepage_View::__edit_field_changed_cb(void *data,
						Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;
	Browser_Settings_Edit_Homepage_View *edit_homepage_view = NULL;
	edit_homepage_view = (Browser_Settings_Edit_Homepage_View *)data;

	const char *url = elm_entry_entry_get(obj);
	if (!url || !strlen(url)) {
		elm_object_disabled_set(edit_homepage_view->m_done_button, EINA_TRUE);
		return;
	}

	Eina_Bool only_has_space = EINA_FALSE;
	int space_count = 0;
	for (int i = 0 ; i < strlen(url) ; i++) {
		if (url[i] == ' ')
			space_count++;
	}
	if (space_count == strlen(url))
		only_has_space = EINA_TRUE;

	if (only_has_space) {
		elm_object_disabled_set(edit_homepage_view->m_done_button, EINA_TRUE);
		return;
	}

	elm_object_disabled_set(edit_homepage_view->m_done_button, EINA_FALSE);
}

Eina_Bool Browser_Settings_Edit_Homepage_View::_create_main_layout(void)
{
	BROWSER_LOGD("[%s]", __func__);

	m_content_layout = elm_layout_add(m_navi_bar);
	if (!m_content_layout) {
		BROWSER_LOGE("elm_layout_add failed");
		return EINA_FALSE;
	}
	if (!elm_layout_file_set(m_content_layout, BROWSER_EDJE_DIR"/browser-settings.edj",
					"edit_homepage_view")) {
		BROWSER_LOGE("elm_layout_file_set failed");
		return EINA_FALSE;
	}
	evas_object_show(m_content_layout);

	m_edit_field = br_elm_editfield_add(m_content_layout, EINA_TRUE);
	if (!m_edit_field) {
		BROWSER_LOGE("elm_editfield_add failed");
		return EINA_FALSE;
	}
	br_elm_editfield_entry_single_line_set(m_edit_field, EINA_TRUE);
	br_elm_editfield_label_set(m_edit_field, BR_STRING_URL);
	elm_object_part_content_set(m_content_layout, "elm.swallow.entry", m_edit_field);

	char *homepage = NULL;
	if (br_preference_get_str(USER_HOMEPAGE_KEY, &homepage) == false) {
		BROWSER_LOGE("failed to get %s preference\n", USER_HOMEPAGE_KEY);
		return EINA_FALSE;
	}
	Evas_Object *entry = br_elm_editfield_entry_get(m_edit_field);
	if (homepage) {
		elm_entry_entry_set(entry, homepage);
		free(homepage);
	}
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_URL);
	evas_object_size_hint_weight_set(m_edit_field, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_edit_field, 1, 0.5);
	elm_entry_cursor_end_set(entry);

	evas_object_show(m_edit_field);
	elm_object_focus_set(m_edit_field, EINA_TRUE);
	evas_object_smart_callback_add(entry, "changed", __edit_field_changed_cb, this);

	Elm_Object_Item *navi_it = elm_naviframe_item_push(m_navi_bar, BR_STRING_HOMEPAGE,
							NULL, NULL, m_content_layout, "browser_titlebar");
	elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_PREV_BTN, NULL);

	m_done_button = elm_button_add(m_content_layout);
	if (!m_done_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_done_button, "browser/title_button");
	elm_object_text_set(m_done_button, BR_STRING_DONE);
	evas_object_show(m_done_button);
	evas_object_smart_callback_add(m_done_button, "clicked", __done_button_clicked_cb, this);
	elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_TITLE_RIGHT_BTN, m_done_button);

	m_cancel_button = elm_button_add(m_content_layout);
	if (!m_cancel_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_cancel_button, "browser/title_button");
	elm_object_text_set(m_cancel_button, BR_STRING_CANCEL);
	evas_object_show(m_cancel_button);
	evas_object_smart_callback_add(m_cancel_button, "clicked", __cancel_button_clicked_cb, this);
	elm_object_item_part_content_set(navi_it, ELM_NAVIFRAME_ITEM_TITLE_LEFT_BTN, m_cancel_button);

	return EINA_TRUE;
}

