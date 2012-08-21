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


#include <ail.h>
#include <url_download.h>

#include "browser-common-view.h"
#include "browser-policy-decision-maker.h"

Browser_Policy_Decision_Maker::Browser_Policy_Decision_Maker(Evas_Object *navi_bar, Browser_View *browser_view)
:
	m_wk_page_ref(NULL)
	,m_list_popup(NULL)
	,m_app_list(NULL)
	,m_navi_bar(navi_bar)
	,m_browser_view(browser_view)
{
	BROWSER_LOGD("[%s]", __func__);

	Ewk_Context *ewk_context = ewk_context_default_get();
	ewk_context_did_start_download_callback_set(ewk_context, __download_did_start_cb, this);
}

Browser_Policy_Decision_Maker::~Browser_Policy_Decision_Maker(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_app_list) {
		evas_object_del(m_app_list);
		m_app_list = NULL;
	}
	if(m_list_popup) {
		evas_object_del(m_list_popup);
		m_list_popup = NULL;
	}
}

void Browser_Policy_Decision_Maker::init(WKPageRef page_ref)
{
	BROWSER_LOGD("[%s]", __func__);

	m_wk_page_ref = page_ref;

	WKPagePolicyClient policy_client = {
	        kWKPagePolicyClientCurrentVersion,	/* version */
	        this,	/* clientInfo */
	        __decide_policy_for_navigation_action,	/* decidePolicyForNavigationAction */
	        0,	/* decidePolicyForNewWindowAction; */
	        __decide_policy_for_response_cb,	/* decidePolicyForResponse */
	        0,	/* unableToImplementPolicy */
	};

	WKPageSetPagePolicyClient(m_wk_page_ref, &policy_client);
}

void Browser_Policy_Decision_Maker::deinit(void)
{
	BROWSER_LOGD("[%s]", __func__);

	if (m_wk_page_ref) {
		WKPagePolicyClient policy_client = {0, };
		WKPageSetPagePolicyClient(m_wk_page_ref, &policy_client);
	}
}

void Browser_Policy_Decision_Maker::pause(void)
{
	if (m_list_popup)
		__popup_response_cb(this, NULL, NULL);
}

void Browser_Policy_Decision_Maker::__download_did_start_cb(const char *download_url, void *user_data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!user_data)
		return;

	Browser_Policy_Decision_Maker *decision_maker = (Browser_Policy_Decision_Maker *)user_data;
	BROWSER_LOGD("download_url=[%s]", download_url);

	if (!decision_maker->_launch_download_app(download_url))
		BROWSER_LOGE("_launch_download_app failed");
}

Eina_Bool Browser_Policy_Decision_Maker::_handle_exscheme(void)
{
	BROWSER_LOGD("request_url = [%s]", m_url.c_str());
	if (m_url.empty()) {
		BROWSER_LOGE("url is null");
		return EINA_FALSE;
	}

	if (!m_url.compare(0, strlen(BROWSER_HTTP_SCHEME), BROWSER_HTTP_SCHEME)
	    || !m_url.compare(0, strlen(BROWSER_HTTPS_SCHEME), BROWSER_HTTPS_SCHEME)
	    || !m_url.compare(0, strlen(BROWSER_FILE_SCHEME), BROWSER_FILE_SCHEME))
		return EINA_FALSE;

	if (!m_url.compare(0, strlen(BROWSER_RTSP_SCHEME), BROWSER_RTSP_SCHEME)) {
		BROWSER_LOGD("rtsp scheme");
		if (!_launch_streaming_player(m_url.c_str()))
			BROWSER_LOGE("_launch_streaming_player failed");
		return EINA_TRUE;
	} else if (!m_url.compare(0, strlen(BROWSER_MAIL_TO_SCHEME), BROWSER_MAIL_TO_SCHEME)) {
		BROWSER_LOGD("mail to scheme");
		if (_send_via_email(m_url.c_str()) != EINA_TRUE)
			BROWSER_LOGE("_send_via_email failed");
		return EINA_TRUE;
	} else if (!m_url.compare(0, strlen(BROWSER_SMS_SCHEME), BROWSER_SMS_SCHEME)) {
		BROWSER_LOGD("sms scheme");
		std::string body_string;

		if (m_url.find("?body=") != string::npos) {
			body_string = m_url.substr(m_url.find("?body="));
			m_url = m_url.substr(0, m_url.length() - body_string.length());
		}

		if (!body_string.empty())
			body_string = std::string(body_string.c_str() + strlen("?body="));
		_send_via_message(body_string, std::string(m_url.c_str() + strlen(BROWSER_SMS_SCHEME)));
		return EINA_TRUE;
	} else if (!m_url.compare(0, strlen(BROWSER_SMS_TO_SCHEME), BROWSER_SMS_TO_SCHEME)) {
		BROWSER_LOGD("sms scheme");
		std::string body_string;

		if (m_url.find("?body=") != string::npos) {
			body_string = m_url.substr(m_url.find("?body="));
			m_url = m_url.substr(0, m_url.length() - body_string.length());
		}

		if (!body_string.empty())
			body_string = std::string(body_string.c_str() + strlen("?body="));
		_send_via_message(body_string, std::string(m_url.c_str() + strlen(BROWSER_SMS_TO_SCHEME)));
		return EINA_TRUE;
	} else if (!m_url.compare(0, strlen(BROWSER_MMS_SCHEME), BROWSER_MMS_SCHEME)) {
		BROWSER_LOGD("mms scheme");
		std::string body_string;

		if (m_url.find("?body=") != string::npos || m_url.find("&body=") != string::npos) {
			if (m_url.find("?body=") != string::npos)
				body_string = m_url.substr(m_url.find("?body="));
			else
				body_string = m_url.substr(m_url.find("&body="));
			m_url = m_url.substr(0, m_url.length() - body_string.length());
		}
		if (m_url.find("?subject=") != string::npos) {
			m_url = m_url.substr(0, m_url.length() - m_url.substr(m_url.find("?subject=")).length());
		}

		if (!body_string.empty())
			body_string = std::string(body_string.c_str() + strlen("?body="));
		_send_via_message(body_string, std::string(m_url.c_str() + strlen(BROWSER_MMS_SCHEME)));
		return EINA_TRUE;
	} else if (!m_url.compare(0, strlen(BROWSER_MMS_TO_SCHEME), BROWSER_MMS_TO_SCHEME)) {
		BROWSER_LOGD("mms scheme");
		std::string body_string;

		if (m_url.find("?body=") != string::npos || m_url.find("&body=") != string::npos) {
			if (m_url.find("?body=") != string::npos)
				body_string = m_url.substr(m_url.find("?body="));
			else
				body_string = m_url.substr(m_url.find("&body="));

			m_url = m_url.substr(0, m_url.length() - body_string.length());
		}
		if (m_url.find("?subject=") != string::npos) {
			m_url = m_url.substr(0, m_url.length() - m_url.substr(m_url.find("?subject=")).length());
		}

		if (!body_string.empty())
			body_string = std::string(body_string.c_str() + strlen("?body="));
		_send_via_message(body_string, std::string(m_url.c_str() + strlen(BROWSER_MMS_TO_SCHEME)));

		return EINA_TRUE;
	} else if (!m_url.compare(0, strlen(BROWSER_WTAI_WP_AP_SCHEME), BROWSER_WTAI_WP_AP_SCHEME)) {
		BROWSER_LOGD("wtai://wp/mc or wtai://wp/ap");
		m_url = std::string(m_url.c_str() + strlen(BROWSER_WTAI_WP_AP_SCHEME));

		if (m_url.find(";") != string::npos) {
			m_url = m_url.substr(0, m_url.length() - m_url.substr(m_url.find(";")).length());
		}

		BROWSER_LOGD("phone number = [%s]", m_url.c_str());

		_add_to_contact(m_url);

		return EINA_TRUE;
	}

	return EINA_FALSE;
}

void Browser_Policy_Decision_Maker::__decide_policy_for_navigation_action(
		WKPageRef page, WKFrameRef frame, WKFrameNavigationType navigationType,
		WKEventModifiers modifiers, WKEventMouseButton mouseButton,
		WKURLRequestRef request, WKFramePolicyListenerRef listener,
		WKTypeRef userData, const void* client_info)
{
	if (!client_info)
		return;

	BROWSER_LOGD("%s", __func__);
	Browser_Policy_Decision_Maker *decision_maker = (Browser_Policy_Decision_Maker *)client_info;

	WKURLRef url_ref = WKURLRequestCopyURL(request);
	WKStringRef url_string_ref = WKURLCopyString(url_ref);
	decision_maker->m_url = decision_maker->_convert_WKStringRef_to_string(url_string_ref);
	WKRelease(url_string_ref);
	WKRelease(url_ref);

	decision_maker->m_cookies.clear();

	if (decision_maker->_handle_exscheme())
		WKFramePolicyListenerIgnore(listener);
	else
		WKFramePolicyListenerUse(listener);
}

void Browser_Policy_Decision_Maker::__decide_policy_for_response_cb(
		WKPageRef page, WKFrameRef frame,
                WKURLResponseRef response, WKURLRequestRef request,
                WKFramePolicyListenerRef listener, WKTypeRef user_data,
                const void *client_info)
{
	if (!client_info)
		return;

	Browser_Policy_Decision_Maker *decision_maker = (Browser_Policy_Decision_Maker *)client_info;

	WKStringRef content_type_ref = WKURLResponseEflCopyContentType(response);
	string content_type = decision_maker->_convert_WKStringRef_to_string(content_type_ref);
	int policy_type = decision_maker->_decide_policy_type(frame, content_type_ref, content_type);
	WKRelease(content_type_ref);

	switch (policy_type) {
	case policy_use:
		BROWSER_LOGD("policy_use");
		WKFramePolicyListenerUse(listener);
		break;

	case policy_download:
		BROWSER_LOGD("policy_download");
		decision_maker->_request_download(request, response, content_type);
		WKFramePolicyListenerIgnore(listener);
		break;

	case policy_ignore:
	default:
		BROWSER_LOGD("policy_ignore");
		WKFramePolicyListenerIgnore(listener);
		break;
	}
}

/* Warning : MUST free() returned char* */
char *Browser_Policy_Decision_Maker::_convert_WKStringRef_to_cstring(WKStringRef string_ref)
{
	if (!string_ref)
		return NULL;

	size_t length = WKStringGetMaximumUTF8CStringSize(string_ref);
	if (length <= 1)	/* returned length is 1 if string_ref is blank. */
		return NULL;

	char *cstring = (char *)calloc(length, sizeof(char));
	if (!cstring) {
		BROWSER_LOGE("calloc failed!");
		return NULL;
	}

	WKStringGetUTF8CString(string_ref, cstring, length);
	return cstring;
}

string Browser_Policy_Decision_Maker::_convert_WKStringRef_to_string(WKStringRef string_ref)
{
	char *cstring = _convert_WKStringRef_to_cstring(string_ref);
	if (!cstring)
		return string();

	string str(cstring);
	free(cstring);
	return str;
}

int Browser_Policy_Decision_Maker::_decide_policy_type(WKFrameRef frame, WKStringRef content_type_ref, string &content_type)
{
	/* ToDo making a decision for SLP browser's policy system first */
	if (content_type.empty())
		return policy_download;

	if (WKFrameCanShowMIMEType(frame, content_type_ref))
		return policy_use;

	return policy_download;
}

void Browser_Policy_Decision_Maker::_request_download(WKURLRequestRef request, WKURLResponseRef response, string& content_type)
{

	string extension_name;
	string ambiguous_mime1 = "text/plain";
	string ambiguous_mime2 = "application/octet-stream";
	int ret = 0;
	char buff[256] = {0,};

	BROWSER_LOGD("[%s]", __func__);

	m_url.clear();
	m_cookies.clear();
	m_default_player_pkg_name.clear();

	WKURLRef url_ref = WKURLRequestCopyURL(request);
	WKStringRef url_string_ref = WKURLCopyString(url_ref);
	m_url = _convert_WKStringRef_to_string(url_string_ref);
	WKRelease(url_string_ref);
	WKRelease(url_ref);

	WKStringRef cookies_ref = WKURLRequestEflCopyCookies(request);
	m_cookies = _convert_WKStringRef_to_string(cookies_ref);
	WKRelease(cookies_ref);

	BROWSER_LOGD("url=[%s]", m_url.c_str());
	BROWSER_LOGD("cookie=[%s]", m_cookies.c_str());

	if (content_type.empty()) {
		BROWSER_LOGD("Download linked file from cotent menu");
		if (!_launch_download_app(m_url.c_str(), m_cookies.c_str()))
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
#ifdef SUPPORT_HLS
		if (extension_name.compare("m3u") == 0 ||
				extension_name.compare("m3u8") == 0) {
			_launch_streaming_player(m_url.c_str(), m_cookies.c_str());
			return;
		} else
#endif
		if (extension_name.compare("mp4") == 0 || extension_name.compare("3gp") == 0) {
			m_default_player_pkg_name = SEC_VIDEO_PLAYER;
			if (!_show_app_list_popup())
				BROWSER_LOGE("_show_app_list_popup failed");
		} else if (extension_name.compare("mp3") == 0) {
			m_default_player_pkg_name = SEC_MUSIC_PLAYER;
			if (!_show_app_list_popup())
				BROWSER_LOGE("_show_app_list_popup failed");
		} else {
			if (!_launch_download_app(m_url.c_str(), m_cookies.c_str()))
				BROWSER_LOGE("_launch_download_app failed");
		}
		return;
	}

#ifdef SUPPORT_HLS
	if (content_type.compare("application/vnd.apple.mpegurl") == 0 ||
					content_type.compare("application/x-mpegurl") == 0 ||
					content_type.compare("application/m3u") == 0 ||
					content_type.compare("audio/x-mpegurl") == 0 ||
					content_type.compare("audio/m3u") == 0 ||
					content_type.compare("audio/x-m3u") == 0) {
		_launch_streaming_player(m_url.c_str(), m_cookies.c_str());
		return;
	}
#endif

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
		if (!_launch_download_app(m_url.c_str(), m_cookies.c_str()))
			BROWSER_LOGE("_launch_download_app failed");
	}
}

Eina_Bool Browser_Policy_Decision_Maker::_launch_download_app(const char *url, const char* cookie)
{
	service_h service_handle = NULL;
	BROWSER_LOGD("%s", __func__);
	if (url && strlen(url)) {

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

		if (service_set_uri(service_handle, url) < 0) {
			BROWSER_LOGE("Fail to set uri");
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

string Browser_Policy_Decision_Maker::_get_extension_name_from_url(string& url)
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

const char *Browser_Policy_Decision_Maker::_get_app_name_from_pkg_name(string& pkg_name)
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

void Browser_Policy_Decision_Maker::__player_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("%s", __func__);
	if (!data)
		return;

	Browser_Policy_Decision_Maker *decision_maker = (Browser_Policy_Decision_Maker *)data;

	if (!decision_maker->_launch_streaming_player(decision_maker->m_url.c_str(), decision_maker->m_cookies.c_str()))
		BROWSER_LOGE("_launch_streaming_player failed");

	__popup_response_cb(decision_maker, NULL, NULL);
}

void Browser_Policy_Decision_Maker::__internet_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("%s", __func__);
	if (!data)
		return;

	Browser_Policy_Decision_Maker *decision_maker = (Browser_Policy_Decision_Maker *)data;

	if (!decision_maker->_launch_download_app(decision_maker->m_url.c_str(), decision_maker->m_cookies.c_str()))
		BROWSER_LOGE("_launch_download_app failed");

	__popup_response_cb(decision_maker, NULL, NULL);
}

Eina_Bool Browser_Policy_Decision_Maker::_show_app_list_popup(void)
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
	if (!cancel_button) {
		BROWSER_LOGE("elm_button_add failed");
		return EINA_FALSE;
	}
	elm_object_text_set(cancel_button, BR_STRING_CLOSE);
	elm_object_part_content_set(m_list_popup, "button1", cancel_button);
	evas_object_smart_callback_add(cancel_button, "clicked", __popup_response_cb, this);

	return EINA_TRUE;
}

void Browser_Policy_Decision_Maker::__popup_response_cb(void* data, Evas_Object* obj,
		void* event_info)
{
	BROWSER_LOGD("%s", __func__);

	if (!data)
		return;

	Browser_Policy_Decision_Maker *decision_maker = (Browser_Policy_Decision_Maker *)data;
	if (decision_maker->m_app_list) {
		evas_object_del(decision_maker->m_app_list);
		decision_maker->m_app_list = NULL;
	}
	if(decision_maker->m_list_popup) {
		evas_object_del(decision_maker->m_list_popup);
		decision_maker->m_list_popup = NULL;
	}
}

