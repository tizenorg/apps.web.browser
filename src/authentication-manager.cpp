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

#include "authentication-manager.h"

#include <Elementary.h>
#include <string>

#include "browser.h"
#include "browser-view.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "platform-service.h"

authentication_manager::authentication_manager(webview *wv)
:
	m_webview(wv)
	,m_auth_challenge(NULL)
	,m_id_field(NULL)
	,m_password_field(NULL)
{
	BROWSER_LOGD("");
	m_webview->attach_event("authentication,challenge", __authentication_challenge_cb, this);
}

authentication_manager::~authentication_manager(void)
{
	BROWSER_LOGD("");

	m_webview->detach_event("authentication,challenge", __authentication_challenge_cb);
}

void authentication_manager::__authentication_challenge_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	authentication_manager *am = (authentication_manager *)data;
	am->m_auth_challenge = (Ewk_Auth_Challenge *)event_info;
	const char *auth_realm = ewk_auth_challenge_realm_get(am->m_auth_challenge);
	ewk_auth_challenge_suspend(am->m_auth_challenge);

	/* FIXME : The text should be translated. */
	std::string msg = std::string("A username and password are being requested by ") + std::string(am->m_webview->get_uri())
			+ std::string(". The site says: ")  + std::string(" \"") + std::string(auth_realm) + std::string("\"");

	am->_show_id_password_popup(msg.c_str());
}

void authentication_manager::_show_id_password_popup(const char *msg)
{
	BROWSER_LOGD("");

	Evas_Object *popup = elm_popup_add(m_browser->get_browser_view()->get_naviframe());
	if (!popup) {
		BROWSER_LOGE("elm_popup_add is failed");
		return;
	}
	evas_object_size_hint_weight_set(popup , EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(popup , EVAS_HINT_FILL, EVAS_HINT_FILL);
	/* FIXME : The text should be translated. */
	elm_object_part_text_set(popup, "title,text", "Authentication Requested");

	Evas_Object *label = elm_label_add(popup);
	if (!label) {
		BROWSER_LOGE("label_add is failed");
		return;
	}

	evas_object_size_hint_weight_set(label , EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_label_line_wrap_set(label , ELM_WRAP_WORD);
	elm_object_text_set(label, msg);

	platform_service ps;
	m_id_field = ps.editfield_add(popup);
	Evas_Object *entry = ps.editfield_entry_get(m_id_field);
	elm_entry_autocapital_type_set(entry, ELM_AUTOCAPITAL_TYPE_NONE);
	ps.editfield_eraser_set(m_id_field, EINA_TRUE);
	elm_entry_text_style_user_push(m_id_field, uri_entry_style);

	m_password_field = ps.editfield_add(popup);
	entry = ps.editfield_entry_get(m_password_field);
	elm_entry_password_set(entry, EINA_TRUE);
	elm_entry_text_style_user_push(m_password_field, uri_entry_style);

	Evas_Object *popup_layout = elm_layout_add(popup);

	elm_layout_file_set(popup_layout, browser_edj_dir"/browser-popup.edj", "auth_challenge_popup");
	evas_object_size_hint_weight_set(popup_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	/* FIXME : The text should be translated. */
	edje_object_part_text_set(elm_layout_edje_get(popup_layout), "idfield_text", " User Name: ");
	edje_object_part_text_set(elm_layout_edje_get(popup_layout), "passwdfield_text", " Password: ");

	elm_object_part_content_set(popup_layout, "elm.swallow.label", label);
	elm_object_part_content_set(popup_layout, "elm.swallow.idfield", m_id_field);
	elm_object_part_content_set(popup_layout, "elm.swallow.passwdfield", m_password_field);
	elm_object_content_set(popup, popup_layout);

	Evas_Object *ok_button = elm_button_add(popup);
	elm_object_text_set(ok_button, BR_STRING_OK);
	elm_object_style_set(ok_button, "popup_button/default");
	elm_object_part_content_set(popup, "button1", ok_button);
	evas_object_smart_callback_add(ok_button, "clicked", __ok_button_cb, this);

	Evas_Object *cancel_button = elm_button_add(popup);
	elm_object_text_set(cancel_button, BR_STRING_CANCEL);
	elm_object_style_set(cancel_button, "popup_button/default");
	elm_object_part_content_set(popup, "button2", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __cancel_button_cb, this);

	evas_object_show(popup);
}

void authentication_manager::__ok_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	authentication_manager *am = (authentication_manager *)data;
	platform_service ps;

	Evas_Object *entry = ps.editfield_entry_get(am->m_id_field);
	const char *input_id = elm_entry_entry_get(entry);
	entry = ps.editfield_entry_get(am->m_password_field);
	const char *input_pw = elm_entry_entry_get(entry);
	ewk_auth_challenge_credential_use(am->m_auth_challenge, (char *)input_id,  (char *)input_pw);

	m_browser->get_browser_view()->destroy_popup(obj);
}

void authentication_manager::__cancel_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	authentication_manager *am = (authentication_manager *)data;
	ewk_auth_challenge_credential_cancel(am->m_auth_challenge);

	m_browser->get_browser_view()->destroy_popup(obj);
}

