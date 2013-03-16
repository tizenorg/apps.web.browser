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

#ifndef CERTIFICATE_MANAGER_H
#define CERTIFICATE_MANAGER_H

#include "browser-object.h"
#include "webview.h"

class certificate_manager : public browser_object {
public:
	certificate_manager(webview *wv);
	~certificate_manager(void);
private:
	webview *m_webview;

	static void __certi_allow_cb(void *data, Evas_Object *obj, void *event_info);
	static void __certi_deny_cb(void *data, Evas_Object *obj, void *event_info);
	static void __request_certi_cb(void *data, Evas_Object *obj, void *event_info);

	static int m_instance_count;
};

#endif /* CERTIFICATE_MANAGER_H */

