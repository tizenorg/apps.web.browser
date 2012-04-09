/*
Copyright (c) 2000-2012 Samsung Electronics Co., Ltd All Rights Reserved

This file is part of browser
Written by Hyerim Bae hyerim.bae@samsung.com.

PROPRIETARY/CONFIDENTIAL

This software is the confidential and proprietary information of
SAMSUNG ELECTRONICS ("Confidential Information"). You shall not
disclose such Confidential Information and shall use it only in
accordance with the terms of the license agreement you entered
into with SAMSUNG ELECTRONICS.

SAMSUNG make no representations or warranties about the suitability
of the software, either express or implied, including but not limited
to the implied warranties of merchantability, fitness for a particular
purpose, or non-infringement. SAMSUNG shall not be liable for any
damages suffered by licensee as a result of using, modifying or
distributing this software or its derivatives.
*/

#include <ail.h>

#include "browser-class.h"
#include "browser-common-view.h"
#include "browser-download-manager.h"
#include <url_download.h>

Browser_Download_Manager::Browser_Download_Manager(Evas_Object *navi_bar, Browser_View *browser_view)
:
	m_webview(NULL)
	,m_list_popup(NULL)
	,m_app_list(NULL)
	,m_navi_bar(navi_bar)
	,m_browser_view(browser_view)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Download_Manager::~Browser_Download_Manager(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if(m_list_popup) {
		evas_object_del(m_list_popup);
		m_list_popup = NULL;
	}
}

void Browser_Download_Manager::init(Evas_Object *webview)
{
	BROWSER_LOGD("[%s]", __func__);

	m_webview = webview;

	Evas_Object *webkit = elm_webview_webkit_get(m_webview);
	deinit();
	evas_object_smart_callback_add(webkit, "download,request", __download_request_cb, this);
}

void Browser_Download_Manager::deinit(void)
{
	BROWSER_LOGD("[%s]", __func__);

	Evas_Object *webkit = elm_webview_webkit_get(m_webview);
	evas_object_smart_callback_del(webkit, "download,request", __download_request_cb);
}

void Browser_Download_Manager::pause(void)
{
	if (m_list_popup)
		__popup_response_cb(this, NULL, NULL);
}

void Browser_Download_Manager::__download_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Download_Manager *download_manager = (Browser_Download_Manager *)data;
	if (event_info)
		download_manager->_request_download((Ewk_Download *)event_info);
}

void Browser_Download_Manager::_request_download(Ewk_Download *download_info)
{
	string extension_name;
	string ambiguous_mime1 = "text/plain";
	string ambiguous_mime2 = "application/octet-stream";
	int ret = 0;
	char buff[256] = {0,};

	m_cookies.clear();
	m_url.clear();
	m_default_player_pkg_name.clear();

	if (!download_info->url) {
		BROWSER_LOGE("download url is null");
		return;
	}

	string content_type;
	if (download_info->mime_type)
		content_type = string(download_info->mime_type);

	m_url = string(download_info->url);
	if (download_info->user_param)
		m_cookies = string(download_info->user_param);

	if (content_type.empty()) {
		BROWSER_LOGD("Download linked file from cotent menu");
		if (!_launch_download_app())
			BROWSER_LOGE("_launch_download_app failed");
		return;
	}
	BROWSER_LOGD("mime type [%s]", content_type.c_str());
	/* If mime is ambiguous mime or not. If it is, get extension name from url */
	if (content_type.compare(ambiguous_mime1) == 0
	    || content_type.compare(ambiguous_mime2) == 0) {
		extension_name = _get_extension_name_from_url(m_url);
	}
	/* If mime is ambiguous and extension name is existed,
	*   decide streaming player according to extension name from url
	* 1. mp4, 3gp : streaming video player case
	* 2. mp3 : streaming music player case
	* 3. otherewise : download app case
	*/
	if (!extension_name.empty()) {
		BROWSER_LOGD("extension name from url [%s]", extension_name.c_str());
		if (extension_name.compare("mp4") == 0 || extension_name.compare("3gp") == 0) {
			m_default_player_pkg_name = SEC_VIDEO_PLAYER;
			if (!_show_app_list_popup())
				BROWSER_LOGE("_show_app_list_popup failed");
		} else if (extension_name.compare("mp3") == 0) {
			m_default_player_pkg_name = SEC_MUSIC_PLAYER;
			if (!_show_app_list_popup())
				BROWSER_LOGE("_show_app_list_popup failed");
		} else {
			if (!_launch_download_app())
				BROWSER_LOGE("_launch_download_app failed");
		}
		return;
	}
	/* If the default player is registered at AUL db, show list popup with the name of it */
	ret = aul_get_defapp_from_mime(content_type.c_str(), buff, (sizeof(buff)-1));
	if (ret == AUL_R_OK) {
		m_default_player_pkg_name = buff;
		BROWSER_LOGD("default app [%s]", m_default_player_pkg_name.c_str());
	} else {
		BROWSER_LOGE("Fail to get default app");
	}

	/* Call streaming player app only if the default player is samsung music player or samsung video plyaer
	*  Otherwiser, call download app
	*/
	if (!m_default_player_pkg_name.empty() && (m_default_player_pkg_name.compare(SEC_VIDEO_PLAYER) == 0 ||
		m_default_player_pkg_name.compare(SEC_MUSIC_PLAYER) == 0)) {
		if (!_show_app_list_popup())
			BROWSER_LOGE("_show_app_list_popup failed");
	} else {
		if (!_launch_download_app())
			BROWSER_LOGE("_launch_download_app failed");
	}
}

Eina_Bool Browser_Download_Manager::_launch_download_app(void)
{
	service_h service_handle = NULL;
	BROWSER_LOGD("%s", __func__);
	if (!m_url.empty()) {

		if (service_create(&service_handle) < 0) {
			BROWSER_LOGE("Fail to create service handle");
			return EINA_FALSE;
		}
		
		if (!service_handle) {
			BROWSER_LOGE("service handle is NULL");
			return EINA_FALSE;
		}

		if (service_set_operation(service_handle, SERVICE_OPERATION_DOWNLOAD) < 0) {
			BROWSER_LOGE("Fail to set service operation");
			service_destroy(service_handle);
			return EINA_FALSE;
		}

		if (service_set_uri(service_handle, m_url.c_str()) < 0) {
			BROWSER_LOGE("Fail to set uri");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
		if (!m_cookies.empty()) {
			if (service_add_extra_data(service_handle, "cookie", m_cookies.c_str()) < 0) {
				BROWSER_LOGE("Fail to set extra data");
				service_destroy(service_handle);
				return EINA_FALSE;
			}
		}

		if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
			BROWSER_LOGE("Fail to launch service operation");
			service_destroy(service_handle);
			return EINA_FALSE;
		}
		service_destroy(service_handle);

		return EINA_TRUE;
	}

	return EINA_FALSE;
}

Eina_Bool Browser_Download_Manager::_call_streaming_player(void)
{
	if (!_launch_streaming_player(m_url.c_str(), m_cookies.c_str())) {
		BROWSER_LOGE("_launch_streaming_player failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

string Browser_Download_Manager::_get_extension_name_from_url(string& url)
{
	string ext;
	size_t pos = 0;

	if(url.empty()) {
		BROWSER_LOGE("url is NULL");
		return string();
	}
	pos = url.find_last_of("/");

	if (pos != string::npos) {
		string tmp;
		tmp = url.substr(pos + 1);
		pos = tmp.find_last_of(".");
		if (pos != string::npos) {
			string tmp2;
			size_t end = 0;
			tmp2 = tmp.substr(pos+1);
			pos = tmp2.find_last_of("?");
			if (pos != string::npos)
				ext = tmp2.substr(0, pos - 1);
			else
				ext = tmp2;
		}
	}
	BROWSER_LOGD("extension name[%s]\n",ext.c_str());
	return ext;
}

const char *Browser_Download_Manager::_get_app_name_from_pkg_name(string& pkg_name)
{
	ail_appinfo_h handle;
	ail_error_e ret;
	string app_name;
	char *str = NULL;

	ret = ail_package_get_appinfo(pkg_name.c_str(), &handle);
	if (ret != AIL_ERROR_OK) {
		return NULL;
	}

	ret = ail_appinfo_get_str(handle, AIL_PROP_NAME_STR, &str);
	if (ret != AIL_ERROR_OK) {
		return NULL;
	}
	app_name = (const char*) str;
	BROWSER_LOGD("pkg's name[%s]pkg[%s]\n",app_name.c_str(),pkg_name.c_str());

	ret = ail_package_destroy_appinfo(handle);
	if (ret != AIL_ERROR_OK)
		return NULL;

	return app_name.c_str();
}

void Browser_Download_Manager::__player_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("%s", __func__);
	if (!data)
		return;

	Browser_Download_Manager *download_manager = (Browser_Download_Manager *)data;

	if (!download_manager->_call_streaming_player())
		BROWSER_LOGE("_call_streaming_player failed");

	__popup_response_cb(download_manager, NULL, NULL);
}

void Browser_Download_Manager::__internet_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("%s", __func__);
	if (!data)
		return;

	Browser_Download_Manager *download_manager = (Browser_Download_Manager *)data;

	if (!download_manager->_launch_download_app())
		BROWSER_LOGE("_launch_download_app failed");

	__popup_response_cb(download_manager, NULL, NULL);
}

Eina_Bool Browser_Download_Manager::_show_app_list_popup(void)
{
	if (m_url.empty()) {
		BROWSER_LOGE("url is empty");
		return EINA_FALSE;
	}

	m_list_popup = elm_popup_add(m_navi_bar);
	if (!m_list_popup) {
		BROWSER_LOGE("elm_popup_add failed");
		return EINA_FALSE;
	}
	elm_object_style_set(m_list_popup, "menustyle");
	elm_object_part_text_set(m_list_popup, "title,text", BR_STRING_TITLE_SELECT_AN_ACTION);
	evas_object_size_hint_weight_set(m_list_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	m_app_list = elm_list_add(m_list_popup);
	if (!m_app_list) {
		BROWSER_LOGE("elm_list_add failed");
		return EINA_FALSE;
	}
	evas_object_size_hint_weight_set(m_app_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(m_app_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

	const char *app_name = _get_app_name_from_pkg_name(m_default_player_pkg_name);
	if (app_name && strlen(app_name))
		elm_list_item_append(m_app_list, app_name, NULL, NULL, __player_cb, this);

	elm_list_item_append(m_app_list, BR_STRING_INTERNET, NULL, NULL, __internet_cb, this);
	evas_object_show(m_app_list);

	elm_object_content_set(m_list_popup, m_app_list);
	evas_object_show(m_list_popup);

	Evas_Object *cancel_button = elm_button_add(m_list_popup);
	elm_object_text_set(cancel_button, BR_STRING_CLOSE);
	elm_object_part_content_set(m_list_popup, "button1", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __popup_response_cb, this);
}

void Browser_Download_Manager::__popup_response_cb(void* data, Evas_Object* obj,
		void* event_info)
{
	BROWSER_LOGD("%s", __func__);

	if (!data)
		return;

	Browser_Download_Manager *download_manager = (Browser_Download_Manager *)data;

	if (download_manager->m_list_popup) {
		evas_object_del(download_manager->m_list_popup);
		download_manager->m_list_popup = NULL;
	}
}

