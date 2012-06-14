/*
  * Copyright 2012  Samsung Electronics Co., Ltd
  *
  * Licensed under the Flora License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *    http://www.tizenopensource.org/license
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */

#include "browser-view.h"
#include "browser-network-manager.h"

Browser_Network_Manager::Browser_Network_Manager(void)
:	m_browser_view(NULL)
{
	BROWSER_LOGD("[%s]", __func__);
	int network_status = 0;
	if (vconf_get_int(VCONFKEY_NETWORK_STATUS, &network_status) < 0) {
		BROWSER_LOGE("vconf_get_int failed");
	}

	if (network_status == VCONFKEY_NETWORK_OFF)
		ewk_network_state_notifier_online_set(EINA_FALSE);
	else
		ewk_network_state_notifier_online_set(EINA_TRUE);

	char *proxy = vconf_get_str(VCONFKEY_NETWORK_PROXY);
	if (!proxy || !strlen(proxy) || strstr(proxy, "0.0.0.0")) {
		BROWSER_LOGE("proxy address is null");
		ewk_network_proxy_uri_set(NULL);
	} else {
		ewk_network_proxy_uri_set(proxy);
	}

	if (proxy)
		free(proxy);

	if (vconf_notify_key_changed(VCONFKEY_NETWORK_CONFIGURATION_CHANGE_IND,
						__network_changed_ind_cb, this) < 0) {
		BROWSER_LOGE("vconf_notify_key_changed failed");
	}
}

Browser_Network_Manager::~Browser_Network_Manager(void)
{
	BROWSER_LOGD("[%s]", __func__);
	if (vconf_ignore_key_changed(VCONFKEY_NETWORK_CONFIGURATION_CHANGE_IND,
						__network_changed_ind_cb) < 0)
		BROWSER_LOGE("vconf_ignore_key_changed failed");
}

Eina_Bool Browser_Network_Manager::init(Browser_View *browser_view, Evas_Object *webview)
{
	BROWSER_LOGD("[%s]", __func__);

	m_browser_view = browser_view;

	elm_webview_scheme_callback_set(webview, "http", __connection_cb);
	elm_webview_scheme_callback_set(webview, "https", __connection_cb);

	return EINA_TRUE;
}

Eina_Bool Browser_Network_Manager::__connection_cb(Evas_Object *webview, const char *uri)
{
	BROWSER_LOGD("[%s]", __func__);

	int network_status = 0;
	if (vconf_get_int(VCONFKEY_NETWORK_STATUS, &network_status) < 0) {
		BROWSER_LOGE("vconf_get_int failed");
		return EINA_FALSE;
	}

	return EINA_TRUE;
}

void Browser_Network_Manager::__network_changed_ind_cb(keynode_t *keynode, void *data)
{
	BROWSER_LOGD("[%s]", __func__);
	if (!data)
		return;

	Browser_Network_Manager *network_manager = (Browser_Network_Manager *)data;

	int network_status = 0;
	if (vconf_get_int(VCONFKEY_NETWORK_STATUS, &network_status) < 0) {
		BROWSER_LOGE("vconf_get_int failed");
		return;
	}

	if (network_status == VCONFKEY_NETWORK_OFF) {
		BROWSER_LOGD("network off");
		ewk_network_state_notifier_online_set(EINA_FALSE);
		return;
	} else {
		BROWSER_LOGD("network on");
		ewk_network_state_notifier_online_set(EINA_TRUE);
	}

	char *proxy = vconf_get_str(VCONFKEY_NETWORK_PROXY);
	if (!proxy || !strlen(proxy) || strstr(proxy, "0.0.0.0")) {
		BROWSER_LOGD("proxy is null");
		ewk_network_proxy_uri_set(NULL);
	} else {
		BROWSER_LOGD("proxy = %s", proxy);
		ewk_network_proxy_uri_set(proxy);
	}

	if (proxy)
		free(proxy);

	network_manager->m_browser_view->stop_and_reload();
}

