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

#include "network-manager.h"

#include <ITapiModem.h>
#include <tapi_common.h>
#include <wifi.h>

#include "browser.h"
#include "browser-dlog.h"
#include "browser-string.h"
#include "browser-view.h"
#include "webview.h"

static void _wifi_activated_cb(wifi_error_e result, void *user_data)
{
	if (result == WIFI_ERROR_NONE)
		BROWSER_LOGD("wifi activated Succeeded\n");
	else
		BROWSER_LOGE("wifi activated Failed, err : %d\n", result);
}

static void _wifi_deactivated_cb(wifi_error_e result, void *user_data)
{
	if (result == WIFI_ERROR_NONE)
		BROWSER_LOGD("wifi de-activated Succeeded\n");
	else
		BROWSER_LOGE("wifi de-activated Failed, err : %d\n", result);
}

static void _change_flight_mode_cb(TapiHandle *handle, int result, void *data, void *user_data)
{
	BROWSER_LOGD("result:%d", result);
}

network_manager::network_manager(void)
:
	m_already_configured(EINA_FALSE)
	,m_network_need_wakeup(EINA_FALSE)
	,m_handle(NULL)
	,m_wifi_popup(NULL)
{
	BROWSER_LOGD("");
}

network_manager::~network_manager(void)
{
	BROWSER_LOGD("");
	if (m_handle)
		connection_destroy(m_handle);

	vconf_ignore_key_changed(VCONFKEY_NETWORK_CELLULAR_STATE, __network_state_changed_cb);
}

void network_manager::init(void)
{
	BROWSER_LOGD("");

	if (connection_create(&m_handle) < 0) {
		BROWSER_LOGE("connection_create failed");
		return;
	}

	if ((_get_cellular_state() == CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE)
		&& (_get_wifi_state() != CONNECTION_WIFI_STATE_CONNECTED)) {
		set_network_unlock(EINA_TRUE);
	}

	_configure_proxy_address();

	vconf_notify_key_changed(VCONFKEY_NETWORK_CELLULAR_STATE, __network_state_changed_cb, this);
}

void network_manager::_configure_proxy_address(void)
{
	BROWSER_LOGD("");

	if (m_already_configured == EINA_TRUE) {
		BROWSER_LOGD("proxy was already configured");
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

	_set_proxy();

	m_already_configured = EINA_TRUE;
}

Eina_Bool network_manager::_turn_wifi_on_off(Eina_Bool on_off)
{
	BROWSER_LOGD("");

	/* To do!! considering the thederting case */

	int ret = WIFI_ERROR_NONE;

	set_network_unlock(EINA_FALSE);

	ret = wifi_initialize();
	if (ret != WIFI_ERROR_NONE) {
		BROWSER_LOGE("Fail to initialize Wi-Fi device [%d]\n", ret);
		return EINA_FALSE;
	}

	if (on_off == EINA_TRUE) {
		ret = wifi_activate(_wifi_activated_cb, NULL);
		if (ret != WIFI_ERROR_NONE) {
			BROWSER_LOGE("Fail to activate Wi-Fi device [%d]\n", ret);
			ret = wifi_deinitialize();
			if (ret != WIFI_ERROR_NONE)
				BROWSER_LOGE("Fail to wifi_deinitialize [%d]\n", ret);

			return EINA_FALSE;
		}
	} else {
		ret = wifi_deactivate(_wifi_deactivated_cb, NULL);
		if (ret != WIFI_ERROR_NONE) {
			BROWSER_LOGE("Fail to activate Wi-Fi device [%d]\n", ret);
			ret = wifi_deinitialize();
			if (ret != WIFI_ERROR_NONE)
				BROWSER_LOGE("Fail to wifi_deinitialize [%d]\n", ret);

			return EINA_FALSE;
		}
	}

	ret = wifi_deinitialize();
	if (ret != WIFI_ERROR_NONE)
		BROWSER_LOGE("Fail to wifi_deinitialize [%d]\n", ret);

	_set_proxy();

	return EINA_TRUE;
}

Eina_Bool network_manager::_turn_data_network_on_off(Eina_Bool on_off)
{
	BROWSER_LOGD("");

	int sim_status = VCONFKEY_TELEPHONY_SIM_UNKNOWN;
	int flight_mode = 0;
	int ret = 0;

	set_network_unlock(EINA_FALSE);

	if (on_off == EINA_TRUE) {
		ret = vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT, &sim_status);
		if (ret < 0) {
			BROWSER_LOGE("Failed to get sim status");
			return EINA_FALSE;
		}
		BROWSER_LOGD("sim_status is [%d]", sim_status);

		ret = vconf_get_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, &flight_mode);
		if (ret < 0) {
			BROWSER_LOGE("Failed to get flight mode");
			return EINA_FALSE;
		}
		BROWSER_LOGD("flight_mode is [%d]", flight_mode);

		if (flight_mode) {
			m_browser->get_browser_view()->show_msg_popup(BR_STRING_WARNING,
														BR_STRING_FLIGHT_MODE_WARNING,
														BR_STRING_ALLOW,
														__leave_flghtmode_confirm_cb,
														BR_STRING_NO,
														__msg_cancel_cb,
														this);
		} else if (sim_status != VCONFKEY_TELEPHONY_SIM_INSERTED) {
			m_browser->get_browser_view()->show_msg_popup(BR_STRING_INSERT_SIM_CARD, 3);
		} else {
			/* To do : Make step to handle data network if it needs */
		}
	}else {
		/* To do : Make step to handle data network off if it needs */
	}

	_set_proxy();

	return EINA_TRUE;
}

void network_manager::show_confirm_network_unlock_popup(void)
{
	set_network_unlock(EINA_FALSE);

	if ((_get_wifi_state() == CONNECTION_WIFI_STATE_DEACTIVATED)
		&& _get_cellular_state() == CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE)
	m_wifi_popup = m_browser->get_browser_view()->show_msg_popup(NULL,
												BR_STRING_ENABLE_NETWORK_Q,
												BR_STRING_WIFI,
												__network_enable_wifi_cb,
												BR_STRING_DATA_NETWORK,
												__network_enable_cellular_cb,
												this,
												BR_STRING_CANCEL,
												__msg_cancel_cb);
}

void network_manager::_set_proxy(void)
{
	char *proxy = NULL;
	connection_address_family_e family = CONNECTION_ADDRESS_FAMILY_IPV4;

	if (connection_get_proxy(m_handle, family, &proxy) < 0) {
		BROWSER_LOGE("connection_get_proxy failed");
		return;
	}

	BROWSER_LOGD("proxy = [%s]", proxy);

	if (proxy && strlen(proxy)) {
		/* if the proxy address is '0.0.0.0', do not set the proxy (default NULL) */
		if (strcmp(proxy, "0.0.0.0"))
			m_webview_context->set_proxy_address((const char *)proxy);
	}
}

connection_type_e network_manager::_get_connection_type(void)
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

connection_cellular_state_e network_manager::_get_cellular_state(void)
{
	BROWSER_LOGD("");

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
	BROWSER_LOGD("");

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

void network_manager::__type_changed_cb(connection_type_e type, void *data)
{
	BROWSER_LOGD("");
}

void network_manager::__ip_changed_cb(const char* ipv4, const char* ipv6, void *data)
{
	BROWSER_LOGD("");

	network_manager *nm = (network_manager *)data;
	nm->_set_proxy();
}

void network_manager::__network_state_changed_cb(keynode_t *key_node,void *user_data)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(key_node);
	EINA_SAFETY_ON_NULL_RETURN(user_data);

	network_manager *nm = (network_manager *)user_data;

	if ((nm->_get_cellular_state() == CONNECTION_CELLULAR_STATE_OUT_OF_SERVICE)
		&& (nm->_get_wifi_state() != CONNECTION_WIFI_STATE_CONNECTED))
		nm->set_network_unlock(EINA_TRUE);
}

void network_manager::__leave_flghtmode_confirm_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	TapiHandle *handle = tel_init(NULL);
	if (!handle) {
		BROWSER_LOGD("tel_init error");
		return;
	}

	if (tel_set_flight_mode(handle, TAPI_POWER_FLIGHT_MODE_LEAVE, _change_flight_mode_cb, NULL) == TAPI_API_SUCCESS)
		vconf_set_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, FALSE);
	else
		BROWSER_LOGE("tel_set_flight_mode error");

	if (tel_deinit(handle) != 0)
		BROWSER_LOGE("tel_deinit error");

	int sim_status = VCONFKEY_TELEPHONY_SIM_UNKNOWN;
	int ret = vconf_get_int(VCONFKEY_TELEPHONY_SIM_SLOT, &sim_status);
	if (ret < 0) {
		BROWSER_LOGE("Failed to get sim status");
		return;
	}

	if (sim_status != VCONFKEY_TELEPHONY_SIM_INSERTED)
		m_browser->get_browser_view()->show_msg_popup(BR_STRING_INSERT_SIM_CARD, 3);

	return;
}

void network_manager::__msg_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	network_manager *nm = m_browser->get_network_manager();

	nm->m_wifi_popup = NULL;
}

void network_manager::__network_enable_wifi_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	network_manager *nm = m_browser->get_network_manager();
	nm->_turn_wifi_on_off(EINA_TRUE);
	nm->m_wifi_popup = NULL;
}

void network_manager::__network_enable_cellular_cb(void *data, Evas_Object *obj, void *event_info)
{
	BROWSER_LOGD("");
	EINA_SAFETY_ON_NULL_RETURN(data);

	network_manager *nm = m_browser->get_network_manager();
	nm->_turn_data_network_on_off(EINA_TRUE);

	nm->m_wifi_popup = NULL;
}

void network_manager::destroy_wifi_popup(void)
{
	BROWSER_LOGD("");
	if (m_wifi_popup) {
		evas_object_del(m_wifi_popup);
		m_wifi_popup = NULL;
	}
}

