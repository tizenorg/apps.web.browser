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

#include "browser-class.h"
#include "browser-common-view.h"
#include "browser-view.h"
#include "browser-window.h"

Browser_Data_Manager *Browser_Common_View::m_data_manager;
Evas_Object *Browser_Common_View::m_win;
Evas_Object *Browser_Common_View::m_navi_bar;
Evas_Object *Browser_Common_View::m_bg;
Evas_Object *Browser_Common_View::m_layout;
Browser_Class *Browser_Common_View::m_browser;

Browser_Common_View::Browser_Common_View(void)
:
	m_selection_info(NULL)
	,m_selection_info_layout(NULL)
	,m_popup(NULL)
	,m_ug(NULL)
	,m_share_popup(NULL)
	,m_share_list(NULL)
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

	m_sns_path_list.clear();
	m_sns_name_list.clear();
	m_sns_icon_list.clear();
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

Eina_Bool Browser_Common_View::show_modal_popup(const char *msg)
{
	BROWSER_LOGD("msg = [%s]", msg);
	FILE *pipe = NULL;

	std::string source_string = std::string(MODAL_LAUNCHER_BIN_PATH) + std::string(" ")
					+ std::string(MODAL_LAUNCHER_BUNDLE_TYPE) + std::string(" confirm ");

	std::string msg_string = std::string("\"") + std::string(msg) + std::string("\"");

	string::size_type pos = string::npos;
	while ((pos = msg_string.find("\n")) != string::npos)
		msg_string.replace(pos, strlen("\n"), std::string("<br>"));

	source_string = source_string + std::string(MODAL_LAUNCHER_BUNDLE_MESSAGE) + std::string(" ") + msg_string;
	BROWSER_LOGD("source_string = [%s]", source_string.c_str());

	if (!(pipe = popen(source_string.c_str(), "r"))) {
		BROWSER_LOGE("popen filed");
		return EINA_FALSE;
	}

	char read_buffer[MODAL_MSG_MAX_BUFFER] = {0, };
	while (fgets(read_buffer, MODAL_MSG_MAX_BUFFER, pipe)) {
		if (!strncmp(read_buffer, MODAL_LAUNCHER_RESULT_KEYWORD,
					strlen(MODAL_LAUNCHER_RESULT_KEYWORD))) {
			BROWSER_LOGD("modal keyword found.");
			return EINA_TRUE;
		}
	}

	return EINA_FALSE;
}

/* Capture snapshot with current focused ewk view. */
Evas_Object *Browser_Common_View::_capture_snapshot(Browser_Window *window, float scale)
{
	BROWSER_LOGD("[%s]", __func__);
	int focused_ewk_view_w = 0;
	int focused_ewk_view_h = 0;
	evas_object_geometry_get(window->m_ewk_view, NULL, NULL,
						&focused_ewk_view_w, &focused_ewk_view_h);

	Evas_Object *snapshot_image = NULL;
	snapshot_image = evas_object_rectangle_add(evas_object_evas_get(m_navi_bar));
	if (!snapshot_image) {
		BROWSER_LOGE("evas_object_rectangle_add failed");
		return NULL;
	}
	evas_object_size_hint_min_set(snapshot_image, (int)(focused_ewk_view_w * scale),
						(int)(focused_ewk_view_h * scale));
	evas_object_resize(snapshot_image, (int)(focused_ewk_view_w * scale),
						(int)(focused_ewk_view_h * scale));
	evas_object_color_set(snapshot_image, 255, 255, 255, 255);

	return snapshot_image;
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

std::string Browser_Common_View::get_domain_name(const char *url)
{
	if (!url || !strcmp(url, ""))
		return std::string();

	std::string domain_name = url;

	/* replacing multiple "/" in a row with one "/" */
	size_t pos = domain_name.find("//");
	while (pos != std::string::npos) {
		domain_name.erase(pos, 1);
		pos = domain_name.find("//");
	}

	/* cut "xxx:/" prefix */
	pos = domain_name.find(":/");
	if (pos != std::string::npos) {
		/* ":/" length */
		domain_name = domain_name.substr(pos + 2);
	}

	/* cut behind "/" */
	pos = domain_name.find("/");
	if (pos != std::string::npos)
		domain_name = domain_name.substr(0, pos);

	return domain_name;
}

Eina_Bool Browser_Common_View::is_landscape(void)
{
	/* The appcore_get_rotation_state fail in U1 HD target, temporary code. */
	int window_w = 0;
	int window_h = 0;
	evas_object_geometry_get(m_win, NULL, NULL, &window_w, &window_h);
	if (window_w < window_h) {
		BROWSER_LOGD("portrait");
		return EINA_FALSE;
	} else {
		BROWSER_LOGD("landscape");
		return EINA_TRUE;
	}
}

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
	    && (strstr(url, BROWSER_URL_SCHEME_CHECK) || strstr(url, BROWSER_MAIL_TO_SCHEME)
	    || strstr(url, BROWSER_TEL_SCHEME) || strstr(url, BROWSER_YOUTUBE_SCHEME)))
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

void Browser_Common_View::__ug_layout_cb(struct ui_gadget *ug, enum ug_mode mode, void *priv)
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

void Browser_Common_View::__ug_result_cb(struct ui_gadget *ug, bundle *result, void *priv)
{
	if (!priv || !ug)
		return;
}

void Browser_Common_View::__ug_destroy_cb(struct ui_gadget *ug, void *priv)
{
	if (!priv || !ug)
		return;

	if (ug_destroy(ug))
		BROWSER_LOGD("ug_destroy is failed.\n");
}

