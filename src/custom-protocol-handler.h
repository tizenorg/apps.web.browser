
/*
 *  browser
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef CUSTOM_PROTOCOL_HANDLER_H
#define CUSTOM_PROTOCOL_HANDLER_H

#include "browser-object.h"

#include <Evas.h>

class webview;
class custom_protocol_handler : public browser_object {
public:
	custom_protocol_handler(webview *wv);
	~custom_protocol_handler(void);

	const char *get_protocol_from_uri(const char *origin_uri);
private:
	webview *m_webview;

	static void __registration_requested_cb(void *data, Evas_Object *obj, void *event_info);
	static void __is_registered_cb(void *data, Evas_Object *obj, void *event_info);
	static void __unregistration_requested_cb(void *data, Evas_Object *obj, void *event_info);
	static void __register_ok_cb(void *data, Evas_Object *obj, void *event_info);
	static void __register_cancel_cb(void *data, Evas_Object *obj, void *event_info);

	const char *m_protocol_converted_uri;
};

#endif /* CUSTOM_PROTOCOL_HANDLER_H */

