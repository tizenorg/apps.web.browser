/*
 * Copyright 2015 Samsung Electronics Co., Ltd
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
 * Contact: Michal Poteralski <m.poteralski@samsung.com>
 *
 */

#include "html5-feature-manager.h"

#include "browser.h"
#include "browser-dlog.h"
#include "preference.h"

#ifdef ENABLE_WEB_NOTIFICATION_API
#include "web-notification.h"
#include <notification.h>
#include <notification_internal.h>
#endif // ENABLE_WEB_NOTIFICATION_API

int html5_feature_manager::m_instance_count;
haptic_device_h html5_feature_manager::m_haptic_handle;
Ecore_Timer *html5_feature_manager::m_haptic_timer_id;
haptic_effect_h html5_feature_manager::m_haptic_effect;

html5_feature_manager::html5_feature_manager(webview *wv) : m_webview(wv)
{
	BROWSER_LOGD("");

#ifdef ENABLE_WEB_NOTIFICATION_API
	ewk_view_notification_permission_callback_set(m_webview->get_ewk_view(), __notification_permission_request_cb, this);
#ifdef WEBKIT_EFL
    m_webview->attach_event("notification,cancel", __notification_cancel_cb, this);
    m_webview->attach_event("notification,show", __notification_show_cb, this);
#else
    ewk_notification_callbacks_set(reinterpret_cast<Ewk_Notification_Show_Callback>(__notification_show_cb),
            reinterpret_cast<Ewk_Notification_Cancel_Callback>(__notification_cancel_cb),
            this);
#endif // WEBKIT_EFL
#endif // ENABLE_WEB_NOTIFICATION_API

	++m_instance_count;
	if (m_instance_count == 1) {
		BROWSER_LOGD("Connecting vibration callbacks.");
		webview_context::instance()->vibration_callbacks_set(__vibration_cb,
			__vibration_cancel_cb, NULL);
	}
}

html5_feature_manager::~html5_feature_manager()
{
	BROWSER_LOGD("");

	cancel_vibration();

	if (m_instance_count == 1) {
		BROWSER_LOGD("Disconnecting vibration callbacks.");
		webview_context::instance()->vibration_callbacks_set(NULL, NULL, NULL);
	}
	--m_instance_count;
}
#ifdef ENABLE_WEB_NOTIFICATION_API
Eina_Bool html5_feature_manager::__notification_permission_request_cb(Evas_Object *ewk_view, Ewk_Notification_Permission_Request *request, void *data)
{
	BROWSER_LOGI("");

	RETV_MSG_IF(!data, EINA_FALSE, "data is NULL");
	RETV_MSG_IF(!request, EINA_FALSE, "request is NULL");

	html5_feature_manager *hfm = (html5_feature_manager *)data;
	m_browser->get_web_noti_manager()->handle_permission_request(hfm->m_webview, request);

	return EINA_TRUE;
}

#ifdef WEBKIT_EFL
void html5_feature_manager::__notification_show_cb(void *data, Evas_Object *obj, void *event_info)
{
    BROWSER_LOGD("");

    RET_MSG_IF(!data, "data is NULL");
    RET_MSG_IF(!event_info, "event_info is NULL");

    html5_feature_manager *hfm = (html5_feature_manager *)data;
    m_browser->get_web_noti_manager()->set_web_notification(hfm->m_webview, (Ewk_Notification *)event_info);
}

void html5_feature_manager::__notification_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
    RET_MSG_IF(!event_info, "event_info is NULL");

    BROWSER_LOGD("ewk_id[%d]", *(int *)event_info);
    m_browser->get_web_noti_manager()->unset_web_notification(*(int *)event_info);
}
#else
Eina_Bool html5_feature_manager::__notification_show_cb(Ewk_Notification *notification, void *data)
{
    BROWSER_LOGI("");

    RETV_MSG_IF(!data, EINA_FALSE, "data is NULL");
    RETV_MSG_IF(!notification, EINA_FALSE, "notification is NULL");

    html5_feature_manager *hfm = (html5_feature_manager *)data;
    m_browser->get_web_noti_manager()->set_web_notification(hfm->m_webview, notification);

    return EINA_TRUE;
}

Eina_Bool html5_feature_manager::__notification_cancel_cb(uint64_t notification_id, void *data)
{
    RETV_MSG_IF(!data, EINA_FALSE, "data is NULL");

    BROWSER_LOGD("noti_id[%d]", notification_id);
    m_browser->get_web_noti_manager()->unset_web_notification((int)notification_id);

    return EINA_TRUE;
}
#endif // WEBKIT_EFL
#endif // ENABLE_WEB_NOTIFICATION_API

Eina_Bool html5_feature_manager::__vibration_timeout_cb(void *data)
{
	BROWSER_LOGI("");

	m_haptic_timer_id = NULL;
	cancel_vibration();

	return ECORE_CALLBACK_CANCEL;
}

void html5_feature_manager::__vibration_cb(uint64_t vibration_time, void *data)
{
	BROWSER_LOGI("");

	cancel_vibration();

	if (device_haptic_open(0, &m_haptic_handle) != DEVICE_ERROR_NONE) {
		BROWSER_LOGE("device_haptic_open failed");
		return;
	}

	const uint64_t duration = vibration_time;
	device_haptic_vibrate(m_haptic_handle, duration, 100, &m_haptic_effect);
	const double in = (double)((double)(duration) / (double)(1000));
	BROWSER_SECURE_LOGD("duration=%f", in);

	m_haptic_timer_id = ecore_timer_add(in, __vibration_timeout_cb, NULL);
}

void html5_feature_manager::__vibration_cancel_cb(void *data)
{
	BROWSER_LOGI("");

	cancel_vibration();
}

void html5_feature_manager::cancel_vibration()
{
	BROWSER_LOGI("");

	if (m_haptic_timer_id) {
		ecore_timer_del(m_haptic_timer_id);
		m_haptic_timer_id = NULL;
	}

	if (m_haptic_handle) {
		device_haptic_stop(m_haptic_handle, m_haptic_effect);
		device_haptic_close(m_haptic_handle);
		m_haptic_handle = NULL;
	}
}
