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

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <vconf.h>

#include "browser-object.h"
#include "net_connection.h"

/* If you want to access the user_agent_manager instance, user browser::get_network_manager() */
class network_manager : public browser_object {
public:
	network_manager(void);
	~network_manager(void);

	/* This func should be called for the first time when try to load webpage not launch time for reducing launching time. */
	void init(void);
	void show_confirm_network_unlock_popup(void);
	Eina_Bool check_network_unlock(void) { return m_network_need_wakeup; };
	void set_network_unlock(Eina_Bool network_need_wakeup) { m_network_need_wakeup = network_need_wakeup; };

private:
	void _set_proxy(void);
	void _configure_proxy_address(void);
	connection_type_e _get_connection_type(void);
	connection_cellular_state_e _get_cellular_state(void);
	connection_wifi_state_e _get_wifi_state(void);
	Eina_Bool _turn_wifi_on_off(Eina_Bool on_off);
	Eina_Bool _turn_data_network_on_off(Eina_Bool on_off);

	static void __type_changed_cb(connection_type_e type, void *data);
	static void __ip_changed_cb(const char* ipv4, const char* ipv6, void *data);
	static void __network_state_changed_cb(keynode_t *key_node,void *user_data);
	static void __leave_flghtmode_confirm_cb(void *data, Evas_Object *obj, void *event_info);
	static void __msg_cancel_cb(void *data, Evas_Object *obj, void *event_info);
	static void __network_enable_cellular_cb(void *data, Evas_Object *obj, void *event_info);
	static void __network_enable_wifi_cb(void *data, Evas_Object *obj, void *event_info);

	Eina_Bool m_already_configured;
	Eina_Bool m_network_need_wakeup;
	connection_h m_handle;
};

#endif /* NETWORK_MANAGER_H */

