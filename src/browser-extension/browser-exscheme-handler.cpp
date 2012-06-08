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

#include <ail.h>

#include "browser-common-view.h"
#include "browser-exscheme-handler.h"
#include "browser-view.h"

Browser_View *Browser_Exscheme_Handler::m_browser_view = NULL;
Evas_Object *Browser_Exscheme_Handler::m_confirm_popup = NULL;
Evas_Object *Browser_Exscheme_Handler::m_webview = NULL;
std::string Browser_Exscheme_Handler::m_excheme_url;
Browser_Exscheme_Handler Browser_Exscheme_Handler::m_excheme_handler;

Browser_Exscheme_Handler::Browser_Exscheme_Handler(void)
{
	BROWSER_LOGD("[%s]", __func__);
}

Browser_Exscheme_Handler::~Browser_Exscheme_Handler(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (m_confirm_popup)
		evas_object_del(m_confirm_popup);
}

void Browser_Exscheme_Handler::init(Browser_View *browser_view, Evas_Object *webview)
{
	BROWSER_LOGD("[%s]", __func__);
	m_browser_view = browser_view;
	m_webview = webview;

	elm_webview_scheme_callback_set(webview, "rtsp", __rtsp_cb);
	elm_webview_scheme_callback_set(webview, "mailto", __mail_to_cb);
	elm_webview_scheme_callback_set(webview, "sms", __sms_cb);
	elm_webview_scheme_callback_set(webview, "mms", __mms_cb);
	elm_webview_scheme_callback_set(webview, "daumtv", __daum_tv_cb);
}

Eina_Bool Browser_Exscheme_Handler::__daum_tv_cb(Evas_Object *webview, const char *uri)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!uri || !strlen(uri))
		return EINA_FALSE;

	if (strlen(uri) <= strlen(BROWSER_DAUM_TV_SCHEME))
		return EINA_FALSE;

	std::string uri_string = std::string(uri);
	if (!_launch_daum_tv(uri_string))
		BROWSER_LOGE("_launch_daum_tv failed");

}

Eina_Bool Browser_Exscheme_Handler::__mms_cb(Evas_Object *webview, const char *uri)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!uri || !strlen(uri))
		return EINA_FALSE;

	/* Same with sms */
	return __sms_cb(webview, uri);
}

Eina_Bool Browser_Exscheme_Handler::__sms_cb(Evas_Object *webview, const char *uri)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!uri || strlen(uri) <= strlen(BROWSER_SMS_SCHEME))
		return EINA_FALSE;

	int scheme_length = strlen(BROWSER_SMS_SCHEME);
	char *body = NULL;
	if (body = strstr((char *)uri, "?body=")) {
		body = body + strlen("?body=");
		BROWSER_LOGD("body=[%s]", body);
	}

	char *number = NULL;
	if ((number = strstr((char *)uri, ",")) || (number = strstr((char *)uri, "?")))
		*number = '\0';

	number = (char *)uri + strlen(BROWSER_SMS_SCHEME);
	BROWSER_LOGD("number=[%s]", number);

	struct ug_cbs cbs = {0, };
	cbs.layout_cb = __ug_layout_cb;
	cbs.result_cb = NULL;//__ug_result_cb;
	cbs.destroy_cb = __ug_destroy_cb;
	cbs.priv = (void *)(&m_excheme_handler);

	bundle *b = bundle_create();
	if (b == NULL) {
		BROWSER_LOGE("fail to create bundle.");
		return EINA_FALSE;
	}

	if (number) {
		if (bundle_add(b, "TO", number)) {
			BROWSER_LOGE("bundle_add is failed.");
			bundle_free(b);
			free(body);
			return EINA_FALSE;
		}
	}
	if (body) {
		if (bundle_add(b, "BODY", body)) {
			BROWSER_LOGE("bundle_add is failed.");
			bundle_free(b);
			free(body);
			return EINA_FALSE;
		}
	}

	if (!ug_create(NULL, "msg-composer-efl", UG_MODE_FULLVIEW, b, &cbs))
		BROWSER_LOGE("ug_create is failed.");

	if (bundle_free(b))
		BROWSER_LOGE("bundle_free is failed.");

	return EINA_FALSE;
}

Eina_Bool Browser_Exscheme_Handler::__mail_to_cb(Evas_Object *webview, const char *uri)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!uri || !strlen(uri))
		return EINA_FALSE;

	if (strlen(uri) <= strlen(BROWSER_MAIL_TO_SCHEME))
		return EINA_FALSE;

	std::string uri_string = std::string(uri);

	service_h service_handle = NULL;

	if (service_create(&service_handle) < 0) {
		BROWSER_LOGE("Fail to create service handle");
		return EINA_FALSE;
	}

	if (!service_handle) {
		BROWSER_LOGE("service handle is NULL");
		return EINA_FALSE;
	}

	if (service_set_operation(service_handle, SERVICE_OPERATION_SEND) < 0) {
		BROWSER_LOGE("Fail to set service operation");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_set_uri(service_handle, uri_string.c_str() + strlen("mailto:")) < 0) {
		BROWSER_LOGE("Fail to set uri");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_set_package(service_handle, SEC_EMAIL) < 0) {
		BROWSER_LOGE("Fail to set package");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	if (service_send_launch_request(service_handle, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch service operation : org.tizen.email");
		service_destroy(service_handle);
		return EINA_FALSE;
	}

	service_destroy(service_handle);

	return EINA_TRUE;
}

Eina_Bool Browser_Exscheme_Handler::__rtsp_cb(Evas_Object *webview, const char *uri)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!uri || !strlen(uri))
		return EINA_FALSE;

	if (strlen(uri) <= strlen(BROWSER_RTSP_SCHEME))
		return EINA_FALSE;

	std::string uri_string = std::string(uri);
	if (!m_excheme_handler._launch_streaming_player(uri_string.c_str()))
		BROWSER_LOGE("_launch_streaming_player failed");
}

Eina_Bool Browser_Exscheme_Handler::_launch_daum_tv(std::string uri)
{
	BROWSER_LOGD("[%s]", __func__);

	if (!uri.empty()) {
		string converted_url = uri;
		if (converted_url.find(BROWSER_DAUM_TV_SCHEME) != string::npos) {
			int pos = converted_url.find(BROWSER_DAUM_TV_SCHEME);
			converted_url.replace(pos, strlen(BROWSER_DAUM_TV_SCHEME),
							BROWSER_HTTP_SCHEME);
		}

		if (!m_excheme_handler._launch_streaming_player(converted_url.c_str())) {
			BROWSER_LOGE("_launch_streaming_player failed");
			return EINA_FALSE;
		}

		return EINA_TRUE;
	}

	return EINA_FALSE;
}

