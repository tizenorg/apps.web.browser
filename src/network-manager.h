/*
 * Copyright 2013  Samsung Electronics Co., Ltd
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
 * Contact: Hyerim Bae <hyerim.bae@samsung.com>
 *
 */

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <vconf.h>

#include "browser.h"
#include "browser-object.h"
#include "net_connection.h"

/* If you want to access the user_agent_manager instance, user browser::get_network_manager() */
class network_manager : public common_view {
public:
	network_manager(void);
	~network_manager(void);

	/* This func should be called for the first time when try to load webpage not launch time for reducing launching time. */
	void init(void);
	Eina_Bool get_network_connected(void);
	Eina_Bool check_network_use_policy(void);
	connection_type_e get_connection_type(void);
	void destroy_network_allow_popup(void);
	char *get_APN_homepage_URL(void);
	Eina_Bool is_wifi_connect_popup_shown(void) { return m_is_wifi_connect_popup_shown; }

private:
	void _check_data_roaming_application_policy(void);
	connection_cellular_state_e _get_cellular_state(void);
	connection_wifi_state_e _get_wifi_state(void);
	connection_bt_state_e _get_bt_tetherting_state(void);
	connection_ethernet_state_e _get_usb_tetherting_state(void);
	Eina_Bool _check_sim_inserted(void);
	Eina_Bool _turn_wifi_on(void);
	Eina_Bool _turn_data_network_on(void);

	static void __type_changed_cb(connection_type_e type, void *data);
	static void __ip_changed_cb(const char* ipv4, const char* ipv6, void *data);
	static void __proxy_changed_cb(const char* ipv4, const char* ipv6, void *data);
	static void __network_enable_cancel_cb(void *data, Evas_Object *obj, void *event_info);
	static void __network_enable_cellular_cb(void *data, Evas_Object *obj, void *event_info);
	static void __network_enable_wifi_cb(void *data, Evas_Object *obj, void *event_info);

	connection_h m_handle;
	connection_cellular_state_e m_celluar_state;
	Evas_Object *m_network_allow_popup;
	Eina_Bool m_is_wifi_connect_popup_shown;
};
#endif /* NETWORK_MANAGER_H */

