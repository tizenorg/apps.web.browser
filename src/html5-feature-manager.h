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

#ifndef HTML5_FEATURE_MANAGER_H
#define HTML5_FEATURE_MANAGER_H

#include <Evas.h>
#include <device/haptic.h>

#include "browser-object.h"
#include "webview.h"

class html5_feature_manager : public browser_object {
public:
	html5_feature_manager(webview *wv);
	virtual ~html5_feature_manager();

private:
#ifdef ENABLE_WEB_NOTIFICATION_API
	static Eina_Bool __notification_permission_request_cb(Evas_Object *ewk_view, Ewk_Notification_Permission_Request *request, void *data);
#ifdef WEBKIT_EFL
    static void __notification_show_cb(void *data, Evas_Object *obj, void *event_info);
    static void __notification_cancel_cb(void *data, Evas_Object *obj, void *event_info);
#else
    static Eina_Bool __notification_show_cb(Ewk_Notification *notification, void *data);
    static Eina_Bool __notification_cancel_cb(uint64_t notification_id, void *data);
#endif // WEBKIT_EFL
#endif // ENABLE_WEB_NOTIFICATION_API

	static Eina_Bool __vibration_timeout_cb(void *data);
	static void __vibration_cb(uint64_t vibration_time, void *data);
	static void __vibration_cancel_cb(void *data);

	static void cancel_vibration();

	webview *m_webview;

	static int m_instance_count;
	static haptic_device_h m_haptic_handle;
	static Ecore_Timer *m_haptic_timer_id;
	static haptic_effect_h m_haptic_effect;
};

#endif // HTML5_FEATURE_MANAGER_H
