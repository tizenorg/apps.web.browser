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

#include "html5-feature-manager.h"

#include <Elementary.h>
#include <string>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-object.h"
#include "browser-string.h"
#include "browser-view.h"
#include "custom-content-handler.h"
#include "custom-protocol-handler.h"
#include "geolocation-manager.h"
#include "preference.h"
#include "webview.h"

#define default_quota_size	(5 * 1024 * 1024) /* 5MB */

int html5_feature_manager::m_instance_count;
haptic_device_h html5_feature_manager::m_haptic_handle;
Ecore_Timer *html5_feature_manager::m_haptic_timer_id;
haptic_effect_h html5_feature_manager::m_haptic_effect;

html5_feature_manager::html5_feature_manager(webview *wv)
:
	m_webview(wv)
{
	BROWSER_LOGD("");

	m_webview->attach_event("fullscreen,enterfullscreen", __fullscreen_enter_cb, this);
	m_webview->attach_event("fullscreen,exitfullscreen", __fullscreen_exit_cb, this);

	m_webview->attach_event("geolocation,permission,request", __geolocation_permission_request_cb, this);
	m_webview->attach_event("geolocation,valid", __geolocation_valid_cb, this);
	m_webview->attach_event("usermedia,permission,request", __usermedia_permission_request_cb, this);

	m_custom_content_handler = new custom_content_handler(m_webview);
	m_custom_protocol_handler = new custom_protocol_handler(m_webview);

	m_instance_count++;
	if (m_instance_count == 1)
		m_webview_context->vibration_callbacks_set(__vibration_on_cb, __vibration_off_cb, NULL);
}

html5_feature_manager::~html5_feature_manager(void)
{
	BROWSER_LOGD("");

	m_webview->detach_event("fullscreen,enterfullscreen", __fullscreen_enter_cb);
	m_webview->detach_event("fullscreen,exitfullscreen", __fullscreen_exit_cb);

	m_webview->detach_event("geolocation,permission,request", __geolocation_permission_request_cb);
	m_webview->detach_event("geolocation,valid", __geolocation_valid_cb);
	m_webview->detach_event("usermedia,permission,request", __usermedia_permission_request_cb);

	if (m_haptic_handle) {
		haptic_stop_effect(m_haptic_handle, m_haptic_effect);
		haptic_close(m_haptic_handle);
		m_haptic_handle = NULL;
	}
	if (m_haptic_timer_id) {
		ecore_timer_del(m_haptic_timer_id);
		m_haptic_timer_id = NULL;
	}

	delete m_custom_content_handler;
	delete m_custom_protocol_handler;

	m_instance_count--;
}

void html5_feature_manager::__usermedia_permission_allow_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	ewk_user_media_permission_request_set((Ewk_User_Media_Permission_Request *)data, EINA_TRUE);
}

void html5_feature_manager::__usermedia_permission_deny_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	ewk_user_media_permission_request_set((Ewk_User_Media_Permission_Request *)data, EINA_FALSE);
}

void html5_feature_manager::__usermedia_permission_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	Ewk_User_Media_Permission_Request *usermedia = (Ewk_User_Media_Permission_Request *)event_info;

	m_browser->get_browser_view()->show_msg_popup(NULL, BR_STRING_USERMEDIA, BR_STRING_ALLOW, __usermedia_permission_allow_cb, BR_STRING_CANCEL, __usermedia_permission_deny_cb, usermedia);
}

void html5_feature_manager::__fullscreen_enter_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_browser->set_full_screen_enable(EINA_TRUE);
}

void html5_feature_manager::__fullscreen_exit_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	m_browser->set_full_screen_enable(EINA_FALSE);
}

Eina_Bool html5_feature_manager::__vibration_timeout_cb(void *data)
{
	BROWSER_LOGD("");
	m_haptic_timer_id = NULL;
	if (m_haptic_handle) {
		haptic_stop_effect(m_haptic_handle, m_haptic_effect);
		haptic_close(m_haptic_handle);
		m_haptic_handle = NULL;
	}

	return ECORE_CALLBACK_CANCEL;
}

void html5_feature_manager::__vibration_on_cb(uint64_t vibration_time, void *data)
{
	BROWSER_LOGD("");
	uint64_t duration = vibration_time;

	if (m_haptic_timer_id) {
		ecore_timer_del(m_haptic_timer_id);
		m_haptic_timer_id = NULL;
	}

	if (m_haptic_handle) {
		haptic_stop_effect(m_haptic_handle, m_haptic_effect);
		haptic_close(m_haptic_handle);
		m_haptic_handle = NULL;
	}

	if (haptic_open(HAPTIC_DEVICE_0, &m_haptic_handle) != HAPTIC_ERROR_NONE) {
		BROWSER_LOGE("haptic_open failed");
		return;
	}

	haptic_vibrate_monotone(m_haptic_handle, duration, &m_haptic_effect);
	double in = (double)((double)(duration) / (double)(1000));
	BROWSER_LOGD("duration=%f", in);

	m_haptic_timer_id = ecore_timer_add(in, __vibration_timeout_cb, NULL);
}

void html5_feature_manager::__vibration_off_cb(void *data)
{
	BROWSER_LOGD("");
	if (m_haptic_timer_id) {
		ecore_timer_del(m_haptic_timer_id);
		m_haptic_timer_id = NULL;
	}

	if (m_haptic_handle) {
		haptic_stop_effect(m_haptic_handle, m_haptic_effect);
		haptic_close(m_haptic_handle);
		m_haptic_handle = NULL;
	}
}

void html5_feature_manager::__geolocation_permission_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Ewk_Geolocation_Permission_Request *permission_request = (Ewk_Geolocation_Permission_Request *)event_info;
	html5_feature_manager *hfm = (html5_feature_manager *)data;

	/* NULL check parameters */
	if (!permission_request) {
		BROWSER_LOGE("permission_data from webkit is as NULL");
		return;
	}

	if (!hfm) {
		BROWSER_LOGE("html5_feature_manager is as NULL");
		return;
	}

	/* get permission request data */
	const Ewk_Security_Origin *origin = ewk_geolocation_permission_request_origin_get(permission_request);
	const char *host_address = ewk_security_origin_host_get(origin);
	if (!host_address) {
		BROWSER_LOGE("Unable to get host name from webkit");
		return;
	}
	BROWSER_LOGD("host_address[%s]", host_address);

	Eina_Bool enable_location = m_preference->get_location_enabled();

	if (enable_location == EINA_TRUE) {
		Eina_Bool is_exist = 0;
		bool is_accepted = false;

		is_exist = m_browser->get_geolocation_manager()->check_geolocation_setting_exist(host_address);

		if (is_exist > 0) {
			BROWSER_LOGD("geolocation info[%s] is existed", host_address);
			if (m_browser->get_geolocation_manager()->get_geolocation_allow(host_address, is_accepted) == EINA_FALSE) {
				BROWSER_LOGE("Unable to get data from geolocation DB");
				return;
			}

			if (is_accepted)
				ewk_geolocation_permission_request_set(permission_request, EINA_TRUE);
			else
				ewk_geolocation_permission_request_set(permission_request, EINA_FALSE);

			return;
		}

		ewk_geolocation_permission_request_suspend(permission_request);
		m_browser->get_geolocation_manager()->show_geolocation_allow_popup(host_address, (void *)permission_request);
	} else
		ewk_geolocation_permission_request_set(permission_request, EINA_FALSE);
}

void html5_feature_manager::__geolocation_valid_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");

	Eina_Bool enable_location = m_preference->get_location_enabled();
	*((bool *)event_info) = enable_location;
}

