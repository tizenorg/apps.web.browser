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

#ifndef CUSTOM_CONTENT_HANDLER_H
#define CUSTOM_CONTENT_HANDLER_H

#include "browser-object.h"

#include <Evas.h>

class webview;
class custom_content_handler : public browser_object {
public:
	custom_content_handler(webview *wv);
	~custom_content_handler(void);

	const char *get_redirect_uri(const char *origin_uri, const char *base_uri, const char *mime);
private:
	webview *m_webview;

	static void __registration_requested_cb(void *data, Evas_Object *obj, void *event_info);
	static void __is_registered_cb(void *data, Evas_Object *obj, void *event_info);
	static void __unregistration_requested_cb(void *data, Evas_Object *obj, void *event_info);
	static void __register_ok_cb(void *data, Evas_Object *obj, void *event_info);
	static void __register_cancel_cb(void *data, Evas_Object *obj, void *event_info);

	const char *m_redirect_uri;
};

#endif /* CUSTOM_CONTENT_HANDLER_H */

