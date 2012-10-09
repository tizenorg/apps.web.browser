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


#include "browser-class.h"
#include "browser-common-view.h"
#include "browser-view.h"
#include "browser-window.h"

Browser_Data_Manager *Browser_Common_View::m_data_manager;
Evas_Object *Browser_Common_View::m_win;
Evas_Object *Browser_Common_View::m_navi_bar;
Evas_Object *Browser_Common_View::m_bg;
Browser_Class *Browser_Common_View::m_browser;

Browser_Common_View::Browser_Common_View(void)
:
	m_selection_info(NULL)
	,m_selection_info_layout(NULL)
	,m_selinfo_layout(NULL)
	,m_popup(NULL)
	,m_ug(NULL)
	,m_share_popup(NULL)
	,m_share_list(NULL)
	,m_database_quota_change_confirm_popup(NULL)
	,m_file_system_change_confirm_popup(NULL)
	,m_call_confirm_popup(NULL)
	,m_call_type(CALL_UNKNOWN)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Common_View::~Browser_Common_View(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_selection_info_layout) {
		evas_object_del(m_selection_info_layout);
		m_selection_info_layout = NULL;
	}
	if (m_selection_info) {
		evas_object_del(m_selection_info);
		m_selection_info = NULL;
	}
	if (m_popup) {
		evas_object_del(m_popup);
		m_popup = NULL;
	}
	if (m_share_popup) {
		evas_object_del(m_share_popup);
		m_share_popup = NULL;
	}
	if (m_share_list) {
		evas_object_del(m_share_list);
		m_share_list = NULL;
	}
	if (m_ug) {
		ug_destroy(m_ug);
		m_ug = NULL;
	}
	if (m_database_quota_change_confirm_popup) {
		evas_object_del(m_database_quota_change_confirm_popup);
		m_database_quota_change_confirm_popup = NULL;
	}
	if (m_file_system_change_confirm_popup) {
		evas_object_del(m_file_system_change_confirm_popup);
		m_file_system_change_confirm_popup = NULL;
	}
	if (m_call_confirm_popup) {
		evas_object_del(m_call_confirm_popup);
		m_call_confirm_popup = NULL;
	}
}

void Browser_Common_View::show_msg_popup(const char *msg, int timeout)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_popup) {
		evas_object_del(m_popup);
		m_popup = NULL;
	}

	m_popup = elm_popup_add(m_navi_bar);
	evas_object_size_hint_weight_set(m_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(m_popup, msg);
	elm_popup_timeout_set(m_popup, timeout);
	evas_object_show(m_popup);
}

void Browser_Common_View::show_msg_popup(const char *title, const char *msg, int timeout)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_popup) {
		evas_object_del(m_popup);
		m_popup = NULL;
	}

	m_popup = elm_popup_add(m_navi_bar);
	evas_object_size_hint_weight_set(m_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(m_popup, "title,text", title);
	elm_object_text_set(m_popup, msg);
	elm_popup_timeout_set(m_popup, timeout);
	evas_object_show(m_popup);
}

void Browser_Common_View::hide_notify_popup(void)
{
	if (m_selection_info_layout) {
		evas_object_del(m_selection_info_layout);
		m_selection_info_layout = NULL;
	}

	if (m_selection_info) {
		evas_object_del(m_selection_info);
		m_selection_info = NULL;
	}
}

void Browser_Common_View::show_notify_popup(const char *msg, int timeout, Eina_Bool has_control_bar)
{
	if (m_selection_info_layout) {
		evas_object_del(m_selection_info_layout);
		m_selection_info_layout = NULL;
	}

	if (m_selection_info) {
		evas_object_del(m_selection_info);
		m_selection_info = NULL;
	}

	int angle = 0;
	angle = elm_win_rotation_get(m_win);
	m_selection_info = elm_notify_add(m_navi_bar);
	if (!m_selection_info) {
		BROWSER_LOGD("elm_notify_add failed");
		return;
	}
	elm_notify_orient_set(m_selection_info, ELM_NOTIFY_ORIENT_BOTTOM);
	evas_object_size_hint_weight_set(m_selection_info, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_selection_info, EVAS_HINT_FILL, EVAS_HINT_FILL);
	m_selection_info_layout = elm_layout_add(m_selection_info);
	if (!m_selection_info_layout) {
		BROWSER_LOGD("elm_layout_add failed");
		return;
	}
	evas_object_size_hint_weight_set(m_selection_info_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_selection_info_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_content_set(m_selection_info, m_selection_info_layout);

	if (has_control_bar) {
		if(angle == 0 || angle == 180)
		   elm_layout_theme_set(m_selection_info_layout, "standard", "selectioninfo",
		   						"vertical/bottom_64");
		else
		   elm_layout_theme_set(m_selection_info_layout, "standard", "selectioninfo",
		   						"horizontal/bottom_64");
	} else {
		if(angle == 0 || angle == 180)
		   elm_layout_theme_set(m_selection_info_layout, "standard", "selectioninfo",
		   						"vertical/bottom_12");
		else
		   elm_layout_theme_set(m_selection_info_layout, "standard", "selectioninfo",
		   						"horizontal/bottom_12");
	}
	edje_object_part_text_set(elm_layout_edje_get(m_selection_info_layout), "elm.text", msg);

	if (timeout)
		elm_notify_timeout_set(m_selection_info, timeout);

	evas_object_show(m_selection_info);
}

void Browser_Common_View::show_notify_popup_layout(const char *msg, int timeout, Evas_Object *parent)
{
	if (m_selinfo_layout) {
		evas_object_del(m_selinfo_layout);
		m_selinfo_layout = NULL;
	}

	m_selinfo_layout = elm_layout_add(parent);
	if (!m_selinfo_layout) {
		BROWSER_LOGD("elm_layout_add failed");
		return;
	}
	elm_object_part_content_set(parent,
			"selinfo.swallow.contents",
			m_selinfo_layout);
	evas_object_size_hint_weight_set(m_selinfo_layout,
			EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_selinfo_layout,
			EVAS_HINT_FILL,
			EVAS_HINT_FILL);

	elm_object_content_set(m_selection_info, m_selection_info_layout);

	/* Set the layout theme */
	elm_layout_theme_set(m_selinfo_layout, "standard", "selectioninfo", "default");
	/* Set the text */
	elm_object_part_text_set(m_selinfo_layout, "elm.text", msg);
	elm_object_signal_emit(parent, "show,selection,info", "elm");
}

void Browser_Common_View::hide_notify_popup_layout(Evas_Object *parent)
{
	if (m_selinfo_layout) {
		evas_object_del(m_selinfo_layout);
		m_selinfo_layout = NULL;
	}

	elm_object_signal_emit(parent, "hide,selection,info", "elm");
}

Eina_Bool Browser_Common_View::find_word_with_text(const char *text_to_find)
{
	BROWSER_LOGD("[%s], text_to_find[%s]", __func__, text_to_find);

	if (!text_to_find)
		return EINA_FALSE;

	Browser_View *browser_view = m_data_manager->get_browser_view();
	return browser_view->launch_find_word_with_text(text_to_find);;
}

/* Capture snapshot with current focused ewk view. */
Evas_Object *Browser_Common_View::_capture_snapshot(Browser_Window *window, float scale)
{
	BROWSER_LOGD("[%s]", __func__);

	int focused_ewk_view_w = 0;
	int focused_ewk_view_h = 0;
	evas_object_geometry_get(window->m_ewk_view, NULL, NULL,
						&focused_ewk_view_w, &focused_ewk_view_h);

	Evas_Object *rectangle = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
	evas_object_size_hint_min_set(rectangle, focused_ewk_view_w * scale, focused_ewk_view_h * scale);
	evas_object_resize(rectangle, focused_ewk_view_w * scale, focused_ewk_view_h * scale);
	evas_object_color_set(rectangle, 255, 255, 255, 255);
	return rectangle;
}


void Browser_Common_View::__send_via_message_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data)
		return;

	Browser_Common_View *common_view = (Browser_Common_View *)data;
	if (!common_view->_send_via_message(common_view->m_share_url, std::string()))
		BROWSER_LOGE("_send_via_message failed");

	__popup_response_cb(common_view, NULL, NULL);
}

Eina_Bool Browser_Common_View::_send_via_message(std::string url, std::string to, Eina_Bool attach_file)
{
	BROWSER_LOGD("[%s], url[%s], to[%s]", __func__, url.c_str(), to.c_str());
	if (url.empty() && to.empty()) {
		show_msg_popup(BR_STRING_EMPTY);
		return EINA_FALSE;
	}

	service_h service_handle = NULL;
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (!service_handle) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (!url.empty()) {
		if (attach_file) {
			if (service_set_operation(service_handle, SERVICE_OPERATION_SEND) < 0) {
				BROWSER_LOGE("Fail to set service operation");
				service_destroy(service_handle);
				return EINA_FALSE;
			}

			if (service_add_extra_data(service_handle, "ATTACHFILE", url.c_str())) {
				BROWSER_LOGE("Fail to set extra data");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		} else {
			if (service_set_operation(service_handle, SERVICE_OPERATION_SEND_TEXT) < 0) {
				BROWSER_LOGE("Fail to set service operation");
				service_destroy(service_handle);
				return EINA_FALSE;
			}

			if (service_add_extra_data(service_handle, SERVICE_DATA_TEXT, url.c_str()) < 0) {
				BROWSER_LOGE("Fail to set extra data");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		}
	}

	if (!to.empty()) {
		if (url.empty()) {
			if (service_set_operation(service_handle, SERVICE_OPERATION_SEND_TEXT) < 0) {
				BROWSER_LOGE("Fail to set service operation");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		}

		if (service_add_extra_data(service_handle, SERVICE_DATA_TO , to.c_str()) < 0) {
			BROWSER_LOGE("Fail to set extra data");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
	}

	if (service_set_package(service_handle, SEC_MESSAGE) < 0) {//SEC_EMAIL
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}
	service_destroy(service_handle);

	return EINA_TRUE;
}

void Browser_Common_View::__send_via_email_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("%s", __func__);
	if (!data)
		return;

	Browser_Common_View *common_view = (Browser_Common_View *)data;
	if (!common_view->_send_via_email(common_view->m_share_url))
		BROWSER_LOGE("_send_via_email failed");

	__popup_response_cb(common_view, NULL, NULL);
}

void Browser_Common_View::__share_via_nfc_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("%s", __func__);
	if (!data)
		return;

	Browser_Common_View *common_view = (Browser_Common_View *)data;
	if (!common_view->_share_via_nfc(common_view->m_share_url))
		BROWSER_LOGE("_share_via_nfc failed");

	__popup_response_cb(common_view, NULL, NULL);
}

Eina_Bool Browser_Common_View::_show_database_quota_size_change_popup(Ewk_Context_Exceeded_Quota *database_quota)
{
	BROWSER_LOGD("[%s]", __func__);

	Ewk_Security_Origin* origin = ewk_context_web_database_exceeded_quota_security_origin_get(database_quota);
	Browser_View *browser_view = m_data_manager->get_browser_view();

	const char *host_name = ewk_security_origin_host_get(origin);
	unsigned long expected_quota_size = ewk_context_web_database_exceeded_quota_expected_usage_get(database_quota);

	m_quota_data.common_view = this;
	m_quota_data.database_quota = database_quota;

	/* Make confirm msg */
	std::string::size_type pos = std::string::npos;
	std::string confirm_msg = std::string(BR_STRING_MSG_ASK_INCREASING_QUOTA_Q);
	char quota_size_str[21] = {0, };
	snprintf(quota_size_str, 20, "%d", (int)(expected_quota_size / (1024 * 1024)));

	while ((pos = confirm_msg.find("%s")) != std::string::npos) {
		if (host_name && strlen(host_name))
			confirm_msg.replace(pos, strlen("%s"), host_name);
		else
			confirm_msg.replace(pos, strlen("%s"), browser_view->get_url().c_str());
	}

	while ((pos = confirm_msg.find("%d")) != std::string::npos)
		confirm_msg.replace(pos, strlen("%d"), quota_size_str);

	if (m_database_quota_change_confirm_popup) {
		evas_object_del(m_database_quota_change_confirm_popup);
		m_database_quota_change_confirm_popup = NULL;
	}
	m_database_quota_change_confirm_popup = elm_popup_add(m_navi_bar);
	if (!m_database_quota_change_confirm_popup) {
		BROWSER_LOGE("Failed to add popup");
		return EINA_FALSE;
	}

	evas_object_size_hint_weight_set(m_database_quota_change_confirm_popup,
									EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(m_database_quota_change_confirm_popup, confirm_msg.c_str());
	evas_object_show(m_database_quota_change_confirm_popup);

	Evas_Object *ok_button = elm_button_add(m_database_quota_change_confirm_popup);
	if (!ok_button) {
		BROWSER_LOGE("Failed to add ok button");
		return EINA_FALSE;
	}
	elm_object_text_set(ok_button, BR_STRING_OK);
	elm_object_part_content_set(m_database_quota_change_confirm_popup, "button1", ok_button);
	elm_object_style_set(ok_button, "popup_button/default");
	evas_object_smart_callback_add(ok_button, "clicked", __database_quota_size_change_popup_ok_cb, &m_quota_data);

	Evas_Object *cancel_button = elm_button_add(m_database_quota_change_confirm_popup);
	if (!cancel_button) {
		BROWSER_LOGE("Failed to add cancel button");
		return EINA_FALSE;
	}
	elm_object_text_set(cancel_button, BR_STRING_CANCEL);
	elm_object_part_content_set(m_database_quota_change_confirm_popup, "button2", cancel_button);
	elm_object_style_set(cancel_button, "popup_button/default");
	evas_object_smart_callback_add(cancel_button, "clicked", __database_quota_size_change_popup_cancel_cb, &m_quota_data);

	return EINA_TRUE;
}

Eina_Bool Browser_Common_View::_show_file_system_permission_change_popup(Ewk_Context_File_System_Permission *file_system_permission)
{
	BROWSER_LOGD("[%s]", __func__);

	Ewk_Security_Origin* origin = ewk_context_file_system_permission_origin_get(file_system_permission);
	Browser_View *browser_view = m_data_manager->get_browser_view();

	m_file_system_permission.common_view = this;
	m_file_system_permission.file_system_permission = file_system_permission;

	/* Make confirm msg */
	std::string confirm_msg = std::string(BR_STRING_MSG_ASK_FILE_PERMISSION_CHANGE_Q);

	if (m_file_system_change_confirm_popup) {
		evas_object_del(m_file_system_change_confirm_popup);
		m_file_system_change_confirm_popup = NULL;
	}
	m_file_system_change_confirm_popup = elm_popup_add(m_navi_bar);
	if (!m_file_system_change_confirm_popup) {
		BROWSER_LOGE("Failed to add popup");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_file_system_change_confirm_popup,
									EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(m_file_system_change_confirm_popup, confirm_msg.c_str());
	evas_object_show(m_file_system_change_confirm_popup);

	Evas_Object *ok_button = elm_button_add(m_file_system_change_confirm_popup);
	if (!ok_button) {
		BROWSER_LOGE("Failed to add ok button");
		return EINA_FALSE;
	}
	elm_object_text_set(ok_button, BR_STRING_OK);
	elm_object_part_content_set(m_file_system_change_confirm_popup, "button1", ok_button);
	elm_object_style_set(ok_button, "popup_button/default");
	evas_object_smart_callback_add(ok_button, "clicked", __file_system_permission_change_popup_ok_cb, &m_file_system_permission);

	Evas_Object *cancel_button = elm_button_add(m_file_system_change_confirm_popup);
	if (!cancel_button) {
		BROWSER_LOGE("Failed to add cancel button");
		return EINA_FALSE;
	}
	elm_object_text_set(cancel_button, BR_STRING_CANCEL);
	elm_object_part_content_set(m_file_system_change_confirm_popup, "button2", cancel_button);
	elm_object_style_set(cancel_button, "popup_button/default");
	evas_object_smart_callback_add(cancel_button, "clicked", __file_system_permission_change_popup_cancel_cb, &m_file_system_permission);

	return EINA_TRUE;
}

Eina_Bool Browser_Common_View::_launch_streaming_player(const char *url, const char *cookie)
{
	BROWSER_LOGD("%s", __func__);
	if (!url || strlen(url) == 0) {
		BROWSER_LOGE("url is empty");
		return EINA_FALSE;
	}

	bool is_running = false;
	if (app_manager_is_running(SEC_VT_CALL, &is_running) < 0) {
		BROWSER_LOGE("Fail to get app running information\n");
		return EINA_FALSE;
	}
	if (is_running) {
		BROWSER_LOGE("video-call is running......\n");
		show_msg_popup(BR_STRING_WARNING_VIDEO_PLAYER);
		return EINA_FALSE;
	}

	service_h service_handle = NULL;

	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (!service_handle) {
		BROWSER_LOGE("service handle is NULL");
		return EINA_FALSE;
	}

	BROWSER_LOGD("url=[%s]", url);
	if (service_add_extra_data(service_handle, "path", url) < 0) {
		BROWSER_LOGE("Fail to set extra data");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (cookie && strlen(cookie)) {
		if (service_add_extra_data(service_handle, "cookie", cookie) < 0) {
			BROWSER_LOGE("Fail to set extra data");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
	}

	if (service_set_package(service_handle,SEC_STREAMING_PLAYER) < 0) {
		BROWSER_LOGE("Fail to set package");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	service_destroy(service_handle);

	return EINA_TRUE;
}

void Browser_Common_View::__database_quota_size_change_popup_ok_cb(void* data, Evas_Object* obj, void* event_info)
{
	BROWSER_LOGD("[%s]\n", __func__);

	if (!data)
		return;

	quota_size_change_callback_data *quota_data = (quota_size_change_callback_data *)data;
	Browser_Common_View *common_view = quota_data->common_view;
	unsigned long expected_database_quota_size = ewk_context_web_database_exceeded_quota_expected_usage_get(quota_data->database_quota);

	/* Added default_quota_database_size is reserved for memory space */
	ewk_context_web_database_exceeded_quota_new_quota_set(quota_data->database_quota, expected_database_quota_size);

	evas_object_del(common_view->m_database_quota_change_confirm_popup);
	common_view->m_database_quota_change_confirm_popup = NULL;
}

void Browser_Common_View::__database_quota_size_change_popup_cancel_cb(void* data, Evas_Object* obj, void* event_info)
{
	BROWSER_LOGD("[%s]\n", __func__);

	if (!data)
		return;

	quota_size_change_callback_data *quota_data = (quota_size_change_callback_data *)data;
	Browser_Common_View *common_view = quota_data->common_view;

	evas_object_del(common_view->m_database_quota_change_confirm_popup);
	common_view->m_database_quota_change_confirm_popup = NULL;
}

void Browser_Common_View::__file_system_permission_change_popup_ok_cb(void* data, Evas_Object* obj, void* event_info)
{
	BROWSER_LOGD("[%s]\n", __func__);

	if (!data)
		return;

	file_system_permission_change_callback_data *permission_change_data = (file_system_permission_change_callback_data *)data;
	Browser_Common_View *common_view = permission_change_data->common_view;
	Ewk_Context_File_System_Permission *file_system_permission = permission_change_data->file_system_permission;

	ewk_context_file_system_permission_allow_set(file_system_permission, EINA_TRUE);

	evas_object_del(common_view->m_file_system_change_confirm_popup);
	common_view->m_file_system_change_confirm_popup = NULL;
}

void Browser_Common_View::__file_system_permission_change_popup_cancel_cb(void* data, Evas_Object* obj, void* event_info)
{
	BROWSER_LOGD("[%s]\n", __func__);

	if (!data)
		return;

	file_system_permission_change_callback_data *permission_change_data = (file_system_permission_change_callback_data *)data;
	Browser_Common_View *common_view = permission_change_data->common_view;
	Ewk_Context_File_System_Permission *file_system_permission = permission_change_data->file_system_permission;

	ewk_context_file_system_permission_allow_set(file_system_permission, EINA_FALSE);

	evas_object_del(common_view->m_file_system_change_confirm_popup);
	common_view->m_file_system_change_confirm_popup = NULL;
}

Eina_Bool Browser_Common_View::_send_via_email(std::string url, Eina_Bool attach_file)
{
	BROWSER_LOGD("[%s], url[%s]", __func__, url.c_str());
	if (url.empty()) {
		show_msg_popup(BR_STRING_EMPTY);
		return EINA_FALSE;
	}

	service_h service_handle = NULL;
	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (!service_handle) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (attach_file) {
		if (service_set_operation(service_handle, SERVICE_OPERATION_SEND) < 0) {
			BROWSER_LOGE("Fail to set service operation");
			service_destroy(service_handle);
		return EINA_FALSE;
	}

		if (service_set_uri(service_handle, url.c_str()) < 0) {
			BROWSER_LOGE("Fail to set uri");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
	} else {
		if (service_set_operation(service_handle, SERVICE_OPERATION_SEND_TEXT) < 0) {
			BROWSER_LOGE("Fail to set service operation");
			service_destroy(service_handle);
			return EINA_FALSE;
		}

		if (strstr(url.c_str(), BROWSER_MAIL_TO_SCHEME)) {
			if (service_add_extra_data(service_handle, SERVICE_DATA_TO, url.c_str() + strlen(BROWSER_MAIL_TO_SCHEME)) < 0) {
				BROWSER_LOGE("Fail to set mailto data");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		} else {
			if (service_add_extra_data(service_handle, SERVICE_DATA_TEXT, url.c_str()) < 0) {
				BROWSER_LOGE("Fail to set extra data");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		}
	}

	if (service_set_app_id(service_handle, SEC_EMAIL_UG) < 0) {
		BROWSER_LOGE("Fail to service_set_app_id");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	Ecore_X_Window win_id = elm_win_xwindow_get(m_win);
	if (service_set_window(service_handle, win_id) < 0) {
		BROWSER_LOGE("Fail to service_set_window");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}
	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool Browser_Common_View::_add_to_contact(std::string number)
{
	if (number.empty()) {
		BROWSER_LOGE("number is null");
		return EINA_FALSE;
	}
	struct ug_cbs cbs = {0, };
	cbs.layout_cb = __ug_layout_cb;
	cbs.result_cb = NULL;//__ug_result_cb;
	cbs.destroy_cb = __ug_destroy_cb;
	cbs.priv = (void *)this;

	char *phone_number = (char *)strdup(number.c_str());
	if (!phone_number) {
		BROWSER_LOGE("strdup failed");
		return EINA_FALSE;
	}

	service_h data = NULL;
	service_create(&data);
	if (data == NULL) {
		BROWSER_LOGE("fail to service_create.");
		return EINA_FALSE;
	}
/*
type.
CT_UG_REQUEST_ADD = 21,
CT_UG_REQUEST_ADD_WITH_NUM = 22,
CT_UG_REQUEST_ADD_WITH_EMAIL = 23,
CT_UG_REQUEST_ADD_WITH_WEB = 24,
*/
	if (service_add_extra_data(data, "type", "22")) {
		BROWSER_LOGE("service_add_extra_data is failed.");
		service_destroy(data);
		return EINA_FALSE;
	}
	if (service_add_extra_data(data, "ct_num", number.c_str())) {
		BROWSER_LOGE("service_add_extra_data is failed.");
		service_destroy(data);
		return EINA_FALSE;
	}

	if (!ug_create(NULL, "contacts-details-efl", UG_MODE_FULLVIEW, data, &cbs))
		BROWSER_LOGE("ug_create is failed.");

	if (service_destroy(data))
		BROWSER_LOGE("service_destroy is failed.");

	free(phone_number);
}

Eina_Bool Browser_Common_View::_share_via_nfc(std::string url)
{
	BROWSER_LOGD("[%s]", __func__);
	if (url.empty()) {
		show_msg_popup(BR_STRING_EMPTY);
		return EINA_FALSE;
	}

	struct ug_cbs cbs = {0, };
	cbs.layout_cb = __ug_layout_cb;
	cbs.result_cb = NULL;//__ug_result_cb;
	cbs.destroy_cb = __ug_destroy_cb;
	cbs.priv = (void *)this;

	char *share_url = (char *)strdup(url.c_str());
	if (!share_url) {
		BROWSER_LOGE("strdup failed");
		return EINA_FALSE;
	}

	service_h data = NULL;
	service_create(&data);
	if (data == NULL) {
		BROWSER_LOGE("fail to service_create.");
		return EINA_FALSE;
	}
	if (service_add_extra_data(data, "count", "1")) {
		BROWSER_LOGE("service_add_extra_data is failed.");
		service_destroy(data);
		return EINA_FALSE;
	}
	if (service_add_extra_data(data, "request_type", "data_buffer")) {
		BROWSER_LOGE("service_add_extra_data is failed.");
		service_destroy(data);
		return EINA_FALSE;
	}
	if (service_add_extra_data(data, "request_data", share_url)) {
		BROWSER_LOGE("service_add_extra_data is failed.");
		service_destroy(data);

		free(share_url);
		return EINA_FALSE;
	}

	if(!ug_create(NULL, "share-nfc-efl", UG_MODE_FULLVIEW, data, &cbs))
		BROWSER_LOGE("ug_create is failed.");

	if (service_destroy(data))
		BROWSER_LOGE("service_destroy is failed.");

	free(share_url);

	return EINA_TRUE;
}

Eina_Bool Browser_Common_View::_show_share_popup(const char *url)
{
	BROWSER_LOGE("url=[%s]", url);
	if (!url || strlen(url) == 0) {
		BROWSER_LOGE("url is empty");
		return EINA_FALSE;
	}

	m_share_url = std::string(url);

	m_share_popup = elm_popup_add(m_navi_bar);
	if (!m_share_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_share_popup, "menustyle");
	elm_object_part_text_set(m_share_popup, "title,text", BR_STRING_SHARE);
	evas_object_size_hint_weight_set(m_share_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_share_list = elm_list_add(m_share_popup);
	if (!m_share_list) {
		BROWSER_LOGE("elm_list_add failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_share_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_share_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_list_item_append(m_share_list, BR_STRING_MESSAGES, NULL, NULL, __send_via_message_cb, this);
	elm_list_item_append(m_share_list, BR_STRING_EMAIL, NULL, NULL, __send_via_email_cb, this);

	evas_object_show(m_share_list);

	Evas_Object *cancel_button = elm_button_add(m_share_popup);
	elm_object_text_set(cancel_button, BR_STRING_CANCEL);
	elm_object_part_content_set(m_share_popup, "button1", cancel_button);
	elm_object_style_set(cancel_button, "popup_button/default");
	evas_object_smart_callback_add(cancel_button, "clicked", __popup_response_cb, this);

	elm_object_content_set(m_share_popup, m_share_list);

	evas_object_show(m_share_popup);

	return EINA_TRUE;
}

void Browser_Common_View::__popup_response_cb(void* data, Evas_Object* obj, void* event_info)
{
	BROWSER_LOGD("%s, event_info=%d", __func__, (int)event_info);

	if (!data)
		return;

	Browser_Common_View *common_view = (Browser_Common_View *)data;
	if (common_view->m_share_popup) {
		evas_object_del(common_view->m_share_popup);
		common_view->m_share_popup = NULL;
	}
	if (common_view->m_share_list) {
		evas_object_del(common_view->m_share_list);
		common_view->m_share_list = NULL;
	}
}

char *Browser_Common_View::_trim(char *str)
{
	char *pos_bos = str;

	while(*pos_bos == ' ')
		pos_bos++;

	char *pos_eos = pos_bos + strlen(pos_bos) - 1;

	while((pos_eos >= str) && (*pos_eos == ' ')) {
		*pos_eos = '\0';
		pos_eos--;
	}

	return pos_bos;
}

#if defined(HORIZONTAL_UI)
Eina_Bool Browser_Common_View::is_landscape(void)
{
	app_device_orientation_e rotation_value = app_get_device_orientation();
	if (rotation_value == APP_DEVICE_ORIENTATION_0 || rotation_value == APP_DEVICE_ORIENTATION_180)
		return EINA_FALSE;
	else
		return EINA_TRUE;
}
#endif

/* set focus to edit field idler callback to show ime. */
Eina_Bool Browser_Common_View::__set_focus_editfield_idler_cb(void *edit_field)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!edit_field)
		return ECORE_CALLBACK_CANCEL;
	elm_object_focus_set((Evas_Object *)edit_field, EINA_TRUE);
	elm_object_signal_emit((Evas_Object *)edit_field, "clicked", "elm");

	Evas_Object *entry = br_elm_editfield_entry_get((Evas_Object *)edit_field);
	elm_entry_cursor_end_set(entry);

	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool Browser_Common_View::_has_url_sheme(const char *url)
{
	if (url && strlen(url)
	    && (strstr(url, BROWSER_URL_SCHEME_CHECK) || strstr(url, BROWSER_MAIL_TO_SCHEME)))
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

void Browser_Common_View::__ug_layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!priv || !ug)
		return;

	Browser_Common_View *common_view = (Browser_Common_View *)priv;
	Evas_Object *base = (Evas_Object*)ug_get_layout(ug);
	if (!base)
		return;

	common_view->m_ug = ug;

	Evas_Object *win = (Evas_Object *)ug_get_window();

	switch (mode) {
	case UG_MODE_FULLVIEW:
		evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);		
		elm_win_resize_object_add(win, base);
		evas_object_show(base);
		break;
	default:
		break;
	}
}

void Browser_Common_View::__ug_result_cb(ui_gadget_h ug, bundle *result, void *priv)
{
	if (!priv || !ug)
		return;
}

void Browser_Common_View::__ug_destroy_cb(ui_gadget_h ug, void *priv)
{
	if (!priv || !ug)
		return;

	if (ug_destroy(ug))
		BROWSER_LOGD("ug_destroy is failed.\n");
}

