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

#ifndef AUTHENTICATION_MANAGER_H
#define AUTHENTICATION_MANAGER_H

#include "browser-object.h"
#include "webview.h"

class authentication_manager : public browser_object {
public:
	authentication_manager(webview *wv);
	~authentication_manager(void);
private:
	void _show_id_password_popup(const char *msg);
	static void __ok_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __cancel_button_cb(void *data, Evas_Object *obj, void *event_info);
	static void __authentication_challenge_cb(void *data, Evas_Object *obj, void *event_info);

	webview *m_webview;
	Ewk_Auth_Challenge *m_auth_challenge;
	Evas_Object *m_id_field;
	Evas_Object *m_password_field;
};

#endif /* AUTHENTICATION_MANAGER_H */

