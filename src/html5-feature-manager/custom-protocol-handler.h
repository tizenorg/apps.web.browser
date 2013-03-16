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

#ifndef CUSTOM_PROTOCOL_HANDLER_H
#define CUSTOM_PROTOCOL_HANDLER_H

#include "browser-config.h"

class Browser_Common_View;
class Custom_Protocol_Handler : public Browser_Common_View{
public:
	Custom_Protocol_Handler(void);
	~Custom_Protocol_Handler(void);

	void activate(Evas_Object *ewk_view);
	void deactivate(Evas_Object *ewk_view);
	const char *get_protocol_uri(const char *protocol);
	const char *get_registered_converted_protocol(const char *uri);
	Eina_Bool unregister_protocol(const char *protocol, const char *uri);
private:
	Eina_Bool _save_protocol_handler(const char *protocol, const char *uri, Eina_Bool allow);
	void _show_register_protocol_confirm_popup(const char *message, void *data);

	static void __isregistered_cb(void *data, Evas_Object *obj, void *event_info);
	static void __registration_requested_cb(void *data, Evas_Object *obj, void *event_info);
	static void __unregistration_requested_cb(void *data, Evas_Object *obj, void *event_info);
	static void __register_protocol_ok_cb(void* data, Evas_Object* obj, void* event_info);
	static void __register_protocol_cancel_cb(void* data, Evas_Object* obj, void* event_info);

	const char *m_protocol_uri;
	const char *m_protocol_converted_uri;
};

#endif /* CUSTOM_PROTOCOL_HANDLER_H */

