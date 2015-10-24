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

#include "network-manager.h"

#include <app_control.h>
#include <ITapiModem.h>
#include <tapi_common.h>
#include <wifi.h>

#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "preference.h"
#include "webview.h"

network_manager::network_manager(void)
:
	  m_handle(NULL)
	, m_celluar_state(CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE)
	, m_network_allow_popup(NULL)
	, m_is_wifi_connect_popup_shown(EINA_FALSE)
{
	BROWSER_LOGD("");
}

network_manager::~network_manager(void)
{
	BROWSER_LOGD("");
	if (m_handle)
		connection_destroy(m_handle);
}

void network_manager::init(void)
{
	BROWSER_LOGD("");

	if (connection_create(&m_handle) < 0) {
		BROWSER_LOGE("connection_create failed");
		return;
	}

	if (connection_set_type_changed_cb(m_handle, __type_changed_cb, this) < 0) {
		BROWSER_LOGE("connection_set_type_changed_cb failed");
		return;
	}

	if (connection_set_ip_address_changed_cb(m_handle, __ip_changed_cb, this) < 0) {
		BROWSER_LOGE("connection_set_ip_address_changed_cb failed");
		return;
	}
}

Eina_Bool network_manager::_check_sim_inserted(void)
{
	int sim_status = VCONFKEY_TELEPHONY_SIM_UNKNOWN;
	int ret = vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT, &sim_status);
	if (ret < 0) {
		BROWSER_LOGE("Failed to get sim status");
		return EINA_FALSE;
	}

	if (sim_status != VCONFKEY_TELEPHONY_SIM_INSERTED) {
		BROWSER_LOGI("Simcard is missed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

Eina_Bool network_manager::_turn_wifi_on(void)
{
	BROWSER_LOGD("");

	/* To do!! considering the tethering case */
	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("app_control_create failed");
		return EINA_FALSE;
	}
	if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
		BROWSER_LOGE("Fail to launch app_control operation");
		app_control_destroy(app_control);
		return EINA_FALSE;
	}
	app_control_destroy(app_control);

	return EINA_TRUE;
}

Eina_Bool network_manager::_turn_data_network_on(void)
{
	BROWSER_LOGD("m_celluar_state[%d]", m_celluar_state);

	Eina_Bool ret_value = EINA_FALSE;
	app_control_h app_control = NULL;
	if (app_control_create(&app_control) < 0) {
		BROWSER_LOGE("app_control_create failed");
		return EINA_FALSE;
	}

	switch(m_celluar_state) {
		case CONNECTION_CELLULAR_STATE_FLIGHT_MODE:
		if (app_control_set_app_id(app_control, "setting-flightmode-efl") < 0) {
			BROWSER_LOGE("Fail to app_control_set_app_id");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
		if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
			BROWSER_LOGE("Fail to launch app_control operation");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
		ret_value = EINA_TRUE;
		break;

		case CONNECTION_CELLULAR_STATE_ROAMING_OFF:
		case CONNECTION_CELLULAR_STATE_CALL_ONLY_AVAILABLE:
		case CONNECTION_CELLULAR_STATE_AVAILABLE:
		if (app_control_set_app_id(app_control, "setting-network-efl") < 0) {
			BROWSER_LOGE("Fail to app_control_set_app_id");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
		if (app_control_send_launch_request(app_control, NULL, NULL) < 0) {
			BROWSER_LOGE("Fail to launch app_control operation");
			app_control_destroy(app_control);
			return EINA_FALSE;
		}
		ret_value = EINA_TRUE;
		break;

		case CONNECTION_CELLULAR_STATE_CONNECTED:
		default:
		break;
	}
	app_control_destroy(app_control);

	return ret_value;
}

Eina_Bool network_manager::get_network_connected(void)
{
	if ((_get_wifi_state() == CONNECTION_WIFI_STATE_CONNECTED)
		|| _get_cellular_state() == CONNECTION_CELLULAR_STATE_CONNECTED
		|| _get_bt_tetherting_state() == CONNECTION_BT_STATE_CONNECTED
		|| _get_usb_tetherting_state() == CONNECTION_ETHERNET_STATE_CONNECTED)
		return EINA_TRUE;
	else
		return EINA_FALSE;
}

Eina_Bool network_manager::check_network_use_policy(void)
{
	if (m_network_allow_popup) {
		BROWSER_LOGD("There is already shown popup for netwkr asking");
		return EINA_FALSE;
	}

	/* Check pre conditions */
	m_celluar_state = _get_cellular_state();
	BROWSER_LOGD("get_connection_type()[%d]", get_connection_type());

	if (get_connection_type() == CONNECTION_TYPE_DISCONNECTED) {
		/* No sim mode */
		if (_check_sim_inserted() == EINA_FALSE || m_celluar_state == CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE) {
			m_is_wifi_connect_popup_shown = EINA_TRUE;
			m_network_allow_popup = show_msg_popup(BR_STRING_NO_NETWORK_CONNECTION,
												BR_STRING_CONNECT_TO_WI_FI,
												__network_enable_cancel_cb,
												BR_STRING_CANCEL, __network_enable_cancel_cb,
												BR_STRING_WIFI, __network_enable_wifi_cb, this);
		} else if (m_celluar_state == CONNECTION_CELLULAR_STATE_FLIGHT_MODE) {
			m_is_wifi_connect_popup_shown = EINA_TRUE;
			m_network_allow_popup = show_msg_popup(BR_STRING_NO_NETWORK_CONNECTION,
												BR_STRING_FLIGHT_MODE,
												__network_enable_cancel_cb,
												BR_STRING_CANCEL, __network_enable_cancel_cb,
												BR_STRING_WIFI, __network_enable_wifi_cb,
												this,
												BR_STRING_SETTINGS, __network_enable_cellular_cb);
		} else if (m_celluar_state == CONNECTION_CELLULAR_STATE_CALL_ONLY_AVAILABLE
				|| m_celluar_state == CONNECTION_CELLULAR_STATE_AVAILABLE) {
			m_is_wifi_connect_popup_shown = EINA_TRUE;
			m_network_allow_popup = show_msg_popup(BR_STRING_NO_NETWORK_CONNECTION,
												BR_STRING_MOBILE_DATA_TURNED_OFF,
												__network_enable_cancel_cb,
												BR_STRING_CANCEL, __network_enable_cancel_cb,
												BR_STRING_WIFI, __network_enable_wifi_cb,
												this,
												BR_STRING_SETTINGS, __network_enable_cellular_cb);
		} else if (m_celluar_state == CONNECTION_CELLULAR_STATE_ROAMING_OFF) {
			_check_data_roaming_application_policy();
		} else {
			BROWSER_LOGE("******************************************************");
			BROWSER_LOGE("Browser cannot recognize current network status.");
			BROWSER_LOGE("connection type is [CONNECTION_TYPE_DISCONNECTED] and");
			BROWSER_LOGE("celluar_state from network part says it can use network");
			BROWSER_LOGE("******************************************************");
		}
	}

	return EINA_FALSE;
}

connection_type_e network_manager::get_connection_type(void)
{
	BROWSER_LOGD("");

	connection_type_e network_type = CONNECTION_TYPE_DISCONNECTED;

	if (!m_handle) {
		BROWSER_LOGE("m_handle is NULL");
		return CONNECTION_TYPE_DISCONNECTED;
	}

	if (connection_get_type(m_handle, &network_type) < 0) {
		BROWSER_LOGD("Fail to get network status");
		return CONNECTION_TYPE_DISCONNECTED;
	}
	BROWSER_LOGD("network_type[%d]", network_type);
	return network_type;
}

char *network_manager::get_APN_homepage_URL(void)
{
	BROWSER_LOGD("");
	RETV_MSG_IF(!m_handle, NULL, "m_handle is NULL");

	connection_profile_h profile;
	char *APN_home_url = NULL;

	if (connection_get_default_cellular_service_profile(m_handle,
							CONNECTION_CELLULAR_SERVICE_TYPE_INTERNET, &profile)
							!= CONNECTION_ERROR_NONE) {
							BROWSER_LOGD("get profile is failed");
							return NULL;
	}

	if (connection_profile_get_cellular_home_url(profile, &APN_home_url)
							== CONNECTION_ERROR_NONE) {
		if (APN_home_url && (strlen(APN_home_url) != 0)) {
			BROWSER_SECURE_LOGD("APN homepage : %s", APN_home_url);
			return APN_home_url;
		}
		BROWSER_SECURE_LOGD("APN_home_url is empty");
	}
	BROWSER_LOGD("getting APN URL is failed.");
	return NULL;
}

void network_manager::_check_data_roaming_application_policy(void)
{
	m_is_wifi_connect_popup_shown = EINA_TRUE;
	m_network_allow_popup = show_msg_popup(BR_STRING_NO_NETWORK_CONNECTION,
										BR_STRING_ENABLE_DATA_ROAMING,
										__network_enable_cancel_cb,
										BR_STRING_CANCEL, __network_enable_cancel_cb,
										BR_STRING_WIFI, __network_enable_wifi_cb,
										this,
										BR_STRING_SETTINGS, __network_enable_cellular_cb);
}

connection_cellular_state_e network_manager::_get_cellular_state(void)
{
	connection_cellular_state_e cellular_state = CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE;

	if (!m_handle) {
		BROWSER_LOGE("m_handle is NULL");
		return CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE;
	}

	if (connection_get_cellular_state(m_handle, &cellular_state) < 0) {
		BROWSER_LOGD("Fail to get network status");
		return CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE;
	}
	BROWSER_LOGD("cellular_state[%d]", cellular_state);
	return cellular_state;
}

connection_wifi_state_e network_manager::_get_wifi_state(void)
{
	connection_wifi_state_e wifi_state = CONNECTION_WIFI_STATE_DEACTIVATED;

	if (!m_handle) {
		BROWSER_LOGE("m_handle is NULL");
		return CONNECTION_WIFI_STATE_DEACTIVATED;
	}

	if (connection_get_wifi_state(m_handle, &wifi_state) < 0) {
		BROWSER_LOGD("Fail to get network status");
		return CONNECTION_WIFI_STATE_DEACTIVATED;
	}
	BROWSER_LOGD("wifi_state[%d]", wifi_state);
	return wifi_state;
}

connection_bt_state_e network_manager::_get_bt_tetherting_state(void)
{
	connection_bt_state_e bt_tetherting_state = CONNECTION_BT_STATE_DEACTIVATED;

	if (!m_handle) {
		BROWSER_LOGE("m_handle is NULL");
		return CONNECTION_BT_STATE_DEACTIVATED;
	}

	if (connection_get_bt_state(m_handle, &bt_tetherting_state) < 0) {
		BROWSER_LOGD("Fail to connection_get_bt_state");
		return CONNECTION_BT_STATE_DEACTIVATED;
	}
	BROWSER_LOGD("bt_tetherting_state[%d]", bt_tetherting_state);
	return bt_tetherting_state;
}

connection_ethernet_state_e network_manager::_get_usb_tetherting_state(void)
{
	connection_ethernet_state_e usb_tetherting_state = CONNECTION_ETHERNET_STATE_DEACTIVATED;

	if (!m_handle) {
		BROWSER_LOGE("m_handle is NULL");
		return CONNECTION_ETHERNET_STATE_DEACTIVATED;
	}

	if (connection_get_ethernet_state(m_handle, &usb_tetherting_state) < 0) {
		BROWSER_LOGD("Fail to get connection_get_ethernet_state");
		return CONNECTION_ETHERNET_STATE_DEACTIVATED;
	}
	BROWSER_LOGD("usb_tetherting_state[%d]", usb_tetherting_state);
	return usb_tetherting_state;
}

void network_manager::__type_changed_cb(connection_type_e type, void *data)
{
	BROWSER_LOGD("");
}

void network_manager::__ip_changed_cb(const char* ipv4, const char* ipv6, void *data)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	webview_context::instance()->network_session_changed();
}

void network_manager::__proxy_changed_cb(const char* ipv4, const char* ipv6, void *data)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");

	webview_context::instance()->network_session_changed();
}

void network_manager::__network_enable_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	network_manager *nm = m_browser->get_network_manager();
	nm->m_is_wifi_connect_popup_shown = EINA_FALSE;
	nm->destroy_network_allow_popup();
}

void network_manager::__network_enable_wifi_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	network_manager *nm = m_browser->get_network_manager();
	nm->m_is_wifi_connect_popup_shown = EINA_FALSE;
	nm->_turn_wifi_on();
	nm->destroy_network_allow_popup();
}

void network_manager::__network_enable_cellular_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	RET_MSG_IF(!data, "data is NULL");
	network_manager *nm = m_browser->get_network_manager();
	nm->m_is_wifi_connect_popup_shown = EINA_FALSE;
	nm->_turn_data_network_on();
	nm->destroy_network_allow_popup();
}

void network_manager::destroy_network_allow_popup(void)
{
	BROWSER_LOGD("");
	m_network_allow_popup = NULL;
}

