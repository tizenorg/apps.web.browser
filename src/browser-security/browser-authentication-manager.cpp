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
  */

#include "browser-authentication-manager.h"

Evas_Object *Browser_Authetication_Manager::m_popup;
Evas_Object *Browser_Authetication_Manager::m_user_name_edit_field;
Evas_Object *Browser_Authetication_Manager::m_password_edit_field;

Browser_Authetication_Manager::Browser_Authetication_Manager(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Authetication_Manager::~Browser_Authetication_Manager(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_popup)
		evas_object_del(m_popup);
	if (m_user_name_edit_field)
		evas_object_del(m_user_name_edit_field);
	if (m_password_edit_field)
		evas_object_del(m_password_edit_field);
}

Eina_Bool Browser_Authetication_Manager::init(void)
{
	BROWSER_LOGD("[%s]", __func__);

	/* Why this callback doesn't support user data? !!! */
	ewk_auth_show_dialog_callback_set(__show_auth_dialog_cb);

	return EINA_TRUE;
}

void Browser_Authetication_Manager::__show_auth_dialog_cb(const char* msg, const char* uri, void* data)
{
	BROWSER_LOGD("[%s]", __func__);

	m_popup = elm_popup_add(m_navi_bar);
	if (!m_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return;
	}

	evas_object_size_hint_weight_set(m_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_popup_orient_set(m_popup, ELM_POPUP_ORIENT_TOP);
	elm_object_part_text_set(m_popup, msg, "elm.text");

	Evas_Object *ok_button = elm_button_add(m_popup);
	elm_object_text_set(ok_button, "OK");
	elm_object_part_content_set(m_popup, "button1", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __popup_reponse_cb, data);

	Evas_Object *cancel_button = elm_button_add(m_popup);
	elm_object_text_set(cancel_button, "Cancel");
	elm_object_part_content_set(m_popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __popup_cancel_cb, data);

	Evas_Object *content_box = elm_box_add(m_popup);
	if (!content_box) {
		BROWSER_LOGE("elm_box_add failed");
		return;
	}
	evas_object_size_hint_weight_set(content_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(content_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(content_box);

	m_user_name_edit_field = br_elm_editfield_add(m_popup);
	if (!m_user_name_edit_field) {
		BROWSER_LOGE("elm_editfield_add failed");
		return;
	}
	br_elm_editfield_guide_text_set(m_user_name_edit_field, BR_STRING_USER_NAME);
	evas_object_size_hint_weight_set(m_user_name_edit_field, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_user_name_edit_field, EVAS_HINT_FILL, EVAS_HINT_FILL);
	br_elm_editfield_entry_single_line_set(m_user_name_edit_field, EINA_TRUE);
	evas_object_show(m_user_name_edit_field);
	elm_box_pack_end(content_box, m_user_name_edit_field);

	m_password_edit_field = br_elm_editfield_add(m_popup);
	if (!m_password_edit_field) {
		BROWSER_LOGE("elm_editfield_add failed");
		return;
	}
	br_elm_editfield_guide_text_set(m_password_edit_field, BR_STRING_PASSWORD);
	evas_object_size_hint_weight_set(m_password_edit_field, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_password_edit_field, EVAS_HINT_FILL, EVAS_HINT_FILL);
	br_elm_editfield_entry_single_line_set(m_password_edit_field, EINA_TRUE);
	elm_entry_password_set(br_elm_editfield_entry_get(m_password_edit_field), EINA_TRUE);
	evas_object_show(m_password_edit_field);
	elm_box_pack_end(content_box, m_password_edit_field);

	elm_object_content_set(m_popup, content_box);

	evas_object_show(m_popup);

	elm_object_focus_set(m_user_name_edit_field, EINA_TRUE);
}

void Browser_Authetication_Manager::__popup_reponse_cb(void* data, Evas_Object* obj, void* event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	char *user_name = NULL;
	char *password = NULL;

	user_name = strdup(elm_entry_entry_get(br_elm_editfield_entry_get(m_user_name_edit_field)));
	password = strdup(elm_entry_entry_get(br_elm_editfield_entry_get(m_password_edit_field)));

	BROWSER_LOGD("user_name=[%s], passwrd=[%s]", user_name, password);
	ewk_auth_credentials_set(user_name, password, data);

	Ecore_IMF_Context *ic = (Ecore_IMF_Context *)elm_entry_imf_context_get(br_elm_editfield_entry_get(m_user_name_edit_field));
	if (!ic)
		ic = (Ecore_IMF_Context *)elm_entry_imf_context_get(br_elm_editfield_entry_get(m_password_edit_field));
	if (ic)
		ecore_imf_context_input_panel_hide(ic);

	if (m_popup) {
		evas_object_del(m_popup);
		m_popup = NULL;
	}
	if (m_user_name_edit_field) {
		evas_object_del(m_user_name_edit_field);
		m_user_name_edit_field = NULL;
	}
	if (m_password_edit_field) {
		evas_object_del(m_password_edit_field);
		m_password_edit_field = NULL;
	}

	if (user_name)
		free(user_name);
	if (password)
		free(password);
}

void Browser_Authetication_Manager::__popup_cancel_cb(void* data, Evas_Object* obj, void* event_info)
{
	BROWSER_LOGD("[%s]", __func__);

	Ecore_IMF_Context *ic = (Ecore_IMF_Context *)elm_entry_imf_context_get(br_elm_editfield_entry_get(m_user_name_edit_field));
	if (!ic)
		ic = (Ecore_IMF_Context *)elm_entry_imf_context_get(br_elm_editfield_entry_get(m_password_edit_field));
	if (ic)
		ecore_imf_context_input_panel_hide(ic);

	if (m_popup) {
		evas_object_del(m_popup);
		m_popup = NULL;
	}
	if (m_user_name_edit_field) {
		evas_object_del(m_user_name_edit_field);
		m_user_name_edit_field = NULL;
	}
	if (m_password_edit_field) {
		evas_object_del(m_password_edit_field);
		m_password_edit_field = NULL;
	}
}

