/*
 *  browser
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef CUSTOM_CONTENT_HANDLER_H
#define CUSTOM_CONTENT_HANDLER_H

#include "browser-config.h"

class Browser_Common_View;
class Custom_Content_Handler : public Browser_Common_View{
public:
	Custom_Content_Handler(void);
	~Custom_Content_Handler(void);

	void activate(Evas_Object *ewk_view);
	void deactivate(Evas_Object *ewk_view);
	const char *get_redirect_uri(const char *orgin, const char *base_uri, const char *mime);
private:
	const char *_get_content_uri(const char *base_uri, const char *mime);
	Eina_Bool _register_content_handler(const char *base_uri, const char *mime, const char *uri, Eina_Bool allow);
	Eina_Bool _unregister_content_handler(const char *base_uri, const char *mime, const char *uri);
	void _show_register_content_handler_confirm_popup(const char *message, void *data);

	static void __isregistered_cb(void *data, Evas_Object *obj, void *event_info);
	static void __registration_requested_cb(void *data, Evas_Object *obj, void *event_info);
	static void __unregistration_requested_cb(void *data, Evas_Object *obj, void *event_info);
	static void __register_protocol_ok_cb(void* data, Evas_Object* obj, void* event_info);
	static void __register_protocol_cancel_cb(void* data, Evas_Object* obj, void* event_info);

	const char *m_custom_content_uri;
	const char *m_redirect_uri;
};

#endif /* CUSTOM_CONTENT_HANDLER_H */

