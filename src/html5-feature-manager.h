/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.0 (the License);
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

#ifndef HTML5_FEATURE_MANAGER_H
#define HTML5_FEATURE_MANAGER_H

#include "browser-object.h"

#include <Evas.h>
#include <haptic.h>

class custom_content_handler;
class custom_protocol_handler;
class webview;
class html5_feature_manager : public browser_object {
public:
	html5_feature_manager(webview *wv);
	~html5_feature_manager(void);

	custom_content_handler *get_custom_content_handler(void) { return m_custom_content_handler; }
	custom_protocol_handler *get_custom_protocol_handler(void) { return m_custom_protocol_handler; }
private:
	static void __fullscreen_enter_cb(void *data, Evas_Object *obj, void *event_info);
	static void __fullscreen_exit_cb(void *data, Evas_Object *obj, void *event_info);
	static void __geolocation_permission_request_cb(void *data, Evas_Object *obj, void *event_info);
	static void __geolocation_valid_cb(void *data, Evas_Object *obj, void *event_info);
	static void __usermedia_permission_request_cb(void *data, Evas_Object *obj, void *event_info);

	static void __usermedia_permission_allow_cb(void *data, Evas_Object *obj, void *event_info);
	static void __usermedia_permission_deny_cb(void *data, Evas_Object *obj, void *event_info);

	static Eina_Bool __vibration_timeout_cb(void *data);
	static void __vibration_on_cb(uint64_t vibration_time, void *data);
	static void __vibration_off_cb(void *data);

	webview *m_webview;

	static haptic_device_h m_haptic_handle;
	static Ecore_Timer *m_haptic_timer_id;
	static haptic_effect_h m_haptic_effect;

	static int m_instance_count;

	custom_content_handler *m_custom_content_handler;
	custom_protocol_handler *m_custom_protocol_handler;
};

#endif /* HTML5_FEATURE_MANAGER_H */

